#pragma once

#include <string_view>
#include <vector>
#include "svg.h"
#include "transport_catalogue.h"

namespace TC {

    struct map_settings_t {
        double width;
        double height;
        double padding;

        double line_width;
        double stop_radius;

        int bus_label_font_size;
        svg::Point bus_label_offset;

        int stop_label_font_size;
        svg::Point stop_label_offset;

        svg::Color underlayer_color;
        double underlayer_width;

        std::vector<svg::Color> color_palette;
    };

    struct routing_settings_t{
        int bus_velocity_kmh;   // km/h
        int bus_wait_time_min;  // minutes
    };

    template <typename Array, typename Dict, typename Node>
    class Reader {
        public:

        virtual const Array& GetRequestNodesAsArray(std::string_view name) = 0;
        virtual const Dict& GetRequestNodesAsMap(std::string_view name) = 0;

        virtual std::string_view GetFieldAsString(const Node& node, std::string_view name) = 0;
        virtual bool GetFieldAsBool(const Node& node, std::string_view name) = 0;
        virtual double GetFieldAsDouble(const Node& node, std::string_view name) = 0;
        virtual int GetFieldAsInt(const Node& node, std::string_view name) = 0;

        virtual const Node& GetFieldAsNode(const Node& node, std::string_view name) = 0;

        virtual const Array& GetFieldAsArrayNodes(const Node& node, std::string_view name) = 0;
        virtual const Dict& GetFieldAsMapNodes(const Node& node, std::string_view name) = 0;

        virtual ~Reader() {};
    };

    class Renderer {
        public:

        virtual void Render(std::vector<const Bus*>  buses, 
                    std::vector<const Stop*>  stops, 
                    std::ostream& output) = 0;

        virtual void SetSettings(map_settings_t& settings) = 0;

        virtual ~Renderer() {};
    };
    
}