#include <cassert>
#include "svg.h"

namespace svg {

using namespace std::literals;

std::ostream& operator<<(std::ostream& out, Color color)
{
    std::visit(PrintColor{out}, color);
    return out;
}

std::ostream& operator<<(std::ostream& out, StrokeLineCap line_cap)
{
    switch (line_cap)
    {
    case StrokeLineCap::BUTT:
        out << "butt"sv;
        break;
    case StrokeLineCap::ROUND:
        out << "round"sv;
        break;
    case StrokeLineCap::SQUARE:
        out << "square"sv;
        break;
    default:
        break;
    }
    return out;
}


std::ostream& operator<<(std::ostream& out, StrokeLineJoin line_cap)
{
    switch (line_cap)
    {
    case StrokeLineJoin::ARCS:
        out << "arcs"sv;
        break;
    case StrokeLineJoin::BEVEL:
        out << "bevel"sv;
        break;
    case StrokeLineJoin::MITER:
        out << "miter"sv;
        break;
    case StrokeLineJoin::MITER_CLIP:
        out << "miter-clip"sv;
        break;
    case StrokeLineJoin::ROUND:
        out << "round"sv;
        break;
    default:
        break;
    }
    return out;
}


void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\" "sv;
    RenderAttrs(out);
    out << "/>"sv;
}

// ---------- Polyline ------------------

Polyline& Polyline::AddPoint(Point point) {
    points_.push_back(point);
    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    if(!points_.empty()) {
        out << "<polyline points=\""sv;
        size_t i = 0;
        out << points_.at(i).x << ","sv << points_.at(i).y;
        ++i;
        for( ;i < points_.size(); ++i)
        {
            out << " "sv << points_.at(i).x << ","sv << points_.at(i).y;
        }
        out << "\" "sv;
        RenderAttrs(out);
        out << "/>"sv;
    }
    else
    {
        out << "<polyline points=\"\" />"sv;
    }
}

// ---------- Text ------------------

    Text& Text::SetPosition(Point pos) {
        position_ = pos;
        return *this;
    }

    Text& Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }

    Text& Text::SetFontSize(uint32_t size) {
        font_size_ = size;
        return *this;
    }

    Text& Text::SetFontFamily(std::string font_family) {
        font_family_ = font_family;
        return *this;
    }

    Text& Text::SetFontWeight(std::string font_weight) {
        font_weight_ = font_weight;
        return *this;
    }

    Text& Text::SetData(std::string data) {
        data_ = data;
        ReplaceAllSubString(std::string("&"s), std::string("&amp;"s), data_);
        ReplaceAllSubString(std::string("\""s), std::string("&quot;"s), data_);
        ReplaceAllSubString(std::string("\'"s), std::string("&apos;"s), data_);
        ReplaceAllSubString(std::string("<"s), std::string("&lt;"s), data_);
        ReplaceAllSubString(std::string(">"s), std::string("&gt;"s), data_);
        return *this;
    }

    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<text "sv; 
        RenderAttrs(out);
        out <<" x=\""sv << position_.x << "\" y=\""sv << position_.y << "\" dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y
        << "\" font-size=\""sv << font_size_ << "\""sv;
        if(!font_family_.empty()) {
            out << " font-family=\""sv << font_family_ << "\""sv;
        }
        if(!font_weight_.empty())
        {
            out << " font-weight=\""sv << font_weight_  << "\""sv;
        }
        out << ">"sv << data_ << "</text>"sv;
    }

    void Text::ReplaceAllSubString(const std::string& sub_string_to_replace, const std::string& sub_string, std::string& input_string)
    {
        if (sub_string_to_replace.empty()) return;

        auto pos = input_string.find(sub_string_to_replace);
        while(pos!= input_string.npos) {
            input_string.replace(pos, sub_string_to_replace.size(), sub_string);
            pos += sub_string.size();
            pos = input_string.find(sub_string_to_replace, pos);
        }
    }

// ---------- Document ------------------

// Добавляет в svg-документ объект-наследник svg::Object
    void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        objects_.push_back(std::move(obj));
    }

// Выводит в ostream svg-представление документа
    void Document::Render(std::ostream& out) const {
        RenderContext ctx(out, 2, 2);
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
        for(size_t i = 0; i < objects_.size(); ++i)
        {
            objects_.at(i)->Render(ctx);
        }
        out << "</svg>"sv;
    }

}  // namespace svg3