/**Item representing the paper. May draw the snapping grid lines if composition is in
 preview mode*/
class QgsPaperItem: QgsComposerItem
{
%TypeHeaderCode
#include <qgspaperitem.h>
%End
  public:
    QgsPaperItem( QgsComposition* c /TransferThis/ );
    QgsPaperItem( qreal x, qreal y, qreal width, qreal height, QgsComposition* composition /TransferThis/ );
    ~QgsPaperItem();

    /** return correct graphics item type. Added in v1.7 */
    virtual int type() const;

    /** \brief Reimplementation of QCanvasItem::paint*/
    void paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget );

    /** stores state in Dom element
       * @param elem is Dom element corresponding to 'Composer' tag
       * @param doc Dom document
       */
    bool writeXML( QDomElement& elem, QDomDocument & doc ) const;

    /** sets state from Dom document
     * @param itemElem is Dom node corresponding to item tag
     * @param doc is the Dom document
     */
    bool readXML( const QDomElement& itemElem, const QDomDocument& doc );

  private:
    QgsPaperItem();
    /**Set flags and z-value*/
    void initialize();
};
