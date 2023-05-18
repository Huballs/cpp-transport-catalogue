#pragma once

#include "transport_catalogue.h"
#include <optional>
#include <unordered_set>

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

        // MapRenderer понадобится в следующей части итогового проекта
        //RequestHandler(const TransportCatalogue& db, const renderer::MapRenderer& renderer);
        RequestHandler(TransportCatalogue& db) : db_(db){};

        /* stores add request for later fulfillment */
        void QueueAddRequest(base_request_t& request);

        /* makes requests to tc in order: add stops, add busses if any*/
        void FulfillAddRequests();

        // Возвращает информацию о маршруте (запрос Bus)
        std::optional<stat_bus_t> GetBusStat(const std::string_view& bus_name) const;
        
        // Возвращает информацию о маршруте (запрос Stop)
        std::optional<stat_stop_t> GetStopStat(const std::string_view& stop_name) const;

        // Возвращает маршруты, проходящие через
        //const std::unordered_set<Bus*>* GetBusesByStop(const std::string_view& stop_name) const;

        // Этот метод будет нужен в следующей части итогового проекта
        //svg::Document RenderMap() const;

    private:

        std::vector<base_request_t> requests_add_stop;
        std::vector<base_request_t> requests_add_bus;

        // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
        TransportCatalogue& db_;
        //const renderer::MapRenderer& renderer_;
    };
} // namespace TC