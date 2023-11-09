/***************************************************************************
  qgsmapinfosymbolconverter.h
  --------------------------------------
  Date                 : March 2021
  Copyright            : (C) 2021 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPINFOSYMBOLCONVERTER_H
#define QGSMAPINFOSYMBOLCONVERTER_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgis.h"
#include <QStringList>
#include <QColor>

class QgsLineSymbol;
class QgsFillSymbol;
class QgsMarkerSymbol;

/**
 * Context for a MapInfo symbol conversion operation.
 * \warning This is private API only, and may change in future QGIS versions
 * \ingroup core
 * \since QGIS 3.20
 */
class CORE_EXPORT QgsMapInfoSymbolConversionContext
{
  public:

    /**
     * Pushes a \a warning message generated during the conversion.
     */
    void pushWarning( const QString &warning );

    /**
     * Returns a list of warning messages generated during the conversion.
     */
    QStringList warnings() const { return mWarnings; }

    /**
     * Clears the list of warning messages.
     */
    void clearWarnings() { mWarnings.clear(); }

  private:

    QStringList mWarnings;

};

/**
 * \ingroup core
 * \brief Handles conversion of MapInfo symbols to QGIS symbology.
 *
 * \since QGIS 3.20
 */
class CORE_EXPORT QgsMapInfoSymbolConverter
{
  public:

    /**
     * Converts the MapInfo line symbol with the specified \a identifier to a QgsLineSymbol.
     *
     * The caller takes ownership of the returned symbol.
     */
    static QgsLineSymbol *convertLineSymbol( int identifier, QgsMapInfoSymbolConversionContext &context, const QColor &foreColor, double size, Qgis::RenderUnit sizeUnit, bool interleaved = false ) SIP_FACTORY;

    /**
     * Converts the MapInfo fill symbol with the specified \a identifier to a QgsFillSymbol.
     *
     * The caller takes ownership of the returned symbol.
     */
    static QgsFillSymbol *convertFillSymbol( int identifier, QgsMapInfoSymbolConversionContext &context, const QColor &foreColor, const QColor &backColor = QColor() ) SIP_FACTORY;

    /**
     * Converts the MapInfo marker symbol with the specified \a identifier to a QgsMarkerSymbol.
     *
     * This method will convert a MapInfo "MapInfo 3.0 Compatible" symbol with a specific \a identifier to a QgsMarkerSymbol.
     *
     * The caller takes ownership of the returned symbol.
     */
    static QgsMarkerSymbol *convertMarkerSymbol( int identifier, QgsMapInfoSymbolConversionContext &context, const QColor &color, double size, Qgis::RenderUnit sizeUnit ) SIP_FACTORY;

};

#endif // QGSMAPINFOSYMBOLCONVERTER_H
