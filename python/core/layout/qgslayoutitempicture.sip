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

    virtual QString stringType() const;


    static QgsLayoutItemPicture *create( QgsLayout *layout ) /Factory/;
%Docstring
 Returns a new picture item for the specified ``layout``.

 The caller takes responsibility for deleting the returned object.
 :rtype: QgsLayoutItemPicture
%End


    void setPicturePath( const QString &path );
%Docstring
 Sets the source ``path`` of the image (may be svg or a raster format). Data defined
 picture source may override this value. The path can either be a local path
 or a remote (http) path.
.. seealso:: picturePath()
%End

    QString picturePath() const;
%Docstring
 Returns the path of the source image. Data defined picture source may override
 this value. The path can either be a local path or a remote (http) path.
 :return: path for the source image
.. seealso:: usePictureExpression()
.. seealso:: setPicturePath()
 :rtype: str
%End

    double pictureRotation() const;
%Docstring
 Returns the rotation used for drawing the picture within the item's frame,
 in degrees clockwise.
.. seealso:: setPictureRotation()
.. seealso:: rotationMap()
 :rtype: float
%End

    void setRotationMap( const QString &uuid );
%Docstring
 Sets the map object for rotation (by ``uuid``). A empty string disables the map
 rotation.  If this is set then the picture will be rotated by the same
 amount as the specified map object. This is useful especially for
 syncing north arrows with a map item.
.. seealso:: setPictureRotation()
.. seealso:: rotationMap()
%End

    QString rotationMap() const;
%Docstring
 Returns the uuid of the rotation map. An empty string means map rotation is
 disabled.  If this is set then the picture is rotated by the same amount
 as the specified map object.
.. seealso:: setRotationMap()
.. seealso:: useRotationMap()
 :rtype: str
%End

    bool useRotationMap() const;
%Docstring
 True if the picture rotation is matched to a map item.
.. seealso:: rotationMap()
.. seealso:: setRotationMap()
 :rtype: bool
%End

    NorthMode northMode() const;
%Docstring
 Returns the mode used to align the picture to a map's North.
.. seealso:: setNorthMode()
.. seealso:: northOffset()
 :rtype: NorthMode
%End

    void setNorthMode( NorthMode mode );
%Docstring
 Sets the ``mode`` used to align the picture to a map's North.
.. seealso:: northMode()
.. seealso:: setNorthOffset()
%End

    double northOffset() const;
%Docstring
 Returns the offset added to the picture's rotation from a map's North.
.. seealso:: setNorthOffset()
.. seealso:: northMode()
 :rtype: float
%End

    void setNorthOffset( double offset );
%Docstring
 Sets the ``offset`` added to the picture's rotation from a map's North.
.. seealso:: northOffset()
.. seealso:: setNorthMode()
%End

    ResizeMode resizeMode() const;
%Docstring
 Returns the resize mode used for drawing the picture within the composer
 item's frame.
.. seealso:: setResizeMode()
 :rtype: ResizeMode
%End

    void setPictureAnchor( QgsLayoutItem::ReferencePoint anchor );
%Docstring
 Sets the picture's ``anchor`` point, which controls how it is placed
 within the picture item's frame.
.. seealso:: pictureAnchor()
%End

    QgsLayoutItem::ReferencePoint pictureAnchor() const;
%Docstring
 Returns the picture's current anchor, which controls how it is placed
 within the picture item's frame.
.. seealso:: setPictureAnchor()
 :rtype: QgsLayoutItem.ReferencePoint
%End

    QColor svgFillColor() const;
%Docstring
 Returns the fill color used for parametrized SVG files.
.. seealso:: setSvgFillColor()
.. seealso:: svgStrokeColor()
 :rtype: QColor
%End

    void setSvgFillColor( const QColor &color );
%Docstring
 Sets the fill ``color`` used for parametrized SVG files.
.. note::

   This setting only has an effect on parametrized SVG files, and is ignored for
 non-parametrized SVG files.
.. seealso:: svgFillColor()
.. seealso:: setSvgStrokeColor()
%End

    QColor svgStrokeColor() const;
%Docstring
 Returns the stroke color used for parametrized SVG files.
.. seealso:: setSvgStrokeColor()
.. seealso:: svgFillColor()
 :rtype: QColor
%End

    void setSvgStrokeColor( const QColor &color );
%Docstring
 Sets the stroke ``color`` used for parametrized SVG files.
 \param color stroke color.
.. note::

   This setting only has an effect on parametrized SVG files, and is ignored for
 non-parametrized SVG files.
.. seealso:: svgStrokelColor()
.. seealso:: setSvgFillColor()
%End

    double svgStrokeWidth() const;
%Docstring
 Returns the stroke width (in layout units) used for parametrized SVG files.
.. seealso:: setSvgStrokeWidth()
.. seealso:: svgStrokeColor()
 :rtype: float
%End

    void setSvgStrokeWidth( double width );
%Docstring
 Sets the stroke ``width`` (in layout units) used for parametrized SVG files.
.. note::

   This setting only has an effect on parametrized SVG files, and is ignored for
 non-parametrized SVG files.
.. seealso:: svgStrokeWidth()
.. seealso:: setSvgStrokeColor()
%End

    Format mode() const;
%Docstring
 Returns the current picture mode (image format).
 :rtype: Format
%End

  public slots:

    void setPictureRotation( double rotation );
%Docstring
 Sets the picture ``rotation`` within the item bounds, in degrees clockwise. This does not affect
 the item's frame, only the way the picture is drawn within the item.
.. seealso:: pictureRotation()
%End

    void setResizeMode( ResizeMode mode );
%Docstring
 Sets the resize ``mode`` used for drawing the picture within the item bounds.
 \param mode ResizeMode to use for image file
.. seealso:: resizeMode
%End

    void refreshPicture( const QgsExpressionContext *context = 0 );
%Docstring
 Recalculates the source image (if using an expression for picture's source)
 and reloads and redraws the picture.
 \param context expression context for evaluating data defined picture sources
%End

    void recalculateSize();
%Docstring
 Forces a recalculation of the picture's frame size
%End

    virtual void refreshDataDefinedProperty( const QgsLayoutObject::DataDefinedProperty property = QgsLayoutObject::AllProperties );


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
