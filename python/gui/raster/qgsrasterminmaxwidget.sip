class QgsRasterMinMaxWidget: QWidget
{
%TypeHeaderCode
#include <qgsrasterminmaxwidget.h>
%End
  public:
    QgsRasterMinMaxWidget( QgsRasterLayer* theLayer, QWidget *parent = 0 );
    ~QgsRasterMinMaxWidget();

    void setExtent( const QgsRectangle & theExtent );

    void setBands( const QList<int> & theBands );

  signals:
    void load( int theBandNo, double theMin, double theMax, int origin );
};
