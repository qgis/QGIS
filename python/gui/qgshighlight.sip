/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgshighlight.h                                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsHighlight: QgsMapCanvasItem
{
%Docstring
 A class for highlight features on the map.

 The QgsHighlight class provides a transparent overlay widget
for highlighting features on the map.
%End

%TypeHeaderCode
#include "qgshighlight.h"
%End
  public:


    QgsHighlight( QgsMapCanvas *mapCanvas, const QgsGeometry &geom, QgsVectorLayer *layer );
%Docstring
 Constructor for QgsHighlight
 \param mapCanvas associated map canvas
 \param geom initial geometry of highlight
 \param layer associated vector layer
%End

    QgsHighlight( QgsMapCanvas *mapCanvas, const QgsFeature &feature, QgsVectorLayer *layer );
%Docstring
 Constructor for highlighting true feature shape using feature attributes
 and renderer.
 \param mapCanvas map canvas
 \param feature
 \param layer vector layer
%End
    ~QgsHighlight();

    void setColor( const QColor &color );
%Docstring
 Set line/stroke to color, polygon fill to color with alpha = 63.
  This is legacy function, use setFillColor() after setColor() if different fill color is required. *
%End

    void setFillColor( const QColor &fillColor );
%Docstring
 Set polygons fill color.
.. versionadded:: 2.3
%End

    void setWidth( int width );
%Docstring
Set stroke width. Ignored in feature mode.
%End

    void setBuffer( double buffer );
%Docstring
 Set line / stroke buffer in millimeters.
.. versionadded:: 2.3
%End

    void setMinWidth( double width );
%Docstring
 Set minimum line / stroke width in millimeters.
.. versionadded:: 2.3
%End

    const QgsMapLayer *layer() const;
%Docstring
 :rtype: QgsMapLayer
%End

    virtual void updatePosition();

  protected:
    virtual void paint( QPainter *p );

    void updateRect();
%Docstring
recalculates needed rectangle
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgshighlight.h                                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
