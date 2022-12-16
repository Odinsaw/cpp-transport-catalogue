#pragma once
#include "transport_catalogue.h"
#include "json.h"
#include <iostream>
#include <cassert>
#include "map_renderer.h"
#include <memory>

namespace DataBaseInterface{

    using namespace  std::literals;

    class RequestsHandler {

    public:

        RequestsHandler(Catalogue::TransportCatalogue& catalogue);

        void DoBusBaseRequest(std::string name, std::vector<std::string> stop_list, std::string end_stop); //добавить маршрут
        void DoStopBaseRequest(Catalogue::Stop stop, Catalogue::BusToDistance distances_map); //добавить остановку

        Catalogue::StopInfo DoStopStatRequest(std::string stop_name); //получить информацию по остановке
        Catalogue::BusInfo DoBusStatRequest(std::string bus_name); //получить информацию по маршруту

        std::vector<Catalogue::Bus*> GetAllBuses();

    private:
    Catalogue::TransportCatalogue& catalogue_; //каталог, с которым будем работать
    };
    
}