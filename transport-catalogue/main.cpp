#include "input_reader.h"
#include "stat_reader.h"
#include <iostream>
#include <iomanip>

using namespace std;

int main() {

	InputReader::DataBaseUpdater update_query;

	StatReader::TransportInfoReader request_query;

	Catalogue::TransportCatalogue catalogue;

	update_query.ReadUpdates(cin, catalogue);

	request_query.ReadRequests(cin,catalogue);

	return 0;
}