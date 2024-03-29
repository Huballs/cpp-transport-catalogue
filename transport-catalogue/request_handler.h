#pragma once

#include <optional>
#include <unordered_set>
#include <map>
#include <optional>
#include <sstream>
#include "domain.h"
#include "map_renderer.h"
#include "transport_catalogue.h"
#include "transport_router.h"

namespace TC {

class RequestHandler {
    
    public:

        struct stat_bus_t{
            double curvature;
            uint32_t length;
            int stop_count;
            int unique_stop_count;
        };

        struct stat_route_t{
            std::string_view to;
            std::string_view from;
        };

        struct stat_stop_t{
            const std::set<std::string_view>& buses;
        };
        
        RequestHandler(TransportCatalogue& db, Renderer *renderer = nullptr) : db_(db), renderer_(renderer){};

        template <typename Array, typename Dict, typename Node>
        void ReadRequests(Reader<Array, Dict, Node>& reader);

        template <typename Array, typename Dict, typename Node, typename OutputBuilder>
        void ReadStatRequests(std::ostream& output, Reader<Array, Dict, Node>& reader, OutputBuilder);

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

        void SerializeToFile(std::string_view file_name);
        void DeserializeFromFile(std::string_view file_name);

        template <typename Array, typename Dict, typename Node>
        inline std::string_view ReadSerializationSettings(Reader<Array, Dict, Node>& reader);

    private:

        std::vector<base_request_t> requests_add_stop;
        std::vector<base_request_t> requests_add_bus;

        TransportCatalogue& db_;
        Renderer *renderer_;
        map_settings_t render_settings_;
        std::unique_ptr<TransportRouter> transport_router_;

        template <typename Array, typename Dict, typename Node>
        routing_settings_t ReadRoutingSettings(Reader<Array, Dict, Node>& reader);

        template <typename Array, typename Dict, typename Node>
        map_settings_t ReadMapRenderSettings(Reader<Array, Dict, Node>& reader);

        template <typename OutputBuilder>
        typename OutputBuilder::Node_t BuildRouteNode(int id, const TransportRouter::Travel& travel);

        template <typename OutputBuilder>
        typename OutputBuilder::Node_t BuildBusStatNode(int id, const stat_bus_t& bus_stat);

        template <typename OutputBuilder>
        typename OutputBuilder::Node_t BuildStopStatNode(int id, const stat_stop_t& stop_stat);

        template <typename OutputBuilder>
        typename OutputBuilder::Node_t BuildMapStatNode(int id, map_settings_t& map_settings);

        template <typename OutputBuilder>
        typename OutputBuilder::Node_t BuildNotFoundNode(int id);

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
            }  
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


template <typename Array, typename Dict, typename Node>
void RequestHandler::ReadRequests(Reader<Array, Dict, Node>& reader){

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

    render_settings_ = ReadMapRenderSettings(reader);

    auto routing_settings = ReadRoutingSettings(reader);
    transport_router_ = std::make_unique<TransportRouter>(db_, routing_settings);

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
                request_output = BuildBusStatNode<OutputBuilder>(request_id, *bus_stat);
            } else {
                request_output = BuildNotFoundNode<OutputBuilder>(request_id);
            }
        } else 
        if (reader.GetFieldAsString(request_node, "type") == "Stop"){

            const auto& name = reader.GetFieldAsString(request_node, "name");

            if (const auto& stop_stat = GetStopStat(name)){
                request_output = BuildStopStatNode<OutputBuilder>(request_id, *stop_stat);
            } else {
                request_output = BuildNotFoundNode<OutputBuilder>(request_id);
            }
        } else 
        if (reader.GetFieldAsString(request_node, "type") == "Map"){
            request_output = BuildMapStatNode<OutputBuilder>(request_id, render_settings_);
        } else 
        if (reader.GetFieldAsString(request_node, "type") == "Route"){

            const auto from = reader.GetFieldAsString(request_node, "from");
            const auto to = reader.GetFieldAsString(request_node, "to");

            const auto route = transport_router_->Route(from, to);
            if(route){
                request_output = BuildRouteNode<OutputBuilder>(request_id, *route);
            } else{
                request_output = BuildNotFoundNode<OutputBuilder>(request_id);
            }
           
        }

        array_output.push_back(request_output);

    }

    OutputBuilder{}.Value(array_output).Print(output);
}

template <typename Array, typename Dict, typename Node>
std::string_view RequestHandler::ReadSerializationSettings(Reader<Array, Dict, Node>& reader){

    const auto& settings_map = reader.GetRequestNodesAsMap("serialization_settings");

    return reader.GetFieldAsString(settings_map, "file");
}

template <typename Array, typename Dict, typename Node>
inline routing_settings_t RequestHandler::ReadRoutingSettings(Reader<Array, Dict, Node>& reader){

    const auto settings_map = reader.GetRequestNodesAsMap("routing_settings");
    routing_settings_t settings;
    settings.bus_velocity_kmh = reader.GetFieldAsInt(settings_map, "bus_velocity");
    settings.bus_wait_time_min = reader.GetFieldAsInt(settings_map, "bus_wait_time");
    return settings;
}

template <typename Array, typename Dict, typename Node>
inline map_settings_t RequestHandler::ReadMapRenderSettings(Reader<Array, Dict, Node>& reader) {

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

template <typename OutputBuilder>
inline typename OutputBuilder::Node_t RequestHandler::BuildRouteNode(int id, const TransportRouter::Travel& travel){

    using namespace std::string_literals;

    typename OutputBuilder::Array_t items;

    for(const auto& t : travel.lines){

        typename OutputBuilder::Dict_t item;

        if(t.type == TransportRouter::RouteLine_t::BUS){
            (item)["bus"] = std::string(t.name);
            (item)["span_count"] = static_cast<uint32_t>(t.span_count);
            (item)["time"] = t.time_min;
            (item)["type"] = "Bus"s;
            
        } else
        if(t.type == TransportRouter::RouteLine_t::WAIT){
            (item)["stop_name"] = std::string(t.name);
            (item)["time"] = t.time_min;
            (item)["type"] = "Wait"s;
        }
        items.push_back(std::move(item));
    }

    return OutputBuilder{}.StartDict()
        .Key("items").Value(items)
        .Key("request_id").Value(id)
        .Key("total_time").Value(travel.total_time_min)
        .EndDict().Build();
}

template <typename OutputBuilder>
inline typename OutputBuilder::Node_t RequestHandler::BuildBusStatNode(int id, const stat_bus_t& bus_stat){
    return OutputBuilder{}.StartDict()
            .Key("request_id").Value(id)
            .Key("curvature").Value(bus_stat.curvature)
            .Key("route_length").Value(bus_stat.length)
            .Key("stop_count").Value(bus_stat.stop_count)
            .Key("unique_stop_count").Value(bus_stat.unique_stop_count)
            .EndDict().Build();
}

template <typename OutputBuilder>
inline typename OutputBuilder::Node_t RequestHandler::BuildStopStatNode(int id, const stat_stop_t& stop_stat){
    typename OutputBuilder::Array_t stops;
                    
    for(const auto stop : stop_stat.buses){
        stops.push_back(OutputBuilder{}.Value(std::string(stop)).Build());
    }

    return OutputBuilder{}.StartDict()
        .Key("request_id").Value(id)
        .Key("buses").Value(stops)
        .EndDict().Build();
}

template <typename OutputBuilder>
inline typename OutputBuilder::Node_t RequestHandler::BuildMapStatNode(int id, map_settings_t& map_settings){
    
    renderer_->SetSettings(map_settings);

    std::stringstream stream;
    renderer_->Render(GetBusesAscendingName(), GetStopsAscendingName(), stream);

    return OutputBuilder{}.StartDict()
            .Key("request_id").Value(id)
            .Key("map").Value(stream.str())
            .EndDict().Build();
}

template <typename OutputBuilder>
inline typename OutputBuilder::Node_t RequestHandler::BuildNotFoundNode(int id){
    using namespace std::string_literals;
    return OutputBuilder{}.StartDict()
            .Key("request_id").Value(id)
            .Key("error_message").Value("not found"s)
            .EndDict().Build();
}

} // namespace TC