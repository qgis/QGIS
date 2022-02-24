/***************************************************************************
    qgsmaptooledit.h  -  base class for editing map tools
    ---------------------
    begin                : Juli 2007
    copyright            : (C) 2007 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLEDIT_H
#define QGSMAPTOOLEDIT_H

#include "qgswkbtypes.h"
#include "qgsmaptool.h"
#include "qgis_gui.h"

class QgsRubberBand;
class QgsGeometryRubberBand;
class QgsVectorLayer;
class QKeyEvent;

/**
 * \ingroup gui
 * \brief Base class for map tools that edit vector geometry
*/
class GUI_EXPORT QgsMapToolEdit: public QgsMapTool
{
    Q_OBJECT

  public:
    QgsMapToolEdit( QgsMapCanvas *canvas );

    Flags flags() const override { return QgsMapTool::EditTool; }

    /**
     * Returns default Z value.
     * Used for setting Z coordinate to new vertex.
     */
    static double defaultZValue();

    /**
     * Returns default M value.
     * Used for setting M coordinate to new vertex.
     *
     * \since QGIS 3.20
     */
    static double defaultMValue();

    /**
     * Creates a  geometry rubber band with the color/line width from
     *   the QGIS settings. The caller takes ownership of the
     *   returned object
     *   \param geometryType
     *   \param alternativeBand if TRUE, rubber band will be set with more transparency and a dash pattern. default is FALSE.
     */
    QgsGeometryRubberBand *createGeometryRubberBand( QgsWkbTypes::GeometryType geometryType = QgsWkbTypes::LineGeometry, bool alternativeBand = false ) const SIP_FACTORY;

  private slots:
    //! Vector layers' editingStopped SIGNAL will eventually trigger a clean
    void connectLayers( const QList<QgsMapLayer *> &layers );

    /**
     * Makes sure rubber bands are removed if there
     * is no editable layer left in the project
     */
    void cleanCanvas();

  protected:

    //! Returns stroke color for rubber bands (from global settings)
    static QColor digitizingStrokeColor();
    //! Returns stroke width for rubber bands (from global settings)
    static int digitizingStrokeWidth();
    //! Returns fill color for rubber bands (from global settings)
    static QColor digitizingFillColor();

    /**
     * Creates a rubber band with the color/line width from
     *   the QGIS settings. The caller takes ownership of the
     *   returned object
     *   \param geometryType
     *   \param alternativeBand if TRUE, rubber band will be set with more transparency and a dash pattern. default is FALSE.
     */
    QgsRubberBand *createRubberBand( QgsWkbTypes::GeometryType geometryType = QgsWkbTypes::LineGeometry, bool alternativeBand = false ) SIP_FACTORY;

    //! Returns the current vector layer of the map canvas or 0
    QgsVectorLayer *currentVectorLayer();

    //! Result of addTopologicalPoints
    enum TopologicalResult
    {
      Success = 0, //!< AddTopologicalPoints was successful
      InvalidCanvas = 1, //!< AddTopologicalPoints failed due to an invalid canvas
      InvalidLayer = 2, //!< AddTopologicalPoints failed due to an invalid canvas
    };

    /**
     * Adds a list of \a vertices to other features to keep topology up to date, e.g. to neighbouring polygons.
     * The \a vertices list specifies a set of topological points to add, in the layer's coordinate reference system.
     * \deprecated since QGIS 3.12 - will be removed in QGIS 4.0. Use the variant which accepts QgsPoint objects instead of QgsPointXY.
     */
    Q_DECL_DEPRECATED TopologicalResult addTopologicalPoints( const QVector<QgsPointXY> &vertices )  SIP_DEPRECATED;

    /**
     * Adds a list of \a vertices to other features to keep topology up to date, e.g. to neighbouring polygons.
     * The \a vertices list specifies a set of topological points to add, in the layer's coordinate reference system.
     * \since QGIS 3.10
     */
    TopologicalResult addTopologicalPoints( const QVector<QgsPoint> &vertices );

    //! Display a timed message bar noting the active layer is not vector.
    void notifyNotVectorLayer();
    //! Display a timed message bar noting the active vector layer is not editable.
    void notifyNotEditableLayer();

  private:
    //! Returns a list of layers filtered to just editable spatial vector layers
    QList<QgsVectorLayer *> editableVectorLayers();
};

#endif
