#include <fstream>
#include <iostream>
#include <string_view>

#include "request_handler.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "domain.h"
#include "transport_router.h"

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
	stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
	if (argc != 2) {
		PrintUsage();
		return 1;
	}

	const std::string_view mode(argv[1]);

	if (mode == "make_base"sv) {

		Catalogue::TransportCatalogue new_catalogue; //создаем каталог
		DataReader::JsonReader new_json_reader(std::cin, new_catalogue); //создаем ридер json
		new_json_reader.ReadBaseRequests();
	}
	else if (mode == "process_requests"sv) {

		Catalogue::TransportCatalogue new_catalogue; //создаем каталог
		DataReader::JsonReader new_json_reader(std::cin, new_catalogue); //создаем ридер json
		new_json_reader.ReadStatRequests(std::cout);
	}
	else {
		PrintUsage();
		return 1;
	}
}