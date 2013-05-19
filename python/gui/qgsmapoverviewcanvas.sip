
class QgsMapOverviewCanvas : QWidget
{
%TypeHeaderCode
#include <qgsmapoverviewcanvas.h>
%End

  public:
    QgsMapOverviewCanvas( QWidget * parent = 0, QgsMapCanvas* mapCanvas = NULL );

    ~QgsMapOverviewCanvas();

    //! used for overview canvas to reflect changed extent in main map canvas
    void drawExtentRect();

    //! renders overview and updates panning widget
    void refresh();

    //! changes background color
    void setBackgroundColor( const QColor& color );

    //! updates layer set for overview
    void setLayerSet( const QStringList& layerSet );

    QStringList& layerSet();

    void enableAntiAliasing( bool flag );

    void updateFullExtent();

  public slots:

    void hasCrsTransformEnabled( bool flag );

    void destinationSrsChanged();

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

