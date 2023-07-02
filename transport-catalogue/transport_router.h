#include "transport_catalogue.h"
#include "router.h"
#include <memory>

namespace TC {

class TransportRouter {

public:
    TransportRouter(const TransportCatalogue& catalogue) : catalogue_(catalogue){

        graph_ = std::make_unique<graph::DirectedWeightedGraph<Weight>>(catalogue.GetStopsDistances().size()*2);

        for(const auto& bus : catalogue.GetBuses()){

            const auto& stops = bus.GetStops();
            
            auto next_stop = std::next(stops.begin());

            for(auto stop = stops.begin(); next_stop != stops.end(); std::advance(next_stop, 1)){
                Weight w{catalogue.GetDistance(*stop, *next_stop), bus.GetName()};
                graph_->AddEdge({(*stop)->GetIndex(), (*next_stop)->GetIndex(), w});
                stop = next_stop;
            }
            if(!bus.IsCircular()) {
                auto rnext_stop = std::next(stops.rbegin());
                for(auto stop = stops.rbegin(); rnext_stop != stops.rend(); std::advance(rnext_stop, 1)){
                    Weight w{catalogue.GetDistance(*stop, *rnext_stop), bus.GetName()};
                    graph_->AddEdge({(*stop)->GetIndex(), (*rnext_stop)->GetIndex(), w});
                    stop = rnext_stop;
                }
            }
        }

        router_ = std::make_unique<graph::Router<Weight>>(*graph_);
    }

    void Route(std::string_view from, std::string_view to){
        size_t index_from = catalogue_.GetStop(from)->GetIndex(); 
        size_t index_to = catalogue_.GetStop(to)->GetIndex();

        auto info = router_->BuildRoute(index_from, index_to);
    }

private:

    struct weight{
        size_t distance;
        std::string_view bus;

        bool operator<(const weight& other) const {
            return distance < other.distance;
        }
        bool operator>(const weight& other) const {
            return distance > other.distance;
        }
        weight& operator+(const weight& other) {
            distance += other.distance;
            return *this;
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
};

} // namespace TC