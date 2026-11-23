// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core/api-build_p.h>
#ifndef ASMJIT_NO_COMPILER

#include <asmjit/core/assembler.h>
#include <asmjit/core/builder_p.h>
#include <asmjit/core/compiler.h>
#include <asmjit/core/cpuinfo.h>
#include <asmjit/core/logger.h>
#include <asmjit/core/rapass_p.h>
#include <asmjit/core/rastack_p.h>
#include <asmjit/core/type.h>
#include <asmjit/support/support.h>

ASMJIT_BEGIN_NAMESPACE

// GlobalConstPoolPass
// ===================

class GlobalConstPoolPass : public Pass {
  ASMJIT_NONCOPYABLE(GlobalConstPoolPass)

public:
  using Base = Pass;

  GlobalConstPoolPass(BaseCompiler& cc) noexcept : Pass(cc, "GlobalConstPoolPass") {}

  Error run(Arena& arena, Logger* logger) override {
    Support::maybe_unused(arena, logger);

    // Flush the global constant pool.
    BaseCompiler& compiler = static_cast<BaseCompiler&>(_cb);
    ConstPoolNode* global_const_pool = compiler._const_pools[uint32_t(ConstPoolScope::kGlobal)];

    if (global_const_pool) {
      compiler.add_after(global_const_pool, compiler.last_node());
      compiler._const_pools[uint32_t(ConstPoolScope::kGlobal)] = nullptr;
    }

    return Error::kOk;
  }
};

// BaseCompiler - Construction & Destruction
// =========================================

BaseCompiler::BaseCompiler() noexcept
  : BaseBuilder(),
    _func(nullptr),
    _virt_regs(),
    _const_pools { nullptr, nullptr } {
  _emitter_type = EmitterType::kCompiler;
  _validation_flags = ValidationFlags::kEnableVirtRegs;
}
BaseCompiler::~BaseCompiler() noexcept {}

// BaseCompiler - Function Management
// ==================================

Error BaseCompiler::new_func_node(Out<FuncNode*> out, const FuncSignature& signature) {
  *out = nullptr;

  // Create FuncNode together with all the required surrounding nodes.
  FuncNode* func_node = nullptr;
  ASMJIT_PROPAGATE(new_node_t<FuncNode>(Out(func_node)));
  ASMJIT_PROPAGATE(new_label_node(Out(func_node->_exit_node)));
  ASMJIT_PROPAGATE(new_node_t<SentinelNode>(Out(func_node->_end), SentinelType::kFuncEnd));

  // Initialize the function's detail info.
  Error err = func_node->detail().init(signature, environment());
  if (ASMJIT_UNLIKELY(err != Error::kOk)) {
    return report_error(err);
  }

  // If the Target guarantees greater stack alignment than required by the calling convention
  // then override it as we can prevent having to perform dynamic stack alignment
  uint32_t environment_stack_alignment = _environment.stack_alignment();

  if (func_node->_func_detail._call_conv.natural_stack_alignment() < environment_stack_alignment) {
    func_node->_func_detail._call_conv.set_natural_stack_alignment(environment_stack_alignment);
  }

  // Initialize the function frame.
  err = func_node->_frame.init(func_node->_func_detail);
  if (ASMJIT_UNLIKELY(err != Error::kOk)) {
    return report_error(err);
  }

  // Allocate space for function arguments.
  func_node->_args = nullptr;
  if (func_node->arg_count() != 0) {
    func_node->_args = _builder_arena.alloc_oneshot<FuncNode::ArgPack>(func_node->arg_count() * sizeof(FuncNode::ArgPack));
    if (ASMJIT_UNLIKELY(!func_node->_args)) {
      return report_error(make_error(Error::kOutOfMemory));
    }
    memset(func_node->_args, 0, func_node->arg_count() * sizeof(FuncNode::ArgPack));
  }

  ASMJIT_PROPAGATE(register_label_node(func_node));

  out = func_node;
  return Error::kOk;
}

Error BaseCompiler::add_func_node(Out<FuncNode*> out, const FuncSignature& signature) {
  State state = _grab_state();

  ASMJIT_PROPAGATE(new_func_node(out, signature));
  Builder_assign_inline_comment(this, *out, state.comment);

  add_func(*out);
  return Error::kOk;
}

Error BaseCompiler::new_func_ret_node(Out<FuncRetNode*> out, const Operand_& o0, const Operand_& o1) {
  uint32_t op_count = !o1.is_none() ? 2u : !o0.is_none() ? 1u : 0u;
  FuncRetNode* node = nullptr;

  ASMJIT_PROPAGATE(new_node_t<FuncRetNode>(Out(node)));
  ASMJIT_ASSUME(node != nullptr);

  node->set_op_count(op_count);
  node->set_op(0, o0);
  node->set_op(1, o1);
  node->reset_op_range(2, node->op_capacity());

  out = node;
  return Error::kOk;
}

Error BaseCompiler::add_func_ret_node(Out<FuncRetNode*> out, const Operand_& o0, const Operand_& o1) {
  State state = _grab_state();

  ASMJIT_PROPAGATE(new_func_ret_node(out, o0, o1));
  Builder_assign_inline_comment(this, *out, state.comment);

  add_node(*out);
  return Error::kOk;
}

FuncNode* BaseCompiler::add_func(FuncNode* func) {
  _func = func;

  add_node(func);             // Function node.
  BaseNode* prev = cursor(); // {CURSOR}.
  add_node(func->exit_node()); // Function exit label.
  add_node(func->end_node());  // Function end sentinel.

  set_cursor(prev);
  return func;
}

Error BaseCompiler::end_func() {
  FuncNode* func = _func;
  reset_state();

  if (ASMJIT_UNLIKELY(!func)) {
    return report_error(make_error(Error::kInvalidState));
  }

  // Add the local constant pool at the end of the function (if exists).
  ConstPoolNode* local_const_pool = _const_pools[uint32_t(ConstPoolScope::kLocal)];
  if (local_const_pool) {
    set_cursor(func->end_node()->prev());
    add_node(local_const_pool);
    _const_pools[uint32_t(ConstPoolScope::kLocal)] = nullptr;
  }

  // Mark as finished.
  _func = nullptr;

  SentinelNode* end = func->end_node();
  set_cursor(end);

  return Error::kOk;
}

// BaseCompiler - Function Invocation
// ==================================

Error BaseCompiler::new_invoke_node(Out<InvokeNode*> out, InstId inst_id, const Operand_& o0, const FuncSignature& signature) {
  InvokeNode* node = nullptr;
  ASMJIT_PROPAGATE(new_node_t<InvokeNode>(Out(node), inst_id, InstOptions::kNone));

  node->set_op_count(1);
  node->set_op(0, o0);
  node->reset_op_range(1, node->op_capacity());

  Error err = node->detail().init(signature, environment());
  if (ASMJIT_UNLIKELY(err != Error::kOk)) {
    return report_error(err);
  }

  // Skip the allocation if there are no arguments.
  uint32_t arg_count = signature.arg_count();
  if (arg_count) {
    node->_args = _builder_arena.alloc_oneshot<InvokeNode::OperandPack>(arg_count * sizeof(InvokeNode::OperandPack));
    if (!node->_args) {
      return report_error(make_error(Error::kOutOfMemory));
    }
    memset(node->_args, 0, arg_count * sizeof(InvokeNode::OperandPack));
  }

  out = node;
  return Error::kOk;
}

Error BaseCompiler::add_invoke_node(Out<InvokeNode*> out, InstId inst_id, const Operand_& o0, const FuncSignature& signature) {
  State state = _grab_state();

  ASMJIT_PROPAGATE(new_invoke_node(out, inst_id, o0, signature));
  Builder_assign_inst_state(this, *out, state);

  add_node(*out);
  return Error::kOk;
}

// BaseCompiler - Virtual Registers
// ================================

Error BaseCompiler::new_virt_reg(Out<VirtReg*> out, TypeId type_id, OperandSignature signature, const char* name) {
  out = nullptr;
  size_t index = _virt_regs.size();

  if (ASMJIT_UNLIKELY(index >= size_t(Operand::kVirtIdCount))) {
    return report_error(make_error(Error::kTooManyVirtRegs));
  }

  if (ASMJIT_UNLIKELY(_virt_regs.reserve_additional(_builder_arena) != Error::kOk)) {
    return report_error(make_error(Error::kOutOfMemory));
  }

  void* virt_reg_ptr = _builder_arena.alloc_oneshot(Arena::aligned_size_of<VirtReg>());
  if (ASMJIT_UNLIKELY(!virt_reg_ptr)) {
    return report_error(make_error(Error::kOutOfMemory));
  }

  uint32_t size = TypeUtils::size_of(type_id);
  uint32_t alignment_log2 = 31 - Support::clz(Support::min<uint32_t>(size, 64) | 1u);

  VirtRegFlags flags = VirtReg::_flags_from_alignment_log2(alignment_log2);
  VirtReg* virt_reg = new(Support::PlacementNew{virt_reg_ptr}) VirtReg(signature.reg_type(), flags, Operand::virt_index_to_virt_id(uint32_t(index)), size, type_id);

#ifndef ASMJIT_NO_LOGGING
  if (name && name[0] != '\0') {
    virt_reg->_name.set_data(_builder_arena, name, SIZE_MAX);
  }
#else
  Support::maybe_unused(name);
#endif

  _virt_regs.append_unchecked(virt_reg);
  out = virt_reg;

  return Error::kOk;
}

Error BaseCompiler::_new_reg_with_name(Out<Reg> out, TypeId type_id, const char* name) {
  OperandSignature reg_signature;
  out->reset();

  Error err = ArchUtils::type_id_to_reg_signature(arch(), type_id, Out(type_id), Out(reg_signature));
  if (ASMJIT_UNLIKELY(err != Error::kOk)) {
    return report_error(err);
  }

  VirtReg* virt_reg;
  ASMJIT_PROPAGATE(new_virt_reg(Out(virt_reg), type_id, reg_signature, name));
  ASMJIT_ASSUME(virt_reg != nullptr);

  out->_init_reg(reg_signature, virt_reg->id());
  return Error::kOk;
}

Error BaseCompiler::_new_reg_with_name(Out<Reg> out, const Reg& ref, const char* name) {
  out->reset();

  OperandSignature reg_signature;
  TypeId type_id;

  if (is_virt_reg_valid(ref)) {
    VirtReg* v_ref = virt_reg_by_reg(ref);
    type_id = v_ref->type_id();

    // NOTE: It's possible to cast one register type to another if it's the same register group. However, VirtReg
    // always contains the TypeId that was used to create the register. This means that in some cases we may end
    // up having different size of `ref` and `v_ref`. In such case we adjust the TypeId to match the `ref` register
    // type instead of the original register type, which should be the expected behavior.
    uint32_t type_size = TypeUtils::size_of(type_id);
    uint32_t ref_size = ref.size();

    if (type_size != ref_size) {
      if (TypeUtils::is_int(type_id)) {
        // GP register - change TypeId to match `ref`, but keep sign of `v_ref`.
        switch (ref_size) {
          case  1: type_id = TypeId(uint32_t(TypeId::kInt8 ) | (uint32_t(type_id) & 1)); break;
          case  2: type_id = TypeId(uint32_t(TypeId::kInt16) | (uint32_t(type_id) & 1)); break;
          case  4: type_id = TypeId(uint32_t(TypeId::kInt32) | (uint32_t(type_id) & 1)); break;
          case  8: type_id = TypeId(uint32_t(TypeId::kInt64) | (uint32_t(type_id) & 1)); break;
          default: type_id = TypeId::kVoid; break;
        }
      }
      else if (TypeUtils::is_mmx(type_id)) {
        // MMX register - always use 64-bit.
        type_id = TypeId::kMmx64;
      }
      else if (TypeUtils::is_mask(type_id)) {
        // Mask register - change TypeId to match `ref` size.
        switch (ref_size) {
          case  1: type_id = TypeId::kMask8; break;
          case  2: type_id = TypeId::kMask16; break;
          case  4: type_id = TypeId::kMask32; break;
          case  8: type_id = TypeId::kMask64; break;
          default: type_id = TypeId::kVoid; break;
        }
      }
      else {
        // Vector register - change TypeId to match `ref` size, keep vector metadata.
        TypeId scalar_type_id = TypeUtils::scalar_of(type_id);
        switch (ref_size) {
          case 16: type_id = TypeUtils::scalar_to_vector(scalar_type_id, TypeId::_kVec128Start); break;
          case 32: type_id = TypeUtils::scalar_to_vector(scalar_type_id, TypeId::_kVec256Start); break;
          case 64: type_id = TypeUtils::scalar_to_vector(scalar_type_id, TypeId::_kVec512Start); break;
          default: type_id = TypeId::kVoid; break;
        }
      }

      if (type_id == TypeId::kVoid) {
        return report_error(make_error(Error::kInvalidState));
      }
    }
  }
  else {
    type_id = RegUtils::type_id_of(ref.reg_type());
  }

  Error err = ArchUtils::type_id_to_reg_signature(arch(), type_id, Out(type_id), Out(reg_signature));
  if (ASMJIT_UNLIKELY(err != Error::kOk)) {
    return report_error(err);
  }

  VirtReg* virt_reg;
  ASMJIT_PROPAGATE(new_virt_reg(Out(virt_reg), type_id, reg_signature, name));
  ASMJIT_ASSUME(virt_reg != nullptr);

  out->_init_reg(reg_signature, virt_reg->id());
  return Error::kOk;
}

Error BaseCompiler::_new_reg_with_vfmt(Out<Reg> out, TypeId type_id, const char* fmt, ...) {
  va_list ap;
  StringTmp<256> sb;

  va_start(ap, fmt);
  sb.append_vformat(fmt, ap);
  va_end(ap);

  return _new_reg(out, type_id, sb.data());
}

Error BaseCompiler::_new_reg_with_vfmt(Out<Reg> out, const Reg& ref, const char* fmt, ...) {
  va_list ap;
  StringTmp<256> sb;

  va_start(ap, fmt);
  sb.append_vformat(fmt, ap);
  va_end(ap);

  return _new_reg(out, ref, sb.data());
}

Error BaseCompiler::_new_stack(Out<BaseMem> out, uint32_t size, uint32_t alignment, const char* name) {
  out->reset();

  if (ASMJIT_UNLIKELY(Support::bool_or(size == 0, !Support::is_zero_or_power_of_2(alignment)))) {
    return report_error(make_error(Error::kInvalidArgument));
  }

  if (alignment == 0u) {
    alignment = 1u;
  }

  if (alignment > 64u) {
    alignment = 64u;
  }

  VirtReg* virt_reg;
  ASMJIT_PROPAGATE(new_virt_reg(Out(virt_reg), TypeId::kVoid, OperandSignature{0}, name));
  ASMJIT_ASSUME(virt_reg != nullptr);

  virt_reg->_virt_size = size;
  virt_reg->_reg_flags |= VirtRegFlags::kIsStackArea | VirtReg::_flags_from_alignment_log2(Support::ctz(alignment));

  // Set the memory operand to GPD/GPQ and its id to VirtReg.
  out = BaseMem(OperandSignature::from_op_type(OperandType::kMem) |
                OperandSignature::from_mem_base_type(_gp_signature.reg_type()) |
                OperandSignature::from_bits(OperandSignature::kMemRegHomeFlag),
                virt_reg->id(), 0, 0);
  return Error::kOk;
}

Error BaseCompiler::set_stack_size(uint32_t virt_id, uint32_t new_size, uint32_t new_alignment) {
  if (!is_virt_id_valid(virt_id)) {
    return make_error(Error::kInvalidVirtId);
  }

  if (!Support::is_zero_or_power_of_2(new_alignment)) {
    return report_error(make_error(Error::kInvalidArgument));
  }

  VirtReg* virt_reg = virt_reg_by_id(virt_id);

  if (new_size) {
    virt_reg->_virt_size = new_size;
  }

  if (new_alignment) {
    uint32_t alignment_log2 = Support::ctz(Support::min<uint32_t>(new_alignment, 64u));
    virt_reg->_reg_flags = (virt_reg->_reg_flags & ~VirtRegFlags::kAlignmentLog2Mask) | VirtReg::_flags_from_alignment_log2(alignment_log2);
  }

  // This is required if the RAPass is already running. There is a chance that a stack-slot has been already
  // allocated and in that case it has to be updated as well, otherwise we would allocate wrong amount of memory.
  RAWorkReg* work_reg = virt_reg->_work_reg;
  if (work_reg && work_reg->_stack_slot) {
    work_reg->_stack_slot->_size = virt_reg->virt_size();
    work_reg->_stack_slot->_alignment = uint8_t(virt_reg->alignment());
  }

  return Error::kOk;
}

Error BaseCompiler::_new_const(Out<BaseMem> out, ConstPoolScope scope, const void* data, size_t size) {
  out->reset();

  if (scope > ConstPoolScope::kMaxValue) {
    return report_error(make_error(Error::kInvalidArgument));
  }

  if (!_const_pools[uint32_t(scope)]) {
    ASMJIT_PROPAGATE(new_const_pool_node(Out(_const_pools[uint32_t(scope)])));
  }

  ConstPoolNode* pool = _const_pools[uint32_t(scope)];
  size_t off;
  Error err = pool->add(data, size, Out(off));

  if (ASMJIT_UNLIKELY(err != Error::kOk)) {
    return report_error(err);
  }

  out = BaseMem(OperandSignature::from_op_type(OperandType::kMem) |
                OperandSignature::from_mem_base_type(RegType::kLabelTag) |
                OperandSignature::from_size(uint32_t(size)),
                pool->label_id(), 0, int32_t(off));
  return Error::kOk;
}

void BaseCompiler::rename(const Reg& reg, const char* fmt, ...) {
  if (!reg.is_virt_reg()) return;

  VirtReg* virt_reg = virt_reg_by_id(reg.id());
  if (!virt_reg) {
    return;
  }

  if (fmt && fmt[0] != '\0') {
    char buf[128];
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(buf, ASMJIT_ARRAY_SIZE(buf), fmt, ap);
    va_end(ap);

    virt_reg->_name.set_data(_builder_arena, buf, SIZE_MAX);
  }
}

// BaseCompiler - Jump Annotations
// ===============================

Error BaseCompiler::new_jump_node(Out<JumpNode*> out, InstId inst_id, InstOptions inst_options, const Operand_& o0, JumpAnnotation* annotation) {
  JumpNode* node = _builder_arena.alloc_oneshot<JumpNode>();

  *out = node;
  if (ASMJIT_UNLIKELY(!node)) {
    return report_error(make_error(Error::kOutOfMemory));
  }

  uint32_t op_count = 1;
  node = new(Support::PlacementNew{node}) JumpNode(inst_id, inst_options, op_count, annotation);
  node->set_op(0, o0);
  node->reset_op_range(op_count, JumpNode::kBaseOpCapacity);

  return Error::kOk;
}

Error BaseCompiler::emit_annotated_jump(InstId inst_id, const Operand_& o0, JumpAnnotation* annotation) {
  State state = _grab_state();

  JumpNode* node;
  ASMJIT_PROPAGATE(new_jump_node(Out(node), inst_id, state.options, o0, annotation));

  node->set_extra_reg(state.extra_reg);
  Builder_assign_inline_comment(this, node, state.comment);

  add_node(node);
  return Error::kOk;
}

JumpAnnotation* BaseCompiler::new_jump_annotation() {
  if (_jump_annotations.reserve_additional(_builder_arena, 1) != Error::kOk) {
    report_error(make_error(Error::kOutOfMemory));
    return nullptr;
  }

  uint32_t id = uint32_t(_jump_annotations.size());
  JumpAnnotation* jump_annotation = _builder_arena.new_oneshot<JumpAnnotation>(this, id);

  if (!jump_annotation) {
    report_error(make_error(Error::kOutOfMemory));
    return nullptr;
  }

  _jump_annotations.append_unchecked(jump_annotation);
  return jump_annotation;
}

// BaseCompiler - Events
// =====================

static ASMJIT_INLINE void BaseCompiler_clear(BaseCompiler* self) noexcept {
  self->_func = nullptr;
  self->_const_pools[uint32_t(ConstPoolScope::kLocal)] = nullptr;
  self->_const_pools[uint32_t(ConstPoolScope::kGlobal)] = nullptr;
  self->_virt_regs.reset();
}

static ASMJIT_INLINE Error BaseCompiler_initDefaultPasses(BaseCompiler* self) noexcept {
  return self->add_pass<GlobalConstPoolPass>();
}


Error BaseCompiler::on_attach(CodeHolder& code) noexcept {
  ASMJIT_PROPAGATE(Base::on_attach(code));

  Error err = BaseCompiler_initDefaultPasses(this);
  if (ASMJIT_UNLIKELY(err != Error::kOk)) {
    on_detach(code);
    return err;
  }
  return Error::kOk;
}

Error BaseCompiler::on_detach(CodeHolder& code) noexcept {
  BaseCompiler_clear(this);
  return Base::on_detach(code);
}

Error BaseCompiler::on_reinit(CodeHolder& code) noexcept {
  BaseCompiler_clear(this);
  Error err = Base::on_reinit(code);

  if (ASMJIT_LIKELY(err == Error::kOk)) {
    err = BaseCompiler_initDefaultPasses(this);
    if (ASMJIT_UNLIKELY(err != Error::kOk)) {
      on_detach(code);
      return err;
    }
  }

  return err;
}

// FuncPass - Construction & Destruction
// =====================================

FuncPass::FuncPass(BaseCompiler& cc, const char* name) noexcept
  : Pass(cc, name) {}

// FuncPass - Run
// ==============

Error FuncPass::run(Arena& arena, Logger* logger) {
  BaseNode* node = cc().first_node();

  while (node) {
    // Find a function by skipping all nodes that are not `NodeType::kFunc`.
    if (node->type() != NodeType::kFunc) {
      node = node->next();
      continue;
    }
    else {
      FuncNode* func = node->as<FuncNode>();
      node = func->end_node();
      ASMJIT_PROPAGATE(run_on_function(arena, logger, func));
    }
  }

  return Error::kOk;
}

// [[pure virtual]]
Error FuncPass::run_on_function(Arena& arena, Logger* logger, FuncNode* func) {
  Support::maybe_unused(arena, logger, func);
  return make_error(Error::kInvalidState);
}

ASMJIT_END_NAMESPACE

#endif // !ASMJIT_NO_COMPILER
