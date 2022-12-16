#include "svg.h"
#include <utility>

using namespace std::literals;

namespace svg {

    std::ostream& operator<<(std::ostream& out, StrokeLineCap stroke_line_cap) {
        switch (stroke_line_cap) {
        case StrokeLineCap::BUTT:
            out << "butt"s;
            break;
        case StrokeLineCap::ROUND:
            out << "round"s;
            break;
        case StrokeLineCap::SQUARE:
            out << "square"s;
            break;
        }
        return out;
    }

    std::ostream& operator<<(std::ostream& out, StrokeLineJoin stroke_line_join) {
        switch (stroke_line_join) {
        case StrokeLineJoin::ARCS:
            out << "arcs"s;
            break;
        case StrokeLineJoin::BEVEL:
            out << "bevel"s;
            break;
        case StrokeLineJoin::MITER:
            out << "miter"s;
            break;
        case StrokeLineJoin::MITER_CLIP:
            out << "miter-clip"s;
            break;
        case StrokeLineJoin::ROUND:
            out << "round"s;
            break;
        }
        return out;
    }

    void ColorRender::operator()(std::monostate) const {
        out << std::get<std::string>(NoneColor);
    }
    void ColorRender::operator()(std::string string_color) const {
        out << string_color;
    }
    void ColorRender::operator()(Rgb rgb_color) const {
        out << "rgb("sv << rgb_color.red+0 << ","sv << rgb_color.green + 0 << ","sv << rgb_color.blue + 0 << ")"sv;
    }
    void ColorRender::operator()(Rgba rgba_color) const {
        out << "rgba("sv << rgba_color.red + 0 << ","sv << rgba_color.green + 0 << ","sv << rgba_color.blue + 0 << ","sv << rgba_color.opacity << ")"sv;
    }

    std::ostream& operator<<(std::ostream& out, Color color) {
        std::visit(ColorRender{ out }, color);
        return out;
    }

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

    // ---------- Circle ------------------

    Circle& Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y;
        out << "\" "sv;
        out << "r=\""sv << radius_ << "\""sv;
        PathProps::RenderAttrs(out);
        out << "/>"sv;
    }
    //-------------------------------------

    //-----------Polyline------------------

    Polyline& Polyline::AddPoint(Point point) {
        points_.push_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;


        out << "<polyline points=\""sv;

        bool is_first = true;
        for (const Point& point : points_) {
            if (is_first) {
                is_first = false;
                out << point.x << ","sv << point.y;
                continue;
            }
            out << " "sv << point.x << ","sv << point.y;
        }
        out << "\""sv;
        PathProps::RenderAttrs(out);
        out << "/>"sv;
    }
    //-------------------------------------

    //--------------Text-------------------

            // Задаёт координаты опорной точки (атрибуты x и y)
    Text& Text::SetPosition(Point pos) {
        position_ = pos;
        return *this;
    }

    // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
    Text& Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }

    // Задаёт размеры шрифта (атрибут font-size)
    Text& Text::SetFontSize(uint32_t size) {
        font_size_ = size;
        return *this;
    }

    // Задаёт название шрифта (атрибут font-family)
    Text& Text::SetFontFamily(std::string font_family) {
        font_family_ = std::move(font_family);
        return *this;
    }

    // Задаёт толщину шрифта (атрибут font-weight)
    Text& Text::SetFontWeight(std::string font_weight) {
        font_weight_ = std::move(font_weight);
        return *this;
    }

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text& Text::SetData(std::string data) {
        text_data_ = std::move(data);
        return *this;
    }

    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<text"sv;
        PathProps::RenderAttrs(out);
        out <<  " x=\""sv << position_.x << "\" y=\""sv << position_.y << "\" "sv;
        out << "dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\" "sv;
        out << "font-size=\""sv << font_size_ << "\""sv;
        if (!font_family_.empty()) {
            out << " font-family=\""sv << font_family_ << "\""sv;
        }
        if (!font_weight_.empty()) {
            out << " font-weight=\""sv << font_weight_ << "\""sv;
        }
        //out << " "sv;
        //PathProps::RenderAttrs(out);
        out << ">"sv;

        for (const char& ch : text_data_) {
            if (ch == '"') {
                out << "&quot;"sv;
            }
            else if (ch == '\'') {
                out << "&apos;"sv;
            }
            else if (ch == '<') {
                out << "&lt;"sv;
            }
            else if (ch == '>') {
                out << "&gt;"sv;
            }
            else if (ch == '&') {
                out << "&amp;"sv;
            }
            else {
                out << ch;
            }
        }

        out << "</text>"sv;
    }
    //-------------------------------------

       // Добавляет в svg-документ объект-наследник svg::Object
    void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        objects_.emplace_back(std::move(obj));
    }

    void Document::Render(std::ostream& out) const {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version =\"1.1\">"sv << std::endl;
        for (auto it = objects_.begin(); it < objects_.end(); ++it) {
            (*it)->Render(out);
            //out << std::endl;
        }
        out << "</svg>"sv;
    }

}  // namespace svg