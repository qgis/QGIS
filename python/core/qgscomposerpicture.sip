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

    /**Reimplementation of QCanvasItem::paint*/
    void paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget );

    /**Sets the source file of the image (may be svg or a raster format)*/
    void setPictureFile( const QString& path );
    QString pictureFile() const;

    /**Sets this items bound in scene coordinates such that 1 item size units
       corresponds to 1 scene size unit*/
    void setSceneRect( const QRectF& rectangle );

    /** stores state in Dom node
       * @param node is Dom node corresponding to 'Composer' tag
       * @param temp write template file
       */
    bool writeXML( QDomElement& elem, QDomDocument & doc ) const;

    /** sets state from Dom document
      * @param itemElem is Dom node corresponding to item tag
      */
    bool readXML( const QDomElement& itemElem, const QDomDocument& doc );

    /**Sets the map object for rotation (by id). A value of -1 disables the map rotation*/
    void setRotationMap( int composerMapId );
    /**Returns the id of the rotation map*/
    int rotationMap() const;
    /**True if the rotation is taken from a map item*/
    bool useRotationMap() const;


  signals:
    /**Tell the configuration widget that the settings need to be updated*/
    void settingsChanged();
};
