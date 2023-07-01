#include "transport_catalogue.h"
#include <unordered_set>

namespace TC {

    uint32_t TransportCatalogue::GetDistance(Stop* stop1, Stop* stop2){

        auto it = stops_distances_.find({stop1, stop2});
        if(it != stops_distances_.end())
            return it->second;

        it = stops_distances_.find({stop2, stop1});
        if(it != stops_distances_.end())
            return it->second;
        return 0;
    }


    void TransportCatalogue::AddDistances(std::string_view name, const std::unordered_map<std::string_view, uint32_t>& stop_to_distance){
        for(const auto [stop, distance] : stop_to_distance){
            stops_distances_[{GetStop(name), GetStop(stop)}] = distance;
        }
    }

    void TransportCatalogue::AddStop(std::string_view name, const Coordinates& coordinates){

        stops_.push_back(Stop{std::string(name), coordinates});

        stops_.back().index = stops_.size() - 1;

        stopname_to_stops_.insert({stops_.back().name, &stops_.back()});
        
    }

    void TransportCatalogue::AddRoute(std::string_view name, const std::pair<std::vector<std::string_view>, bool>& stops){

        buses_.push_back(Bus((std::string(name))));
        Bus* bus = &buses_.back();

        std::unordered_set<std::string_view> unique_stops;
        Stop* prev_stop = stopname_to_stops_[stops.first[0]];
        Coordinates prev_coordinate = prev_stop->coordinates;

        for (std::string_view stop : stops.first){
            Stop* stored_stop = stopname_to_stops_[stop];

            bus->stops.push_back(stored_stop);
            unique_stops.insert(stop);

            stored_stop->buses.insert(bus->name);

            bus->length_direct += ComputeDistance(stored_stop->coordinates, prev_coordinate);
            prev_coordinate = stored_stop->coordinates;

            bus->length_traveled += GetDistance(prev_stop, stored_stop);
            prev_stop = stored_stop;
        }

        if(stops.second == true){
            bus->isCircle = true;

            bus->stops_count = bus->stops.size();
        } else {
            bus->stops_count = bus->stops.size() * 2 - 1;
            bus->length_direct *= 2;

            prev_stop = bus->stops.back();
            
            for(auto begin = bus->stops.rbegin(); begin != bus->stops.rend(); ++begin){
                bus->length_traveled += GetDistance(prev_stop, *begin);
                prev_stop = *begin;
            }
        }

        bus->curvature = bus->length_traveled / bus->length_direct;

        bus->stops_unique_count = unique_stops.size();

        busname_to_bus_.insert({bus->name, bus});

    }

    bool TransportCatalogue::ContainsBus(std::string_view name) const{
        if(busname_to_bus_.find(name) == busname_to_bus_.end())
            return false;
        return true;
    }

    Bus* TransportCatalogue::GetBus(std::string_view name) const{
        return busname_to_bus_.at(name);
    }

    bool TransportCatalogue::ContainsStop(std::string_view name) const{
        if(stopname_to_stops_.find(name) == stopname_to_stops_.end())
            return false;
        return true;
    }

    Stop* TransportCatalogue::GetStop(std::string_view name) const{
        return stopname_to_stops_.at(name);
    }

    const std::deque<Bus>& TransportCatalogue::GetBuses() const{
        return buses_;
    }

    const std::deque<Stop>& TransportCatalogue::GetStops() const{
        return stops_;
    }

    double TransportCatalogue::GetRouteLengthDirect(std::string_view name) const{
        return busname_to_bus_.at(name)->length_direct;
    }

    int TransportCatalogue::GetRouteStopsCount(std::string_view name) const{
        return busname_to_bus_.at(name)->stops_count;
    }

    int TransportCatalogue::GetRouteStopsUniqueCount(std::string_view name) const{
        return busname_to_bus_.at(name)->stops_unique_count;
    }

} // namespace TC