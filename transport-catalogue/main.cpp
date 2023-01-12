#include "request_handler.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "domain.h"
#include "transport_router.h"

#include <fstream>
#include <memory>

#include<cassert>
int main() {

	std::ifstream input("C:\\Users\\112\\source\\repos\\Транспортный справочник - поддержка JSON\\Debug\\Пример2.json"s, std::ios::in);
	assert(input);

//Catalogue::TransportCatalogue new_catalogue; //создаем каталог
//std::unique_ptr<DataBaseInterface::RequestsHandler> new_handler = std::make_unique<DataBaseInterface::RequestsHandler>(new_catalogue); //создаем обработчик запросов и привязываем к каталогу
//DataReader::JsonReader new_json_reader(input, std::move(new_handler)); //создаем обработчик json и даем ссылку на обработчик запросов
//new_json_reader.ReadBaseRequests();
//
//std::unique_ptr<DataBaseInterface::RequestsHandler> new_handler2 = std::make_unique<DataBaseInterface::RequestsHandler>(new_catalogue);
//std::unique_ptr<Router::TransportRouter> new_router = std::make_unique<Router::TransportRouter>(new_json_reader.ReadRouterSettings(), 
//	new_handler2->GetAllBuses(), new_handler2->GetDistances());
//new_json_reader.ReadStatRequests(std::cout);

	Catalogue::TransportCatalogue new_catalogue; //создаем каталог
	DataReader::JsonReader new_json_reader(input, new_catalogue);
	new_json_reader.ReadBaseRequests();
	new_json_reader.ReadStatRequests(std::cout);

return 0;
}