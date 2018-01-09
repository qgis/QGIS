/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsmapsettings.h                                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/







class QgsMapSettings
{
%Docstring
The QgsMapSettings class contains configuration for rendering of the map.
The rendering itself is done by QgsMapRendererJob subclasses.

In order to set up QgsMapSettings instance, it is necessary to set at least
few members: extent, output size and layers.

QgsMapSettings and QgsMapRendererJob (+subclasses) are intended to replace
QgsMapRenderer class that existed before QGIS 2.4. The advantage of the new
classes is that they separate the settings from the rendering and provide
asynchronous API for map rendering.

.. versionadded:: 2.4
%End

%TypeHeaderCode
#include "qgsmapsettings.h"
%End
  public:
    QgsMapSettings();

    QgsRectangle extent() const;
%Docstring
Return geographical coordinates of the rectangle that should be rendered.
The actual visible extent used for rendering could be slightly different
since the given extent may be expanded in order to fit the aspect ratio
of output size. Use visibleExtent() to get the resulting extent.
%End

    void setExtent( const QgsRectangle &rect, bool magnified = true );
%Docstring
Set coordinates of the rectangle which should be rendered.
The actual visible extent used for rendering could be slightly different
since the given extent may be expanded in order to fit the aspect ratio
of output size. Use visibleExtent() to get the resulting extent.
%End

    QSize outputSize() const;
%Docstring
Return the size of the resulting map image
%End
    void setOutputSize( QSize size );
%Docstring
Set the size of the resulting map image
%End

    double rotation() const;
%Docstring
Returns the rotation of the resulting map image, in degrees clockwise.

.. versionadded:: 2.8

.. seealso:: :py:func:`setRotation`
%End

    void setRotation( double rotation );
%Docstring
Sets the ``rotation`` of the resulting map image, in degrees clockwise.

.. versionadded:: 2.8

.. seealso:: :py:func:`rotation`
%End

    double outputDpi() const;
%Docstring
Return DPI used for conversion between real world units (e.g. mm) and pixels
Default value is 96
%End
    void setOutputDpi( double dpi );
%Docstring
Set DPI used for conversion between real world units (e.g. mm) and pixels
%End

    void setMagnificationFactor( double factor );
%Docstring
Set the magnification factor.

:param factor: the factor of magnification

.. versionadded:: 2.16

.. seealso:: :py:func:`magnificationFactor`
%End

    double magnificationFactor() const;
%Docstring
Return the magnification factor.

.. versionadded:: 2.16

.. seealso:: :py:func:`setMagnificationFactor`
%End

    QStringList layerIds() const;
%Docstring
Get list of layer IDs for map rendering
The layers are stored in the reverse order of how they are rendered (layer with index 0 will be on top)
%End

    QList<QgsMapLayer *> layers() const;
%Docstring
Get list of layers for map rendering
The layers are stored in the reverse order of how they are rendered (layer with index 0 will be on top)
%End

    void setLayers( const QList<QgsMapLayer *> &layers );
%Docstring
Set list of layers for map rendering. The layers must be registered in :py:class:`QgsProject`.
The layers are stored in the reverse order of how they are rendered (layer with index 0 will be on top)
%End

    QMap<QString, QString> layerStyleOverrides() const;
%Docstring
Get map of map layer style overrides (key: layer ID, value: style name) where a different style should be used instead of the current one

.. versionadded:: 2.8
%End

    void setLayerStyleOverrides( const QMap<QString, QString> &overrides );
%Docstring
Set map of map layer style overrides (key: layer ID, value: style name) where a different style should be used instead of the current one

.. versionadded:: 2.8
%End

    QString customRenderFlags() const;
%Docstring
Get custom rendering flags. Layers might honour these to alter their rendering.

:return: custom flags strings, separated by ';'

.. versionadded:: 2.16

.. seealso:: :py:func:`setCustomRenderFlags`
%End

    void setCustomRenderFlags( const QString &customRenderFlags );
%Docstring
Sets the custom rendering flags. Layers might honour these to alter their rendering.

:param customRenderFlags: custom flags strings, separated by ';'

.. versionadded:: 2.16

.. seealso:: :py:func:`customRenderFlags`
%End

    void setDestinationCrs( const QgsCoordinateReferenceSystem &crs );
%Docstring
sets destination coordinate reference system
%End
    QgsCoordinateReferenceSystem destinationCrs() const;
%Docstring
returns CRS of destination coordinate reference system
%End

    QgsUnitTypes::DistanceUnit mapUnits() const;
%Docstring
Get units of map's geographical coordinates - used for scale calculation
%End

    bool setEllipsoid( const QString &ellipsoid );
%Docstring
Sets the ``ellipsoid`` by its acronym. Known ellipsoid acronyms can be
retrieved using :py:func:`QgsEllipsoidUtils.acronyms()`
Calculations will only use the ellipsoid if a valid ellipsoid has been set.

:return: true if ellipsoid was successfully set

.. versionadded:: 3.0

.. seealso:: :py:func:`ellipsoid`
%End

    QString ellipsoid() const;
%Docstring
Returns ellipsoid's acronym. Calculations will only use the
ellipsoid if a valid ellipsoid has been set.

.. versionadded:: 3.0

.. seealso:: :py:func:`setEllipsoid`
%End

    void setBackgroundColor( const QColor &color );
%Docstring
Set the background color of the map
%End
    QColor backgroundColor() const;
%Docstring
Get the background color of the map
%End

    void setSelectionColor( const QColor &color );
%Docstring
Set color that is used for drawing of selected vector features
%End
    QColor selectionColor() const;
%Docstring
Get color that is used for drawing of selected vector features
%End

    enum Flag
    {
      Antialiasing,
      DrawEditingInfo,
      ForceVectorOutput,
      UseAdvancedEffects,
      DrawLabeling,
      UseRenderingOptimization,
      DrawSelection,
      DrawSymbolBounds,
      RenderMapTile,
      RenderPartialOutput,
      RenderPreviewJob,
      // TODO
    };
    typedef QFlags<QgsMapSettings::Flag> Flags;


    void setFlags( QgsMapSettings::Flags flags );
%Docstring
Set combination of flags that will be used for rendering
%End
    void setFlag( Flag flag, bool on = true );
%Docstring
Enable or disable a particular flag (other flags are not affected)
%End
    Flags flags() const;
%Docstring
Return combination of flags used for rendering
%End
    bool testFlag( Flag flag ) const;
%Docstring
Check whether a particular flag is enabled
%End

    void setOutputImageFormat( QImage::Format format );
%Docstring
sets format of internal QImage
%End
    QImage::Format outputImageFormat() const;
%Docstring
format of internal QImage, default QImage.Format_ARGB32_Premultiplied
%End

    bool hasValidSettings() const;
%Docstring
Check whether the map settings are valid and can be used for rendering
%End
    QgsRectangle visibleExtent() const;
%Docstring
Return the actual extent derived from requested extent that takes takes output image size into account
%End

    QPolygonF visiblePolygon() const;
%Docstring
Return the visible area as a polygon (may be rotated)

.. versionadded:: 2.8
%End
    double mapUnitsPerPixel() const;
%Docstring
Return the distance in geographical coordinates that equals to one pixel in the map
%End

    double scale() const;
%Docstring
Returns the calculated map scale.
The scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
%End

    void setExpressionContext( const QgsExpressionContext &context );
%Docstring
Sets the expression context. This context is used for all expression evaluation
associated with this map settings.

.. seealso:: :py:func:`expressionContext`

.. versionadded:: 2.12
%End

    const QgsExpressionContext &expressionContext() const;
%Docstring
Gets the expression context. This context should be used for all expression evaluation
associated with this map settings.

.. seealso:: :py:func:`setExpressionContext`

.. versionadded:: 2.12
%End

    QgsCoordinateTransformContext transformContext() const;
%Docstring
Returns the coordinate transform context, which stores various
information regarding which datum transforms should be used when transforming points
from a source to destination coordinate reference system.

.. versionadded:: 3.0

.. seealso:: :py:func:`setTransformContext`
%End

    void setTransformContext( const QgsCoordinateTransformContext &context );
%Docstring
Sets the coordinate transform ``context``, which stores various
information regarding which datum transforms should be used when transforming points
from a source to destination coordinate reference system.

.. versionadded:: 3.0

.. seealso:: :py:func:`transformContext`
%End

    const QgsMapToPixel &mapToPixel() const;

    double layerToMapUnits( const QgsMapLayer *layer, const QgsRectangle &referenceExtent = QgsRectangle() ) const;
%Docstring
Computes an *estimated* conversion factor between layer and map units: layerUnits * layerToMapUnits = mapUnits

:param layer: The layer
:param referenceExtent: A reference extent based on which to perform the computation. If not specified, the layer extent is used

.. versionadded:: 2.12
%End

    QgsRectangle layerExtentToOutputExtent( const QgsMapLayer *layer, QgsRectangle extent ) const;
%Docstring
 transform bounding box from layer's CRS to output CRS

.. seealso:: :py:func:`layerToMapCoordinates`

:return: a bounding box (aligned rectangle) containing the transformed extent
%End

    QgsRectangle outputExtentToLayerExtent( const QgsMapLayer *layer, QgsRectangle extent ) const;
%Docstring
 transform bounding box from output CRS to layer's CRS

.. seealso:: :py:func:`mapToLayerCoordinates`

:return: a bounding box (aligned rectangle) containing the transformed extent
%End

    QgsPointXY layerToMapCoordinates( const QgsMapLayer *layer, QgsPointXY point ) const;
%Docstring
 transform point coordinates from layer's CRS to output CRS

:return: the transformed point
%End

    QgsRectangle layerToMapCoordinates( const QgsMapLayer *layer, QgsRectangle rect ) const;
%Docstring
 transform rectangle from layer's CRS to output CRS

.. seealso:: :py:func:`layerExtentToOutputExtent`

:return: the transformed rectangle
%End

    QgsPointXY mapToLayerCoordinates( const QgsMapLayer *layer, QgsPointXY point ) const;
%Docstring
 transform point coordinates from output CRS to layer's CRS

:return: the transformed point
%End

    QgsRectangle mapToLayerCoordinates( const QgsMapLayer *layer, QgsRectangle rect ) const;
%Docstring
 transform rectangle from output CRS to layer's CRS

.. seealso:: :py:func:`outputExtentToLayerExtent`

:return: the transformed rectangle
%End

    QgsCoordinateTransform layerTransform( const QgsMapLayer *layer ) const;
%Docstring
 Return coordinate transform from layer's CRS to destination CRS

:param layer:

:return: transform - may be invalid if the transform is not needed
%End

    QgsRectangle fullExtent() const;
%Docstring
returns current extent of layer set
%End


    void readXml( QDomNode &node );
%Docstring
serialization */
%End

    void writeXml( QDomNode &node, QDomDocument &doc );

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

    void setLabelingEngineSettings( const QgsLabelingEngineSettings &settings );
%Docstring
Sets global configuration of the labeling engine

.. versionadded:: 3.0
%End

    const QgsLabelingEngineSettings &labelingEngineSettings() const;
%Docstring
Returns global configuration of the labeling engine

.. versionadded:: 3.0
%End

  protected:















    void updateDerived();
};

QFlags<QgsMapSettings::Flag> operator|(QgsMapSettings::Flag f1, QFlags<QgsMapSettings::Flag> f2);



/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsmapsettings.h                                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
