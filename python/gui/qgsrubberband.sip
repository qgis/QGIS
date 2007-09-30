
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
    void addPoint(const QgsPoint & p);

    // ! Remove last point
    void removePoint(bool update = true);

    void movePoint(const QgsPoint & p);
    void movePoint(int index, const QgsPoint& p);

    int size() const;
    const QList<QgsPoint>& getPoints() const;
    const QgsPoint& getPoint(int index) const;

  protected:
    virtual void paint(QPainter* p);
    
    //! recalculates needed rectangle
    void updateRect();

};

