#pragma once
#include <string>
#include <deque>
#include <unordered_map>
#include <string_view>
#include <map>
#include <vector>
#include "geo.h"

namespace Catalogue{

	struct Stop {
		std::string name;
		Geo::Coordinates c{ 0,0 };
	};

	struct Bus {
		std::string name;
		std::vector<Stop*> stops;
	};


	class PointerPairHasher {
	public:
		std::size_t operator()(const std::pair<const Stop*, const Stop*>& input) const {
			return 39 * 39 * std::hash<const void*>()(input.first) + std::hash<const void*>()(input.second);
		}
	};

	class TransportCatalogue {

	public:
		void AddStop(std::vector<std::string> stop);
		Stop& FindStop(const std::string& name);
		void AddBus(std::vector<std::string> stops); // 0 - name
		Bus* FindBus(const std::string& name);
		std::map<std::string, std::string> GetBusInfo(std::string name);
		std::vector<std::string> GetBusesForStop(std::string name);
	private:
		std::deque<Stop> stops_ = { Stop() }; //0 - null stop
		std::deque<Bus> buses_ = { Bus() }; //0 - null bus
		std::unordered_map<std::string_view, Stop*> stopname_to_stop_;
		std::unordered_map<std::string_view, Bus*> busname_to_bus_;
		std::unordered_map<std::pair<const Stop*, const Stop*>, std::size_t, PointerPairHasher> distances_;
	};
}