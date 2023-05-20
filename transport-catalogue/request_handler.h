#pragma once

#include "transport_catalogue.h"
#include <optional>
#include <unordered_set>
#include <map>

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

        RequestHandler(TransportCatalogue& db) : db_(db){};

        /* stores add request for later fulfillment */
        void QueueAddRequest(base_request_t& request);

        /* makes requests to tc in order: add stops, add busses if any*/
        void FulfillAddRequests();

        // Возвращает информацию о маршруте (запрос Bus)
        std::optional<stat_bus_t> GetBusStat(const std::string_view& bus_name) const;
        
        // Возвращает информацию о маршруте (запрос Stop)
        std::optional<stat_stop_t> GetStopStat(const std::string_view& stop_name) const;

        const std::deque<Bus>& GetBuses() const;

        std::map<std::string_view, const Bus*> GetBusMapAscendingName() const;
        std::map<std::string_view, const Stop*> GetStopMapAscendingName() const;

    private:

        std::vector<base_request_t> requests_add_stop;
        std::vector<base_request_t> requests_add_bus;

        TransportCatalogue& db_;

    };
} // namespace TC