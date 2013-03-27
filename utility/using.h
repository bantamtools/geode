#pragma once

#include <othercore/utility/macro_map.h>
#include <othercore/utility/remove_commas.h>

#define OTHER_USING_HELPER(name) using Base::name;
#define OTHER_USING(...) OTHER_REMOVE_COMMAS(OTHER_MAP(OTHER_USING_HELPER,__VA_ARGS__))
