#include "json_reader.h"
#include <sstream>

void InputReader::FormCatalogue(std::istream& input, transport_catalogue::TransportCatalogue& catalogue, map_render::RenderSettings* settings) {
    ReadJson(input);
    ParseJsonInputRequests();
    ParseJsonRenderSettings(settings);
    ParseJsonRouterSettings(catalogue);
    ApplyCommands(catalogue);
}

void InputReader::FormRequsts(std::istream& input, std::queue<std::unique_ptr<RequestDescription>>& requests) {
    (void)input;
    ParseJsonStatRequests(requests);
}

void InputReader::ReadJson(std::istream& input) {
    using namespace json;
    Document doc = Load(input);
    doc_ = std::make_unique<Document>(std::move(doc));
}

void InputReader::ParseJsonInputRequests() {
    using namespace std::literals;
    auto&& base_req = doc_->GetRoot().AsMap().find("base_requests"s);
    if(base_req->second.IsArray()) {
        for(size_t i = 0; i < base_req->second.AsArray().size(); ++i)
        {
            auto& dict = base_req->second.AsArray().at(i);
            if(dict.AsMap().at("type"s).AsString() == "Bus")
            {
                auto command_ptr = std::make_unique<BusReadCommand>();
                command_ptr->name_ = dict.AsMap().at("name"s).AsString();
                command_ptr->is_round_trip = dict.AsMap().at("is_roundtrip").AsBool();
                command_ptr->stops_.reserve(dict.AsMap().at("stops"s).AsArray().size());
                for(auto& stop_node : dict.AsMap().at("stops"s).AsArray())
                {
                    command_ptr->stops_.push_back(stop_node.AsString());
                }
                if(!command_ptr->is_round_trip)
                {
                    command_ptr->stops_.reserve(command_ptr->stops_.size() * 2);
                    for(int i = static_cast<int>(command_ptr->stops_.size()) - 2; i >= 0; --i)
                    {
                        command_ptr->stops_.push_back(command_ptr->stops_.at(i));
                    }
                }
                bus_comands_.emplace_back(std::move(command_ptr));
            }
            else
            {
                auto command_ptr = std::make_unique<StopReadCommand>();
                command_ptr->name_ = dict.AsMap().at("name"s).AsString();
                command_ptr->cor_.lat = dict.AsMap().at("latitude"s).AsDouble();
                command_ptr->cor_.lng = dict.AsMap().at("longitude"s).AsDouble();
                for(auto& [stop, dist] : dict.AsMap().at("road_distances"s).AsMap())
                {
                    command_ptr->distance_to_stop_[stop] = dist.AsDouble();
                }
                stop_comands_.emplace_back(std::move(command_ptr));
            }
        }
    }

}

void InputReader::ParseJsonStatRequests(std::queue<std::unique_ptr<RequestDescription>>& requests) {
    using namespace std::literals;
    auto&& stat_req = doc_->GetRoot().AsMap().find("stat_requests"s);
    if(stat_req->second.IsArray()) {
        for(size_t i = 0; i < stat_req->second.AsArray().size(); ++i)
        {
            auto& req_dict = stat_req->second.AsArray().at(i);
            if(req_dict.AsMap().at("type").AsString() == "Map"s)
            {
                auto req_des = std::make_unique<RequestDescription>();
                req_des->id_ = req_dict.AsMap().at("id"s).AsInt();
                req_des->type_ = req_dict.AsMap().at("type"s).AsString();
                requests.push(std::move(req_des));
            }
            else if (req_dict.AsMap().at("type").AsString() == "Bus"){
                auto req_des = std::make_unique<BusRequestDescription>();
                req_des->id_ = req_dict.AsMap().at("id"s).AsInt();
                req_des->type_ = req_dict.AsMap().at("type"s).AsString();
                req_des->name_ = req_dict.AsMap().at("name"s).AsString();
                requests.push(std::move(req_des));
            }
            else if (req_dict.AsMap().at("type").AsString() == "Stop"){
                auto req_des = std::make_unique<StopRequestDescription>();
                req_des->id_ = req_dict.AsMap().at("id"s).AsInt();
                req_des->type_ = req_dict.AsMap().at("type"s).AsString();
                req_des->name_ = req_dict.AsMap().at("name"s).AsString();
                requests.push(std::move(req_des));
            }
            else if (req_dict.AsMap().at("type").AsString() == "Route"){
                auto req_des = std::make_unique<RouteRequestDescription>();
                req_des->id_ = req_dict.AsMap().at("id"s).AsInt();
                req_des->type_ = req_dict.AsMap().at("type"s).AsString();
                req_des->from = req_dict.AsMap().at("from"s).AsString();
                req_des->to = req_dict.AsMap().at("to"s).AsString();
                requests.push(std::move(req_des));
            }
        }
    }
}

svg::Color ConvertNodeToColor(const json::Node& node)
{
    using namespace std::literals;
    if(node.IsString())
    {
        return (node.AsString());
    }
    else if(node.AsArray().size() == 4)
    {
        svg::Rgba color {
                            static_cast<uint8_t>(node.AsArray().at(0).AsInt()),
                            static_cast<uint8_t>(node.AsArray().at(1).AsInt()),
                            static_cast<uint8_t>(node.AsArray().at(2).AsInt()),
                            (node.AsArray().at(3).AsDouble())
                        };
        return (color);
    }
    else 
    {
        svg::Rgb color {
            static_cast<uint8_t>(node.AsArray().at(0).AsInt()),
            static_cast<uint8_t>(node.AsArray().at(1).AsInt()),
            static_cast<uint8_t>(node.AsArray().at(2).AsInt()),
            };
        return (color);
    }
}

void InputReader::ParseJsonRenderSettings(map_render::RenderSettings* settings) {
    using namespace std::literals;
    auto&& render_set = doc_->GetRoot().AsMap().find("render_settings"s);
    if(render_set->second.IsMap()) {
        auto& req_dict = render_set->second;
        settings->width_ = req_dict.AsMap().at("width"s).AsDouble();
        settings->height_ = req_dict.AsMap().at("height"s).AsDouble();
        settings->padding_ = req_dict.AsMap().at("padding"s).AsDouble();
        settings->line_width_ = req_dict.AsMap().at("line_width"s).AsDouble();
        settings->stop_radius_ = req_dict.AsMap().at("stop_radius"s).AsDouble();
        settings->bus_label_font_size_ = req_dict.AsMap().at("bus_label_font_size"s).AsInt();
        for(auto label_offset : req_dict.AsMap().at("bus_label_offset").AsArray())
        {
            settings->bus_label_offset_.push_back(label_offset.AsDouble());
        }
        settings->stop_label_font_size_ = req_dict.AsMap().at("stop_label_font_size"s).AsInt();
        for(auto label_offset : req_dict.AsMap().at("stop_label_offset").AsArray())
        {
            settings->stop_label_offset_.push_back(label_offset.AsDouble());
        }

        settings->underlayer_color_ = ConvertNodeToColor(req_dict.AsMap().at("underlayer_color"s));
        settings->underlayer_width_ = req_dict.AsMap().at("underlayer_width"s).AsDouble();

        settings->color_palette_.reserve(req_dict.AsMap().at("color_palette"s).AsArray().size());
        for(size_t i = 0; i < req_dict.AsMap().at("color_palette"s).AsArray().size(); ++i)
        {
            auto& colo_js = req_dict.AsMap().at("color_palette"s).AsArray().at(i);
            settings->color_palette_.push_back(ConvertNodeToColor(colo_js));
        }
    }
}

void InputReader::ParseJsonRouterSettings(transport_catalogue::TransportCatalogue& catalogue)
{
    using namespace std::literals;
    auto&& render_set = doc_->GetRoot().AsMap().find("routing_settings"s);
    if(render_set->second.IsMap()) {
        auto& req_dict = render_set->second;
        int bus_wait_time = req_dict.AsMap().at("bus_wait_time"s).AsInt();
        double bus_velocity = req_dict.AsMap().at("bus_velocity"s).AsDouble();
        catalogue.SetRouteSettings(bus_wait_time, bus_velocity);
    }
}

void InputReader::ApplyCommands(transport_catalogue::TransportCatalogue& catalogue) const {
    using namespace transport_catalogue;
    for(auto& command : stop_comands_)
    {
        Stop bus_stop = {static_cast<StopReadCommand*>(command.get())->name_, static_cast<StopReadCommand*>(command.get())->cor_};
        catalogue.AddStop(bus_stop);
    }
    for(auto& command : stop_comands_)
    {
        for(auto& [end_stop, distance] : static_cast<StopReadCommand*>(command.get())->distance_to_stop_)
        catalogue.SetDistance(static_cast<StopReadCommand*>(command.get())->name_, end_stop, distance);
    }
    for(auto& bus_comand : bus_comands_)
    {
        catalogue.AddRoute(static_cast<BusReadCommand*>(bus_comand.get())->name_, 
                                                static_cast<BusReadCommand*>(bus_comand.get())->stops_.begin(), 
                                                static_cast<BusReadCommand*>(bus_comand.get())->stops_.end(),
                                                static_cast<BusReadCommand*>(bus_comand.get())->is_round_trip);
    }
    catalogue.FormGraph();

}

void StatAnswer::HandleRequests(std::ostream& output, std::queue<std::unique_ptr<RequestDescription>>& requests, 
                                const transport_catalogue::TransportCatalogue& catalogue, map_render::RenderSettings& render_settings) {
    using namespace std::literals;
    using namespace json;
    builder_.StartArray();
    arr_.reserve(requests.size());
    while(!requests.empty())
    {
        if(requests.front()->type_ == "Bus"s)
        {
            auto req_ptr = dynamic_cast<BusRequestDescription*>(requests.front().get());
            std::optional<RouteInfo> info = catalogue.GetInfoAboutRoute(req_ptr->name_);
            if(!info)
            {
                AnswerError ans_error("not found"s);
                ans_error.request_id_ = requests.front()->id_;
                AddAnswerToArr(&ans_error);
            }
            else
            {
                AnswerBus ans_bus;
                ans_bus.rourte_info_ = info.value();
                ans_bus.request_id_ =requests.front()->id_;
                AddAnswerToArr(&ans_bus);
            }
        }
        else if(requests.front()->type_ == "Map")
        {
            AnswerMap ans_map;
            map_render::Render render;
            std::stringstream svg_str;
            render.DrawMap(svg_str, catalogue.GetAllRoutes(), &render_settings);
            ans_map.request_id_ = requests.front()->id_;
            ans_map.svg_ = svg_str.str();
            AddAnswerToArr(&ans_map);
        }
        else if(requests.front()->type_ == "Stop"s)
        {
            auto req_ptr = dynamic_cast<StopRequestDescription*>(requests.front().get());
            std::optional<StopInfo> info = catalogue.GetInfoAboutBusesViaStop(req_ptr->name_);
            if(!info)
            {
                AnswerError ans_error("not found"s);
                ans_error.request_id_ = requests.front()->id_;
                AddAnswerToArr(&ans_error);
            }
            else
            {
                AnswerStop ans_stop;
                ans_stop.stop_info_ = info.value();
                ans_stop.request_id_ =requests.front()->id_;
                AddAnswerToArr(&ans_stop);
            }
        }
        else if(requests.front()->type_ == "Route"s)
        {
            auto req_ptr = dynamic_cast<RouteRequestDescription*>(requests.front().get());
            auto router_info = catalogue.BuildRoute(req_ptr->from, req_ptr->to);
            if(!router_info)
            {
                AnswerError ans_error("not found"s);
                ans_error.request_id_ = requests.front()->id_;
                AddAnswerToArr(&ans_error);
            }
            else
            {
                AnswerRoute ans_route;
                ans_route.request_id_ = requests.front()->id_;
                ans_route.total_time_ = router_info.value().weight;
                if(!router_info.value().edges.empty())
                {
                    for(size_t i = 0; i < router_info.value().edges.size(); ++i)
                    {
                        if(catalogue.GetEdgeType(router_info.value().edges.at(i))->type_ == "wait")
                        {
                            WaitRouteItem route_item;
                            auto wait_edge = *static_cast<const transport_catalogue::WaitEdgeType*>(catalogue.GetEdgeType(router_info.value().edges.at(i)));
                            route_item.stop_name_ = wait_edge.stop_name_;
                            route_item.time_ = catalogue.GetRouteSettings().bus_wait_time_;
                            ans_route.items_.push_back(std::make_unique<WaitRouteItem>(route_item));
                        }
                        else
                        {
                            BusRouteItem route_item;
                            auto bus_edge = *static_cast<const transport_catalogue::BusEdgeType*>(catalogue.GetEdgeType(router_info.value().edges.at(i)));
                            route_item.bus_ = bus_edge.bus_name_;
                            route_item.span_count_ = bus_edge.span_count_;
                            route_item.time_ = bus_edge.time_;
                            ans_route.items_.push_back(std::make_unique<BusRouteItem>(route_item));
                        }
                    }
                }
                AddAnswerToArr(&ans_route);
            }
        }
        requests.pop();
    }
    builder_.EndArray();
    json::Print(json::Document{builder_.Build()}, output);
}

void StatAnswer::AddAnswerToArr(AnswerDescription* answer) {
    using namespace std::literals;
    using namespace json;
    if(answer->type_ == "Bus"s)
    {
        builder_.StartDict()
                .Key("curvature"s).Value(static_cast<AnswerBus*>(answer)->rourte_info_.curvature)
                .Key("request_id"s).Value(answer->request_id_)
                .Key("route_length"s).Value(static_cast<AnswerBus*>(answer)->rourte_info_.route_length_)
                .Key("stop_count"s).Value(static_cast<int>(static_cast<AnswerBus*>(answer)->rourte_info_.number_of_stops_))
                .Key("unique_stop_count"s).Value(static_cast<int>(static_cast<AnswerBus*>(answer)->rourte_info_.number_of_uniq_stops_))
                .EndDict();
    }
    else if (answer->type_ == "Stop"s)
    {
        builder_.StartDict()
                .Key("buses"s).StartArray();
        for(auto name_ : static_cast<AnswerStop*>(answer)->stop_info_.route_names_)
        {
            builder_.Value(std::string(name_));
        }
        builder_.EndArray()
                .Key("request_id"s).Value(answer->request_id_)
                .EndDict();
    }
    else if (answer->type_ == "Map"s)
    {
        builder_.StartDict()
                .Key("map"s).Value(static_cast<AnswerMap*>(answer)->svg_)
                .Key("request_id").Value(answer->request_id_)
                .EndDict();
    }
    else if (answer->type_ == "Route")
    {
        builder_.StartDict()
                .Key("items").StartArray();
        auto answ = static_cast<AnswerRoute*>(answer);
        for(size_t i = 0; i < answ->items_.size(); ++i)
        {
            builder_.StartDict();
            if(answ->items_.at(i)->type_ == "Wait")
            {
                auto wait_route_item = static_cast<WaitRouteItem*>(answ->items_.at(i).get());
                builder_.Key("stop_name"s).Value(wait_route_item->stop_name_)
                        .Key("time"s).Value(wait_route_item->time_)
                        .Key("type"s).Value("Wait"s);
            }
            else
            {
                auto bus_route_item = static_cast<BusRouteItem*>(answ->items_.at(i).get());
                builder_.Key("bus"s).Value(bus_route_item->bus_)
                        .Key("span_count"s).Value(bus_route_item->span_count_)
                        .Key("time"s).Value(bus_route_item->time_)
                        .Key("type"s).Value("Bus"s);
            }
            builder_.EndDict();
        }
        builder_.EndArray();
        builder_.Key("request_id"s).Value(answ->request_id_)
                .Key("total_time"s).Value(answ->total_time_)
                .EndDict();
    }
    else
    {
        builder_.StartDict()
                .Key("request_id"s).Value(answer->request_id_)
                .Key("error_message"s).Value(static_cast<AnswerError*>(answer)->error_message_)
                .EndDict();
    }
}