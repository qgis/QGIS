/***************************************************************************
 qgsmarkersymbolbounds.h
 -----------------------
 begin                : May 2021
 copyright            : (C) 2021 by Nyall Dawson
 email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMARKERSYMBOLBOUNDS_H
#define QGSMARKERSYMBOLBOUNDS_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include <QRectF>

/**
 * \ingroup core
 * \class QgsMarkerSymbolBounds
 *
 * \brief Describes the bounds of a marker symbol.
 *
 * \since QGIS 3.20
 */
class CORE_EXPORT QgsMarkerSymbolBounds
{
  public:

    /**
     * Returns TRUE if the bounds are null or unset.
     */
    bool isNull() const { return mBoundingBox.isNull(); }

    /**
     * Returns the bounding box of the symbol, which represents
     * the extent of the rendered symbol.
     *
     * \see setBoundingBox()
     */
    QRectF boundingBox() const { return mBoundingBox; }

    /**
     * Sets the bounding box of the symbol, which represents
     * the extent of the rendered symbol.
     *
     * \see boundingBox()
     */
    void setBoundingBox( const QRectF &bounds ) { mBoundingBox = bounds;}

    /**
     * Unites these bounds with an \a other bounds.
     */
    void unite( const QgsMarkerSymbolBounds &other )
    {
      mBoundingBox = mBoundingBox.united( other.boundingBox() );
    }

  private:

    QRectF mBoundingBox;

};

#endif // QGSMARKERSYMBOLBOUNDS_H

