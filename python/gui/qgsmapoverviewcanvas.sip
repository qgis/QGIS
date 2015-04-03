
class QgsMapOverviewCanvas : QWidget
{
%TypeHeaderCode
#include <qgsmapoverviewcanvas.h>
%End

  public:
    QgsMapOverviewCanvas( QWidget * parent /TransferThis/ = 0, QgsMapCanvas* mapCanvas = NULL );

    ~QgsMapOverviewCanvas();

    //! renders overview and updates panning widget
    void refresh();

    //! changes background color
    void setBackgroundColor( const QColor& color );

    //! updates layer set for overview
    void setLayerSet( const QStringList& layerSet );

    QStringList layerSet() const;

    void enableAntiAliasing( bool flag );

    void updateFullExtent();

  public slots:

    // ### QGIS 3: make protected
    //! used for overview canvas to reflect changed extent in main map canvas
    void drawExtentRect();

    // ### QGIS 3: rename so it does not look like getter, make protected
    void hasCrsTransformEnabled( bool flag );

    // ### QGIS 3: rename Srs to Crs, make protected
    void destinationSrsChanged();

  protected slots:
    void mapRenderingFinished();
    void layerRepaintRequested();

  protected:

    //! Overridden paint event
    void paintEvent( QPaintEvent * pe );

    //! Overridden resize event
    void resizeEvent( QResizeEvent * e );

    //! Overridden mouse move event
    void mouseMoveEvent( QMouseEvent * e );

    //! Overridden mouse press event
    void mousePressEvent( QMouseEvent * e );

    //! Overridden mouse release event
    void mouseReleaseEvent( QMouseEvent * e );

    //! called when panning to reflect mouse movement
    void updatePanningWidget( const QPoint& pos );
};

