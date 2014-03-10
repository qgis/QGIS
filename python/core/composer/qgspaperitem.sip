/**Item representing a grid. This is drawn separately to the underlying paper item since the grid needs to be
 * drawn above all other composer items, while the paper item is drawn below all others.*/
class QgsPaperGrid : QGraphicsRectItem
{
%TypeHeaderCode
#include <qgspaperitem.h>
%End
  public:
    QgsPaperGrid( double x, double y, double width, double height, QgsComposition* composition );
    ~QgsPaperGrid();

    /** \brief Reimplementation of QCanvasItem::paint*/
    void paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget );

};

/**Item representing the paper.*/
class QgsPaperItem : QgsComposerItem
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

    virtual void setSceneRect( const QRectF& rectangle );

  private:
    QgsPaperItem();
    /**Set flags and z-value*/
    void initialize();
};
