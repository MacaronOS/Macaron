#include "Bitmap.hpp"

Graphics::Bitmap::Bitmap(Graphics::Color* colors, size_t width, size_t height)
    : m_colors(colors)
    , m_width(width)
    , m_height(height)
{
}

Graphics::Bitmap::Bitmap(Graphics::Bitmap&& bitmap)
{
    m_colors = bitmap.m_colors;
    m_width = bitmap.m_width;
    m_height = bitmap.m_height;
    bitmap.m_colors = nullptr;
}

Graphics::Bitmap& Graphics::Bitmap::operator=(Graphics::Bitmap&& bitmap)
{
    m_colors = bitmap.m_colors;
    m_width = bitmap.m_width;
    m_height = bitmap.m_height;
    bitmap.m_colors = nullptr;
    return *this;
}
