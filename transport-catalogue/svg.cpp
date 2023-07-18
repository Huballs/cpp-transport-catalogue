#include "svg.h"

namespace svg {

using namespace std::literals;

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
    out << "r=\""sv << radius_ << "\""sv;
    RenderAttrs(out);
    out << "/>"sv;
}

// ---------- Polyline -----------------

Polyline& Polyline::AddPoint(Point point){
    points_.push_back(point);
    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;

    out << "<polyline points=\""sv;

    bool isFirst = true;

    for(const auto& point : points_){
        out << (isFirst ? "" : " ");
        isFirst = false;
        out << point.x << ',' << point.y;
    }
    
    out << "\"";

    RenderAttrs(out);

    out << "/>";
}

// -------------- Text ----------------

Text& Text::SetPosition(Point pos){
    pos_ = pos;
    return *this;
}

Text& Text::SetOffset(Point offset){
    offset_ = offset;
    return *this;
}

Text& Text::SetFontSize(uint32_t size){
    font_size_ = size;
    return *this;
}

Text& Text::SetFontFamily(std::string font_family){
    font_family_ = std::move(font_family);
    return *this;
}

Text& Text::SetFontWeight(std::string font_weight){
    font_weight_ = std::move(font_weight);
    return *this;
}

Text& Text::SetData(std::string data){
    data_ = std::move(data);
    return *this;
}


void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;

    out << "<text";

    RenderAttrs(out);

    out << " x=\"" << pos_.x << "\" y=\"" << pos_.y << "\"";
    out << " dx=\"" << offset_.x << "\" dy=\"" << offset_.y << "\"";
    out << " font-size=\"" << font_size_ << "\"";
    if(!font_weight_.empty())
        out << " font-weight=\"" << font_weight_ << "\"";
    if(!font_family_.empty())
        out << " font-family=\"" << font_family_ << "\"";
    
    out << ">";

    for (char c : data_){

        switch(c){
            case '&':
                out << "&amp;"sv;
                break;
            case '"':
                out <<  "&quot;";
                break;
            case '\'':
                out <<  "&apos;";
                break;
            case '<':
                out <<  "&lt;";
                break;
            case '>':
                out <<  "&gt;";
                break;
            default:
                out << c;
        }
    }

    out << "</text>";
}
//ObjectContainer::ObjectContainer(){};
//ObjectContainer::~ObjectContainer(){};

// -------------- Document ---------------------

// Добавляет в svg-документ объект-наследник svg::Object
void Document::AddPtr(std::unique_ptr<Object>&& obj){
    objects_.push_back(std::move(obj));
}

// Выводит в ostream svg-представление документа
void Document::Render(std::ostream& out) const{
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>" << std::endl;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">" << std::endl;

    for(const auto& obj : objects_){
        obj->Render(out);
        
    }

    out << "</svg>";
}

std::ostream& operator<<(std::ostream& stream, const StrokeLineCap& line_cap){
    using namespace std::literals;
    switch(line_cap){
        case StrokeLineCap::BUTT:
            stream << "butt"sv;
            break;
        case StrokeLineCap::ROUND:
            stream << "round"sv;
            break;
        case StrokeLineCap::SQUARE:
            stream << "square"sv;
            break;
    }
    return stream;
}

std::ostream& operator<<(std::ostream& stream, const StrokeLineJoin& line_join){
    using namespace std::literals;
    switch(line_join){
        case StrokeLineJoin::ROUND:
            stream << "round"sv;
            break;
        case StrokeLineJoin::ARCS:
            stream << "arcs"sv;
            break;
        case StrokeLineJoin::BEVEL:
            stream << "bevel"sv;
            break;
        case StrokeLineJoin::MITER:
            stream << "miter"sv;
            break;
        case StrokeLineJoin::MITER_CLIP:
            stream << "miter-clip"sv;
            break;
    }
    return stream;
}


std::ostream& operator<<(std::ostream& stream, const Color& color){

    if (std::holds_alternative<std::monostate>(color)) {
        stream << "none";
    } else
    if (std::holds_alternative<Rgb>(color)) {
        const auto& rgb = std::get<Rgb>(color);

        stream << "rgb("    // uint8_t aka char to int
            << static_cast<int>(rgb.red) << ','
            << static_cast<int>(rgb.green) << ','
            << static_cast<int>(rgb.blue)
            << ')';
    } else
    if (std::holds_alternative<Rgba>(color)) {
        const auto& rgba = std::get<Rgba>(color);
        stream << "rgba("
            << static_cast<int>(rgba.red) << ','
            << static_cast<int>(rgba.green) << ','
            << static_cast<int>(rgba.blue) << ','
            << rgba.opacity
            << ')';
    } else
    if (std::holds_alternative<std::string>(color)) {
        stream << std::get<std::string>(color);
    }
    
    return stream;
}

}  // namespace svg

