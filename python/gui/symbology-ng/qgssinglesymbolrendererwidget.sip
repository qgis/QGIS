class QgsSingleSymbolRendererWidget : QgsRendererWidget
{
%TypeHeaderCode
#include <qgssinglesymbolrendererwidget.h>
%End
  public:
    static QgsRendererWidget* create( QgsVectorLayer* layer, QgsStyle* style, QgsFeatureRenderer* renderer ) /Factory/;

    QgsSingleSymbolRendererWidget( QgsVectorLayer* layer, QgsStyle* style, QgsFeatureRenderer* renderer );
    ~QgsSingleSymbolRendererWidget();

    virtual QgsFeatureRenderer* renderer();

    virtual void setContext( const QgsSymbolWidgetContext& context );

    virtual void setDockMode( bool dockMode );

  public slots:
    void changeSingleSymbol();

    void showSymbolLevels();
};
