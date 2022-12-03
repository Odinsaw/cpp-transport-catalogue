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
	void TransportCatalogue::AddStop(vector<string> stop) {
		if (&FindStop(stop[0]) == &stops_.front()) { //при повторном проходе пропускаем
			stops_.push_back({ stop[0], stod(stop[1]), stod(stop[2]) });
			stopname_to_stop_.insert({ string_view(stops_.back().name),&stops_.back() });
		}

		if (stop.size() == 3) {
			return;
		}

		const Stop* this_stop = &FindStop(stop[0]);

		for (auto i = 3; i < stop.size(); i = i + 2) {
			const Stop* cur_stop = &FindStop(stop[i + 1]);
			if (cur_stop != &stops_.front()) { //при повторном проходе пропускаем
				auto p = pair{ this_stop, cur_stop };

				stringstream sstream(stop[i]);
				size_t num;
				sstream >> num;
				distances_.insert({ p, num });
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

	void TransportCatalogue::AddBus(vector<string> stops) {
		Bus new_bus{ move(stops[0]),{} };
		stops.erase(stops.begin());
		new_bus.stops.reserve(stops.size());
		for (string& stop : stops) {
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

	//возвращает информацию в виде словаря. Вместо словаря можно более подходящий контейнер
	map<string, string> TransportCatalogue::GetBusInfo(string name) {
		map<string, string> info;

		Bus* c = FindBus(name);

		if (c == nullptr) {
			info.insert({ "name"s, name });
			return info;
		}

		Bus& b = *c;

		info["name"s] = name;
		info["stops_num"s] = to_string(b.stops.size());
		set<Stop*> s(b.stops.begin(), b.stops.end());
		info["unique_num"s] = to_string(s.size());

		double geo_l = transform_reduce(b.stops.begin() + 1, b.stops.end(), b.stops.begin(), 0.0, plus<>(), [&](Stop* s1, Stop* s2) {
			return Geo::ComputeDistance(s1->c, s2->c); });

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
		out_s << setprecision(6) << l;
		string len = out_s.str();
		info["length"s] = len;

		out_s.str(std::string());
		out_s << setprecision(6) << (l / geo_l);
		string curv = out_s.str();
		info["curvature"s] = curv;

		return info;
	}

	//возвращает вектор {имя автобуса, остановка1, остановка2...}
	vector<string> TransportCatalogue::GetBusesForStop(string name) {
		vector<string> out{ name };
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

