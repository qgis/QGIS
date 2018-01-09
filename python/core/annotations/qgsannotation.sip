/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/annotations/qgsannotation.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsAnnotation : QObject
{
%Docstring

.. versionadded:: 3.0

 Abstract base class for annotation items which are drawn over a map.

QgsAnnotation is an abstract base class for map annotation items. These annotations can be
drawn within a map, and have either a fixed map position (retrieved using mapPosition())
or are placed relative to the map's frame (retrieved using relativePosition()).
Annotations with a fixed map position also have a corresponding
QgsCoordinateReferenceSystem, which can be determined by calling mapPositionCrs().

Derived classes should implement their custom painting routines within
a renderAnnotation() override.
%End

%TypeHeaderCode
#include "qgsannotation.h"
%End
%ConvertToSubClassCode
    if ( dynamic_cast< QgsTextAnnotation * >( sipCpp ) )
      sipType = sipType_QgsTextAnnotation;
    else if ( dynamic_cast< QgsSvgAnnotation * >( sipCpp ) )
      sipType = sipType_QgsSvgAnnotation;
    else if ( dynamic_cast< QgsHtmlAnnotation * >( sipCpp ) )
      sipType = sipType_QgsHtmlAnnotation;
    else
      sipType = NULL;
%End
  public:

    QgsAnnotation( QObject *parent /TransferThis/ = 0 );
%Docstring
Constructor for QgsAnnotation.
%End

    virtual QgsAnnotation *clone() const = 0 /Factory/;
%Docstring
Clones the annotation, returning a new copy of the annotation
reflecting the annotation's current state.
%End

    bool isVisible() const;
%Docstring
Returns true if the annotation is visible and should be rendered.

.. seealso:: :py:func:`setVisible`
%End

    void setVisible( bool visible );
%Docstring
Sets whether the annotation is visible and should be rendered.

.. seealso:: :py:func:`isVisible`
%End

    bool hasFixedMapPosition() const;
%Docstring
Returns true if the annotation is attached to a fixed map position, or
false if the annotation uses a position relative to the current map
extent.

.. seealso:: :py:func:`setHasFixedMapPosition`

.. seealso:: :py:func:`mapPosition`

.. seealso:: :py:func:`relativePosition`
%End

    void setHasFixedMapPosition( bool fixed );
%Docstring
Sets whether the annotation is attached to a fixed map position, or
uses a position relative to the current map extent.

.. seealso:: :py:func:`hasFixedMapPosition`
%End

    QgsPointXY mapPosition() const;
%Docstring
Returns the map position of the annotation, if it is attached to a fixed map
position.

.. seealso:: :py:func:`setMapPosition`

.. seealso:: :py:func:`hasFixedMapPosition`

.. seealso:: :py:func:`mapPositionCrs`
%End

    void setMapPosition( const QgsPointXY &position );
%Docstring
Sets the map position of the annotation, if it is attached to a fixed map
position.

.. seealso:: :py:func:`mapPosition`
%End

    QgsCoordinateReferenceSystem mapPositionCrs() const;
%Docstring
Returns the CRS of the map position, or an invalid CRS if the annotation does
not have a fixed map position.

.. seealso:: :py:func:`setMapPositionCrs`
%End

    void setMapPositionCrs( const QgsCoordinateReferenceSystem &crs );
%Docstring
Sets the CRS of the map position.

.. seealso:: :py:func:`mapPositionCrs`
%End

    QPointF relativePosition() const;
%Docstring
Returns the relative position of the annotation, if it is not attached to a fixed map
position. The coordinates in the return point should be between 0 and 1, and represent
the relative percentage for the position compared to the map width and height.

.. seealso:: :py:func:`setRelativePosition`
%End

    void setRelativePosition( QPointF position );
%Docstring
Sets the relative position of the annotation, if it is not attached to a fixed map
position. The coordinates in the return point should be between 0 and 1, and represent
the relative percentage for the position compared to the map width and height.

.. seealso:: :py:func:`relativePosition`
%End

    void setFrameOffsetFromReferencePoint( QPointF offset );
%Docstring
Sets the annotation's frame's offset from the mapPosition() reference point.

.. seealso:: :py:func:`frameOffsetFromReferencePoint`
%End

    QPointF frameOffsetFromReferencePoint() const;
%Docstring
Returns the annotation's frame's offset from the mapPosition() reference point.

.. seealso:: :py:func:`setFrameOffsetFromReferencePoint`
%End

    void setFrameSize( QSizeF size );
%Docstring
Sets the size of the annotation's frame (the main area in which
the annotation's content is drawn).

.. seealso:: :py:func:`frameSize`
%End

    QSizeF frameSize() const;
%Docstring
Returns the size of the annotation's frame (the main area in which
the annotation's content is drawn).

.. seealso:: :py:func:`setFrameSize`
%End

    void setContentsMargin( const QgsMargins &margins );
%Docstring
Sets the margins (in millimeters) between the outside of the frame and the annotation
content.

.. seealso:: :py:func:`contentsMargin`
%End

    QgsMargins contentsMargin() const;
%Docstring
Returns the margins (in millimeters) between the outside of the frame and the annotation
content.

.. seealso:: :py:func:`setContentsMargin`
%End

    void setFillSymbol( QgsFillSymbol *symbol /Transfer/ );
%Docstring
Sets the fill symbol used for rendering the annotation frame. Ownership
of the symbol is transferred to the annotation.

.. seealso:: :py:func:`fillSymbol`
%End

    QgsFillSymbol *fillSymbol() const;
%Docstring
Returns the symbol that is used for rendering the annotation frame.

.. seealso:: :py:func:`setFillSymbol`
%End

    void render( QgsRenderContext &context ) const;
%Docstring
Renders the annotation to a target render context.
%End

    virtual void writeXml( QDomElement &elem, QDomDocument &doc, const QgsReadWriteContext &context ) const = 0;
%Docstring
Writes the annotation state to a DOM element. Derived classes should
call _writeXml() within their implementation of this method.

.. seealso:: :py:func:`readXml`

.. seealso:: _writeXml
%End

    virtual void readXml( const QDomElement &itemElem, const QgsReadWriteContext &context ) = 0;
%Docstring
Restores the annotation's state from a DOM element. Derived classes should
call _readXml() within their implementation of this method.

.. seealso:: :py:func:`writeXml`

.. seealso:: _readXml
%End

    void setMarkerSymbol( QgsMarkerSymbol *symbol /Transfer/ );
%Docstring
Sets the symbol that is drawn at the annotation's map position. Ownership
of the symbol is transferred to the annotation.

.. seealso:: :py:func:`markerSymbol`
%End

    QgsMarkerSymbol *markerSymbol() const;
%Docstring
Returns the symbol that is drawn at the annotation's map position.

.. seealso:: :py:func:`setMarkerSymbol`
%End

    QgsMapLayer *mapLayer() const;
%Docstring
Returns the map layer associated with the annotation. Annotations can be
associated with a map layer if their visibility should be synchronized
with the layer's visibility.

.. seealso:: :py:func:`setMapLayer`
%End

    void setMapLayer( QgsMapLayer *layer );
%Docstring
Sets the map layer associated with the annotation. Annotations can be
associated with a map layer if their visibility should be synchronized
with the layer's visibility.

.. seealso:: :py:func:`mapLayer`
%End

    QgsFeature associatedFeature() const;
%Docstring
Returns the feature associated with the annotation, or an invalid
feature if none has been set.

.. seealso:: :py:func:`setAssociatedFeature`
%End

    virtual void setAssociatedFeature( const QgsFeature &feature );
%Docstring
Sets the feature associated with the annotation.

.. seealso:: :py:func:`associatedFeature`
%End

  signals:

    void appearanceChanged();
%Docstring
Emitted whenever the annotation's appearance changes
%End

    void moved();
%Docstring
Emitted when the annotation's position has changed and items need
to be moved to reflect this.
%End

    void mapLayerChanged();
%Docstring
Emitted when the map layer associated with the annotation changes.
%End

  protected:

    virtual void renderAnnotation( QgsRenderContext &context, QSizeF size ) const = 0;
%Docstring
Renders the annotation's contents to a target /a context at the specified /a size.
Derived classes should implement their custom annotation drawing logic here.
%End

    virtual QSizeF minimumFrameSize() const;
%Docstring
Returns the minimum frame size for the annotation. Subclasses should implement this if they
cannot be resized smaller than a certain minimum size.
%End

    void _writeXml( QDomElement &itemElem, QDomDocument &doc, const QgsReadWriteContext &context ) const;
%Docstring
Writes common annotation properties to a DOM element.
This method should be called from subclasses in their writeXml method.

.. seealso:: :py:func:`writeXml`

.. seealso:: _readXml
%End

    void _readXml( const QDomElement &annotationElem, const QgsReadWriteContext &context );
%Docstring
Reads common annotation properties from a DOM element.
This method should be called from subclasses in their readXml method.

.. seealso:: :py:func:`readXml`

.. seealso:: _writeXml
%End

    void copyCommonProperties( QgsAnnotation *target ) const;
%Docstring
Copies common annotation properties to the ``targe``
annotation.
Can be used within QgsAnnotation.clone() implementations
to assist with creating copies.
%End

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/annotations/qgsannotation.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
