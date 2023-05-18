#include "json_reader.h"
#include <string>
#include "map_renderer.h"

json::Document TC::Input::Json::Reader(RequestHandler& request_handler, std::istream& input){
    
    const auto document = json::Load(input);

    if(!document.GetRoot().IsMap())
        throw json::ParsingError("Json::reader top level parsing error");

    // -- base requests --
    const auto& base_requests = document.GetRoot().AsMap().find("base_requests")->second;

    if(!base_requests.IsArray())
        throw json::ParsingError("Json::reader second level base parsing error");

    for(const auto& request : base_requests.AsArray()){

        if(!request.IsMap())
            throw json::ParsingError("Json::reader bottom level base parsing error");
        
        RequestHandler::base_request_t base_request;

        const auto& request_map = request.AsMap();

        if(request_map.at("type").AsString() == "Bus"){
            base_request.type = request_map.at("type").AsString();
            base_request.name = request_map.at("name").AsString();
            base_request.is_roundtrip = request_map.at("is_roundtrip").AsBool();

            for(const auto& stop_node : request_map.at("stops").AsArray()){
                base_request.stops.push_back(stop_node.AsString());
            }
        }

        if(request_map.at("type").AsString() == "Stop"){
            base_request.type = request_map.at("type").AsString();
            base_request.name = request_map.at("name").AsString();
            base_request.latitude = request_map.at("latitude").AsDouble();
            base_request.longitude = request_map.at("longitude").AsDouble();

            for(const auto& [name, dist_node]: request_map.at("road_distances").AsMap()){
                base_request.road_distances.insert({name, dist_node.AsInt()});
            }
        }

        request_handler.QueueAddRequest(base_request);

    }

    request_handler.FulfillAddRequests();

    return document;
}

void TC::Input::Json::ReadStatRequests(RequestHandler& request_handler, const json::Document& document, std::ostream& output){

    const auto&  stat_requests = document.GetRoot().AsMap().find("stat_requests")->second;

    if(!stat_requests.IsArray())
        throw json::ParsingError("Json::reader second level stat parsing error");

    json::Array json_output;

    for(const auto& request : stat_requests.AsArray()){
        
        using namespace std::string_literals;

        if(!request.IsMap())
            throw json::ParsingError("Json::reader bottom level stat parsing error");

        const auto& request_map = request.AsMap();

        int id = request_map.at("id").AsInt();
        const auto& type = request_map.at("type").AsString();
        const auto& name = request_map.at("name").AsString();

        json::Dict json_request;
        json_request["request_id"] = json::Node{id};

        if (type == "Bus"){

            if (const auto& bus_stat = request_handler.GetBusStat(name)){
                
                json_request["curvature"] = json::Node{bus_stat->curvature};
                json_request["route_length"] = json::Node{bus_stat->length};
                json_request["stop_count"] = json::Node{bus_stat->stop_count};
                json_request["unique_stop_count"] = json::Node{bus_stat->unique_stop_count};

            } else {
                json_request["error_message"] = json::Node{"not found"s};

            }

        } else 
        if (type == "Stop"){

            if (const auto& stop_stat = request_handler.GetStopStat(name)){
                
                json::Array stops;
                
                for(const auto stop : stop_stat->buses){
                    stops.push_back(json::Node{std::string(stop)});
                }
                json_request["buses"] = json::Node{stops};

            } else {
                json_request["error_message"] = json::Node{"not found"s};

            }
        }

        json_output.push_back(json::Node{json_request});
    }

    json::PrintNode(json_output, output);
    
}

Renderer::map_settings_t TC::Input::Json::ReadMapRenderSettings(const json::Document& document){

    const auto&  settings_node = document.GetRoot().AsMap().find("render_settings")->second;

    if(!settings_node.IsMap())
            throw json::ParsingError("Json::reader bottom level render parsing error");

    const auto&  settings_map = settings_node.AsMap();

    Renderer::map_settings_t settings;

    settings.width = settings_map.at("width").As;
    settings.height = settings_map.at("height");
    settings.padding = settings_map.at("padding");
    settings.line_width = settings_map.at("line_width");
    settings.stop_radius = settings_map.at("stop_radius");
    settings.bus_label_font_size = settings_map.at("bus_label_font_size");
    settings.stop_label_font_size = settings_map.at("stop_label_font_size");
    settings.underlayer_width = settings_map.at("underlayer_width");

    settings.bus_label_offset = settings_map[""];
}