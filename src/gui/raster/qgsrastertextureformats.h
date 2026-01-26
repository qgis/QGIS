/***************************************************************************
  qgsrastertextureformats.h - GPU texture format mapping for rasters
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

#ifndef QGSRASTERTEXTUREFORMATS_H
#define QGSRASTERTEXTUREFORMATS_H

#include "qgis.h"
#include "qgis_gui.h"

#include <QOpenGLFunctions>
#include <QString>

/**
 * \ingroup gui
 * \class QgsRasterTextureFormat
 * \brief Maps QGIS/GDAL raster data types to OpenGL texture formats.
 *
 * Inspired by deck.gl-raster's format mapping approach, this class provides
 * optimal GPU texture formats for different raster data types, enabling
 * zero-copy uploads and GPU-native rendering.
 *
 * \since QGIS 3.40
 */
class GUI_EXPORT QgsRasterTextureFormat
{
  public:
    /**
     * \brief GPU texture format information
     */
    struct FormatInfo
    {
        GLenum internalFormat = GL_R8;  //!< OpenGL internal format (storage)
        GLenum format = GL_RED;         //!< Pixel data format
        GLenum type = GL_UNSIGNED_BYTE; //!< Pixel data type
        int bytesPerPixel = 1;          //!< Bytes per pixel
        int channelCount = 1;           //!< Number of channels (1, 2, 3, or 4)
        QString shaderType;             //!< Shader type identifier ("u8", "u16", "f32", etc.)
        bool isSupported = true;        //!< Whether format is supported on this hardware
    };

    /**
     * \brief Get optimal texture format for a QGIS data type
     * \param dataType The QGIS/GDAL data type
     * \param channelCount Number of channels (1 for single-band, 3-4 for RGB/RGBA)
     * \returns Format information
     */
    static FormatInfo getFormat( Qgis::DataType dataType, int channelCount = 1 );

    /**
     * \brief Check if a specific texture format is supported
     * \param format The OpenGL internal format to check
     * \returns TRUE if supported on current hardware
     */
    static bool isFormatSupported( GLenum format );

    /**
     * \brief Get shader type string for a QGIS data type
     * \param dataType The QGIS/GDAL data type
     * \returns Shader type identifier (for shader selection)
     */
    static QString shaderType( Qgis::DataType dataType );

  private:
    //! Initialize format table (called once)
    static void initializeFormats();

    //! Format lookup table
    static QHash<QPair<Qgis::DataType, int>, FormatInfo> sFormatTable;
    static bool sInitialized;
};

#endif // QGSRASTERTEXTUREFORMATS_H
