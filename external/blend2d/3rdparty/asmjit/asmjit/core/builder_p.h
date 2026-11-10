// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_CORE_BUILDER_P_H_INCLUDED
#define ASMJIT_CORE_BUILDER_P_H_INCLUDED

#include <asmjit/core/api-config.h>
#ifndef ASMJIT_NO_BUILDER

#include <asmjit/core/builder.h>

ASMJIT_BEGIN_NAMESPACE

//! \cond INTERNAL
//! \addtogroup asmjit_builder
//! \{

static ASMJIT_INLINE void Builder_assign_inline_comment(BaseBuilder* self, BaseNode* node, const char* comment) noexcept {
  if (comment) {
    node->set_inline_comment(static_cast<char*>(self->_builder_arena.dup(comment, strlen(comment), true)));
  }
}

static ASMJIT_INLINE void Builder_assign_inst_state(BaseBuilder* self, InstNode* node, const BaseEmitter::State& state) noexcept {
  node->set_options(state.options);
  node->set_extra_reg(state.extra_reg);
  Builder_assign_inline_comment(self, node, state.comment);
}

//! \}
//! \endcond

ASMJIT_END_NAMESPACE

#endif // !ASMJIT_NO_BUILDER
#endif // ASMJIT_CORE_BUILDER_P_H_INCLUDED
