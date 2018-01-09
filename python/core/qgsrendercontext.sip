/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsrendercontext.h                                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/







class QgsRenderContext
{
%Docstring
Contains information about the context of a rendering operation.
The context of a rendering operation defines properties such as
the conversion ratio between screen and map units, the extents
to be rendered etc.
*
%End

%TypeHeaderCode
#include "qgsrendercontext.h"
%End
  public:
    QgsRenderContext();

    QgsRenderContext( const QgsRenderContext &rh );

    enum Flag
    {
      DrawEditingInfo,
      ForceVectorOutput,
      UseAdvancedEffects,
      UseRenderingOptimization,
      DrawSelection,
      DrawSymbolBounds,
      RenderMapTile,
      Antialiasing,
      RenderPartialOutput,
      RenderPreviewJob,
    };
    typedef QFlags<QgsRenderContext::Flag> Flags;


    void setFlags( QgsRenderContext::Flags flags );
%Docstring
Set combination of flags that will be used for rendering.

.. versionadded:: 2.14
%End

    void setFlag( Flag flag, bool on = true );
%Docstring
Enable or disable a particular flag (other flags are not affected)

.. versionadded:: 2.14
%End

    Flags flags() const;
%Docstring
Return combination of flags used for rendering.

.. versionadded:: 2.14
%End

    bool testFlag( Flag flag ) const;
%Docstring
Check whether a particular flag is enabled.

.. versionadded:: 2.14
%End

    static QgsRenderContext fromMapSettings( const QgsMapSettings &mapSettings );
%Docstring
create initialized QgsRenderContext instance from given :py:class:`QgsMapSettings`

.. versionadded:: 2.4
%End

    static QgsRenderContext fromQPainter( QPainter *painter );
%Docstring
Creates a default render context given a pixel based QPainter destination.
If no painter is specified or the painter has no device, then a default
DPI of 88 will be assumed.

.. versionadded:: 3.0
%End


    QPainter *painter();
%Docstring
Returns the destination QPainter for the render operation.

.. seealso:: :py:func:`setPainter`
%End

    QgsCoordinateTransform coordinateTransform() const;
%Docstring
Returns the current coordinate transform for the context, or an invalid
transform is no coordinate transformation is required.
%End

    const QgsDistanceArea &distanceArea() const;
%Docstring
A general purpose distance and area calculator, capable of performing ellipsoid based calculations.

.. versionadded:: 3.0
%End

    QgsCoordinateTransformContext transformContext() const;
%Docstring
Returns the context's coordinate transform context, which stores various
information regarding which datum transforms should be used when transforming points
from a source to destination coordinate reference system.

.. versionadded:: 3.0

.. seealso:: :py:func:`setTransformContext`
%End

    void setTransformContext( const QgsCoordinateTransformContext &context );
%Docstring
Sets the context's coordinate transform ``context``, which stores various
information regarding which datum transforms should be used when transforming points
from a source to destination coordinate reference system.

.. versionadded:: 3.0

.. seealso:: :py:func:`transformContext`
%End

    const QgsRectangle &extent() const;

    const QgsMapToPixel &mapToPixel() const;

    double scaleFactor() const;
%Docstring
Returns the scaling factor for the render to convert painter units
to physical sizes. This is usually equal to the number of pixels
per millimeter.

.. seealso:: :py:func:`setScaleFactor`
%End

    bool renderingStopped() const;

    bool forceVectorOutput() const;

    bool useAdvancedEffects() const;
%Docstring
Returns true if advanced effects such as blend modes such be used
%End

    void setUseAdvancedEffects( bool enabled );
%Docstring
Used to enable or disable advanced effects such as blend modes
%End

    bool drawEditingInformation() const;

    double rendererScale() const;
%Docstring
Returns the renderer map scale. This will match the desired scale denominator
for the rendered map, eg 1000.0 for a 1:1000 map render.

.. seealso:: :py:func:`setRendererScale`
%End


    QColor selectionColor() const;

    bool showSelection() const;
%Docstring
Returns true if vector selections should be shown in the rendered map

:return: true if selections should be shown

.. seealso:: :py:func:`setShowSelection`

.. seealso:: :py:func:`selectionColor`

.. versionadded:: 2.4
%End


    void setCoordinateTransform( const QgsCoordinateTransform &t );
%Docstring
Sets coordinate transformation.
%End
    void setMapToPixel( const QgsMapToPixel &mtp );
    void setExtent( const QgsRectangle &extent );

    void setDrawEditingInformation( bool b );

    void setRenderingStopped( bool stopped );

    void setDistanceArea( const QgsDistanceArea &distanceArea );
%Docstring
A general purpose distance and area calculator, capable of performing ellipsoid based calculations.
Will be used to convert meter distances to active MapUnit values for QgsUnitTypes.RenderMetersInMapUnits

.. versionadded:: 3.0
%End

    void setScaleFactor( double factor );
%Docstring
Sets the scaling factor for the render to convert painter units
to physical sizes. This should usually be equal to the number of pixels
per millimeter.

.. seealso:: :py:func:`scaleFactor`
%End

    void setRendererScale( double scale );
%Docstring
Sets the renderer map scale. This should match the desired scale denominator
for the rendered map, eg 1000.0 for a 1:1000 map render.

.. seealso:: :py:func:`rendererScale`
%End

    void setPainter( QPainter *p );
%Docstring
Sets the destination QPainter for the render operation. Ownership of the painter
is not transferred and the QPainter destination must stay alive for the duration
of any rendering operations.

.. seealso:: :py:func:`painter`
%End

    void setForceVectorOutput( bool force );

    void setSelectionColor( const QColor &color );

    void setShowSelection( const bool showSelection );
%Docstring
Sets whether vector selections should be shown in the rendered map

:param showSelection: set to true if selections should be shown

.. seealso:: :py:func:`showSelection`

.. seealso:: :py:func:`setSelectionColor`

.. versionadded:: 2.4
%End

    bool useRenderingOptimization() const;
%Docstring
Returns true if the rendering optimization (geometry simplification) can be executed
%End

    void setUseRenderingOptimization( bool enabled );

    const QgsVectorSimplifyMethod &vectorSimplifyMethod() const;
%Docstring
Added in QGIS v2.4
%End
    void setVectorSimplifyMethod( const QgsVectorSimplifyMethod &simplifyMethod );

    void setExpressionContext( const QgsExpressionContext &context );
%Docstring
Sets the expression context. This context is used for all expression evaluation
associated with this render context.

.. seealso:: :py:func:`expressionContext`

.. versionadded:: 2.12
%End

    QgsExpressionContext &expressionContext();
%Docstring
Gets the expression context. This context should be used for all expression evaluation
associated with this render context.

.. seealso:: :py:func:`setExpressionContext`

.. versionadded:: 2.12
%End


    const QgsAbstractGeometry *geometry() const;
%Docstring
Returns pointer to the unsegmentized geometry
%End
    void setGeometry( const QgsAbstractGeometry *geometry );
%Docstring
Sets pointer to original (unsegmentized) geometry
%End

    void setFeatureFilterProvider( const QgsFeatureFilterProvider *ffp );
%Docstring
Set a filter feature provider used for additional filtering of rendered features.

:param ffp: the filter feature provider

.. versionadded:: 2.14

.. seealso:: :py:func:`featureFilterProvider`
%End

    const QgsFeatureFilterProvider *featureFilterProvider() const;
%Docstring
Get the filter feature provider used for additional filtering of rendered features.

:return: the filter feature provider

.. versionadded:: 2.14

.. seealso:: :py:func:`setFeatureFilterProvider`
%End

    void setSegmentationTolerance( double tolerance );
%Docstring
Sets the segmentation tolerance applied when rendering curved geometries

:param tolerance: the segmentation tolerance*
%End
    double segmentationTolerance() const;
%Docstring
Gets the segmentation tolerance applied when rendering curved geometries
%End

    void setSegmentationToleranceType( QgsAbstractGeometry::SegmentationToleranceType type );
%Docstring
Sets segmentation tolerance type (maximum angle or maximum difference between curve and approximation)

:param type: the segmentation tolerance typename*
%End
    QgsAbstractGeometry::SegmentationToleranceType segmentationToleranceType() const;
%Docstring
Gets segmentation tolerance type (maximum angle or maximum difference between curve and approximation)
%End


    double convertToPainterUnits( double size, QgsUnitTypes::RenderUnit unit, const QgsMapUnitScale &scale = QgsMapUnitScale() ) const;
%Docstring
Converts a size from the specified units to painter units (pixels). The conversion respects the limits
specified by the optional scale parameter.

.. versionadded:: 3.0

.. seealso:: :py:func:`convertToMapUnits`
%End

    double convertToMapUnits( double size, QgsUnitTypes::RenderUnit unit, const QgsMapUnitScale &scale = QgsMapUnitScale() ) const;
%Docstring
Converts a size from the specified units to map units. The conversion respects the limits
specified by the optional scale parameter.

.. versionadded:: 3.0

.. seealso:: :py:func:`convertToPainterUnits`
%End

    double convertFromMapUnits( double sizeInMapUnits, QgsUnitTypes::RenderUnit outputUnit ) const;
%Docstring
Converts a size from map units to the specified units.

.. versionadded:: 3.0

.. seealso:: :py:func:`convertToMapUnits`
%End

    double convertMetersToMapUnits( double meters ) const;
%Docstring
Convert meter distances to active MapUnit values for QgsUnitTypes.RenderMetersInMapUnits
\note
When the sourceCrs() is geographic, the center of the Extent will be used

.. versionadded:: 3.0
%End

};

QFlags<QgsRenderContext::Flag> operator|(QgsRenderContext::Flag f1, QFlags<QgsRenderContext::Flag> f2);


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsrendercontext.h                                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
