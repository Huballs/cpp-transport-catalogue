#pragma once

#include "transport_catalogue.h"
#include "router.h"
#include "domain.h"
#include <memory>

namespace TC {

class TransportRouter {

public:
    TransportRouter(const TransportCatalogue& catalogue, routing_settings_t settings);

    enum class RouteLine_t{
        WAIT,
        BUS,
    };

    struct RouteLine{
        RouteLine_t type;
        std::string_view name;
        double time_min;   // time in minutes
        size_t span_count;
    };

    struct Travel{
        std::vector<RouteLine> lines;
        double total_time_min;
    };

    std::optional<Travel> Route(std::string_view from, std::string_view to);

    // convert distance in meters to time traveled in minutes
    inline double DistanceToTime(size_t distance_m){
        return 1.0 * distance_m  / settings_.bus_velocity_kmh / distanceTimeMulti;
    }

private:

    struct EdgeInfo{
        std::string_view bus;
        size_t from;
        size_t to;
        size_t distance_m;
        size_t span_count;
    };

    inline void StoreEdgeInfo(graph::EdgeId id, const EdgeInfo& info){
        if(id >= edge_infos_.size())
            edge_infos_.resize(id * 1.5 + 2);
        edge_infos_[id] = std::move(info);
    }

    template <typename It>
    void AddStopsToGraph(It begin, It end, std::string_view bus_name){
        for(auto stop = begin; stop != end; std::advance(stop, 1)){

            size_t distance = 0;
            size_t span_count = 1;

            auto prev_stop = stop;

            for(auto stop2 = std::next(stop); stop2 != end; std::advance(stop2, 1)){
                distance += catalogue_.GetDistance(*prev_stop, *stop2);
                graph::EdgeId edge_id = graph_->AddEdge({(*stop)->GetIndex(), (*stop2)->GetIndex(), distance + bus_wait_distance_});

                EdgeInfo edge_info;
                edge_info.bus = bus_name;
                edge_info.from = (*stop)->GetIndex();
                edge_info.to = (*stop2)->GetIndex();
                edge_info.distance_m = distance;
                edge_info.span_count = span_count++;
                StoreEdgeInfo(edge_id, edge_info);

                prev_stop = stop2;
            }

        }
    }

    using Weight = size_t;

    const TransportCatalogue& catalogue_;
    std::unique_ptr<graph::DirectedWeightedGraph<Weight>> graph_;
    std::unique_ptr<graph::Router<Weight>> router_;
    routing_settings_t settings_;
    std::vector<EdgeInfo> edge_infos_;

    static constexpr double distanceTimeMulti = 1000.0/60.0; //(meters in km)/(minutes in h)

    const size_t bus_wait_distance_ = 1.0 * settings_.bus_wait_time_min * settings_.bus_velocity_kmh * distanceTimeMulti;
};

} // namespace TC