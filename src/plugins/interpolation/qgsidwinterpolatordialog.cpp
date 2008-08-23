#include "qgsidwinterpolatordialog.h"
#include "qgsidwinterpolator.h"

QgsIDWInterpolatorDialog::QgsIDWInterpolatorDialog(QWidget* parent, QgisInterface* iface): QgsInterpolatorDialog(parent, iface)
{
  setupUi(this);
}

QgsIDWInterpolatorDialog::~QgsIDWInterpolatorDialog()
{

}

QgsInterpolator* QgsIDWInterpolatorDialog::createInterpolator() const
{
  QList<QgsVectorLayer*> inputLayerList;

  QList< QPair <QgsVectorLayer*, QgsInterpolator::InputType> >::const_iterator data_it = mInputData.constBegin();
  for(; data_it != mInputData.constEnd(); ++data_it)
    {
      inputLayerList.push_back(data_it->first);
    }

  QgsIDWInterpolator* theInterpolator = new QgsIDWInterpolator(inputLayerList);
  theInterpolator->setDistanceCoefficient(mPSpinBox->value());
  return theInterpolator;
}
