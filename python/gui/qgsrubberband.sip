class QgsRubberBand: QgsMapCanvasItem
{
%TypeHeaderCode
#include <qgsrubberband.h>
%End

  public:
    QgsRubberBand(QgsMapCanvas* mapCanvas /TransferThis/, bool isPolygon = false);
    ~QgsRubberBand();

    void setColor(const QColor & color);
    void setWidth(int width);

    void reset(bool isPolygon = false);

    //! Add point to rubberband and update canvas
    //! If adding more points consider using update=false for better performance
    //! geometryIndex is the index of the feature part (in case of multipart geometries)
    void addPoint(const QgsPoint & p, bool update = true, int geometryIndex = 0);

    //!Removes the last point. Most useful in connection with undo operations
    void removeLastPoint(int geometryIndex = 0);

    void movePoint(const QgsPoint & p, int geometryIndex = 0);
    /**Moves the rubber band point specified by index. Note that if the rubber band is 
     not used to track the last mouse position, the first point of the rubber band has two vertices*/
    void movePoint(int index, const QgsPoint& p, int geometryIndex = 0);

    /**Sets this rubber band to the geometry of an existing feature.
     This is useful for feature highlighting.
    @param geom the geometry object
    @param layer the layer containing the feature, used for coord transformation to map 
    crs. In case of 0 pointer, the coordinates are not going to be transformed.
    @param render the maprender object (used for coord transformation)*/
    void setToGeometry(QgsGeometry* geom, QgsVectorLayer* layer);

    /**Add the geometry of an existing feature to the rubberband.
     This is useful for multi feature highlighting.
    @param geom the geometry object
    @param layer the layer containing the feature, used for coord transformation to map 
    crs. In case of 0 pointer, the coordinates are not going to be transformed.
    @param render the maprender object (used for coord transformation)
    @added in 1.5
    */
    void addGeometry(QgsGeometry* geom, QgsVectorLayer* layer);

    /**Sets this rubber band to a map canvas rectangle
      @param rect rectangle in canvas coordinates
      @note added in version 1.7*/
    void setToCanvasRectangle( const QRect& rect );

    /**Adds translation to original coordinates (all in map coordinates)*/
    void setTranslationOffset(double dx, double dy);

    /**Return number of geometries 
     @added in 1.5
     */
    int size();

    /**Returns count of vertices in all lists of mPoint*/
    int numberOfVertices() const;

    /**Return vertex*/
    const QgsPoint *getPoint(int i, int j=0) const;
    
    /**Returns the rubberband as a Geometry.
    * added in 1.6 */
    QgsGeometry* asGeometry();

  protected:
    virtual void paint(QPainter* p);
    
    //! recalculates needed rectangle
    void updateRect();

};
