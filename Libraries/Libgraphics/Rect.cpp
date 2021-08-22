#include "Rect.hpp"

#include <Wisterialib/Common.hpp>

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

bool Rect::contains(int x, int y) const
{
    return left <= x && top <= y && right >= x && bottom >= y;
}

bool Rect::operator==(const Rect& rect) const
{
    return left == rect.left && top == rect.top && right == rect.right && bottom == rect.bottom;
}

bool Rect::intersects(const Rect& rect) const {
    int l = max(left, rect.left);
    int t = max(top, rect.top);
    int r = min(right, rect.right);
    int b = min(bottom, rect.bottom);

    return r >= l && b >= t; 
}

Rect Rect::intersection(const Rect& rect) const {
    int l = max(left, rect.left);
    int t = max(top, rect.top);
    int r = min(right, rect.right);
    int b = min(bottom, rect.bottom);

    return Rect(l, t, r, b);
}

}