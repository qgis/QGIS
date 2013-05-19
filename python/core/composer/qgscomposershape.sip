/**A composer items that draws common shapes (ellipse, triangle, rectangle)*/
class QgsComposerShape: QgsComposerItem
{
%TypeHeaderCode
#include "qgscomposershape.h"
%End
  public:

    enum Shape
    {
      Ellipse,
      Rectangle,
      Triangle
    };

    QgsComposerShape( QgsComposition* composition /TransferThis/);
    QgsComposerShape( qreal x, qreal y, qreal width, qreal height, QgsComposition* composition /TransferThis/);
    ~QgsComposerShape();

    /** return correct graphics item type. Added in v1.7 */
    virtual int type() const;

    /** \brief Reimplementation of QCanvasItem::paint - draw on canvas */
    void paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget );

    /** stores state in Dom element
     * @param elem is Dom element corresponding to 'Composer' tag
     * @param doc write template file
     */
    bool writeXML( QDomElement& elem, QDomDocument & doc ) const;

    /** sets state from Dom document
     * @param itemElem is Dom node corresponding to item tag
     * @param doc is Dom document
     */
    bool readXML( const QDomElement& itemElem, const QDomDocument& doc );

    //setters and getters
    QgsComposerShape::Shape shapeType() const;
    void setShapeType( QgsComposerShape::Shape s );

    /**Sets this items bound in scene coordinates such that 1 item size units
     corresponds to 1 scene size unit. Also, the shape is scaled*/
    void setSceneRect( const QRectF& rectangle );

  public slots:
    /**Sets item rotation and resizes item bounds such that the shape always has the same size*/
    virtual void setRotation( double r );
};
