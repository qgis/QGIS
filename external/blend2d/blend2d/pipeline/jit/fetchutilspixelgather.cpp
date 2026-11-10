// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#if !defined(BL_BUILD_NO_JIT)

#include <blend2d/pipeline/jit/fetchutilspixelgather_p.h>

namespace bl::Pipeline::JIT::FetchUtils {

// bl::Pipeline::JIT::FetchUtils - IndexExtractor
// ==============================================

IndexExtractor::IndexExtractor(PipeCompiler* pc) noexcept
  : _pc(pc),
    _vec(),
    _mem(),
    _type(kTypeNone),
    _index_size(0),
    _mem_size(0) {}

void IndexExtractor::begin(uint32_t type, const Vec& vec) noexcept {
  BL_ASSERT(type != kTypeNone);
  BL_ASSERT(type < kTypeCount);

#if defined(BL_JIT_ARCH_X86)
  Mem mem = _pc->tmp_stack(PipeCompiler::StackId::kIndex, vec.size());

  _pc->v_storeavec(mem, vec, Alignment(16));
  begin(type, mem, vec.size());
#else
  _type = type;
  _vec = vec;

  switch (_type) {
    case kTypeInt16:
    case kTypeUInt16:
      _index_size = 2;
      break;

    case kTypeInt32:
    case kTypeUInt32:
      _index_size = 4;
      break;

    default:
      BL_NOT_REACHED();
  }
#endif
}

void IndexExtractor::begin(uint32_t type, const Mem& mem, uint32_t mem_size) noexcept {
  BL_ASSERT(type != kTypeNone);
  BL_ASSERT(type < kTypeCount);

  _type = type;
  _vec.reset();
  _mem = mem;
  _mem_size = uint16_t(mem_size);

  switch (_type) {
    case kTypeInt16:
    case kTypeUInt16:
      _index_size = 2;
      break;

    case kTypeInt32:
    case kTypeUInt32:
      _index_size = 4;
      break;

    default:
      BL_NOT_REACHED();
  }
}

void IndexExtractor::extract(const Gp& dst, uint32_t index) noexcept {
  BL_ASSERT(dst.size() >= 4);
  BL_ASSERT(_type != kTypeNone);

  if (!_vec.is_valid()) {
    BL_ASSERT((index + 1u) * _index_size <= _mem_size);
    Mem m = _mem;
    m.add_offset(int(index * _index_size));

    switch (_type) {
      case kTypeInt16 : _pc->load_i16(dst, m); return;
      case kTypeUInt16: _pc->load_u16(dst, m); return;
      case kTypeInt32 : _pc->load_i32(dst, m); return;
      case kTypeUInt32: _pc->load_u32(dst, m); return;

      default:
        BL_NOT_REACHED();
    }
  }
  else {
#if defined(BL_JIT_ARCH_X86)
    BL_NOT_REACHED();
#elif defined(BL_JIT_ARCH_A64)
    BackendCompiler* cc = _pc->cc;
    switch (_type) {
      case kTypeInt16 : cc->smov(dst      , _vec.h(index)); return;
      case kTypeUInt16: cc->umov(dst.r32(), _vec.h(index)); return;
      case kTypeInt32 : cc->smov(dst      , _vec.s(index)); return;
      case kTypeUInt32: cc->umov(dst.r32(), _vec.s(index)); return;

      default:
        BL_NOT_REACHED();
    }
#endif
  }
}

// bl::Pipeline::JIT::FetchUtils - FetchContext
// ============================================

void FetchContext::_init(PixelCount n) noexcept {
  BL_ASSERT(n >= PixelCount(4));

  _pixel->set_count(n);
  _fetch_mode = FetchMode::kNone;
  _fetch_done = false;

  _init_fetch_mode();
  _init_fetch_regs();
  _init_target_pixel();
}

void FetchContext::_init_fetch_mode() noexcept {
  switch (_pixel->type()) {
    case PixelType::kA8: {
      if (!bl_test_flag(_fetch_flags, PixelFlags::kPA_PI_UA_UI))
        _fetch_flags |= PixelFlags::kPA;

      switch (fetch_format()) {
        case FormatExt::kA8: {
          if (bl_test_flag(_fetch_flags, PixelFlags::kPA))
            _fetch_mode = FetchMode::kA8FromA8_PA;
          else
            _fetch_mode = FetchMode::kA8FromA8_UA;
          break;
        }

        case FormatExt::kPRGB32:
        case FormatExt::kFRGB32:
        case FormatExt::kZERO32: {
          if (bl_test_flag(_fetch_flags, PixelFlags::kPA))
            _fetch_mode = FetchMode::kA8FromRGBA32_PA;
          else
            _fetch_mode = FetchMode::kA8FromRGBA32_UA;
          break;
        }

        default:
          BL_NOT_REACHED();
      }
      break;
    }

    case PixelType::kRGBA32: {
      if (!bl_test_flag(_fetch_flags, PixelFlags::kPA_PI_UA_UI | PixelFlags::kPC_UC))
        _fetch_flags |= PixelFlags::kPC;

      switch (fetch_format()) {
        case FormatExt::kA8: {
          if (bl_test_flag(_fetch_flags, PixelFlags::kPC))
            _fetch_mode = FetchMode::kRGBA32FromA8_PC;
          else
            _fetch_mode = FetchMode::kRGBA32FromA8_UC;
          break;
        }

        case FormatExt::kPRGB32:
        case FormatExt::kFRGB32:
        case FormatExt::kXRGB32:
        case FormatExt::kZERO32: {
          if (bl_test_flag(_fetch_flags, PixelFlags::kPC))
            _fetch_mode = FetchMode::kRGBA32FromRGBA32_PC;
          else
            _fetch_mode = FetchMode::kRGBA32FromRGBA32_UC;
          break;
        }

        default:
          BL_NOT_REACHED();
      }

      break;
    }

    case PixelType::kRGBA64: {
      _fetch_mode = FetchMode::kRGBA64FromRGBA64_PC;
      break;
    }

    default:
      BL_NOT_REACHED();
  }
}

static BL_INLINE_NODEBUG uint32_t vec128_reg_count_from_bytes(uint32_t n) noexcept {
  return (n + 15) / 16u;
}

void FetchContext::_init_fetch_regs() noexcept {
  uint32_t pixel_count = uint32_t(_pixel->count());
  BL_ASSERT(pixel_count >= 2u);

#if defined(BL_JIT_ARCH_X86)
  uint32_t alpha_acc_size = 0;
  WideningOp default_widening_op = WideningOp::kInterleave;
#endif // BL_JIT_ARCH_X86

  _pixel_index = 0u;
  _vec_index = 0u;
  _vec_step = 1u;
  _lane_index = 0u;

  uint32_t full_byte_count = 0;
  uint32_t p128_vec_count = 0;

  bool packed = (_fetch_mode == FetchMode::kA8FromA8_PA ||
                 _fetch_mode == FetchMode::kA8FromRGBA32_PA ||
                 _fetch_mode == FetchMode::kRGBA32FromA8_PC ||
                 _fetch_mode == FetchMode::kRGBA32FromA8_UC ||
                 _fetch_mode == FetchMode::kRGBA32FromRGBA32_PC ||
                 _fetch_mode == FetchMode::kRGBA64FromRGBA64_PC);

  switch (_fetch_mode) {
    case FetchMode::kA8FromA8_PA:
    case FetchMode::kA8FromA8_UA:
    case FetchMode::kA8FromRGBA32_PA:
    case FetchMode::kA8FromRGBA32_UA: {
#if defined(BL_JIT_ARCH_X86)
      alpha_acc_size = packed && pixel_count <= 4 ? uint32_t(4u) : uint32_t(_pc->register_size());
#endif // BL_JIT_ARCH_X86

      _lane_count = bl_min<uint32_t>(uint32_t(packed ? 16u : 8u), pixel_count);

      full_byte_count = packed ? pixel_count : pixel_count * 2u;
      p128_vec_count = vec128_reg_count_from_bytes(full_byte_count);
      break;
    }

    case FetchMode::kRGBA32FromA8_PC: {
#if defined(BL_JIT_ARCH_X86)
      alpha_acc_size = packed && pixel_count <= 4 ? uint32_t(4u) : uint32_t(_pc->register_size());
#endif // BL_JIT_ARCH_X86

      _lane_count = bl_min<uint32_t>(8u << uint32_t(_pc->use_512bit_simd()), pixel_count);

      full_byte_count = pixel_count * 4u;
      p128_vec_count = bl_max<uint32_t>(vec128_reg_count_from_bytes(full_byte_count) >> uint32_t(_pc->vec_width()), 1u);

#if defined(BL_JIT_ARCH_X86)
      default_widening_op = WideningOp::kRepeat;
#endif // BL_JIT_ARCH_X86
      break;
    }

    case FetchMode::kRGBA32FromA8_UC: {
#if defined(BL_JIT_ARCH_X86)
      alpha_acc_size = packed && pixel_count <= 4 ? uint32_t(4u) : uint32_t(_pc->register_size());
#endif // BL_JIT_ARCH_X86

      _lane_count = bl_min<uint32_t>(8u, pixel_count);

      full_byte_count = pixel_count * 8u;

#if defined(BL_JIT_ARCH_X86)
      if (_pc->use_512bit_simd() && pixel_count >= 8u) {
        default_widening_op = WideningOp::kRepeat8xA8ToRGBA32_UC_AVX512;
        p128_vec_count = (pixel_count + 7u) / 8u;
      }
      else if (_pc->use_256bit_simd() && pixel_count >= 4u) {
        default_widening_op = WideningOp::kUnpack2x;
        p128_vec_count = (pixel_count + 1u) / 2u;
        _vec_step = 2u;
      }
      else
#endif // BL_JIT_ARCH_X86
      {
        p128_vec_count = (pixel_count + 1u) / 2u;
        _vec_step = bl_min<uint32_t>(p128_vec_count, 4u);
      }
      break;
    }

    case FetchMode::kRGBA32FromRGBA32_PC: {
      _lane_count = bl_min<uint32_t>(4u, pixel_count);

      full_byte_count = pixel_count * 4u;
      p128_vec_count = vec128_reg_count_from_bytes(full_byte_count);
      break;
    }

    case FetchMode::kRGBA32FromRGBA32_UC: {
      _lane_count = bl_min<uint32_t>(2u << uint32_t(_pc->use_256bit_simd()), pixel_count);

      full_byte_count = pixel_count * 8u;
      p128_vec_count = vec128_reg_count_from_bytes(full_byte_count >> uint32_t(_pc->use_256bit_simd()));
      break;
    }

    case FetchMode::kRGBA64FromRGBA64_PC: {
      _lane_count = 2u;

      full_byte_count = pixel_count * 8u;
      p128_vec_count = vec128_reg_count_from_bytes(full_byte_count);
      break;
    }

    default:
      BL_NOT_REACHED();
  }

  BL_ASSERT(p128_vec_count != 0u);
  _p128_count = uint8_t(p128_vec_count);

  _p_tmp[0] = _pc->new_vec128("@pTmp[0]");
  _p_tmp[1] = _pc->new_vec128("@pTmp[1]");

  for (size_t i = 0; i < p128_vec_count; i++) {
    _p128[i] = _pc->new_vec128("@p128[%u]", i);
  }

#if defined(BL_JIT_ARCH_X86)
  // Let's only use GP accumulator on X86 platform as that's pretty easy to implement and
  // it's fast. Other platforms seem to be just okay with SIMD lane to lane insertion.
  if (alpha_acc_size > 4)
    _a_acc = _pc->new_gp64("@a_acc");
  else if (alpha_acc_size > 0)
    _a_acc = _pc->new_gp32("@a_acc");

  _widening256_op = WideningOp::kNone;
  _widening512_op = WideningOp::kNone;

  if (_pc->use_256bit_simd() && full_byte_count > 16u) {
    size_t p256_vec_count = (full_byte_count + 31u) / 32u;
    _pc->new_vec256_array(_p256, p256_vec_count, "@p256");

    if (_pc->use_512bit_simd() && full_byte_count > 32u) {
      size_t p512_vec_count = (full_byte_count + 63u) / 64u;
      _pc->new_vec512_array(_p512, p512_vec_count, "@p512");

      _widening256_op = default_widening_op;
      _widening512_op = packed ? default_widening_op : WideningOp::kUnpack;
    }
    else {
      _widening256_op = packed ? default_widening_op : WideningOp::kUnpack;
      _widening512_op = WideningOp::kNone;
    }
  }
#endif // BL_JIT_ARCH_X86
}

void FetchContext::_init_target_pixel() noexcept {
  const Vec* v_array = _p128;
  size_t v_count = _p128_count;

#if defined(BL_JIT_ARCH_X86)
  if (_p512.size()) {
    v_array = static_cast<const Vec*>(static_cast<const Operand_*>(_p512.v));
    v_count = _p512.size();
  }
  else if (_p256.size()) {
    v_array = static_cast<const Vec*>(static_cast<const Operand_*>(_p256.v));
    v_count = _p256.size();
  }
#endif // BL_JIT_ARCH_X86

  switch (_pixel->type()) {
    case PixelType::kA8: {
      if (bl_test_flag(_fetch_flags, PixelFlags::kPA)) {
        _pixel->pa.init(v_array, v_count);
        _pc->rename(_pixel->pa, _pixel->name(), "pa");
      }
      else {
        _pixel->ua.init(v_array, v_count);
        _pc->rename(_pixel->ua, _pixel->name(), "ua");
      }
      break;
    }

    case PixelType::kRGBA32: {
      if (bl_test_flag(_fetch_flags, PixelFlags::kPC)) {
        _pixel->pc.init(v_array, v_count);
        _pc->rename(_pixel->pc, _pixel->name(), "pc");
      }
      else {
        _pixel->uc.init(v_array, v_count);
        _pc->rename(_pixel->uc, _pixel->name(), "uc");
      }
      break;
    }

    case PixelType::kRGBA64: {
      _pixel->uc.init(v_array, v_count);
      _pc->rename(_pixel->uc, _pixel->name(), "uc");
      break;
    }

    default:
      BL_NOT_REACHED();
  }
}

void FetchContext::fetch_pixel(const Mem& src) noexcept {
  uint32_t pixel_count = uint32_t(_pixel->count());
  BL_ASSERT(_pixel_index < pixel_count);

  Vec v = _p128[_vec_index];
  BL_ASSERT(v.is_valid());

  Mem m(src);

  uint32_t quantity = (_pixel_index + 2u == pixel_count && _gather_mode == GatherMode::kNeverFull) ? 2u : 1u;

  switch (_fetch_mode) {
    case FetchMode::kA8FromA8_PA:
    case FetchMode::kA8FromA8_UA:
    case FetchMode::kA8FromRGBA32_PA:
    case FetchMode::kA8FromRGBA32_UA:
    case FetchMode::kRGBA32FromA8_PC:
    case FetchMode::kRGBA32FromA8_UC: {
      bool fetch_packed = (_fetch_mode == FetchMode::kA8FromA8_PA ||
                          _fetch_mode == FetchMode::kA8FromRGBA32_PA ||
                          _fetch_mode == FetchMode::kRGBA32FromA8_PC ||
                          _fetch_mode == FetchMode::kRGBA32FromA8_UC);

      bool a8FromRgba32 = (_fetch_mode == FetchMode::kA8FromRGBA32_PA ||
                           _fetch_mode == FetchMode::kA8FromRGBA32_UA);

#if defined(BL_JIT_ARCH_A64)
      if (_lane_index == 0) {
        if (a8FromRgba32 && _fetch_info.fetch_alpha_offset()) {
          _pc->v_loadu32(v, m);
          _pc->v_srli_u32(v, v, 24);
        }
        else {
          _pc->v_load8(v, m);
        }
      }
      else {
        uint32_t src_lane = 0;
        if (a8FromRgba32 && _fetch_info.fetch_alpha_offset()) {
          _pc->v_loadu32(_p_tmp[0], m);
          src_lane = 3;
        }
        else {
          _pc->v_load8(_p_tmp[0], m);
        }

        _pc->cc->ins(v.b(fetch_packed ? _lane_index : _lane_index * 2u), _p_tmp[0].b(src_lane));
      }
#else
      uint32_t acc_byte_size = _a_acc.size();

      if (a8FromRgba32) {
        m.add_offset(_fetch_info.fetch_alpha_offset());
      }

      if (_a_acc_index == 0)
        _pc->load_u8(_a_acc, m);
      else
        _pc->load_merge_u8(_a_acc, m);

      _pc->ror(_a_acc, _a_acc, (fetch_packed ? 8u : 16u) * quantity);

      uint32_t acc_bytes_scale = fetch_packed ? 1 : 2;
      uint32_t acc_bytes = (_a_acc_index + quantity) * acc_bytes_scale;

      _a_acc_index++;

      if (acc_bytes >= acc_byte_size || _pixel_index + quantity >= pixel_count) {
        if (acc_byte_size == 4) {
          uint32_t dst_lane_index = (fetch_packed ? _lane_index : _lane_index * 2u) / 4u;

          if (dst_lane_index == 0) {
            _pc->s_mov(v, _a_acc);
          }
          else if (!_pc->has_sse4_1()) {
            if (dst_lane_index == 1) {
              _pc->s_mov(_p_tmp[0], _a_acc);
              _pc->v_interleave_lo_u32(v, v, _p_tmp[0]);
            }
            else if (dst_lane_index == 2) {
              _pc->s_mov(_p_tmp[0], _a_acc);
            }
            else if (dst_lane_index == 3) {
              _pc->s_mov(_p_tmp[1], _a_acc);
              _pc->v_interleave_lo_u32(_p_tmp[0], _p_tmp[0], _p_tmp[1]);
              _pc->v_interleave_lo_u64(v, v, _p_tmp[0]);
            }
          }
          else {
            _pc->s_insert_u32(v, _a_acc, dst_lane_index);
          }
        }
        else {
          uint32_t dst_lane_index = (fetch_packed ? _lane_index : _lane_index * 2u) / 8u;
          if (dst_lane_index == 0) {
            _pc->s_mov(v, _a_acc);
          }
          else if (!_pc->has_sse4_1()) {
            _pc->s_mov(_p_tmp[0], _a_acc);
            _pc->v_interleave_lo_u64(v, v, _p_tmp[0]);
          }
          else {
            _pc->s_insert_u64(v, _a_acc, dst_lane_index);
          }
        }

        _a_acc_index = 0;
      }
#endif
      break;
    }

    case FetchMode::kRGBA32FromRGBA32_PC:
    case FetchMode::kRGBA32FromRGBA32_UC: {
      if (_lane_index == 0) {
        _pc->v_loadu32(v, m);
      }
      else {
#if defined(BL_JIT_ARCH_X86)
        if (!_pc->has_sse4_1()) {
          if (_lane_index == 1) {
            _pc->v_loadu32(_p_tmp[0], m);
            _pc->v_interleave_lo_u32(v, v, _p_tmp[0]);
          }
          else if (_lane_index == 2) {
            _pc->v_loadu32(_p_tmp[0], m);

            // If quantity == 2 it means we are avoiding the last pixel and the following branch would never get called.
            if (quantity == 2u)
              _pc->v_interleave_lo_u64(v, v, _p_tmp[0]);
          }
          else {
            _pc->v_loadu32(_p_tmp[1], m);
            _pc->v_interleave_lo_u32(_p_tmp[0], _p_tmp[0], _p_tmp[1]);
            _pc->v_interleave_lo_u64(v, v, _p_tmp[0]);
          }
        }
        else
#endif
        {
          _pc->v_insert_u32(v, m, _lane_index);
        }
      }

      break;
    }

    case FetchMode::kRGBA64FromRGBA64_PC: {
      if (_lane_index == 0)
        _pc->v_loadu64(v, m);
      else
        _pc->v_insert_u64(v, m, _lane_index);
      break;
    }

    default:
      BL_NOT_REACHED();
  }

  // NOTE: This is better to be done with a loop as it perfectly emulates the "fetch" of a possible last pixel,
  // which was avoided. Theoretically we could just do `_lane_index += quantity` here, but it could be source of
  // future bugs if more features are added.
  do {
    if (++_lane_index >= _lane_count) {
      _lane_index = 0u;
      _done_vec(_vec_index);
      _vec_index += _vec_step;
    }

    _pixel_index++;
  } while (--quantity != 0);
}

void FetchContext::_fetch_all(const Mem& src, uint32_t src_shift, IndexExtractor& extractor, const uint8_t* indexes, InterleaveCallback cb, void* cb_data) noexcept {
  // Fetching all pixels assumes no pixels were fetched previously.
  BL_ASSERT(_pixel_index == 0);

  uint32_t pixel_count = uint32_t(_pixel->count());

  Gp idx0 = _pc->new_gpz("@idx0");
  Gp idx1 = _pc->new_gpz("@idx1");

  Mem src0 = src;
  Mem src1 = src;

  src0.set_index(idx0, src_shift);
  src1.set_index(idx1, src_shift);

  switch (pixel_count) {
    case 2: {
      extractor.extract(idx0, indexes[0]);
      extractor.extract(idx1, indexes[1]);

      cb(0, cb_data);
      fetch_pixel(src0);

      cb(1, cb_data);
      fetch_pixel(src1);

      cb(0xFF, cb_data);
      break;
    }

    case 4: {
      extractor.extract(idx0, indexes[0]);
      extractor.extract(idx1, indexes[1]);

      cb(0, cb_data);
      fetch_pixel(src0);
      extractor.extract(idx0, indexes[2]);

      cb(1, cb_data);
      fetch_pixel(src1);

      if (_gather_mode == GatherMode::kFetchAll)
        extractor.extract(idx1, indexes[3]);

      cb(2, cb_data);
      fetch_pixel(src0);

      cb(3, cb_data);
      if (_gather_mode == GatherMode::kFetchAll)
        fetch_pixel(src1);

      cb(0xFF, cb_data);
      break;
    }

    case 8:
    case 16: {
#if defined(BL_JIT_ARCH_X86)
      bool hasFastInsert32 = _pc->has_sse4_1();
#else
      constexpr bool hasFastInsert32 = true;
#endif

      if (_fetch_mode == FetchMode::kRGBA32FromRGBA32_PC && hasFastInsert32) {
        for (uint32_t i = 0; i < pixel_count; i += 8) {
          Vec& v0 = _p128[_vec_index];
          Vec& v1 = _p128[_vec_index + _vec_step];

          extractor.extract(idx0, indexes[i + 0u]);
          extractor.extract(idx1, indexes[i + 4u]);

          cb(i + 0u, cb_data);
          _pc->v_loada32(v0, src0);
          extractor.extract(idx0, indexes[i + 1u]);

          cb(i + 1u, cb_data);
          _pc->v_loada32(v1, src1);
          extractor.extract(idx1, indexes[i + 5u]);

          cb(i + 2u, cb_data);
          _pc->v_insert_u32(v0, src0, 1);
          extractor.extract(idx0, indexes[i + 2u]);

          cb(i + 3u, cb_data);
          _pc->v_insert_u32(v1, src1, 1);
          extractor.extract(idx1, indexes[i + 6u]);

          cb(i + 4u, cb_data);
          _pc->v_insert_u32(v0, src0, 2);
          extractor.extract(idx0, indexes[i + 3u]);

          cb(i + 5u, cb_data);
          _pc->v_insert_u32(v1, src1, 2);

          if (_gather_mode == GatherMode::kFetchAll)
            extractor.extract(idx1, indexes[i + 7u]);

          cb(i + 6u, cb_data);
          _pc->v_insert_u32(v0, src0, 3);

          _pixel_index += 4;
          _done_vec(_vec_index);
          _vec_index += _vec_step;

          cb(i + 7u, cb_data);
          if (_gather_mode == GatherMode::kFetchAll)
            _pc->v_insert_u32(v1, src1, 3);

          _pixel_index += 4;
          _done_vec(_vec_index);
          _vec_index += _vec_step;
        }
      }
      else {
        for (uint32_t i = 0; i < pixel_count; i += 8) {
          extractor.extract(idx0, indexes[i + 0u]);
          extractor.extract(idx1, indexes[i + 1u]);

          cb(i + 0u, cb_data);
          fetch_pixel(src0);
          extractor.extract(idx0, indexes[i + 2u]);

          cb(i + 1u, cb_data);
          fetch_pixel(src1);
          extractor.extract(idx1, indexes[i + 3u]);

          cb(i + 2u, cb_data);
          fetch_pixel(src0);
          extractor.extract(idx0, indexes[i + 4u]);

          cb(i + 3u, cb_data);
          fetch_pixel(src1);
          extractor.extract(idx1, indexes[i + 5u]);

          cb(i + 4u, cb_data);
          fetch_pixel(src0);
          extractor.extract(idx0, indexes[i + 6u]);

          cb(i + 5u, cb_data);
          fetch_pixel(src1);
          if (_gather_mode == GatherMode::kFetchAll)
            extractor.extract(idx1, indexes[i + 7u]);

          cb(i + 6u, cb_data);
          fetch_pixel(src0);

          cb(i + 7u, cb_data);
          if (_gather_mode == GatherMode::kFetchAll)
            fetch_pixel(src1);
        }
      }

      cb(0xFF, cb_data);
      break;
    }

    default:
      BL_NOT_REACHED();
  }
}

void FetchContext::_done_vec(uint32_t index) noexcept {
  if (_fetch_mode == FetchMode::kRGBA32FromA8_PC) {
    if (_lane_count <= 4u) {
      _pc->v_interleave_lo_u8(_p128[index + 0], _p128[index + 0], _p128[index + 0]);
    }

    if (_lane_count <= 8u) {
      _pc->v_interleave_lo_u8(_p128[index + 0], _p128[index + 0], _p128[index + 0]);
    }

    if (_vec_step == 2u) {
      _pc->v_interleave_hi_u8(_p128[index + 1], _p128[index + 0], _p128[index + 0]);
      _pc->v_interleave_lo_u8(_p128[index + 0], _p128[index + 0], _p128[index + 0]);
    }
  }

  if (_fetch_mode == FetchMode::kRGBA32FromA8_UC) {
#if defined(BL_JIT_ARCH_X86)
    if (_widening512_op != WideningOp::kNone) {
      // Keep it AS IS as we are widening 8 packed bytes to 64 unpacked bytes - a single byte to [0A 0A 0A 0A].
      BL_ASSERT(_pixel->count() >= PixelCount(8));
    }
    else if (_widening256_op != WideningOp::kNone) {
      BL_ASSERT(_pixel->count() >= PixelCount(4));
      BL_ASSERT(_widening256_op == WideningOp::kUnpack2x);

      _pc->v_interleave_lo_u8(_p128[index + 0], _p128[index + 0], _p128[index + 0]);     // [a7 a7 a6 a6 a5 a5 a4 a4 a3 a3 a2 a2 a1 a1 a0 a0]
      _pc->v_interleave_hi_u8(_p128[index + 1], _p128[index + 0], _p128[index + 0]);     // [a7 a7 a7 a7 a6 a6 a6 a6 a5 a5 a5 a5 a4 a4 a4 a4]
      _pc->v_interleave_lo_u8(_p128[index + 0], _p128[index + 0], _p128[index + 0]);     // [a3 a3 a3 a3 a2 a2 a2 a2 a1 a1 a1 a1 a0 a0 a0 a0]
    }
    else
#endif
    {
      switch (_vec_step) {
        case 1: {
          _pc->v_interleave_lo_u8(_p128[index + 0], _p128[index + 0], _p128[index + 0]); // [?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? a1 a1 a0 a0]
          _pc->v_interleave_lo_u8(_p128[index + 0], _p128[index + 0], _p128[index + 0]); // [?? ?? ?? ?? ?? ?? ?? ?? a1 a1 a1 a1 a0 a0 a0 a0]
          _pc->v_cvt_u8_lo_to_u16(_p128[index + 0], _p128[index + 0]);                   // [00 a1 00 a1 00 a1 00 a1 00 a0 00 a0 00 a0 00 a0]
          break;
        }

        case 2: {
          _pc->v_interleave_lo_u8(_p128[index + 0], _p128[index + 0], _p128[index + 0]); // [?? ?? ?? ?? ?? ?? ?? ?? a3 a3 a2 a2 a1 a1 a0 a0]
          _pc->v_interleave_lo_u8(_p128[index + 0], _p128[index + 0], _p128[index + 0]); // [a3 a3 a3 a3 a2 a2 a2 a2 a1 a1 a1 a1 a0 a0 a0 a0]
          _pc->v_cvt_u8_hi_to_u16(_p128[index + 1], _p128[index + 0]);                   // [00 a3 00 a3 00 a3 00 a3 00 a2 00 a2 00 a2 00 a2]
          _pc->v_cvt_u8_lo_to_u16(_p128[index + 0], _p128[index + 0]);                   // [00 a1 00 a1 00 a1 00 a1 00 a0 00 a0 00 a0 00 a0]
          break;
        }

        case 4: {
          _pc->v_interleave_lo_u8(_p128[index + 0], _p128[index + 0], _p128[index + 0]); // [a7 a7 a6 a6 a5 a5 a4 a4 a3 a3 a2 a2 a1 a1 a0 a0]
          _pc->v_interleave_hi_u8(_p128[index + 2], _p128[index + 0], _p128[index + 0]); // [a7 a7 a7 a7 a6 a6 a6 a6 a5 a5 a5 a5 a4 a4 a4 a4]
          _pc->v_interleave_lo_u8(_p128[index + 0], _p128[index + 0], _p128[index + 0]); // [a3 a3 a3 a3 a2 a2 a2 a2 a1 a1 a1 a1 a0 a0 a0 a0]
          _pc->v_cvt_u8_hi_to_u16(_p128[index + 3], _p128[index + 2]);                   // [00 a7 00 a7 00 a7 00 a7 00 a6 00 a6 00 a6 00 a6]
          _pc->v_cvt_u8_lo_to_u16(_p128[index + 2], _p128[index + 2]);                   // [00 a5 00 a5 00 a5 00 a5 00 a4 00 a4 00 a4 00 a4]
          _pc->v_cvt_u8_hi_to_u16(_p128[index + 1], _p128[index + 0]);                   // [00 a3 00 a3 00 a3 00 a3 00 a2 00 a2 00 a2 00 a2]
          _pc->v_cvt_u8_lo_to_u16(_p128[index + 0], _p128[index + 0]);                   // [00 a1 00 a1 00 a1 00 a1 00 a0 00 a0 00 a0 00 a0]
          break;
        }

        default:
          BL_NOT_REACHED();
      }
    }
  }

  if (_fetch_mode == FetchMode::kRGBA32FromRGBA32_UC) {
    if (_lane_count == 2u) {
      _pc->v_cvt_u8_lo_to_u16(_p128[index], _p128[index]);
    }
  }

#if defined(BL_JIT_ARCH_X86)
  // Firstly, widen to 256-bit wide registers and then decide whether to widen to 512-bit registers. In general both
  // can execute if we want to for example interleave and then unpack. However, if both widening operations are to
  // interleave, then they would not execute both here (as interleave to 512-bit requires 4 128-bit registers).
  const CommonTable& ct = _pc->ct<CommonTable>();
  bool widen512 = false;

  switch (_widening256_op) {
    case WideningOp::kNone: {
      break;
    }

    case WideningOp::kInterleave: {
      // We can interleave two vectors once we processed them, so check whether the currently processed vector was odd.
      if ((index & 0x1u) == 1u) {
        uint32_t index256 = index / 2u;
        uint32_t index128a = index - 1u;
        uint32_t index128b = index;

        _pc->v_insert_v128(_p256[index256], _p128[index128a].ymm(), _p128[index128b], 1u);
        widen512 = true;
      }
      break;
    }

    case WideningOp::kUnpack: {
      _pc->v_cvt_u8_lo_to_u16(_p256[index], _p128[index]);
      // Only used to unpack to 128-bit vector to 256-bit vector, so keep `widen512` false.
      break;
    }

    case WideningOp::kUnpack2x: {
      // Only used to unpack to 128-bit vectors to 256-bit vectors, so keep `widen512` false.
      _pc->v_cvt_u8_lo_to_u16(_p256[index + 0], _p128[index + 0]);
      _pc->v_cvt_u8_lo_to_u16(_p256[index + 1], _p128[index + 1]);
      break;
    }

    case WideningOp::kRepeat: {
      if (_widening512_op == WideningOp::kUnpack) {
        _pc->v_cvt_u8_to_u32(_p512[index], _p128[index]);
        _pc->v_swizzlev_u8(_p512[index], _p512[index], _pc->simd_const(&ct.swizu8_xxx3xxx2xxx1xxx0_to_z3z3z2z2z1z1z0z0, Bcst::kNA, _p512[index]));
      }
      else if (_widening512_op == WideningOp::kRepeat) {
        _pc->v_cvt_u8_to_u32(_p512[index], _p128[index]);
        _pc->v_swizzlev_u8(_p512[index], _p512[index], _pc->simd_const(&ct.swizu8_xxx3xxx2xxx1xxx0_to_3333222211110000, Bcst::kNA, _p512[index]));
      }
      else {
        _pc->v_cvt_u8_lo_to_u16(_p256[index], _p128[index]);
        _pc->v_mul_u16(_p256[index], _p256[index], _pc->simd_const(&ct.p_0101010101010101, Bcst::k32, _p256[index]));
        widen512 = true;
      }
      break;
    }

    case WideningOp::kRepeat8xA8ToRGBA32_UC_AVX512: {
      // This case widens 128-bit vector directly to a 512-bit vector, so keep `widen512` as false.
      if (_pc->has_avx512_vbmi()) {
        Vec pred = _pc->simd_vec_const(&ct.permu8_a8_to_rgba32_uc, Bcst::kNA_Unique, _p512[index]);
        _pc->v_permute_u8(_p512[index], pred, _p128[index].zmm());
      }
      else {
        _pc->cc->vpmovzxbq(_p512[index], _p128[index]);
        _pc->v_swizzle_lo_u16x4(_p512[index], _p512[index], swizzle(0, 0, 0, 0));
        _pc->v_swizzle_hi_u16x4(_p512[index], _p512[index], swizzle(0, 0, 0, 0));
      }
      break;
    }
  }

  // Secondly, widen to 512-bit wide registers.
  //
  // NOTE: Widening to 512-bit registers is there more for testing than practical use, because in general
  // we expect that gathers would be faster than scalar loads in AVX-512 mode. However, since DOWNFALL made
  // gathers pretty slow after a microcode update it's still possible we would hit this path in production.
  if (widen512) {
    switch (_widening512_op) {
      case WideningOp::kNone: {
        break;
      }

      case WideningOp::kInterleave: {
        if ((index & 0x3u) == 3u) {
          uint32_t index512 = index / 4u;
          uint32_t index256a = (index / 2u) - 1u;
          uint32_t index256b = (index / 2u);

          _pc->v_insert_v256(_p512[index512], _p256[index256a].zmm(), _p256[index256b], 1u);
        }
        break;
      }

      case WideningOp::kUnpack: {
        if ((index & 0x1u) == 1u) {
          uint32_t index512 = index / 2u;
          uint32_t index256 = index / 2u;
          _pc->v_cvt_u8_lo_to_u16(_p512[index512], _p256[index256]);
        }
        break;
      }

      case WideningOp::kRepeat: {
        if ((index & 0x1u) == 1u) {
          uint32_t index512 = index / 2u;
          uint32_t index256 = index;

          _pc->v_cvt_u8_lo_to_u16(_p512[index512], _p256[index256]);
          _pc->v_mul_u16(_p512[index512], _p512[index512], _pc->simd_const(&ct.p_0101010101010101, Bcst::k32, _p512[index]));
        }
        break;
      }

      default:
        BL_NOT_REACHED();
    }
  }
#endif // BL_JIT_ARCH_X86
}

void FetchContext::end() noexcept {}

// bl::Pipeline::JIT::FetchUtils - Convert Gathered Pixels
// =======================================================

static void convert_gathered_pixels(PipeCompiler* pc, Pixel& p, PixelCount n, PixelFlags flags, const VecArray& gPix) noexcept {
  if (p.isA8()) {
    pc->v_srli_u32(gPix, gPix, 24);

    if (bl_test_flag(flags, PixelFlags::kPA)) {
      VecWidth pa_vec_width = pc->vec_width_of(DataWidth::k8, n);
      size_t pa_reg_count = pc->vec_count_of(DataWidth::k8, n);

      pc->new_vec_array(p.pa, pa_reg_count, pa_vec_width, p.name(), "pa");
      BL_ASSERT(p.pa.size() == 1);

#if defined(BL_JIT_ARCH_X86)
      if (pc->has_avx512()) {
        pc->cc->vpmovdb(p.pa[0], gPix[0]);
      }
      else
#endif // BL_JIT_ARCH_X86
      {
        pc->x_packs_i16_u8(p.pa[0].clone_as(gPix[0]), gPix[0], gPix[0]);
        pc->x_packs_i16_u8(p.pa[0], p.pa[0], p.pa[0]);
      }
    }
    else {
      VecWidth ua_vec_width = pc->vec_width_of(DataWidth::k16, n);
      size_t ua_reg_count = pc->vec_count_of(DataWidth::k16, n);

      pc->new_vec_array(p.ua, ua_reg_count, ua_vec_width, p.name(), "ua");
      BL_ASSERT(p.ua.size() == 1);

#if defined(BL_JIT_ARCH_X86)
      if (pc->has_avx512()) {
        pc->cc->vpmovdw(p.ua[0], gPix[0]);
      }
      else
#endif // BL_JIT_ARCH_X86
      {
        pc->x_packs_i16_u8(p.ua[0].clone_as(gPix[0]), gPix[0], gPix[0]);
      }
    }
  }
  else if (p.isRGBA32()) {
    p.pc = gPix;
    pc->rename(p.pc, p.name(), "pc");
  }
  else {
#if defined(BL_JIT_ARCH_X86)
    if (!pc->use_256bit_simd() && gPix[0].is_vec256()) {
      Vec uc1 = pc->new_vec128(p.name(), "uc1");
      p.uc.init(gPix[0].xmm(), uc1);
      pc->cc->vextracti128(uc1, gPix[0], 1);
    }
    else
#endif // BL_JIT_ARCH_X86
    {
      p.uc = gPix;
      pc->rename(p.uc, p.name(), "uc");
    }
  }
}

// bl::Pipeline::JIT::FetchUtils - Gather Pixels
// =============================================

void gather_pixels(PipeCompiler* pc, Pixel& p, PixelCount n, PixelFlags flags, PixelFetchInfo f_info, const Mem& src, const Vec& idx, uint32_t shift, IndexLayout index_layout, GatherMode mode, InterleaveCallback cb, void* cb_data) noexcept {
  Mem mem(src);

#if defined(BL_JIT_ARCH_X86)
  uint32_t bpp = f_info.bpp();

  // Disabled gather means that we would gather to a wider register than enabled by the pipeline.
  bool disabled_gather = pc->vec_width() == VecWidth::k128 && uint32_t(n) * bpp > 16u;

  // Forced gather means that we have to use gather because of the width of gathered data.
  bool forced_gather = pc->has_avx512() && n > PixelCount(8);

  if (!disabled_gather && (pc->has_cpu_hint(CpuHints::kVecFastGather) || forced_gather)) {
    // NOTE: Gathers are provided by AVX2 and later, thus if we are here it means at least AVX2 is available.
    BL_ASSERT(pc->has_avx2());

    BackendCompiler* cc = pc->cc;
    uint32_t count = uint32_t(p.count());

    if (bpp == 4) {
      VecArray pixels;

      if (n <= PixelCount(4))
        pc->new_vec128_array(pixels, 1, p.name(), "pc");
      else if (n <= PixelCount(8))
        pc->new_vec256_array(pixels, 1, p.name(), "pc");
      else
        pc->new_vec512_array(pixels, 1, p.name(), "pc");

      Vec gather_index = idx.clone_as(pixels[0]);

      switch (index_layout) {
        case IndexLayout::kUInt16:
          gather_index = pc->new_similar_reg(pixels[0], "gather_index");
          cc->vpmovzxwd(gather_index, idx.xmm());
          break;

        case IndexLayout::kUInt32:
        case IndexLayout::kUInt32Lo16:
          // UInt32Lo16 expects that the high part is zero, so we can treat it as 32-bit index.
          break;

        case IndexLayout::kUInt32Hi16:
          gather_index = pc->new_similar_reg(pixels[0], "gather_index");
          pc->v_srli_u32(gather_index, idx.clone_as(gather_index), 16);
          break;

        default:
          BL_NOT_REACHED();
      }

      mem.set_index(gather_index);
      mem.set_shift(shift);

      pc->v_zero_i(pixels[0]);
      if (pc->has_avx512()) {
        KReg pred = cc->new_kw("pred");
        cc->kxnorw(pred, pred, pred);
        cc->k(pred).vpgatherdd(pixels[0], mem);
      }
      else {
        Vec pred = pc->new_similar_reg(pixels[0], "pred");
        pc->v_ones_i(pred);
        cc->vpgatherdd(pixels[0], mem, pred);
      }

      for (uint32_t i = 0; i < count; i++)
        cb(i, cb_data);

      convert_gathered_pixels(pc, p, n, flags, pixels);
      cb(0xFF, cb_data);
      return;
    }

    if (bpp == 8) {
      VecArray pixels;

      if (n <= PixelCount(4)) {
        pc->new_vec256_array(pixels, 1, p.name(), "pc");
      }
      else if (pc->use_512bit_simd()) {
        pc->new_vec512_array(pixels, uint32_t(n) / 8u, p.name(), "pc");
      }
      else {
        pc->new_vec256_array(pixels, 2, p.name(), "pc");
      }

      Vec gather_index = idx.clone_as(pixels[0]);

      switch (index_layout) {
        case IndexLayout::kUInt16:
          gather_index = pc->new_similar_reg(pixels[0], "gather_index");
          cc->vpmovzxwd(gather_index, idx.xmm());
          break;

        case IndexLayout::kUInt32:
        case IndexLayout::kUInt32Lo16:
          // UInt32Lo16 expects that the high part is zero, so we can treat it as 32-bit index.
          break;

        case IndexLayout::kUInt32Hi16:
          gather_index = pc->new_similar_reg(pixels[0], "gather_index");
          pc->v_srli_u32(gather_index, idx.clone_as(gather_index), 16);
          break;

        default:
          BL_NOT_REACHED();
      }

      if (pc->use_512bit_simd() && n >= PixelCount(8))
        mem.set_index(gather_index.ymm());
      else
        mem.set_index(gather_index.xmm());
      mem.set_shift(shift);

      for (uint32_t i = 0; i < pixels.size(); i++) {
        if (i == 1) {
          if (pc->use_512bit_simd() && n == PixelCount(16)) {
            Vec gi2 = pc->new_similar_reg(gather_index, "gatherIndex2");
            cc->vextracti32x8(gi2.ymm(), gather_index.zmm(), 1);
            mem.set_index(gi2.ymm());
          }
          else {
            Vec gi2 = pc->new_similar_reg(gather_index, "gatherIndex2");
            cc->vextracti128(gi2.xmm(), gather_index.ymm(), 1);
            mem.set_index(gi2.xmm());
          }
        }

        pc->v_zero_i(pixels[i]);
        if (pc->has_avx512()) {
          KReg pred = cc->new_kw("pred");
          cc->kxnorw(pred, pred, pred);
          cc->k(pred).vpgatherdq(pixels[i], mem);
        }
        else {
          Vec pred = pc->new_similar_reg(pixels[i], "pred");
          pc->v_ones_i(pred);
          cc->vpgatherdq(pixels[i], mem, pred);
        }

        uint32_t granularity = pixels[i].size() / 8u;
        for (uint32_t step = 0; step < granularity; step++)
          cb(i * granularity + step, cb_data);
      }

      convert_gathered_pixels(pc, p, n, flags, pixels);
      cb(0xFF, cb_data);
      return;
    }
  }
#endif // BL_JIT_ARCH_X86

  uint32_t index_type = 0;
  const uint8_t* index_sequence = nullptr;

  static const uint8_t odd_indexes[] = { 1, 3, 5, 7, 9, 11, 13, 15 };
  static const uint8_t even_indexes[] = { 0, 2, 4, 6, 8, 10, 12, 14 };
  static const uint8_t consecutive_indexes[] = { 0, 1, 2, 3, 4, 5, 6, 7 };

  switch (index_layout) {
    case IndexLayout::kUInt16:
      index_type = IndexExtractor::kTypeUInt16;
      index_sequence = consecutive_indexes;
      break;

    case IndexLayout::kUInt32:
      index_type = IndexExtractor::kTypeUInt32;
      index_sequence = consecutive_indexes;
      break;

    case IndexLayout::kUInt32Lo16:
      index_type = IndexExtractor::kTypeUInt16;
      index_sequence = even_indexes;
      break;

    case IndexLayout::kUInt32Hi16:
      index_type = IndexExtractor::kTypeUInt16;
      index_sequence = odd_indexes;
      break;

    default:
      BL_NOT_REACHED();
  }

  IndexExtractor index_extractor(pc);
  index_extractor.begin(index_type, idx);

  FetchContext fCtx(pc, &p, n, flags, f_info, mode);
  fCtx._fetch_all(src, shift, index_extractor, index_sequence, cb, cb_data);
  fCtx.end();
}

} // {bl::Pipeline::JIT::FetchUtils}

#endif // !BL_BUILD_NO_JIT
