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

        void DoBusBaseRequest(std::string name, std::vector<std::string> stop_list, std::string end_stop); //�������� �������
        void DoStopBaseRequest(Catalogue::Stop stop, Catalogue::BusToDistance distances_map); //�������� ���������

        Catalogue::StopInfo DoStopStatRequest(std::string stop_name); //�������� ���������� �� ���������
        Catalogue::BusInfo DoBusStatRequest(std::string bus_name); //�������� ���������� �� ��������

        std::vector<Catalogue::Bus*> GetAllBuses();

    private:
    Catalogue::TransportCatalogue& catalogue_; //�������, � ������� ����� ��������
    };
    
}