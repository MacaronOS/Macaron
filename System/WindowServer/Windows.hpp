#pragma once

#include "Window.hpp"

#include <Macaronlib/List.hpp>

class Windows {
public:
    using Iterator = List<Window>::Iterator;

    template <typename... Args>
    inline Window& create_window(Args&&... args)
    {
        m_windows.push_back(Window(forward<Args>(args)...));
        return m_windows.back();
    }

    // Window which is being picked by the mouse.
    inline Iterator picked_window() { return m_picked_window; }
    inline void set_picked_window(Iterator window_it) { m_picked_window = window_it; }
    inline void unset_picked_window() { m_picked_window = Iterator(nullptr); }

    inline Iterator top() { return m_windows.rbegin(); }

    inline void bring_to_top(Iterator window_it)
    {
        if (m_windows.rbegin() == window_it) {
            return;
        }
        m_windows.push_back(move(*window_it));
        m_windows.remove(window_it);
    }

    inline void destroy_window(Iterator window_it)
    {
        m_windows.remove(window_it);
    }

    inline Iterator find_window_by_id(int id)
    {
        Iterator result_window;

        from_top_to_bottom([&](Iterator window_it) {
            if ((*window_it).id == id) {
                result_window = window_it;
                return true;
            }
            return false;
        });

        return result_window;
    }

    template <typename Callback>
    inline void from_top_to_bottom_visible(Callback callback)
    {
        from_top_to_bottom([&](Iterator window_it) {
            if ((*window_it).visibility()) {
                return callback(window_it);
            }
            return false;
        });
    }

    template <typename Callback>
    inline void from_top_to_bottom(Callback callback)
    {
        for (auto it = m_windows.rbegin(); it != m_windows.rend(); it--) {
            if (callback(it)) {
                return;
            }
        }
    }

    Iterator begin() { return m_windows.begin(); }
    Iterator end() { return m_windows.end(); }

private:
    List<Window> m_windows {};
    Iterator m_picked_window {};
};