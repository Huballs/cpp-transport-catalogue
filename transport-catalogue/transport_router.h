#include "transport_catalogue.h"
#include "router.h"
#include "domain.h"
#include <memory>

namespace TC {

class TransportRouter {

public:
    TransportRouter(const TransportCatalogue& catalogue, routing_settings_t settings) : catalogue_(catalogue), settings_(std::move(settings)){

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

    enum class RouteLine_t{
        WAIT,
        BUS,
    };

    struct RouteLine{
        RouteLine_t type;
        std::string_view name;
        double time;
        size_t span_count;
    };

    struct Travel{
        std::vector<RouteLine> lines;
        double total_time;
    };

    std::optional<Travel> Route(std::string_view from, std::string_view to){

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

        travel.total_time = DistanceToTime(info->weight);

        for(auto begin = (info->edges.begin()); begin < info->edges.end(); ++begin ){
            
            auto edgeID = *begin;

            RouteLine line;

            index_from = edge_infos_[edgeID].from;

            travel.lines.push_back({RouteLine_t::WAIT
                ,catalogue_.GetStopByIndex(index_from)->GetName()
                ,static_cast<double>(settings_.bus_wait_time)
                ,0
                });

            line.type = RouteLine_t::BUS;
            line.time = DistanceToTime(edge_infos_[edgeID].distance);
            line.name = edge_infos_[edgeID].bus;
            line.span_count = edge_infos_[edgeID].span_count;

            travel.lines.push_back(std::move(line));

        }

        return travel;

    }

    inline double DistanceToTime(size_t distance){
        return 1.0 * distance  / settings_.bus_velocity /1000 *60;
    }

private:

    struct EdgeInfo{
        std::string_view bus;
        size_t from;
        size_t to;
        size_t distance;
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
                edge_info.distance = distance;
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

    const size_t bus_wait_distance_ = 1.0 * settings_.bus_wait_time / 60 * settings_.bus_velocity *1000;
};

} // namespace TC