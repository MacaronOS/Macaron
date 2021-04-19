#include "Rect.hpp"

#include <wisterialib/common.hpp>

namespace Graphics {

Rect::Rect(int left, int top, int right, int bottom)
    : left(left)
    , top(top)
    , right(right)
    , bottom(bottom)
{
}

size_t Rect::width() const
{
    return right - left;
}

size_t Rect::height() const
{
    return top - bottom;
}

bool Rect::contains(int _left, int _top, int _right, int _bottom) const
{
    return left <= _left && top >= _top && right >= _right && bottom <= _bottom;
}

bool Rect::contains(const Rect& rect) const
{
    return contains(rect.left, rect.top, rect.right, rect.bottom);
}

bool Rect::operator==(const Rect& rect) const
{
    return left == rect.left && top == rect.top && right == rect.right && bottom == rect.bottom;
}

}