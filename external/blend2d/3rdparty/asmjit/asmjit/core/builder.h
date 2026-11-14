// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_CORE_BUILDER_H_INCLUDED
#define ASMJIT_CORE_BUILDER_H_INCLUDED

#include <asmjit/core/api-config.h>
#ifndef ASMJIT_NO_BUILDER

#include <asmjit/core/assembler.h>
#include <asmjit/core/codeholder.h>
#include <asmjit/core/constpool.h>
#include <asmjit/core/formatter.h>
#include <asmjit/core/inst.h>
#include <asmjit/core/operand.h>
#include <asmjit/core/string.h>
#include <asmjit/core/type.h>
#include <asmjit/support/arena.h>
#include <asmjit/support/arenavector.h>
#include <asmjit/support/support.h>

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_builder
//! \{

class BaseBuilder;
class Pass;

class BaseNode;
class InstNode;
class SectionNode;
class LabelNode;
class AlignNode;
class EmbedDataNode;
class EmbedLabelNode;
class ConstPoolNode;
class CommentNode;
class SentinelNode;
class LabelDeltaNode;

//! Type of node used by \ref BaseBuilder and \ref BaseCompiler.
enum class NodeType : uint8_t {
  //! Invalid node (internal, don't use).
  kNone = 0,

  // [BaseBuilder]

  //! Node is \ref InstNode.
  kInst = 1,
  //! Node is \ref SectionNode.
  kSection = 2,
  //! Node is \ref LabelNode.
  kLabel = 3,
  //! Node is \ref AlignNode.
  kAlign = 4,
  //! Node is \ref EmbedDataNode.
  kEmbedData = 5,
  //! Node is \ref EmbedLabelNode.
  kEmbedLabel = 6,
  //! Node is \ref EmbedLabelDeltaNode.
  kEmbedLabelDelta = 7,
  //! Node is \ref ConstPoolNode.
  kConstPool = 8,
  //! Node is \ref CommentNode.
  kComment = 9,
  //! Node is \ref SentinelNode.
  kSentinel = 10,

  // [BaseCompiler]

  //! Node is \ref JumpNode (acts as InstNode).
  kJump = 15,
  //! Node is \ref FuncNode (acts as LabelNode).
  kFunc = 16,
  //! Node is \ref FuncRetNode (acts as InstNode).
  kFuncRet = 17,
  //! Node is \ref InvokeNode (acts as InstNode).
  kInvoke = 18,

  // [UserDefined]

  //! First id of a user-defined node.
  kUser = 32
};

//! Node flags, specify what the node is and/or does.
enum class NodeFlags : uint8_t {
  //! No flags.
  kNone = 0,
  //! Node is code that can be executed (instruction, label, align, etc...).
  kIsCode = 0x01u,
  //! Node is data that cannot be executed (data, const-pool, etc...).
  kIsData = 0x02u,
  //! Node is informative, can be removed and ignored.
  kIsInformative = 0x04u,
  //! Node can be safely removed if unreachable.
  kIsRemovable = 0x08u,
  //! Node does nothing when executed (label, align, explicit nop).
  kHasNoEffect = 0x10u,
  //! Node is an instruction or acts as it.
  kActsAsInst = 0x20u,
  //! Node is a label or acts as it.
  kActsAsLabel = 0x40u,
  //! Node is active (part of the code).
  kIsActive = 0x80u
};
ASMJIT_DEFINE_ENUM_FLAGS(NodeFlags)

//! Node position represents a unique position of a node (most likely an \ref InstNode) in code.
enum class NodePosition : uint32_t {};

//! Type of the sentinel (purely informative purpose).
enum class SentinelType : uint8_t {
  //! Type of the sentinel is not known.
  kUnknown = 0u,
  //! This is a sentinel used at the end of \ref FuncNode.
  kFuncEnd = 1u
};

//! Node list.
//!
//! A double-linked list of pointers to \ref BaseNode, managed by \ref BaseBuilder or \ref BaseCompiler.
//!
//! \note At the moment NodeList is just a view, but it's planned that it will get more functionality in the future.
class NodeList {
public:
  //! \name Members
  //! \{

  //! First node in the list or nullptr if there are no nodes in the list.
  BaseNode* _first = nullptr;
  //! Last node in the list or nullptr if there are no nodes in the list.
  BaseNode* _last = nullptr;

  //! \}

  //! \name Construction & Destruction
  //! \{

  ASMJIT_INLINE_NODEBUG NodeList() noexcept {}

  ASMJIT_INLINE_NODEBUG NodeList(BaseNode* first, BaseNode* last) noexcept
    : _first(first),
      _last(last) {}

  //! \}

  //! \name Reset
  //! \{

  ASMJIT_INLINE_NODEBUG void reset() noexcept {
    _first = nullptr;
    _last = nullptr;
  }

  ASMJIT_INLINE_NODEBUG void reset(BaseNode* first, BaseNode* last) noexcept {
    _first = first;
    _last = last;
  }

  //! \}

  //! \name Accessors
  //! \{

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_empty() const noexcept { return _first == nullptr; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG BaseNode* first() const noexcept { return _first; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG BaseNode* last() const noexcept { return _last; }

  //! \}
};

//! Builder interface.
//!
//! `BaseBuilder` interface was designed to be used as a \ref BaseAssembler replacement in case pre-processing or
//! post-processing of the generated code is required. The code can be modified during or after code generation.
//! Pre processing or post processing can be done manually or through a \ref Pass object. \ref BaseBuilder stores
//! the emitted code as a double-linked list of nodes, which allows O(1) insertion and removal during processing.
//!
//! Check out architecture specific builders for more details and examples:
//!
//!   - \ref x86::Builder - X86/X64 builder implementation.
//!   - \ref a64::Builder - AArch64 builder implementation.
class ASMJIT_VIRTAPI BaseBuilder : public BaseEmitter {
public:
  ASMJIT_NONCOPYABLE(BaseBuilder)
  using Base = BaseEmitter;

  //! \name Members
  //! \{

  //! Arena used to allocate nodes and passes.
  Arena _builder_arena;
  //! Arena only used by `Pass::run()`.
  Arena _pass_arena;

  //! Array of `Pass` objects.
  ArenaVector<Pass*> _passes;
  //! Maps section indexes to `LabelNode` nodes.
  ArenaVector<SectionNode*> _section_nodes;
  //! Maps label indexes to `LabelNode` nodes.
  ArenaVector<LabelNode*> _label_nodes;

  //! Current node (cursor).
  BaseNode* _cursor = nullptr;
  //! First and last nodes.
  NodeList _node_list;

  //! The sections links are dirty (used internally).
  bool _dirty_section_links = false;

  //! \}

  //! \name Construction & Destruction
  //! \{

  //! Creates a new `BaseBuilder` instance.
  ASMJIT_API BaseBuilder() noexcept;
  //! Destroys the `BaseBuilder` instance.
  ASMJIT_API ~BaseBuilder() noexcept override;

  //! \}

  //! \name Node Management
  //! \{

  //! Returns first and last node of Builder/Compiler wrapped in \ref NodeList.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG NodeList node_list() const noexcept { return _node_list; }

  //! Returns the first node.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG BaseNode* first_node() const noexcept { return _node_list.first(); }

  //! Returns the last node.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG BaseNode* last_node() const noexcept { return _node_list.last(); }

  //! Allocates data required for a node.
  template<typename T, typename... Args>
  ASMJIT_INLINE Error new_node_with_size_t(Out<T*> out, size_t size, Args&&... args) {
    ASMJIT_ASSERT(Support::is_aligned(size, Arena::kAlignment));

    void* ptr =_builder_arena.alloc_oneshot(size);
    if (ASMJIT_UNLIKELY(!ptr)) {
      return report_error(make_error(Error::kOutOfMemory));
    }

    out = new(Support::PlacementNew{ptr}) T(std::forward<Args>(args)...);
    return Error::kOk;
  }

  //! Allocates and instantiates a new node of type `T` and returns its instance. If the allocation fails `nullptr`
  //! is returned.
  //!
  //! The template argument `T` must be a type that is extends \ref BaseNode.
  //!
  //! \remarks The pointer returned (if non-null) is owned by the Builder or Compiler. When the Builder/Compiler
  //! is destroyed it destroys all nodes it created so no manual memory management is required.
  template<typename T, typename... Args>
  ASMJIT_INLINE Error new_node_t(Out<T*> out, Args&&... args) {
    void* ptr = _builder_arena.alloc_oneshot(Arena::aligned_size_of<T>());

    if (ASMJIT_UNLIKELY(!ptr)) {
      return report_error(make_error(Error::kOutOfMemory));
    }

    out = new(Support::PlacementNew{ptr}) T(std::forward<Args>(args)...);
    return Error::kOk;
  }

  //! Creates a new \ref InstNode.
  ASMJIT_API Error new_inst_node(Out<InstNode*> out, InstId inst_id, InstOptions inst_options, uint32_t op_count);
  //! Creates a new \ref LabelNode.
  ASMJIT_API Error new_label_node(Out<LabelNode*> out);
  //! Creates a new \ref AlignNode.
  ASMJIT_API Error new_align_node(Out<AlignNode*> out, AlignMode align_mode, uint32_t alignment);
  //! Creates a new \ref EmbedDataNode.
  ASMJIT_API Error new_embed_data_node(Out<EmbedDataNode*> out, TypeId type_id, const void* data, size_t item_count, size_t repeat_count = 1);
  //! Creates a new \ref ConstPoolNode.
  ASMJIT_API Error new_const_pool_node(Out<ConstPoolNode*> out);
  //! Creates a new \ref CommentNode.
  ASMJIT_API Error new_comment_node(Out<CommentNode*> out, const char* data, size_t size);

  //! Adds `node` after the current and sets the current node to the given `node`.
  ASMJIT_API BaseNode* add_node(BaseNode* ASMJIT_NONNULL(node)) noexcept;
  //! Inserts the given `node` after `ref`.
  ASMJIT_API BaseNode* add_after(BaseNode* ASMJIT_NONNULL(node), BaseNode* ASMJIT_NONNULL(ref)) noexcept;
  //! Inserts the given `node` before `ref`.
  ASMJIT_API BaseNode* add_before(BaseNode* ASMJIT_NONNULL(node), BaseNode* ASMJIT_NONNULL(ref)) noexcept;
  //! Removes the given `node`.
  ASMJIT_API BaseNode* remove_node(BaseNode* ASMJIT_NONNULL(node)) noexcept;
  //! Removes multiple nodes.
  ASMJIT_API void remove_nodes(BaseNode* first, BaseNode* last) noexcept;

  //! Returns the cursor.
  //!
  //! When the Builder/Compiler is created it automatically creates a '.text' \ref SectionNode, which will be the
  //! initial one. When instructions are added they are always added after the cursor and the cursor is changed
  //! to be that newly added node. Use `set_cursor()` to change where new nodes are inserted.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG BaseNode* cursor() const noexcept { return _cursor; }

  //! Sets the current cursor to `node` and returns the previous one.
  //!
  //! \remarks This function returns the previous cursor for convenience, but the return value can be safely ignored
  //! in case it's not important for the user.
  ASMJIT_INLINE_NODEBUG BaseNode* set_cursor(BaseNode* node) noexcept {
    BaseNode* old = _cursor;
    _cursor = node;
    return old;
  }

  //! \}

  //! \name Section Management
  //! \{

  //! Returns a vector of SectionNode objects.
  //!
  //! \note If a section of some id is not associated with the Builder/Compiler it would be null, so always check
  //! for nulls if you iterate over the vector.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Span<SectionNode*> section_nodes() const noexcept {
    return _section_nodes.as_span();
  }

  //! Tests whether the `SectionNode` of the given `section_id` was registered.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_registered_section_node(uint32_t section_id) const noexcept {
    return section_id < _section_nodes.size() && _section_nodes[section_id] != nullptr;
  }

  //! Returns or creates a `SectionNode` that matches the given `section_id`.
  //!
  //! \remarks This function will either get the existing `SectionNode` or create it in case it wasn't created before.
  //! You can check whether a section has a registered `SectionNode` by using `BaseBuilder::has_registered_section_node()`.
  ASMJIT_API Error section_node_of(Out<SectionNode*> out, uint32_t section_id);

  ASMJIT_API Error section(Section* ASMJIT_NONNULL(section)) override;

  //! Returns whether the section links of active section nodes are dirty. You can update these links by calling
  //! `update_section_links()` in such case.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_dirty_section_links() const noexcept { return _dirty_section_links; }

  //! Updates links of all active section nodes.
  ASMJIT_API void update_section_links() noexcept;

  //! \}

  //! \name Label Management
  //! \{

  //! Returns a vector of \ref LabelNode nodes.
  //!
  //! \note If a label of some id is not associated with the Builder/Compiler it would be null, so always check for
  //! nulls if you iterate over the vector.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Span<LabelNode*> label_nodes() const noexcept { return _label_nodes.as_span(); }

  //! Tests whether the `LabelNode` of the given `label_id` was registered.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_registered_label_node(uint32_t label_id) const noexcept {
    return label_id < _label_nodes.size() && _label_nodes[label_id] != nullptr;
  }

  //! \overload
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_registered_label_node(const Label& label) const noexcept {
    return has_registered_label_node(label.id());
  }

  //! Gets or creates a \ref LabelNode that matches the given `label_id`.
  //!
  //! \remarks This function will either get the existing `LabelNode` or create it in case it wasn't created before.
  //! You can check whether a label has a registered `LabelNode` by calling \ref BaseBuilder::has_registered_label_node().
  ASMJIT_API Error label_node_of(Out<LabelNode*> out, uint32_t label_id);

  //! \overload
  ASMJIT_INLINE_NODEBUG Error label_node_of(Out<LabelNode*> out, const Label& label) {
    return label_node_of(out, label.id());
  }

  //! Registers this \ref LabelNode (internal).
  //!
  //! This function is used internally to register a newly created `LabelNode` with this instance of Builder/Compiler.
  //! Use \ref label_node_of() functions to get back \ref LabelNode from a label or its identifier.
  [[nodiscard]]
  ASMJIT_API Error register_label_node(LabelNode* ASMJIT_NONNULL(node));

  [[nodiscard]]
  ASMJIT_API Label new_label() override;

  [[nodiscard]]
  ASMJIT_API Label new_named_label(const char* name, size_t name_size = SIZE_MAX, LabelType type = LabelType::kGlobal, uint32_t parent_id = Globals::kInvalidId) override;

  [[nodiscard]]
  ASMJIT_INLINE Label new_named_label(Span<const char> name, LabelType type = LabelType::kGlobal, uint32_t parent_id = Globals::kInvalidId) {
    return new_named_label(name.data(), name.size(), type, parent_id);
  }

  ASMJIT_API Error bind(const Label& label) override;

  //! \}

  //! \name Passes
  //! \{

  //! Returns a vector of `Pass` instances that will be executed by `run_passes()`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Span<Pass*> passes() const noexcept { return _passes.as_span(); }

  //! Allocates and instantiates a new pass of type `T` and returns its instance. If the allocation fails `nullptr` is
  //! returned.
  //!
  //! The template argument `T` must be a type that is extends \ref Pass.
  //!
  //! \remarks The pointer returned (if non-null) is owned by the Builder or Compiler. When the Builder/Compiler is
  //! destroyed it destroys all passes it created so no manual memory management is required.
  template<typename PassT, typename... Args>
  [[nodiscard]]
  ASMJIT_INLINE PassT* new_pass(Args&&... args) noexcept { return _builder_arena.new_oneshot<PassT>(*this, std::forward<Args>(args)...); }

  template<typename T, typename... Args>
  ASMJIT_INLINE Error add_pass(Args&&... args) { return _add_pass(new_pass<T, Args...>(std::forward<Args>(args)...)); }

  //! Returns `Pass` by name.
  //!
  //! If the pass having the given `name` doesn't exist `nullptr` is returned.
  [[nodiscard]]
  ASMJIT_API Pass* pass_by_name(const char* name) const noexcept;

  //! Adds `pass` to the list of passes.
  ASMJIT_API Error _add_pass(Pass* pass) noexcept;

  //! Runs all passes in order.
  ASMJIT_API Error run_passes();

  //! \}

  //! \name Emit
  //! \{

  ASMJIT_API Error _emit(InstId inst_id, const Operand_& o0, const Operand_& o1, const Operand_& o2, const Operand_* op_ext) override;

  //! \}

  //! \name Align
  //! \{

  ASMJIT_API Error align(AlignMode align_mode, uint32_t alignment) override;

  //! \}

  //! \name Embed
  //! \{

  ASMJIT_API Error embed(const void* data, size_t data_size) override;
  ASMJIT_API Error embed_data_array(TypeId type_id, const void* data, size_t count, size_t repeat = 1) override;
  ASMJIT_API Error embed_const_pool(const Label& label, const ConstPool& pool) override;

  ASMJIT_API Error embed_label(const Label& label, size_t data_size = 0) override;
  ASMJIT_API Error embed_label_delta(const Label& label, const Label& base, size_t data_size = 0) override;

  //! \}

  //! \name Comment
  //! \{

  ASMJIT_API Error comment(const char* data, size_t size = SIZE_MAX) override;

  ASMJIT_INLINE Error comment(Span<const char> data) { return comment(data.data(), data.size()); }

  //! \}

  //! \name Serialization
  //! \{

  //! Serializes everything the given emitter `dst`.
  //!
  //! Although not explicitly required the emitter will most probably be of Assembler type. The reason is that
  //! there is no known use of serializing nodes held by Builder/Compiler into another Builder-like emitter.
  ASMJIT_API Error serialize_to(BaseEmitter* dst);

  //! \}

  //! \name Events
  //! \{

  ASMJIT_API Error on_attach(CodeHolder& code) noexcept override;
  ASMJIT_API Error on_detach(CodeHolder& code) noexcept override;
  ASMJIT_API Error on_reinit(CodeHolder& code) noexcept override;

  //! \}
};

//! Base node.
//!
//! Every node represents a building-block used by \ref BaseBuilder. It can be instruction, data, label, comment,
//! directive, or any other high-level representation that can be transformed to the building blocks mentioned.
//! Every class that inherits \ref BaseBuilder can define its own high-level nodes that can be later lowered to
//! basic nodes like instructions.
class BaseNode {
public:
  ASMJIT_NONCOPYABLE(BaseNode)

  //! \name Members
  //! \{

  union {
    struct {
      //! Previous node.
      BaseNode* _prev;
      //! Next node.
      BaseNode* _next;
    };
    //! Links (an alternative view of previous and next nodes).
    BaseNode* _links[2];
  };

  //! Node type.
  NodeType _node_type;
  //! Node flags.
  NodeFlags _node_flags;

  //! Data shared between all types of nodes.
  struct AnyData {
    //! Not used by BaseNode.
    uint8_t _reserved_0;
    //! Not used by BaseNode.
    uint8_t _reserved_1;
  };

  //! Data used by \ref AlignNode.
  struct AlignData {
    //! Align mode.
    AlignMode _align_mode;
    //! Not used by AlignNode.
    uint8_t _reserved;
  };

  //! Data used by \ref InstNode.
  struct InstData {
    //! Instruction operands count (used).
    uint8_t _op_count;
    //! Instruction operands capacity (allocated).
    uint8_t _op_capacity;
  };

  //! Data used by \ref EmbedDataNode.
  struct EmbedData {
    //! Type id.
    TypeId _type_id;
    //! Size of `_type_id`.
    uint8_t _type_size;
  };

  //! Data used by \ref SentinelNode.
  struct SentinelData {
    //! Sentinel type.
    SentinelType _sentinel_type;
    //! Not used by BaseNode.
    uint8_t _reserved_1;
  };

  //! Data that can have different meaning depending on \ref NodeType.
  union {
    //! Data useful by any node type.
    AnyData _any;
    //! Data specific to \ref AlignNode.
    AlignData _align_data;
    //! Data specific to \ref InstNode.
    InstData _inst;
    //! Data specific to \ref EmbedDataNode.
    EmbedData _embed;
    //! Data specific to \ref SentinelNode.
    SentinelData _sentinel;
  };

  //! Node position in code (should be unique).
  NodePosition _position;

#if !defined(ASMJIT_NO_NODE_USERDATA)
  //! Value reserved for AsmJit users never touched by AsmJit itself.
  union {
    //! User data as 64-bit integer.
    uint64_t _user_data_u64;
    //! User data as pointer.
    void* _user_data_ptr;
  };
#endif

  //! Data used exclusively by the current `Pass`.
  void* _pass_data;

  //! Inline comment/annotation or nullptr if not used.
  const char* _inline_comment;

  //! \}

  //! \name Construction & Destruction
  //! \{

  //! Creates a new `BaseNode` - always use `BaseBuilder` to allocate nodes.
  ASMJIT_INLINE_NODEBUG explicit BaseNode(NodeType node_type, NodeFlags node_flags = NodeFlags::kNone) noexcept {
    _prev = nullptr;
    _next = nullptr;
    _node_type = node_type;
    _node_flags = node_flags;
    _any._reserved_0 = 0;
    _any._reserved_1 = 0;
    _position = NodePosition(0);
#if !defined(ASMJIT_NO_NODE_USERDATA)
    _user_data_u64 = 0;
#endif
    _pass_data = nullptr;
    _inline_comment = nullptr;
  }

  //! \}

  //! \name Accessors
  //! \{

  //! Casts this node to `T*`.
  template<typename T>
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG T* as() noexcept { return static_cast<T*>(this); }

  //! Casts this node to `const T*`.
  template<typename T>
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const T* as() const noexcept { return static_cast<const T*>(this); }

  //! Returns previous node or `nullptr` if this node is either first or not part of Builder/Compiler node-list.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG BaseNode* prev() const noexcept { return _prev; }

  //! Returns next node or `nullptr` if this node is either last or not part of Builder/Compiler node-list.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG BaseNode* next() const noexcept { return _next; }

  //! Returns the type of the node, see \ref NodeType.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG NodeType type() const noexcept { return _node_type; }

  //! Sets the type of the node, see `NodeType` (internal).
  //!
  //! \remarks You should never set a type of a node to anything else than the initial value. This function is only
  //! provided for users that use custom nodes and need to change the type either during construction or later.
  ASMJIT_INLINE_NODEBUG void _set_type(NodeType type) noexcept { _node_type = type; }

  //! Tests whether this node is either `InstNode` or extends it.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_inst() const noexcept { return has_flag(NodeFlags::kActsAsInst); }

  //! Tests whether this node is `SectionNode`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_section() const noexcept { return type() == NodeType::kSection; }

  //! Tests whether this node is either `LabelNode` or extends it.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_label() const noexcept { return has_flag(NodeFlags::kActsAsLabel); }

  //! Tests whether this node is `AlignNode`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_align() const noexcept { return type() == NodeType::kAlign; }

  //! Tests whether this node is `EmbedDataNode`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_embed_data() const noexcept { return type() == NodeType::kEmbedData; }

  //! Tests whether this node is `EmbedLabelNode`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_embed_label() const noexcept { return type() == NodeType::kEmbedLabel; }

  //! Tests whether this node is `EmbedLabelDeltaNode`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_embed_label_delta() const noexcept { return type() == NodeType::kEmbedLabelDelta; }

  //! Tests whether this node is `ConstPoolNode`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_const_pool() const noexcept { return type() == NodeType::kConstPool; }

  //! Tests whether this node is `CommentNode`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_comment() const noexcept { return type() == NodeType::kComment; }

  //! Tests whether this node is `SentinelNode`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_sentinel() const noexcept { return type() == NodeType::kSentinel; }

  //! Tests whether this node is `FuncNode`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_func() const noexcept { return type() == NodeType::kFunc; }

  //! Tests whether this node is `FuncRetNode`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_func_ret() const noexcept { return type() == NodeType::kFuncRet; }

  //! Tests whether this node is `InvokeNode`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_invoke() const noexcept { return type() == NodeType::kInvoke; }

  //! Returns the node flags.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG NodeFlags flags() const noexcept { return _node_flags; }

  //! Tests whether the node has the given `flag` set.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_flag(NodeFlags flag) const noexcept { return Support::test(_node_flags, flag); }

  //! Replaces node flags with `flags`.
  ASMJIT_INLINE_NODEBUG void _assign_flags(NodeFlags flags) noexcept { _node_flags = flags; }

  //! Adds the given `flags` to node flags.
  ASMJIT_INLINE_NODEBUG void _add_flags(NodeFlags flags) noexcept { _node_flags |= flags; }

  //! Clears the given `flags` from node flags.
  ASMJIT_INLINE_NODEBUG void _clear_flags(NodeFlags flags) noexcept { _node_flags &= ~flags; }

  //! Tests whether the node is code that can be executed.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_code() const noexcept { return has_flag(NodeFlags::kIsCode); }

  //! Tests whether the node is data that cannot be executed.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_data() const noexcept { return has_flag(NodeFlags::kIsData); }

  //! Tests whether the node is informative only (is never encoded like comment, etc...).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_informative() const noexcept { return has_flag(NodeFlags::kIsInformative); }

  //! Tests whether the node is removable if it's in an unreachable code block.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_removable() const noexcept { return has_flag(NodeFlags::kIsRemovable); }

  //! Tests whether the node has no effect when executed (label, .align, nop, ...).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_no_effect() const noexcept { return has_flag(NodeFlags::kHasNoEffect); }

  //! Tests whether the node is part of the code.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_active() const noexcept { return has_flag(NodeFlags::kIsActive); }

  //! Tests whether the node has a position assigned.
  //!
  //! \remarks Returns `true` if node position is non-zero.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_position() const noexcept { return _position != NodePosition(0); }

  //! Returns node position.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG NodePosition position() const noexcept { return _position; }

  //! Sets node position.
  //!
  //! Node position is a 32-bit unsigned integer that is used by Compiler to track where the node is relatively to
  //! the start of the function. It doesn't describe a byte position in a binary, instead it's just a pseudo position
  //! used by liveness analysis and other tools around Compiler.
  //!
  //! If you don't use Compiler then you may use `position()` and `set_position()` freely for your own purposes if
  //! the 32-bit value limit is okay for you.
  ASMJIT_INLINE_NODEBUG void set_position(NodePosition position) noexcept { _position = position; }

#if !defined(ASMJIT_NO_NODE_USERDATA)
  //! Returns user data casted to `T*`.
  //!
  //! User data is dedicated to be used only by AsmJit users and not touched by the library. The data is of a pointer
  //! size so you can either store a pointer or `int64_t` value through `set_user_data_as_ptr()`, `set_user_data_as_int64()`
  //! and `set_user_data_as_uint64()`.
  template<typename T>
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG T* user_data_as_ptr() const noexcept { return static_cast<T*>(_user_data_ptr); }

  //! Returns user data casted to `int64_t`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG int64_t user_data_as_int64() const noexcept { return int64_t(_user_data_u64); }

  //! Returns user data casted to `uint64_t`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint64_t user_data_as_uint64() const noexcept { return _user_data_u64; }

  //! Sets user data to `data`.
  template<typename T>
  ASMJIT_INLINE_NODEBUG void set_user_data_as_ptr(T* data) noexcept { _user_data_ptr = static_cast<void*>(data); }
  //! Sets used data to the given 64-bit signed `value`.
  ASMJIT_INLINE_NODEBUG void set_user_data_as_int64(int64_t value) noexcept { _user_data_u64 = uint64_t(value); }
  //! Sets used data to the given 64-bit unsigned `value`.
  ASMJIT_INLINE_NODEBUG void set_user_data_as_uint64(uint64_t value) noexcept { _user_data_u64 = value; }

  //! Resets user data to zero / nullptr.
  ASMJIT_INLINE_NODEBUG void reset_user_data() noexcept { _user_data_u64 = 0; }
#endif

  //! Tests whether the node has an associated pass data.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_pass_data() const noexcept { return _pass_data != nullptr; }

  //! Returns the node pass data - data used during processing & transformations.
  template<typename T>
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG T* pass_data() const noexcept { return (T*)_pass_data; }

  //! Sets the node pass data to `data`.
  template<typename T>
  ASMJIT_INLINE_NODEBUG void set_pass_data(T* data) noexcept { _pass_data = (void*)data; }
  //! Resets the node pass data to nullptr.
  ASMJIT_INLINE_NODEBUG void reset_pass_data() noexcept { _pass_data = nullptr; }

  //! Tests whether the node has an inline comment/annotation.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_inline_comment() const noexcept { return _inline_comment != nullptr; }

  //! Returns an inline comment/annotation string.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const char* inline_comment() const noexcept { return _inline_comment; }

  //! Sets an inline comment/annotation string to `s`.
  ASMJIT_INLINE_NODEBUG void set_inline_comment(const char* s) noexcept { _inline_comment = s; }
  //! Resets an inline comment/annotation string to nullptr.
  ASMJIT_INLINE_NODEBUG void reset_inline_comment() noexcept { _inline_comment = nullptr; }

  //! \}
};

//! Instruction node.
//!
//! Wraps an instruction with its options and operands.
class InstNode : public BaseNode {
public:
  ASMJIT_NONCOPYABLE(InstNode)

  //! \name Constants
  //! \{

  //! The number of embedded operands for a default \ref InstNode instance that are always allocated as a part of
  //! the instruction itself. Minimum embedded operands is 4, but in 32-bit more pointers are smaller and we can
  //! embed 5. The rest (up to 6 operands) is considered extended.
  //!
  //! The number of operands InstNode holds is decided when \ref InstNode is created.
  static inline constexpr uint32_t kBaseOpCapacity = 3u;

  //! Count of maximum number of operands \ref InstNode can hold.
  static inline constexpr uint32_t kFullOpCapacity = Globals::kMaxOpCount;

  //! \}

  //! \cond INTERNAL
  //! \name Operand Capacity Utilities
  //! \{

  //! Returns the capacity required for the given operands count `op_count`.
  //!
  //! There are only two capacities used - \ref kBaseOpCapacity and \ref kFullOpCapacity, so this function
  //! is used to decide between these two. The general rule is that instructions that can be represented with
  //! \ref kBaseOpCapacity would use this value, and all others would take \ref kFullOpCapacity.
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR uint32_t capacity_of_op_count(uint32_t op_count) noexcept {
    return op_count <= kBaseOpCapacity ? kBaseOpCapacity : kFullOpCapacity;
  }

  //! Calculates the size of \ref InstNode required to hold at most `op_capacity` operands.
  //!
  //! This function is used internally to allocate \ref InstNode.
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR size_t node_size_of_op_capacity(uint32_t op_capacity) noexcept {
    return Arena::aligned_size(sizeof(InstNode) + op_capacity * sizeof(Operand));
  }

  //! \}
  //! \endcond

  //! \name Members
  //! \{

  //! Base instruction data.
  BaseInst _base_inst;

  //! \}

  //! \name Construction & Destruction
  //! \{

  //! Creates a new `InstNode` instance.
  ASMJIT_INLINE_NODEBUG InstNode(InstId inst_id, InstOptions options, uint32_t op_count, uint32_t op_capacity = kBaseOpCapacity) noexcept
    : BaseNode(NodeType::kInst, NodeFlags::kIsCode | NodeFlags::kIsRemovable | NodeFlags::kActsAsInst),
      _base_inst(inst_id, options) {
    _inst._op_capacity = uint8_t(op_capacity);
    _inst._op_count = uint8_t(op_count);
  }

  //! \cond INTERNAL
  //! Reset all built-in operands, including `extra_reg`.
  ASMJIT_INLINE_NODEBUG void _reset_ops() noexcept {
    _base_inst.reset_extra_reg();
    reset_op_range(0, op_capacity());
  }
  //! \endcond

  //! \}

  //! \name Instruction Object
  //! \{

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG BaseInst& baseInst() noexcept { return _base_inst; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const BaseInst& baseInst() const noexcept { return _base_inst; }

  //! \}

  //! \name Instruction Id
  //! \{

  //! Returns the instruction id, see `BaseInst::Id`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG InstId inst_id() const noexcept { return _base_inst.inst_id(); }

  //! Returns the instruction real id, see `BaseInst::Id`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG InstId real_id() const noexcept { return _base_inst.real_id(); }

  //! Sets the instruction id to `id`, see `BaseInst::Id`.
  ASMJIT_INLINE_NODEBUG void set_inst_id(InstId id) noexcept { _base_inst.set_inst_id(id); }

  //! \}

  //! \name Instruction Options
  //! \{

  //! Returns instruction options, see \ref InstOptions for more details.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG InstOptions options() const noexcept { return _base_inst.options(); }

  //! Tests whether instruction has the given `option` set/enabled.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_option(InstOptions option) const noexcept { return _base_inst.has_option(option); }

  //! Sets instruction `options` to the provided value, resetting all others.
  ASMJIT_INLINE_NODEBUG void set_options(InstOptions options) noexcept { _base_inst.set_options(options); }
  //! Adds instruction `options` to the instruction.
  ASMJIT_INLINE_NODEBUG void add_options(InstOptions options) noexcept { _base_inst.add_options(options); }
  //! Clears instruction `options` of the instruction (disables the given options).
  ASMJIT_INLINE_NODEBUG void clear_options(InstOptions options) noexcept { _base_inst.clear_options(options); }
  //! Resets instruction options to none - disabling all instruction options.
  ASMJIT_INLINE_NODEBUG void reset_options() noexcept { _base_inst.reset_options(); }

  //! \}

  //! \name Extra Register
  //! \{

  //! Tests whether the node has an extra register operand.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_extra_reg() const noexcept { return _base_inst.has_extra_reg(); }

  //! Returns extra register operand.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RegOnly& extra_reg() noexcept { return _base_inst.extra_reg(); }

  //! \overload
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const RegOnly& extra_reg() const noexcept { return _base_inst.extra_reg(); }

  //! Sets extra register operand to `reg`.
  ASMJIT_INLINE_NODEBUG void set_extra_reg(const Reg& reg) noexcept { _base_inst.set_extra_reg(reg); }
  //! Sets extra register operand to `reg`.
  ASMJIT_INLINE_NODEBUG void set_extra_reg(const RegOnly& reg) noexcept { _base_inst.set_extra_reg(reg); }
  //! Resets extra register operand.
  ASMJIT_INLINE_NODEBUG void reset_extra_reg() noexcept { _base_inst.reset_extra_reg(); }

  //! \}

  //! \name Instruction Operands
  //! \{

  //! Returns all operands instruction uses as `Span<Operand>`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Span<Operand> operands() noexcept { return Span<Operand>(operands_data(), _inst._op_count); }

  //! Returns all operands instruction uses as `Span<const Operand>`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Span<const Operand> operands() const noexcept { return Span<const Operand>(operands_data(), _inst._op_count); }

  //! Returns operands array.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Operand* operands_data() noexcept { return Support::offset_ptr<Operand>(this, sizeof(InstNode)); }

  //! Returns operands array (const).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const Operand* operands_data() const noexcept { return Support::offset_ptr<Operand>(this, sizeof(InstNode)); }

  //! Returns operand count.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t op_count() const noexcept { return _inst._op_count; }

  //! Returns operand capacity.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t op_capacity() const noexcept { return _inst._op_capacity; }

  //! Sets operand count.
  ASMJIT_INLINE_NODEBUG void set_op_count(size_t op_count) noexcept { _inst._op_count = uint8_t(op_count); }

  //! Returns operand at the given `index`.
  [[nodiscard]]
  inline Operand& op(size_t index) noexcept {
    ASMJIT_ASSERT(index < op_capacity());

    Operand* ops = operands_data();
    return ops[index].as<Operand>();
  }

  //! Returns operand at the given `index` (const).
  [[nodiscard]]
  inline const Operand& op(size_t index) const noexcept {
    ASMJIT_ASSERT(index < op_capacity());

    const Operand* ops = operands_data();
    return ops[index].as<Operand>();
  }

  //! Sets operand at the given `index` to `op`.
  inline void set_op(size_t index, const Operand_& op) noexcept {
    ASMJIT_ASSERT(index < op_capacity());

    Operand* ops = operands_data();
    ops[index].copy_from(op);
  }

  //! Resets operand at the given `index` to none.
  inline void reset_op(size_t index) noexcept {
    ASMJIT_ASSERT(index < op_capacity());

    Operand* ops = operands_data();
    ops[index].reset();
  }

  //! Resets operands at `[start, end)` range.
  inline void reset_op_range(size_t start, size_t end) noexcept {
    Operand* ops = operands_data();
    for (size_t i = start; i < end; i++)
      ops[i].reset();
  }

  //! \}

  //! \name Utilities
  //! \{

  //! Tests whether the given operand type `op_type` is used by the instruction.
  [[nodiscard]]
  inline bool has_op_type(OperandType op_type) const noexcept {
    for (const Operand& op : operands()) {
      if (op.op_type() == op_type) {
        return true;
      }
    }
    return false;
  }

  //! Tests whether the instruction uses at least one register operand.
  [[nodiscard]]
  inline bool has_reg_op() const noexcept { return has_op_type(OperandType::kReg); }

  //! Tests whether the instruction uses at least one memory operand.
  [[nodiscard]]
  inline bool has_mem_op() const noexcept { return has_op_type(OperandType::kMem); }

  //! Tests whether the instruction uses at least one immediate operand.
  [[nodiscard]]
  inline bool has_imm_op() const noexcept { return has_op_type(OperandType::kImm); }

  //! Tests whether the instruction uses at least one label operand.
  [[nodiscard]]
  inline bool has_label_op() const noexcept { return has_op_type(OperandType::kLabel); }

  //! Returns the index of the given operand type `op_type`.
  //!
  //! \note If the operand type wa found, the value returned represents its index in \ref operands()
  //! array, otherwise `SIZE_MAX` is returned to signalize that the operand was not found.
  [[nodiscard]]
  inline size_t index_of_op_type(OperandType op_type) const noexcept {
    Span<const Operand> ops = operands();
    for (size_t i = 0u; i < ops.size(); i++) {
      if (ops[i].op_type() == op_type)
        return i;
    }

    return SIZE_MAX;
  }

  //! A shortcut that calls `index_of_op_type(OperandType::kMem)`.
  [[nodiscard]]
  inline size_t index_of_mem_op() const noexcept { return index_of_op_type(OperandType::kMem); }

  //! A shortcut that calls `index_of_op_type(OperandType::kImm)`.
  [[nodiscard]]
  inline size_t index_of_imm_op() const noexcept { return index_of_op_type(OperandType::kImm); }

  //! A shortcut that calls `index_of_op_type(OperandType::kLabel)`.
  [[nodiscard]]
  inline size_t index_of_label_op() const noexcept { return index_of_op_type(OperandType::kLabel); }

  //! \}

  //! \cond INTERNAL
  //! \name Rewriting
  //! \{

  //! Returns `uint32_t[]` view that represents BaseInst::RegOnly and instruction operands.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t* _get_rewrite_array() noexcept { return &_base_inst._extra_reg._id; }

  //! \overload
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const uint32_t* _get_rewrite_array() const noexcept { return &_base_inst._extra_reg._id; }

  //! Maximum value of rewrite id - 6 operands each having 4 slots is 24, one RegOnly having 2 slots => 26.
  static inline constexpr uint32_t kMaxRewriteId = 26 - 1;

  //! Returns a rewrite index of the given pointer to `id`.
  //!
  //! This function returns a value that can be then passed to \ref _rewrite_id_at_index() function. It can address
  //! any id from any operand that is used by the instruction in addition to \ref BaseInst::extra_reg field, which
  //! can also be used by the register allocator.
  [[nodiscard]]
  inline uint32_t _get_rewrite_index(const uint32_t* id) const noexcept {
    const uint32_t* array = _get_rewrite_array();
    ASMJIT_ASSERT(array <= id);

    size_t index = (size_t)(id - array);
    ASMJIT_ASSERT(index <= kMaxRewriteId);

    return uint32_t(index);
  }

  //! Rewrites the given `index` to the provided identifier `id`.
  //!
  //! \note This is an internal function that is used by a \ref BaseCompiler implementation to rewrite virtual
  //! registers to physical registers. The rewriter in this case sees all operands as array of uint32 values
  //! and the given `index` describes a position in this array. For example a single \ref Operand would be
  //! decomposed to 4 uint32_t values, where the first at index 0 would be operand signature, next would be
  //! base id, etc... This is a comfortable way of patching operands without having to check for their types.
  inline void _rewrite_id_at_index(uint32_t index, uint32_t id) noexcept {
    ASMJIT_ASSERT(index <= kMaxRewriteId);

    uint32_t* array = _get_rewrite_array();
    array[index] = id;
  }

  //! \}
  //! \endcond
};

//! Instruction node with embedded operands following \ref InstNode layout.
//!
//! \note This is used to make tools such as static analysis and compilers happy about the layout. There were two
//! instruction nodes in the past, having the second extend the operand array of the first, but that has caused
//! undefined behavior and made recent tools unhappy about that.
template<uint32_t kN>
class InstNodeWithOperands : public InstNode {
public:
  Operand_ _operands[kN];

  //! Creates a new `InstNodeWithOperands` instance.
  ASMJIT_INLINE_NODEBUG InstNodeWithOperands(InstId inst_id, InstOptions options, uint32_t op_count) noexcept
    : InstNode(inst_id, options, op_count, kN) {}
};

//! Section node.
class SectionNode : public BaseNode {
public:
  ASMJIT_NONCOPYABLE(SectionNode)

  //! \name Members
  //! \{

  //! Section id.
  uint32_t _section_id;

  //! Next section node that follows this section.
  //!
  //! This link is only valid when the section is active (is part of the code) and when `Builder::has_dirty_section_links()`
  //! returns `false`. If you intend to use this field you should always call `Builder::update_section_links()` before you
  //! do so.
  SectionNode* _next_section;

  //! \}

  //! \name Construction & Destruction
  //! \{

  //! Creates a new `SectionNode` instance.
  ASMJIT_INLINE_NODEBUG explicit SectionNode(uint32_t section_id = 0) noexcept
    : BaseNode(NodeType::kSection, NodeFlags::kHasNoEffect),
      _section_id(section_id),
      _next_section(nullptr) {}

  //! \}

  //! \name Accessors
  //! \{

  //! Returns the section id.
  ASMJIT_INLINE_NODEBUG uint32_t section_id() const noexcept { return _section_id; }

  //! \}
};

//! Label node.
class LabelNode : public BaseNode {
public:
  ASMJIT_NONCOPYABLE(LabelNode)

  //! \name Members
  //! \{

  //! Label identifier.
  uint32_t _label_id;

  //! \}

  //! \name Construction & Destruction
  //! \{

  //! Creates a new `LabelNode` instance.
  ASMJIT_INLINE_NODEBUG explicit LabelNode(uint32_t label_id = Globals::kInvalidId) noexcept
    : BaseNode(NodeType::kLabel, NodeFlags::kHasNoEffect | NodeFlags::kActsAsLabel),
      _label_id(label_id) {}

  //! \}

  //! \name Accessors
  //! \{

  //! Returns \ref Label representation of the \ref LabelNode.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Label label() const noexcept { return Label(_label_id); }

  //! Returns the id of the label.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t label_id() const noexcept { return _label_id; }

  //! \}
};

//! Align directive (BaseBuilder).
//!
//! Wraps `.align` directive.
class AlignNode : public BaseNode {
public:
  ASMJIT_NONCOPYABLE(AlignNode)

  //! \name Members
  //! \{

  //! Alignment (in bytes).
  uint32_t _alignment;

  //! \}

  //! \name Construction & Destruction
  //! \{

  //! Creates a new `AlignNode` instance.
  ASMJIT_INLINE_NODEBUG AlignNode(AlignMode align_mode, uint32_t alignment) noexcept
    : BaseNode(NodeType::kAlign, NodeFlags::kIsCode | NodeFlags::kHasNoEffect) {

    _align_data._align_mode = align_mode;
    _alignment = alignment;
  }

  //! \}

  //! \name Accessors
  //! \{

  //! Returns align mode.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG AlignMode align_mode() const noexcept { return _align_data._align_mode; }

  //! Sets align mode to `align_mode`.
  ASMJIT_INLINE_NODEBUG void set_align_mode(AlignMode align_mode) noexcept { _align_data._align_mode = align_mode; }

  //! Returns align offset in bytes.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t alignment() const noexcept { return _alignment; }

  //! Sets align offset in bytes to `offset`.
  ASMJIT_INLINE_NODEBUG void set_alignment(uint32_t alignment) noexcept { _alignment = alignment; }

  //! \}
};

//! Embed data node.
//!
//! Wraps `.data` directive. The node contains data that will be placed at the node's position in the assembler
//! stream. The data is considered to be RAW; no analysis nor byte-order conversion is performed on RAW data.
class EmbedDataNode : public BaseNode {
public:
  ASMJIT_NONCOPYABLE(EmbedDataNode)

  //! \name Members
  //! \{

  size_t _item_count;
  size_t _repeat_count;

  //! \}

  //! \name Construction & Destruction
  //! \{

  //! Creates a new `EmbedDataNode` instance.
  ASMJIT_INLINE_NODEBUG EmbedDataNode(TypeId type_id, uint8_t type_size, size_t item_count, size_t repeat_count) noexcept
    : BaseNode(NodeType::kEmbedData, NodeFlags::kIsData),
      _item_count(item_count),
      _repeat_count(repeat_count) {
    _embed._type_id = type_id;
    _embed._type_size = type_size;
  }

  //! \}

  //! \name Accessors
  //! \{

  //! Returns data type as \ref TypeId.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG TypeId type_id() const noexcept { return _embed._type_id; }

  //! Returns the size of a single data element.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t type_size() const noexcept { return _embed._type_size; }

  //! Returns a pointer to the data casted to `T*` - `uint8_t*` by default.
  template<typename T = uint8_t>
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint8_t* data() noexcept { return Support::offset_ptr<T>(this, sizeof(EmbedDataNode)); }

  //! Returns a pointer to the data casted to `T*` - `const uint8_t*` by default (const).
  template<typename T = uint8_t>
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const uint8_t* data() const noexcept { return Support::offset_ptr<T>(this, sizeof(EmbedDataNode)); }

  //! Returns the number of (typed) items in the array.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t item_count() const noexcept { return _item_count; }

  //! Returns how many times the data is repeated (default 1).
  //!
  //! Repeated data is useful when defining constants for SIMD, for example.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t repeat_count() const noexcept { return _repeat_count; }

  //! Returns the size of the data, not considering the number of times it repeats.
  //!
  //! \note The returned value is the same as `type_size() * item_count()`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t data_size() const noexcept { return size_t(type_size()) * _item_count; }

  //! \}
};

//! Label data node.
class EmbedLabelNode : public BaseNode {
public:
  ASMJIT_NONCOPYABLE(EmbedLabelNode)

  //! \name Members
  //! \{

  uint32_t _label_id;
  uint32_t _data_size;

  //! \}

  //! \name Construction & Destruction
  //! \{

  //! Creates a new `EmbedLabelNode` instance.
  ASMJIT_INLINE_NODEBUG EmbedLabelNode(uint32_t label_id = 0, uint32_t data_size = 0) noexcept
    : BaseNode(NodeType::kEmbedLabel, NodeFlags::kIsData),
      _label_id(label_id),
      _data_size(data_size) {}

  //! \}

  //! \name Accessors
  //! \{

  //! Returns the label to embed as \ref Label operand.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Label label() const noexcept { return Label(_label_id); }

  //! Sets the label id from `label` operand.
  ASMJIT_INLINE_NODEBUG void set_label(const Label& label) noexcept { set_label_id(label.id()); }

  //! Returns the id of the label.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t label_id() const noexcept { return _label_id; }

  //! Sets the label id (use with caution, improper use can break a lot of things).
  ASMJIT_INLINE_NODEBUG void set_label_id(uint32_t label_id) noexcept { _label_id = label_id; }

  //! Returns the data size.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t data_size() const noexcept { return _data_size; }

  //! Sets the data size.
  ASMJIT_INLINE_NODEBUG void set_data_size(uint32_t data_size) noexcept { _data_size = data_size; }

  //! \}
};

//! Label data node.
class EmbedLabelDeltaNode : public BaseNode {
public:
  ASMJIT_NONCOPYABLE(EmbedLabelDeltaNode)

  //! \name Members
  //! \{

  uint32_t _label_id;
  uint32_t _base_label_id;
  uint32_t _data_size;

  //! \}

  //! \name Construction & Destruction
  //! \{

  //! Creates a new `EmbedLabelDeltaNode` instance.
  ASMJIT_INLINE_NODEBUG EmbedLabelDeltaNode(uint32_t label_id = 0, uint32_t base_label_id = 0, uint32_t data_size = 0) noexcept
    : BaseNode(NodeType::kEmbedLabelDelta, NodeFlags::kIsData),
      _label_id(label_id),
      _base_label_id(base_label_id),
      _data_size(data_size) {}

  //! \}

  //! \name Accessors
  //! \{

  //! Returns the label as `Label` operand.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Label label() const noexcept { return Label(_label_id); }

  //! Sets the label id from `label` operand.
  ASMJIT_INLINE_NODEBUG void set_label(const Label& label) noexcept { set_label_id(label.id()); }

  //! Returns the id of the label.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t label_id() const noexcept { return _label_id; }

  //! Sets the label id.
  ASMJIT_INLINE_NODEBUG void set_label_id(uint32_t label_id) noexcept { _label_id = label_id; }

  //! Returns the base label as `Label` operand.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Label base_babel() const noexcept { return Label(_base_label_id); }

  //! Returns the id of the base label.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t base_label_id() const noexcept { return _base_label_id; }

  //! Sets the base label id from `label` operand.
  ASMJIT_INLINE_NODEBUG void set_base_label(const Label& base_babel) noexcept { set_base_label_id(base_babel.id()); }
  //! Sets the base label id.
  ASMJIT_INLINE_NODEBUG void set_base_label_id(uint32_t base_label_id) noexcept { _base_label_id = base_label_id; }

  //! Returns the size of the embedded label address.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t data_size() const noexcept { return _data_size; }

  //! Sets the size of the embedded label address.
  ASMJIT_INLINE_NODEBUG void set_data_size(uint32_t data_size) noexcept { _data_size = data_size; }

  //! \}
};

//! A node that wraps `ConstPool`.
class ConstPoolNode : public LabelNode {
public:
  ASMJIT_NONCOPYABLE(ConstPoolNode)

  //! \name Members
  //! \{

  ConstPool _const_pool;

  //! \}

  //! \name Construction & Destruction
  //! \{

  //! Creates a new `ConstPoolNode` instance.
  ASMJIT_INLINE_NODEBUG ConstPoolNode(Arena& arena, uint32_t id = 0) noexcept
    : LabelNode(id),
      _const_pool(arena) {

    _set_type(NodeType::kConstPool);
    _add_flags(NodeFlags::kIsData);
    _clear_flags(NodeFlags::kIsCode | NodeFlags::kHasNoEffect);
  }

  //! \}

  //! \name Accessors
  //! \{

  //! Tests whether the constant-pool is empty.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_empty() const noexcept { return _const_pool.is_empty(); }

  //! Returns the size of the constant-pool in bytes.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t size() const noexcept { return _const_pool.size(); }

  //! Returns minimum alignment.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t alignment() const noexcept { return _const_pool.alignment(); }

  //! Returns the wrapped `ConstPool` instance.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG ConstPool& const_pool() noexcept { return _const_pool; }

  //! Returns the wrapped `ConstPool` instance (const).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const ConstPool& const_pool() const noexcept { return _const_pool; }

  //! \}

  //! \name Utilities
  //! \{

  //! See `ConstPool::add()`.
  ASMJIT_INLINE_NODEBUG Error add(const void* data, size_t size, Out<size_t> offset_out) noexcept {
    return _const_pool.add(data, size, offset_out);
  }

  //! \}
};

//! Comment node.
class CommentNode : public BaseNode {
public:
  ASMJIT_NONCOPYABLE(CommentNode)

  //! \name Construction & Destruction
  //! \{

  //! Creates a new `CommentNode` instance.
  ASMJIT_INLINE_NODEBUG CommentNode(const char* comment) noexcept
    : BaseNode(NodeType::kComment, NodeFlags::kIsInformative | NodeFlags::kHasNoEffect | NodeFlags::kIsRemovable) {
    _inline_comment = comment;
  }

  //! \}
};

//! Sentinel node.
//!
//! Sentinel is a marker that is completely ignored by the code builder. It's used to remember a position in a code
//! as it never gets removed by any pass.
class SentinelNode : public BaseNode {
public:
  ASMJIT_NONCOPYABLE(SentinelNode)

  //! \name Construction & Destruction
  //! \{

  //! Creates a new `SentinelNode` instance.
  ASMJIT_INLINE_NODEBUG SentinelNode(SentinelType sentinel_type = SentinelType::kUnknown) noexcept
    : BaseNode(NodeType::kSentinel, NodeFlags::kIsInformative | NodeFlags::kHasNoEffect) {

    _sentinel._sentinel_type = sentinel_type;
  }

  //! \}

  //! \name Accessors
  //! \{

  //! Returns the type of the sentinel.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG SentinelType sentinel_type() const noexcept {
    return _sentinel._sentinel_type;
  }

  //! Sets the type of the sentinel.
  ASMJIT_INLINE_NODEBUG void set_sentinel_type(SentinelType type) noexcept {
    _sentinel._sentinel_type = type;
  }

  //! \}
};

//! Pass can be used to implement code transformations, analysis, and lowering.
class ASMJIT_VIRTAPI Pass {
public:
  ASMJIT_BASE_CLASS(Pass)
  ASMJIT_NONCOPYABLE(Pass)

  //! \name Members
  //! \{

  //! BaseBuilder this pass is assigned to.
  BaseBuilder& _cb;
  //! Name of the pass.
  const char* _name {};

  //! \}

  //! \name Construction & Destruction
  //! \{

  ASMJIT_API Pass(BaseBuilder& cb, const char* name) noexcept;
  ASMJIT_API virtual ~Pass() noexcept;

  //! \}

  //! \name Accessors
  //! \{

  //! Returns \ref BaseBuilder associated with the pass.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG BaseBuilder& cb() const noexcept { return _cb; }

  //! Returns the name of the pass.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const char* name() const noexcept { return _name; }

  //! \}

  //! \name Pass Interface
  //! \{

  //! Processes the code stored in Builder or Compiler.
  //!
  //! This is the only function that is called by the `BaseBuilder` to process the code. It passes `arena`,
  //! which will be reset after the `run()` finishes.
  ASMJIT_API virtual Error run(Arena& arena, Logger* logger);

  //! \}
};

//! \}

ASMJIT_END_NAMESPACE

#endif // !ASMJIT_NO_BUILDER
#endif // ASMJIT_CORE_BUILDER_H_INCLUDED
