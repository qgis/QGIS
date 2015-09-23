#include "qgslabelingwidget.h"

#include "qgslabelengineconfigdialog.h"
#include "qgslabelinggui.h"
#include "qgsrulebasedlabelingwidget.h"
#include "qgsvectorlayerlabeling.h"

QgsLabelingWidget::QgsLabelingWidget( QgsVectorLayer* layer, QgsMapCanvas* canvas, QWidget* parent )
    : QWidget( parent )
    , mLayer( layer )
    , mCanvas( canvas )
    , mWidget( 0 )
{
  setupUi( this );

  connect( mEngineSettingsButton, SIGNAL( clicked() ), this, SLOT( showEngineConfigDialog() ) );

  mLabelModeComboBox->setCurrentIndex( -1 );

  connect( mLabelModeComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( labelModeChanged( int ) ) );

  // pick the right mode of the layer
  if ( mLayer->labeling() && mLayer->labeling()->type() == "rule-based" )
  {
    mLabelModeComboBox->setCurrentIndex( 3 );
  }
  else
  {
    // load labeling settings from layer
    QgsPalLayerSettings lyr;
    lyr.readFromLayer( mLayer );

    // enable/disable main options based upon whether layer is being labeled
    if ( !lyr.enabled )
    {
      mLabelModeComboBox->setCurrentIndex( 0 );
    }
    else
    {
      mLabelModeComboBox->setCurrentIndex( lyr.drawLabels ? 1 : 2 );
    }
  }
}

void QgsLabelingWidget::writeSettingsToLayer()
{
  if ( mLabelModeComboBox->currentIndex() == 3 )
  {
    qobject_cast<QgsRuleBasedLabelingWidget*>( mWidget )->writeSettingsToLayer();
  }
  else
  {
    qobject_cast<QgsLabelingGui*>( mWidget )->writeSettingsToLayer();
  }
}


void QgsLabelingWidget::labelModeChanged( int index )
{
  if ( index < 3 )
  {
    if ( QgsLabelingGui* widgetSimple = qobject_cast<QgsLabelingGui*>( mWidget ) )
    {
      // lighter variant - just change the mode of existing widget
      widgetSimple->setLabelMode(( QgsLabelingGui::LabelMode ) index );
      return;
    }
  }

  // in general case we need to recreate the widget

  if ( mWidget )
    mStackedWidget->removeWidget( mWidget );

  delete mWidget;
  mWidget = 0;

  if ( index == 3 )
  {
    mWidget = new QgsRuleBasedLabelingWidget( mLayer, mCanvas, this );
  }
  else
  {
    QgsLabelingGui* w = new QgsLabelingGui( mLayer, mCanvas, 0, this );
    w->setLabelMode(( QgsLabelingGui::LabelMode ) index );
    w->init();
    mWidget = w;
  }

  mStackedWidget->addWidget( mWidget );
  mStackedWidget->setCurrentWidget( mWidget );
}

void QgsLabelingWidget::showEngineConfigDialog()
{
  QgsLabelEngineConfigDialog dlg( this );
  dlg.exec();
}
