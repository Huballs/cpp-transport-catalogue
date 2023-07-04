#include "transport_catalogue.h"
#include "router.h"
#include "domain.h"
#include <memory>
#include <map>
#include <iostream>

namespace TC {

class TransportRouter {

public:
    TransportRouter(const TransportCatalogue& catalogue, routing_settings_t settings) : catalogue_(catalogue), settings_(std::move(settings)){

        size_t graph_stops_count = catalogue.GetStops().size()*2 + catalogue.GetBuses().size() * catalogue.GetStops().size();

        graph_ = std::make_unique<graph::DirectedWeightedGraph<Weight>>(graph_stops_count);

        size_t bus_wait_distance_ = 1.0 * settings_.bus_wait_time / 60 * settings_.bus_velocity *1000 / 2; // bus wait time converted to distance

        size_t bus_stop_id = catalogue.GetStops().size();

        for(const auto& bus : catalogue.GetBuses()){

            const auto& stops = bus.GetStops();
            
            auto next_stop = std::next(stops.begin());

            for(auto stop = stops.begin(); next_stop != stops.end(); std::advance(next_stop, 1)){

                size_t actual_stop_id = (*stop)->GetIndex();
                size_t actual_next_stop_id = (*next_stop)->GetIndex();

                weight w{catalogue.GetDistance(*stop, *next_stop), false, bus.GetName(), actual_stop_id, actual_next_stop_id};

                graph_->AddEdge({bus_stop_id, bus_stop_id+1, w});

                graph_->AddEdge({actual_stop_id, bus_stop_id, {bus_wait_distance_, true, {}, actual_stop_id, actual_stop_id}});
                graph_->AddEdge({bus_stop_id, actual_stop_id, {bus_wait_distance_, true, {},  actual_stop_id, actual_stop_id}});
                
                stop = next_stop;
                ++bus_stop_id;
            }

            graph_->AddEdge({(stops.back())->GetIndex(), bus_stop_id, {bus_wait_distance_, true, {}, (stops.back())->GetIndex(),(stops.back())->GetIndex()}});
            graph_->AddEdge({bus_stop_id, (stops.back())->GetIndex(), {bus_wait_distance_, true, {}, (stops.back())->GetIndex(),(stops.back())->GetIndex()}});

            size_t bus_stop_id_go_back = bus_stop_id;

            bus_stop_id++;

            if(!bus.IsCircular()) {
                auto rnext_stop = std::next(stops.rbegin());

                for(auto stop = stops.rbegin(); rnext_stop != stops.rend(); std::advance(rnext_stop, 1)){

                    size_t actual_stop_id = (*stop)->GetIndex();
                    size_t actual_next_stop_id = (*rnext_stop)->GetIndex();

                    weight w{catalogue.GetDistance(*stop, *rnext_stop),false ,bus.GetName(), actual_stop_id, actual_next_stop_id};

                    graph_->AddEdge({bus_stop_id_go_back, bus_stop_id_go_back-1, w});

                    stop = rnext_stop;
                    --bus_stop_id_go_back;
                }
            }
        }
        
        router_ = std::make_unique<graph::Router<Weight>>(*graph_);
    }

    enum class Route_t{
        WAIT,
        BUS,
    };

    struct RouteLine{
        Route_t type;
        std::string_view name;
        double time;
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

        if(!stop_from || !stop_to)
            return std::nullopt;

        size_t index_from = stop_from->GetIndex(); 
        size_t index_to = stop_to->GetIndex();

        auto info = router_->BuildRoute(index_from, index_to);

        if(!info)
            return std::nullopt;

        Travel travel;

        travel.lines.reserve(info->edges.size());

        travel.total_time = DistanceToTime(info->weight.distance);
        

        size_t from_stop = graph_->GetEdge(info->edges[0]).weight.from;

        travel.lines.push_back({Route_t::WAIT
                        ,catalogue_.GetStopByIndex(from_stop)->GetName()
                        ,static_cast<double>(settings_.bus_wait_time)
                        });

        for(auto begin = ++(info->edges.begin()); begin < info->edges.end(); ++begin ){
            
            auto edgeID = *begin;

            RouteLine line;

            from_stop = graph_->GetEdge(edgeID).weight.from;

            if(graph_->GetEdge(edgeID).weight.is_waiting){
                line.type = Route_t::WAIT;
                line.time = settings_.bus_wait_time;
                line.name = catalogue_.GetStopByIndex(from_stop)->GetName();
                ++begin; // there are 2 wait times per stop in the graph, skip one
            } else {
                line.type = Route_t::BUS;
                line.time = DistanceToTime(graph_->GetEdge(edgeID).weight.distance);
                line.name = graph_->GetEdge(edgeID).weight.bus;
            }

            travel.lines.push_back(std::move(line));

        }

        travel.lines.erase(--travel.lines.end());
/*
        for(auto& r : travel.lines){
            if(r.type == Route_t::WAIT){
                std::cout << "wait: " << r.name << " t: " << r.time;
            } else {
                std::cout << "bus:  " << r.name << " t: " << r.time;
            }
            std::cout << std::endl;
        }
        std::cout << "total: " << travel.total_time << std::endl << std::endl;*/

        return travel;

    }

    inline double DistanceToTime(size_t distance){
        return 1.0 * distance  / settings_.bus_velocity /1000 *60;
    }

private:

    struct weight {
        size_t distance;
        bool is_waiting;
        std::string_view bus;
        size_t from;
        size_t to;

        bool operator>(const weight& other) const {
            return distance > other.distance;
        }
        bool operator<(const weight& other) const {
            return distance < other.distance;
        }
        weight operator+(const weight& other) const{
            weight w;
            w.distance = distance + other.distance;
            return w;
        }
    };

    using Weight = weight;

    const TransportCatalogue& catalogue_;
    std::unique_ptr<graph::DirectedWeightedGraph<Weight>> graph_;
    std::unique_ptr<graph::Router<Weight>> router_;
    routing_settings_t settings_;
};

} // namespace TC