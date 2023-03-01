#include "serialization.h"

using namespace std;

namespace Serialization {

	void SerializeCatalogueDB(transport_catalogue::Catalogue& catalogue_proto, const Catalogue::TransportCatalogue& catalogue) {

		for (const Catalogue::Stop* stop : catalogue.GetAllStops()) {
			transport_catalogue::Stop stop_proto;
			stop_proto.set_name(stop->name);
			stop_proto.set_latitude(stop->coords.lat);
			stop_proto.set_longitude(stop->coords.lng);
			*catalogue_proto.add_stops() = move(stop_proto);
		}

		for (const auto& [pair_of_stops, distance] : catalogue.GetDistances()) {
			transport_catalogue::Distance distance_proto;
			distance_proto.set_start(pair_of_stops.first->name);
			distance_proto.set_end(pair_of_stops.second->name);
			distance_proto.set_distance(distance);
			*catalogue_proto.add_distances() = move(distance_proto);
		}

		for (const Catalogue::Bus* bus : catalogue.GetAllBuses()) {
			transport_catalogue::Bus bus_proto;
			bus_proto.set_name(bus->name);
			for (const Catalogue::Stop* stop : bus->stops) {
				*bus_proto.add_stops() = stop->name;
			}
			bus_proto.set_end_stop(bus->end_stop->name);
			*catalogue_proto.add_buses() = move(bus_proto);
		}

	}

	transport_catalogue::Color SerializeColor(const svg::Color& color) {
		transport_catalogue::Color ser_color;
		if (holds_alternative<monostate>(color)) {
			ser_color.set_monostate(true);
		}
		else if (holds_alternative<string>(color)) {
			ser_color.set_string_val(get<string>(color));
		}
		else if (holds_alternative<svg::Rgb>(color)) {
			ser_color.mutable_rgb_val()->set_r(get<svg::Rgb>(color).red);
			ser_color.mutable_rgb_val()->set_g(get<svg::Rgb>(color).green);
			ser_color.mutable_rgb_val()->set_b(get<svg::Rgb>(color).blue);
		}
		else {
			ser_color.mutable_rgb_val()->set_r(get<svg::Rgba>(color).red);
			ser_color.mutable_rgb_val()->set_g(get<svg::Rgba>(color).green);
			ser_color.mutable_rgb_val()->set_b(get<svg::Rgba>(color).blue);
			ser_color.set_rgba(true);
			ser_color.set_opacity(get<svg::Rgba>(color).opacity);
		}
		return ser_color;
	}

	void SerializeMapSettings(transport_catalogue::Catalogue& catalogue_proto, const Visual::MapSettings& map_settings) {

		transport_catalogue::MapSettings map_settings_proto;

		map_settings_proto.set_width(map_settings.width);
		map_settings_proto.set_height(map_settings.height);
		map_settings_proto.set_padding(map_settings.padding);
		map_settings_proto.set_line_width(map_settings.line_width);
		map_settings_proto.set_stop_radius(map_settings.stop_radius);
		map_settings_proto.set_bus_label_font_size(map_settings.bus_label_font_size);
		map_settings_proto.mutable_bus_label_offset()->set_x(map_settings.bus_label_offset.first);
		map_settings_proto.mutable_bus_label_offset()->set_y(map_settings.bus_label_offset.second);
		map_settings_proto.set_stop_label_font_size(map_settings.stop_label_font_size);
		map_settings_proto.mutable_stop_label_offset()->set_x(map_settings.stop_label_offset.first);
		map_settings_proto.mutable_stop_label_offset()->set_y(map_settings.stop_label_offset.second);
		*map_settings_proto.mutable_underlayer_color() = SerializeColor(map_settings.underlayer_color);
		map_settings_proto.set_underlayer_width(map_settings.underlayer_width);
		map_settings_proto.set_stop_label_font_family(map_settings.stop_label_font_family);
		map_settings_proto.set_bus_label_font_family(map_settings.bus_label_font_family);
		for (const svg::Color& color : map_settings.color_palette) {
			*map_settings_proto.add_color_palette() = SerializeColor(color);
		}

		*catalogue_proto.mutable_map_settings() = move(map_settings_proto);
	}

	void SerializeRouterSettings(transport_catalogue::Catalogue& catalogue_proto, const Router::RouterSettings& router_settings) {

		catalogue_proto.mutable_router() -> mutable_routing_settings() ->set_bus_wait_time(router_settings.bus_wait_time);
		catalogue_proto.mutable_router()->mutable_routing_settings()->set_bus_wait_time(router_settings.bus_velocity);
	}

	void SerializeGraph(transport_catalogue::Catalogue& catalogue_proto, const graph::DirectedWeightedGraph<Router::TransportRouter::Time>& graph) {

		for (size_t i = 0; i < graph.GetEdgeCount(); ++i) {
			auto ser_edge = catalogue_proto.mutable_router()->mutable_graph()->add_edge();
			graph::Edge cur_edge = graph.GetEdge(i);

			ser_edge->set_from(cur_edge.from);
			ser_edge->set_to(cur_edge.to);
			ser_edge->set_weight(cur_edge.weight);
		}

		for (size_t i = 0; i < graph.GetVertexCount(); ++i) {
			auto ser_inc_list = catalogue_proto.mutable_router()->mutable_graph()->add_incidence_list();
			auto edges_range = graph.GetIncidentEdges(i);

			for (const auto& edge_id : edges_range) {
				ser_inc_list->add_edge_id(edge_id);
			}
		}
	}

	void SerializeStopToIdsMap(transport_catalogue::Catalogue& catalogue_proto, 
		const std::unordered_map<const Catalogue::Stop*, Router::TransportRouter::StopAsEdge, Router::TransportRouter::PointerHasher>& stop_to_id) {
		for (const auto& [key, val] : stop_to_id) {
			auto ser_stid = catalogue_proto.mutable_router()->add_stops_to_ids();

			ser_stid->set_stop_name(key->name);
			ser_stid->mutable_stop_as_edge()->set_in(val.in);
			ser_stid->mutable_stop_as_edge()->set_out(val.out);
			ser_stid->mutable_stop_as_edge()->set_wait(val.wait);
		}
	}

	void SerializeRouteItems(transport_catalogue::Catalogue& catalogue_proto,
		const std::vector<Router::TransportRouter::RouteItemPtr>& route_items) {

		auto router_proto = catalogue_proto.mutable_router();

		using RouteItemType = Router::TransportRouter::RouteItem::ItemType;
		for (int i = 0; i < route_items.size(); ++i) {

			const Router::TransportRouter::RouteItemPtr& route_item = route_items[i];
			auto route_item_proto = router_proto->add_route_items();

			route_item_proto->set_id(i);
			if (route_item->GetType() == RouteItemType::WaitItem) {
				Router::TransportRouter::WaitItem* w_item = dynamic_cast<Router::TransportRouter::WaitItem*>(route_item.get());
				auto wait_item_proto = route_item_proto ->mutable_wait_item();
				wait_item_proto -> set_stop_name(w_item->stop->name);
				wait_item_proto -> set_time(w_item ->time);
			}
			else if (route_item->GetType() == RouteItemType::BusItem) {
				Router::TransportRouter::BusItem* b_item = dynamic_cast<Router::TransportRouter::BusItem*>(route_item.get());
				auto bus_item_proto = route_item_proto ->mutable_bus_item();
				bus_item_proto->set_bus_name(b_item->bus->name);
				bus_item_proto->set_span_count(b_item->span_count);
				bus_item_proto->set_time(b_item->time);
			}
		}
	}

	void SerializeRouter(transport_catalogue::Catalogue& catalogue_proto, const Router::TransportRouter& router) {
		SerializeRouterSettings(catalogue_proto, router.GetRouterSettings());
		SerializeGraph(catalogue_proto, router.GetGraph());
		SerializeStopToIdsMap(catalogue_proto, router.GetStopToIdMap());
		SerializeRouteItems(catalogue_proto, router.GetRouteItems());
	}

	void DoSerialization(std::filesystem::path file,
		const Catalogue::TransportCatalogue& catalogue,
		const Visual::MapSettings& map_settings,
		const Router::TransportRouter& router) {

		transport_catalogue::Catalogue catalogue_proto;

		SerializeCatalogueDB(catalogue_proto, catalogue);
		SerializeMapSettings(catalogue_proto, map_settings);
		SerializeRouter(catalogue_proto, router);

		ofstream output(file, std::ios::binary);

		catalogue_proto.SerializeToOstream(&output);
	}

	void DeSerializeCatalogueDB(const transport_catalogue::Catalogue& catalogue_proto, Catalogue::TransportCatalogue& restored_catalogue) {


		for (auto stop_proto : catalogue_proto.stops()) {
			restored_catalogue.AddStop(stop_proto.name(), { stop_proto.latitude(), stop_proto.longitude() });
		}

		for (auto bus_proto : catalogue_proto.buses()) {
			vector<string> stops_to_add;
			stops_to_add.reserve(bus_proto.stops().size());
			for (string stop : bus_proto.stops()) {
				stops_to_add.push_back(stop);
			}
			restored_catalogue.AddBus(bus_proto.name(), stops_to_add, bus_proto.end_stop());
		}

		std::unordered_map<std::pair<const Catalogue::Stop*, const Catalogue::Stop*>, std::size_t, Catalogue::PointerPairHasher> restored_distances;
		for (auto distance_proto : catalogue_proto.distances()) {
			const Catalogue::Stop& start = restored_catalogue.FindStop(distance_proto.start());
			const Catalogue::Stop& end = restored_catalogue.FindStop(distance_proto.end());
			restored_distances[{&start, & end}] = distance_proto.distance();
		}
		restored_catalogue.LoadDistances(restored_distances);
	}

	svg::Color DeSerializeColor(const transport_catalogue::Color& color) {
		svg::Color deser_color;

		if (color.has_rgb_val()) {
			if (color.rgba()) {
				deser_color = svg::Rgba(
				static_cast<uint8_t>(color.rgb_val().r()),
				static_cast<uint8_t>(color.rgb_val().g()),
				static_cast<uint8_t>(color.rgb_val().b()),
				color.opacity());
			}
			else {
				deser_color = svg::Rgb(
				static_cast<uint8_t>(color.rgb_val().r()),
				static_cast<uint8_t>(color.rgb_val().g()),
				static_cast<uint8_t>(color.rgb_val().b()));
			}
		}
		else if (!color.monostate()) {
			deser_color = svg::Color{ color.string_val() };
		}
		return deser_color;
	}

	void DeSerializeMapSettings(const transport_catalogue::Catalogue& catalogue_proto, Visual::MapSettings& map_settings) {

		transport_catalogue::MapSettings map_settings_proto = catalogue_proto.map_settings();;

		map_settings.width = map_settings_proto.width();
		map_settings.height = map_settings_proto.height();
		map_settings.padding = map_settings_proto.padding();
		map_settings.line_width = map_settings_proto.line_width();
		map_settings.stop_radius = map_settings_proto.stop_radius();
		map_settings.bus_label_font_size = map_settings_proto.bus_label_font_size();
		map_settings.bus_label_offset.first = map_settings_proto.mutable_bus_label_offset()->x();
		map_settings.bus_label_offset.second = map_settings_proto.mutable_bus_label_offset()->y();
		map_settings.stop_label_font_size = map_settings_proto.stop_label_font_size();
		map_settings.stop_label_offset.first = map_settings_proto.mutable_stop_label_offset()->x();
		map_settings.stop_label_offset.second = map_settings_proto.mutable_stop_label_offset()->y();
		map_settings.underlayer_color = DeSerializeColor(*map_settings_proto.mutable_underlayer_color());
		map_settings.underlayer_width = map_settings_proto.underlayer_width();
		map_settings.stop_label_font_family = map_settings_proto.stop_label_font_family();
		map_settings.bus_label_font_family = map_settings_proto.bus_label_font_family();

		vector<svg::Color> palette;
		for (const transport_catalogue::Color& color : map_settings_proto.color_palette()) {
			palette.push_back(DeSerializeColor(color));
		}
		map_settings.color_palette = palette;
	}

	void DeSerializeRouterSettings(const transport_catalogue::Catalogue& catalogue_proto, Router::RouterSettings& router_settings) {
		router_settings.bus_wait_time = catalogue_proto.router().routing_settings().bus_wait_time();
		router_settings.bus_velocity = catalogue_proto.router().routing_settings().bus_velocity();
	}

	void DeSerializeGraph(const transport_catalogue::Catalogue& catalogue_proto, graph::DirectedWeightedGraph<Router::TransportRouter::Time>& graph) {
		auto graph_proto = catalogue_proto.router().graph();
		auto& edges = graph.GetEdges();
		auto& incidence_lists = graph.GetIncidenceList();

		edges.clear();
		for (int i = 0; i < graph_proto.edge_size(); ++i) {
			graph::Edge<Router::TransportRouter::Time> edge = {
			  graph_proto.edge(i).from(),
			  graph_proto.edge(i).to(),
			  static_cast<Router::TransportRouter::Time>(graph_proto.edge(i).weight()) };
			edges.push_back(edge);
		}

		incidence_lists.resize(graph_proto.incidence_list_size());
		for (int i = 0; i < graph_proto.incidence_list_size(); ++i) {
			for (int j = 0; j < graph_proto.incidence_list(i).edge_id_size(); ++j) {
				incidence_lists[i].push_back(graph_proto.incidence_list(i).edge_id(j));
			}
		}
	}

	void DeSerializeStopToIdsMap(const transport_catalogue::Catalogue& catalogue_proto,
		std::unordered_map<const Catalogue::Stop*, Router::TransportRouter::StopAsEdge, Router::TransportRouter::PointerHasher>& stop_to_id_map,
		const Catalogue::TransportCatalogue& catalogue) {
		auto map_proto = catalogue_proto.router().stops_to_ids();
		for (const auto& stop_to_id : map_proto) {

			Router::TransportRouter::StopAsEdge stop_as_edge{
			  stop_to_id.stop_as_edge().in(),
			  stop_to_id.stop_as_edge().out(),
			  static_cast<Router::TransportRouter::Time>(stop_to_id.stop_as_edge().wait()) };
			stop_to_id_map[catalogue.GetStop(stop_to_id.stop_name())] = stop_as_edge;
		}
	}

	void DeSerializeRouteItems(const transport_catalogue::Catalogue& catalogue_proto,
		std::vector<Router::TransportRouter::RouteItemPtr>& route_items, const Catalogue::TransportCatalogue& catalogue) {

		auto route_items_proto = catalogue_proto.router().route_items();
		route_items.resize(route_items_proto.size());

		using RouteItemType = Router::TransportRouter::RouteItem::ItemType;
		for (const auto& route_item_proto : route_items_proto) {

			Router::TransportRouter::RouteItemPtr route_item;

			if (route_item_proto.has_wait_item()) {
				route_item = make_shared<Router::TransportRouter::WaitItem>(
					catalogue.GetStop(route_item_proto.wait_item().stop_name()),
					static_cast<Router::TransportRouter::Time>(route_item_proto.wait_item().time())
					);
			}
			else if (route_item_proto.has_bus_item()) {
				route_item = make_shared<Router::TransportRouter::BusItem>(
				catalogue.GetBus(route_item_proto.bus_item().bus_name()),
					route_item_proto.bus_item().span_count(),
					static_cast<Router::TransportRouter::Time>(route_item_proto.bus_item().time())
					);
			}
			route_items[route_item_proto.id()] = route_item;
		}
	}

	void DeSerializeRouter(const transport_catalogue::Catalogue& catalogue_proto, Router::TransportRouter& router, const Catalogue::TransportCatalogue& restored_catalogue) {
		DeSerializeRouterSettings(catalogue_proto, router.AccessRouterSettings());
		DeSerializeGraph(catalogue_proto, *router.AccessGraph());
		router.UpdateRouter(); //после нового графа надо обновить роутер
		DeSerializeStopToIdsMap(catalogue_proto, router.AccessStopToIdMap(), restored_catalogue);
		DeSerializeRouteItems(catalogue_proto, router.AccessRouteItems(), restored_catalogue);
		router.LoadCatalogueData(restored_catalogue);
	}

	void DoDeSerialization(std::filesystem::path file, Catalogue::TransportCatalogue& restored_catalogue, Visual::MapSettings& map_settings, Router::TransportRouter& router) {

		ifstream input;
		input.open(file, std::ios::binary | std::ios::in);

		transport_catalogue::Catalogue catalogue_proto;
		catalogue_proto.ParseFromIstream(&input);

		DeSerializeCatalogueDB(catalogue_proto, restored_catalogue);
		DeSerializeMapSettings(catalogue_proto, map_settings);
		DeSerializeRouter(catalogue_proto, router, restored_catalogue);
	}
}//end Serialization