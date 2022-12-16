#include "request_handler.h"
#include "json_reader.h"
#include "map_renderer.h"
#include <memory>
#include <fstream>
#include "domain.h"

int main() {

Catalogue::TransportCatalogue new_catalogue; //создаем каталог
std::unique_ptr<DataBaseInterface::RequestsHandler> new_handler = std::make_unique<DataBaseInterface::RequestsHandler>(new_catalogue); //создаем обработчик запросов и привязываем к каталогу
DataReader::JsonReader new_json_reader(std::cin, std::move(new_handler)); //создаем обработчик json и даем ссылку на обработчик запросов
new_json_reader.ReadBaseRequests();
new_json_reader.ReadStatRequests(std::cout);

return 0;
}