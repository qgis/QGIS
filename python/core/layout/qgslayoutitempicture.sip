/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutitempicture.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsLayoutItemPicture: QgsLayoutItem
{
%Docstring
A layout item subclass that displays SVG files or raster format images (jpg, png, ...).

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutitempicture.h"
%End
  public:

    enum ResizeMode
    {
      Zoom,
      Stretch,
      Clip,
      ZoomResizeFrame,
      FrameToImageSize
    };

    enum Format
    {
      FormatSVG,
      FormatRaster,
      FormatUnknown,
    };

    enum NorthMode
    {
      GridNorth,
      TrueNorth,
    };

    QgsLayoutItemPicture( QgsLayout *layout );
%Docstring
Constructor for QgsLayoutItemPicture, with the specified parent ``layout``.
%End

    virtual int type() const;

    virtual QIcon icon() const;


    static QgsLayoutItemPicture *create( QgsLayout *layout ) /Factory/;
%Docstring
Returns a new picture item for the specified ``layout``.

The caller takes responsibility for deleting the returned object.
%End


    void setPicturePath( const QString &path );
%Docstring
Sets the source ``path`` of the image (may be svg or a raster format). Data defined
picture source may override this value. The path can either be a local path
or a remote (http) path.

.. seealso:: :py:func:`picturePath`
%End

    QString picturePath() const;
%Docstring
Returns the path of the source image. Data defined picture source may override
this value. The path can either be a local path or a remote (http) path.

:return: path for the source image

.. seealso:: :py:func:`usePictureExpression`

.. seealso:: :py:func:`setPicturePath`
%End

    double pictureRotation() const;
%Docstring
Returns the rotation used for drawing the picture within the item's frame,
in degrees clockwise.

.. seealso:: :py:func:`setPictureRotation`

.. seealso:: :py:func:`rotationMap`
%End

    void setLinkedMap( QgsLayoutItemMap *map );
%Docstring
Sets the ``map`` object for rotation.

If this is set then the picture will be rotated by the same
amount as the specified map object. This is useful especially for
syncing north arrows with a map item.

.. seealso:: :py:func:`setPictureRotation`

.. seealso:: :py:func:`linkedMap`
%End

    QgsLayoutItemMap *linkedMap() const;
%Docstring
Returns the linked rotation map, if set. An None means map rotation is
disabled.  If this is set then the picture is rotated by the same amount
as the specified map object.

.. seealso:: :py:func:`setLinkedMap`
%End

    NorthMode northMode() const;
%Docstring
Returns the mode used to align the picture to a map's North.

.. seealso:: :py:func:`setNorthMode`

.. seealso:: :py:func:`northOffset`
%End

    void setNorthMode( NorthMode mode );
%Docstring
Sets the ``mode`` used to align the picture to a map's North.

.. seealso:: :py:func:`northMode`

.. seealso:: :py:func:`setNorthOffset`
%End

    double northOffset() const;
%Docstring
Returns the offset added to the picture's rotation from a map's North.

.. seealso:: :py:func:`setNorthOffset`

.. seealso:: :py:func:`northMode`
%End

    void setNorthOffset( double offset );
%Docstring
Sets the ``offset`` added to the picture's rotation from a map's North.

.. seealso:: :py:func:`northOffset`

.. seealso:: :py:func:`setNorthMode`
%End

    ResizeMode resizeMode() const;
%Docstring
Returns the resize mode used for drawing the picture within the composer
item's frame.

.. seealso:: :py:func:`setResizeMode`
%End

    void setPictureAnchor( QgsLayoutItem::ReferencePoint anchor );
%Docstring
Sets the picture's ``anchor`` point, which controls how it is placed
within the picture item's frame.

.. seealso:: :py:func:`pictureAnchor`
%End

    QgsLayoutItem::ReferencePoint pictureAnchor() const;
%Docstring
Returns the picture's current anchor, which controls how it is placed
within the picture item's frame.

.. seealso:: :py:func:`setPictureAnchor`
%End

    QColor svgFillColor() const;
%Docstring
Returns the fill color used for parametrized SVG files.

.. seealso:: :py:func:`setSvgFillColor`

.. seealso:: :py:func:`svgStrokeColor`
%End

    void setSvgFillColor( const QColor &color );
%Docstring
Sets the fill ``color`` used for parametrized SVG files.

.. note::

   This setting only has an effect on parametrized SVG files, and is ignored for
non-parametrized SVG files.

.. seealso:: :py:func:`svgFillColor`

.. seealso:: :py:func:`setSvgStrokeColor`
%End

    QColor svgStrokeColor() const;
%Docstring
Returns the stroke color used for parametrized SVG files.

.. seealso:: :py:func:`setSvgStrokeColor`

.. seealso:: :py:func:`svgFillColor`
%End

    void setSvgStrokeColor( const QColor &color );
%Docstring
Sets the stroke ``color`` used for parametrized SVG files.

:param color: stroke color.

.. note::

   This setting only has an effect on parametrized SVG files, and is ignored for
non-parametrized SVG files.

.. seealso:: :py:func:`svgStrokelColor`

.. seealso:: :py:func:`setSvgFillColor`
%End

    double svgStrokeWidth() const;
%Docstring
Returns the stroke width (in layout units) used for parametrized SVG files.

.. seealso:: :py:func:`setSvgStrokeWidth`

.. seealso:: :py:func:`svgStrokeColor`
%End

    void setSvgStrokeWidth( double width );
%Docstring
Sets the stroke ``width`` (in layout units) used for parametrized SVG files.

.. note::

   This setting only has an effect on parametrized SVG files, and is ignored for
non-parametrized SVG files.

.. seealso:: :py:func:`svgStrokeWidth`

.. seealso:: :py:func:`setSvgStrokeColor`
%End

    Format mode() const;
%Docstring
Returns the current picture mode (image format).
%End

    virtual void finalizeRestoreFromXml();


  public slots:

    void setPictureRotation( double rotation );
%Docstring
Sets the picture ``rotation`` within the item bounds, in degrees clockwise. This does not affect
the item's frame, only the way the picture is drawn within the item.

.. seealso:: :py:func:`pictureRotation`
%End

    void setResizeMode( ResizeMode mode );
%Docstring
Sets the resize ``mode`` used for drawing the picture within the item bounds.

:param mode: ResizeMode to use for image file

.. seealso:: :py:func:`resizeMode`
%End

    void refreshPicture( const QgsExpressionContext *context = 0 );
%Docstring
Recalculates the source image (if using an expression for picture's source)
and reloads and redraws the picture.

:param context: expression context for evaluating data defined picture sources
%End

    void recalculateSize();
%Docstring
Forces a recalculation of the picture's frame size
%End

    virtual void refreshDataDefinedProperty( const QgsLayoutObject::DataDefinedProperty property = QgsLayoutObject::AllProperties );

    virtual bool containsAdvancedEffects() const;


  signals:
    void pictureRotationChanged( double newRotation );
%Docstring
Is emitted on picture rotation change
%End

  protected:

    virtual void draw( QgsRenderContext &context, const QStyleOptionGraphicsItem *itemStyle = 0 );

    virtual QSizeF applyItemSizeConstraint( const QSizeF &targetSize );

    virtual bool writePropertiesToElement( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const;

    virtual bool readPropertiesFromElement( const QDomElement &element, const QDomDocument &document, const QgsReadWriteContext &context );


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutitempicture.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
