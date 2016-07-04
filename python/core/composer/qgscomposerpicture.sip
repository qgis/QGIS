/** \ingroup core
 * A composer class that displays svg files or raster format (jpg, png, ...)
 * */
class QgsComposerPicture: QgsComposerItem
{

%TypeHeaderCode
#include "qgscomposerpicture.h"
%End

  public:

    /** Controls how pictures are scaled within the item's frame
     */
    enum ResizeMode
    {
      Zoom, /*!< enlarges image to fit frame while maintaining aspect ratio of picture */
      Stretch, /*!< stretches image to fit frame, ignores aspect ratio */
      Clip, /*!< draws image at original size and clips any portion which falls outside frame */
      ZoomResizeFrame, /*!< enlarges image to fit frame, then resizes frame to fit resultant image */
      FrameToImageSize /*!< sets size of frame to match original size of image without scaling */
    };

    /** Format of source image
     */
    enum Mode
    {
      SVG,
      RASTER,
      Unknown
    };

    QgsComposerPicture( QgsComposition *composition /TransferThis/);
    ~QgsComposerPicture();

    /** Return correct graphics item type. */
    virtual int type() const;

    /** Reimplementation of QCanvasItem::paint*/
    void paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget );

    /** Sets the source file of the image (may be svg or a raster format). Data defined
     * picture source may override this value.
     * @param path full path to the source image
     * @see usePictureExpression
     * @see pictureFile
     * @deprecated use setPicturePath instead
     */
    void setPictureFile( const QString& path ) /Deprecated/;

    /** Returns the path of the source image file. Data defined picture source may override
     * this value.
     * @returns path to the source image
     * @see usePictureExpression
     * @see setPictureFile
     * @deprecated use picturePath instead
     */
    QString pictureFile() const /Deprecated/;

    /** Sets the source path of the image (may be svg or a raster format). Data defined
     * picture source may override this value. The path can either be a local path
     * or a remote (http) path.
     * @param path path for the source image
     * @see usePictureExpression
     * @see picturePath
     * @note added in QGIS 2.5
     */
    void setPicturePath( const QString& path );

    /** Returns the path of the source image. Data defined picture source may override
     * this value. The path can either be a local path or a remote (http) path.
     * @returns path for the source image
     * @see usePictureExpression
     * @see setPicturePath
     * @note added in QGIS 2.5
     */
    QString picturePath() const;

    /** Sets this items bound in scene coordinates such that 1 item size units
     * corresponds to 1 scene size unit and resizes the svg symbol / image
     */
    void setSceneRect( const QRectF& rectangle );

    /** Stores state in Dom element
     * @param elem is Dom element corresponding to 'Composer' tag
     * @param doc is Dom document
     */
    bool writeXML( QDomElement& elem, QDomDocument & doc ) const;

    /** Sets state from Dom document
     * @param itemElem is Dom node corresponding to item tag
     * @param doc is Dom document
     */
    bool readXML( const QDomElement& itemElem, const QDomDocument& doc );

    /** Returns the rotation used for drawing the picture within the composer item
     * @deprecated Use pictureRotation() instead
     */
    double rotation() const /Deprecated/;

    /** Returns the rotation used for drawing the picture within the item's frame
     * @returns picture rotation in degrees
     * @note added in 2.2
     * @see setPictureRotation
     * @see rotationMap
     */
    double pictureRotation() const;

    /** Sets the map object for rotation (by id). A value of -1 disables the map
     * rotation.  If this is set then the picture will be rotated by the same
     * amount as the specified map object. This is useful especially for
     * syncing north arrows with a map item.
     * @param composerMapId composer map id to sync rotation with
     * @see setPictureRotation
     * @see rotationMap
     */
    void setRotationMap( int composerMapId );

    /** Returns the id of the rotation map.  A value of -1 means map rotation is
     * disabled.  If this is set then the picture is rotated by the same amount
     * as the specified map object.
     * @returns id of map object
     * @see setRotationMap
     * @see useRotationMap
     */
    int rotationMap() const;

    /** True if the picture rotation is matched to a map item.
     * @returns true if rotation map is in use
     * @see rotationMap
     * @see setRotationMap
     */
    bool useRotationMap() const;

    /** Returns the resize mode used for drawing the picture within the composer
     * item's frame.
     * @returns resize mode of picture
     * @note added in 2.3
     * @see setResizeMode
     */
    ResizeMode resizeMode() const;

    /** Sets the picture's anchor point, which controls how it is placed
     * within the picture item's frame.
     * @param anchor anchor point for picture
     * @note added in 2.3
     * @see pictureAnchor
     */
    void setPictureAnchor( QgsComposerItem::ItemPositionMode anchor );

    /** Returns the picture's current anchor, which controls how it is placed
     * within the picture item's frame.
     * @returns anchor point for picture
     * @note added in 2.3
     * @see setPictureAnchor
     */
    ItemPositionMode pictureAnchor() const;

    /** Returns the fill color used for parameterized SVG files.
     * @see setSvgFillColor()
     * @see svgBorderColor()
     * @note added in QGIS 2.14.1
     */
    QColor svgFillColor() const;

    /** Sets the fill color used for parameterized SVG files.
     * @param color fill color.
     * @note this setting only has an effect on parameterized SVG files, and is ignored for
     * non-parameterized SVG files.
     * @see svgFillColor()
     * @see setSvgBorderColor()
     * @note added in QGIS 2.14.1
     */
    void setSvgFillColor( const QColor& color );

    /** Returns the border color used for parameterized SVG files.
     * @see setSvgBorderColor()
     * @see svgFillColor()
     * @note added in QGIS 2.14.1
     */
    QColor svgBorderColor() const;

    /** Sets the border color used for parameterized SVG files.
     * @param color border color.
     * @note this setting only has an effect on parameterized SVG files, and is ignored for
     * non-parameterized SVG files.
     * @see svgBorderlColor()
     * @see setSvgFillColor()
     * @note added in QGIS 2.14.1
     */
    void setSvgBorderColor( const QColor& color );

    /** Returns the border width (in mm) used for parameterized SVG files.
     * @see setSvgBorderWidth()
     * @see svgBorderColor()
     * @note added in QGIS 2.14.1
     */
    double svgBorderWidth() const;

    /** Sets the border width used for parameterized SVG files.
     * @param width border width in mm
     * @note this setting only has an effect on parameterized SVG files, and is ignored for
     * non-parameterized SVG files.
     * @see svgBorderWidth()
     * @see setSvgBorderColor()
     * @note added in QGIS 2.14.1
     */
    void setSvgBorderWidth( double width );

    /** Returns whether the picture item is using an expression for the image source.
     * @returns true if the picture is using an expression for the source, false if
     * it is using a single static file path for the source.
     * @note added in 2.3
     * @see setUsePictureExpression
     * @see pictureFile
     * @see pictureExpression
     * @deprecated use QgsComposerObject::dataDefinedProperty( QgsComposerObject::PictureSource ) instead
     */
    bool usePictureExpression() const /Deprecated/;

    /** Returns the expression the item is using for the picture source. This is only
     * used if usePictureExpression() is true.
     * @returns expression for the picture item's image path
     * @note added in 2.3
     * @see setPictureExpression
     * @see usePictureExpression
     * @deprecated use QgsComposerObject::dataDefinedProperty( QgsComposerObject::PictureSource ) instead
     */
    QString pictureExpression() const /Deprecated/;

    /** Calculates width and hight of the picture (in mm) such that it fits into the item frame with the given rotation
     * @deprecated Use bool QgsComposerItem::imageSizeConsideringRotation( double& width, double& height, double rotation )
     * instead
     */
    bool imageSizeConsideringRotation( double& width, double& height ) const /Deprecated/;

    /** Calculates corner point after rotation and scaling
     * @deprecated Use QgsComposerItem::cornerPointOnRotatedAndScaledRect( double& x, double& y, double width, double height, double rotation )
     * instead
     */
    bool cornerPointOnRotatedAndScaledRect( double& x, double& y, double width, double height ) const /Deprecated/;

    /** Calculates width / height of the bounding box of a rotated rectangle
     * @deprecated Use QgsComposerItem::sizeChangedByRotation( double& width, double& height, double rotation )
     * instead
     */
    void sizeChangedByRotation( double& width, double& height ) /Deprecated/;

    /** Returns the current picture mode (image format).
     * @returns picture mode
     * @note added in 2.3
     */
    Mode mode() const;

  public slots:
    /** Sets the picture rotation within the item bounds. This does not affect
     * the item rectangle, only the way the picture is drawn within the item.
     * @deprecated Use setPictureRotation( double rotation ) instead
     */
    virtual void setRotation( double r ) /Deprecated/;

    /** Sets the picture rotation within the item bounds. This does not affect
     * the item's frame, only the way the picture is drawn within the item.
     * @param r rotation in degrees clockwise
     * @see pictureRotation
     * @note added in 2.2
     */
    virtual void setPictureRotation( double r );

    /** Sets the resize mode used for drawing the picture within the item bounds.
     * @param mode ResizeMode to use for image file
     * @note added in 2.3
     * @see resizeMode
     */
    virtual void setResizeMode( ResizeMode mode );

    /** Sets whether the picture should use an expression based image source path
     * @param useExpression set to true to use an expression based image source,
     * set to false to use a single image source path
     * @note added in 2.3
     * @see usePictureExpression
     * @see setPictureFile
     * @see setPictureExpression
     * @deprecated use QgsComposerObject::dataDefinedProperty( QgsComposerObject::PictureSource ) instead
     */
    virtual void setUsePictureExpression( bool useExpression ) /Deprecated/;

    /** Sets an expression to use for the picture source. This expression is only
     * used if usePictureExpression() is true.
     * @param expression to use for picture path
     * @note added in 2.3
     * @see setUsePictureExpression
     * @see pictureExpression
     * @deprecated use QgsComposerObject::dataDefinedProperty( QgsComposerObject::PictureSource ) instead
     */
    virtual void setPictureExpression( const QString& expression ) /Deprecated/;

    /** Recalculates the source image (if using an expression for picture's source)
     * and reloads and redraws the picture.
     * @param context expression context for evaluating data defined picture sources
     * @note added in 2.3
     */
    void refreshPicture( const QgsExpressionContext* context = 0 );

    /** Prepares the picture's source expression after it is altered or the compositions
     * atlas coverage layer changes.
     * @note added in 2.3
     * @deprecated no longer required
     */
    void updatePictureExpression() /Deprecated/;

    /** Forces a recalculation of the picture's frame size
     * @note added in 2.3
     */
    void recalculateSize();

    virtual void refreshDataDefinedProperty( const QgsComposerObject::DataDefinedProperty property = QgsComposerObject::AllProperties, const QgsExpressionContext *context = 0 );

  signals:
    /** Is emitted on picture rotation change*/
    void pictureRotationChanged( double newRotation );
};
