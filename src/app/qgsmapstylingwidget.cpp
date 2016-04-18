#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QSizePolicy>

#include "qgsapplication.h"
#include "qgslabelingwidget.h"
#include "qgsmapstylingwidget.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayer.h"

QgsMapStylingWidget::QgsMapStylingWidget( QgsMapCanvas* canvas, QWidget *parent ) :
    QWidget( parent ), mMapCanvas( canvas ), mBlockAutoApply( false ), mCurrentLayer( nullptr )
{
  QBoxLayout* layout = new QVBoxLayout();
  layout->setContentsMargins( 0, 0, 0, 0 );
  this->setLayout( layout );

  mStackedWidget = new QStackedWidget( this );
  mMapStyleTabs = new QTabWidget( this );
  mMapStyleTabs->setDocumentMode( true );
  mNotSupportedPage = mStackedWidget->addWidget( new QLabel( "Not supported currently" ) );
  mVectorPage = mStackedWidget->addWidget( mMapStyleTabs );

  layout->addWidget( mStackedWidget );
  mButtonBox = new QDialogButtonBox( QDialogButtonBox::Reset | QDialogButtonBox::Apply );
  mLiveApplyCheck = new QCheckBox( "Live update" );
  mLiveApplyCheck->setChecked( true );

  QHBoxLayout* bottomLayout = new QHBoxLayout( );
  bottomLayout->addWidget( mButtonBox );
  bottomLayout->addWidget( mLiveApplyCheck );
  layout->addLayout( bottomLayout );

  mLabelingWidget = new QgsLabelingWidget( 0, mMapCanvas, this );
  mLabelingWidget->setDockMode( true );
  connect( mLabelingWidget, SIGNAL( widgetChanged() ), this, SLOT( autoApply() ) );

  // Only labels for now but styles and diagrams will come later
//  int styleTabIndex = mMapStyleTabs->addTab( new QWidget(), QgsApplication::getThemeIcon( "propertyicons/symbology.png" ), "Styles" );
  mLabelTabIndex = mMapStyleTabs->addTab( mLabelingWidget, QgsApplication::getThemeIcon( "labelingSingle.svg" ), "Labeling" );
//  int diagramTabIndex = mMapStyleTabs->addTab( new QWidget(), QgsApplication::getThemeIcon( "propertyicons/diagram.png" ), "Diagrams" );
//  mMapStyleTabs->setTabEnabled( styleTabIndex, false );
//  mMapStyleTabs->setTabEnabled( diagramTabIndex, false );
  mMapStyleTabs->setCurrentIndex( mLabelTabIndex );

  connect( mLiveApplyCheck, SIGNAL( toggled( bool ) ), mButtonBox->button( QDialogButtonBox::Apply ), SLOT( setDisabled( bool ) ) );

  connect( mButtonBox->button( QDialogButtonBox::Apply ), SIGNAL( clicked() ), this, SLOT( apply() ) );
  connect( mButtonBox->button( QDialogButtonBox::Reset ), SIGNAL( clicked() ), this, SLOT( resetSettings() ) );

  mButtonBox->button( QDialogButtonBox::Apply )->setEnabled( false );
  mButtonBox->button( QDialogButtonBox::Reset )->setEnabled( false );

}

void QgsMapStylingWidget::setLayer( QgsMapLayer *layer )
{
  if ( !layer )
    return;

  mBlockAutoApply = true;

  mCurrentLayer = layer;

  if ( layer->type() == QgsMapLayer::VectorLayer )
  {
    mStackedWidget->setCurrentIndex( mVectorPage );
    // TODO Once there is support for more then just labels
    // we need to add a check for the just the current tab
    mMapStyleTabs->setCurrentIndex( mLabelTabIndex );
    mLabelingWidget->setLayer( layer );
  }
  else if ( layer->type() == QgsMapLayer::RasterLayer )
  {
    mStackedWidget->setCurrentIndex( mNotSupportedPage );
  }
  else if ( layer->type() == QgsMapLayer::PluginLayer )
  {
    mStackedWidget->setCurrentIndex( mNotSupportedPage );
  }

  mBlockAutoApply = false;

  mButtonBox->button( QDialogButtonBox::Reset )->setEnabled( false );
}

void QgsMapStylingWidget::apply()
{
  if ( mStackedWidget->currentIndex() == mVectorPage &&
       mMapStyleTabs->currentIndex() == mLabelTabIndex )
  {
    mLabelingWidget->apply();
    mButtonBox->button( QDialogButtonBox::Reset )->setEnabled( true );
    emit styleChanged( mCurrentLayer );
  }
}

void QgsMapStylingWidget::autoApply()
{
  if ( mLiveApplyCheck->isChecked() && !mBlockAutoApply )
    apply();
}

void QgsMapStylingWidget::resetSettings()
{
  if ( mStackedWidget->currentIndex() == mVectorPage &&
       mMapStyleTabs->currentIndex() == mLabelTabIndex )
  {
    mLabelingWidget->resetSettings();
  }
}
