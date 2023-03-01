#include "request_handler.h"
#include <sstream>
#include <iomanip>
#include <utility>
#include <algorithm>

namespace DataBaseInterface {

	RequestsHandler::RequestsHandler(Catalogue::TransportCatalogue& catalogue)
		:Modules::Module(Modules::ModuleType::RequestHandler), catalogue_(catalogue)
	{
	}

	void RequestsHandler::DoBusBaseRequest(std::string name, std::vector<std::string> stop_list, std::string end_stop) {

		catalogue_.AddBus(std::move(name), std::move(stop_list), std::move(end_stop));
	}


	void RequestsHandler::DoStopBaseRequest(Catalogue::Stop stop, Catalogue::BusToDistance distances_map) {

		catalogue_.AddStop(stop.name, stop.coords);

		if (distances_map.empty()) {
			return;
		}

		catalogue_.AddDistances(std::move(stop.name), std::move(distances_map));
	}

	Catalogue::StopInfo RequestsHandler::DoStopStatRequest(std::string stop_name) {

		return catalogue_.GetBusesForStop(stop_name);
	}

	Catalogue::BusInfo RequestsHandler::DoBusStatRequest(std::string bus_name) {

		return catalogue_.GetBusInfo(bus_name);
	}

	std::vector<const Catalogue::Bus*> RequestsHandler::GetAllBuses() {
		return std::move(catalogue_.GetAllBuses());
	}

	std::vector<const Catalogue::Stop*> RequestsHandler::GetAllStops() {
		return std::move(catalogue_.GetAllStops());
	}

	const std::unordered_map<std::pair<const Catalogue::Stop*, const Catalogue::Stop*>, std::size_t, Catalogue::PointerPairHasher>& RequestsHandler::GetDistances() const {
		return catalogue_.GetDistances();
	}

	RequestsHandler::RouteInfo RequestsHandler::FindRoute(Router::RouterSettings settings, 
		std::string stop_from, std::string stop_to) {
			RouteInfo route_info;
			route_info.router = MakeRouter(settings);
			route_info.route = route_info.router->FindRoute(GetStop(stop_from), GetStop(stop_to));
			return route_info;
	}

	std::unique_ptr<Router::TransportRouter> RequestsHandler::MakeRouter(Router::RouterSettings settings) {
		return std::make_unique<Router::TransportRouter>(settings, catalogue_);
	}

	void RequestsHandler::DoSerialization(std::filesystem::path file, const Visual::MapSettings& map_settings, const Router::TransportRouter& router) {
		Serialization::DoSerialization(file, catalogue_, map_settings, router);
	}

	void RequestsHandler::DoDeSerialization(std::filesystem::path file, Visual::MapSettings& map_settings, Router::TransportRouter& router) {
		Serialization::DoDeSerialization(file, catalogue_, map_settings, router);
	}
}