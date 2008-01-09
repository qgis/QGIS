
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

    void addPoint(const QgsPoint & p, int geometryIndex);

//!Removes the last point. Most usefull in connection with undo operations
    void removeLastPoint(int geometryIndex = 0);	

    void movePoint(const QgsPoint & p, int geometryIndex);
    void movePoint(int index, const QgsPoint& p, int geometryIndex);

/**Sets this rubber band to the geometry of an existing feature.
     This is usefull for feature highlighting.
    @param geom the geometry object
    @param layer the layer containing the feature (used for coord transformation)
    @param render the maprender object (used for coord transformation)*/
    void setToGeometry(QgsGeometry* geom, QgsVectorLayer& layer);

/**Adds translation to original coordinates (all in map coordinates)*/
    void setTranslationOffset(double dx, double dy);

 /**Returns count of vertices in all lists of mPoint*/
    int numberOfVertices() const;

  protected:
    virtual void paint(QPainter* p);
    
    //! recalculates needed rectangle
    void updateRect();

};

