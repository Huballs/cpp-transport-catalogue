#include "input_reader.h"
#include <string>
#include <vector>
#include <cstdlib>
#include <iostream>
#include <unordered_map>

namespace TC {

    namespace Input {

        namespace detail {

            std::unordered_map<std::string_view, uint32_t> SplitDistances(std::string_view line){
                
                size_t comma = line.find_first_of(',', line.find_first_of(',')+1);

                std::unordered_map<std::string_view, uint32_t> result;

                while(comma != line.npos){
                    size_t name_start = line.find_first_of('t', comma) + 3;
                    uint32_t distance = strtod(line.data()+comma+1, 0);
                    comma = line.find_first_of(',', name_start);
                    result.insert({
                        line.substr(name_start, (comma == line.npos ? comma : comma - name_start)), 
                        distance
                        });
                }

                return result;
            }

            std::pair<std::vector<std::string_view>, bool> SplitBusStops(std::string_view line){

                char delim;
                bool isCircular;
                size_t start = line.find_first_of(':');
                size_t pos = line.find_first_of('-', start);  

                std::vector<std::string_view> result;

                if(pos != line.npos){
                    delim = '-';
                    isCircular = false;
                }
                else {
                    delim = '>';
                    pos = line.find_first_of(delim);
                    isCircular = true;
                }

                do{
                    result.push_back(line.substr(start+2, pos-start-(pos != line.npos ? 3 : 0)));
                    start = pos;
                    pos = line.find_first_of(delim, start+2); 
                }while(start != line.npos);
                
                return {result, isCircular};
            }

            std::pair<double, double> SplitCoordinates(std::string_view line){

                size_t colon = line.find_first_of(':');
                size_t comma = line.find_first_of(',');
                double lat, lon;
                
                lat = strtod(line.data()+colon+2, 0);
                lon = strtod(line.data()+comma+2, 0);

                return {lat, lon};
            }



            std::pair<std::string_view, std::string_view> SplitCommand(std::string_view line){

                size_t colon = line.find_first_of(':');
                size_t space = line.find_first_of(' ');

                return {
                    (line.substr(0, space)), 
                    (line.substr(space+1, colon-space-1))
                    };
            }
        } // namespace detail

        void Reader(TransportCatalogue& catalogue, std::istream& input){

        using namespace std::string_view_literals;
        using namespace detail;
        
        int n_queries;

        input >> n_queries;
        std::string extra_new_line;
        std::getline(input, extra_new_line);

        std::deque<std::string> lines;
        
        std::unordered_map<std::string_view, std::pair<std::vector<std::string_view>, bool>> bus_to_stops;
        std::unordered_map<std::string_view, std::unordered_map<std::string_view, uint32_t>> stop_to_stops_distances;

        for(int i = 0; i < n_queries; ++i){

            lines.push_back({});

            std::getline(input,lines.back());
            auto command = SplitCommand(lines.back());

            if(command.first == "Stop"sv){
                auto coordinates = SplitCoordinates(lines.back());
                auto distances = SplitDistances(lines.back());

                if(!distances.empty())
                    stop_to_stops_distances[command.second] = distances;

                catalogue.AddStop(command.second, {coordinates.first, coordinates.second});

            } else if(command.first == "Bus"sv){
                bus_to_stops[(command.second)] = SplitBusStops(lines.back());
            }
        }

        for(const auto& [stop, stops_distance] : stop_to_stops_distances){
            catalogue.AddDistances(stop, stops_distance);
        }

        for(const auto& [bus, stops] : bus_to_stops){

            std::vector<size_t> indexed_stops;
            indexed_stops.reserve(stops.first.size());

            for(auto stop : stops.first){
                indexed_stops.push_back(catalogue.GetStop(stop)->GetIndex());
            }

            catalogue.AddRoute(bus, indexed_stops, stops.second);
        }

    }

    } // namespace Input
} // namespace TC