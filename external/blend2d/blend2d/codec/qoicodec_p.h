// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_CODEC_QOICODEC_P_H_INCLUDED
#define BLEND2D_CODEC_QOICODEC_P_H_INCLUDED

#include <blend2d/core/api-internal_p.h>
#include <blend2d/core/image_p.h>
#include <blend2d/core/imagecodec.h>
#include <blend2d/core/imagedecoder.h>
#include <blend2d/core/imageencoder.h>
#include <blend2d/core/runtime_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_codec_impl
//! \{

namespace bl::Qoi {

BL_HIDDEN void qoi_codec_on_init(BLRuntimeContext* rt, BLArray<BLImageCodec>* codecs) noexcept;

} // {bl::Qoi}

struct BLQoiDecoderImpl : public BLImageDecoderImpl {
  //! Decoder image information.
  BLImageInfo image_info;
};

struct BLQoiEncoderImpl : public BLImageEncoderImpl {
};

struct BLQoiCodecImpl : public BLImageCodecImpl {
};

//! \}
//! \endcond

#endif // BLEND2D_CODEC_QOICODEC_P_H_INCLUDED
