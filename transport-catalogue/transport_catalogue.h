#pragma once
#include <string>
#include <deque>
#include <unordered_map>
#include <string_view>
#include <map>
#include <vector>
#include "domain.h"

namespace Catalogue{

	using BusToDistance = std::unordered_map<std::string, double>;

	class PointerPairHasher {
	public:
		std::size_t operator()(const std::pair<const Stop*, const Stop*>& input) const {
			return 39 * 39 * std::hash<const void*>()(input.first) + std::hash<const void*>()(input.second);
		}
	};

	class TransportCatalogue {

	public:

		void AddStop(std::string stop_name, Geo::Coordinates coords);//��������� ���������: ��� ��������� � ����������

		void AddDistances(std::string stop_name, BusToDistance distances);//��������� ��������� ��� ��������� � ��������� � ������������
		
		Stop& FindStop(const std::string& name);

		void AddBus(std::string bus_name, std::vector<std::string> stops_of_bus, std::string end_stop);//��������� ��������� ��� �������� � ������ � ����������� � �������� ���������

		Bus* FindBus(const std::string& name);

		BusInfo GetBusInfo(std::string name); //���������� ��������� � ����������� �� ��������

		StopInfo GetBusesForStop(std::string name);//���������� ��������� � ����������� �� ���������

		std::vector<const Bus*> GetAllBuses() const;
		std::vector<const Stop*> GetAllStops() const;
		const std::unordered_map<std::pair<const Stop*, const Stop*>, std::size_t, PointerPairHasher>& GetDistances() const;

		const Stop* GetStop(std::string stop) const{
			return stopname_to_stop_.at(stop);
		}

		const Bus* GetBus(std::string bus) const {
			return busname_to_bus_.at(bus);
		}

		void LoadDistances(std::unordered_map<std::pair<const Stop*, const Stop*>, std::size_t, PointerPairHasher>& distances);

	private:
		std::deque<Stop> stops_ = { Stop() }; //0 - null stop
		std::deque<Bus> buses_ = { Bus() }; //0 - null bus
		std::unordered_map<std::string_view, Stop*> stopname_to_stop_;
		std::unordered_map<std::string_view, Bus*> busname_to_bus_;
		std::unordered_map<std::pair<const Stop*, const Stop*>, std::size_t, PointerPairHasher> distances_;
	};
}