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


    enum TextRenderFormat
    {
      // refs for below dox: https://github.com/qgis/QGIS/pull/1286#issuecomment-39806854
      // https://github.com/qgis/QGIS/pull/8573#issuecomment-445585826

      TextFormatAlwaysOutlines,

      TextFormatAlwaysText,
    };

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
Returns combination of flags used for rendering.

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
Returns the current coordinate transform for the context.

This represents the coordinate transform required to transform a layer
which is being rendered back to the CRS of the rendered map. If no coordinate
transformation is required, or the render context is not associated with
a map layer render, then an invalid coordinate transformation is returned.

.. seealso:: :py:func:`setCoordinateTransform`
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

.. seealso:: :py:func:`setTransformContext`

.. versionadded:: 3.0
%End

    void setTransformContext( const QgsCoordinateTransformContext &context );
%Docstring
Sets the context's coordinate transform ``context``, which stores various
information regarding which datum transforms should be used when transforming points
from a source to destination coordinate reference system.

.. seealso:: :py:func:`transformContext`

.. versionadded:: 3.0
%End

    const QgsPathResolver &pathResolver() const;
%Docstring
Returns the path resolver for conversion between relative and absolute paths
during rendering operations, e.g. for resolving relative symbol paths.

.. seealso:: :py:func:`setPathResolver`

.. versionadded:: 3.0
%End

    void setPathResolver( const QgsPathResolver &resolver );
%Docstring
Sets the path ``resolver`` for conversion between relative and absolute paths
during rendering operations, e.g. for resolving relative symbol paths.

.. seealso:: :py:func:`pathResolver`

.. versionadded:: 3.0
%End

    const QgsRectangle &extent() const;
%Docstring
When rendering a map layer, calling this method returns the "clipping"
extent for the layer (in the layer's CRS).

This extent is a "worst-case" scenario, which is guaranteed to cover the complete
visible portion of the layer when it is rendered to a map. It is often larger
than the actual visible portion of that layer.

.. warning::

   For some layers, depending on the visible extent and the coordinate
   transforms involved, this extent will represent the entire globe. This method
   should never be used to determine the actual visible extent of a map render.

.. seealso:: :py:func:`setExtent`
%End

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
Returns ``True`` if advanced effects such as blend modes such be used
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
Returns ``True`` if vector selections should be shown in the rendered map

:return: ``True`` if selections should be shown

.. seealso:: :py:func:`setShowSelection`

.. seealso:: :py:func:`selectionColor`

.. versionadded:: 2.4
%End


    void setCoordinateTransform( const QgsCoordinateTransform &t );
%Docstring
Sets the current coordinate transform for the context.

This represents the coordinate transform required to transform the layer
which is being rendered back to the CRS of the rendered map.

Set to an invalid QgsCoordinateTransform to indicate that no transformation is required.

.. seealso:: :py:func:`coordinateTransform`
%End

    void setMapToPixel( const QgsMapToPixel &mtp );

    void setExtent( const QgsRectangle &extent );
%Docstring
When rendering a map layer, calling this method sets the "clipping"
extent for the layer (in the layer's CRS).

This extent should be a "worst-case" scenario, which is guaranteed to
completely cover the entire visible portion of the layer when it is rendered
to the map. It may be larger than the actual visible area, but MUST contain at least the
entire visible area.

.. seealso:: :py:func:`setExtent`
%End

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

    void setShowSelection( bool showSelection );
%Docstring
Sets whether vector selections should be shown in the rendered map

:param showSelection: set to ``True`` if selections should be shown

.. seealso:: :py:func:`showSelection`

.. seealso:: :py:func:`setSelectionColor`

.. versionadded:: 2.4
%End

    bool useRenderingOptimization() const;
%Docstring
Returns ``True`` if the rendering optimization (geometry simplification) can be executed
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

.. seealso:: :py:func:`featureFilterProvider`

.. versionadded:: 2.14
%End

    const QgsFeatureFilterProvider *featureFilterProvider() const;
%Docstring
Gets the filter feature provider used for additional filtering of rendered features.

:return: the filter feature provider

.. seealso:: :py:func:`setFeatureFilterProvider`

.. versionadded:: 2.14
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

.. seealso:: :py:func:`convertToMapUnits`

.. versionadded:: 3.0
%End

    double convertToMapUnits( double size, QgsUnitTypes::RenderUnit unit, const QgsMapUnitScale &scale = QgsMapUnitScale() ) const;
%Docstring
Converts a size from the specified units to map units. The conversion respects the limits
specified by the optional scale parameter.

.. seealso:: :py:func:`convertToPainterUnits`

.. versionadded:: 3.0
%End

    double convertFromMapUnits( double sizeInMapUnits, QgsUnitTypes::RenderUnit outputUnit ) const;
%Docstring
Converts a size from map units to the specified units.

.. seealso:: :py:func:`convertToMapUnits`

.. versionadded:: 3.0
%End

    double convertMetersToMapUnits( double meters ) const;
%Docstring
Convert meter distances to active MapUnit values for QgsUnitTypes.RenderMetersInMapUnits
\note
When the sourceCrs() is geographic, the center of the Extent will be used

.. versionadded:: 3.0
%End

    TextRenderFormat textRenderFormat() const;
%Docstring
Returns the text render format, which dictates how text is rendered (e.g. as paths or real text objects).

.. seealso:: :py:func:`setTextRenderFormat`

.. versionadded:: 3.4.3
%End

    void setTextRenderFormat( TextRenderFormat format );
%Docstring
Sets the text render ``format``, which dictates how text is rendered (e.g. as paths or real text objects).

.. seealso:: :py:func:`textRenderFormat`

.. versionadded:: 3.4.3
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
