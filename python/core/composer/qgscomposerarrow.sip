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

    /** Constructor
     * @param c parent composition
     */
    QgsComposerArrow( QgsComposition* c /TransferThis/ );

    /** Constructor
     * @param startPoint start point for line
     * @param stopPoint end point for line
     * @param c parent composition
     */
    QgsComposerArrow( QPointF startPoint, QPointF stopPoint, QgsComposition* c /TransferThis/ );

    ~QgsComposerArrow();

    /** Return composer item type. */
    virtual int type() const;

    /** \brief Reimplementation of QCanvasItem::paint - draw on canvas */
    void paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget );

    /** Modifies position of start and endpoint and calls QgsComposerItem::setSceneRect
     */
    void setSceneRect( const QRectF& rectangle );

    /** Sets the width of the arrow head in mm
     * @param width width of arrow head
     * @see arrowHeadWidth
     */
    void setArrowHeadWidth( double width );

    /** Returns the width of the arrow head in mm
     * @returns width of arrow head
     * @see setArrowHeadWidth
     */
    double arrowHeadWidth() const;

    /** Sets the marker to draw at the start of the line
     * @param svgPath file path for svg marker graphic to draw
     * @see startMarker
     * @see setEndMarker
     */
    void setStartMarker( const QString& svgPath );

    /** Returns the marker drawn at the start of the line
     * @returns file path for svg marker graphic
     * @see setStartMarker
     * @see endMarker
     */
    QString startMarker() const;

    /** Sets the marker to draw at the end of the line
     * @param svgPath file path for svg marker graphic to draw
     * @see endMarker
     * @see setStartMarker
     */
    void setEndMarker( const QString& svgPath );

    /** Returns the marker drawn at the end of the line
     * @returns file path for svg marker graphic
     * @see setEndMarker
     * @see startMarker
     */
    QString endMarker() const;

    /** Returns the color used to draw stroke around the the arrow head.
     * @returns arrow head stroke color
     * @see arrowHeadFillColor
     * @see setArrowHeadStrokeColor
     * @note added in 2.5
     */
    QColor arrowHeadStrokeColor() const;

    /** Sets the color used to draw the stroke around the arrow head.
     * @param color arrow head stroke color
     * @see setArrowHeadFillColor
     * @see arrowHeadStrokeColor
     * @note added in 2.5
     */
    void setArrowHeadStrokeColor( const QColor& color );

    /** Returns the color used to fill the arrow head.
     * @returns arrow head fill color
     * @see arrowHeadStrokeColor
     * @see setArrowHeadFillColor
     * @note added in 2.5
     */
    QColor arrowHeadFillColor() const;

    /** Sets the color used to fill the arrow head.
     * @param color arrow head fill color
     * @see arrowHeadFillColor
     * @see setArrowHeadStrokeColor
     * @note added in 2.5
     */
    void setArrowHeadFillColor( const QColor& color );

    /** Sets the pen width for the stroke of the arrow head
     * @param width pen width for arrow head stroke
     * @see arrowHeadStrokeWidth
     * @see setArrowHeadStrokeColor
     * @note added in 2.5
     */
    void setArrowHeadStrokeWidth( const double width );

    /** Returns the pen width for the stroke of the arrow head
     * @returns pen width for arrow head stroke
     * @see setArrowHeadStrokeWidth
     * @see arrowHeadStrokeColor
     * @note added in 2.5
     */
    double arrowHeadStrokeWidth() const;

    /** Sets the line symbol used for drawing the line portion of the arrow
     * @param symbol line symbol
     * @see lineSymbol
     * @note added in 2.5
     */
    void setLineSymbol( QgsLineSymbol* symbol /Transfer/ );

    /** Returns the line symbol used for drawing the line portion of the arrow
     * @returns line symbol
     * @see setLineSymbol
     * @note added in 2.5
     */
    QgsLineSymbol* lineSymbol();

    /** Returns marker mode, which controls how the arrow endpoints are drawn
     * @returns marker mode
     * @see setMarkerMode
     */
    MarkerMode markerMode() const;

    /** Sets the marker mode, which controls how the arrow endpoints are drawn
     * @param mode marker mode
     * @see setMarkerMode
     */
    void setMarkerMode( MarkerMode mode );

    /** Stores state in DOM element
     * @param elem is DOM element corresponding to 'Composer' tag
     * @param doc document
     */
    bool writeXml( QDomElement& elem, QDomDocument & doc ) const;

    /** Sets state from DOM document
     * @param itemElem is DOM node corresponding to item tag
     * @param doc is the document to read
     */
    bool readXml( const QDomElement& itemElem, const QDomDocument& doc );
};
