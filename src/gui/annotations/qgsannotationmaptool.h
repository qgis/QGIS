/***************************************************************************
    qgsannotationmaptool.h
    ----------------
    copyright            : (C) 2025 by Mathieu Pellerin
    email                : mathieu at opengis dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSANNOTATIONMAPTOOL_H
#define QGSANNOTATIONMAPTOOL_H

#include "qgis_gui.h"
#include "qgis_sip.h"
#include "qgsmapmouseevent.h"
#include "qgsmaptooladvanceddigitizing.h"
#include "qgsmaptooledit.h"

class QgsRenderedAnnotationItemDetails;
class QgsAnnotationLayer;
class QgsAnnotationItem;

#define SIP_NO_FILE


/**
 * \ingroup gui
 * \brief A base class for annotation map tools
 * \note Not available in Python bindings
 * \since QGIS 4.0
 */
class GUI_EXPORT QgsAnnotationMapTool : public QgsMapToolAdvancedDigitizing
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsAnnotationMapTool
     */
    explicit QgsAnnotationMapTool( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget );
    ~QgsAnnotationMapTool() override = default;

    /**
     * Returns the closest item from a list of annotation items to a given map point.
     * \param mapPoint the map point
     * \param items the rendered annotation items list
     * \param bounds the bounds
     */
    const QgsRenderedAnnotationItemDetails *findClosestItemToPoint( const QgsPointXY &mapPoint, const QList<const QgsRenderedAnnotationItemDetails *> &items, QgsRectangle &bounds );

    /**
     * Returns the annotation layer matching a given ID.
     * \param layerId the annotation layer ID
     */
    QgsAnnotationLayer *annotationLayerFromId( const QString &layerId );

    /**
     * Returns the annotation item matching a given pair of layer and item IDs.
     * \param layerId the layer ID
     * \param itemId the item ID
     */
    QgsAnnotationItem *annotationItemFromId( const QString &layerId, const QString &itemId );
};

#endif // QGSANNOTATIONMAPTOOL_H
