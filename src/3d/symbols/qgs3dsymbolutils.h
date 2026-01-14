/***************************************************************************
  qgs3dsymbolutils.h
  --------------------------------------
  Date                 : January 2026
  Copyright            : (C) 2026 by Jean Felder
  Email                : jean dot felder at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGS3DSYMBOLUTILS_H
#define QGS3DSYMBOLUTILS_H

#include "qgis_3d.h"
#include "qgsscreenproperties.h"

#include <QColor>
#include <QIcon>

#define SIP_NO_FILE

class QgsAbstract3DSymbol;

/**
 * \ingroup qgis_3d
 * \brief Miscellaneous utility functions used in 3D code for symbols
 * \note Not available in Python bindings
 * \since QGIS 4.0
 */
class _3D_EXPORT Qgs3DSymbolUtils
{
  public:
    /**
     * Computes an approximate color from a 3D vector symbol.
     *
     * Extracts a representative color from the symbol's material settings,
     * accounting for its specific shading model (Phong, Gooch, etc.).
     *
     * \param symbol The 3D symbol to extract the color from.
     * \returns A QColor representing the symbol's average appearance, or a default color if the symbol type is not supported.
     */
    static QColor vectorSymbolAverageColor( const QgsAbstract3DSymbol *symbol );

    /**
     * Returns an icon preview for a 3D vector symbol.
     *
     * \param symbol 3D symbol to generate the preview for.
     * \param size Target pixmap size.
     * \param screen Destination screen properties used to determine DPI and device pixel ratio.
     * \param padding Space between icon edge and symbol.
     * \returns An icon containing a preview of the symbol.
     *
     * \note For line and circle symbols, the size of the drawn icon depends on the symbol's parameters.
     */
    static QIcon vectorSymbolPreviewIcon( const QgsAbstract3DSymbol *symbol, const QSize &size, const QgsScreenProperties &screen, int padding );

    /**
     * Sets the base color of the material settings associated with a 3D vector symbol.
     *
     * \param symbol 3D symbol whose material settings will be modified.
     * \param baseColor Base color to apply to the symbol's material settings.
     * \returns TRUE if the material's base color was successfully set, FALSE otherwise.
     */
    static bool setVectorSymbolBaseColor( QgsAbstract3DSymbol *symbol, const QColor &baseColor );
};

#endif // QGS3DSYMBOLUTILS_H
