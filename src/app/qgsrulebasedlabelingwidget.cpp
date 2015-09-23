#include "qgsrulebasedlabelingwidget.h"

QgsRuleBasedLabelingWidget::QgsRuleBasedLabelingWidget( QgsVectorLayer* layer, QgsMapCanvas* canvas, QWidget* parent )
  : QWidget( parent )
  , mLayer( layer )
  , mCanvas( canvas )
{
  setupUi( this );
}

void QgsRuleBasedLabelingWidget::init()
{
  // TODO
}

void QgsRuleBasedLabelingWidget::writeSettingsToLayer()
{
  // TODO
}

