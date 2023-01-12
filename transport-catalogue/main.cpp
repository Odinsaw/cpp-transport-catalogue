#include "request_handler.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "domain.h"
#include "transport_router.h"

#include <fstream>
#include <memory>

#include<cassert>
int main() {

	std::ifstream input("C:\\Users\\112\\source\\repos\\������������ ���������� - ��������� JSON\\Debug\\������2.json"s, std::ios::in);
	assert(input);

//Catalogue::TransportCatalogue new_catalogue; //������� �������
//std::unique_ptr<DataBaseInterface::RequestsHandler> new_handler = std::make_unique<DataBaseInterface::RequestsHandler>(new_catalogue); //������� ���������� �������� � ����������� � ��������
//DataReader::JsonReader new_json_reader(input, std::move(new_handler)); //������� ���������� json � ���� ������ �� ���������� ��������
//new_json_reader.ReadBaseRequests();
//
//std::unique_ptr<DataBaseInterface::RequestsHandler> new_handler2 = std::make_unique<DataBaseInterface::RequestsHandler>(new_catalogue);
//std::unique_ptr<Router::TransportRouter> new_router = std::make_unique<Router::TransportRouter>(new_json_reader.ReadRouterSettings(), 
//	new_handler2->GetAllBuses(), new_handler2->GetDistances());
//new_json_reader.ReadStatRequests(std::cout);

	Catalogue::TransportCatalogue new_catalogue; //������� �������
	DataReader::JsonReader new_json_reader(input, new_catalogue);
	new_json_reader.ReadBaseRequests();
	new_json_reader.ReadStatRequests(std::cout);

return 0;
}