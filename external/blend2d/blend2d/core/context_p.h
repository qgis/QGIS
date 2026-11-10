// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_CONTEXT_P_H_INCLUDED
#define BLEND2D_CONTEXT_P_H_INCLUDED

#include <blend2d/core/api-internal_p.h>
#include <blend2d/core/array_p.h>
#include <blend2d/core/context.h>
#include <blend2d/core/object_p.h>
#include <blend2d/core/path_p.h>
#include <blend2d/threading/atomic_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_internal
//! \{

namespace bl::ContextInternal {

//! \name BLContext - Private - Constants
//! \{

static constexpr const double kMinimumTolerance = 0.01;
static constexpr const double kMaximumTolerance = 0.50;

//! \}

//! \name BLContext - Private - State Construction & Destruction
//! \{

static BL_INLINE void init_state(BLContextState* self) noexcept {
  self->target_image = nullptr;
  self->target_size.reset();
  self->hints.reset();
  self->comp_op = uint8_t(BL_COMP_OP_SRC_OVER);
  self->fill_rule = uint8_t(BL_FILL_RULE_NON_ZERO);
  self->style_type[BL_CONTEXT_STYLE_SLOT_FILL] = uint8_t(BL_OBJECT_TYPE_NULL);
  self->style_type[BL_CONTEXT_STYLE_SLOT_STROKE] = uint8_t(BL_OBJECT_TYPE_NULL);
  self->saved_state_count = 0u;

  self->global_alpha = 1.0;
  self->style_alpha[BL_CONTEXT_STYLE_SLOT_FILL] = 1.0;
  self->style_alpha[BL_CONTEXT_STYLE_SLOT_STROKE] = 1.0;

  bl_stroke_options_init(&self->stroke_options);
  self->approximation_options = PathInternal::make_default_approximation_options();

  self->meta_transform.reset();
  self->user_transform.reset();
}

static BL_INLINE void destroy_state(BLContextState* self) noexcept {
  bl_array_reset(&self->stroke_options.dash_array);
}

//! \}

} // {bl::ContextInternal}

//! \}
//! \endcond

#endif // BLEND2D_CONTEXT_P_H_INCLUDED
