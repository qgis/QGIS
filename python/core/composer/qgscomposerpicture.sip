/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/composer/qgscomposerpicture.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsComposerPicture: QgsComposerItem
{
%Docstring
 A composer class that displays svg files or raster format (jpg, png, ...)
 *
%End

%TypeHeaderCode
#include "qgscomposerpicture.h"
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

    enum Mode
    {
      SVG,
      RASTER,
      Unknown
    };

    enum NorthMode
    {
      GridNorth,
      TrueNorth,
    };

    QgsComposerPicture( QgsComposition *composition /TransferThis/ );

    virtual int type() const;
%Docstring
Return correct graphics item type.
 :rtype: int
%End

    virtual void paint( QPainter *painter, const QStyleOptionGraphicsItem *itemStyle, QWidget *pWidget );

%Docstring
Reimplementation of QCanvasItem.paint
%End

    void setPicturePath( const QString &path );
%Docstring
 Sets the source path of the image (may be svg or a raster format). Data defined
 picture source may override this value. The path can either be a local path
 or a remote (http) path.
 \param path path for the source image
.. seealso:: usePictureExpression
.. seealso:: picturePath
.. versionadded:: 2.5
%End

    QString picturePath() const;
%Docstring
 Returns the path of the source image. Data defined picture source may override
 this value. The path can either be a local path or a remote (http) path.
 :return: path for the source image
.. seealso:: usePictureExpression
.. seealso:: setPicturePath
.. versionadded:: 2.5
 :rtype: str
%End

    virtual void setSceneRect( const QRectF &rectangle );

%Docstring
 Sets this items bound in scene coordinates such that 1 item size units
 corresponds to 1 scene size unit and resizes the svg symbol / image
%End

    virtual bool writeXml( QDomElement &elem, QDomDocument &doc ) const;

%Docstring
 Stores state in Dom element
 \param elem is Dom element corresponding to 'Composer' tag
 \param doc is Dom document
 :rtype: bool
%End

    virtual bool readXml( const QDomElement &itemElem, const QDomDocument &doc );

%Docstring
 Sets state from Dom document
 \param itemElem is Dom node corresponding to item tag
 \param doc is Dom document
 :rtype: bool
%End

    double pictureRotation() const;
%Docstring
 Returns the rotation used for drawing the picture within the item's frame,
 in degrees clockwise.
.. versionadded:: 2.2
.. seealso:: setPictureRotation()
.. seealso:: rotationMap()
 :rtype: float
%End

    void setRotationMap( int composerMapId );
%Docstring
 Sets the map object for rotation (by id). A value of -1 disables the map
 rotation.  If this is set then the picture will be rotated by the same
 amount as the specified map object. This is useful especially for
 syncing north arrows with a map item.
 \param composerMapId composer map id to sync rotation with
.. seealso:: setPictureRotation
.. seealso:: rotationMap
%End

    int rotationMap() const;
%Docstring
 Returns the id of the rotation map.  A value of -1 means map rotation is
 disabled.  If this is set then the picture is rotated by the same amount
 as the specified map object.
 :return: id of map object
.. seealso:: setRotationMap
.. seealso:: useRotationMap
 :rtype: int
%End

    bool useRotationMap() const;
%Docstring
 True if the picture rotation is matched to a map item.
 :return: true if rotation map is in use
.. seealso:: rotationMap
.. seealso:: setRotationMap
 :rtype: bool
%End

    NorthMode northMode() const;
%Docstring
 Returns the mode used to align the picture to a map's North.
.. seealso:: setNorthMode()
.. seealso:: northOffset()
.. versionadded:: 2.18
 :rtype: NorthMode
%End

    void setNorthMode( NorthMode mode );
%Docstring
 Sets the mode used to align the picture to a map's North.
.. seealso:: northMode()
.. seealso:: setNorthOffset()
.. versionadded:: 2.18
%End

    double northOffset() const;
%Docstring
 Returns the offset added to the picture's rotation from a map's North.
.. seealso:: setNorthOffset()
.. seealso:: northMode()
.. versionadded:: 2.18
 :rtype: float
%End

    void setNorthOffset( double offset );
%Docstring
 Sets the offset added to the picture's rotation from a map's North.
.. seealso:: northOffset()
.. seealso:: setNorthMode()
.. versionadded:: 2.18
%End

    ResizeMode resizeMode() const;
%Docstring
 Returns the resize mode used for drawing the picture within the composer
 item's frame.
 :return: resize mode of picture
.. versionadded:: 2.3
.. seealso:: setResizeMode
 :rtype: ResizeMode
%End

    void setPictureAnchor( QgsComposerItem::ItemPositionMode anchor );
%Docstring
 Sets the picture's anchor point, which controls how it is placed
 within the picture item's frame.
 \param anchor anchor point for picture
.. versionadded:: 2.3
.. seealso:: pictureAnchor
%End

    ItemPositionMode pictureAnchor() const;
%Docstring
 Returns the picture's current anchor, which controls how it is placed
 within the picture item's frame.
 :return: anchor point for picture
.. versionadded:: 2.3
.. seealso:: setPictureAnchor
 :rtype: ItemPositionMode
%End

    QColor svgFillColor() const;
%Docstring
 Returns the fill color used for parametrized SVG files.
.. seealso:: setSvgFillColor()
.. seealso:: svgStrokeColor()
.. versionadded:: 2.14.1
 :rtype: QColor
%End

    void setSvgFillColor( const QColor &color );
%Docstring
 Sets the fill color used for parametrized SVG files.
 \param color fill color.
.. note::

   this setting only has an effect on parametrized SVG files, and is ignored for
 non-parametrized SVG files.
.. seealso:: svgFillColor()
.. seealso:: setSvgStrokeColor()
.. versionadded:: 2.14.1
%End

    QColor svgStrokeColor() const;
%Docstring
 Returns the stroke color used for parametrized SVG files.
.. seealso:: setSvgStrokeColor()
.. seealso:: svgFillColor()
.. versionadded:: 2.14.1
 :rtype: QColor
%End

    void setSvgStrokeColor( const QColor &color );
%Docstring
 Sets the stroke color used for parametrized SVG files.
 \param color stroke color.
.. note::

   this setting only has an effect on parametrized SVG files, and is ignored for
 non-parametrized SVG files.
.. seealso:: svgStrokelColor()
.. seealso:: setSvgFillColor()
.. versionadded:: 2.14.1
%End

    double svgStrokeWidth() const;
%Docstring
 Returns the stroke width (in mm) used for parametrized SVG files.
.. seealso:: setSvgStrokeWidth()
.. seealso:: svgStrokeColor()
.. versionadded:: 2.14.1
 :rtype: float
%End

    void setSvgStrokeWidth( double width );
%Docstring
 Sets the stroke width used for parametrized SVG files.
 \param width stroke width in mm
.. note::

   this setting only has an effect on parametrized SVG files, and is ignored for
 non-parametrized SVG files.
.. seealso:: svgStrokeWidth()
.. seealso:: setSvgStrokeColor()
.. versionadded:: 2.14.1
%End

    Mode mode() const;
%Docstring
 Returns the current picture mode (image format).
 :return: picture mode
.. versionadded:: 2.3
 :rtype: Mode
%End

  public slots:

    virtual void setPictureRotation( double rotation );
%Docstring
 Sets the picture ``rotation`` within the item bounds, in degrees clockwise. This does not affect
 the item's frame, only the way the picture is drawn within the item.
.. seealso:: pictureRotation()
.. versionadded:: 2.2
%End

    virtual void setResizeMode( ResizeMode mode );
%Docstring
 Sets the resize mode used for drawing the picture within the item bounds.
 \param mode ResizeMode to use for image file
.. versionadded:: 2.3
.. seealso:: resizeMode
%End

    void refreshPicture( const QgsExpressionContext *context = 0 );
%Docstring
 Recalculates the source image (if using an expression for picture's source)
 and reloads and redraws the picture.
 \param context expression context for evaluating data defined picture sources
.. versionadded:: 2.3
%End

    void recalculateSize();
%Docstring
 Forces a recalculation of the picture's frame size
.. versionadded:: 2.3
%End

    virtual void refreshDataDefinedProperty( const QgsComposerObject::DataDefinedProperty property = QgsComposerObject::AllProperties, const QgsExpressionContext *context = 0 );

  signals:
    void pictureRotationChanged( double newRotation );
%Docstring
Is emitted on picture rotation change
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/composer/qgscomposerpicture.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
