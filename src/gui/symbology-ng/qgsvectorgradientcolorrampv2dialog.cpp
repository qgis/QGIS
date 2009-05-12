
#include "qgsvectorgradientcolorrampv2dialog.h"

#include "qgsvectorcolorrampv2.h"

#include <QColorDialog>

static void updateColorButton(QAbstractButton* button, QColor color)
{
  QPixmap p(20,20);
  p.fill(color);
  button->setIcon(QIcon(p));
}

/////////


QgsVectorGradientColorRampV2Dialog::QgsVectorGradientColorRampV2Dialog(QgsVectorGradientColorRampV2* ramp, QWidget* parent)
  : QDialog(parent), mRamp(ramp)
{

  setupUi(this);
  
  connect(btnColor1, SIGNAL(clicked()), this, SLOT(setColor1()));
  connect(btnColor2, SIGNAL(clicked()), this, SLOT(setColor2()));
  
  updatePreview();
}

void QgsVectorGradientColorRampV2Dialog::updatePreview()
{
  QSize size(300,40);
  lblPreview->setPixmap(QgsSymbolLayerV2Utils::colorRampPreviewPixmap(mRamp, size));
  
  updateColorButton(btnColor1, mRamp->color1());
  updateColorButton(btnColor2, mRamp->color2());
}

void QgsVectorGradientColorRampV2Dialog::setColor1()
{
  QColor color = QColorDialog::getColor(mRamp->color1(), this);
  if (!color.isValid())
    return;
  mRamp->setColor1(color);
  updatePreview();
}

void QgsVectorGradientColorRampV2Dialog::setColor2()
{
  QColor color = QColorDialog::getColor(mRamp->color2(), this);
  if (!color.isValid())
    return;
  mRamp->setColor2(color);
  updatePreview();
}
