#pragma once
#include "domain.h"
#include "json.h"
#include "json_builder.h"
#include "request_handler.h"
#include "map_renderer.h"
#include "transport_router.h"

#include <iostream>
#include <memory>
#include <tuple>

namespace DataReader {

	class JsonReader : public Reader {

	public:

		struct RouteRequest {
			std::string from = "";
			std::string to = "";
			int id = 0;
		};

		//����������� ������ ���� ����� ������� �������, ���� ����� ��������������� �������� ������ ������ ������
		JsonReader(std::istream& input, Catalogue::TransportCatalogue& catalogue);
		JsonReader(std::istream& input, std::unique_ptr<DataBaseInterface::RequestsHandler>&& handler);

		void ReadBaseRequests() override;
		void ReadStatRequests(std::ostream& output) override;
		Visual::MapSettings ReadMapSettings() override;
		Router::RouterSettings ReadRouterSettings() override;

		void ReadBusesBaseRequests();
		void ReadStopsBaseRequests();

		JsonReader& SetNewRequestHandler(Catalogue::TransportCatalogue& catalogue);
		JsonReader& SetNewRequestHandler(std::unique_ptr<DataBaseInterface::RequestsHandler> new_handler);

		JsonReader& SetNewMap(); //��������� ��������� �� ���������
		JsonReader& SetNewMap(Visual::MapSettings settings); //������ ��������� ��������
		JsonReader& SetNewMap(std::unique_ptr<Visual::MapRenderer> new_renderer); //����������� ������� ������

		JsonReader& SetNewTransportRouter(); //��������� ������ �� ���������, ���������� �� ��������� ����� �� ��������
		JsonReader& SetNewTransportRouter(Router::RouterSettings settings); //���������� �� ��������� ����� �� ��������
		JsonReader& SetNewTransportRouter(std::unique_ptr<Router::TransportRouter> new_router); //������������� ������� �������� ������� ��������

	private:

		std::tuple<std::string, std::vector<std::string>, std::string> ReadBusBaseRequest(const json::Dict base_request); //��� ���������� �������� ����� ��� � ������ ���������
		std::pair<Catalogue::Stop, Catalogue::BusToDistance> ReadStopBaseRequest(const json::Dict base_request); //��� ���������� ��������� ����� ��������� ��������� � ������� ���������� (����� ���� ������)

		std::pair<int, std::string> ReadStatRequest(const json::Dict stat_request); //���� ������ ��� ���� + ��� (����� map)
		RouteRequest ReadRouteRequest(const json::Dict stat_request);
		std::optional<Router::TransportRouter::RouteInfo> DoRouteRequest(RouteRequest request_parsed);
		std::string DoMapRequest();

		json::Document MakeBusInfoJSON(int id, Catalogue::BusInfo& bus_info);
		json::Document MakeStopInfoJSON(int id, Catalogue::StopInfo& stop_info);
		json::Document MakeMapJSON(int id, std::string map_string);
		json::Document MakeRouteJSON(int id, std::optional<Router::TransportRouter::RouteInfo> route);

		json::Document input_document_{ json::Node{} };

		std::unique_ptr<DataBaseInterface::RequestsHandler> handler_;
		using Reader::modules_;
	};

	svg::Color GetColorFromNode(json::Node node);

}

