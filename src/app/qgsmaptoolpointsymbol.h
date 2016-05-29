/***************************************************************************
    qgsmaptoolpointsymbol.h
    -----------------------
    begin                : April 2016
    copyright            : (C) 2016 by Marco Hugentobler, Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLPOINTSYMBOL_H
#define QGSMAPTOOLPOINTSYMBOL_H

#include "qgsmaptooledit.h"
#include "qgsfeature.h"

class QgsMarkerSymbolV2;

/** \ingroup app
 * \class QgsMapToolPointSymbol
 * \brief An abstract base class that allows interactive manipulation of the symbols for point layers. Handles
 * snapping the mouse press to a feature, and detecting whether the clicked feature has symbology which is
 * compatible with the map tool.
 */
class APP_EXPORT QgsMapToolPointSymbol: public QgsMapToolEdit
{
    Q_OBJECT

  public:
    QgsMapToolPointSymbol( QgsMapCanvas* canvas );

    virtual Flags flags() const override { return QgsMapTool::EditTool; }

    void canvasPressEvent( QgsMapMouseEvent* e ) override;

  protected:
    QgsVectorLayer* mActiveLayer;
    QgsFeatureId mFeatureNumber;

    /** Screen coordinate of the snaped feature*/
    QPoint mSnappedPoint;

    virtual void canvasPressOnFeature( QgsMapMouseEvent* e, const QgsFeature& feature, const QgsPoint& snappedPoint ) = 0;

    virtual bool checkSymbolCompatibility( QgsMarkerSymbolV2* markerSymbol, QgsRenderContext& context ) = 0;

    virtual void noCompatibleSymbols() {}

};

#endif // QGSMAPTOOLPOINTSYMBOL_H
