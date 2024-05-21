/***************************************************************************
                         qgsrasterrendererutils.h
                         -------------------
    begin                : September 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dawson dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRASTERRENDERERUTILS_H
#define QGSRASTERRENDERERUTILS_H

#include "qgis.h"
#include "qgscolorrampshader.h"

/**
 * \ingroup core
  * \brief Utility functions for raster renderers.
  *
  * \since QGIS 3.16
  */
class CORE_EXPORT QgsRasterRendererUtils
{
  public:

    /**
     * Parses an exported color map file at the specified \a path and extracts the stored color ramp \a items
     * and ramp shader \a type.
     *
     * Returns TRUE if the parsing was successful. If not, a list of \a errors will be generated.
     *
     * \see saveColorMapFile()
     */
    static bool parseColorMapFile( const QString &path, QList<QgsColorRampShader::ColorRampItem> &items SIP_OUT,
                                   Qgis::ShaderInterpolationMethod &type SIP_OUT,
                                   QStringList &errors SIP_OUT );

    /**
     * Exports a list of color ramp \a items and ramp shader \a type to a color map file at the specified
     * \a path.
     *
     * Returns TRUE if the save was successful.
     *
     * \see parseColorMapFile()
     */
    static bool saveColorMapFile( const QString &path, const QList<QgsColorRampShader::ColorRampItem> &items, Qgis::ShaderInterpolationMethod type );
};

#endif // QGSRASTERRENDERERUTILS_H
