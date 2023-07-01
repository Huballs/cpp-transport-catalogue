#include "transport_catalogue.h"
#include "router.h"
#include <memory>

namespace TC {

class TransportRouter {

public:
    TransportRouter(const TransportCatalogue& catalogue) : catalogue_(catalogue){

        graph_ = std::make_unique<graph::DirectedWeightedGraph<uint32_t>>(catalogue.GetStopsDistances().size());

        for(const auto& [stops, distance] : catalogue.GetStopsDistances()){
            graph_->AddEdge({stops.first->GetIndex()
                         , stops.second->GetIndex()
                         , distance});
        }

        router_ = std::make_unique<graph::Router<uint32_t>>(*graph_);
    }

    void Route(std::string_view from, std::string_view to){
        size_t index_from = catalogue_.GetStop(from)->GetIndex(); 
        size_t index_to = catalogue_.GetStop(to)->GetIndex();

        auto info = router_->BuildRoute(index_from, index_to);
    }

private:
    const TransportCatalogue& catalogue_;
    std::unique_ptr<graph::DirectedWeightedGraph<uint32_t>> graph_;
    std::unique_ptr<graph::Router<uint32_t>> router_;
};

} // namespace TC