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

	std::vector<Catalogue::Bus*> RequestsHandler::GetAllBuses() {
		return std::move(catalogue_.GetAllBuses());
	}

	const std::unordered_map<std::pair<const Catalogue::Stop*, const Catalogue::Stop*>, std::size_t, Catalogue::PointerPairHasher>& RequestsHandler::GetDistances() const {
		return catalogue_.GetDistances();
	}

}
