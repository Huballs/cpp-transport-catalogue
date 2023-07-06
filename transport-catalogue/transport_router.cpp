#include "transport_router.h"

namespace TC {

    TransportRouter::TransportRouter(const TransportCatalogue& catalogue, routing_settings_t settings) : catalogue_(catalogue), settings_(std::move(settings)){

        graph_ = std::make_unique<graph::DirectedWeightedGraph<Weight>>();

        for(const auto& bus : catalogue_.GetBuses()){

            const auto& stops = bus.GetStops();

            AddStopsToGraph(stops.begin(), stops.end(), bus.GetName());

            if(!bus.IsCircular()) {

                AddStopsToGraph(stops.rbegin(), stops.rend(), bus.GetName());

            }
        }
        
        router_ = std::make_unique<graph::Router<Weight>>(*graph_);
    }

    std::optional<TransportRouter::Travel> TransportRouter::Route(std::string_view from, std::string_view to){

        if(from == to)
            return Travel{{}, 0};

        auto stop_from = catalogue_.GetStop(from);
        auto stop_to = catalogue_.GetStop(to);

        if(!stop_from || !stop_to || !stop_from->GetBusCount() || !stop_to->GetBusCount())
            return std::nullopt;

        size_t index_from = stop_from->GetIndex(); 
        size_t index_to = stop_to->GetIndex();

        auto info = router_->BuildRoute(index_from, index_to);

        if(!info)
            return std::nullopt;

        Travel travel;

        travel.lines.reserve(info->edges.size());

        travel.total_time_min = DistanceToTime(info->weight);

        for(auto begin = (info->edges.begin()); begin < info->edges.end(); ++begin ){
            
            auto edgeID = *begin;

            RouteLine line;

            index_from = edge_infos_[edgeID].from;

            travel.lines.push_back({RouteLine_t::WAIT
                ,catalogue_.GetStopByIndex(index_from)->GetName()
                ,static_cast<double>(settings_.bus_wait_time_min)
                ,0
                });

            line.type = RouteLine_t::BUS;
            line.time_min = DistanceToTime(edge_infos_[edgeID].distance_m);
            line.name = edge_infos_[edgeID].bus;
            line.span_count = edge_infos_[edgeID].span_count;

            travel.lines.push_back(std::move(line));

        }

        return travel;

    }

} // namespace TC