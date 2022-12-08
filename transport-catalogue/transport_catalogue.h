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
		Geo::Coordinates coords{ 0,0 };
	};

	struct Bus {
		std::string name;
		std::vector<Stop*> stops;
	};

	struct BusInfo {
		std::string name ="";
		bool is_not_found = false;
		std::size_t stops_number=0;
		std::size_t unique_stops_number=0;
		double length=0.0;
		double curvature = 0.0;
	};

	struct StopInfo {
		std::vector<std::string> bus_list;
		bool is_not_found = false;
	};

	class PointerPairHasher {
	public:
		std::size_t operator()(const std::pair<const Stop*, const Stop*>& input) const {
			return 39 * 39 * std::hash<const void*>()(input.first) + std::hash<const void*>()(input.second);
		}
	};

	class TransportCatalogue {

		using BusToDistance = std::unordered_map<std::string, double>;

	public:
		void AddStop(std::string stop_name, Geo::Coordinates coords);//��������� ���������: ��� ��������� � ����������

		void AddDistances(std::string stop_name, BusToDistance distances);//��������� ��������� ��� ��������� � ��������� � ������������
		
		Stop& FindStop(const std::string& name);

		void AddBus(std::string bus_name, std::vector<std::string> stops_of_bus);//��������� ��������� ��� �������� � ������ � �����������

		Bus* FindBus(const std::string& name);

		BusInfo GetBusInfo(std::string name); //���������� ��������� � ����������� �� ��������

		StopInfo GetBusesForStop(std::string name);//���������� ��������� � ����������� �� ���������

	private:
		std::deque<Stop> stops_ = { Stop() }; //0 - null stop
		std::deque<Bus> buses_ = { Bus() }; //0 - null bus
		std::unordered_map<std::string_view, Stop*> stopname_to_stop_;
		std::unordered_map<std::string_view, Bus*> busname_to_bus_;
		std::unordered_map<std::pair<const Stop*, const Stop*>, std::size_t, PointerPairHasher> distances_;
	};
}