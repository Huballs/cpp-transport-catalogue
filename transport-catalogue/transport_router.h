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

        graph_ = std::make_unique<graph::DirectedWeightedGraph<Weight>>(catalogue.GetStops().size()*10);

        size_t bus_wait_distance_ = (double)settings_.bus_wait_time / 60 * settings_.bus_velocity *1000 / 2; // bus wait time converted to distance

        //std::map<std::pair<size_t, size_t>, std::vector<const Bus*>> stops_to_buses;

        size_t bus_stop_id = catalogue.GetStops().size();

        for(const auto& bus : catalogue.GetBuses()){

            const auto& stops = bus.GetStops();
            
            auto next_stop = std::next(stops.begin());

            for(auto stop = stops.begin(); next_stop != stops.end(); std::advance(next_stop, 1)){

                //graph_->AddEdge({(*stop)->GetIndex(), (*next_stop)->GetIndex(), catalogue.GetDistance(*stop, *next_stop)});

                weight w{catalogue.GetDistance(*stop, *next_stop), bus.GetName()};

                graph_->AddEdge({bus_stop_id, bus_stop_id+1, w});

                std::cout << "edge from: " << bus_stop_id << " to: " << bus_stop_id+1 << " dist: " << w.distance << std::endl; 

                size_t actual_stop_id = (*stop)->GetIndex();

                graph_->AddEdge({actual_stop_id, bus_stop_id, bus_wait_distance_});
                graph_->AddEdge({bus_stop_id, actual_stop_id, bus_wait_distance_});

                std::cout << "intercon : " << actual_stop_id << " to: " << bus_stop_id << std::endl; 
                
                stop = next_stop;
                ++bus_stop_id;
            }

            graph_->AddEdge({(stops.back())->GetIndex(), bus_stop_id, bus_wait_distance_});
            graph_->AddEdge({bus_stop_id, (stops.back())->GetIndex(), bus_wait_distance_});

            size_t bus_stop_id_go_back = bus_stop_id;

            bus_stop_id++;

            if(!bus.IsCircular()) {
                auto rnext_stop = std::next(stops.rbegin());
                //--bus_stop_id;
                for(auto stop = stops.rbegin(); rnext_stop != stops.rend(); std::advance(rnext_stop, 1)){

                    //graph_->AddEdge({(*stop)->GetIndex(), (*rnext_stop)->GetIndex(), catalogue.GetDistance(*stop, *rnext_stop)});

                    weight w{catalogue.GetDistance(*stop, *rnext_stop), bus.GetName()};

                    graph_->AddEdge({bus_stop_id_go_back, bus_stop_id_go_back-1, w});

                    stop = rnext_stop;
                    --bus_stop_id_go_back;
                }
            }
        }
/*
        for (const auto& [stops, distance] : catalogue.GetStopsDistances()){

            Weight w{catalogue.GetDistance(stops.first, stops.second)
                   , FindConnectingBuses(stops.first, stops.second)};

            graph_->AddEdge({stops.first->GetIndex(), stops.second->GetIndex(), w});
        }*/
        
        router_ = std::make_unique<graph::Router<Weight>>(*graph_);
    }

    void Route(std::string_view from, std::string_view to){
        size_t index_from = catalogue_.GetStop(from)->GetIndex(); 
        size_t index_to = catalogue_.GetStop(to)->GetIndex();

        auto info = router_->BuildRoute(index_from, index_to);

        for(const auto& edgeID : info->edges){
            //index_from = graph_->GetEdge(edgeID).from;
            //index_to = graph_->GetEdge(edgeID).to;
            //std::cout << catalogue_.GetStopByIndex(index_from)->GetName() << " - " 
            //          << catalogue_.GetStopByIndex(index_to)->GetName() << " - " 
            std::cout << graph_->GetEdge(edgeID).weight.bus << " - "
                      << graph_->GetEdge(edgeID).weight.distance << " - "
                      << (double)graph_->GetEdge(edgeID).weight.distance / settings_.bus_velocity /1000 *60
                      << std::endl;
        }
        std::cout << "total: " 
                  <<  info->weight.distance << " - " 
                  << (double)info->weight.distance  / settings_.bus_velocity /1000 *60
                  << std::endl;
        std::cout << std::endl;
    }

private:

    std::vector<std::string_view> FindConnectingBuses(Stop* from, Stop* to){
        std::vector<std::string_view> result;
        for(const auto& bus : from->GetBuses()){
            const auto stops = catalogue_.GetBus(bus)->GetStops();
            auto it = std::find(stops.begin(), stops.end(), from);
            if(it != stops.end()){
                if(*(it+1) == to)
                    result.push_back(bus);
            }
        }
        return result;
    }

    struct weight {
        size_t distance;
        std::string_view bus;

        bool operator>(const weight& other) const {
            return distance > other.distance;
        }
        bool operator<(const weight& other) const {
            return distance < other.distance;
        }
        weight operator+(const weight& other) const{
            weight w{distance + other.distance};
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