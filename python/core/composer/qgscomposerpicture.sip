/** \ingroup MapComposer
 * A composer class that displays svg files or raster format (jpg, png, ...)
 * */
class QgsComposerPicture: QgsComposerItem
{

%TypeHeaderCode
#include "qgscomposerpicture.h"
%End

  public:
    QgsComposerPicture( QgsComposition *composition /TransferThis/);
    ~QgsComposerPicture();

    /** return correct graphics item type. Added in v1.7 */
    virtual int type() const;

    /**Reimplementation of QCanvasItem::paint*/
    void paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget );

    /**Sets the source file of the image (may be svg or a raster format)*/
    void setPictureFile( const QString& path );
    QString pictureFile() const;

    /**Sets this items bound in scene coordinates such that 1 item size units
       corresponds to 1 scene size unit and resizes the svg symbol / image*/
    void setSceneRect( const QRectF& rectangle );

    /** stores state in Dom element
       * @param elem is Dom element corresponding to 'Composer' tag
       * @param doc is Dom document
       */
    bool writeXML( QDomElement& elem, QDomDocument & doc ) const;

    /** sets state from Dom document
      * @param itemElem is Dom node corresponding to item tag
      * @param doc is Dom document
      */
    bool readXML( const QDomElement& itemElem, const QDomDocument& doc );

    /**Returns the rotation used for drawing the picture within the composer item
     * @deprecated Use pictureRotation() instead
     */
    double rotation() const;

    /**Returns the rotation used for drawing the picture within the item
      @note this function was added in version 2.1*/
    double pictureRotation() const;

    /**Sets the map object for rotation (by id). A value of -1 disables the map rotation*/
    void setRotationMap( int composerMapId );
    /**Returns the id of the rotation map*/
    int rotationMap() const;
    /**True if the rotation is taken from a map item*/
    bool useRotationMap() const;

    /**Calculates width and hight of the picture (in mm) such that it fits into the item frame with the given rotation
     * @deprecated Use bool QgsComposerItem::imageSizeConsideringRotation( double& width, double& height, double rotation )
     * instead
     */
    bool imageSizeConsideringRotation( double& width, double& height ) const;
    /**Calculates corner point after rotation and scaling
     * @deprecated Use QgsComposerItem::cornerPointOnRotatedAndScaledRect( double& x, double& y, double width, double height, double rotation )
     * instead
     */
    bool cornerPointOnRotatedAndScaledRect( double& x, double& y, double width, double height ) const;
    /**Calculates width / height of the bounding box of a rotated rectangle
    * @deprecated Use QgsComposerItem::sizeChangedByRotation( double& width, double& height, double rotation )
    * instead
    */
    void sizeChangedByRotation( double& width, double& height );

  public slots:
    /**Sets the picture rotation within the item bounds. This does not affect the item rectangle,
      only the way the picture is drawn within the item.
     * @deprecated Use setPictureRotation( double rotation ) instead
     */
    virtual void setRotation( double r );

    /**Sets the picture rotation within the item bounds. This does not affect the item rectangle,
      only the way the picture is drawn within the item.
      @note this function was added in version 2.1*/
    virtual void setPictureRotation( double r );

  signals:
    /**Is emitted on picture rotation change*/
    void pictureRotationChanged( double newRotation );
};
