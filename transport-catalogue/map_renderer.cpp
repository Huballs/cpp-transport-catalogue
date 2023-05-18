#include "map_renderer.h"

namespace TC::Renderer {




    namespace detail {
        bool IsZero(double value) {
            return std::abs(value) < EPSILON;
        }
    } // namespace detail
} // namespace TC