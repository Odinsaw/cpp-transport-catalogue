#pragma once

#include <map>
#include <string>
#include <vector>
#include <iostream>
#include <string_view>
#include "transport_catalogue.h"
#include "stat_reader.h"

namespace InputReader {

	using BusToDistance = std::unordered_map<std::string, double>;

	class DataBaseUpdater {

		using UpdateVector = std::vector<std::string>;

	public:

		void ReadCommands(std::istream& input, Catalogue::TransportCatalogue& catalogue); //��������� cin � ��������� ��������� � ��������

		void ClearQuery();

	private:

		std::pair<Catalogue::Stop, BusToDistance> StopUpdate(UpdateVector command); 

		std::pair<std::string, std::vector<std::string>> BusUpdate(UpdateVector command);

		std::deque<UpdateVector> stop_updates_; //����� ��� ������� ���������� ��������� ��� ������� ��������

		std::deque<UpdateVector> bus_updates_; //����� ��� ������� ���������� ���������

	};

	namespace detail {

		int ReadNumber(std::istream& input);

		std::vector<std::string_view> SplitIntoWords(std::string_view str);

		std::vector<std::string_view> SplitIntoWordsByONESPACE(std::string_view str);

	}

}