#include "json_reader.h"
#include "request_handler.h"

void TC::Input::Json::Reader(TransportCatalogue& catalogue, std::istream& input){
    
    auto document = json::Load(input);

    if(!document.GetRoot().IsMap())
        throw json::ParsingError("Json::reader top level parsing error");

    auto requests = document.GetRoot().AsMap().find("base_requests");

    if(!requests->second.IsArray())
        throw json::ParsingError("Json::reader second level parsing error");

    RequestHandler request_handler(catalogue);

    for(const auto& request : requests->second.AsArray()){

        if(!request.IsMap())
            throw json::ParsingError("Json::reader bottom level parsing error");
        
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

        request_handler.QueueRequest(base_request);

    }
}