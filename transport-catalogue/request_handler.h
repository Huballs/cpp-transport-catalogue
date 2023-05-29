#pragma once

#include "transport_catalogue.h"
#include <optional>
#include <unordered_set>
#include <map>
#include <memory>
#include <sstream>
#include "domain.h"
#include "map_renderer.h"

namespace TC {

class RequestHandler {
    
    public:

        struct base_request_t {

            std::string_view type;
            std::string_view name;

            std::vector<std::string_view> stops;
            bool is_roundtrip = false;

            double latitude;
            double longitude;

            std::unordered_map<std::string_view, uint32_t> road_distances;

        };

        struct stat_bus_t{

            double curvature;
            uint32_t length;
            int stop_count;
            int unique_stop_count;

        };

        struct stat_stop_t{

            const std::set<std::string_view>& buses;
            
        };
        
        
        RequestHandler(TransportCatalogue& db, Renderer& renderer) : db_(db), renderer_(renderer){};

        template <typename Array, typename Dict, typename Node, typename OutputBuilder>
        void ReadRequests(std::ostream& output, Reader<Array, Dict, Node>& reader, OutputBuilder);

        template <typename Array, typename Dict, typename Node, typename OutputBuilder>
        void ReadStatRequests(std::ostream& output, Reader<Array, Dict, Node>& reader, OutputBuilder);

        template <typename Array, typename Dict, typename Node>
        map_settings_t ReadMapRenderSettings(Reader<Array, Dict, Node>& reader);

        /* stores add request for later fulfillment */
        void QueueAddRequest(base_request_t& request);

        /* makes requests to tc in order: add stops, add busses if any*/
        void FulfillAddRequests();

        // Возвращает информацию о маршруте (запрос Bus)
        std::optional<stat_bus_t> GetBusStat(const std::string_view& bus_name) const;
        
        // Возвращает информацию о маршруте (запрос Stop)
        std::optional<stat_stop_t> GetStopStat(const std::string_view& stop_name) const;

        const std::deque<Bus>& GetBuses() const;

        std::vector<const Bus*> GetBusesAscendingName() const;
        std::vector<const Stop*> GetStopsAscendingName() const;

    private:

        std::vector<base_request_t> requests_add_stop;
        std::vector<base_request_t> requests_add_bus;

        TransportCatalogue& db_;
        Renderer& renderer_;

    };

namespace detail {

    template <typename Node>
    svg::Color NodeToSvgColor(const Node& node){

        if(node.IsArray()){
            const auto& color_array = node.AsArray();

            if (color_array.size() == 3){
                svg::Rgb color;
                color.red = color_array[0].AsInt();
                color.green = color_array[1].AsInt();
                color.blue = color_array[2].AsInt();
                return color;
            } else 
            if (color_array.size() == 4){
                svg::Rgba color;
                color.red = color_array[0].AsInt();
                color.green = color_array[1].AsInt();
                color.blue = color_array[2].AsInt();
                color.opacity = color_array[3].AsDouble();
                return color;
            }
        } else 
        if (node.IsString()){
            return node.AsString();
        }
    return svg::Rgb();
    }
} // namespace detail


template <typename Array, typename Dict, typename Node, typename OutputBuilder>
void RequestHandler::ReadRequests(std::ostream& output, Reader<Array, Dict, Node>& reader, OutputBuilder){

    const auto request_nodes = reader.GetRequestNodesAsArray("base_requests");

    for(const auto& request_node : request_nodes){

        base_request_t request;

        if(reader.GetFieldAsString(request_node, "type") == "Bus"){
                
                request.type = "Bus";
                request.name = reader.GetFieldAsString(request_node, "name");
                request.is_roundtrip = reader.GetFieldAsBool(request_node, "is_roundtrip");

                for(const auto& stop_node : reader.GetFieldAsArrayNodes(request_node, "stops")){
                    request.stops.push_back(stop_node.AsString());
                }
        } else 
        if(reader.GetFieldAsString(request_node, "type") == "Stop"){

            request.type = "Stop";
            request.name = reader.GetFieldAsString(request_node, "name");
            request.latitude = reader.GetFieldAsDouble(request_node, "latitude");
            request.longitude = reader.GetFieldAsDouble(request_node, "longitude");

            for(const auto& [name, dist_node]: reader.GetFieldAsMapNodes(request_node, "road_distances")){
                request.road_distances.insert({name, dist_node.AsInt()});
            }
        }

        QueueAddRequest(request);
    }

    FulfillAddRequests();

    ReadStatRequests(output, reader, OutputBuilder{});
}

template <typename Array, typename Dict, typename Node, typename OutputBuilder>
void RequestHandler::ReadStatRequests(std::ostream& output, Reader<Array, Dict, Node>& reader, OutputBuilder){

        using namespace std::string_literals;

        const auto request_nodes = reader.GetRequestNodesAsArray("stat_requests");

        typename OutputBuilder::Array_t array_output;

        for(const auto& request_node : request_nodes){

            typename OutputBuilder::Node_t request_output;

            int request_id = reader.GetFieldAsInt(request_node, "id");

            if(reader.GetFieldAsString(request_node, "type") == "Bus"){

                const auto& name = reader.GetFieldAsString(request_node, "name");

                if (const auto& bus_stat = GetBusStat(name)){

                    request_output = OutputBuilder{}.StartDict()
                        .Key("request_id").Value(request_id)
                        .Key("curvature").Value(bus_stat->curvature)
                        .Key("route_length").Value(bus_stat->length)
                        .Key("stop_count").Value(bus_stat->stop_count)
                        .Key("unique_stop_count").Value(bus_stat->unique_stop_count)
                        .EndDict().Build();
                        
                } else {

                    request_output = OutputBuilder{}.StartDict()
                        .Key("request_id").Value(request_id)
                        .Key("error_message").Value("not found"s)
                        .EndDict().Build();
                }
            } else 
            if (reader.GetFieldAsString(request_node, "type") == "Stop"){

                const auto& name = reader.GetFieldAsString(request_node, "name");

                if (const auto& stop_stat = GetStopStat(name)){
                    
                    typename OutputBuilder::Array_t stops;
                    
                    for(const auto stop : stop_stat->buses){
                        stops.push_back(OutputBuilder{}.Value(std::string(stop)).Build());
                    }

                    request_output = OutputBuilder{}.StartDict()
                        .Key("request_id").Value(request_id)
                        .Key("buses").Value(stops)
                        .EndDict().Build();

                } else {

                    request_output = OutputBuilder{}.StartDict()
                        .Key("request_id").Value(request_id)
                        .Key("error_message").Value("not found"s)
                        .EndDict().Build();
                }
            } else 
            if (reader.GetFieldAsString(request_node, "type") == "Map"){

                auto map_renderer_settings = ReadMapRenderSettings(reader);
                
                renderer_.SetSettings(map_renderer_settings);

                std::stringstream stream;
                renderer_.Render(GetBusesAscendingName(), GetStopsAscendingName(), stream);

                request_output = OutputBuilder{}.StartDict()
                        .Key("request_id").Value(request_id)
                        .Key("map").Value(stream.str())
                        .EndDict().Build();
            }

            array_output.push_back(request_output);

        }

        OutputBuilder{}.Value(array_output).Print(output);
    }

template <typename Array, typename Dict, typename Node>
map_settings_t RequestHandler::ReadMapRenderSettings(Reader<Array, Dict, Node>& reader) {

        const auto settings_map = reader.GetRequestNodesAsMap("render_settings");

        map_settings_t settings;

        settings.width = reader.GetFieldAsDouble(settings_map, "width");
        settings.height = reader.GetFieldAsDouble(settings_map, "height");
        settings.padding = reader.GetFieldAsDouble(settings_map, "padding");
        settings.line_width = reader.GetFieldAsDouble(settings_map, "line_width");
        settings.stop_radius = reader.GetFieldAsDouble(settings_map, "stop_radius");
        settings.bus_label_font_size = reader.GetFieldAsInt(settings_map, "bus_label_font_size");
        settings.stop_label_font_size = reader.GetFieldAsInt(settings_map, "stop_label_font_size");
        settings.underlayer_width = reader.GetFieldAsDouble(settings_map, "underlayer_width");

        settings.bus_label_offset = {reader.GetFieldAsArrayNodes(settings_map, "bus_label_offset")[0].AsDouble(),
                                     reader.GetFieldAsArrayNodes(settings_map, "bus_label_offset")[1].AsDouble()};
        
        settings.stop_label_offset = {reader.GetFieldAsArrayNodes(settings_map, "stop_label_offset")[0].AsDouble(),
                                     reader.GetFieldAsArrayNodes(settings_map, "stop_label_offset")[1].AsDouble()};
       

        settings.underlayer_color = detail::NodeToSvgColor(reader.GetFieldAsNode(settings_map, "underlayer_color"));

        const auto color_palette = reader.GetFieldAsArrayNodes(settings_map,"color_palette");

        for(const auto& color_node : color_palette){
            settings.color_palette.push_back(detail::NodeToSvgColor(color_node));
        }

        return settings;
    }

} // namespace TC