class QgsPalettedRendererWidget: QgsRasterRendererWidget
{
%TypeHeaderCode
#include <qgspalettedrendererwidget.h>
%End
  public:
    QgsPalettedRendererWidget( QgsRasterLayer* layer, const QgsRectangle &extent = QgsRectangle() );
    static QgsRasterRendererWidget* create( QgsRasterLayer* layer, const QgsRectangle &theExtent ) /Factory/;
    ~QgsPalettedRendererWidget();

    QgsRasterRenderer* renderer();

    void setFromRenderer( const QgsRasterRenderer* r );
};
