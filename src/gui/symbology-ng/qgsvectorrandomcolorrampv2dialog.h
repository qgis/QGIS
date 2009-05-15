
#ifndef QGSVECTORRANDOMCOLORRAMPV2DIALOG_H
#define QGSVECTORRANDOMCOLORRAMPV2DIALOG_H

#include <QDialog>

#include "ui_qgsvectorrandomcolorrampv2dialogbase.h"

class QgsVectorRandomColorRampV2;

class QgsVectorRandomColorRampV2Dialog : public QDialog, private Ui::QgsVectorRandomColorRampV2DialogBase
{
  Q_OBJECT
      
public:
  QgsVectorRandomColorRampV2Dialog(QgsVectorRandomColorRampV2* ramp, QWidget* parent = NULL);
  
public slots:
  void setCount( int val );
  void setHue1( int val );
  void setHue2( int val );
  void setSat1( int val );
  void setSat2( int val );
  void setVal1( int val );
  void setVal2( int val );

protected:
  
  void updatePreview();
  
  QgsVectorRandomColorRampV2* mRamp;
};

#endif
