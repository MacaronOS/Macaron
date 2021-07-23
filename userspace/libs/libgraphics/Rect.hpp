#pragma once

#include <wisterialib/common.hpp>

namespace Graphics {

class Rect {
public:
    Rect() = default;
    Rect(int left, int top, int right, int bottom);

    Rect(const Rect& rect) = default;
    Rect& operator=(const Rect&) = default;

    inline size_t width() const;
    inline size_t height() const;

    inline bool contains(int left, int top, int right, int bottom) const;
    inline bool contains(const Rect&) const;

    inline bool intersects(const Rect&) const;
    inline Rect intersection(const Rect&) const;

    inline bool operator==(const Rect&) const;

public:
    int left {};
    int top {};
    int right {};
    int bottom {};
};

}
