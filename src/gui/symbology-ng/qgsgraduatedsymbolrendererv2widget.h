#ifndef QGSGRADUATEDSYMBOLRENDERERV2WIDGET_H
#define QGSGRADUATEDSYMBOLRENDERERV2WIDGET_H

#include "qgsrendererv2widget.h"

class QgsGraduatedSymbolRendererV2;

#include "ui_qgsgraduatedsymbolrendererv2widget.h"

class QgsGraduatedSymbolRendererV2Widget : public QgsRendererV2Widget, private Ui::QgsGraduatedSymbolRendererV2Widget
{
  Q_OBJECT

public:

  static QgsRendererV2Widget* create(QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer);

  QgsGraduatedSymbolRendererV2Widget(QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer);
  ~QgsGraduatedSymbolRendererV2Widget();

  virtual QgsFeatureRendererV2* renderer();

public slots:
  void changeGraduatedSymbol();
  void classifyGraduated();
  void rangesDoubleClicked(const QModelIndex & idx);

protected:
  void updateUiFromRenderer();

  void updateGraduatedSymbolIcon();

  //! populate column combos in categorized and graduated page
  void populateColumns();

  void populateColorRamps();

  //! populate ranges of graduated symbol renderer
  void populateRanges();

  void changeRangeSymbol(int rangeIdx);


protected:
  QgsGraduatedSymbolRendererV2* mRenderer;

  QgsSymbolV2* mGraduatedSymbol;
};


#endif // QGSGRADUATEDSYMBOLRENDERERV2WIDGET_H
