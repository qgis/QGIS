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


/** Base class for map tools that edit vector geometry*/
class QgsMapToolEdit: QgsMapTool
{
%TypeHeaderCode
#include "qgsmaptooledit.h"
%End

  public:
    QgsMapToolEdit( QgsMapCanvas* canvas );
    virtual ~QgsMapToolEdit();

    virtual Flags flags() const;

  protected:

    /** Creates a rubber band with the color/line width from
     *   the QGIS settings. The caller takes ownership of the
     *   returned object
     *   @param geometryType
     *   @param alternativeBand if true, rubber band will be set with more transparency and a dash pattern. defaut is false.
     */
    QgsRubberBand* createRubberBand( QGis::GeometryType geometryType = QGis::Line, bool alternativeBand = false );

    QgsGeometryRubberBand* createGeometryRubberBand( QGis::GeometryType geometryType = QGis::Line, bool alternativeBand = false ) const;

    /** Returns the current vector layer of the map canvas or 0*/
    QgsVectorLayer* currentVectorLayer();

    /** Adds vertices to other features to keep topology up to date, e.g. to neighbouring polygons.
     * @param geom list of points (in layer coordinate system)
     * @return 0 in case of success
     */
    int addTopologicalPoints( const QList<QgsPoint>& geom );

    /** Display a timed message bar noting the active layer is not vector. */
    void notifyNotVectorLayer();
    /** Display a timed message bar noting the active vector layer is not editable. */
    void notifyNotEditableLayer();
};
