/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/symbology/qgspointdistancerenderer.h                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsPointDistanceRenderer: QgsFeatureRenderer
{
%Docstring
An abstract base class for distance based point renderers (e.g., clusterer and displacement renderers).
QgsPointDistanceRenderer handles calculation of point clusters using a distance based threshold.
Subclasses must implement drawGroup() to handle the rendering of individual point clusters
in the desired style.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgspointdistancerenderer.h"
%End
  public:

    struct GroupedFeature
    {

        GroupedFeature( const QgsFeature &feature, QgsMarkerSymbol *symbol /Transfer/, bool isSelected, const QString &label = QString() );
%Docstring
Constructor for GroupedFeature.

:param feature: feature
:param symbol: base symbol for rendering feature (owned by GroupedFeature)
:param isSelected: set to true if feature is selected and should be rendered in a selected state
:param label: optional label text, or empty string for no label
%End

        QgsFeature feature;

        QgsMarkerSymbol *symbol() const;
%Docstring
Base symbol for rendering feature
%End

        bool isSelected;

        QString label;

    };

    typedef QList< QgsPointDistanceRenderer::GroupedFeature > ClusteredGroup;

    QgsPointDistanceRenderer( const QString &rendererName, const QString &labelAttributeName = QString() );
%Docstring
Constructor for QgsPointDistanceRenderer.

:param rendererName: name of renderer for registry
:param labelAttributeName: optional attribute for labeling points
%End

    virtual void toSld( QDomDocument &doc, QDomElement &element, const QgsStringMap &props = QgsStringMap() ) const;

    virtual bool renderFeature( QgsFeature &feature, QgsRenderContext &context, int layer = -1, bool selected = false, bool drawVertexMarker = false );

    virtual QSet<QString> usedAttributes( const QgsRenderContext &context ) const;

    virtual QgsFeatureRenderer::Capabilities capabilities();

    virtual QgsSymbolList symbols( QgsRenderContext &context );

    virtual QgsSymbol *symbolForFeature( QgsFeature &feature, QgsRenderContext &context );

    virtual QgsSymbol *originalSymbolForFeature( QgsFeature &feat, QgsRenderContext &context );

    virtual QgsSymbolList symbolsForFeature( QgsFeature &feat, QgsRenderContext &context );

    virtual QgsSymbolList originalSymbolsForFeature( QgsFeature &feat, QgsRenderContext &context );

    virtual QSet< QString > legendKeysForFeature( QgsFeature &feature, QgsRenderContext &context );

    virtual bool willRenderFeature( QgsFeature &feat, QgsRenderContext &context );

    virtual void startRender( QgsRenderContext &context, const QgsFields &fields );

    virtual void stopRender( QgsRenderContext &context );

    virtual QgsLegendSymbolList legendSymbolItems() const;

    virtual void setEmbeddedRenderer( QgsFeatureRenderer *r /Transfer/ );

    virtual const QgsFeatureRenderer *embeddedRenderer() const;

    virtual void setLegendSymbolItem( const QString &key, QgsSymbol *symbol /Transfer/ );

    virtual bool legendSymbolItemsCheckable() const;

    virtual bool legendSymbolItemChecked( const QString &key );

    virtual void checkLegendSymbolItem( const QString &key, bool state );

    virtual QString filter( const QgsFields &fields = QgsFields() );


    void setLabelAttributeName( const QString &name );
%Docstring
Sets the attribute name for labeling points.

:param name: attribute name, or empty string to avoid labeling features by the renderer

.. seealso:: :py:func:`labelAttributeName`

.. seealso:: :py:func:`setLabelFont`

.. seealso:: :py:func:`setLabelColor`

.. seealso:: :py:func:`setMinimumLabelScale`
%End

    QString labelAttributeName() const;
%Docstring
Returns the attribute name used for labeling points, or an empty string if no labeling
will be done by the renderer.

.. seealso:: :py:func:`setLabelAttributeName`

.. seealso:: :py:func:`labelFont`

.. seealso:: :py:func:`minimumLabelScale`

.. seealso:: :py:func:`labelColor`
%End

    void setLabelFont( const QFont &font );
%Docstring
Sets the font used for labeling points.

:param font: label font

.. seealso:: :py:func:`labelFont`

.. seealso:: :py:func:`setLabelAttributeName`

.. seealso:: :py:func:`setLabelColor`
%End

    QFont labelFont() const;
%Docstring
Returns the font used for labeling points.

.. seealso:: :py:func:`setLabelFont`

.. seealso:: :py:func:`labelAttributeName`

.. seealso:: :py:func:`labelColor`
%End

    void setMinimumLabelScale( double scale );
%Docstring
Sets the minimum map ``scale`` (i.e. most "zoomed out") at which points should be labeled by the renderer.
The ``scale`` value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.

.. seealso:: :py:func:`minimumLabelScale`

.. seealso:: :py:func:`setLabelAttributeName`
%End

    double minimumLabelScale() const;
%Docstring
Returns the minimum map scale (i.e. most "zoomed out") at which points should be labeled by the renderer.
The scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.

.. seealso:: :py:func:`setMinimumLabelScale`

.. seealso:: :py:func:`labelAttributeName`
%End

    void setLabelColor( const QColor &color );
%Docstring
Sets the color to use for for labeling points.

:param color: label color

.. seealso:: :py:func:`labelColor`

.. seealso:: :py:func:`setLabelAttributeName`

.. seealso:: :py:func:`setLabelFont`
%End

    QColor labelColor() const;
%Docstring
Returns the color used for for labeling points.

.. seealso:: :py:func:`setLabelColor`

.. seealso:: :py:func:`labelAttributeName`

.. seealso:: :py:func:`labelFont`
%End

    void setTolerance( double distance );
%Docstring
Sets the tolerance distance for grouping points. Units are specified using
setToleranceUnit().

:param distance: tolerance distance

.. seealso:: :py:func:`tolerance`

.. seealso:: :py:func:`setToleranceUnit`
%End

    double tolerance() const;
%Docstring
Returns the tolerance distance for grouping points. Units are retrieved using
toleranceUnit().

.. seealso:: :py:func:`setTolerance`

.. seealso:: :py:func:`toleranceUnit`
%End

    void setToleranceUnit( QgsUnitTypes::RenderUnit unit );
%Docstring
Sets the units for the tolerance distance.

:param unit: tolerance distance units

.. seealso:: :py:func:`setTolerance`

.. seealso:: :py:func:`toleranceUnit`

.. versionadded:: 2.12
%End

    QgsUnitTypes::RenderUnit toleranceUnit() const;
%Docstring
Returns the units for the tolerance distance.

.. seealso:: :py:func:`tolerance`

.. seealso:: :py:func:`setToleranceUnit`

.. versionadded:: 2.12
%End

    void setToleranceMapUnitScale( const QgsMapUnitScale &scale );
%Docstring
Sets the map unit scale object for the distance tolerance. This is only used if the
toleranceUnit() is set to QgsUnitTypes.RenderMapUnits.

:param scale: scale for distance tolerance

.. seealso:: :py:func:`toleranceMapUnitScale`

.. seealso:: :py:func:`setToleranceUnit`
%End

    const QgsMapUnitScale &toleranceMapUnitScale() const;
%Docstring
Returns the map unit scale object for the distance tolerance. This is only used if the
toleranceUnit() is set to QgsUnitTypes.RenderMapUnits.

.. seealso:: :py:func:`setToleranceMapUnitScale`

.. seealso:: :py:func:`toleranceUnit`
%End

  protected:










    void drawLabels( QPointF centerPoint, QgsSymbolRenderContext &context, const QList<QPointF> &labelShifts, const ClusteredGroup &group );
%Docstring
Renders the labels for a group.

:param centerPoint: center point of group
:param context: destination render context
:param labelShifts: displacement for individual label positions
:param group: group of clustered features to label

.. note::

   may not be available in Python bindings on some platforms
%End

  private:
    virtual void drawGroup( QPointF centerPoint, QgsRenderContext &context, const ClusteredGroup &group ) = 0 ;
%Docstring
Draws a group of clustered points.

:param centerPoint: central point (geographic centroid) of all points contained within the cluster
:param context: destination render context
:param group: contents of group
%End
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/symbology/qgspointdistancerenderer.h                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
