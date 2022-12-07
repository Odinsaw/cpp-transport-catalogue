#include "transport_catalogue.h"
#include <utility>
#include <algorithm>
#include <numeric>
#include <set>
#include <sstream>
#include <iomanip>


using namespace std;

namespace Catalogue {

	//добавить остановку
	void TransportCatalogue::AddStop(Stop new_stop, BusToDistance distances) {
		if (&FindStop(new_stop.name) == &stops_.front()) { //при повторном проходе пропускаем
			stops_.push_back(move(new_stop));
			stopname_to_stop_.insert({ string_view(stops_.back().name),&stops_.back() });
		}

		if (distances.size() == 0) {
			return;
		}

		const Stop* this_stop = &FindStop(new_stop.name);

		for (auto [stop_name, distance] : distances) {
			const Stop* cur_stop = &FindStop(stop_name);
			if (cur_stop != &stops_.front()) { //при повторном проходе пропускаем
				auto p = pair{ this_stop, cur_stop };
				distances_.insert({ p, distance });
			}
		}

	}
	Stop& TransportCatalogue::FindStop(const string& name) {
		auto stop = stopname_to_stop_.find(string_view(name));
		if (stop == stopname_to_stop_.end()) {
			return stops_.front();
		}
		else {
			return *(stop->second);
		}
	}

	void TransportCatalogue::AddBus(string new_bus_name, vector<string> stops_of_bus) {
		Bus new_bus{ move(new_bus_name),{} };
		new_bus.stops.reserve(stops_of_bus.size());
		for (string& stop : stops_of_bus) {
			new_bus.stops.push_back(&FindStop(stop));
		}
		buses_.push_back(new_bus);
		busname_to_bus_.insert({ string_view(buses_.back().name),&buses_.back() });
	}
	Bus* TransportCatalogue::FindBus(const string& name) {
		auto bus = busname_to_bus_.find(string_view(name));
		if (bus == busname_to_bus_.end()) {
			return nullptr;
		}
		else {
			return bus->second;
		}
	}

	std::vector<std::string> TransportCatalogue::GetBusInfo(string name) {

		vector<string> out;

		out.push_back(name);

		Bus* c = FindBus(name);

		if (c == nullptr) {
			out.push_back("not found"s);
				return out;
		}

		Bus& b = *c;

		out.push_back(to_string(b.stops.size()));
		set<Stop*> s(b.stops.begin(), b.stops.end());
		out.push_back(to_string(s.size()));

		double geo_l = transform_reduce(b.stops.begin() + 1, b.stops.end(), b.stops.begin(), 0.0, plus<>(), [&](Stop* s1, Stop* s2) {
			return Geo::ComputeDistance(s1->coords, s2->coords); });

		double l = transform_reduce(b.stops.begin() + 1, b.stops.end(), b.stops.begin(), 0.0, plus<>(),

			[&](Stop* s2, Stop* s1) {
				if (distances_.count(pair(s1, s2))) {
					return static_cast<double>(distances_.at(pair(s1, s2)));
				}
				else if (distances_.count(pair(s2, s1))) {
					return static_cast<double>(distances_.at(pair(s2, s1)));
				}
				else {
					return 0.0;
				}
			}

		);

		ostringstream out_s;
		out_s << l;
		out.push_back(out_s.str());

		out_s.str(std::string());
		out_s << (l / geo_l);
		out.push_back(out_s.str());

		return out;
	}

	//возвращает вектор {имя автобуса, остановка1, остановка2...}
	vector<string> TransportCatalogue::GetBusesForStop(string name) {
		vector<string> out;
		if (!stopname_to_stop_.count(name)) {
			out.push_back(""s);
			return out;
		}
		Stop* s = stopname_to_stop_.at(name);
		vector<string> temp;
		for (const Bus& b : buses_) {
			if (find(b.stops.begin(), b.stops.end(), s) != b.stops.end()) {
				temp.push_back(b.name);
			}
		}
		sort(temp.begin(), temp.end());
		out.insert(out.end(), temp.begin(), temp.end());

		return out;
	}
}

