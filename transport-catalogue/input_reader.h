#pragma once

#include <map>
#include <string>
#include <vector>
#include <iostream>
#include <string_view>
#include "transport_catalogue.h"
#include "stat_reader.h"

namespace InputReader {

	enum class QueryType
	{
		AddStop,
		AddBus,
		GetBusInfo,
		GetBusesForStop,
		GetStop
	};

	class Query {
		using QueryVector = std::vector<std::string_view>;
	public:
		void ReadCIn(std::istream& input); //считывает cin и заолняет очередь
		void Compute(Catalogue::TransportCatalogue& t); //обрабатывает очередь
		void ClearQuery();
		std::vector<std::string> AddStop(QueryVector command);
		std::vector<std::string> AddBus(QueryVector command);
		std::string GetBusInfo(QueryVector command);
		std::string GetBusesForStop(QueryVector command);
	private:
		std::deque<std::string> query_data_; //буфер для всей очереди запросов
		std::map<QueryType, std::vector<QueryVector>> query_; //контейнер для упорядочения: сначала остановки, потом автобусы
		std::vector< std::pair<QueryType, QueryVector>> query2_; //контейнер для запросов
	};

	namespace detail {
		int ReadNumber(std::istream& input);
		std::vector<std::string_view> SplitIntoWords(std::string_view str);
		std::vector<std::string_view> SplitIntoWordsByONESPACE(std::string_view str);
	}

}