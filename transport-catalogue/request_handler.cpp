#include "request_handler.h"


namespace TC {

    void RequestHandler::QueueAddRequest(base_request_t& request) {

        if (request.type == "Stop"){
            requests_add_stop.push_back(std::move(request));
        } else 
        if (request.type == "Bus"){
            requests_add_bus.push_back(std::move(request));
        }
    }

    void RequestHandler::FulfillAddRequests(){

        for(const auto& request : requests_add_stop){
            db_.AddStop(request.name, {request.latitude, request.longitude});
            
        }

        for(const auto& request : requests_add_stop){
            db_.AddDistances(request.name, request.road_distances);
        }

        for(const auto& request : requests_add_bus){
            db_.AddRoute(request.name, {request.stops, request.is_roundtrip});
        }

    }

    std::optional<RequestHandler::stat_bus_t> RequestHandler::GetBusStat(const std::string_view& bus_name) const {

        if (!db_.ContainsBus(bus_name))
            return std::nullopt;

        const auto bus = db_.GetBus(bus_name);

        stat_bus_t bus_stat;

        bus_stat.curvature = bus->GetCurvature();
        bus_stat.length = bus->GetLengthTraveled();
        bus_stat.stop_count = bus->GetStopsCount();
        bus_stat.unique_stop_count = bus->GetStopsCountUnique();

        return bus_stat;
    }

    std::optional<RequestHandler::stat_stop_t> RequestHandler::GetStopStat(const std::string_view& stop_name) const {
        
        if (!db_.ContainsStop(stop_name))
            return std::nullopt;

        const auto stop = db_.GetStop(stop_name);

        stat_stop_t stop_stat{stop->GetBuses()};

        return stop_stat;

    }

    std::map<std::string_view, const Bus*> RequestHandler::GetBusMapAscendingName() const {

        std::map<std::string_view, const Bus*> result;

        for(const auto& bus : db_.GetBuses()){
            result.insert({bus.GetName(), &bus});
        }

        return result;
    }

    std::map<std::string_view, const Stop*> RequestHandler::GetStopMapAscendingName() const {

        std::map<std::string_view, const Stop*> result;

        for(const auto& stop : db_.GetStops()){
            result.insert({stop.GetName(), &stop});
        }

        return result;
    }

    const std::deque<Bus>& RequestHandler::GetBuses() const{
        return db_.GetBuses();
    }


} // TC

