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

#include "kernels/Div.h"

#include "kernels/Utils.h"

#include <tensorflow/lite/kernels/internal/reference/reference_ops.h>

namespace luci_interpreter
{
namespace kernels
{

Div::Div(const Tensor *input1, const Tensor *input2, Tensor *output, const DivParams &params)
    : KernelWithParams<DivParams>({input1, input2}, {output}, params)
{
}

void Div::configure()
{
  LUCI_INTERPRETER_CHECK(input1()->element_type() == input2()->element_type());
  LUCI_INTERPRETER_CHECK(input1()->element_type() == output()->element_type());

  output()->resize(calculateShapeForBroadcast(input1()->shape(), input2()->shape()));
}

void Div::execute() const
{
  switch (input1()->element_type())
  {
    case DataType::FLOAT32:
      evalFloat();
      break;
    case DataType::U8:
      evalQuantized();
      break;
    default:
      throw std::runtime_error("Unsupported type.");
  }
}

void Div::evalFloat() const
{
  float activation_min{};
  float activation_max{};
  calculateActivationRange(_params.activation, &activation_min, &activation_max);

  tflite::ArithmeticParams params{};
  params.float_activation_min = activation_min;
  params.float_activation_max = activation_max;
  const bool need_broadcast = tflite::reference_ops::ProcessBroadcastShapes(
      getTensorShape(input1()), getTensorShape(input2()), &params);

  if (need_broadcast)
  {
    tflite::reference_ops::BroadcastDivSlow(
        params, getTensorShape(input1()), getTensorData<float>(input1()), getTensorShape(input2()),
        getTensorData<float>(input2()), getTensorShape(output()), getTensorData<float>(output()));
  }
  else
  {
    tflite::reference_ops::Div(params, getTensorShape(input1()), getTensorData<float>(input1()),
                               getTensorShape(input2()), getTensorData<float>(input2()),
                               getTensorShape(output()), getTensorData<float>(output()));
  }
}

void Div::evalQuantized() const
{
  const auto input1_scale = static_cast<double>(input1()->scale());
  const auto input2_scale = static_cast<double>(input2()->scale());
  const auto output_scale = static_cast<double>(output()->scale());

  const double real_output_multiplier = input1_scale / (input2_scale * output_scale);

  int32_t output_multiplier{};
  int output_shift{};

  quantizeMultiplier(real_output_multiplier, &output_multiplier, &output_shift);

  int32_t activation_min{};
  int32_t activation_max{};
  calculateActivationRangeQuantized(_params.activation, output(), &activation_min, &activation_max);

  tflite::ArithmeticParams params{};

  params.input1_offset = -input1()->zero_point(); // Note the '-'.
  params.input2_offset = -input2()->zero_point(); // Note the '-'.
  params.output_offset = output()->zero_point();
  params.output_multiplier = output_multiplier;
  params.output_shift = output_shift;
  params.quantized_activation_min = activation_min;
  params.quantized_activation_max = activation_max;

  const bool need_broadcast = tflite::reference_ops::ProcessBroadcastShapes(
      getTensorShape(input1()), getTensorShape(input2()), &params);

  if (need_broadcast)
  {
    tflite::reference_ops::BroadcastDivSlow(
        params, getTensorShape(input1()), getTensorData<uint8_t>(input1()),
        getTensorShape(input2()), getTensorData<uint8_t>(input2()), getTensorShape(output()),
        getTensorData<uint8_t>(output()));
  }
  else
  {
    tflite::reference_ops::Div(params, getTensorShape(input1()), getTensorData<uint8_t>(input1()),
                               getTensorShape(input2()), getTensorData<uint8_t>(input2()),
                               getTensorShape(output()), getTensorData<uint8_t>(output()));
  }
}

} // namespace kernels
} // namespace luci_interpreter
