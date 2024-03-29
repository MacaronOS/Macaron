#pragma once

#include <Macaronlib/Common.hpp>

namespace Graphics {

class Rect {
public:
    Rect() = default;
    Rect(int left, int top, int right, int bottom);

    Rect(const Rect& rect) = default;
    Rect& operator=(const Rect&) = default;

    size_t width() const;
    size_t height() const;

    size_t area() const { return width() * height(); }

    bool contains(int left, int top, int right, int bottom) const;
    bool contains(const Rect&) const;
    bool contains(int x, int y) const;

    bool intersects(const Rect&) const;
    Rect intersection(const Rect&) const;
    Rect union_rect(const Rect&) const;

    bool operator==(const Rect&) const;

public:
    int left {};
    int top {};
    // Note that the right and bottom coordinates are exclusive.
    int right {};
    int bottom {};
};

}
