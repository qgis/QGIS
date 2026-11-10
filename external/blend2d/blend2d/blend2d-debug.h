// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

// ----------------------------------------------------------------------------
// IMPORTANT: DO NOT USE THIS HEADER IN PRODUCTION, ONLY FOR BUG REPORTING!
//
// This file provides debug helpers that are not a part of Blend2D library.
// Functions this header provides are not exported, they are only provided
// for making it easier to report bugs and to give more information about the
// environment where such bugs happened to Blend2D developers. The functions
// defined below are not stable and are subject to change in future Blend2D
// releases.
//
// This file can be included by both C and C++ API users.
// ----------------------------------------------------------------------------

#ifndef BLEND2D_DEBUG_H_INCLUDED
#define BLEND2D_DEBUG_H_INCLUDED

#include <blend2d/blend2d.h>

#include <stdio.h>

//! \cond INTERNAL

// Forward Declarations
// ====================

static void bl_debug_object_(const void* obj, const char* name, int indent);

// BLDebug - Begin
// ===============

#define BL_DEBUG_OUT(MSG) bl_runtime_message_fmt("%*s%s", indent * 2, "", MSG);
#define BL_DEBUG_FMT(FMT, ...) bl_runtime_message_fmt("%*s" FMT, indent * 2, "", __VA_ARGS__);

// BLDebug - Utilities
// ===================

static const char* bl_debug_get_enum_as_string(uint32_t value, const char* enum_data) {
  uint32_t i = 0;
  const char* p = enum_data;

  while (i != value) {
    // 0 indicates end of data.
    if (!*p)
      return "Unknown";

    // Skip the entire sub-string.
    while (*++p)
      continue;

    i++; // Advance the current index.
    p++; // Skip the zero value.
  }

  return p;
}

// BLDebug - Runtime
// =================

static void bl_debug_runtime_cpu_features(char* buf, size_t buf_size, uint32_t arch, uint32_t features) {
  if (!arch) {
    #if defined(_M_X64) || defined(__x86_64__) || defined(_M_IX86) || defined(__X86__) || defined(__i386__)
    arch = BL_RUNTIME_CPU_ARCH_X86;
    #endif

    #if defined(__arm64__) || defined(__aarch64__) || defined(_M_ARM) || defined(_M_ARMT) || defined(__arm__) || defined(__thumb__) || defined(__thumb2__)
    arch = BL_RUNTIME_CPU_ARCH_ARM;
    #endif

    #if defined(_MIPS_ARCH_MIPS64) || defined(__mips64) || defined(_MIPS_ARCH_MIPS32) || defined(_M_MRX000) || defined(__mips__)
    arch = BL_RUNTIME_CPU_ARCH_MIPS;
    #endif
  }

  switch (arch) {
    case BL_RUNTIME_CPU_ARCH_X86:
      snprintf(buf, buf_size, "%s%s%s%s%s%s%s%s",
        (features & BL_RUNTIME_CPU_FEATURE_X86_SSE2  ) ? "SSE2 "   : "",
        (features & BL_RUNTIME_CPU_FEATURE_X86_SSE3  ) ? "SSE3 "   : "",
        (features & BL_RUNTIME_CPU_FEATURE_X86_SSSE3 ) ? "SSSE3 "  : "",
        (features & BL_RUNTIME_CPU_FEATURE_X86_SSE4_1) ? "SSE4.1 " : "",
        (features & BL_RUNTIME_CPU_FEATURE_X86_SSE4_2) ? "SSE4.2 " : "",
        (features & BL_RUNTIME_CPU_FEATURE_X86_AVX   ) ? "AVX "    : "",
        (features & BL_RUNTIME_CPU_FEATURE_X86_AVX2  ) ? "AVX2 "   : "",
        (features & BL_RUNTIME_CPU_FEATURE_X86_AVX512) ? "AVX512 " : "");
      break;

    default:
      buf[0] = '\0';
      break;
  }
}

//! Dumps `BLRuntimeBuildInfo` queried through `bl_runtime_query_info()`.
static void bl_debug_runtime_build_info(void) {
  const char* build_mode = "";
  char baseline_cpu_features[128];
  char supported_cpu_features[128];

  BLRuntimeBuildInfo info;
  bl_runtime_query_info(BL_RUNTIME_INFO_TYPE_BUILD, &info);

  #if defined(BL_STATIC)
  build_mode = "Static";
  #else
  build_mode = "Shared";
  #endif

  bl_debug_runtime_cpu_features(baseline_cpu_features, 128, 0, info.baseline_cpu_features);
  bl_debug_runtime_cpu_features(supported_cpu_features, 128, 0, info.supported_cpu_features);

  bl_runtime_message_fmt(
    "BuildInformation: {\n"
    "  Version: %u.%u.%u\n"
    "  BuildType: %s\n"
    "  BuildMode: %s\n"
    "  BaselineCpuFeatures: %s\n"
    "  SupportedCpuFeatures: %s\n"
    "  Compiler: %s\n"
    "  MaxImageSize: %u\n"
    "  MaxThreadCount: %u\n"
    "}\n",
    info.major_version,
    info.minor_version,
    info.patch_version,
    info.build_type == BL_RUNTIME_BUILD_TYPE_DEBUG ? "Debug" : "Release",
    build_mode,
    baseline_cpu_features,
    supported_cpu_features,
    info.compiler_info,
    info.max_image_size,
    info.max_thread_count);
}

//! Dumps `BLRuntimeSystemInfo` queried through `bl_runtime_query_info()`.
static void bl_debug_runtime_system_info(void) {
  static const char cpu_arch_enum[] =
    "NONE\0"
    "X86\0"
    "ARM\0"
    "MIPS\0";

  const char* os = "Unknown";
  char cpu_features[128];

  BLRuntimeSystemInfo info;
  bl_runtime_query_info(BL_RUNTIME_INFO_TYPE_SYSTEM, &info);
  cpu_features[0] = '\0';

#if defined(__linux__)
  os = "Linux";
#elif defined(__APPLE__)
  os = "Apple";
#elif defined(__DragonFly__)
  os = "DragonFlyBSD";
#elif defined(__FreeBSD__)
  os = "FreeBSD";
#elif defined(__NetBSD__)
  os = "NetBSD";
#elif defined(__OpenBSD__)
  os = "OpenBSD";
#elif defined(__HAIKU__)
  os = "Haiku";
#elif defined(_WIN32)
  os = "Windows";
#endif

  bl_debug_runtime_cpu_features(cpu_features, 128, info.cpu_arch, info.cpu_features);

  bl_runtime_message_fmt(
    "SystemInformation: {\n"
    "  OperatingSystem: %s\n"
    "  CpuArch: %s [%u bit]\n"
    "  CpuFeatures: %s\n"
    "  ThreadCount: %u\n"
    "  ThreadStackSize: %u\n"
    "  AllocationGranularity: %u\n"
    "}\n",
    os,
    bl_debug_get_enum_as_string(info.cpu_arch, cpu_arch_enum), sizeof(void*) >= 8 ? 64 : 32,
    cpu_features,
    info.thread_count,
    info.thread_stack_size,
    info.allocation_granularity);
}

// BLDebug - Matrix
// ================

static void bl_debug_matrix2d_(const BLMatrix2D* obj, const char* name, int indent) {
  static const char matrix_type_enum[] =
    "IDENTITY\0"
    "TRANSLATE\0"
    "SCALE\0"
    "SWAP\0"
    "AFFINE\0"
    "INVALID\0";

  BL_DEBUG_FMT("%s: [%s] {\n", name, bl_debug_get_enum_as_string(bl_matrix2d_get_type(obj), matrix_type_enum));
  BL_DEBUG_FMT("  [% 3.14f |% 3.14f]\n", obj->m00, obj->m01);
  BL_DEBUG_FMT("  [% 3.14f |% 3.14f]\n", obj->m10, obj->m11);
  BL_DEBUG_FMT("  [% 3.14f |% 3.14f]\n", obj->m20, obj->m21);
  BL_DEBUG_OUT("}\n");
}

// BLDebug - StrokeOptions
// =======================

static void bl_debug_stroke_options_(const BLStrokeOptionsCore* obj, const char* name, int indent) {
  static const char stroke_cap_position_enum[] =
    "StartCap\0"
    "EndCap\0";

  static const char stroke_cap_enum[] =
    "BUTT\0"
    "SQUARE\0"
    "ROUND\0"
    "ROUND_REV\0"
    "TRIANGLE\0"
    "TRIANGLE_REV\0";

  static const char stroke_join_enum[] =
    "MITER_CLIP\0"
    "MITER_BEVEL\0"
    "MITER_ROUND\0"
    "BEVEL\0ROUND\0";

  static const char stroke_transform_order_enum[] =
    "AFTER\0"
    "BEFORE\0";

  uint32_t i;

  BL_DEBUG_FMT("%s: {\n", name);
  indent++;

  for (i = 0; i <= BL_STROKE_CAP_POSITION_MAX_VALUE; i++)
    BL_DEBUG_FMT("%s: %s\n", bl_debug_get_enum_as_string(i, stroke_cap_position_enum), bl_debug_get_enum_as_string(obj->caps[i], stroke_cap_enum));
  BL_DEBUG_FMT("Join: %s\n", bl_debug_get_enum_as_string(obj->join, stroke_join_enum));
  BL_DEBUG_FMT("TransformOrder: %s\n", bl_debug_get_enum_as_string(obj->transform_order, stroke_transform_order_enum));
  BL_DEBUG_FMT("Width: %g\n", obj->width);
  BL_DEBUG_FMT("MiterLimit: %g\n", obj->miter_limit);
  BL_DEBUG_FMT("DashOffset: %g\n", obj->dash_offset);
  bl_debug_object_(&obj->dash_array, "DashArray", indent);

  indent--;
  BL_DEBUG_OUT("}\n");
}
static void bl_debug_stroke_options(const BLStrokeOptionsCore* obj) { bl_debug_stroke_options_(obj, "BLStrokeOptions", 0); }

// BLDebug - Array
// ===============

static void bl_debug_array_(const BLArrayCore* obj, const char* name, int indent) {
  BLObjectType object_type = bl_var_get_type(obj);
  const void* void_data = bl_array_get_data(obj);

  size_t i;
  size_t size = bl_array_get_size(obj);

  if (size == 0) {
    BL_DEBUG_FMT("%s: {}\n", name);
    return;
  }

  BL_DEBUG_FMT("%s: {\n", name);
  indent++;

  switch (object_type) {
    case BL_OBJECT_TYPE_ARRAY_OBJECT: {
      char prefix[64];
      const BLObjectCore** data = (const BLObjectCore**)void_data;
      for (i = 0; i < size; i++) {
        snprintf(prefix, 64, "[%zu]", i);
        bl_debug_object_(data[i], prefix, indent);
      }
      break;
    }
    case BL_OBJECT_TYPE_ARRAY_INT8: {
      const int8_t* data = (const int8_t*)void_data;
      for (i = 0; i < size; i++)
        BL_DEBUG_FMT("[%zu] %d", i, data[i]);
      break;
    }
    case BL_OBJECT_TYPE_ARRAY_UINT8: {
      const uint8_t* data = (const uint8_t*)void_data;
      for (i = 0; i < size; i++)
        BL_DEBUG_FMT("[%zu] %u", i, data[i]);
      break;
    }
    case BL_OBJECT_TYPE_ARRAY_INT16: {
      const int16_t* data = (const int16_t*)void_data;
      for (i = 0; i < size; i++)
        BL_DEBUG_FMT("[%zu] %d", i, data[i]);
      break;
    }
    case BL_OBJECT_TYPE_ARRAY_UINT16: {
      const uint16_t* data = (const uint16_t*)void_data;
      for (i = 0; i < size; i++)
        BL_DEBUG_FMT("[%zu] %u", i, data[i]);
      break;
    }
    case BL_OBJECT_TYPE_ARRAY_INT32: {
      const int32_t* data = (const int32_t*)void_data;
      for (i = 0; i < size; i++)
        BL_DEBUG_FMT("[%zu] %d", i, data[i]);
      break;
    }
    case BL_OBJECT_TYPE_ARRAY_UINT32: {
      const uint32_t* data = (const uint32_t*)void_data;
      for (i = 0; i < size; i++)
        BL_DEBUG_FMT("[%zu] %u", i, data[i]);
      break;
    }
    case BL_OBJECT_TYPE_ARRAY_INT64: {
      const int64_t* data = (const int64_t*)void_data;
      for (i = 0; i < size; i++)
        BL_DEBUG_FMT("[%zu] %lld", i, (long long)data[i]);
      break;
    }
    case BL_OBJECT_TYPE_ARRAY_UINT64: {
      const uint64_t* data = (const uint64_t*)void_data;
      for (i = 0; i < size; i++)
        BL_DEBUG_FMT("[%zu] %llu", i, (unsigned long long)data[i]);
      break;
    }
    case BL_OBJECT_TYPE_ARRAY_FLOAT32: {
      const float* data = (const float*)void_data;
      for (i = 0; i < size; i++)
        BL_DEBUG_FMT("[%zu] %g", i, data[i]);
      break;
    }
    case BL_OBJECT_TYPE_ARRAY_FLOAT64: {
      const double* data = (const double*)void_data;
      for (i = 0; i < size; i++)
        BL_DEBUG_FMT("[%zu] %g", i, data[i]);
      break;
    }
    default:
      break;
  }

  indent--;
  BL_DEBUG_OUT("}\n");
}

// BLDebug - Image
// ===============

static void bl_debug_image_(const BLImageCore* obj, const char* name, int indent) {
  static const char format_enum[] =
    "NONE\0"
    "PRGB32\0"
    "XRGB32\0"
    "A8\0";

  BLImageData data;
  bl_image_get_data(obj, &data);

  BL_DEBUG_FMT("%s: {\n", name);
  BL_DEBUG_FMT("  Size: %ux%u\n", data.size.w, data.size.h);
  BL_DEBUG_FMT("  Format: %s\n", bl_debug_get_enum_as_string(data.format, format_enum));
  BL_DEBUG_OUT("}\n");
}

// BLDebug - Pattern
// =================

static void bl_debug_pattern_(const BLPatternCore* obj, const char* name, int indent) {
  static const char extend_mode_enum[] =
    "PAD\0"
    "REPEAT\0"
    "REFLECT\0"
    "PAD_X_REPEAT_Y\0"
    "PAD_X_REFLECT_Y\0"
    "REPEAT_X_PAD_Y\0"
    "REPEAT_X_REFLECT_Y\0"
    "REFLECT_X_PAD_Y\0"
    "REFLECT_X_REPEAT_Y\0";

  BLImageCore image;
  BLMatrix2D transform;

  BLExtendMode extend_mode = bl_pattern_get_extend_mode(obj);

  bl_image_init(&image);
  bl_pattern_get_image(obj, &image);
  bl_pattern_get_transform(obj, &transform);

  BL_DEBUG_FMT("%s: {\n", name);
  {
    indent++;
    bl_debug_image_(&image, "Image", indent);
    BL_DEBUG_FMT("ExtendMode: %s\n", bl_debug_get_enum_as_string(extend_mode, extend_mode_enum));
    bl_debug_matrix2d_(&transform, "Transform", indent);
    indent--;
  }
  BL_DEBUG_OUT("}\n");

  bl_image_destroy(&image);
}

// BLDebug - Gradient
// ==================

static void bl_debug_gradient_(const BLGradientCore* obj, const char* name, int indent) {
  static const char gradient_type_enum[] =
    "LINEAR\0"
    "RADIAL\0"
    "CONIC\0";

  static const char extend_mode_enum[] =
    "PAD\0"
    "REPEAT\0"
    "REFLECT\0";

  size_t i;
  size_t value_count = 0;
  BLMatrix2D transform;

  BLGradientType gradient_type = bl_gradient_get_type(obj);
  BLExtendMode extend_mode = bl_gradient_get_extend_mode(obj);
  size_t stop_count = bl_gradient_get_size(obj);
  const BLGradientStop* stop_data = bl_gradient_get_stops(obj);

  bl_gradient_get_transform(obj, &transform);

  switch (gradient_type) {
    case BL_GRADIENT_TYPE_LINEAR: value_count = 4; break;
    case BL_GRADIENT_TYPE_RADIAL: value_count = 5; break;
    case BL_GRADIENT_TYPE_CONIC : value_count = 3; break;
    default: break;
  }

  double vals[6];
  for (i = 0; i < value_count; i++)
    vals[i] = bl_gradient_get_value(obj, i);

  BL_DEBUG_FMT("%s: {\n", name);
  {
    indent++;
    BL_DEBUG_FMT("Type: %s\n", bl_debug_get_enum_as_string(gradient_type, gradient_type_enum));
    BL_DEBUG_FMT("ExtendMode: %s\n", bl_debug_get_enum_as_string(extend_mode, extend_mode_enum));

    switch (gradient_type) {
      case BL_GRADIENT_TYPE_LINEAR:
        BL_DEBUG_FMT("Values: Start=[%f, %f], End=[%f, %f]\n",
          vals[0], vals[1],
          vals[2], vals[3]);
        break;

      case BL_GRADIENT_TYPE_RADIAL:
        BL_DEBUG_FMT("Values: Center=[%f, %f], Focal=[%f, %f] R=%f\n",
          vals[0], vals[1],
          vals[2], vals[3],
          vals[4]);
        break;

      case BL_GRADIENT_TYPE_CONIC:
        BL_DEBUG_FMT("Values: Center=[%f, %f], Angle=%f\n",
          vals[0], vals[1],
          vals[2]);
        break;

      default:
        break;
    }

    BL_DEBUG_OUT("Stops: {\n");
    indent++;
    for (i = 0; i < stop_count; i++) {
      uint64_t rgba64 = stop_data[i].rgba.value;
      BL_DEBUG_FMT("[%zu] Offset=%f BLRgba64(R=%d, G=%d, B=%d, A=%d)\n",
        i,
        stop_data[i].offset,
        (rgba64 >> 32) & 0xFFFF,
        (rgba64 >> 16) & 0xFFFF,
        (rgba64 >>  0) & 0xFFFF,
        (rgba64 >> 48) & 0xFFFF);
    }
    indent--;
    BL_DEBUG_OUT("}\n");

    bl_debug_matrix2d_(&transform, "Transform", indent);
    indent--;
  }
  BL_DEBUG_OUT("}\n");
}

// BLDebug - Path
// ==============

static void bl_debug_path_(const BLPathCore* obj, const char* name, int indent) {
  size_t i = 0;
  size_t size = bl_path_get_size(obj);

  const uint8_t* cmd = bl_path_get_command_data(obj);
  const BLPoint* vtx = bl_path_get_vertex_data(obj);

  BL_DEBUG_FMT("%s: {\n", name);
  indent++;

  while (i < size) {
    switch (cmd[i]) {
      case BL_PATH_CMD_MOVE:
        BL_DEBUG_FMT("p.move_to(%g, %g);\n", vtx[i].x, vtx[i].y);
        i++;
        continue;
      case BL_PATH_CMD_ON:
        BL_DEBUG_FMT("p.line_to(%g, %g);\n", vtx[i].x, vtx[i].y);
        i++;
        continue;
      case BL_PATH_CMD_QUAD:
        if ((size - i) < 2)
          break;
        BL_DEBUG_FMT("p.quad_to(%g, %g, %g, %g);\n", vtx[i].x, vtx[i].y, vtx[i+1].x, vtx[i+1].y);
        i += 2;
        continue;
      case BL_PATH_CMD_CUBIC:
        if ((size - i) < 3)
          break;
        BL_DEBUG_FMT("p.cubic_to(%g, %g, %g, %g, %g, %g);\n", vtx[i].x, vtx[i].y, vtx[i+1].x, vtx[i+1].y, vtx[i+2].x, vtx[i+2].y);
        i += 3;
        continue;
      case BL_PATH_CMD_CLOSE:
        BL_DEBUG_OUT("p.close();\n");
        i++;
        continue;
    }

    BL_DEBUG_FMT("p.unknown_command(%u, %g, %g);\n", cmd[i], vtx[i].x, vtx[i].y);
    i++;
  }

  indent--;
  BL_DEBUG_OUT("}\n");
}

// BLDebug - FontFeatureSettings
// =============================

static void bl_debug_font_feature_settings_(const BLFontFeatureSettingsCore* obj, const char* name, int indent) {
  size_t i;
  BLFontFeatureSettingsView view;

  bl_font_feature_settings_get_view(obj, &view);

  if (view.size == 0) {
    BL_DEBUG_FMT("%s: {}\n", name);
  }
  else {
    BL_DEBUG_FMT("%s: {\n", name);
    indent++;

    for (i = 0; i < view.size; i++) {
      BLTag tag = view.data[i].tag;
      uint32_t value = view.data[i].value;
      BL_DEBUG_FMT("'%c%c%c%c': %u\n",
        (tag >> 24) & 0xFF,
        (tag >> 16) & 0xFF,
        (tag >>  8) & 0xFF,
        (tag >>  0) & 0xFF,
        (unsigned)value);
    }

    indent--;
    BL_DEBUG_OUT("}\n");
  }
}

// BLDebug - FontVariationSettings
// ==============================

static void bl_debug_font_variation_settings_(const BLFontVariationSettingsCore* obj, const char* name, int indent) {
  size_t i;
  BLFontVariationSettingsView view;

  bl_font_variation_settings_get_view(obj, &view);

  if (view.size == 0) {
    BL_DEBUG_FMT("%s: {}\n", name);
  }
  else {
    BL_DEBUG_FMT("%s: {\n", name);
    indent++;

    for (i = 0; i < view.size; i++) {
      BLTag tag = view.data[i].tag;
      float value = view.data[i].value;
      BL_DEBUG_FMT("'%c%c%c%c': %f\n",
        (tag >> 24) & 0xFF,
        (tag >> 16) & 0xFF,
        (tag >>  8) & 0xFF,
        (tag >>  0) & 0xFF,
        value);
    }

    indent--;
    BL_DEBUG_OUT("}\n");
  }
}

// BLDebug - Font
// ==============

static void bl_debug_font_(const BLFontCore* obj, const char* name, int indent) {
  float size = bl_font_get_size(obj);

  BLStringCore str;
  BLFontFaceCore face;
  BLFontFeatureSettingsCore features;
  BLFontVariationSettingsCore variations;

  bl_string_init(&str);
  bl_font_face_init(&face);
  bl_font_feature_settings_init(&features);
  bl_font_variation_settings_init(&variations);

  bl_font_get_face(obj, &face);
  bl_font_get_feature_settings(obj, &features);
  bl_font_get_variation_settings(obj, &variations);

  BL_DEBUG_FMT("%s: {\n", name);
  {
    indent++;
    bl_font_face_get_family_name(&face, &str);
    BL_DEBUG_FMT("Face: %s\n", bl_string_get_data(&str));
    BL_DEBUG_FMT("Size: %f\n", size);
    bl_debug_font_feature_settings_(&features, "FeatureSettings", indent);
    bl_debug_font_variation_settings_(&variations, "VariationSettings", indent);
    indent--;
  }
  BL_DEBUG_OUT("}\n");

  bl_font_variation_settings_destroy(&variations);
  bl_font_feature_settings_destroy(&features);
  bl_font_face_destroy(&face);
  bl_string_destroy(&str);
}

// BLDebug - Context
// =================

static void bl_debug_context_(const BLContextCore* obj, const char* name, int indent) {
  static const char context_type_enum[] =
    "NONE\0"
    "DUMMY\0"
    "PROXY\0"
    "RASTER\0";

  static const char fill_rule_enum[] =
    "NON_ZERO\0"
    "EVEN_ODD\0";

  const BLContextState* state = ((BLContextImpl*)obj->_d.impl)->state;

  BLVarCore fill_style;
  BLVarCore stroke_style;

  bl_var_init_null(&fill_style);
  bl_var_init_null(&stroke_style);

  bl_context_get_transformed_fill_style(obj, &fill_style);
  bl_context_get_transformed_stroke_style(obj, &stroke_style);

  BL_DEBUG_FMT("%s: {\n", name);
  {
    indent++;
    BL_DEBUG_FMT("Type: %s\n", bl_debug_get_enum_as_string(bl_context_get_type(obj), context_type_enum));
    BL_DEBUG_FMT("GlobalAlpha: %f\n", state->global_alpha);
    BL_DEBUG_FMT("SavedStateCount: %u\n", state->saved_state_count);

    bl_debug_matrix2d_(&state->meta_transform, "MetaTransform", indent);
    bl_debug_matrix2d_(&state->user_transform, "UserTransform", indent);
    bl_debug_matrix2d_(&state->final_transform, "FinalTransform", indent);

    bl_debug_object_(&fill_style, "FillStyle", indent);
    BL_DEBUG_FMT("FillAlpha: %f\n", state->style_alpha[BL_CONTEXT_STYLE_SLOT_FILL]);
    BL_DEBUG_FMT("FillRule: %s\n", bl_debug_get_enum_as_string(state->fill_rule, fill_rule_enum));

    bl_debug_object_(&fill_style, "StrokeStyle", indent);
    BL_DEBUG_FMT("StrokeAlpha: %f\n", state->style_alpha[BL_CONTEXT_STYLE_SLOT_STROKE]);
    bl_debug_stroke_options_(&state->stroke_options, "StrokeOptions", indent);
    indent--;
  }
  BL_DEBUG_OUT("}\n");

  bl_var_destroy(&stroke_style);
  bl_var_destroy(&fill_style);
}

// BLDebug - Object
// ================

static void bl_debug_object_(const void* obj, const char* name, int indent) {
  BLObjectType type = bl_var_get_type(obj);
  switch (type) {
    case BL_OBJECT_TYPE_RGBA: {
      BLRgba rgba;
      bl_var_to_rgba(obj, &rgba);
      BL_DEBUG_FMT("%s: Rgba(R=%f, G=%f, B=%f, A=%f)\n",
        name,
        rgba.r,
        rgba.g,
        rgba.b,
        rgba.a);
      break;
    }

    case BL_OBJECT_TYPE_RGBA32: {
      uint32_t rgba32;
      bl_var_to_rgba32(obj, &rgba32);
      BL_DEBUG_FMT("%s: BLRgba32(R=%d, G=%d, B=%d, A=%d)\n",
        name,
        (rgba32 >> 16) & 0xFF,
        (rgba32 >>  8) & 0xFF,
        (rgba32 >>  0) & 0xFF,
        (rgba32 >> 24) & 0xFF);
      break;
    }

    case BL_OBJECT_TYPE_RGBA64: {
      uint64_t rgba64;
      bl_var_to_rgba64(obj, &rgba64);
      BL_DEBUG_FMT("%s: BLRgba64(R=%d, G=%d, B=%d, A=%d)\n",
        name,
        (rgba64 >> 32) & 0xFFFF,
        (rgba64 >> 16) & 0xFFFF,
        (rgba64 >>  0) & 0xFFFF,
        (rgba64 >> 48) & 0xFFFF);
      break;
    }

    case BL_OBJECT_TYPE_NULL: {
      BL_DEBUG_FMT("%s: Null\n", name);
      break;
    }

    case BL_OBJECT_TYPE_PATTERN: {
      bl_debug_pattern_((const BLPatternCore*)obj, name, indent);
      break;
    }

    case BL_OBJECT_TYPE_GRADIENT: {
      bl_debug_gradient_((const BLGradientCore*)obj, name, indent);
      break;
    }

    case BL_OBJECT_TYPE_IMAGE: {
      bl_debug_image_((const BLImageCore*)obj, name, indent);
      break;
    }

    case BL_OBJECT_TYPE_PATH: {
      bl_debug_path_((const BLPathCore*)obj, name, indent);
      break;
    }

    case BL_OBJECT_TYPE_FONT: {
      bl_debug_font_((BLFontCore*)obj, name, indent);
      break;
    }

    case BL_OBJECT_TYPE_FONT_FEATURE_SETTINGS: {
      bl_debug_font_feature_settings_((BLFontFeatureSettingsCore*)obj, name, indent);
      break;
    }

    case BL_OBJECT_TYPE_FONT_VARIATION_SETTINGS: {
      bl_debug_font_variation_settings_((BLFontVariationSettingsCore*)obj, name, indent);
      break;
    }

    case BL_OBJECT_TYPE_BOOL: {
      bool val;
      bl_var_to_bool(obj, &val);
      BL_DEBUG_FMT("%s: Bool(%s)\n", name, val ? "true" : "false");
      break;
    }

    case BL_OBJECT_TYPE_INT64: {
      int64_t val;
      bl_var_to_int64(obj, &val);
      BL_DEBUG_FMT("%s: Int64(%lld)\n", name, (long long)val);
      break;
    }

    case BL_OBJECT_TYPE_UINT64: {
      uint64_t val;
      bl_var_to_uint64(obj, &val);
      BL_DEBUG_FMT("%s: UInt64(%llu)\n", name, (unsigned long long)val);
      break;
    }

    case BL_OBJECT_TYPE_DOUBLE: {
      double val;
      bl_var_to_double(obj, &val);
      BL_DEBUG_FMT("%s: Double(%f)\n", name, val);
      break;
    }

    case BL_OBJECT_TYPE_ARRAY_OBJECT:
    case BL_OBJECT_TYPE_ARRAY_INT8:
    case BL_OBJECT_TYPE_ARRAY_UINT8:
    case BL_OBJECT_TYPE_ARRAY_INT16:
    case BL_OBJECT_TYPE_ARRAY_UINT16:
    case BL_OBJECT_TYPE_ARRAY_INT32:
    case BL_OBJECT_TYPE_ARRAY_UINT32:
    case BL_OBJECT_TYPE_ARRAY_INT64:
    case BL_OBJECT_TYPE_ARRAY_UINT64:
    case BL_OBJECT_TYPE_ARRAY_FLOAT32:
    case BL_OBJECT_TYPE_ARRAY_FLOAT64:
    case BL_OBJECT_TYPE_ARRAY_STRUCT_1:
    case BL_OBJECT_TYPE_ARRAY_STRUCT_2:
    case BL_OBJECT_TYPE_ARRAY_STRUCT_3:
    case BL_OBJECT_TYPE_ARRAY_STRUCT_4:
    case BL_OBJECT_TYPE_ARRAY_STRUCT_6:
    case BL_OBJECT_TYPE_ARRAY_STRUCT_8:
    case BL_OBJECT_TYPE_ARRAY_STRUCT_10:
    case BL_OBJECT_TYPE_ARRAY_STRUCT_12:
    case BL_OBJECT_TYPE_ARRAY_STRUCT_16:
    case BL_OBJECT_TYPE_ARRAY_STRUCT_20:
    case BL_OBJECT_TYPE_ARRAY_STRUCT_24:
    case BL_OBJECT_TYPE_ARRAY_STRUCT_32:
      bl_debug_array_((const BLArrayCore*)obj, name, indent);
      break;

    case BL_OBJECT_TYPE_CONTEXT:
      bl_debug_context_((const BLContextCore*)obj, name, indent);
      break;

    default:
      BL_DEBUG_FMT("BLObject { Type: %u }\n", (uint32_t)type);
      break;
  }
}

// BLDebug - Public API
// ====================

//! Dumps both `BLRuntimeBuildInfo` and `BLRuntimeSystemInfo`.
//!
//! This function should be used to retrieve information for Blend2D bug reports.
static void bl_debug_runtime(void) {
  bl_debug_runtime_build_info();
  bl_debug_runtime_system_info();
}

//! Dumps BLArrayCore or BLArray<T>.
static void bl_debug_array(const BLArrayCore* obj) {
  bl_debug_array_(obj, "BLArray", 0);
}

//! Dumps BLContextCore or BLContext.
static void bl_debug_context(const BLContextCore* obj) {
  bl_debug_context_(obj, "BLContext", 0);
}

//! Dumps BLFontCore or BLFont.
static void bl_debug_font(const BLFontCore* obj) {
  bl_debug_font_(obj, "BLFont", 0);
}

//! Dumps BLFontFeatureSettingsCore or BLFontFeatureSettings.
static void bl_debug_font_feature_settings(const BLFontFeatureSettingsCore* obj) {
  bl_debug_font_feature_settings_(obj, "BLFontFeatureSettings", 0);
}

//! Dumps BLFontVariationSettingsCore or BLFontVariationSettings.
static void bl_debug_font_variation_settings(const BLFontVariationSettingsCore* obj) {
  bl_debug_font_variation_settings_(obj, "BLFontVariationSettings", 0);
}

//! Dumps BLGradientCore or BLGradient.
static void bl_debug_gradient(const BLGradientCore* obj) {
  bl_debug_gradient_(obj, "BLGradient", 0);
}

//! Dumps BLImageCore or BLImage.
static void bl_debug_image(const BLImageCore* obj) {
  bl_debug_image_(obj, "BLImage", 0);
}

static void bl_debug_matrix2d(const BLMatrix2D* obj) {
  bl_debug_matrix2d_(obj, "BLMatrix", 0);
}

//! Dumps BLObjectCore or BLObject.
//!
//! You can use this function with any object that implements `BLObject` interface.
static void bl_debug_object(const void* obj) {
  bl_debug_object_(obj, "BLObject", 0);
}

//! Dumps BLPathCore or BLPath.
static void bl_debug_path(const BLPathCore* obj) {
  bl_debug_path_(obj, "BLPath", 0);
}

//! Dumps BLPatternCore or BLPattern.
static void bl_debug_pattern(const BLPatternCore* obj) {
  bl_debug_pattern_(obj, "BLPattern", 0);
}

// BLDebug - End
// =============

#undef BL_DEBUG_FMT
#undef BL_DEBUG_OUT

//! \endcond

#endif // BLEND2D_DEBUG_H_INCLUDED
