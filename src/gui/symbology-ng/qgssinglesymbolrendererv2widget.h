#ifndef QGSSINGLESYMBOLRENDERERV2WIDGET_H
#define QGSSINGLESYMBOLRENDERERV2WIDGET_H

#include "qgsrendererv2widget.h"

class QgsSingleSymbolRendererV2;
class QgsSymbolV2SelectorDialog;

class QMenu;

class GUI_EXPORT QgsSingleSymbolRendererV2Widget : public QgsRendererV2Widget
{
    Q_OBJECT

  public:

    static QgsRendererV2Widget* create( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer );

    QgsSingleSymbolRendererV2Widget( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer );
    ~QgsSingleSymbolRendererV2Widget();

    virtual QgsFeatureRendererV2* renderer();

  public slots:
    void changeSingleSymbol();

    void rotationFieldChanged( QString fldName );
    void sizeScaleFieldChanged( QString fldName );

  protected:

    QgsSingleSymbolRendererV2* mRenderer;
    QgsSymbolV2SelectorDialog* mSelector;
    QgsSymbolV2* mSingleSymbol;

    QgsRendererV2DataDefinedMenus* mDataDefinedMenus;
};


#endif // QGSSINGLESYMBOLRENDERERV2WIDGET_H
