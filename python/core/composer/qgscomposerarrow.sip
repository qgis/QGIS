class QgsComposerArrow: QgsComposerItem
{
%TypeHeaderCode
#include <qgscomposerarrow.h>
%End
  public:

    enum MarkerMode
    {
      DefaultMarker,
      NoMarker,
      SVGMarker
    };

    QgsComposerArrow( QgsComposition* c );
    QgsComposerArrow( const QPointF& startPoint, const QPointF& stopPoint, QgsComposition* c );
    ~QgsComposerArrow();

    /** return correct graphics item type. Added in v1.7 */
    virtual int type() const;

    /** \brief Reimplementation of QCanvasItem::paint - draw on canvas */
    void paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget );

    /**Modifies position of start and endpoint and calls QgsComposerItem::setSceneRect*/
    void setSceneRect( const QRectF& rectangle );

    /**Sets the width of the arrow head in mm*/
    void setArrowHeadWidth( double width );
    double arrowHeadWidth() const;

    void setOutlineWidth( double width );
    double outlineWidth() const;

    void setStartMarker( const QString& svgPath );
    QString startMarker() const;
    void setEndMarker( const QString& svgPath );
    QString endMarker() const;

    QColor arrowColor() const;
    void setArrowColor( const QColor& c );

    MarkerMode markerMode() const;
    void setMarkerMode( MarkerMode mode );

    /** stores state in Dom element
    * @param elem is Dom element corresponding to 'Composer' tag
    * @param doc document
    */
    bool writeXML( QDomElement& elem, QDomDocument & doc ) const;

    /** sets state from Dom document
    * @param itemElem is Dom node corresponding to item tag
    * @param doc is the document to read
    */
    bool readXML( const QDomElement& itemElem, const QDomDocument& doc );
};
