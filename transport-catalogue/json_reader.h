#pragma once

#include <istream>
#include "json.h"

auto LoadJson(std::istream& stream){
    return json::Load(stream);
}