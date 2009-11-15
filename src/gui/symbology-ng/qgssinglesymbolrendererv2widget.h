#ifndef QGSSINGLESYMBOLRENDERERV2WIDGET_H
#define QGSSINGLESYMBOLRENDERERV2WIDGET_H

#include "qgsrendererv2widget.h"

class QgsSingleSymbolRendererV2;
class QgsSymbolV2SelectorDialog;

class QgsSingleSymbolRendererV2Widget : public QgsRendererV2Widget
{
  Q_OBJECT

public:

  static QgsRendererV2Widget* create(QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer);

  QgsSingleSymbolRendererV2Widget(QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer);
  ~QgsSingleSymbolRendererV2Widget();

  virtual QgsFeatureRendererV2* renderer();

public slots:
  void changeSingleSymbol();

protected:
  QgsSingleSymbolRendererV2* mRenderer;
  QgsSymbolV2SelectorDialog* mSelector;
  QgsSymbolV2* mSingleSymbol;
};


#endif // QGSSINGLESYMBOLRENDERERV2WIDGET_H
