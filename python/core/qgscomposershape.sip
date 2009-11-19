/**A composer items that draws common shapes (ellipse, triangle, rectangle)*/
class QgsComposerShape: public QgsComposerItem
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
    QgsComposerShape( qreal x, qreal y, qreal width, qreal height, QgsComposition* composition );
    ~QgsComposerShape();

    /** \brief Reimplementation of QCanvasItem::paint - draw on canvas */
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

    //setters and getters
    void setLineWidth( double width );
    double lineWidth() const;
    void setOutlineColor( const QColor& color );
    QColor outlineColor() const;
    void setFillColor( const QColor& color );
    QColor fillColor() const;
    QgsComposerShape::Shape shapeType() const;
    void setShapeType( QgsComposerShape::Shape s );
}; 
