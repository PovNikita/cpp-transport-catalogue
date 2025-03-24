#include "json_reader.h"
#include <sstream>

void InputReader::FormCatalogue(std::istream& input, transport_catalogue::TransportCatalogue& catalogue, map_render::RenderSettings* settings) {
    ReadJson(input);
    ParseJsonInputRequests();
    ParseJsonRenderSettings(settings);
    ApplyCommands(catalogue);
}

void InputReader::FormRequsts(std::istream& input, std::queue<RequestDescription>& requests) {
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

void InputReader::ParseJsonStatRequests(std::queue<RequestDescription>& requests) {
    using namespace std::literals;
    auto&& stat_req = doc_->GetRoot().AsMap().find("stat_requests"s);
    if(stat_req->second.IsArray()) {
        for(size_t i = 0; i < stat_req->second.AsArray().size(); ++i)
        {
            auto& req_dict = stat_req->second.AsArray().at(i);
            RequestDescription req_des;
            if(req_dict.AsMap().at("type").AsString() == "Map"s)
            {
                req_des.id_ = req_dict.AsMap().at("id"s).AsInt();
                req_des.type_ = req_dict.AsMap().at("type"s).AsString();
                requests.push(std::move(req_des));
            }
            else {
                req_des.id_ = req_dict.AsMap().at("id"s).AsInt();
                req_des.type_ = req_dict.AsMap().at("type"s).AsString();
                req_des.name_ = req_dict.AsMap().at("name"s).AsString();
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

void InputReader::ApplyCommands(transport_catalogue::TransportCatalogue& catalogue) const {
    using namespace transport_catalogue;
    for(auto& command : stop_comands_)
    {
        Stop bus_stop = {command->name_, static_cast<StopReadCommand*>(command.get())->cor_};
        catalogue.AddStop(bus_stop);
    }
    for(auto& command : stop_comands_)
    {
        for(auto& [end_stop, distance] : static_cast<StopReadCommand*>(command.get())->distance_to_stop_)
        catalogue.SetDistance(command->name_, end_stop, distance);
    }
    for(auto& bus_comand : bus_comands_)
    {
        catalogue.AddRoute(bus_comand->name_, static_cast<BusReadCommand*>(bus_comand.get())->stops_.begin(), 
                                                static_cast<BusReadCommand*>(bus_comand.get())->stops_.end(),
                                                static_cast<BusReadCommand*>(bus_comand.get())->is_round_trip);
    }
}

void StatAnswer::HandleRequests(std::ostream& output, std::queue<RequestDescription>& requsts, const transport_catalogue::TransportCatalogue& catalogue, map_render::RenderSettings& render_settings) {
    using namespace std::literals;
    using namespace json;
    builder_.StartArray();
    arr_.reserve(requsts.size());
    while(!requsts.empty())
    {
        if(requsts.front().type_ == "Bus"s)
        {
            std::optional<RouteInfo> info = catalogue.GetInfoAboutRoute(requsts.front().name_);
            if(!info)
            {
                AnswerError ans_error("not found"s);
                ans_error.request_id_ = requsts.front().id_;
                AddAnswerToArr(&ans_error);
            }
            else
            {
                AnswerBus ans_bus;
                ans_bus.rourte_info_ = info.value();
                ans_bus.request_id_ =requsts.front().id_;
                AddAnswerToArr(&ans_bus);
            }
        }
        else if(requsts.front().type_ == "Map")
        {
            AnswerMap ans_map;
            map_render::Render render;
            std::stringstream svg_str;
            render.DrawMap(svg_str, catalogue.GetAllRoutes(), &render_settings);
            ans_map.request_id_ = requsts.front().id_;
            ans_map.svg_ = svg_str.str();
            AddAnswerToArr(&ans_map);
        }
        else
        {
            std::optional<StopInfo> info = catalogue.GetInfoAboutBusesViaStop(requsts.front().name_);
            if(!info)
            {
                AnswerError ans_error("not found"s);
                ans_error.request_id_ = requsts.front().id_;
                AddAnswerToArr(&ans_error);
            }
            else
            {
                AnswerStop ans_stop;
                ans_stop.stop_info_ = info.value();
                ans_stop.request_id_ =requsts.front().id_;
                AddAnswerToArr(&ans_stop);
            }
        }
        requsts.pop();
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
    else
    {
        builder_.StartDict()
                .Key("request_id"s).Value(answer->request_id_)
                .Key("error_message"s).Value(static_cast<AnswerError*>(answer)->error_message_)
                .EndDict();
    }
}