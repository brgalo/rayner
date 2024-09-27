#include "state.hpp"
#include <string>

namespace rn {
bool State::itemGetter(void *data, int idx, const char **out_str) {
    *out_str =
        reinterpret_cast<std::string*>(data)[idx].c_str();
    return true;
};
}