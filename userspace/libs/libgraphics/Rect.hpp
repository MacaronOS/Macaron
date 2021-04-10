#pragma once

class Rect {
public:
    Rect() = default;
    Rect(int left, int top, int right, int bottom);

    Rect(const Rect& rect);
    Rect& operator=(const Rect&);

    inline bool width() const;
    inline bool height() const;

    inline bool contains(int left, int top, int right, int bottom) const;
    inline bool contains(const Rect&) const;

    inline bool operator==(const Rect&) const;

private:
    int left {};
    int top {};
    int right {};
    int bottom {};
};