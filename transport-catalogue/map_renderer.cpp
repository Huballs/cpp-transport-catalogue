#include "map_renderer.h"

namespace TC::Renderer {

    void  MapRenderer::Render(const RequestHandler& request_handler, std::ostream& output){

        const auto& buses = request_handler.GetBusMapAscendingName();

        const auto coordinates = GetUsedCoordinates(buses);

        SphereProjector projector(coordinates.begin(), 
                                  coordinates.end(), 
                                  settings_.width, 
                                  settings_.height, 
                                  settings_.padding);

        svg::Document document;

        // bus route lines
        int total_colors = settings_.color_palette.size();
        int i = 0;

        for(const auto& bus : buses){
            if(bus.second->GetStopsCount()){
                DrawRoute(*bus.second, document, projector, settings_.color_palette[i++]);
                if(i == total_colors)
                    i = 0;
            }
        }

        // bus route names
        i = 0;

        for(const auto& bus : buses){
            if(bus.second->GetStopsCount()){
                DrawRouteName(*bus.second, document, projector, settings_.color_palette[i++]);
                if(i == total_colors)
                    i = 0;
            }
        }

        // stops
        const auto & stops = request_handler.GetStopMapAscendingName();

        for(const auto& stop : stops){
            DrawStopCircles(*stop.second, document, projector);
        }

        for(const auto& stop : stops){
            DrawStopName(*stop.second, document, projector);
        }

        document.Render(output);

    }

    void MapRenderer::DrawStopName(const Stop& stop, svg::Document& document, SphereProjector& projector){
        
        if(!stop.GetBusCount())
            return;

        svg::Text text_foreground;
        text_foreground.SetFillColor("black");
        text_foreground.SetOffset(settings_.stop_label_offset);
        text_foreground.SetFontSize(settings_.stop_label_font_size);
        text_foreground.SetFontFamily("Verdana");
        text_foreground.SetData(std::string(stop.GetName()));

        svg::Text text_background(text_foreground);
        text_background.SetFillColor(settings_.underlayer_color);
        text_background.SetStrokeColor(settings_.underlayer_color);
        text_background.SetStrokeWidth(settings_.underlayer_width);
        text_background.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        text_background.SetStrokeLineCap(svg::StrokeLineCap::ROUND);

        svg::Point pos = projector(stop.getCoordinates());
        text_foreground.SetPosition(pos);
        text_background.SetPosition(pos);
        
        document.Add(text_background);
        document.Add(text_foreground);
    }

    void MapRenderer::DrawStopCircles(const Stop& stop, svg::Document& document, SphereProjector& projector){
        
        if(!stop.GetBusCount())
            return;

        svg::Circle circle;
        circle.SetCenter(projector(stop.getCoordinates()));
        circle.SetRadius(settings_.stop_radius);
        circle.SetFillColor("white");

        document.Add(circle);
    }

    void MapRenderer::DrawRouteName(const Bus& bus, svg::Document& document, SphereProjector& projector, svg::Color color){

        if(!bus.GetStopsCount())
            return;

        svg::Text text_foreground;
        text_foreground.SetFillColor(color);
        text_foreground.SetOffset(settings_.bus_label_offset);
        text_foreground.SetFontSize(settings_.bus_label_font_size);
        text_foreground.SetFontFamily("Verdana");
        text_foreground.SetFontWeight("bold");
        text_foreground.SetData(std::string(bus.GetName()));

        svg::Text text_background(text_foreground);
        text_background.SetFillColor(settings_.underlayer_color);
        text_background.SetStrokeColor(settings_.underlayer_color);
        text_background.SetStrokeWidth(settings_.underlayer_width);
        text_background.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        text_background.SetStrokeLineCap(svg::StrokeLineCap::ROUND);

        svg::Point pos = projector(bus.GetStops().front()->getCoordinates());
        text_foreground.SetPosition(pos);
        text_background.SetPosition(pos);
        
        document.Add(text_background);
        document.Add(text_foreground);
        
        if (bus.GetStops().front() != bus.GetStops().back()) {
            pos = projector(bus.GetStops().back()->getCoordinates());
            text_foreground.SetPosition(pos);
            text_background.SetPosition(pos);
            document.Add(text_background);
            document.Add(text_foreground);
        }
    }

    void MapRenderer::DrawRoute(const Bus& bus, svg::Document& document, SphereProjector& projector, svg::Color color){
        
        if(!bus.GetStopsCount())
            return;

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

    std::vector<Geo::Coordinates> MapRenderer::GetUsedCoordinates(const std::map<std::string_view, const TC::Bus *> &buses) const {
        std::vector<Geo::Coordinates> result;
        for(const auto& bus : buses){
            for(const auto& stop : bus.second->GetStops()){
                result.push_back(stop->getCoordinates());
            }
        }
        return result;
    }

    namespace detail {
        bool IsZero(double value) {
            return std::abs(value) < EPSILON;
        }
    } // namespace detail
} // namespace TC