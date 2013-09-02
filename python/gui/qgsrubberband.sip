class QgsRubberBand: QgsMapCanvasItem
{
%TypeHeaderCode
#include <qgsrubberband.h>
%End

  public:
    /** Icons
     *  Added in 1.9 */
    enum IconType
    {
      ICON_NONE,
      ICON_CROSS,
      ICON_X,
      ICON_BOX,
      ICON_CIRCLE
    };

    QgsRubberBand( QgsMapCanvas* mapCanvas /TransferThis/, QGis::GeometryType geometryType = QGis::Line );
    QgsRubberBand( QgsMapCanvas* mapCanvas /TransferThis/, bool isPolygon );
    ~QgsRubberBand();

    /** Set the color for the rubberband */
    void setColor( const QColor & color );

    /** Set the width of the line. Outline width for polygon. */
    void setWidth( int width );

    /** Set the icon type to highlight point geometries.
     *  Added in 1.9 */
    void setIcon( IconType icon );

    /** Set the size of the point icons
     *   Added in 1.9 */
    void setIconSize ( int iconSize );

    void setLineStyle( Qt::PenStyle penStyle );

    void setBrushStyle( Qt::BrushStyle brushStyle );

    void reset( QGis::GeometryType geometryType = QGis::Line );
    void reset( bool isPolygon );

    //! Add point to rubberband and update canvas
    //! If adding more points consider using update=false for better performance
    //! geometryIndex is the index of the feature part (in case of multipart geometries)
    void addPoint( const QgsPoint & p, bool update = true, int geometryIndex = 0 );

    /**Remove a vertex from the rubberband and (optionally) update canvas.
    @param index The index of the vertex/point to remove, negative indexes start at end
    @param doUpdate Should the map canvas be updated immediately?
    @param geometryIndex The index of the feature part (in case of multipart geometries)
    */
    void removePoint( int index = 0, bool doUpdate = true, int geometryIndex = 0 );

    //!Removes the last point. Most useful in connection with undo operations
    void removeLastPoint( int geometryIndex = 0 );

    void movePoint( const QgsPoint & p, int geometryIndex = 0 );
    /**Moves the rubber band point specified by index. Note that if the rubber band is
     not used to track the last mouse position, the first point of the rubber band has two vertices*/
    void movePoint( int index, const QgsPoint& p, int geometryIndex = 0 );

    /**Sets this rubber band to the geometry of an existing feature.
     This is useful for feature highlighting.
     In contrast to addGeometry, this method does also change the geometry type of the rubberband.
    @param geom the geometry object
    @param layer the layer containing the feature, used for coord transformation to map
    crs. In case of 0 pointer, the coordinates are not going to be transformed.
    */
    void setToGeometry( QgsGeometry* geom, QgsVectorLayer* layer );

    /**Sets this rubber band to a map canvas rectangle
      @param rect rectangle in canvas coordinates
      @note added in version 1.7*/
    void setToCanvasRectangle( const QRect& rect );

    /**Add the geometry of an existing feature to a rubberband
     This is useful for multi feature highlighting.
    @param geom the geometry object
    @param layer the layer containing the feature, used for coord transformation to map
    crs. In case of 0 pointer, the coordinates are not going to be transformed.
    @note added in 1.5
    */
    void addGeometry( QgsGeometry* geom, QgsVectorLayer* layer );

    /**Adds translation to original coordinates (all in map coordinates)*/
    void setTranslationOffset( double dx, double dy );

    /**Returns number of geometries
     * added in 1.5 */
    int size() const;

    /**Returns count of vertices in all lists of mPoint*/
    int numberOfVertices() const;

    /**Return vertex*/
    const QgsPoint *getPoint( int i, int j = 0 ) const;

    /**Returns the rubberband as a Geometry.
    * added in 1.6 */
    QgsGeometry* asGeometry();

  protected:
    virtual void paint( QPainter* p );

    //! recalculates needed rectangle
    void updateRect();

};
