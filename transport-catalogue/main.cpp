#include "input_reader.h"
#include <iostream>
#include <iomanip>

using namespace std;

int main() {
	InputReader::Query q;
	Catalogue::TransportCatalogue t;
	q.ReadCIn(cin);
	q.Compute(t);
	return 0;
}