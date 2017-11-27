/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsmaptooledit.h                                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsMapToolEdit: QgsMapTool
{
%Docstring
 Base class for map tools that edit vector geometry
%End

%TypeHeaderCode
#include "qgsmaptooledit.h"
%End
  public:
    QgsMapToolEdit( QgsMapCanvas *canvas );

    virtual Flags flags() const;

    double defaultZValue() const;
%Docstring
 Return default Z value
 Use for set Z coordinate to new vertex for 2.5d geometries
 :rtype: float
%End

  protected:

    static QColor digitizingStrokeColor();
%Docstring
Returns stroke color for rubber bands (from global settings)
 :rtype: QColor
%End
    static int digitizingStrokeWidth();
%Docstring
Returns stroke width for rubber bands (from global settings)
 :rtype: int
%End
    static QColor digitizingFillColor();
%Docstring
Returns fill color for rubber bands (from global settings)
 :rtype: QColor
%End

    QgsRubberBand *createRubberBand( QgsWkbTypes::GeometryType geometryType = QgsWkbTypes::LineGeometry, bool alternativeBand = false ) /Factory/;
%Docstring
 Creates a rubber band with the color/line width from
   the QGIS settings. The caller takes ownership of the
   returned object
   \param geometryType
   \param alternativeBand if true, rubber band will be set with more transparency and a dash pattern. default is false.
 :rtype: QgsRubberBand
%End

    QgsGeometryRubberBand *createGeometryRubberBand( QgsWkbTypes::GeometryType geometryType = QgsWkbTypes::LineGeometry, bool alternativeBand = false ) const /Factory/;
%Docstring
 :rtype: QgsGeometryRubberBand
%End

    QgsVectorLayer *currentVectorLayer();
%Docstring
Returns the current vector layer of the map canvas or 0
 :rtype: QgsVectorLayer
%End

    int addTopologicalPoints( const QVector<QgsPointXY> &geom );
%Docstring
 Adds vertices to other features to keep topology up to date, e.g. to neighbouring polygons.
 \param geom list of points (in layer coordinate system)
 :return: 0 in case of success
 :rtype: int
%End

    void notifyNotVectorLayer();
%Docstring
Display a timed message bar noting the active layer is not vector.
%End
    void notifyNotEditableLayer();
%Docstring
Display a timed message bar noting the active vector layer is not editable.
%End
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsmaptooledit.h                                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
