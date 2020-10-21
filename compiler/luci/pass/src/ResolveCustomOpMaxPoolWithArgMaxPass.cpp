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

#include "luci/Pass/ResolveCustomOpMaxPoolWithArgMaxPass.h"

#include "flatbuffers/flexbuffers.h"

#include <luci/IR/CircleNodes.h>

namespace
{

luci::Padding AsPadding(int32_t v)
{
  switch (v)
  {
  case 0:
   
    break;
  
  default:
    break;
  }
}

bool resolve_max_pool_with_arg_max(luci::CircleCustom *cop)
{
  auto graph = cop->graph();
  const std::vector<uint8_t> custom_options = cop->custom_options();
  const std::string custom_code = cop->custom_code();
 
  if (custom_code == "MaxPoolWithArgMax")
  {
    auto max_pool_with_argmax = cop->graph()->nodes()->create<luci::CircleMaxPoolWithArgMax>();
    max_pool_with_argmax->input(cop->inputs(0));
    auto map = flexbuffers::GetRoot(custom_options).AsMap();

    max_pool_with_argmax->padding(map["padding"].AsInt32());
    max_pool_with_argmax->stride()->w(map["stride_w"].AsInt32());
    max_pool_with_argmax->stride()->h(map["stride_h"].AsInt32());
    max_pool_with_argmax->filter()->w(map["filter_width"].AsInt32());
    max_pool_with_argmax->filter()->h(map["filter_height"].AsInt32());
    max_pool_with_argmax->fusedActivationFunction(map["activation"].AsInt32());
    max_pool_with_argmax->output_type(map["output_type"].AsInt32());

    replace(cop).with(max_pool_with_argmax);
    return true;
  }
  
  return false;
}

} // namespace

namespace luci
{

bool ResolveCustomOpMaxPoolWithArgMaxPass::run(loco::Graph *g)
{
  bool changed = false;
  for (auto node : loco::active_nodes(loco::output_nodes(g)))
  {
    auto cop = dynamic_cast<luci::CircleCustom *>(node);
    if (not cop)
      continue;

    if (cop->custom_code() != "MaxPoolWithArgMax")
      continue;

    if (!resolve_max_pool_with_arg_max(cop))
      continue;

    changed = true;
  }

  return changed;
}

} // namespace luci
