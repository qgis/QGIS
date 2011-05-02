#include "qgsidwinterpolatordialog.h"
#include "qgsidwinterpolator.h"

QgsIDWInterpolatorDialog::QgsIDWInterpolatorDialog( QWidget* parent, QgisInterface* iface ): QgsInterpolatorDialog( parent, iface )
{
  setupUi( this );
}

QgsIDWInterpolatorDialog::~QgsIDWInterpolatorDialog()
{

}

QgsInterpolator* QgsIDWInterpolatorDialog::createInterpolator() const
{
  QgsIDWInterpolator* theInterpolator = new QgsIDWInterpolator( mInputData );
  theInterpolator->setDistanceCoefficient( mPSpinBox->value() );
  return theInterpolator;
}
