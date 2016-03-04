class QgsComposerPointsBasedShape: QgsComposerItem
{
%TypeHeaderCode
#include <qgscomposerpointsbasedshape.h>
%End
  public:

    QgsComposerPointsBasedShape( QString mTagName, QgsComposition* c );
    QgsComposerPointsBasedShape( QString mTagName, QPolygonF polygon, QgsComposition* c );
    ~QgsComposerPointsBasedShape();

    /** Add a point in current shape.
     * @param pt is the location of the new point
     * @param checkArea is a flag to indicate if there's a space constraint.
     * @param radius is the space contraint and is used only if checkArea is
     * true. Typically, if this flag is true, the new point has to be nearest
     * than radius to the shape to be added.
     */
    bool addPoint( const QPointF &pt, const bool checkArea = true, const double radius = 10 );

    /** Set a tag to indicate if we want to draw or not the shape's points.
     * @param display
     */
    void setDisplayPoints( const bool display = true );

    /** Move a point to a new position.
     * @param index the index of the point to move
     * @param point is the new position in scene coordinate
     */
    bool movePoint( const int index, const QPointF &point );

    /** \brief Reimplementation of QCanvasItem::paint - draw on canvas */
    void paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget );

    /** Search the nearest point in shape within a maximal area. Returns the
     * index of the nearest point or -1.
     * @param point is where a shape's point is searched
     * @param searchInRadius is a flag to indicate if the area of research is
     * limited in space.
     * @param radius is only used if searchInRadius is true
     */
    int pointAtPosition( const QPointF &point, const bool searchInRadius = true, const double radius = 10 );

    /** Sets state from Dom document
     * @param itemElem is Dom node corresponding to item tag
     * @param doc is Dom document
     */
    bool readXML( const QDomElement& itemElem, const QDomDocument& doc );

    /** Remove a point from the shape.
     * @param index of the point to delete
     */
    bool removePoint( const int index );

    /** Returns the number of points in the shape. */
    int pointsSize();

    /** Select a point.
     * @param index the point to select
     */
    bool setSelectedPoint( const int index );

    /** Unselect a point.
     */
    void unselectPoint();

    /** Stores state in Dom element
     * @param elem is Dom element corresponding to 'Composer' tag
     * @param doc write template file
     */
    bool writeXML( QDomElement& elem, QDomDocument & doc ) const;

  protected:

    /** Method called in addPoint. */
    virtual bool _addPoint( const int pointIndex, const QPointF &newPoint, const double radius ) = 0;

    /** Method called in paint. */
    virtual void _draw( QPainter *painter ) = 0;

    /** Method called in readXML. */
    virtual void _readXMLStyle( const QDomElement &elmt ) = 0;

    /** Method called in writeXML. */
    virtual void _writeXMLStyle( QDomDocument &doc, QDomElement &elmt ) const = 0;

    /** Rescale the current shape according to the boudning box. Useful when
     * the shape is resized thanks to the rubber band. */
    void rescaleToFitBoundingBox();

    /** Compute an euclidian distance between 2 points. */
    double computeDistance(const QPointF &pt1, const QPointF &pt2) const;

    /** Convert scene coordinate to item coordinates */
    QPointF convertToItemCoordinate(const QPointF &point);

    /** Update the current scene rectangle for this item. */
    void updateSceneRect();
};
