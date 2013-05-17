// Nearly exact geometric constructions
#pragma once

#include <other/core/exact/config.h>
#include <other/core/vector/Vector.h>
namespace other {

// Construct the intersection of two segments, assuming they actually do intersect
OTHER_CORE_EXPORT OTHER_CONST exact::Vec2 segment_segment_intersection(const exact::Point2 a0, const exact::Point2 a1, const exact::Point2 b0, const exact::Point2 b1);

}
