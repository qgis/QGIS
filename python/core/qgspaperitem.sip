/**Item representing the paper. May draw the snapping grid lines if composition is in
 preview mode*/
class QgsPaperItem: QgsComposerItem
{
%TypeHeaderCode
#include <qgspaperitem.h>
%End
  public:
    QgsPaperItem( QgsComposition* c );
    QgsPaperItem( qreal x, qreal y, qreal width, qreal height, QgsComposition* composition );
    ~QgsPaperItem();

    /** \brief Reimplementation of QCanvasItem::paint*/
    void paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget );

    /** stores state in Dom node
       * @param node is Dom node corresponding to 'Composer' tag
       * @param temp write template file
       */
    bool writeXML( QDomElement& elem, QDomDocument & doc ) const;

    /** sets state from Dom document
     * @param itemElem is Dom node corresponding to item tag
     */
    bool readXML( const QDomElement& itemElem, const QDomDocument& doc );

  private:
    QgsPaperItem();
};