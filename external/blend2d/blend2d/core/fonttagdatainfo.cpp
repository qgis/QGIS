// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#include <blend2d/core/fonttagdatainfo_p.h>

namespace bl {
namespace FontTagData {

struct FeatureInfoTableGen {
  static constexpr FeatureInfo value(size_t id) noexcept {
    return FeatureInfo {
      // Enabled by default:
      uint8_t(id == uint32_t(FeatureId::kCALT) ? 1u :
              id == uint32_t(FeatureId::kCLIG) ? 1u :
              id == uint32_t(FeatureId::kCPSP) ? 1u :
              id == uint32_t(FeatureId::kKERN) ? 1u :
              id == uint32_t(FeatureId::kLIGA) ? 1u :
              id == uint32_t(FeatureId::kOPBD) ? 1u :
              id == uint32_t(FeatureId::kRVRN) ? 1u :
              0u),

      // User control:
      uint8_t(id == uint32_t(FeatureId::kAALT) ? 1u :
              id == uint32_t(FeatureId::kAFRC) ? 1u :
              id == uint32_t(FeatureId::kC2PC) ? 1u :
              id == uint32_t(FeatureId::kC2SC) ? 1u :
              id == uint32_t(FeatureId::kCALT) ? 1u :
              id == uint32_t(FeatureId::kCASE) ? 1u :
              id == uint32_t(FeatureId::kCHWS) ? 1u :
              id == uint32_t(FeatureId::kCLIG) ? 1u :
              id == uint32_t(FeatureId::kCPCT) ? 1u :
              id == uint32_t(FeatureId::kCPSP) ? 1u :
              id == uint32_t(FeatureId::kCSWH) ? 1u :
              id >= uint32_t(FeatureId::kCV01) && id <= uint32_t(FeatureId::kCV99) ? 1u :
              id == uint32_t(FeatureId::kDLIG) ? 1u :
              id == uint32_t(FeatureId::kDNOM) ? 1u :
              id == uint32_t(FeatureId::kEXPT) ? 1u :
              id == uint32_t(FeatureId::kFALT) ? 1u :
              id == uint32_t(FeatureId::kFRAC) ? 1u :
              id == uint32_t(FeatureId::kFWID) ? 1u :
              id == uint32_t(FeatureId::kHALT) ? 1u :
              id == uint32_t(FeatureId::kHIST) ? 1u :
              id == uint32_t(FeatureId::kHKNA) ? 1u :
              id == uint32_t(FeatureId::kHLIG) ? 1u :
              id == uint32_t(FeatureId::kHNGL) ? 1u :
              id == uint32_t(FeatureId::kHOJO) ? 1u :
              id == uint32_t(FeatureId::kHWID) ? 1u :
              id == uint32_t(FeatureId::kJALT) ? 1u :
              id == uint32_t(FeatureId::kJP04) ? 1u :
              id == uint32_t(FeatureId::kJP78) ? 1u :
              id == uint32_t(FeatureId::kJP83) ? 1u :
              id == uint32_t(FeatureId::kJP90) ? 1u :
              id == uint32_t(FeatureId::kKERN) ? 1u :
              id == uint32_t(FeatureId::kLFBD) ? 1u :
              id == uint32_t(FeatureId::kLIGA) ? 1u :
              id == uint32_t(FeatureId::kLNUM) ? 1u :
              id == uint32_t(FeatureId::kMGRK) ? 1u :
              id == uint32_t(FeatureId::kNALT) ? 1u :
              id == uint32_t(FeatureId::kNLCK) ? 1u :
              id == uint32_t(FeatureId::kONUM) ? 1u :
              id == uint32_t(FeatureId::kOPBD) ? 1u :
              id == uint32_t(FeatureId::kORDN) ? 1u :
              id == uint32_t(FeatureId::kORNM) ? 1u :
              id == uint32_t(FeatureId::kPALT) ? 1u :
              id == uint32_t(FeatureId::kPCAP) ? 1u :
              id == uint32_t(FeatureId::kPKNA) ? 1u :
              id == uint32_t(FeatureId::kPNUM) ? 1u :
              id == uint32_t(FeatureId::kPWID) ? 1u :
              id == uint32_t(FeatureId::kQWID) ? 1u :
              id == uint32_t(FeatureId::kRAND) ? 1u :
              id == uint32_t(FeatureId::kRTBD) ? 1u :
              id == uint32_t(FeatureId::kRUBY) ? 1u :
              id == uint32_t(FeatureId::kSALT) ? 1u :
              id == uint32_t(FeatureId::kSINF) ? 1u :
              id == uint32_t(FeatureId::kSMCP) ? 1u :
              id == uint32_t(FeatureId::kSMPL) ? 1u :
              id >= uint32_t(FeatureId::kSS01) && id <= uint32_t(FeatureId::kSS20) ? 1u :
              id == uint32_t(FeatureId::kSUBS) ? 1u :
              id == uint32_t(FeatureId::kSUPS) ? 1u :
              id == uint32_t(FeatureId::kSWSH) ? 1u :
              id == uint32_t(FeatureId::kTITL) ? 1u :
              id == uint32_t(FeatureId::kTNAM) ? 1u :
              id == uint32_t(FeatureId::kTNUM) ? 1u :
              id == uint32_t(FeatureId::kTRAD) ? 1u :
              id == uint32_t(FeatureId::kTWID) ? 1u :
              id == uint32_t(FeatureId::kUNIC) ? 1u :
              id == uint32_t(FeatureId::kVHAL) ? 1u :
              id == uint32_t(FeatureId::kVKNA) ? 1u :
              id == uint32_t(FeatureId::kVKRN) ? 1u :
              id == uint32_t(FeatureId::kVPAL) ? 1u :
              id == uint32_t(FeatureId::kVRT2) ? 1u :
              id == uint32_t(FeatureId::kVRTR) ? 1u :
              id == uint32_t(FeatureId::kZERO) ? 1u :
              0u),

      // Feature bit id:
      uint8_t(id == uint32_t(FeatureId::kCASE) ? 0u :
              id == uint32_t(FeatureId::kCLIG) ? 1u :
              id == uint32_t(FeatureId::kCPCT) ? 2u :
              id == uint32_t(FeatureId::kCPSP) ? 3u :
              id == uint32_t(FeatureId::kDLIG) ? 4u :
              id == uint32_t(FeatureId::kDNOM) ? 5u :
              id == uint32_t(FeatureId::kEXPT) ? 6u :
              id == uint32_t(FeatureId::kFALT) ? 7u :
              id == uint32_t(FeatureId::kFRAC) ? 8u :
              id == uint32_t(FeatureId::kFWID) ? 9u :
              id == uint32_t(FeatureId::kHALT) ? 10u :
              id == uint32_t(FeatureId::kHIST) ? 11u :
              id == uint32_t(FeatureId::kHWID) ? 12u :
              id == uint32_t(FeatureId::kJALT) ? 13u :
              id == uint32_t(FeatureId::kKERN) ? 14u :
              id == uint32_t(FeatureId::kLIGA) ? 15u :
              id == uint32_t(FeatureId::kLNUM) ? 16u :
              id == uint32_t(FeatureId::kONUM) ? 17u :
              id == uint32_t(FeatureId::kORDN) ? 18u :
              id == uint32_t(FeatureId::kPALT) ? 19u :
              id == uint32_t(FeatureId::kPCAP) ? 20u :
              id == uint32_t(FeatureId::kRUBY) ? 21u :
              id == uint32_t(FeatureId::kSMCP) ? 22u :
              id == uint32_t(FeatureId::kSUBS) ? 23u :
              id == uint32_t(FeatureId::kSUPS) ? 24u :
              id == uint32_t(FeatureId::kTITL) ? 25u :
              id == uint32_t(FeatureId::kTNAM) ? 26u :
              id == uint32_t(FeatureId::kTNUM) ? 27u :
              id == uint32_t(FeatureId::kUNIC) ? 28u :
              id == uint32_t(FeatureId::kVALT) ? 29u :
              id == uint32_t(FeatureId::kVKRN) ? 30u :
              id == uint32_t(FeatureId::kZERO) ? 31u :
              unsigned(kInvalidFeatureBitId))
    };
  }
};

BL_CONSTEXPR_TABLE(feature_info_table, FeatureInfoTableGen, FeatureInfo, kFeatureIdCount + 1u);

// This is a reverse table that must match feature bit its defined by `feature_info_table`.
const uint8_t feature_bit_id_to_feature_id_table[32] = {
  uint8_t(FeatureId::kCASE),
  uint8_t(FeatureId::kCLIG),
  uint8_t(FeatureId::kCPCT),
  uint8_t(FeatureId::kCPSP),
  uint8_t(FeatureId::kDLIG),
  uint8_t(FeatureId::kDNOM),
  uint8_t(FeatureId::kEXPT),
  uint8_t(FeatureId::kFALT),
  uint8_t(FeatureId::kFRAC),
  uint8_t(FeatureId::kFWID),
  uint8_t(FeatureId::kHALT),
  uint8_t(FeatureId::kHIST),
  uint8_t(FeatureId::kHWID),
  uint8_t(FeatureId::kJALT),
  uint8_t(FeatureId::kKERN),
  uint8_t(FeatureId::kLIGA),
  uint8_t(FeatureId::kLNUM),
  uint8_t(FeatureId::kONUM),
  uint8_t(FeatureId::kORDN),
  uint8_t(FeatureId::kPALT),
  uint8_t(FeatureId::kPCAP),
  uint8_t(FeatureId::kRUBY),
  uint8_t(FeatureId::kSMCP),
  uint8_t(FeatureId::kSUBS),
  uint8_t(FeatureId::kSUPS),
  uint8_t(FeatureId::kTITL),
  uint8_t(FeatureId::kTNAM),
  uint8_t(FeatureId::kTNUM),
  uint8_t(FeatureId::kUNIC),
  uint8_t(FeatureId::kVALT),
  uint8_t(FeatureId::kVKRN),
  uint8_t(FeatureId::kZERO)
};

} // {FontTagData}
} // {bl}
