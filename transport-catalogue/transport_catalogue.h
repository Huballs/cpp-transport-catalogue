#pragma once
#include <string_view>
#include <vector>
#include <deque>
#include <string>
#include <unordered_map>
#include <set>
#include <cstdint>
#include "geo.h"

namespace TC {

    using namespace Geo;

    class TransportCatalogue;

    class Stop {
        std::string name;
        Coordinates coordinates;
        std::set<std::string_view> buses;
        size_t index;

        friend TransportCatalogue;

    public:
        explicit Stop(const std::string& _name, const Coordinates& _coordinates) : name(_name), coordinates(_coordinates){};

        const std::set<std::string_view>& GetBuses() const {return buses;}
        std::string_view GetName() const { return name;}
        Coordinates getCoordinates() const { return coordinates;}
        size_t GetBusCount() const { return buses.size();}
        size_t GetIndex() const {return index;}
    };

    class Bus {
        std::string name;
        std::deque<Stop*> stops;

        friend TransportCatalogue;

        int stops_count = 0;
        int stops_unique_count = 0;
        double length_direct = 0;
        uint32_t length_traveled = 0;
        double curvature = 0;

        bool isCircle = false;

    public:
        explicit Bus(const std::string& _name) : name(_name){};

        std::string_view GetName() const { return name;}
        double GetLengthDirect() const {return length_direct;}
        uint32_t GetLengthTraveled() const {return length_traveled;}
        double GetCurvature() const {return curvature;}
        int GetStopsCount() const {return stops_count;}
        int GetStopsCountUnique() const {return stops_unique_count;}
        bool IsCircular() const {return isCircle;}
        const std::deque<Stop*>& GetStops() const {return stops;}
    };

    class TransportCatalogue {

    private:

        std::deque<Stop> stops_;
        std::unordered_map<std::string_view, Stop*> stopname_to_stops_;

        std::deque<Bus> buses_;
        std::unordered_map<std::string_view, Bus*> busname_to_bus_;

        struct TwoPointerStopsJasher {
            size_t operator()(const std::pair<Stop*, Stop*>& stops) const {
                    return hasher(stops.first) + hasher(stops.second);
                }
            std::hash<const void*> hasher;
        };

        std::unordered_map<std::pair<Stop*, Stop*>, uint32_t, TwoPointerStopsJasher> stops_distances_;

    public:
        
        void AddDistances(std::string_view name, const std::unordered_map<std::string_view, uint32_t>& stop_to_distance);
        uint32_t GetDistance(Stop* stop1, Stop* stop2) const;

        void AddStop(std::string_view name, const Coordinates& coordinates);

        /* stops - <vector<string>, isCircular> */
        void AddRoute(std::string_view name, const std::pair<std::vector<std::string_view>, bool>& stops);

        bool ContainsBus(std::string_view name) const;
        Bus* GetBus(std::string_view name) const;

        bool ContainsStop(std::string_view name) const;
        Stop* GetStop(std::string_view name) const;

        const std::deque<Bus>& GetBuses() const;
        const std::deque<Stop>& GetStops() const;

        const auto& GetStopsDistances() const {
            return stops_distances_;
        }

        const Stop* GetStopByIndex(size_t index) const{
            return &(stops_[index]);
        }

        double GetRouteLengthDirect(std::string_view name) const;
        int GetRouteStopsCount(std::string_view name) const;
        int GetRouteStopsUniqueCount(std::string_view name) const;

    };

} // namespace TC