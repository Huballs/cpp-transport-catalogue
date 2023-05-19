#include "map_renderer.h"

namespace TC::Renderer {

    void  MapRenderer::Render(const RequestHandler& request_handler, std::ostream& output){

        const auto& buses = request_handler.GetBusMapAscendingName();

        /*SphereProjector projector(request_handler.GetMinCoordinates(), 
                                  request_handler.GetMaxCoordinates(), 
                                  settings_.width, 
                                  settings_.height, 
                                  settings_.padding);*/
        const auto coordinates = GetUsedCoordinates(buses);

        SphereProjector projector(coordinates.begin(), 
                                  coordinates.end(), 
                                  settings_.width, 
                                  settings_.height, 
                                  settings_.padding);

        svg::Document document;

        int total_colors = settings_.color_palette.size();
        int i = 0;

        for(const auto& bus : buses){
            if(bus.second->GetStopsCount() != 0){
                DrawRoute(*bus.second, document, projector, settings_.color_palette[i++]);
                if(i == total_colors)
                    i = 0;
            }
        }

        document.Render(output);

    }

    std::vector<Geo::Coordinates> MapRenderer::GetUsedCoordinates(const std::map<std::string_view, const TC::Bus *> &buses) const {
        std::vector<Geo::Coordinates> result;
        for(const auto& bus : buses){
            for(const auto& stop : bus.second->GetStops()){
                result.push_back(stop->getCoordinates());
            }
        }
        return result;
    }

    void MapRenderer::DrawRoute(const Bus& bus, svg::Document& document, SphereProjector& projector, svg::Color color){
        
        svg::Polyline route;

        route.SetStrokeWidth(settings_.line_width);
        route.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        route.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        route.SetStrokeColor(color);

        for(const auto& stop : bus.GetStops()){
            route.AddPoint(projector(stop->getCoordinates()));      
        }

        if(!bus.IsCircular()){
            auto rbegin = bus.GetStops().rbegin();
            auto rend = bus.GetStops().rend();
            if(rbegin != rend) rbegin++;
            while(rbegin != rend){
                route.AddPoint(projector((*rbegin++)->getCoordinates()));    
            }
        }

        document.Add(route);
    }

    namespace detail {
        bool IsZero(double value) {
            return std::abs(value) < EPSILON;
        }
    } // namespace detail
} // namespace TC