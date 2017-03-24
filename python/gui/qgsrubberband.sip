class QgsRubberBand: QgsMapCanvasItem
{
%TypeHeaderCode
#include <qgsrubberband.h>
%End

  public:

    enum IconType
    {
      ICON_NONE,
      ICON_CROSS,
      ICON_X,
      ICON_BOX,
      ICON_CIRCLE,
      ICON_FULL_BOX
    };

    QgsRubberBand( QgsMapCanvas* mapCanvas /TransferThis/, QgsWkbTypes::GeometryType geometryType = QgsWkbTypes::LineGeometry );
    ~QgsRubberBand();

    void setColor( const QColor & color );

    void setFillColor( const QColor & color );

    QColor fillColor() const;

    void setStrokeColor( const QColor & color );

    QColor strokeColor() const;

    void setSecondaryStrokeColor( const QColor &color );

    QColor secondaryStrokeColor() const;

    void setWidth( int width );

    int width() const;

    void setIcon( IconType icon );

    IconType icon() const;

    void setIconSize( int iconSize );

    int iconSize() const;

    void setLineStyle( Qt::PenStyle penStyle );

    void setBrushStyle( Qt::BrushStyle brushStyle );

    void reset( QgsWkbTypes::GeometryType geometryType = QgsWkbTypes::LineGeometry );

    void addPoint( const QgsPoint & p, bool doUpdate = true, int geometryIndex = 0 );

    void closePoints( bool doUpdate = true, int geometryIndex = 0 );

    void removePoint( int index = 0, bool doUpdate = true, int geometryIndex = 0 );

    void removeLastPoint( int geometryIndex = 0, bool doUpdate = true );

    void movePoint( const QgsPoint & p, int geometryIndex = 0 );

    void movePoint( int index, const QgsPoint& p, int geometryIndex = 0 );

    int partSize( int geometryIndex ) const;

    void setToGeometry( const QgsGeometry& geom, QgsVectorLayer* layer );

    void setToCanvasRectangle( QRect rect );

    void addGeometry( const QgsGeometry& geom, QgsVectorLayer* layer );

    void setTranslationOffset( double dx, double dy );

    int size() const;

    int numberOfVertices() const;

    const QgsPoint *getPoint( int i, int j = 0 ) const;

    QgsGeometry asGeometry() const;

    virtual void updatePosition();

  protected:
    virtual void paint( QPainter* p );

    void drawShape( QPainter *p, QVector<QPointF> &pts );

    void updateRect();

};
