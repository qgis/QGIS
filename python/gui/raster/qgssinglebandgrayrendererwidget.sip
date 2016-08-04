class QgsSingleBandGrayRendererWidget: QgsRasterRendererWidget
{
%TypeHeaderCode
#include <qgssinglebandgrayrendererwidget.h>
%End
  public:
    QgsSingleBandGrayRendererWidget( QgsRasterLayer* layer, const QgsRectangle &extent = QgsRectangle() );
    ~QgsSingleBandGrayRendererWidget();

    static QgsRasterRendererWidget* create( QgsRasterLayer* layer, const QgsRectangle &theExtent ) /Factory/;

    QgsRasterRenderer* renderer();
    void setMapCanvas( QgsMapCanvas* canvas );

    void setFromRenderer( const QgsRasterRenderer* r );

    QString min( int index = 0 );
    QString max( int index = 0 );
    void setMin( const QString& value, int index = 0 );
    void setMax( const QString& value, int index = 0 );
    int selectedBand( int index = 0 );

  public slots:
    void loadMinMax( int theBandNo, double theMin, double theMax, int theOrigin );
};
