#pragma once

#include "domain.h"
#include "svg.h"

#include <cassert>
#include <map>
#include <algorithm>
#include <sstream>

namespace Visual {

    class Map : public svg::Drawable {
    public:
        Map(MapSettings settings, std::vector<const Catalogue::Bus*> buses);

        void Draw(svg::ObjectContainer& container) const override;

    private:
        void DrawBusLines(svg::ObjectContainer&) const;
        void DrawBusLabels(svg::ObjectContainer&) const;
        void DrawStops(svg::ObjectContainer&) const;
        void DrawStopLabels(svg::ObjectContainer&) const;

        const svg::Color& GetBusLineColor(size_t index) const;

        struct LexicSorterByName {
            bool operator()(const Catalogue::Stop* lhs, const Catalogue::Stop* rhs) const;
        };

        MapSettings settings_;
        std::vector<const Catalogue::Bus*> buses_;
        std::map<const Catalogue::Stop*, svg::Point, LexicSorterByName> stops_positions_;
    };

    class MapRenderer : public Modules::Module {
    public:
        MapRenderer(MapSettings settings, std::vector<const Catalogue::Bus*> buses);

        std::string GetMap();

    private:
        std::string map_;
    };
} 