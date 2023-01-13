#include "request_handler.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "domain.h"
#include "transport_router.h"

#include <fstream>
#include <memory>

#include<cassert>
int main() {

	Catalogue::TransportCatalogue new_catalogue; //������� �������
	DataReader::JsonReader new_json_reader(std::cin, new_catalogue); //������� ����� json
	new_json_reader.ReadBaseRequests();
	new_json_reader.ReadStatRequests(std::cout);

return 0;
}