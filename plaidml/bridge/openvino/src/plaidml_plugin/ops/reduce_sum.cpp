// Copyright (C) 2020 Intel Corporation
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

static OpRegistration reg("reducesum", [](const Context& ctx) {
  IE_ASSERT(ctx.operands.size() == 2);
  auto I = ctx.operands.at(0);
  std::vector<size_t> axes = get_axis_vector_from_constant_operand(1, ctx.layer);
  auto* layer = dynamic_cast<ngraph::opset1::ReduceSum*>(ctx.layer);
  return edsl::make_tuple(op::sum(I, edsl::make_tuple(axes), layer->get_keep_dims()));
});

}  // namespace PlaidMLPlugin