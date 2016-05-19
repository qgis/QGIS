#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QSizePolicy>
#include <QUndoStack>

#include "qgsapplication.h"
#include "qgslabelingwidget.h"
#include "qgsmapstylingwidget.h"
#include "qgsrendererv2propertiesdialog.h"
#include "qgsrendererrasterpropertieswidget.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayer.h"
#include "qgsstylev2.h"
#include "qgsvectorlayer.h"
#include "qgsproject.h"
#include "qgsundowidget.h"
#include "qgsrendererv2.h"
#include "qgsrendererv2registry.h"
#include "qgsmaplayerregistry.h"
#include "qgsrasterlayer.h"

QgsMapStylingWidget::QgsMapStylingWidget( QgsMapCanvas* canvas, QWidget *parent )
    : QWidget( parent )
    , mMapCanvas( canvas )
    , mBlockAutoApply( false )
    , mCurrentLayer( nullptr )
    , mVectorStyleWidget( nullptr )
{
  QBoxLayout* layout = new QVBoxLayout();
  layout->setContentsMargins( 0, 0, 0, 0 );
  this->setLayout( layout );

  connect( QgsMapLayerRegistry::instance(), SIGNAL( layerWillBeRemoved( QgsMapLayer* ) ), this, SLOT( layerAboutToBeRemoved( QgsMapLayer* ) ) );

  mAutoApplyTimer = new QTimer( this );
  mAutoApplyTimer->setSingleShot( true );
  connect( mAutoApplyTimer, SIGNAL( timeout() ), this, SLOT( apply() ) );

  mStackedWidget = new QStackedWidget( this );
  mVectorLayerTabs = new QTabWidget( this );
  mVectorLayerTabs->setDocumentMode( true );
  mRasterLayerTabs = new QTabWidget( this );
  mRasterLayerTabs->setDocumentMode( true );
  mNotSupportedPage = mStackedWidget->addWidget( new QLabel( "Not supported currently" ) );
  mVectorPage = mStackedWidget->addWidget( mVectorLayerTabs );
  mRasterPage = mStackedWidget->addWidget( mRasterLayerTabs );

  // create undo widget
  mUndoWidget = new QgsUndoWidget( this->mVectorLayerTabs, mMapCanvas );
  mUndoWidget->setObjectName( "Undo Styles" );

  mLayerTitleLabel = new QLabel();
  mLayerTitleLabel->setAlignment( Qt::AlignHCenter );
  layout->addWidget( mLayerTitleLabel );
  layout->addWidget( mStackedWidget );
  mButtonBox = new QDialogButtonBox( QDialogButtonBox::Apply );
  mLiveApplyCheck = new QCheckBox( "Live update" );
  mLiveApplyCheck->setChecked( true );

  mUndoButton = new QToolButton( this );
  mUndoButton->setIcon( QgsApplication::getThemeIcon( "mActionUndo.png" ) );
  mRedoButton = new QToolButton( this );
  mRedoButton->setIcon( QgsApplication::getThemeIcon( "mActionRedo.png" ) );

  connect( mUndoButton, SIGNAL( pressed() ), mUndoWidget, SLOT( undo() ) );
  connect( mRedoButton, SIGNAL( pressed() ), mUndoWidget, SLOT( redo() ) );

  QHBoxLayout* bottomLayout = new QHBoxLayout( );
  bottomLayout->addWidget( mUndoButton );
  bottomLayout->addWidget( mRedoButton );
  bottomLayout->addWidget( mButtonBox );
  bottomLayout->addWidget( mLiveApplyCheck );
  layout->addLayout( bottomLayout );

  mLabelingWidget = new QgsLabelingWidget( 0, mMapCanvas, this );
  mLabelingWidget->setDockMode( true );
  connect( mLabelingWidget, SIGNAL( widgetChanged() ), this, SLOT( autoApply() ) );

  QScrollArea* stylescroll = new QScrollArea;
  stylescroll->setWidgetResizable( true );
  stylescroll->setFrameStyle( QFrame::NoFrame );

  QScrollArea* labelscroll = new QScrollArea;
  labelscroll->setWidgetResizable( true );
  labelscroll->setFrameStyle( QFrame::NoFrame );
  labelscroll->setWidget( mLabelingWidget );

  mVectorStyleTabIndex = mVectorLayerTabs->addTab( stylescroll, QgsApplication::getThemeIcon( "propertyicons/symbology.png" ), "Style" );
  mVectorLabelTabIndex = mVectorLayerTabs->addTab( labelscroll, QgsApplication::getThemeIcon( "labelingSingle.svg" ), "Labeling" );
  mVectorLayerTabs->addTab( mUndoWidget, QgsApplication::getThemeIcon( "labelingSingle.svg" ), "History" );
//  int diagramTabIndex = mMapStyleTabs->addTab( new QWidget(), QgsApplication::getThemeIcon( "propertyicons/diagram.png" ), "Diagrams" );
//  mMapStyleTabs->setTabEnabled( styleTabIndex, false );
//  mMapStyleTabs->setTabEnabled( diagramTabIndex, false );
  mVectorLayerTabs->setCurrentIndex( mVectorStyleTabIndex );

  QScrollArea* rasterstylescroll = new QScrollArea;
  rasterstylescroll->setWidgetResizable( true );
  rasterstylescroll->setFrameStyle( QFrame::NoFrame );

  mRasterStyleTabIndex = mRasterLayerTabs->addTab( rasterstylescroll, QgsApplication::getThemeIcon( "propertyicons/symbology.png" ), "Style" );

  connect( mVectorLayerTabs, SIGNAL( currentChanged( int ) ), this, SLOT( updateCurrentWidgetLayer( int ) ) );
  connect( mRasterLayerTabs, SIGNAL( currentChanged( int ) ), this, SLOT( updateCurrentWidgetLayer( int ) ) );

  connect( mLiveApplyCheck, SIGNAL( toggled( bool ) ), mButtonBox->button( QDialogButtonBox::Apply ), SLOT( setDisabled( bool ) ) );

  connect( mButtonBox->button( QDialogButtonBox::Apply ), SIGNAL( clicked() ), this, SLOT( apply() ) );

  mButtonBox->button( QDialogButtonBox::Apply )->setEnabled( false );

}

void QgsMapStylingWidget::setLayer( QgsMapLayer *layer )
{
  if ( !layer || !layer->isSpatial() )
  {
    mLayerTitleLabel->clear();
    mStackedWidget->setCurrentIndex( mNotSupportedPage );
    return;
  }

  mCurrentLayer = layer;

  // TODO Adjust for raster
  updateCurrentWidgetLayer();
}

void QgsMapStylingWidget::apply()
{
  if ( mStackedWidget->currentIndex() == mVectorPage )
  {
    QString undoName = "Style Change";
    int currentPage = mVectorLayerTabs->currentIndex();
    if ( currentPage == mVectorLabelTabIndex )
    {
      mLabelingWidget->apply();
      emit styleChanged( mCurrentLayer );
      undoName = "Label Change";
    }
    else if ( currentPage == mVectorStyleTabIndex )
    {
      mVectorStyleWidget->apply();
      QgsProject::instance()->setDirty( true );
      mMapCanvas->clearCache();
      mMapCanvas->refresh();
      emit styleChanged( mCurrentLayer );
      QgsVectorLayer* layer = qobject_cast<QgsVectorLayer*>( mCurrentLayer );
      QgsRendererV2AbstractMetadata* m = QgsRendererV2Registry::instance()->rendererMetadata( layer->rendererV2()->type() );
      undoName = QString( "Style Change - %1" ).arg( m->visibleName() );
    }
    QString errorMsg;
    QDomDocument doc( "style" );
    QDomElement rootNode = doc.createElement( "qgis" );
    doc.appendChild( rootNode );
    mCurrentLayer->writeStyle( rootNode, doc, errorMsg );
    mCurrentLayer->undoStackStyles()->beginMacro( undoName );
    mCurrentLayer->undoStackStyles()->push( new QgsMapLayerStyleCommand( mCurrentLayer, rootNode, mLastStyleXml ) );
    mCurrentLayer->undoStackStyles()->endMacro();
    // Override the last style on the stack
    mLastStyleXml = rootNode.cloneNode();
  }
  else if ( mStackedWidget->currentIndex() == mRasterPage )
  {
    int currentPage = mRasterLayerTabs->currentIndex();
    if ( currentPage == mRasterStyleTabIndex )
    {
      mRasterStyleWidget->apply();
      emit styleChanged( mCurrentLayer );
      QgsProject::instance()->setDirty( true );
      mMapCanvas->clearCache();
      mMapCanvas->refresh();
    }

  }
}

void QgsMapStylingWidget::autoApply()
{
  if ( mLiveApplyCheck->isChecked() && !mBlockAutoApply )
  {
    mAutoApplyTimer->start( 100 );
  }
}

void QgsMapStylingWidget::updateCurrentWidgetLayer()
{
  mBlockAutoApply = true;

  QgsMapLayer* layer = mCurrentLayer;
  mUndoWidget->setUndoStack( layer->undoStackStyles() );

  mLayerTitleLabel->setText( layer->name() );

  if ( layer->type() == QgsMapLayer::VectorLayer )
  {
    mStackedWidget->setCurrentIndex( mVectorPage );
    int currentPage = mVectorLayerTabs->currentIndex();
    if ( currentPage == mVectorLabelTabIndex )
    {
      mLabelingWidget->setLayer( layer );
    }
    if ( currentPage == mVectorStyleTabIndex )
    {
      // TODO Refactor props dialog so we don't have to do this
      QScrollArea* area = qobject_cast<QScrollArea*>( mVectorLayerTabs->widget( mVectorStyleTabIndex ) );
      QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer*>( layer );
      mVectorStyleWidget = new QgsRendererV2PropertiesDialog( vlayer, QgsStyleV2::defaultStyle(), true );
      connect( mVectorStyleWidget, SIGNAL( widgetChanged() ), this, SLOT( autoApply() ) );
      area->setWidget( mVectorStyleWidget );
    }
    QString errorMsg;
    QDomDocument doc( "style" );
    mLastStyleXml = doc.createElement( "style" );
    doc.appendChild( mLastStyleXml );
    mCurrentLayer->writeSymbology( mLastStyleXml, doc, errorMsg );
  }
  else if ( layer->type() == QgsMapLayer::RasterLayer )
  {
    mStackedWidget->setCurrentIndex( mRasterPage );
    int currentPage = mRasterLayerTabs->currentIndex();
    if ( currentPage == mRasterStyleTabIndex )
    {
      // TODO Refactor props dialog so we don't have to do this
      QScrollArea* area = qobject_cast<QScrollArea*>( mRasterLayerTabs->widget( mRasterStyleTabIndex ) );
      QgsRasterLayer *rlayer = qobject_cast<QgsRasterLayer*>( layer );
      mRasterStyleWidget = new QgsRendererRasterPropertiesWidget( rlayer, mMapCanvas, area );
      connect( mRasterStyleWidget, SIGNAL( widgetChanged() ), this, SLOT( autoApply() ) );
      area->setWidget( mRasterStyleWidget );
    }
  }
  else if ( layer->type() == QgsMapLayer::PluginLayer )
  {
    mStackedWidget->setCurrentIndex( mNotSupportedPage );
  }

  mBlockAutoApply = false;
}

void QgsMapStylingWidget::layerAboutToBeRemoved( QgsMapLayer* layer )
{
  if ( layer == mCurrentLayer )
  {
    mAutoApplyTimer->stop();
    mStackedWidget->setCurrentIndex( mNotSupportedPage );
    mCurrentLayer = nullptr;
  }
}


QgsMapLayerStyleCommand::QgsMapLayerStyleCommand( QgsMapLayer *layer, const QDomNode &current, const QDomNode &last )
    : QUndoCommand()
    , mLayer( layer )
    , mXml( current )
    , mLastState( last )
{
}

void QgsMapLayerStyleCommand::undo()
{
  QString error;
  mLayer->readStyle( mLastState, error );
  mLayer->triggerRepaint();
}

void QgsMapLayerStyleCommand::redo()
{
  QString error;
  mLayer->readStyle( mXml, error );
  mLayer->triggerRepaint();
}
