class QgsSingleBandPseudoColorRendererWidget : QgsRasterRendererWidget
{
%TypeHeaderCode
#include <qgssinglebandpseudocolorrendererwidget.h>
%End

  public:

    QgsSingleBandPseudoColorRendererWidget( QgsRasterLayer* layer, const QgsRectangle &extent = QgsRectangle() );
    ~QgsSingleBandPseudoColorRendererWidget();

    static QgsRasterRendererWidget* create( QgsRasterLayer* layer, const QgsRectangle &theExtent ) /Factory/;
    QgsRasterRenderer* renderer();
    void setMapCanvas( QgsMapCanvas* canvas );

    void setFromRenderer( const QgsRasterRenderer* r );

    void doComputations();

  public slots:

    /** Executes the single band pseudo raster classficiation
     */
    void classify();
    void loadMinMax( int theBandNo, double theMin, double theMax );

};

