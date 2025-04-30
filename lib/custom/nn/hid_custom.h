/**
 * @file hid.h
 * @brief Functions that help process gamepad inputs.
 */

#pragma once

#include <nn/hid.h>
#include <nn/types.h>
#include <nn/util.h>
#include <nn/util/MathTypes.h>
#include <nn/util/util_BitFlagSet.h>
#include <nn/util/util_BitPack.h>

namespace nn {
namespace hid {

template <size_t T>
void GetTouchScreenState(nn::hid::TouchScreenState<T>*);
} // namespace hid
} // namespace nn
