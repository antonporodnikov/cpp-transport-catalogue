// REMAKE MODULE
#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <sstream>
#include <variant>
#include <vector>

namespace svg {

struct Point {
    Point() = default;
    Point(double x, double y) : x(x), y(y) {}

    double x = 0;
    double y = 0;
};

struct Rgb {
    Rgb() = default;
    Rgb(int red, int green, int blue) : red(red), green(green), blue(blue) {}

    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
};

struct Rgba {
    Rgba() = default;
    Rgba(int red, int green, int blue, double opacity)
        : red(red), green(green), blue(blue), opacity(opacity)
    {
    }

    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
    double opacity = 1.0;
};

struct ColorPrinter {
    std::string operator()(std::monostate) const
    {
        return "none";
    }

    std::string operator()(std::string color) const
    {
        return color;
    }

    std::string operator()(Rgb color) const
    {
        std::stringstream temp;
        temp << "rgb(" << static_cast<int>(color.red) << ","
            << static_cast<int>(color.green) << ","
            << static_cast<int>(color.blue) << ")";

        std::string result = temp.str();

        return result;
    }

    std::string operator()(Rgba color) const
    {
        std::stringstream temp;
        temp << "rgba(" << static_cast<int>(color.red) << ","
            << static_cast<int>(color.green) << ","
            << static_cast<int>(color.blue) << "," << color.opacity << ")";

        std::string result = temp.str();

        return result;
    }
};

using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;

inline const Color NoneColor{"none"};

enum class StrokeLineCap {
    BUTT,
    ROUND,
    SQUARE,
};

enum class StrokeLineJoin {
    ARCS,
    BEVEL,
    MITER,
    MITER_CLIP,
    ROUND,
};

std::ostream& operator<<(std::ostream& out,
    const StrokeLineCap stroke_line_cup);

std::ostream& operator<<(std::ostream& out,
    const StrokeLineJoin stroke_line_join);

std::ostream& operator<<(std::ostream& out, const Color color);

struct RenderContext {
    RenderContext(std::ostream& out) : out(out) {}

    RenderContext(std::ostream& out, int indent_step, int indent = 0)
        : out(out), indent_step(indent_step), indent(indent)
    {
    }

    RenderContext Indented() const
    {
        return {out, indent_step, indent + indent_step};
    }

    void RenderIndent() const
    {
        for (int i = 0; i < indent; ++i)
        {
            out.put(' ');
        }
    }

    std::ostream& out;
    int indent_step = 0;
    int indent = 0;
};

class Object {
public:
    virtual ~Object() = default;

    void Render(const RenderContext& context) const;

private:
    virtual void RenderObject(const RenderContext& context) const = 0;
};

template <typename Owner>
class PathProps {
public:
    Owner& SetFillColor(Color color)
    {
        fill_color_ = std::move(color);
        return AsOwner();
    }

    Owner& SetStrokeColor(Color color)
    {
        stroke_color_ = std::move(color);
        return AsOwner();
    }

    Owner& SetStrokeWidth(double width)
    {
        stroke_width_ = std::move(width);
        return AsOwner();
    }

    Owner& SetStrokeLineCap(StrokeLineCap line_cap)
    {
        stroke_line_cap_ = std::move(line_cap);
        return AsOwner();
    }

    Owner& SetStrokeLineJoin(StrokeLineJoin line_join)
    {
        stroke_line_join_ = std::move(line_join);
        return AsOwner();
    }

protected:
    ~PathProps() = default;

    void RenderAttrs(std::ostream& out) const
    {
        using namespace std::literals;

        if (fill_color_)
        {
            out << " fill=\""sv << *fill_color_ << "\""sv;
        }

        if (stroke_color_)
        {
            out << " stroke=\""sv << *stroke_color_ << "\""sv;
        }

        if (stroke_width_)
        {
            out << " stroke-width=\""sv << *stroke_width_ << "\""sv;
        }

        if (stroke_line_cap_)
        {
            out << " stroke-linecap=\""sv << *stroke_line_cap_ << "\""sv;
        }

        if (stroke_line_join_)
        {
            out << " stroke-linejoin=\""sv << *stroke_line_join_ << "\""sv;
        }
    }

private:
    Owner& AsOwner()
    {
        return static_cast<Owner&>(*this);
    }

    std::optional<Color> fill_color_;
    std::optional<Color> stroke_color_;
    std::optional<double> stroke_width_;
    std::optional<StrokeLineCap> stroke_line_cap_;
    std::optional<StrokeLineJoin> stroke_line_join_;
};

class Circle final : public Object, public PathProps<Circle> {
public:
    ~Circle() = default;

    Circle& SetCenter(Point center);
    Circle& SetRadius(double radius);

private:
    void RenderObject(const RenderContext& context) const override;

    Point center_;
    double radius_ = 1.0;
};

class Polyline final : public Object, public PathProps<Polyline> {
public:
    ~Polyline() = default;

    Polyline& AddPoint(Point point);

private:
    void RenderObject(const RenderContext& context) const override;

    std::vector<std::shared_ptr<Point>> points_;
};

class Text final : public Object, public PathProps<Text> {
public:
    ~Text() = default;

    Text& SetPosition(Point pos);
    Text& SetOffset(Point offset);
    Text& SetFontSize(uint32_t size);
    Text& SetFontFamily(std::string font_family);
    Text& SetFontWeight(std::string font_weight);
    Text& SetData(std::string data);

private:
    void ChangeSymbol(std::string& data, char symbol,
        const std::string& change_to);

    std::string ProcessData(std::string& data);

    void RenderObject(const RenderContext& context) const override;

    Point pos_;
    Point offset_;
    uint32_t size_ = 1;
    std::string font_family_;
    std::string font_weight_;
    std::string data_;
};

class ObjectContainer {
public:
    virtual ~ObjectContainer() = default;

    template <typename T>
    void Add(T obj);

    virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;

protected:
    std::vector<std::shared_ptr<Object>> objects_;
};

class Document : public ObjectContainer {
public:
    void AddPtr(std::unique_ptr<Object>&& obj) override;

    void Render(std::ostream& out) const;
};

template <typename T>
void ObjectContainer::Add(T obj)
{
    objects_.emplace_back(std::make_unique<T>(std::move(obj)));
}

class Drawable {
public:
    virtual ~Drawable() = default;

    virtual void Draw(ObjectContainer& container) const = 0;
};

}  // namespace svg
