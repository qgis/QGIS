/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/symbology/qgsheatmaprenderer.h                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsHeatmapRenderer : QgsFeatureRenderer
{
%Docstring
 A renderer which draws points as a live heatmap
.. versionadded:: 2.7
%End

%TypeHeaderCode
#include "qgsheatmaprenderer.h"
%End
  public:

    QgsHeatmapRenderer();
    virtual ~QgsHeatmapRenderer();


    virtual QgsHeatmapRenderer *clone() const /Factory/;
    virtual void startRender( QgsRenderContext &context, const QgsFields &fields );
    virtual bool renderFeature( QgsFeature &feature, QgsRenderContext &context, int layer = -1, bool selected = false, bool drawVertexMarker = false );
    virtual void stopRender( QgsRenderContext &context );
    virtual QgsSymbol *symbolForFeature( QgsFeature &feature, QgsRenderContext &context );
%Docstring
.. note::

   symbolForFeature2 in Python bindings
 :rtype: QgsSymbol
%End
    virtual QgsSymbolList symbols( QgsRenderContext &context );
%Docstring
.. note::

   symbol2 in Python bindings
 :rtype: QgsSymbolList
%End
    virtual QString dump() const;
    virtual QSet<QString> usedAttributes( const QgsRenderContext &context ) const;
    static QgsFeatureRenderer *create( QDomElement &element, const QgsReadWriteContext &context ) /Factory/;
%Docstring
Creates a new heatmap renderer instance from XML
 :rtype: QgsFeatureRenderer
%End
    virtual QDomElement save( QDomDocument &doc, const QgsReadWriteContext &context );
    static QgsHeatmapRenderer *convertFromRenderer( const QgsFeatureRenderer *renderer ) /Factory/;
%Docstring
 :rtype: QgsHeatmapRenderer
%End

    virtual void modifyRequestExtent( QgsRectangle &extent, QgsRenderContext &context );


    QgsColorRamp *colorRamp() const;
%Docstring
 Returns the color ramp used for shading the heatmap.
 :return: color ramp for heatmap
.. seealso:: setColorRamp
 :rtype: QgsColorRamp
%End

    void setColorRamp( QgsColorRamp *ramp /Transfer/ );
%Docstring
 Sets the color ramp to use for shading the heatmap.
 \param ramp color ramp for heatmap. Ownership of ramp is transferred to the renderer.
.. seealso:: colorRamp
%End

    double radius() const;
%Docstring
 Returns the radius for the heatmap
 :return: heatmap radius
.. seealso:: setRadius
.. seealso:: radiusUnit
.. seealso:: radiusMapUnitScale
 :rtype: float
%End

    void setRadius( const double radius );
%Docstring
 Sets the radius for the heatmap
 \param radius heatmap radius
.. seealso:: radius
.. seealso:: setRadiusUnit
.. seealso:: setRadiusMapUnitScale
%End

    QgsUnitTypes::RenderUnit radiusUnit() const;
%Docstring
 Returns the units used for the heatmap's radius
 :return: units for heatmap radius
.. seealso:: radius
.. seealso:: setRadiusUnit
.. seealso:: radiusMapUnitScale
 :rtype: QgsUnitTypes.RenderUnit
%End

    void setRadiusUnit( const QgsUnitTypes::RenderUnit unit );
%Docstring
 Sets the units used for the heatmap's radius
 \param unit units for heatmap radius
.. seealso:: radiusUnit
.. seealso:: setRadius
.. seealso:: radiusMapUnitScale
%End

    const QgsMapUnitScale &radiusMapUnitScale() const;
%Docstring
 Returns the map unit scale used for the heatmap's radius
 :return: map unit scale for heatmap's radius
.. seealso:: radius
.. seealso:: radiusUnit
.. seealso:: setRadiusMapUnitScale
 :rtype: QgsMapUnitScale
%End

    void setRadiusMapUnitScale( const QgsMapUnitScale &scale );
%Docstring
 Sets the map unit scale used for the heatmap's radius
 \param scale map unit scale for heatmap's radius
.. seealso:: setRadius
.. seealso:: setRadiusUnit
.. seealso:: radiusMapUnitScale
%End

    double maximumValue() const;
%Docstring
 Returns the maximum value used for shading the heatmap.
 :return: maximum value for heatmap shading. If 0, then maximum value will be automatically
 calculated.
.. seealso:: setMaximumValue
 :rtype: float
%End

    void setMaximumValue( const double value );
%Docstring
 Sets the maximum value used for shading the heatmap.
 \param value maximum value for heatmap shading. Set to 0 for automatic calculation of
 maximum value.
.. seealso:: maximumValue
%End

    double renderQuality() const;
%Docstring
 Returns the render quality used for drawing the heatmap.
 :return: render quality. A value of 1 indicates maximum quality, and increasing the
 value will result in faster drawing but lower quality rendering.
.. seealso:: setRenderQuality
 :rtype: float
%End

    void setRenderQuality( const int quality );
%Docstring
 Sets the render quality used for drawing the heatmap.
 \param quality render quality. A value of 1 indicates maximum quality, and increasing the
 value will result in faster drawing but lower quality rendering.
.. seealso:: renderQuality
%End

    QString weightExpression() const;
%Docstring
 Returns the expression used for weighting points when generating the heatmap.
 :return: point weight expression. If empty, all points are equally weighted.
.. seealso:: setWeightExpression
 :rtype: str
%End

    void setWeightExpression( const QString &expression );
%Docstring
 Sets the expression used for weighting points when generating the heatmap.
 \param expression point weight expression. If set to empty, all points are equally weighted.
.. seealso:: weightExpression
%End

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/symbology/qgsheatmaprenderer.h                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
