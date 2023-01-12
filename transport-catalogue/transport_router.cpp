#include "transport_router.h"

using namespace std;

namespace Router {

	TransportRouter::TransportRouter(RouterSettings settings, vector<Catalogue::Bus*> buses, Distances& distances)
		:Modules::Module(Modules::ModuleType::TransportRouter), buses_(std::move(buses)), settings_(settings), distances_(distances)
	{
		for (const auto bus : buses_) {
			stops_.insert(bus->stops.begin(), bus->stops.end());
		}
		const size_t vertex_count = stops_.size() * 2;
		graph_ = std::make_unique<graph::DirectedWeightedGraph<Time>>(vertex_count);
		LoadGraph();
		router_ = std::make_unique<graph::Router<Time>>(*graph_);
	}

	void TransportRouter::LoadGraph() {
		LoadStopsToGraph();
		LoadBusesToGraph();
	}

	void TransportRouter::LoadStopsToGraph() {

		VertexId id = 0;

		for (const Catalogue::Stop* stop : stops_) { //размещаем остановки на графе

			StopAsEdge stop_g{ id++, id++, settings_.bus_wait_time };
			stop_to_id_[stop] = stop_g;
			graph::EdgeId fresh_edge_id = graph_->AddEdge({ stop_g.in, stop_g.out, stop_g.wait });
			RouteItemPtr fresh_wait_item = make_shared<WaitItem>(stop, settings_.bus_wait_time);
			route_items_.insert(route_items_.begin() + fresh_edge_id, move(fresh_wait_item));
		}
	}

	void TransportRouter::LoadBusesToGraph() {

		for (const Catalogue::Bus* bus : buses_) {

			const vector<Catalogue::Stop*>& bus_stops = bus->stops;
			const size_t stop_count = bus_stops.size();

			if (stop_count <= 1) {
				continue;
			}
			
			//для каждой остановки добавляем все достижимые остановки в виде ребра: остановка1 - остановка2 - остановка3 это одно ребро "остановка1 - остановка 3" и тд
			for (size_t cur_stop = 0; cur_stop + 1 < stop_count; ++cur_stop) {

				double distance = 0;
				const graph::VertexId cur_stop_id = stop_to_id_.at(bus_stops[cur_stop]).out; //считаем вершиной "вершину после ожидания"

				for (size_t reachable_stop = cur_stop + 1; reachable_stop < stop_count; ++reachable_stop) {

					distance += GetDistance(bus_stops[reachable_stop - 1], bus_stops[reachable_stop]); //расстояния от текущей до достижимой остановки

					Time time = (distance * 60) / (settings_.bus_velocity * 1000.0); //км/ч в м/с
					graph::EdgeId fresh_edge_id = graph_->AddEdge({cur_stop_id, stop_to_id_.at(bus_stops[reachable_stop]).in, time});
					RouteItemPtr fresh_bus_item = make_shared<BusItem>(bus, reachable_stop - cur_stop, time);
					route_items_.insert(route_items_.begin() + fresh_edge_id, move(fresh_bus_item));
				}
			}
		}
	}

	optional<TransportRouter::RouteInfo> TransportRouter::FindRoute(const Catalogue::Stop* start, const Catalogue::Stop* stop) const {

		RouteInfo route_info;

		if (!stop_to_id_.count(start) || !stop_to_id_.count(stop)) {
			return nullopt;
		}

		 StopAsEdge departure = stop_to_id_.at(start);
		 StopAsEdge arrival = stop_to_id_.at(stop);

		 optional<graph::Router<Time>::RouteInfo> route = router_->BuildRoute(departure.in, arrival.in);

		 if (route.has_value()) {
			 route_info.time = route.value().weight;
			 for (graph::EdgeId edge_id : route.value().edges) {
				 route_info.route_items.push_back(route_items_[edge_id]);
			 }
			 return route_info;
		 }
		 else {
			 return nullopt;
		 }
	}

	 double TransportRouter::GetDistance(const Catalogue::Stop* stop1, const Catalogue::Stop* stop2) const{
		 if (distances_.count({ stop1, stop2 })) {
			 return distances_.at({ stop1, stop2 });
		 }
		 else if (distances_.count({ stop2, stop1 })) {
			 return distances_.at({ stop2, stop1 });
		 }
		 else {
			 throw runtime_error("No distance for a pair of stops"s);
		 }
	 }

}