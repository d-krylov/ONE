/*
 * Copyright (c) 2020 Samsung Electronics Co., Ltd. All Rights Reserved
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "luci/Import/Nodes/CircleWhere.h"

#include <luci/IR/Nodes/CircleWhere.h>

#include <loco.h>

namespace luci
{

bool CircleWhereGraphBuilder::validate(const ValidateArgs &args) const
{
  const auto &inputs  = args.op.inputs;
  const auto &outputs = args.op.outputs;

  auto in_size = inputs.size();

  if ((in_size != 1) || (in_size != 3))
    return false;

  if (outputs.size() != 1)
    return false;

  const auto &tensors = args.reader.tensors();
  const auto &tensor_0 = tensors.at(inputs[0]);

  if (tensor_0->type != circle::TensorType_BOOL)
    return false;

  return true;
}

CircleNode *CircleWhereGraphBuilder::build_node(const circle::OperatorT &,
                                                const std::vector<CircleNode *> &inputs,
                                                loco::Graph *graph) const
{
  auto *node = graph->nodes()->create<CircleWhere>(inputs.size());
  for (uint32_t i = 0; i < inputs.size(); ++i)
  {
    node->values(i, inputs[i]);
  }

  return node;
}

} // namespace luci
