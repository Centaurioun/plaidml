// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include "plaidml_ops.hpp"
#include "plaidml_util.hpp"

#include "ngraph/opsets/opset.hpp"
#include "ngraph/opsets/opset1.hpp"

#include "plaidml/op/op.h"

using namespace plaidml;          // NOLINT[build/namespaces]
using namespace InferenceEngine;  // NOLINT[build/namespaces]

namespace PlaidMLPlugin {

void registerStridedSlice() {
  registerOp("StridedSlice", [](const Context& ctx) {
    auto* layer = ngraph::as_type<ngraph::opset1::StridedSlice>(ctx.layer);
    IE_ASSERT(ctx.operands.size() <= 4);
    IE_ASSERT(ctx.operands.size() >= 3);
    auto I = ctx.operands.at(0);
    auto input_sizes = I.compute_shape().sizes();
    auto starts = cast_constant_operand<int64_t>(1, layer);
    auto stops = cast_constant_operand<int64_t>(2, layer);
    std::vector<int64_t> steps;
    if (ctx.operands.size() == 4) {
      steps = cast_constant_operand<int64_t>(3, layer);
    } else {
      steps.assign(starts.size(), 1);
    }
    auto begin_mask = layer->get_begin_mask();
    auto end_mask = layer->get_end_mask();
    auto new_axis_mask = layer->get_new_axis_mask();
    auto shrink_axis_mask = layer->get_shrink_axis_mask();
    auto ellipsis_mask = layer->get_ellipsis_mask();
    for (auto ell : ellipsis_mask) {
      if (ell) {
        THROW_IE_EXCEPTION << "PlaidML plugin does not yet support StridedSlice ellipses";
      }
    }

    for (size_t i = 0; i < new_axis_mask.size(); i++) {
      if (new_axis_mask[i]) {
        I = op::unsqueeze(I, {static_cast<int64_t>(i)});
      }
    }
    auto result = op::slice(I);

    if (starts.size() > I.rank()) {
      THROW_IE_EXCEPTION
          << "StridedSlice must not have more begin values than the input rank after accounting for new_axis_mask";
    }
    if (stops.size() != starts.size() || steps.size() != starts.size()) {
      THROW_IE_EXCEPTION << "StridedSlice must have the same number of begin values, end values, and stride values";
    }
    while (begin_mask.size() < I.rank()) {
      begin_mask.push_back(0);
    }
    while (end_mask.size() < I.rank()) {
      end_mask.push_back(0);
    }
    while (shrink_axis_mask.size() < I.rank()) {
      shrink_axis_mask.push_back(0);
    }

    for (size_t i = 0; i < I.rank(); i++) {
      if (i >= starts.size()) {
        result.add_dim(edsl::None(), edsl::None(), edsl::None());
        continue;
      }

      // The abs bounds in different direction are not equal sine SliceOp uses 0 vs -1 as begin
      // The slice region is a half-open interval [...), out-of-bounds values are clamped in differnt way
      // TODO: Check sign of "steps" when zero dimension is allowed in openvino
      if (std::abs(starts[i]) > input_sizes[i]) {
        starts[i] = starts[i] < 0 ? -1 * input_sizes[i] : input_sizes[i] - 1;
      }
      if (std::abs(stops[i]) > input_sizes[i]) {
        stops[i] = stops[i] < 0 ? -1 * input_sizes[i] - 1 : input_sizes[i];
      }

      if (!begin_mask[i] && !end_mask[i]) {
        if (starts[i] == stops[i] || (steps[i] == 1 && stops[i] == 0) || shrink_axis_mask[i]) {
          // We hack around nGraph's approach of allowing size 0 dimensions by explicitly looking for the patterns of
          // singleton dims
          result.add_dim(starts[i]);
          continue;
        }
      }
      // We're in the general case
      result.add_dim(begin_mask[i] ? edsl::None() : edsl::Value(starts[i]),  //
                     end_mask[i] ? edsl::None() : edsl::Value(stops[i]),     //
                     edsl::Value(steps[i]));
    }

    return edsl::make_tuple(result);
  });
}

}  // namespace PlaidMLPlugin
