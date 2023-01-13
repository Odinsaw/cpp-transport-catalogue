#pragma once
#include "transport_catalogue.h"
#include "json.h"
#include "transport_router.h"
#include <iostream>
#include <cassert>
#include "map_renderer.h"
#include <memory>

namespace DataBaseInterface{

    using namespace  std::literals;

    class RequestsHandler : public Modules::Module {

    public:

        struct RouteInfo {
            std::optional<Router::TransportRouter::RouteInfo> route;
            std::unique_ptr<Router::TransportRouter> router;
        };

        RequestsHandler(Catalogue::TransportCatalogue& catalogue);

        void DoBusBaseRequest(std::string name, std::vector<std::string> stop_list, std::string end_stop); //добавить маршрут
        void DoStopBaseRequest(Catalogue::Stop stop, Catalogue::BusToDistance distances_map); //добавить остановку

        Catalogue::StopInfo DoStopStatRequest(std::string stop_name); //получить информацию по остановке
        Catalogue::BusInfo DoBusStatRequest(std::string bus_name); //получить информацию по маршруту

        std::vector<Catalogue::Bus*> GetAllBuses();
        const std::unordered_map<std::pair<const Catalogue::Stop*, const Catalogue::Stop*>, std::size_t, Catalogue::PointerPairHasher>& GetDistances() const;

        Catalogue::Stop* GetStop(std::string stop) {
            return catalogue_.GetStop(stop);
        }

        RouteInfo FindRoute(Router::RouterSettings settings, std::string stop_from, std::string stop_to);
        std::unique_ptr<Router::TransportRouter> MakeRouter(Router::RouterSettings settings);

    private:
    Catalogue::TransportCatalogue& catalogue_; //каталог, с которым будем работать
    };
    
}