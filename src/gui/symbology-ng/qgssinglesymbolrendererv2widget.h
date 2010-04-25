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

    void rotationFieldSelected();
    void sizeScaleFieldSelected();

  protected:

    void populateMenu( QMenu* menu, const char* slot, QString fieldName );
    void updateMenu( QMenu* menu, QString fieldName );

    QgsSingleSymbolRendererV2* mRenderer;
    QgsSymbolV2SelectorDialog* mSelector;
    QgsSymbolV2* mSingleSymbol;

    QMenu* mRotationMenu;
    QMenu* mSizeScaleMenu;
};


#endif // QGSSINGLESYMBOLRENDERERV2WIDGET_H
