#include "transport_router.h"

using namespace std;

namespace Router {

	TransportRouter::TransportRouter()
		:Modules::Module(Modules::ModuleType::TransportRouter)
	{
		graph_ = std::make_unique<graph::DirectedWeightedGraph<Time>>();
		router_ = std::make_unique<graph::Router<Time>>(*graph_);
	}

	TransportRouter::TransportRouter(RouterSettings settings, vector<const Catalogue::Bus*> buses, Distances& distances)
		:Modules::Module(Modules::ModuleType::TransportRouter), buses_(std::move(buses)), settings_(settings), distances_(&distances)
	{
		for (const auto bus : buses_) {
			stops_.insert(bus->stops.begin(), bus->stops.end());
		}
		const size_t vertex_count = stops_.size() * 2;
		graph_ = std::make_unique<graph::DirectedWeightedGraph<Time>>(vertex_count);
		LoadGraph();
		router_ = std::make_unique<graph::Router<Time>>(*graph_);
	}

	TransportRouter::TransportRouter(RouterSettings settings, Catalogue::TransportCatalogue& catalogue)
		:TransportRouter::TransportRouter(settings, catalogue.GetAllBuses(), catalogue.GetDistances())
	{
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

		static const double units_coeff = 60.0 / 1000;

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

					Time time = (distance * units_coeff) / settings_.bus_velocity; //км/ч в м/с за счет units_coeff
					graph::EdgeId fresh_edge_id = graph_->AddEdge({cur_stop_id, stop_to_id_.at(bus_stops[reachable_stop]).in, time});
					RouteItemPtr fresh_bus_item = make_shared<BusItem>(bus, reachable_stop - cur_stop, time);
					route_items_.insert(route_items_.begin() + fresh_edge_id, move(fresh_bus_item));
				}
			}
		}
	}

	optional<TransportRouter::RouteInfo> TransportRouter::FindRoute(const Catalogue::Stop* stop_from, const Catalogue::Stop* stop_to) const {

		RouteInfo route_info;

		if (!stop_to_id_.count(stop_from) || !stop_to_id_.count(stop_to)) {
			return nullopt;
		}

		 StopAsEdge departure = stop_to_id_.at(stop_from);
		 StopAsEdge arrival = stop_to_id_.at(stop_to);

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
		 if (distances_ -> count({ stop1, stop2 })) {
			 return distances_->at({ stop1, stop2 });
		 }
		 else if (distances_->count({ stop2, stop1 })) {
			 return distances_->at({ stop2, stop1 });
		 }
		 else {
			 throw runtime_error("No distance for a pair of stops"s);
		 }
	 }

	 const RouterSettings& TransportRouter::GetRouterSettings() const{
		 return settings_;
	 }

	 const graph::DirectedWeightedGraph<TransportRouter::Time>& TransportRouter::GetGraph() const {
		 return *graph_.get();
	 }

	 const std::unordered_map<const Catalogue::Stop*, TransportRouter::StopAsEdge, TransportRouter::PointerHasher>& TransportRouter::GetStopToIdMap() const {
		 return stop_to_id_;
	 }

	 const std::vector<TransportRouter::RouteItemPtr>& TransportRouter::GetRouteItems() const {
		 return route_items_;
	 }

	 RouterSettings& TransportRouter::AccessRouterSettings() {
		 return settings_;
	 }

	 std::unique_ptr<graph::DirectedWeightedGraph<TransportRouter::Time>>& TransportRouter::AccessGraph() {
		 return graph_;
	 }

	 void TransportRouter::UpdateRouter() {
		 router_ = std::make_unique<graph::Router<Time>>(*graph_);
	 }

	 std::unordered_map<const Catalogue::Stop*, TransportRouter::StopAsEdge, TransportRouter::PointerHasher>& TransportRouter::AccessStopToIdMap() {
		 return stop_to_id_;
	 }

	 std::vector<TransportRouter::RouteItemPtr>& TransportRouter::AccessRouteItems() {
		 return route_items_;
	 }

	 void TransportRouter::LoadCatalogueData(const Catalogue::TransportCatalogue& catalogue) {
		 buses_ = catalogue.GetAllBuses();
		 for (const auto bus : buses_) {
			 stops_.insert(bus->stops.begin(), bus->stops.end());
		 }
		 distances_ = &catalogue.GetDistances();
	 }
}