
#ifndef QGSVECTORCOLORBREWERCOLORRAMPV2DIALOG_H
#define QGSVECTORCOLORBREWERCOLORRAMPV2DIALOG_H

#include <QDialog>

#include "ui_qgsvectorcolorbrewercolorrampv2dialogbase.h"

class QgsVectorColorBrewerColorRampV2;

class QgsVectorColorBrewerColorRampV2Dialog : public QDialog, private Ui::QgsVectorColorBrewerColorRampV2DialogBase
{
  Q_OBJECT
      
public:
  QgsVectorColorBrewerColorRampV2Dialog(QgsVectorColorBrewerColorRampV2* ramp, QWidget* parent = NULL);
  
public slots:
  void setSchemeName();
  void setColors();

  void populateVariants();

protected:
  
  void updatePreview();

  QgsVectorColorBrewerColorRampV2* mRamp;
};

#endif
