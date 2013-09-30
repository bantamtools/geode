#include <othercore/exact/circle_offsets.h>
#include <othercore/exact/circle_predicates.h>
#include <othercore/geometry/polygon.h>
#include <othercore/utility/stl.h>
#include <othercore/python/wrap.h>
#include <othercore/python/stl.h>
#include <othercore/exact/scope.h>

namespace other {

typedef RawArray<const ExactCircleArc> Arcs;
typedef RawArray<const Vertex> Vertices;

static Array<ExactCircleArc> build_arc_capsule(Arcs arcs, const Vertex& start, const Vertex& end, ExactInt signed_offset, int& next_free_index) {
  assert(start.i1 == end.i0);

  const ExactCircleArc& curr_arc = arcs[start.i1];
  const ExactInt abs_offset = abs(signed_offset);
  const ExactInt outer_r = curr_arc.radius + abs_offset;
  const ExactInt inner_r = curr_arc.radius - abs_offset;

  assert(curr_arc.radius > 0 && outer_r > 0 && abs_offset > 0);

  const bool positive = signed_offset > 0;

  Array<ExactCircleArc> result;

  const Interval endpoint_dist = assume_safe_sqrt(sqr_magnitude(start.p() - end.p()));

  // Since the endpoints are approximate, their intersections could be in the wrong order. If we don't catch this, a small section of arc can get turned into a complete circle
  // This checks that neither endpoint could cross the radial line through midpoint of the original arc
  const bool left_flags_safe = certainly_less(Interval(2*Vertex::tolerance()), endpoint_dist);
  if(!left_flags_safe) { // If endcaps are very close together we treat them as a single point

    // Check if the arc we are offseting was more than a half circle
    const bool is_small_arc = circle_intersections_ccw(arcs, start.reverse(), end) == curr_arc.positive;

    result.append(ExactCircleArc());
    result.append(ExactCircleArc());
    ExactCircleArc& c0 = result[0];
    ExactCircleArc& c1 = result[1];

    if(is_small_arc) {
      // If original arc was a small angle we...
      c0.center = (start.rounded + end.rounded); // Use midpoint
      c0.radius = abs_offset + Vertex::tolerance();  // 'round up' by the vertex tolerance to cover parts we might have missed
    }
    else {
      // If original arc was a large angle we treat original arc as a full circle
      c0.center = curr_arc.center;
      c0.radius = curr_arc.radius + abs_offset;
      if(inner_r > 0) {
        OTHER_WARNING("Offset filled interior of annulus"); // FIXME: Need to generate the annulus in this case, but that requires changing return type to handle multiple arcs
      }
    }

    c0.index = next_free_index++;
    c0.positive = true;
    c0.left = true;

    // FIXME: We just want a circle, but we currently need at least two arcs, so this constructs a very small circle on the rightmost bit of c0
    c1.center = c0.center;
    c1.center.x += c0.radius;
    c1.radius = 1;
    c1.index = next_free_index++;
    c1.positive = true;
    c1.left = true;
  }
  else {
    assert((start.rounded - end.rounded).L1_Norm() > 0); // It shouldn't be possible for left flags to be safe if endcaps round to same point
    result.append(ExactCircleArc());
    result.back().center = curr_arc.positive ? start.rounded : end.rounded;
    result.back().radius = abs_offset + 2*Vertex::tolerance(); // This ensures a non-degenerate intersection with the other arcs
    result.back().index = next_free_index++;
    result.back().positive = true;
    result.back().left = true;
    const int start_cap_index = result.size() - 1;
    // We will grow one end of each capsule very slightly
    // This helps avoid the cost of evaluting perturbed degenerate intersections where capsules for consecutive arcs share endpoints
    if(curr_arc.positive) result.back().radius += 2;

    result.append(ExactCircleArc());
    result.back().center = curr_arc.center;
    result.back().radius = outer_r;
    result.back().index = next_free_index++;
    result.back().positive = true;
    result.back().left = true;
    int outer_arc_index = result.size() - 1;

    result.append(ExactCircleArc());
    result.back().center = curr_arc.positive ? end.rounded : start.rounded;
    result.back().radius = abs_offset + 2*Vertex::tolerance(); // This ensures a non-degenerate intersection with the other arcs
    result.back().index = next_free_index++;
    result.back().positive = true;
    result.back().left = false;
    int end_cap_index = result.size() - 1;
    // We will grow one end of each capsule very slightly
    // This helps avoid the cost of evaluting perturbed degenerate intersections where capsules for consecutive arcs share endpoints
    if(!curr_arc.positive) result.back().radius += 2;

    if(!circles_intersect(result, start_cap_index, outer_arc_index)
        || !circles_intersect(result, end_cap_index, outer_arc_index)) {
      result.remove_index(outer_arc_index); // Just get rid of outer arc if it doesn't hit endcaps (it must be inside)
      outer_arc_index = -1;
      end_cap_index -= 1;
      result[start_cap_index].left = false; // Fix left flag since will hit endcap now
    }

    if(inner_r > 0) { // if inner_r < 0, then inside arc has completely eroded and endcaps will connect together
      result.append(ExactCircleArc());
      result.back().center = curr_arc.center;
      result.back().radius = inner_r;
      result.back().index = next_free_index++;
      result.back().positive = false;
      result.back().left = false;
      assert(result.size() == 4);
      const int inner_arc_index = result.size()-1;

      if(!circles_intersect(result, start_cap_index, inner_arc_index)
        || !circles_intersect(result, end_cap_index, inner_arc_index)) {
        result.remove_index(inner_arc_index); // Just get rid of inner arc as well if it doesn't hit endcaps (it must be inside)
      }
    }
    tweak_arcs_to_intersect(result);
  }

  if(!positive) {
    std::reverse(result.begin(), result.end());
    for(auto& arc : result) {
      arc.positive = !arc.positive;
      arc.left = !arc.left;
    }
  }

  return result;
}

Nested<ExactCircleArc> exact_offset_arcs(const Nested<const ExactCircleArc> nested_arcs, const ExactInt offset) {
  // Compute some preliminary info
  const RawArray<const ExactCircleArc> arcs = nested_arcs.flat;
  const Array<const int> next = closed_contours_next(nested_arcs);
  const Array<const Vertex> vertices = compute_vertices(arcs, next); // vertices[i] is the start of arcs[i]

  // offset shape is union of the origional shape and the thickened border
  // for a negative offset the thickened border will need to be negatively oriented

  // start with a copy of the input arcs
  auto minkowski_terms = Nested<ExactCircleArc, false>::copy(nested_arcs);

  // We will use consecutive indicies of newly constructed geometry starting from max(arc index)+1
  // TODO: For a large series of operations we might need to use some sort of free list to avoid overflow
  int next_free_index = 0;
  for(const ExactCircleArc& ea : minkowski_terms.flat) {
    if(ea.index >= next_free_index) next_free_index = ea.index+1;
  }

  IntervalScope scope;
  // for each arc in the original shape, thicken by offset and add to back
  // capsule will have negitive winding if offset < 0
  for(int arc_i : range(arcs.size())) {
    minkowski_terms.append(build_arc_capsule(arcs, vertices[arc_i], vertices[next[arc_i]], offset, next_free_index));
    OTHER_ASSERT(next_free_index > 0, "Index overflow using consecutive indicies. Free index data structure needed!");
  }
  auto result = exact_split_circle_arcs(minkowski_terms, 0);
  return result;
}

static Quantized min_safe_exact_offset() { return 1; }

Nested<CircleArc> offset_arcs(const Nested<const CircleArc> raw_arcs, const real offset_amount) {
  if(raw_arcs.total_size() == 0) {
    return Nested<CircleArc>(); // Empty input --> empty output
  }
  const auto min_bounds = approximate_bounding_box(raw_arcs).thickened(abs(offset_amount));
  const auto qu_ea = quantize_circle_arcs(raw_arcs, min_bounds);

  const Quantizer<real,2>& quantizer = qu_ea.x;
  const Nested<ExactCircleArc>& exact_arcs = qu_ea.y;

  const ExactInt exact_offset = ExactInt(floor(quantizer.scale*offset_amount));

  if(abs(exact_offset) < min_safe_exact_offset()) {
    OTHER_WARNING("Arc offset amount was below numerical representation threshold! (this should be a few nm for geometry that fits in meter bounds)");
    return raw_arcs.copy();
  }

  auto result = unquantize_circle_arcs(quantizer, exact_offset_arcs(exact_arcs, exact_offset));
  return result;
}

std::vector<Nested<CircleArc>> offset_shells(const Nested<const CircleArc> raw_arcs, const real shell_thickness, const int num_shells) {
  OTHER_ASSERT(num_shells > 0);
  if(raw_arcs.total_size() == 0) {
    return make_vector(Nested<CircleArc>()); // Empty input --> empty output
  }

  const auto min_bounds = approximate_bounding_box(raw_arcs).thickened(abs(shell_thickness)*num_shells);
  const auto qu_ea = quantize_circle_arcs(raw_arcs, min_bounds);

  const Quantizer<real,2>& quantizer = qu_ea.x;
  const Nested<ExactCircleArc>& exact_arcs = qu_ea.y;

  const ExactInt exact_shell = ExactInt(floor(quantizer.scale*shell_thickness));

  if(abs(exact_shell) < min_safe_exact_offset()) {
    OTHER_WARNING("Arc offset amount was below numerical representation threshold! (this should be a few nm for geometry that fits in meter bounds)");
    return make_vector(raw_arcs.copy());
  }

  std::vector<Nested<CircleArc>> result;
  Nested<ExactCircleArc> temp_shell = exact_arcs;
  for(int i = 0; i< num_shells; ++i) {
    temp_shell = exact_offset_arcs(temp_shell, exact_shell);
    result.push_back(unquantize_circle_arcs(quantizer, temp_shell));
  }
  return result;
}

} // namespace other
using namespace other;
void wrap_circle_offsets() {
  OTHER_FUNCTION(offset_arcs)
  OTHER_FUNCTION(offset_shells)
}

