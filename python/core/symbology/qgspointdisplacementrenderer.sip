/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/symbology/qgspointdisplacementrenderer.h                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsPointDisplacementRenderer: QgsPointDistanceRenderer
{
%Docstring
 A renderer that automatically displaces points with the same geographic location.
%End

%TypeHeaderCode
#include "qgspointdisplacementrenderer.h"
%End
  public:

    enum Placement
    {
      Ring,
      ConcentricRings,
      Grid
    };

    QgsPointDisplacementRenderer( const QString &labelAttributeName = QString() );
%Docstring
 Constructor for QgsPointDisplacementRenderer.
 \param labelAttributeName optional attribute name for labeling points
%End

    virtual QgsPointDisplacementRenderer *clone() const /Factory/;

    virtual void startRender( QgsRenderContext &context, const QgsFields &fields );
    virtual void stopRender( QgsRenderContext &context );

    virtual QDomElement save( QDomDocument &doc, const QgsReadWriteContext &context );

    virtual QSet<QString> usedAttributes( const QgsRenderContext &context ) const;

    static QgsFeatureRenderer *create( QDomElement &symbologyElem, const QgsReadWriteContext &context ) /Factory/;
%Docstring
Create a renderer from XML element
 :rtype: QgsFeatureRenderer
%End

    void setCircleWidth( double width );
%Docstring
 Sets the line width for the displacement group circle.
 \param width line width in mm
.. seealso:: circleWidth()
.. seealso:: setCircleColor()
%End

    double circleWidth() const;
%Docstring
 Returns the line width for the displacement group circle in mm.
.. seealso:: setCircleWidth()
.. seealso:: circleColor()
 :rtype: float
%End

    void setCircleColor( const QColor &color );
%Docstring
 Sets the color used for drawing the displacement group circle.
 \param color circle color
.. seealso:: circleColor()
.. seealso:: setCircleWidth()
%End

    QColor circleColor() const;
%Docstring
 Returns the color used for drawing the displacement group circle.
.. seealso:: setCircleColor()
.. seealso:: circleWidth()
 :rtype: QColor
%End

    void setCircleRadiusAddition( double distance );
%Docstring
 Sets a factor for increasing the ring size of displacement groups.
 \param distance addition factor
.. seealso:: circleRadiusAddition()
%End

    double circleRadiusAddition() const;
%Docstring
 Returns the factor for increasing the ring size of displacement groups.
.. seealso:: setCircleRadiusAddition()
 :rtype: float
%End

    Placement placement() const;
%Docstring
 Returns the placement method used for dispersing the points.
.. seealso:: setPlacement()
.. versionadded:: 2.12
 :rtype: Placement
%End

    void setPlacement( Placement placement );
%Docstring
 Sets the placement method used for dispersing the points.
 \param placement placement method
.. seealso:: placement()
.. versionadded:: 2.12
%End

    QgsMarkerSymbol *centerSymbol();
%Docstring
 Returns the symbol for the center of a displacement group (but not ownership of the symbol).
.. seealso:: setCenterSymbol()
 :rtype: QgsMarkerSymbol
%End

    void setCenterSymbol( QgsMarkerSymbol *symbol /Transfer/ );
%Docstring
 Sets the center symbol for a displacement group.
 \param symbol new center symbol. Ownership is transferred to the renderer.
.. seealso:: centerSymbol()
%End

    static QgsPointDisplacementRenderer *convertFromRenderer( const QgsFeatureRenderer *renderer ) /Factory/;
%Docstring
 Creates a QgsPointDisplacementRenderer from an existing renderer.
.. versionadded:: 2.5
 :return: a new renderer if the conversion was possible, otherwise None.
 :rtype: QgsPointDisplacementRenderer
%End

  private:
    QgsPointDisplacementRenderer( const QgsPointDisplacementRenderer & );
    QgsPointDisplacementRenderer &operator=( const QgsPointDisplacementRenderer & );
%Docstring
 :rtype: QgsPointDisplacementRenderer
%End
    virtual void drawGroup( QPointF centerPoint, QgsRenderContext &context, const QgsPointDistanceRenderer::ClusteredGroup &group ) ;
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/symbology/qgspointdisplacementrenderer.h                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
