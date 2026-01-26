/***************************************************************************
  qgsrastertextureformats.cpp - GPU texture format mapping for rasters
  --------------------------------------
  Date                 : January 2026
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrastertextureformats.h"

#include "qgslogger.h"

#include <QOpenGLContext>

QHash<QPair<Qgis::DataType, int>, QgsRasterTextureFormat::FormatInfo> QgsRasterTextureFormat::sFormatTable;
bool QgsRasterTextureFormat::sInitialized = false;

void QgsRasterTextureFormat::initializeFormats()
{
  if ( sInitialized )
    return;

  // Single-channel formats (grayscale/DEM/etc.)

  // Byte (8-bit unsigned)
  sFormatTable[{ Qgis::DataType::Byte, 1 }] = {
    GL_R8,                  // internal format
    GL_RED,                 // format
    GL_UNSIGNED_BYTE,       // type
    1,                      // bytes per pixel
    1,                      // channel count
    QStringLiteral( "u8" ), // shader type
    true                    // supported
  };

  // UInt16 (16-bit unsigned)
  sFormatTable[{ Qgis::DataType::UInt16, 1 }] = {
    GL_R16,
    GL_RED,
    GL_UNSIGNED_SHORT,
    2,
    1,
    QStringLiteral( "u16" ),
    true
  };

  // Int16 (16-bit signed)
  sFormatTable[{ Qgis::DataType::Int16, 1 }] = {
    GL_R16_SNORM,
    GL_RED,
    GL_SHORT,
    2,
    1,
    QStringLiteral( "i16" ),
    true
  };

  // Float32 (32-bit float)
  sFormatTable[{ Qgis::DataType::Float32, 1 }] = {
    GL_R32F,
    GL_RED,
    GL_FLOAT,
    4,
    1,
    QStringLiteral( "f32" ),
    true
  };

  // Float64 → Float32 (downcast to 32-bit for GPU)
  sFormatTable[{ Qgis::DataType::Float64, 1 }] = {
    GL_R32F,
    GL_RED,
    GL_FLOAT,
    4, // Note: CPU data is 8 bytes, but we convert to 4 for GPU
    1,
    QStringLiteral( "f32" ),
    true
  };

  // Multi-channel formats (RGB/RGBA)

  // RGB Byte (3-channel, 8-bit)
  sFormatTable[{ Qgis::DataType::Byte, 3 }] = {
    GL_RGB8,
    GL_RGB,
    GL_UNSIGNED_BYTE,
    3,
    3,
    QStringLiteral( "rgb8" ),
    true
  };

  // RGBA Byte (4-channel, 8-bit)
  sFormatTable[{ Qgis::DataType::Byte, 4 }] = {
    GL_RGBA8,
    GL_RGBA,
    GL_UNSIGNED_BYTE,
    4,
    4,
    QStringLiteral( "rgba8" ),
    true
  };

  // RGBA UInt16 (4-channel, 16-bit)
  sFormatTable[{ Qgis::DataType::UInt16, 4 }] = {
    GL_RGBA16,
    GL_RGBA,
    GL_UNSIGNED_SHORT,
    8,
    4,
    QStringLiteral( "rgba16" ),
    true
  };

  // RGBA Float32 (4-channel, 32-bit float)
  sFormatTable[{ Qgis::DataType::Float32, 4 }] = {
    GL_RGBA32F,
    GL_RGBA,
    GL_FLOAT,
    16,
    4,
    QStringLiteral( "rgba32f" ),
    true
  };

  sInitialized = true;

  QgsDebugMsgLevel( QStringLiteral( "Initialized %1 texture formats" ).arg( sFormatTable.size() ), 2 );
}

QgsRasterTextureFormat::FormatInfo QgsRasterTextureFormat::getFormat( Qgis::DataType dataType, int channelCount )
{
  initializeFormats();

  const QPair<Qgis::DataType, int> key( dataType, channelCount );

  if ( sFormatTable.contains( key ) )
  {
    return sFormatTable.value( key );
  }

  // Fallback: try single-channel if multi-channel not found
  if ( channelCount > 1 )
  {
    const QPair<Qgis::DataType, int> fallbackKey( dataType, 1 );
    if ( sFormatTable.contains( fallbackKey ) )
    {
      QgsDebugMsgLevel( QStringLiteral( "Using single-channel format for multi-channel %1" ).arg( channelCount ), 2 );
      return sFormatTable.value( fallbackKey );
    }
  }

  // Default fallback: R8
  QgsDebugError( QStringLiteral( "No texture format for dataType=%1, channels=%2, using R8 fallback" )
                   .arg( static_cast<int>( dataType ) )
                   .arg( channelCount ) );

  FormatInfo fallback;
  fallback.internalFormat = GL_R8;
  fallback.format = GL_RED;
  fallback.type = GL_UNSIGNED_BYTE;
  fallback.bytesPerPixel = 1;
  fallback.channelCount = 1;
  fallback.shaderType = QStringLiteral( "u8" );
  fallback.isSupported = false; // Mark as unsupported to warn user

  return fallback;
}

bool QgsRasterTextureFormat::isFormatSupported( GLenum format )
{
  // Basic format support check
  // Could be extended to query actual hardware capabilities

  switch ( format )
  {
    case GL_R8:
    case GL_R16:
    case GL_R32F:
    case GL_RGB8:
    case GL_RGBA8:
      return true; // Widely supported

    case GL_R16_SNORM:
    case GL_RGBA16:
    case GL_RGBA32F:
      // Supported on most modern hardware
      return QOpenGLContext::currentContext() != nullptr;

    default:
      return false;
  }
}

QString QgsRasterTextureFormat::shaderType( Qgis::DataType dataType )
{
  const FormatInfo format = getFormat( dataType, 1 );
  return format.shaderType;
}
