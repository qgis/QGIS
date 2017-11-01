#include "qgsattributeformwidget.h"

#include "qgsattributeform.h"

QgsAttributeFormWidget::QgsAttributeFormWidget( QgsWidgetWrapper *widget, QgsAttributeForm *form )
  : QWidget( form )
  , mMode( DefaultMode )
  , mForm( form )
{

}

void QgsAttributeFormWidget::setMode( QgsAttributeFormWidget::Mode mode )
{
  mMode = mode;
  updateWidgets();
}

QgsAttributeForm *QgsAttributeFormWidget::form() const
{
  return mForm;
}

QgsVectorLayer *QgsAttributeFormWidget::layer()
{
  QgsAttributeForm *aform = form();
  return aform ? aform->layer() : nullptr;
}
