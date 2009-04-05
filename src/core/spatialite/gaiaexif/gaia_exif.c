/*

 gaia_exif.c -- Gaia EXIF support

 version 2.3, 2008 October 13

 Author: Sandro Furieri a.furieri@lqt.it

 ------------------------------------------------------------------------------

 Version: MPL 1.1/GPL 2.0/LGPL 2.1

 The contents of this file are subject to the Mozilla Public License Version
 1.1 (the "License"); you may not use this file except in compliance with
 the License. You may obtain a copy of the License at
 http://www.mozilla.org/MPL/

Software distributed under the License is distributed on an "AS IS" basis,
WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
for the specific language governing rights and limitations under the
License.

The Original Code is the SpatiaLite library

The Initial Developer of the Original Code is Alessandro Furieri

Portions created by the Initial Developer are Copyright (C) 2008
the Initial Developer. All Rights Reserved.

Contributor(s):

Alternatively, the contents of this file may be used under the terms of
either the GNU General Public License Version 2 or later (the "GPL"), or
the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
in which case the provisions of the GPL or the LGPL are applicable instead
of those above. If you wish to allow use of your version of this file only
under the terms of either the GPL or the LGPL, and not to allow others to
use your version of this file under the terms of the MPL, indicate your
decision by deleting the provisions above and replace them with the notice
and other provisions required by the GPL or the LGPL. If you do not delete
the provisions above, a recipient may use your version of this file under
the terms of any one of the MPL, the GPL or the LGPL.

*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <math.h>
#include <float.h>

#include <spatialite/sqlite3ext.h>
#include <spatialite/gaiageo.h>
#include <spatialite/gaiaexif.h>
#include <spatialite.h>

#ifdef _MSC_VER
#define strcasecmp(a,b) _stricmp(a,b)
#endif

static void
exifTagName( char gps, unsigned short tag_id, char *str, int len )
{
  /* returns the canonical name corresponding to an EXIF TAG ID */
  int l;
  char *name = "UNKNOWN";
  if ( gps )
  {
    switch ( tag_id )
    {
      case 0x00:
        name = "GPSVersionID";
        break;
      case 0x01:
        name = "GPSLatitudeRef";
        break;
      case 0x02:
        name = "GPSLatitude";
        break;
      case 0x03:
        name = "GPSLongitudeRef";
        break;
      case 0x04:
        name = "GPSLongitude";
        break;
      case 0x05:
        name = "GPSAltitudeRef";
        break;
      case 0x06:
        name = "GPSAltitude";
        break;
      case 0x07:
        name = "GPSTimeStamp";
        break;
      case 0x08:
        name = "GPSSatellites";
        break;
      case 0x09:
        name = "GPSStatus";
        break;
      case 0x0A:
        name = "GPSMeasureMode";
        break;
      case 0x0B:
        name = "GPSDOP";
        break;
      case 0x0C:
        name = "GPSSpeedRef";
        break;
      case 0x0D:
        name = "GPSSpeed";
        break;
      case 0x0E:
        name = "GPSTrackRef";
        break;
      case 0x0F:
        name = "GPSTrack";
        break;
      case 0x10:
        name = "GPSImgDirectionRef";
        break;
      case 0x11:
        name = "GPSImgDirection";
        break;
      case 0x12:
        name = "GPSMapDatum";
        break;
      case 0x13:
        name = "GPSDestLatitudeRef";
        break;
      case 0x14:
        name = "GPSDestLatitude";
        break;
      case 0x15:
        name = "GPSDestLongitudeRef";
        break;
      case 0x16:
        name = "GPSDestLongitude";
        break;
      case 0x17:
        name = "GPSDestBearingRef";
        break;
      case 0x18:
        name = "GPSDestBearing";
        break;
      case 0x19:
        name = "GPSDestDistanceRef";
        break;
      case 0x1A:
        name = "GPSDestDistance";
        break;
      case 0x1B:
        name = "GPSProcessingMethod";
        break;
      case 0x1C:
        name = "GPSAreaInformation";
        break;
      case 0x1D:
        name = "GPSDateStamp";
        break;
      case 0x1E:
        name = "GPSDifferential";
        break;
    };
  }
  else
  {
    switch ( tag_id )
    {
      case 0x000B:
        name = "ACDComment";
        break;
      case 0x00FE:
        name = "NewSubFile";
        break;
      case 0x00FF:
        name = "SubFile";
        break;
      case 0x0100:
        name = "ImageWidth";
        break;
      case 0x0101:
        name = "ImageLength";
        break;
      case 0x0102:
        name = "BitsPerSample";
        break;
      case 0x0103:
        name = "Compression";
        break;
      case 0x0106:
        name = "PhotometricInterpretation";
        break;
      case 0x010A:
        name = "FillOrder";
        break;
      case 0x010D:
        name = "DocumentName";
        break;
      case 0x010E:
        name = "ImageDescription";
        break;
      case 0x010F:
        name = "Make";
        break;
      case 0x0110:
        name = "Model";
        break;
      case 0x0111:
        name = "StripOffsets";
        break;
      case 0x0112:
        name = "Orientation";
        break;
      case 0x0115:
        name = "SamplesPerPixel";
        break;
      case 0x0116:
        name = "RowsPerStrip";
        break;
      case 0x0117:
        name = "StripByteCounts";
        break;
      case 0x0118:
        name = "MinSampleValue";
        break;
      case 0x0119:
        name = "MaxSampleValue";
        break;
      case 0x011A:
        name = "XResolution";
        break;
      case 0x011B:
        name = "YResolution";
        break;
      case 0x011C:
        name = "PlanarConfiguration";
        break;
      case 0x011D:
        name = "PageName";
        break;
      case 0x011E:
        name = "XPosition";
        break;
      case 0x011F:
        name = "YPosition";
        break;
      case 0x0120:
        name = "FreeOffsets";
        break;
      case 0x0121:
        name = "FreeByteCounts";
        break;
      case 0x0122:
        name = "GrayResponseUnit";
        break;
      case 0x0123:
        name = "GrayResponseCurve";
        break;
      case 0x0124:
        name = "T4Options";
        break;
      case 0x0125:
        name = "T6Options";
        break;
      case 0x0128:
        name = "ResolutionUnit";
        break;
      case 0x0129:
        name = "PageNumber";
        break;
      case 0x012D:
        name = "TransferFunction";
        break;
      case 0x0131:
        name = "Software";
        break;
      case 0x0132:
        name = "DateTime";
        break;
      case 0x013B:
        name = "Artist";
        break;
      case 0x013C:
        name = "HostComputer";
        break;
      case 0x013D:
        name = "Predictor";
        break;
      case 0x013E:
        name = "WhitePoint";
        break;
      case 0x013F:
        name = "PrimaryChromaticities";
        break;
      case 0x0140:
        name = "ColorMap";
        break;
      case 0x0141:
        name = "HalfToneHints";
        break;
      case 0x0142:
        name = "TileWidth";
        break;
      case 0x0143:
        name = "TileLength";
        break;
      case 0x0144:
        name = "TileOffsets";
        break;
      case 0x0145:
        name = "TileByteCounts";
        break;
      case 0x014A:
        name = "SubIFD";
        break;
      case 0x014C:
        name = "InkSet";
        break;
      case 0x014D:
        name = "InkNames";
        break;
      case 0x014E:
        name = "NumberOfInks";
        break;
      case 0x0150:
        name = "DotRange";
        break;
      case 0x0151:
        name = "TargetPrinter";
        break;
      case 0x0152:
        name = "ExtraSample";
        break;
      case 0x0153:
        name = "SampleFormat";
        break;
      case 0x0154:
        name = "SMinSampleValue";
        break;
      case 0x0155:
        name = "SMaxSampleValue";
        break;
      case 0x0156:
        name = "TransferRange";
        break;
      case 0x0157:
        name = "ClipPath";
        break;
      case 0x0158:
        name = "XClipPathUnits";
        break;
      case 0x0159:
        name = "YClipPathUnits";
        break;
      case 0x015A:
        name = "Indexed";
        break;
      case 0x015B:
        name = "JPEGTables";
        break;
      case 0x015F:
        name = "OPIProxy";
        break;
      case 0x0200:
        name = "JPEGProc";
        break;
      case 0x0201:
        name = "JPEGInterchangeFormat";
        break;
      case 0x0202:
        name = "JPEGInterchangeFormatLength";
        break;
      case 0x0203:
        name = "JPEGRestartInterval";
        break;
      case 0x0205:
        name = "JPEGLosslessPredictors";
        break;
      case 0x0206:
        name = "JPEGPointTransforms";
        break;
      case 0x0207:
        name = "JPEGQTables";
        break;
      case 0x0208:
        name = "JPEGDCTables";
        break;
      case 0x0209:
        name = "JPEGACTables";
        break;
      case 0x0211:
        name = "YCbCrCoefficients";
        break;
      case 0x0212:
        name = "YCbCrSubSampling";
        break;
      case 0x0213:
        name = "YCbCrPositioning";
        break;
      case 0x0214:
        name = "ReferenceBlackWhite";
        break;
      case 0x02BC:
        name = "ExtensibleMetadataPlatform";
        break;
      case 0x0301:
        name = "Gamma";
        break;
      case 0x0302:
        name = "ICCProfileDescriptor";
        break;
      case 0x0303:
        name = "SRGBRenderingIntent";
        break;
      case 0x0320:
        name = "ImageTitle";
        break;
      case 0x5001:
        name = "ResolutionXUnit";
        break;
      case 0x5002:
        name = "ResolutionYUnit";
        break;
      case 0x5003:
        name = "ResolutionXLengthUnit";
        break;
      case 0x5004:
        name = "ResolutionYLengthUnit";
        break;
      case 0x5005:
        name = "PrintFlags";
        break;
      case 0x5006:
        name = "PrintFlagsVersion";
        break;
      case 0x5007:
        name = "PrintFlagsCrop";
        break;
      case 0x5008:
        name = "PrintFlagsBleedWidth";
        break;
      case 0x5009:
        name = "PrintFlagsBleedWidthScale";
        break;
      case 0x500A:
        name = "HalftoneLPI";
        break;
      case 0x500B:
        name = "HalftoneLPIUnit";
        break;
      case 0x500C:
        name = "HalftoneDegree";
        break;
      case 0x500D:
        name = "HalftoneShape";
        break;
      case 0x500E:
        name = "HalftoneMisc";
        break;
      case 0x500F:
        name = "HalftoneScreen";
        break;
      case 0x5010:
        name = "JPEGQuality";
        break;
      case 0x5011:
        name = "GridSize";
        break;
      case 0x5012:
        name = "ThumbnailFormat";
        break;
      case 0x5013:
        name = "ThumbnailWidth";
        break;
      case 0x5014:
        name = "ThumbnailHeight";
        break;
      case 0x5015:
        name = "ThumbnailColorDepth";
        break;
      case 0x5016:
        name = "ThumbnailPlanes";
        break;
      case 0x5017:
        name = "ThumbnailRawBytes";
        break;
      case 0x5018:
        name = "ThumbnailSize";
        break;
      case 0x5019:
        name = "ThumbnailCompressedSize";
        break;
      case 0x501A:
        name = "ColorTransferFunction";
        break;
      case 0x501B:
        name = "ThumbnailData";
        break;
      case 0x5020:
        name = "ThumbnailImageWidth";
        break;
      case 0x5021:
        name = "ThumbnailImageHeight";
        break;
      case 0x5022:
        name = "ThumbnailBitsPerSample";
        break;
      case 0x5023:
        name = "ThumbnailCompression";
        break;
      case 0x5024:
        name = "ThumbnailPhotometricInterp";
        break;
      case 0x5025:
        name = "ThumbnailImageDescription";
        break;
      case 0x5026:
        name = "ThumbnailEquipMake";
        break;
      case 0x5027:
        name = "ThumbnailEquipModel";
        break;
      case 0x5028:
        name = "ThumbnailStripOffsets";
        break;
      case 0x5029:
        name = "ThumbnailOrientation";
        break;
      case 0x502A:
        name = "ThumbnailSamplesPerPixel";
        break;
      case 0x502B:
        name = "ThumbnailRowsPerStrip";
        break;
      case 0x502C:
        name = "ThumbnailStripBytesCount";
        break;
      case 0x502D:
        name = "ThumbnailResolutionX";
        break;
      case 0x502E:
        name = "ThumbnailResolutionY";
        break;
      case 0x502F:
        name = "ThumbnailPlanarConfig";
        break;
      case 0x5030:
        name = "ThumbnailResolutionUnit";
        break;
      case 0x5031:
        name = "ThumbnailTransferFunction";
        break;
      case 0x5032:
        name = "ThumbnailSoftwareUsed";
        break;
      case 0x5033:
        name = "ThumbnailDateTime";
        break;
      case 0x5034:
        name = "ThumbnailArtist";
        break;
      case 0x5035:
        name = "ThumbnailWhitePoint";
        break;
      case 0x5036:
        name = "ThumbnailPrimaryChromaticities";
        break;
      case 0x5037:
        name = "ThumbnailYCbCrCoefficients";
        break;
      case 0x5038:
        name = "ThumbnailYCbCrSubsampling";
        break;
      case 0x5039:
        name = "ThumbnailYCbCrPositioning";
        break;
      case 0x503A:
        name = "ThumbnailRefBlackWhite";
        break;
      case 0x503B:
        name = "ThumbnailCopyRight";
        break;
      case 0x5090:
        name = "LuminanceTable";
        break;
      case 0x5091:
        name = "ChrominanceTable";
        break;
      case 0x5100:
        name = "FrameDelay";
        break;
      case 0x5101:
        name = "LoopCount";
        break;
      case 0x5110:
        name = "PixelUnit";
        break;
      case 0x5111:
        name = "PixelPerUnitX";
        break;
      case 0x5112:
        name = "PixelPerUnitY";
        break;
      case 0x5113:
        name = "PaletteHistogram";
        break;
      case 0x1000:
        name = "RelatedImageFileFormat";
        break;
      case 0x800D:
        name = "ImageID";
        break;
      case 0x80E3:
        name = "Matteing";
        break;
      case 0x80E4:
        name = "DataType";
        break;
      case 0x80E5:
        name = "ImageDepth";
        break;
      case 0x80E6:
        name = "TileDepth";
        break;
      case 0x828D:
        name = "CFARepeatPatternDim";
        break;
      case 0x828E:
        name = "CFAPattern";
        break;
      case 0x828F:
        name = "BatteryLevel";
        break;
      case 0x8298:
        name = "Copyright";
        break;
      case 0x829A:
        name = "ExposureTime";
        break;
      case 0x829D:
        name = "FNumber";
        break;
      case 0x83BB:
        name = "IPTC/NAA";
        break;
      case 0x84E3:
        name = "IT8RasterPadding";
        break;
      case 0x84E5:
        name = "IT8ColorTable";
        break;
      case 0x8649:
        name = "ImageResourceInformation";
        break;
      case 0x8769:
        name = "Exif IFD Pointer";
        break;
      case 0x8773:
        name = "ICC_Profile";
        break;
      case 0x8822:
        name = "ExposureProgram";
        break;
      case 0x8824:
        name = "SpectralSensitivity";
        break;
      case 0x8825:
        name = "GPSInfo IFD Pointer";
        break;
      case 0x8827:
        name = "ISOSpeedRatings";
        break;
      case 0x8828:
        name = "OECF";
        break;
      case 0x9000:
        name = "ExifVersion";
        break;
      case 0x9003:
        name = "DateTimeOriginal";
        break;
      case 0x9004:
        name = "DateTimeDigitized";
        break;
      case 0x9101:
        name = "ComponentsConfiguration";
        break;
      case 0x9102:
        name = "CompressedBitsPerPixel";
        break;
      case 0x9201:
        name = "ShutterSpeedValue";
        break;
      case 0x9202:
        name = "ApertureValue";
        break;
      case 0x9203:
        name = "BrightnessValue";
        break;
      case 0x9204:
        name = "ExposureBiasValue";
        break;
      case 0x9205:
        name = "MaxApertureValue";
        break;
      case 0x9206:
        name = "SubjectDistance";
        break;
      case 0x9207:
        name = "MeteringMode";
        break;
      case 0x9208:
        name = "LightSource";
        break;
      case 0x9209:
        name = "Flash";
        break;
      case 0x920A:
        name = "FocalLength";
        break;
      case 0x920B:
      case 0xA20B:
        name = "FlashEnergy";
        break;
      case 0x920C:
      case 0xA20C:
        name = "SpatialFrequencyResponse";
        break;
      case 0x920D:
        name = "Noise";
        break;
      case 0x920E:
      case 0xA20E:
        name = "FocalPlaneXResolution";
        break;
      case 0x920F:
      case 0XA20F:
        name = "FocalPlaneYResolution";
        break;
      case 0x9210:
      case 0xA210:
        name = "FocalPlaneResolutionUnit";
        break;
      case 0x9211:
        name = "ImageNumber";
        break;
      case 0x9212:
        name = "SecurityClassification";
        break;
      case 0x9213:
        name = "ImageHistory";
        break;
      case 0x9214:
      case 0xA214:
        name = "SubjectLocation";
        break;
      case 0x9215:
      case 0xA215:
        name = "ExposureIndex";
        break;
      case 0x9216:
        name = "TIFF/EPStandardID";
        break;
      case 0x9217:
      case 0xA217:
        name = "SensingMethod";
        break;
      case 0x923F:
        name = "StoNits";
        break;
      case 0x927C:
        name = "MakerNote";
        break;
      case 0x9286:
        name = "UserComment";
        break;
      case 0x9290:
        name = "SubSecTime";
        break;
      case 0x9291:
        name = "SubSecTimeOriginal";
        break;
      case 0x9292:
        name = "SubSecTimeDigitized";
        break;
      case 0xA000:
        name = "FlashpixVersion";
        break;
      case 0xA001:
        name = "ColorSpace";
        break;
      case 0xA002:
        name = "ExifImageWidth";
        break;
      case 0xA003:
        name = "ExifImageLength";
        break;
      case 0xA004:
        name = "RelatedSoundFile";
        break;
      case 0xA005:
        name = "Interoperability IFD Pointer";
        break;
      case 0xA20D:
        name = "Noise";
        break;
      case 0xA211:
        name = "ImageNumber";
        break;
      case 0xA212:
        name = "SecurityClassification";
        break;
      case 0xA213:
        name = "ImageHistory";
        break;
      case 0xA216:
        name = "TIFF/EPStandardID";
        break;
      case 0xA300:
        name = "FileSource";
        break;
      case 0xA301:
        name = "SceneType";
        break;
      case 0xA302:
        name = "CFAPattern";
        break;
      case 0xA401:
        name = "CustomRendered";
        break;
      case 0xA402:
        name = "ExposureMode";
        break;
      case 0xA403:
        name = "WhiteBalance";
        break;
      case 0xA404:
        name = "DigitalZoomRatio";
        break;
      case 0xA405:
        name = "FocalLengthIn35mmFilm";
        break;
      case 0xA406:
        name = "SceneCaptureType";
        break;
      case 0xA407:
        name = "GainControl";
        break;
      case 0xA408:
        name = "Contrast";
        break;
      case 0xA409:
        name = "Saturation";
        break;
      case 0xA40A:
        name = "Sharpness";
        break;
      case 0xA40B:
        name = "DeviceSettingDescription";
        break;
      case 0xA40C:
        name = "SubjectDistanceRange";
        break;
      case 0xA420:
        name = "ImageUniqueID";
        break;
    };
  }
  l = strlen( name );
  if ( len > l )
    strcpy( str, name );
  else
  {
    memset( str, '\0', len );
    memcpy( str, name, len - 1 );
  }
}

static unsigned short
exifImportU16( const unsigned char *p, int little_endian,
               int little_endian_arch )
{
  /* fetches an unsigned 16bit int from BLOB respecting declared endiannes */
  union cvt
  {
    unsigned char byte[2];
    unsigned short short_value;
  } convert;
  if ( little_endian_arch )
  {
    /* Litte-Endian architecture [e.g. x86] */
    if ( !little_endian )
    {
      /* Big Endian data */
      convert.byte[0] = *( p + 1 );
      convert.byte[1] = *( p + 0 );
    }
    else
    {
      /* Little Endian data */
      convert.byte[0] = *( p + 0 );
      convert.byte[1] = *( p + 1 );
    }
  }
  else
  {
    /* Big Endian architecture [e.g. PPC] */
    if ( !little_endian )
    {
      /* Big Endian data */
      convert.byte[0] = *( p + 0 );
      convert.byte[1] = *( p + 1 );
    }
    else
    {
      /* Little Endian data */
      convert.byte[0] = *( p + 1 );
      convert.byte[1] = *( p + 0 );
    }
  }
  return convert.short_value;
}

static unsigned int
exifImportU32( const unsigned char *p, int little_endian,
               int little_endian_arch )
{
  /* fetches an unsigned 32bit int from BLOB respecting declared endiannes */
  union cvt
  {
    unsigned char byte[4];
    unsigned int int_value;
  } convert;
  if ( little_endian_arch )
  {
    /* Litte-Endian architecture [e.g. x86] */
    if ( !little_endian )
    {
      /* Big Endian data */
      convert.byte[0] = *( p + 3 );
      convert.byte[1] = *( p + 2 );
      convert.byte[2] = *( p + 1 );
      convert.byte[3] = *( p + 0 );
    }
    else
    {
      /* Little Endian data */
      convert.byte[0] = *( p + 0 );
      convert.byte[1] = *( p + 1 );
      convert.byte[2] = *( p + 2 );
      convert.byte[3] = *( p + 3 );
    }
  }
  else
  {
    /* Big Endian architecture [e.g. PPC] */
    if ( !little_endian )
    {
      /* Big Endian data */
      convert.byte[0] = *( p + 0 );
      convert.byte[1] = *( p + 1 );
      convert.byte[2] = *( p + 2 );
      convert.byte[3] = *( p + 3 );
    }
    else
    {
      /* Little Endian data */
      convert.byte[0] = *( p + 3 );
      convert.byte[1] = *( p + 2 );
      convert.byte[2] = *( p + 1 );
      convert.byte[3] = *( p + 0 );
    }
  }
  return convert.int_value;
}

static float
exifImportFloat32( const unsigned char *p, int little_endian,
                   int little_endian_arch )
{
  /* fetches a 32bit FLOAT from BLOB respecting declared endiannes */
  union cvt
  {
    unsigned char byte[4];
    float float_value;
  } convert;
  if ( little_endian_arch )
  {
    /* Litte-Endian architecture [e.g. x86] */
    if ( !little_endian )
    {
      /* Big Endian data */
      convert.byte[0] = *( p + 3 );
      convert.byte[1] = *( p + 2 );
      convert.byte[2] = *( p + 1 );
      convert.byte[3] = *( p + 0 );
    }
    else
    {
      /* Little Endian data */
      convert.byte[0] = *( p + 0 );
      convert.byte[1] = *( p + 1 );
      convert.byte[2] = *( p + 2 );
      convert.byte[3] = *( p + 3 );
    }
  }
  else
  {
    /* Big Endian architecture [e.g. PPC] */
    if ( !little_endian )
    {
      /* Big Endian data */
      convert.byte[0] = *( p + 0 );
      convert.byte[1] = *( p + 1 );
      convert.byte[2] = *( p + 2 );
      convert.byte[3] = *( p + 3 );
    }
    else
    {
      /* Little Endian data */
      convert.byte[0] = *( p + 3 );
      convert.byte[1] = *( p + 2 );
      convert.byte[2] = *( p + 1 );
      convert.byte[3] = *( p + 0 );
    }
  }
  return convert.float_value;
}

static void
exifSetTagValue( gaiaExifTagPtr tag, const unsigned char *blob, int endian_mode,
                 int endian_arch )
{
  /* setting the TAG value */
  int i;
  int sz = 0;
  unsigned int offset;
  const unsigned char *ptr;
  unsigned short short_value;
  unsigned int int_value;
  short sign_short_value;
  int sign_int_value;
  float float_value;
  double double_value;
  if ( tag->Type == 1 || tag->Type == 2 || tag->Type == 6 || tag->Type == 7 )
    sz = tag->Count;
  if ( tag->Type == 3 || tag->Type == 8 )
    sz = tag->Count * 2;
  if ( tag->Type == 4 || tag->Type == 9 || tag->Type == 11 )
    sz = tag->Count * 4;
  if ( tag->Type == 5 || tag->Type == 10 || tag->Type == 12 )
    sz = tag->Count * 8;
  if ( sz <= 4 )
  {
    /* TAG values is stored within the offset */
    ptr = tag->TagOffset;
  }
  else
  {
    /* jumping to offset */
    offset = exifImportU32( tag->TagOffset, endian_mode, endian_arch );
    offset += 12;
    ptr = blob + offset;
  }
  if ( tag->Type == 1 || tag->Type == 6 || tag->Type == 7 )
  {
    /* BYTE type */
    tag->ByteValue = malloc( tag->Count );
    memcpy( tag->ByteValue, ptr, tag->Count );
  }
  if ( tag->Type == 2 )
  {
    /* STRING type */
    tag->StringValue = malloc( tag->Count );
    memcpy( tag->StringValue, ptr, tag->Count );
  }
  if ( tag->Type == 3 )
  {
    /* SHORT type */
    tag->ShortValues = malloc( tag->Count * sizeof( unsigned short ) );
    for ( i = 0; i < tag->Count; i++ )
    {
      short_value =
        exifImportU16( ptr + ( i * 2 ), endian_mode, endian_arch );
      *( tag->ShortValues + i ) = short_value;
    }
  }
  if ( tag->Type == 4 )
  {
    /* LONG type */
    tag->LongValues = malloc( tag->Count * sizeof( unsigned int ) );
    for ( i = 0; i < tag->Count; i++ )
    {
      int_value =
        exifImportU32( ptr + ( i * 4 ), endian_mode, endian_arch );
      *( tag->LongValues + i ) = int_value;
    }
  }
  if ( tag->Type == 5 )
  {
    /* RATIONAL type */
    tag->LongRationals1 = malloc( tag->Count * sizeof( unsigned int ) );
    tag->LongRationals2 = malloc( tag->Count * sizeof( unsigned int ) );
    for ( i = 0; i < tag->Count; i++ )
    {
      int_value =
        exifImportU32( ptr + ( i * 8 ), endian_mode, endian_arch );
      *( tag->LongRationals1 + i ) = int_value;
      int_value =
        exifImportU32( ptr + ( i * 8 ) + 4, endian_mode, endian_arch );
      *( tag->LongRationals2 + i ) = int_value;
    }
  }
  if ( tag->Type == 8 )
  {
    /* SSHORT type */
    tag->SignedShortValues = malloc( tag->Count * sizeof( short ) );
    for ( i = 0; i < tag->Count; i++ )
    {
      sign_short_value =
        gaiaImport16( ptr + ( i * 2 ), endian_mode, endian_arch );
      *( tag->SignedShortValues + i ) = sign_short_value;
    }
  }
  if ( tag->Type == 9 )
  {
    /* SIGNED LONG type */
    tag->SignedLongValues = malloc( tag->Count * sizeof( int ) );
    for ( i = 0; i < tag->Count; i++ )
    {
      sign_int_value =
        gaiaImport32( ptr + ( i * 4 ), endian_mode, endian_arch );
      *( tag->SignedLongValues + i ) = sign_int_value;
    }
  }
  if ( tag->Type == 10 )
  {
    /* SIGNED RATIONAL type */
    tag->SignedLongRationals1 = malloc( tag->Count * sizeof( int ) );
    tag->SignedLongRationals2 = malloc( tag->Count * sizeof( int ) );
    for ( i = 0; i < tag->Count; i++ )
    {
      sign_int_value =
        gaiaImport32( ptr + ( i * 8 ), endian_mode, endian_arch );
      *( tag->SignedLongRationals1 + i ) = sign_int_value;
      sign_int_value =
        gaiaImport32( ptr + ( i * 8 ) + 4, endian_mode, endian_arch );
      *( tag->SignedLongRationals2 + i ) = sign_int_value;
    }
  }
  if ( tag->Type == 11 )
  {
    /* FLOAT type */
    tag->FloatValues = malloc( tag->Count * sizeof( float ) );
    for ( i = 0; i < tag->Count; i++ )
    {
      float_value =
        exifImportFloat32( ptr + ( i * 4 ), endian_mode, endian_arch );
      *( tag->FloatValues + i ) = float_value;
    }
  }
  if ( tag->Type == 12 )
  {
    /* DOUBLE type */
    tag->DoubleValues = malloc( tag->Count * sizeof( double ) );
    for ( i = 0; i < tag->Count; i++ )
    {
      double_value =
        gaiaImport64( ptr + ( i * 8 ), endian_mode, endian_arch );
      *( tag->DoubleValues + i ) = double_value;
    }
  }
}

static void
exifParseTag( const unsigned char *blob, unsigned int offset, int endian_mode,
              int endian_arch, gaiaExifTagListPtr list, int gps )
{
  /* parsing some TAG and inserting into the list */
  unsigned short tag_id;
  unsigned short type;
  unsigned int count;
  gaiaExifTagPtr tag;
  tag_id = exifImportU16( blob + offset, endian_mode, endian_arch );
  type = exifImportU16( blob + offset + 2, endian_mode, endian_arch );
  count = exifImportU32( blob + offset + 4, endian_mode, endian_arch );
  tag = malloc( sizeof( gaiaExifTag ) );
  tag->Gps = gps;
  tag->TagId = tag_id;
  tag->Type = type;
  tag->Count = count;
  memcpy( tag->TagOffset, blob + offset + 8, 4 );
  tag->ByteValue = NULL;
  tag->StringValue = NULL;
  tag->ShortValues = NULL;
  tag->LongValues = NULL;
  tag->LongRationals1 = NULL;
  tag->LongRationals2 = NULL;
  tag->SignedShortValues = NULL;
  tag->SignedLongValues = NULL;
  tag->SignedLongRationals1 = NULL;
  tag->SignedLongRationals2 = NULL;
  tag->FloatValues = NULL;
  tag->DoubleValues = NULL;
  exifSetTagValue( tag, blob, endian_mode, endian_arch );
  tag->Next = NULL;
  if ( !( list->First ) )
    list->First = tag;
  if ( list->Last )
    ( list->Last )->Next = tag;
  list->Last = tag;
  ( list->NumTags )++;
}

static void
exifExpandIFD( gaiaExifTagListPtr list, const unsigned char *blob,
               int endian_mode, int endian_arch )
{
  /* trying to expand the EXIF-IFD */
  unsigned int offset;
  unsigned short items;
  unsigned short i;
  gaiaExifTagPtr tag;
  if ( !list )
    return;
  tag = list->First;
  while ( tag )
  {
    if ( tag->TagId == 34665 )
    {
      /* ok, this one is an IFD pointer */
      offset =
        exifImportU32( tag->TagOffset, endian_mode, endian_arch );
      offset += 12;
      items = exifImportU16( blob + offset, endian_mode, endian_arch );
      offset += 2;
      for ( i = 0; i < items; i++ )
      {
        /* fetching the TAGs */
        exifParseTag( blob, offset, endian_mode, endian_arch,
                      list, 0 );
        offset += 12;
      }
    }
    tag = tag->Next;
  }
}

static void
exifExpandGPS( gaiaExifTagListPtr list, const unsigned char *blob,
               int endian_mode, int endian_arch )
{
  /* trying to expand the EXIF-GPS */
  unsigned int offset;
  unsigned short items;
  unsigned short i;
  gaiaExifTagPtr tag;
  if ( !list )
    return;
  tag = list->First;
  while ( tag )
  {
    if ( tag->TagId == 34853 )
    {
      /* ok, this one is a GPSinfo-IFD pointer */
      offset =
        exifImportU32( tag->TagOffset, endian_mode, endian_arch );
      offset += 12;
      items = exifImportU16( blob + offset, endian_mode, endian_arch );
      offset += 2;
      for ( i = 0; i < items; i++ )
      {
        /* fetching the TAGs */
        exifParseTag( blob, offset, endian_mode, endian_arch,
                      list, 1 );
        offset += 12;
      }
    }
    tag = tag->Next;
  }
}

GAIAEXIF_DECLARE gaiaExifTagListPtr
gaiaGetExifTags( const unsigned char *blob, int size )
{
  /* trying to parse a BLOB as an EXIF photo */
  gaiaExifTagListPtr list;
  int endian_arch = gaiaEndianArch();
  int endian_mode;
  unsigned short app1_size;
  unsigned int offset;
  unsigned short items;
  unsigned short i;
  if ( !blob )
    goto error;
  if ( size < 14 )
    goto error;
  /* cecking for SOI [Start Of Image] */
  if ( *( blob + 0 ) == 0xff && *( blob + 1 ) == 0xd8 )
    ;
  else
    goto error;
  /* checking for APP1 Marker */
  if ( *( blob + 2 ) == 0xff && *( blob + 3 ) == 0xe1 )
    ;
  else
    goto error;
  /* checking for EXIF identifier */
  if ( memcmp( blob + 6, "Exif", 4 ) == 0 )
    ;
  else
    goto error;
  /* checking for Pad */
  if ( *( blob + 10 ) == 0x00 && *( blob + 11 ) == 0x00 )
    ;
  else
    goto error;
  if ( memcmp( blob + 12, "II", 2 ) == 0 )
    endian_mode = GAIA_LITTLE_ENDIAN;
  else if ( memcmp( blob + 12, "MM", 2 ) == 0 )
    endian_mode = GAIA_BIG_ENDIAN;
  else
    goto error;
  /* OK: this BLOB seems to contain a valid EXIF */
  app1_size = exifImportU16( blob + 4, endian_mode, endian_arch );
  if (( app1_size + 6 ) > size )
    goto error;
  /* checking for marker */
  if ( endian_mode == GAIA_BIG_ENDIAN )
  {
    if ( *( blob + 14 ) == 0x00 && *( blob + 15 ) == 0x2a )
      ;
    else
      goto error;
  }
  else
  {
    if ( *( blob + 14 ) == 0x2a && *( blob + 15 ) == 0x00 )
      ;
    else
      goto error;
  }
  /* allocating an EXIF TAG LIST */
  list = malloc( sizeof( gaiaExifTagList ) );
  list->First = NULL;
  list->Last = NULL;
  list->NumTags = 0;
  list->TagsArray = NULL;
  offset = exifImportU32( blob + 16, endian_mode, endian_arch );
  offset += 12;
  /* jump to offset */
  items = exifImportU16( blob + offset, endian_mode, endian_arch );
  offset += 2;
  for ( i = 0; i < items; i++ )
  {
    /* fetching the EXIF TAGs */
    exifParseTag( blob, offset, endian_mode, endian_arch, list, 0 );
    offset += 12;
  }
  /* expanding the IFD and GPS tags */
  exifExpandIFD( list, blob, endian_mode, endian_arch );
  exifExpandGPS( list, blob, endian_mode, endian_arch );
  if ( list->NumTags )
  {
    gaiaExifTagPtr pT;

    /* organizing the EXIF TAGS as an Array */
    list->TagsArray = malloc( sizeof( gaiaExifTagPtr ) * list->NumTags );
    pT = list->First;
    i = 0;
    while ( pT )
    {
      *( list->TagsArray + i++ ) = pT;
      pT = pT->Next;
    }
  }
  return list;
error:
  return NULL;
}

GAIAEXIF_DECLARE void
gaiaExifTagsFree( gaiaExifTagListPtr p )
{
  /* memory cleanup; freeing the EXIF TAG list */
  gaiaExifTagPtr pT;
  gaiaExifTagPtr pTn;
  if ( !p )
    return;
  pT = p->First;
  while ( pT )
  {
    pTn = pT->Next;
    if ( pT->ByteValue )
      free( pT->ByteValue );
    if ( pT->StringValue )
      free( pT->StringValue );
    if ( pT->ShortValues )
      free( pT->ShortValues );
    if ( pT->LongValues )
      free( pT->LongValues );
    if ( pT->LongRationals1 )
      free( pT->LongRationals1 );
    if ( pT->LongRationals2 )
      free( pT->LongRationals2 );
    if ( pT->SignedShortValues )
      free( pT->SignedShortValues );
    if ( pT->SignedLongValues )
      free( pT->SignedLongValues );
    if ( pT->SignedLongRationals1 )
      free( pT->SignedLongRationals1 );
    if ( pT->SignedLongRationals2 )
      free( pT->SignedLongRationals2 );
    if ( pT->FloatValues )
      free( pT->FloatValues );
    if ( pT->DoubleValues )
      free( pT->DoubleValues );
    free( pT );
    pT = pTn;
  }
  if ( p->TagsArray )
    free( p->TagsArray );
  free( p );
}

GAIAEXIF_DECLARE int
gaiaGetExifTagsCount( gaiaExifTagListPtr tag_list )
{
  /* returns the # TAGSs into this list */
  return tag_list->NumTags;
}

GAIAEXIF_DECLARE gaiaExifTagPtr
gaiaGetExifTagByPos( gaiaExifTagListPtr tag_list, const int pos )
{
  /* returns the Nth TAG from this list */
  if ( pos >= 0 && pos < tag_list->NumTags )
    return *( tag_list->TagsArray + pos );
  return NULL;
}

GAIAEXIF_DECLARE gaiaExifTagPtr
gaiaGetExifTagById( const gaiaExifTagListPtr tag_list,
                    const unsigned short tag_id )
{
  /* returns a not-GPS TAG identified by its ID */
  gaiaExifTagPtr pT = tag_list->First;
  while ( pT )
  {
    if ( !( pT->Gps ) && pT->TagId == tag_id )
      return pT;
    pT = pT->Next;
  }
  return NULL;
}

GAIAEXIF_DECLARE gaiaExifTagPtr
gaiaGetExifGpsTagById( const gaiaExifTagListPtr tag_list,
                       const unsigned short tag_id )
{
  /* returns a GPS TAG identified by its ID */
  gaiaExifTagPtr pT = tag_list->First;
  while ( pT )
  {
    if ( pT->Gps && pT->TagId == tag_id )
      return pT;
    pT = pT->Next;
  }
  return NULL;
}

GAIAEXIF_DECLARE gaiaExifTagPtr
gaiaGetExifTagByName( const gaiaExifTagListPtr tag_list, const char *tag_name )
{
  /* returns a TAG identified by its Name */
  char name[128];
  gaiaExifTagPtr pT = tag_list->First;
  while ( pT )
  {
    exifTagName( pT->Gps, pT->TagId, name, 128 );
    if ( strcasecmp( name, tag_name ) == 0 )
      return pT;
    pT = pT->Next;
  }
  return NULL;
}

GAIAEXIF_DECLARE unsigned short
gaiaExifTagGetId( const gaiaExifTagPtr tag )
{
  /* returns the TAG ID */
  return tag->TagId;
}

GAIAEXIF_DECLARE int
gaiaIsExifGpsTag( const gaiaExifTagPtr tag )
{
  /* checks if this one is a GPS tag */
  return tag->Gps;
}

GAIAEXIF_DECLARE void
gaiaExifTagGetName( const gaiaExifTagPtr tag, char *str, int len )
{
  /* returns the TAG symbolic Name */
  exifTagName( tag->Gps, tag->TagId, str, len );
}

GAIAEXIF_DECLARE unsigned short
gaiaExifTagGetValueType( const gaiaExifTagPtr tag )
{
  /* returns the TAG value Type */
  return tag->Type;
}

GAIAEXIF_DECLARE unsigned short
gaiaExifTagGetNumValues( const gaiaExifTagPtr tag )
{
  /* returns the # TAG Values */
  return tag->Count;
}

GAIAEXIF_DECLARE unsigned char
gaiaExifTagGetByteValue( const gaiaExifTagPtr tag, const int ind, int *ok )
{
  /* returns the Nth Byte value */
  if ( ind >= 0 && ind < tag->Count
       && ( tag->Type == 1 || tag->Type == 6 || tag->Type == 7 ) )
  {
    *ok = 1;
    return *( tag->ByteValue + ind );
  }
  *ok = 0;
  return 0;
}

GAIAEXIF_DECLARE void
gaiaExifTagGetStringValue( const gaiaExifTagPtr tag, char *str, int len,
                           int *ok )
{
  /* returns the String value */
  int l;
  if ( tag->Type == 2 )
  {
    *ok = 1;
    l = strlen( tag->StringValue );
    if ( len > l )
      strcpy( str, tag->StringValue );
    else
    {
      memset( str, '\0', len );
      memcpy( str, tag->StringValue, len - 1 );
    }
    return;
  }
  *ok = 0;
}

GAIAEXIF_DECLARE unsigned short
gaiaExifTagGetShortValue( const gaiaExifTagPtr tag, const int ind, int *ok )
{
  /* returns the Nth Short value */
  if ( ind >= 0 && ind < tag->Count && tag->Type == 3 )
  {
    *ok = 1;
    return *( tag->ShortValues + ind );
  }
  *ok = 0;
  return 0;
}

GAIAEXIF_DECLARE unsigned int
gaiaExifTagGetLongValue( const gaiaExifTagPtr tag, const int ind, int *ok )
{
  /* returns the Nth Long value */
  if ( ind >= 0 && ind < tag->Count && tag->Type == 4 )
  {
    *ok = 1;
    return *( tag->LongValues + ind );
  }
  *ok = 0;
  return 0;
}

GAIAEXIF_DECLARE unsigned int
gaiaExifTagGetRational1Value( const gaiaExifTagPtr tag, const int ind, int *ok )
{
  /* returns the Nth Rational (1) value */
  if ( ind >= 0 && ind < tag->Count && tag->Type == 5 )
  {
    *ok = 1;
    return *( tag->LongRationals1 + ind );
  }
  *ok = 0;
  return 0;
}

GAIAEXIF_DECLARE unsigned int
gaiaExifTagGetRational2Value( const gaiaExifTagPtr tag, const int ind, int *ok )
{
  /* returns the Nth Rational (2) value */
  if ( ind >= 0 && ind < tag->Count && tag->Type == 5 )
  {
    *ok = 1;
    return *( tag->LongRationals2 + ind );
  }
  *ok = 0;
  return 0;
}

GAIAEXIF_DECLARE double
gaiaExifTagGetRationalValue( const gaiaExifTagPtr tag, const int ind, int *ok )
{
  /* returns the Nth Rational  value as Double */
  double x;
  if ( ind >= 0 && ind < tag->Count && tag->Type == 5 &&
       *( tag->LongRationals2 + ind ) )
  {
    *ok = 1;
    x = ( double )( *( tag->LongRationals1 + ind ) ) /
        ( double )( *( tag->LongRationals2 + ind ) );
    return x;
  }
  *ok = 0;
  return 0;
}

GAIAEXIF_DECLARE short
gaiaExifTagGetSignedShortValue( const gaiaExifTagPtr tag, const int ind,
                                int *ok )
{
  /* returns the Nth Signed Short value */
  if ( ind >= 0 && ind < tag->Count && tag->Type == 8 )
  {
    *ok = 1;
    return *( tag->SignedShortValues + ind );
  }
  *ok = 0;
  return 0;
}

GAIAEXIF_DECLARE int
gaiaExifTagGetSignedLongValue( const gaiaExifTagPtr tag, const int ind, int *ok )
{
  /* returns the Nth Signed Long value */
  if ( ind >= 0 && ind < tag->Count && tag->Type == 9 )
  {
    *ok = 1;
    return *( tag->SignedLongValues + ind );
  }
  *ok = 0;
  return 0;
}

GAIAEXIF_DECLARE int
gaiaExifTagGetSignedRational1Value( const gaiaExifTagPtr tag, const int ind,
                                    int *ok )
{
  /* returns the Nth Signed Rational (1) value */
  if ( ind >= 0 && ind < tag->Count && tag->Type == 10 )
  {
    *ok = 1;
    return *( tag->SignedLongRationals1 + ind );
  }
  *ok = 0;
  return 0;
}

GAIAEXIF_DECLARE int
gaiaExifTagGetSignedRational2Value( const gaiaExifTagPtr tag, const int ind,
                                    int *ok )
{
  /* returns the Nth Signed Rational (2) value */
  if ( ind >= 0 && ind < tag->Count && tag->Type == 10 )
  {
    *ok = 1;
    return *( tag->SignedLongRationals2 + ind );
  }
  *ok = 0;
  return 0;
}

GAIAEXIF_DECLARE double
gaiaExifTagGetSignedRationalValue( const gaiaExifTagPtr tag, const int ind,
                                   int *ok )
{
  /* returns the Nth Signed Rational  value as Double */
  double x;
  if ( ind >= 0 && ind < tag->Count && tag->Type == 10 &&
       *( tag->SignedLongRationals2 + ind ) )
  {
    *ok = 1;
    x = ( double )( *( tag->SignedLongRationals1 + ind ) ) /
        ( double )( *( tag->SignedLongRationals2 + ind ) );
    return x;
  }
  *ok = 0;
  return 0;
}

GAIAEXIF_DECLARE float
gaiaExifTagGetFloatValue( const gaiaExifTagPtr tag, const int ind, int *ok )
{
  /* returns the Nth Float value */
  if ( ind >= 0 && ind < tag->Count && tag->Type == 11 )
  {
    *ok = 1;
    return *( tag->FloatValues + ind );
  }
  *ok = 0;
  return 0;
}

GAIAEXIF_DECLARE double
gaiaExifTagGetDoubleValue( const gaiaExifTagPtr tag, const int ind, int *ok )
{
  /* returns the Nth Double value */
  if ( ind >= 0 && ind < tag->Count && tag->Type == 12 )
  {
    *ok = 1;
    return *( tag->DoubleValues + ind );
  }
  *ok = 0;
  return 0;
}

GAIAEXIF_DECLARE void
gaiaExifTagGetHumanReadable( const gaiaExifTagPtr tag, char *str, int len,
                             int *ok )
{
  /* returns the Human Readable value */
  char *human = "";
  char dummy[1024];
  int l;
  int xok;
  double dblval;
  switch ( tag->TagId )
  {
    case 0x0128:  /* ResolutionUnit */
      if ( tag->Type == 3 && tag->Count == 1 )
      {
        switch ( *( tag->ShortValues + 0 ) )
        {
          case 2:
            human = "Inches";
            break;
          case 3:
            human = "Centimeters";
            break;
        };
      }
      break;
    case 0x8822:  /* ExposureProgram */
      if ( tag->Type == 3 && tag->Count == 1 )
      {
        switch ( *( tag->ShortValues + 0 ) )
        {
          case 0:
            human = "Not defined";
            break;
          case 1:
            human = "Manual";
            break;
          case 2:
            human = "Normal program";
            break;
          case 3:
            human = "Aperture priority";
            break;
          case 4:
            human = "Shutter priority";
            break;
          case 5:
            human = "Creative program (biased toward depth of field)";
            break;
          case 6:
            human =
              "Action program (biased toward fast shutter speed)";
            break;
          case 7:
            human =
              "Portrait mode (for closeup photos with the background out of focus)";
            break;
          case 8:
            human =
              "Landscape mode (for landscape photos with the background in focus)";
            break;
        };
      }
      break;
    case 0xA402:  /* ExposureMode */
      if ( tag->Type == 3 && tag->Count == 1 )
      {
        switch ( *( tag->ShortValues + 0 ) )
        {
          case 0:
            human = "Auto exposure";
            break;
          case 1:
            human = "Manual exposure";
            break;
          case 2:
            human = "Auto bracket";
            break;
        };
      }
      break;
    case 0x0112:  /* Orientation */
      if ( tag->Type == 3 && tag->Count == 1 )
      {
        switch ( *( tag->ShortValues + 0 ) )
        {
          case 1:
            human = "Normal";
            break;
          case 2:
            human = "Mirrored";
            break;
          case 3:
            human = "Upsidedown";
            break;
          case 4:
            human = "Upsidedown Mirrored";
            break;
          case 5:
            human = "90 deg Clockwise Mirrored";
            break;
          case 6:
            human = "90 deg Counterclocwise";
            break;
          case 7:
            human = "90 deg Counterclocwise Mirrored";
            break;
          case 8:
            human = "90 deg Mirrored";
            break;
        };
      }
      break;
    case 0x9207:  /* MeteringMode */
      if ( tag->Type == 3 && tag->Count == 1 )
      {
        switch ( *( tag->ShortValues + 0 ) )
        {
          case 1:
            human = "Average";
            break;
          case 2:
            human = "Center Weighted Average";
            break;
          case 3:
            human = "Spot";
            break;
          case 4:
            human = "MultiSpot";
            break;
          case 5:
            human = "MultiSegment";
            break;
          case 6:
            human = "Partial";
            break;
          case 255:
            human = "Other";
            break;
        };
      }
      break;
    case 0xA403:  /* WhiteBalance */
      if ( tag->Type == 3 && tag->Count == 1 )
      {
        switch ( *( tag->ShortValues + 0 ) )
        {
          case 0:
            human = "Auto";
            break;
          case 1:
            human = "Sunny";
            break;
          case 2:
            human = "Cloudy";
            break;
          case 3:
            human = "Tungsten";
            break;
          case 4:
            human = "Fluorescent";
            break;
          case 5:
            human = "Flash";
            break;
          case 6:
            human = "Custom";
            break;
          case 129:
            human = "Manual";
            break;
        };
      }
      break;
    case 0x9209:  /* Flash */
      if ( tag->Type == 3 && tag->Count == 1 )
      {
        switch ( *( tag->ShortValues + 0 ) )
        {
          case 0:
          case 16:
          case 24:
          case 32:
            human = "No Flash";
            break;
          case 1:
            human = "Flash";
            break;
          case 5:
            human = "Flash, strobe return light not detected";
            break;
          case 7:
            human = "Flash, strobe return light detected";
            break;
          case 9:
            human = "Compulsory Flash";
            break;
          case 13:
            human = "Compulsory Flash, Return light not detected";
            break;
          case 15:
            human = "Compulsory Flash, Return light detected";
            break;
          case 25:
            human = "Flash, Auto-Mode";
            break;
          case 29:
            human = "Flash, Auto-Mode, Return light not detected";
            break;
          case 31:
            human = "Flash, Auto-Mode, Return light detected";
            break;
          case 65:
            human = "Red Eye";
            break;
          case 69:
            human = "Red Eye, Return light not detected";
            break;
          case 71:
            human = "Red Eye, Return light detected";
            break;
          case 73:
            human = "Red Eye, Compulsory Flash";
            break;
          case 77:
            human =
              "Red Eye, Compulsory Flash, Return light not detected";
            break;
          case 79:
            human =
              "Red Eye, Compulsory Flash, Return light detected";
            break;
          case 89:
            human = "Red Eye, Auto-Mode";
            break;
          case 93:
            human = "Red Eye, Auto-Mode, Return light not detected";
            break;
          case 95:
            human = "Red Eye, Auto-Mode, Return light detected";
            break;
        };
      }
      break;
    case 0xA217:  /* SensingMethod */
      if ( tag->Type == 3 && tag->Count == 1 )
      {
        switch ( *( tag->ShortValues + 0 ) )
        {
          case 1:
            human = "Not defined";
            break;
          case 2:
            human = "One Chip Color Area Sensor";
            break;
          case 3:
            human = "Two Chip Color Area Sensor";
            break;
          case 4:
            human = "Three Chip Color Area Sensor";
            break;
          case 5:
            human = "Color Sequential Area Sensor";
            break;
          case 7:
            human = "Trilinear Sensor";
            break;
          case 8:
            human = "Color Sequential Linear Sensor";
            break;
        };
      }
      break;
    case 0xA406:  /* SceneCaptureType */
      if ( tag->Type == 3 && tag->Count == 1 )
      {
        switch ( *( tag->ShortValues + 0 ) )
        {
          case 0:
            human = "Standard";
            break;
          case 1:
            human = "Landscape";
            break;
          case 2:
            human = "Portrait";
            break;
          case 3:
            human = "Night scene";
            break;
        };
      }
      break;
    case 0xA407:  /* GainControl */
      if ( tag->Type == 3 && tag->Count == 1 )
      {
        switch ( *( tag->ShortValues + 0 ) )
        {
          case 0:
            human = "None";
            break;
          case 1:
            human = "Low gain up";
            break;
          case 2:
            human = "High gain up";
            break;
          case 3:
            human = "Low gain down";
            break;
          case 4:
            human = "High gain down";
            break;
        };
      }
      break;
    case 0xA408:  /* Contrast */
      if ( tag->Type == 3 && tag->Count == 1 )
      {
        switch ( *( tag->ShortValues + 0 ) )
        {
          case 0:
            human = "Normal";
            break;
          case 1:
            human = "Soft";
            break;
          case 2:
            human = "Hard";
            break;
        };
      }
      break;
    case 0xA409:  /* Saturation */
      if ( tag->Type == 3 && tag->Count == 1 )
      {
        switch ( *( tag->ShortValues + 0 ) )
        {
          case 0:
            human = "Normal";
            break;
          case 1:
            human = "Low saturation";
            break;
          case 2:
            human = "High saturation";
            break;
        };
      }
      break;
    case 0xA40A:  /* Sharpness */
      if ( tag->Type == 3 && tag->Count == 1 )
      {
        switch ( *( tag->ShortValues + 0 ) )
        {
          case 0:
            human = "Normal";
            break;
          case 1:
            human = "Soft";
            break;
          case 2:
            human = "Hard";
            break;
        };
      }
      break;
    case 0xA40C:  /* SubjectDistanceRange */
      if ( tag->Type == 3 && tag->Count == 1 )
      {
        switch ( *( tag->ShortValues + 0 ) )
        {
          case 0:
            human = "Unknown";
            break;
          case 1:
            human = "Macro";
            break;
          case 2:
            human = "Close view";
            break;
          case 3:
            human = "Distant view";
            break;
        };
      }
      break;
    case 0x9208:  /* LightSource */
      if ( tag->Type == 3 && tag->Count == 1 )
      {
        switch ( *( tag->ShortValues + 0 ) )
        {
          case 0:
            human = "Unknown";
            break;
          case 1:
            human = "Daylight";
            break;
          case 2:
            human = "Fluorescent";
            break;
          case 3:
            human = "Tungsten (incandescent light)";
            break;
          case 4:
            human = "Flash";
            break;
          case 9:
            human = "Fine weather";
            break;
          case 10:
            human = "Cloudy weather";
            break;
          case 11:
            human = "Shade";
          case 12:
            human = "Daylight fluorescent (D 5700 – 7100K)";
            break;
          case 13:
            human = "Day white fluorescent (N 4600 – 5400K)";
            break;
          case 14:
            human = "Cool white fluorescent (W 3900 – 4500K)";
          case 15:
            human = "White fluorescent (WW 3200 – 3700K)";
            break;
          case 17:
            human = "Standard light A";
            break;
          case 18:
            human = "Standard light B";
            break;
          case 19:
            human = "Standard light C";
            break;
          case 20:
            human = "D55";
            break;
          case 21:
            human = "D65";
            break;
          case 22:
            human = "D75";
            break;
          case 23:
            human = "D50";
            break;
          case 24:
            human = "ISO studio tungsten";
            break;
          case 255:
            human = "other light source";
            break;
        };
      }
      break;
    case 0xA001:  /* ColorSpace */
      if ( tag->Type == 3 && tag->Count == 1 )
      {
        switch ( *( tag->ShortValues + 0 ) )
        {
          case 1:
            human = "sRGB";
            break;
          case 0xffff:
            human = "Uncalibrated";
            break;
        };
      }
      break;
    case 0x8827:  /* ISOSpeedRatings */
      if ( tag->Type == 3 && tag->Count == 1 )
      {
        sprintf( dummy, "%u ISO", *( tag->ShortValues + 0 ) );
        human = dummy;
      }
      break;
    case 0xA002:  /* ExifImageWidth */
    case 0xA003:  /* ExifImageLength */
      if ( tag->Type == 3 && tag->Count == 1 )
      {
        sprintf( dummy, "%u pixels", *( tag->ShortValues + 0 ) );
        human = dummy;
      }
      else if ( tag->Type == 4 && tag->Count == 1 )
      {
        sprintf( dummy, "%u pixels", *( tag->LongValues + 0 ) );
        human = dummy;
      }
      break;
    case 0x829A:  /* ExposureTime */
      if ( tag->Type == 5 && tag->Count == 1 )
      {
        dblval = gaiaExifTagGetRationalValue( tag, 0, &xok );
        if ( xok )
        {
          if ( dblval < 1.0 )
          {
            dblval = 1.0 / dblval;
            sprintf( dummy, "1/%1.0lf sec", dblval );
            human = dummy;
          }
          else
          {
            sprintf( dummy, "%1.0lf sec", dblval );
            human = dummy;
          }
        }
      }
      break;
    case 0x9201:  /* ShutterSpeedValue */
      if ( tag->Type == 10 && tag->Count == 1 )
      {
        dblval = gaiaExifTagGetSignedRationalValue( tag, 0, &xok );
        if ( xok )
        {
          dblval = exp( dblval * log( 2 ) );
          if ( dblval > 1.0 )
            dblval = floor( dblval );
          if ( dblval < 1.0 )
          {
            dblval = math_round( 1.0 / dblval );
            sprintf( dummy, "%1.0lf sec", dblval );
            human = dummy;
          }
          else
          {
            sprintf( dummy, "1/%1.0lf sec", dblval );
            human = dummy;
          }
        }
      }
      break;
    case 0x829D:  /* FNumber */
      if ( tag->Type == 5 && tag->Count == 1 )
      {
        dblval = gaiaExifTagGetRationalValue( tag, 0, &xok );
        if ( xok )
        {
          sprintf( dummy, "F %1.1lf", dblval );
          human = dummy;
        }
      }
      break;
    case 0x9202:  /* ApertureValue */
    case 0x9205:  /* MaxApertureValue */
      if ( tag->Type == 5 && tag->Count == 1 )
      {
        dblval = gaiaExifTagGetRationalValue( tag, 0, &xok );
        if ( xok )
        {
          dblval = exp(( dblval * log( 2 ) ) / 2.0 );
          sprintf( dummy, "F %1.1lf", dblval );
          human = dummy;
        }
      }
      break;
    case 0x920A:  /* FocalLength */
      if ( tag->Type == 5 && tag->Count == 1 )
      {
        dblval = gaiaExifTagGetRationalValue( tag, 0, &xok );
        if ( xok )
        {
          sprintf( dummy, "%1.1lf mm", dblval );
          human = dummy;
        }
      }
      break;
    case 0xA405:  /* FocalLengthIn35mmFilm */
      if ( tag->Type == 3 && tag->Count == 1 )
      {
        sprintf( dummy, "%u mm", *( tag->ShortValues + 0 ) );
        human = dummy;
      }
      break;
    case 0x9204:  /* ExposureBiasValue */
      if ( tag->Type == 10 && tag->Count == 1 )
      {
        dblval = gaiaExifTagGetSignedRationalValue( tag, 0, &xok );
        if ( xok )
        {
          sprintf( dummy, "%1.2lf EV", dblval );
          human = dummy;
        }
      }
      break;
  };
  l = strlen( human );
  if ( l > 0 )
  {
    if ( len > l )
      strcpy( str, human );
    else
    {
      memset( str, '\0', len );
      memcpy( str, human, len - 1 );
    }
    *ok = 1;
    return;
  }
  *ok = 0;
}

GAIAEXIF_DECLARE int
gaiaGuessBlobType( const unsigned char *blob, int size )
{
  /* returns the BLOB content type */
  int jpeg = 0;
  int exif = 0;
  int exif_gps = 0;
  int geom = 1;
  gaiaExifTagListPtr exif_list;
  gaiaExifTagPtr pT;
  unsigned char jpeg1_signature[2];
  unsigned char jpeg2_signature[2];
  unsigned char jpeg3_signature[4];
  unsigned char jfif_signature[4];
  unsigned char exif_signature[4];
  unsigned char png_signature[8];
  unsigned char zip_signature[4];
  jpeg1_signature[0] = 0xff;
  jpeg1_signature[1] = 0xd8;
  jpeg2_signature[0] = 0xff;
  jpeg2_signature[1] = 0xd9;
  jpeg3_signature[0] = 0xff;
  jpeg3_signature[1] = 0xd8;
  jpeg3_signature[2] = 0xff;
  jpeg3_signature[3] = 0xe0;
  jfif_signature[0] = 0x4a;
  jfif_signature[1] = 0x46;
  jfif_signature[2] = 0x49;
  jfif_signature[3] = 0x46;
  exif_signature[0] = 0x45;
  exif_signature[1] = 0x78;
  exif_signature[2] = 0x69;
  exif_signature[3] = 0x66;
  png_signature[0] = 0x89;
  png_signature[1] = 0x50;
  png_signature[2] = 0x4e;
  png_signature[3] = 0x47;
  png_signature[4] = 0x0d;
  png_signature[5] = 0x0a;
  png_signature[6] = 0x1a;
  png_signature[7] = 0x0a;
  zip_signature[0] = 0x50;
  zip_signature[1] = 0x4b;
  zip_signature[2] = 0x03;
  zip_signature[3] = 0x04;
  if ( size < 1 || !blob )
    return GAIA_HEX_BLOB;
  if ( size > 5 )
  {
    if ( strncmp(( char * ) blob, "%PDF-", 5 ) == 0 )
      return GAIA_PDF_BLOB;
  }
  if ( size > 4 )
  {
    if ( memcmp( blob, zip_signature, 4 ) == 0 )
      return GAIA_ZIP_BLOB;
  }
  if ( size > 6 )
  {
    if ( strncmp(( char * ) blob, "GIF87a", 6 ) == 0
         || strncmp(( char * ) blob, "GIF89a", 6 ) == 0 )
      return GAIA_GIF_BLOB;
  }
  if ( size > 8 )
  {
    if ( memcmp( blob, png_signature, 8 ) == 0 )
      return GAIA_PNG_BLOB;
  }
  if ( size > 4 )
  {
    if ( memcmp( blob, jpeg1_signature, 2 ) == 0
         && memcmp( blob + size - 2, jpeg2_signature, 2 ) == 0 )
      jpeg = 1;  /* this one is the standard JPEG signature */
    if ( memcmp( blob, jpeg3_signature, 4 ) == 0 )
      jpeg = 1;  /* another common JPEG signature */
  }
  if ( size > 10 )
  {
    if ( memcmp( blob + 6, jfif_signature, 4 ) == 0 )
      jpeg = 1;  /* standard JFIF signature */
    if ( memcmp( blob + 6, exif_signature, 4 ) == 0 )
      jpeg = 1;  /* standard EXIF signature */
  }
  if ( jpeg )
  {
    exif_list = gaiaGetExifTags( blob, size );
    if ( exif_list )
    {
      exif = 1;
      pT = exif_list->First;
      while ( pT )
      {
        if ( pT->Gps )
        {
          exif_gps = 1;
          break;
        }
        pT = pT->Next;
      }
      gaiaExifTagsFree( exif_list );
    }
  }
  if ( jpeg && exif && exif_gps )
    return GAIA_EXIF_GPS_BLOB;
  if ( jpeg && exif )
    return GAIA_EXIF_BLOB;
  if ( jpeg )
    return GAIA_JPEG_BLOB;
  /* testing for GEOMETRY */
  if ( size < 45 )
    geom = 0;
  else
  {
    if ( *( blob + 0 ) != GAIA_MARK_START )
      geom = 0;
    if ( *( blob + ( size - 1 ) ) != GAIA_MARK_END )
      geom = 0;
    if ( *( blob + 38 ) != GAIA_MARK_MBR )
      geom = 0;
    if ( *( blob + 1 ) == 0 || *( blob + 1 ) == 1 )
      ;
    else
      geom = 0;
  }
  if ( geom )
    return GAIA_GEOMETRY_BLOB;
  return GAIA_HEX_BLOB;
}

GAIAEXIF_DECLARE int
gaiaGetGpsCoords( const unsigned char *blob, int size, double *longitude,
                  double *latitude )
{
  /* returns the ExifGps coords, if they exists */
  gaiaExifTagListPtr exif_list;
  gaiaExifTagPtr pT;
  char lat_ref = '\0';
  char long_ref = '\0';
  double lat_degs = DBL_MIN;
  double lat_mins = DBL_MIN;
  double lat_secs = DBL_MIN;
  double long_degs = DBL_MIN;
  double long_mins = DBL_MIN;
  double long_secs = DBL_MIN;
  double dblval;
  double sign;
  int ok;
  if ( size < 1 || !blob )
    return 0;
  exif_list = gaiaGetExifTags( blob, size );
  if ( exif_list )
  {
    pT = exif_list->First;
    while ( pT )
    {
      if ( pT->Gps && pT->TagId == 0x01 )
      {
        // ok, this one is the GPSLatitudeRef tag
        if ( pT->Type == 2 )
          lat_ref = *( pT->StringValue );
      }
      if ( pT->Gps && pT->TagId == 0x03 )
      {
        // ok, this one is the GPSLongitudeRef tag
        if ( pT->Type == 2 )
          long_ref = *( pT->StringValue );
      }
      if ( pT->Gps && pT->TagId == 0x02 )
      {
        // ok, this one is the GPSLatitude tag
        if ( pT->Type == 5 && pT->Count == 3 )
        {
          dblval = gaiaExifTagGetRationalValue( pT, 0, &ok );
          if ( ok )
            lat_degs = dblval;
          dblval = gaiaExifTagGetRationalValue( pT, 1, &ok );
          if ( ok )
            lat_mins = dblval;
          dblval = gaiaExifTagGetRationalValue( pT, 2, &ok );
          if ( ok )
            lat_secs = dblval;
        }
      }
      if ( pT->Gps && pT->TagId == 0x04 )
      {
        // ok, this one is the GPSLongitude tag
        if ( pT->Type == 5 && pT->Count == 3 )
        {
          dblval = gaiaExifTagGetRationalValue( pT, 0, &ok );
          if ( ok )
            long_degs = dblval;
          dblval = gaiaExifTagGetRationalValue( pT, 1, &ok );
          if ( ok )
            long_mins = dblval;
          dblval = gaiaExifTagGetRationalValue( pT, 2, &ok );
          if ( ok )
            long_secs = dblval;
        }
      }
      pT = pT->Next;
    }
    gaiaExifTagsFree( exif_list );
    if (( lat_ref == 'N' || lat_ref == 'S' || long_ref == 'E'
          || long_ref == 'W' ) && lat_degs != DBL_MIN && lat_mins != DBL_MIN
        && lat_secs != DBL_MIN && long_degs != DBL_MIN
        && long_mins != DBL_MIN && long_secs != DBL_MIN )
    {
      if ( lat_ref == 'S' )
        sign = -1.0;
      else
        sign = 1.0;
      lat_degs = math_round( lat_degs * 1000000.0 );
      lat_mins = math_round( lat_mins * 1000000.0 );
      lat_secs = math_round( lat_secs * 1000000.0 );
      dblval =
        math_round( lat_degs + ( lat_mins / 60.0 ) +
                    ( lat_secs / 3600.0 ) ) * ( sign / 1000000.0 );
      *latitude = dblval;
      if ( long_ref == 'W' )
        sign = -1.0;
      else
        sign = 1.0;
      long_degs = math_round( long_degs * 1000000.0 );
      long_mins = math_round( long_mins * 1000000.0 );
      long_secs = math_round( long_secs * 1000000.0 );
      dblval =
        math_round( long_degs + ( long_mins / 60.0 ) +
                    ( long_secs / 3600.0 ) ) * ( sign / 1000000.0 );
      *longitude = dblval;
      return 1;
    }
  }
  return 0;
}

GAIAEXIF_DECLARE int
gaiaGetGpsLatLong( const unsigned char *blob, int size, char *latlong,
                   int ll_size )
{
  /* returns the ExifGps Latitude and Longitude, if they exists */
  gaiaExifTagListPtr exif_list;
  gaiaExifTagPtr pT;
  char lat_ref = '\0';
  char long_ref = '\0';
  double lat_degs = DBL_MIN;
  double lat_mins = DBL_MIN;
  double lat_secs = DBL_MIN;
  double long_degs = DBL_MIN;
  double long_mins = DBL_MIN;
  double long_secs = DBL_MIN;
  double dblval;
  int ok;
  char ll[1024];
  int len;
  *latlong = '\0';
  if ( size < 1 || !blob )
    return 0;
  exif_list = gaiaGetExifTags( blob, size );
  if ( exif_list )
  {
    pT = exif_list->First;
    while ( pT )
    {
      if ( pT->Gps && pT->TagId == 0x01 )
      {
        // ok, this one is the GPSLatitudeRef tag
        if ( pT->Type == 2 )
          lat_ref = *( pT->StringValue );
      }
      if ( pT->Gps && pT->TagId == 0x03 )
      {
        // ok, this one is the GPSLongitudeRef tag
        if ( pT->Type == 2 )
          long_ref = *( pT->StringValue );
      }
      if ( pT->Gps && pT->TagId == 0x02 )
      {
        // ok, this one is the GPSLatitude tag
        if ( pT->Type == 5 && pT->Count == 3 )
        {
          dblval = gaiaExifTagGetRationalValue( pT, 0, &ok );
          if ( ok )
            lat_degs = dblval;
          dblval = gaiaExifTagGetRationalValue( pT, 1, &ok );
          if ( ok )
            lat_mins = dblval;
          dblval = gaiaExifTagGetRationalValue( pT, 2, &ok );
          if ( ok )
            lat_secs = dblval;
        }
      }
      if ( pT->Gps && pT->TagId == 0x04 )
      {
        // ok, this one is the GPSLongitude tag
        if ( pT->Type == 5 && pT->Count == 3 )
        {
          dblval = gaiaExifTagGetRationalValue( pT, 0, &ok );
          if ( ok )
            long_degs = dblval;
          dblval = gaiaExifTagGetRationalValue( pT, 1, &ok );
          if ( ok )
            long_mins = dblval;
          dblval = gaiaExifTagGetRationalValue( pT, 2, &ok );
          if ( ok )
            long_secs = dblval;
        }
      }
      pT = pT->Next;
    }
    gaiaExifTagsFree( exif_list );
    if (( lat_ref == 'N' || lat_ref == 'S' || long_ref == 'E'
          || long_ref == 'W' ) && lat_degs != DBL_MIN && lat_mins != DBL_MIN
        && lat_secs != DBL_MIN && long_degs != DBL_MIN
        && long_mins != DBL_MIN && long_secs != DBL_MIN )
    {
      sprintf( ll,
               "%c %1.2lf %1.2lf %1.2lf / %c %1.2lf %1.2lf %1.2lf",
               lat_ref, lat_degs, lat_mins, lat_secs, long_ref,
               long_degs, long_mins, long_secs );
      len = strlen( ll );
      if ( len < ll_size )
        strcpy( latlong, ll );
      else
      {
        memcpy( latlong, ll, ll_size - 1 );
        latlong[ll_size] = '\0';
      }
      return 1;
    }
  }
  return 0;
}
