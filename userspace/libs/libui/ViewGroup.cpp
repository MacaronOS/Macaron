#include "ViewGroup.hpp"

namespace UI {

LayoutParams* ViewGroup::generate_default_layout_params()
{
    return new LayoutParams { LayoutParams::MATCH_PARENT, LayoutParams::MATCH_PARENT };
}

}