#include "qgslabelingwidget.h"

#include "qgslabelengineconfigdialog.h"
#include "qgslabelinggui.h"
#include "qgsrulebasedlabelingwidget.h"
#include "qgsvectorlayerlabeling.h"

QgsLabelingWidget::QgsLabelingWidget( QgsVectorLayer* layer, QgsMapCanvas* canvas, QWidget* parent )
    : QWidget( parent )
    , mLayer( layer )
    , mCanvas( canvas )
{
  setupUi( this );

  connect( mEngineSettingsButton, SIGNAL( clicked() ), this, SLOT( showEngineConfigDialog() ) );

  mWidgetSimple = new QgsLabelingGui( layer, canvas, this );
  mWidgetRules = new QgsRuleBasedLabelingWidget( layer, canvas, this );
  mStackedWidget->addWidget( mWidgetSimple );
  mStackedWidget->addWidget( mWidgetRules );

  mStackedWidget->setCurrentIndex( 0 );
}

void QgsLabelingWidget::init()
{
  if ( !mLayer->labeling() || mLayer->labeling()->type() == "simple" )
  {
    mStackedWidget->setCurrentIndex( 0 );

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

    mWidgetSimple->init();
  }
  else if ( mLayer->labeling() && mLayer->labeling()->type() == "rule-based" )
  {
    mStackedWidget->setCurrentIndex( 1 );
    mWidgetRules->init();
  }
}

void QgsLabelingWidget::writeSettingsToLayer()
{
  if ( mLabelModeComboBox->currentIndex() < 3 )
  {
    mWidgetSimple->writeSettingsToLayer();
  }
  else
  {
    mWidgetRules->writeSettingsToLayer();
  }
}


void QgsLabelingWidget::on_mLabelModeComboBox_currentIndexChanged( int index )
{
  if ( index < 3 )
  {
    mStackedWidget->setCurrentIndex( 0 );
    mWidgetSimple->setLabelMode( ( QgsLabelingGui::LabelMode ) index );
  }
  else
  {
    // rule-based labeling
    mStackedWidget->setCurrentIndex( 1 );
  }
}

void QgsLabelingWidget::showEngineConfigDialog()
{
  QgsLabelEngineConfigDialog dlg( this );
  dlg.exec();
}
