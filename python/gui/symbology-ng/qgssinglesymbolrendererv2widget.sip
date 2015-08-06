class QgsSingleSymbolRendererV2Widget : QgsRendererV2Widget
{
%TypeHeaderCode
#include <qgssinglesymbolrendererv2widget.h>
%End
  public:
    static QgsRendererV2Widget* create( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer ) /Factory/;

    QgsSingleSymbolRendererV2Widget( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer );
    ~QgsSingleSymbolRendererV2Widget();

    virtual QgsFeatureRendererV2* renderer();

  public slots:
    void changeSingleSymbol();

    void sizeScaleFieldChanged( QString fldName );
    void scaleMethodChanged( QgsSymbolV2::ScaleMethod scaleMethod );

    void showSymbolLevels();
};
