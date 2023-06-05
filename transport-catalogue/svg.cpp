#include "svg.h"

namespace svg {

using namespace std::literals;

std::ostream& operator<<(std::ostream& out, const StrokeLineCap stroke_line_cup)
{
    switch (stroke_line_cup)
    {
        case StrokeLineCap::BUTT:
            out << "butt";
            break;
        case StrokeLineCap::ROUND:
            out << "round";
            break;
        case StrokeLineCap::SQUARE:
            out << "square";
            break;
    }

    return out;
}

std::ostream& operator<<(std::ostream& out,
    const StrokeLineJoin stroke_line_join)
{
    switch (stroke_line_join)
    {
        case StrokeLineJoin::ARCS:
            out << "arcs";
            break;
        case StrokeLineJoin::BEVEL:
            out << "bevel";
            break;
        case StrokeLineJoin::MITER:
            out << "miter";
            break;
        case StrokeLineJoin::MITER_CLIP:
            out << "miter-clip";
            break;
        case StrokeLineJoin::ROUND:
            out << "round";
            break;
    }

    return out;
}

std::ostream& operator<<(std::ostream& out, const Color color)
{
    std::string color_str = std::visit(ColorPrinter{}, color);
    out << color_str;

    return out;
}

void Object::Render(const RenderContext& context) const
{
    context.RenderIndent();

    RenderObject(context);

    context.out << std::endl;
}

Circle& Circle::SetCenter(Point center)
{
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)
{
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const
{
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y
        << "\" "sv;
    out << "r=\""sv << radius_ << "\""sv;
    RenderAttrs(context.out);
    out << "/>"sv;
}

Polyline& Polyline::AddPoint(Point point)
{
    points_.emplace_back(std::make_unique<Point>(std::move(point)));
    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const
{
    auto& out = context.out;
    out << "<polyline points=\""sv;

    if (points_.empty())
    {
        out << "\" />"sv;
    
        return;
    }
    
    const auto range_begin = points_.begin();
    out << (*range_begin)->x << ',' << (*range_begin)->y;

    for (auto it = std::next(range_begin, 1); it != points_.end(); ++it)
    {
        out << ' ' << (*it)->x << ',' << (*it)->y;
    }
    out << "\""sv;

    RenderAttrs(context.out);
    out << "/>"sv;
}

Text& Text::SetPosition(Point pos)
{
    pos_ = pos;
    return *this;
}

Text& Text::SetOffset(Point offset)
{
    offset_ = offset;
    return *this;
}

Text& Text::SetFontSize(uint32_t size)
{
    size_ = size;
    return *this;
}

Text& Text::SetFontFamily(std::string font_family)
{
    font_family_ = font_family;
    return *this;
}

Text& Text::SetFontWeight(std::string font_weight)
{
    font_weight_ = font_weight;
    return *this;
}

Text& Text::SetData(std::string data)
{
    data_ = ProcessData(data);
    return *this;
}

void Text::ChangeSymbol(std::string& data, char symbol,
    const std::string& change_to)
{
    size_t pos;
    while ((pos = data.find(symbol)) != std::string::npos)
    {
        data.replace(pos, 1, change_to);
    }
}

std::string Text::ProcessData(std::string& data)
{
    ChangeSymbol(data, '&', "&amp;");
    ChangeSymbol(data, '"', "&quot;");
    ChangeSymbol(data, '\'', "&apos;");
    ChangeSymbol(data, '<', "&lt;");
    ChangeSymbol(data, '>', "&gt;");

    return data;
}

void Text::RenderObject(const RenderContext& context) const
{
    auto& out = context.out;    
    out << "<text"sv;
    
    RenderAttrs(context.out);
    out << " x=\""sv << pos_.x << "\" y=\""sv << pos_.y;
    out << "\" dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y;
    out << "\" font-size=\""sv << size_ << "\""sv;

    if (!font_family_.empty())
    {
        out << " font-family=\""sv << font_family_ << "\""sv;
    }

    if (!font_weight_.empty())
    {
        out << " font-weight=\""sv << font_weight_ << "\""sv;
    }

    out << ">"sv;
    out << data_ << "</text>"sv;
}

void Document::AddPtr(std::unique_ptr<Object>&& obj)
{
    objects_.emplace_back(std::move(obj));
}

void Document::Render(std::ostream& out) const
{
    RenderContext ctx(out, 2, 2);

    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv
        << std::endl;

    for (const auto& obj : objects_)
    {
        obj->Render(ctx);
    }

    out << "</svg>"sv;
}

}
