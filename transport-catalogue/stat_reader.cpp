#include "stat_reader.h"
#include "input_reader.h"
#include <iomanip> /* for setprecision(6) */

namespace TC::Output {

        void StatReader(TransportCatalogue& catalogue, std::istream& input, std::ostream& output){
            
            using namespace std::string_view_literals;

            int n_queries = 0;

            input >> n_queries;
            std::string line;
            std::getline(input,line);

            for(int i = 0; i < n_queries; ++i){
                std::getline(input,line);

                auto command = Input::detail::SplitCommand(line);
                
                if(command.first == "Bus"sv){
                    if(!catalogue.ContainsBus(command.second)){
                        output << "Bus " << command.second << ": not found" << std::endl;
                        continue;
                    }
                    auto *bus = catalogue.GetBus(command.second);

                    output << std::fixed << std::showpoint;
                    output << std::setprecision(6);

                    output << "Bus " << command.second << ": "
                            << bus->GetStopsCount() << " stops on route, "
                            << bus->GetStopsCountUnique() << " unique stops, "
                            << bus->GetLengthTraveled() <<  " route length, "
                            << bus->GetCurvature() << " curvature"
                            << std::endl;

                } else if(command.first == "Stop"sv){

                    output << "Stop " << command.second << ": ";

                if(!catalogue.ContainsStop(command.second)){
                        output << "not found" << std::endl;
                        continue;
                    } 
                    auto *stop = catalogue.GetStop(command.second);
                    if(stop->GetBuses().empty()){
                        output << "no buses" << std::endl;
                        continue;
                    }
                    output << "buses";
                    for(const auto bus : stop->GetBuses()){
                        output << ' ' << bus;
                        
                    }
                    output << std::endl;
                }
            }
        }
} // namespace TC::Output