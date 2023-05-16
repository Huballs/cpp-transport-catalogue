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

            bool operator>(const base_request_t& other) const {
                return (type > other.type) || (name < other.name);
            }
        };

        // MapRenderer понадобится в следующей части итогового проекта
        //RequestHandler(const TransportCatalogue& db, const renderer::MapRenderer& renderer);
        RequestHandler(const TransportCatalogue& db) : db_(db){};

        /* stores request for later fulfillment - stops first, as required by transport catalogue */
        void QueueRequest(base_request_t& request);

        // Возвращает информацию о маршруте (запрос Bus)
        //std::optional<BusStat> GetBusStat(const std::string_view& bus_name) const;

        // Возвращает маршруты, проходящие через
        const std::unordered_set<Bus*>* GetBusesByStop(const std::string_view& stop_name) const;

        // Этот метод будет нужен в следующей части итогового проекта
        //svg::Document RenderMap() const;

    private:

        std::set<base_request_t, std::greater<base_request_t>> request_queue_; // needs to be sorted - stops first

        // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
        const TransportCatalogue& db_;
        //const renderer::MapRenderer& renderer_;
    };
} // namespace TC