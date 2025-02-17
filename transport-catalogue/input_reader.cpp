#include "input_reader.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <queue>
#include <utility>

using namespace transport_catalogue;

/**
 * Парсит строку вида "10.123,  -30.1837" и возвращает пару координат (широта, долгота)
 */
Coordinates ParseCoordinates(std::string_view str) {
    static const double nan = std::nan("");

    auto not_space = str.find_first_not_of(' ');
    auto comma = str.find(',');

    if (comma == str.npos) {
        return {nan, nan};
    }

    auto not_space2 = str.find_first_not_of(' ', comma + 1);

    double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
    double lng = std::stod(std::string(str.substr(not_space2)));

    return {lat, lng};
}

/**
 * Удаляет пробелы в начале и конце строки
 */
std::string_view Trim(std::string_view string) {
    const auto start = string.find_first_not_of(' ');
    if (start == string.npos) {
        return {};
    }
    return string.substr(start, string.find_last_not_of(' ') + 1 - start);
}

/**
 * Разбивает строку string на n строк, с помощью указанного символа-разделителя delim
 */
std::vector<std::string_view> Split(std::string_view string, char delim) {
    std::vector<std::string_view> result;

    size_t pos = 0;
    while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
        auto delim_pos = string.find(delim, pos);
        if (delim_pos == string.npos) {
            delim_pos = string.size();
        }
        if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
            result.push_back(substr);
        }
        pos = delim_pos + 1;
    }

    return result;
}

/**
 * Парсит маршрут.
 * Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
 * Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]
 */
std::vector<std::string_view> ParseRoute(std::string_view route) {
    if (route.find('>') != route.npos) {
        return Split(route, '>');
    }

    auto stops = Split(route, '-');
    std::vector<std::string_view> results(stops.begin(), stops.end());
    results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

    return results;
}

CommandDescription ParseCommandDescription(std::string_view line) {
    auto colon_pos = line.find(':');
    if (colon_pos == line.npos) {
        return {};
    }

    auto space_pos = line.find(' ');
    if (space_pos >= colon_pos) {
        return {};
    }

    auto not_space = line.find_first_not_of(' ', space_pos);
    if (not_space >= colon_pos) {
        return {};
    }

    return {std::string(line.substr(0, space_pos)),
            std::string(line.substr(not_space, colon_pos - not_space)),
            std::string(line.substr(colon_pos + 1))};
}

void InputReader::ParseLine(std::string_view line) {
    auto command_description = ParseCommandDescription(line);
    if (command_description) {
        commands_.push_back(std::move(command_description));
    }
}

void InputReader::ApplyCommands(TransportCatalogue& catalogue) {
    using namespace std;
    queue<std::pair<std::string_view, std::unordered_map<std::string, double>>> distance_to_stop;
    queue<CommandDescription*> routes;
    for(auto &command : commands_)
    {
        if(command.command == "Stop"s)
        {
            StopRequest req;
            if(command.description.find("to") != command.description.npos)
            {
                req = ParseStopRequest(command.description);
                distance_to_stop.push({command.id, std::move(req.distance_to_stop_)});
            }
            else{
                req.cor_ = GetCoordinatesFromString(command.description);
            }
            Stop bus_stop = {command.id, req.cor_};
            catalogue.AddStop(bus_stop);
        }
        else if(command.command == "Bus"s)
        {
            routes.push(&command);
        }
    }
    while(!distance_to_stop.empty())
    {
        for(auto [end_stop, distance] : distance_to_stop.front().second)
        {
            catalogue.SetDistance(distance_to_stop.front().first, end_stop, distance);
        }
        distance_to_stop.pop();
    }
    while(!routes.empty())
    {
        CommandDescription command = *routes.front();
        vector<string_view> stop_names = ParseBusDescription(command.description);
        catalogue.AddRoute(command.id, stop_names.begin(), stop_names.end());
        routes.pop();
    }
}

StopRequest ParseStopRequest(const std::string &request)
{
    using namespace std::literals;
    Coordinates cor;
    size_t prev_pos = request.find(',');
    cor.lat = std::stod(std::string(request.begin(), request.begin() + prev_pos));
    size_t pos = request.find(',', prev_pos + 1);
    cor.lng = std::stod(std::string(request.begin() + prev_pos + 1, request.begin() + pos));
    std::unordered_map<std::string, double> distance_to_stop;
    prev_pos = pos;
    pos = request.find(',', prev_pos + 1);

    size_t m_pos = 0;
    size_t to_pos = 0;
    double distance = 0.0;
    std::string stop_name = ""s;

    while(pos != request.npos)
    {
        m_pos = request.find('m', prev_pos + 1);
        to_pos = request.find("to", m_pos);
        distance = std::stod(std::string(request.begin() + prev_pos + 1, request.begin() + m_pos));
        stop_name = std::string(request.begin() + to_pos + 3, request.begin() + pos);
        distance_to_stop[stop_name] = distance;
        prev_pos = pos;
        pos = request.find(',', prev_pos + 1);
    }

    m_pos = request.find('m', prev_pos + 1);
    to_pos = request.find("to", m_pos);
    distance = std::stod(std::string(request.begin() + prev_pos + 1, request.begin() + m_pos));
    stop_name = std::string(request.begin() + to_pos + 3, request.end());
    distance_to_stop[stop_name] = distance;

    return {cor, std::move(distance_to_stop)};
}

Coordinates GetCoordinatesFromString(const std::string &string)
{
    Coordinates cor;
    auto pos = string.find(',');
    cor.lat = std::stod(std::string(string.begin(), string.begin() + pos));
    cor.lng = std::stod(std::string(string.begin() + pos + 1, string.end()));
    return cor;
}

std::vector<std::string_view> ParseBusDescription(const std::string &description)
{
    std::vector<std::string_view> stop_names;
    bool is_circle_route = false;
    size_t sign_pos = description.find_first_of("->");
    if(sign_pos != description.npos && (description.at(sign_pos) == '-'))
    {
        is_circle_route = true;
    }
    size_t first_letter_pos = 0;
    size_t last_letter_pos = 0;
    while(first_letter_pos < description.size())
    {        
        first_letter_pos = description.find_first_not_of(' ', first_letter_pos);
        last_letter_pos = (sign_pos == description.npos) ? description.size() : sign_pos - 1;
        stop_names.emplace_back(description.data() + first_letter_pos, last_letter_pos - first_letter_pos);
        first_letter_pos = (sign_pos == description.npos) ? description.size() : sign_pos + 1;
        sign_pos = description.find_first_of("->", sign_pos + 1);
    }

    if(is_circle_route)
    {
        stop_names.reserve(stop_names.size() * 2);
        for(int i = static_cast<int>(stop_names.size()) - 2; i >= 0; --i)
        {
            stop_names.push_back(stop_names.at(i));
        }
    }
    return stop_names;
}