#include "request_handler.h"

void TC::RequestHandler::QueueRequest(base_request_t& request) {

    if (request.type == "Stop"){
        requests_add_stop.push_back(std::move(request));
    } else 
    if (request.type == "Bus"){
         requests_add_bus.push_back(std::move(request));
    }
}

void TC::RequestHandler::QueueRequest(stat_request_t& request){
    requests_stat.push_back(std::move(request));
}

void TC::RequestHandler::FulfillRequests(){

    for(auto& request : requests_add_stop){
        db_.AddStop(request.name, {request.latitude, request.longitude});
        
    }

    for(auto& request : requests_add_stop){
        db_.AddDistances(request.name, request.road_distances);
    }

    for(auto& request : requests_add_bus){
        db_.AddRoute(request.name, {request.stops, request.is_roundtrip});
    }
}

