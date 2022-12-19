#pragma once
#include "domain.h"
#include "json.h"
#include "request_handler.h"
#include "map_renderer.h"
#include <iostream>
#include <memory>
#include <tuple>

namespace DataReader {

	class JsonReader : public Reader {

	public:

		JsonReader(std::istream& input, std::unique_ptr<DataBaseInterface::RequestsHandler>&& handler);

		void ReadBaseRequests() override;
		void ReadStatRequests(std::ostream& output) override;
		Visual::MapSettings ReadMapSettings() override;

		void ReadBusesBaseRequests();
		void ReadStopsBaseRequests();

	private:

		std::tuple<std::string, std::vector<std::string>, std::string> ReadBusBaseRequest(const json::Dict base_request); //��� ���������� �������� ����� ��� � ������ ���������
		std::pair<Catalogue::Stop, Catalogue::BusToDistance> ReadStopBaseRequest(const json::Dict base_request); //��� ���������� ��������� ����� ��������� ��������� � ������� ���������� (����� ���� ������)

		std::pair<int, std::string> ReadStatRequest(const json::Dict stat_request); //���� ������ ��� ���� + ��� (����� map)

		json::Document MakeBusInfoJSON(int id, Catalogue::BusInfo& bus_info);
		json::Document MakeStopInfoJSON(int id, Catalogue::StopInfo& stop_info);
		json::Document MakeMapJSON(int id);

		json::Document input_document_{ json::Node{} };

		std::unique_ptr<DataBaseInterface::RequestsHandler> handler_;
	};

	svg::Color GetColorFromNode(json::Node node);

}

