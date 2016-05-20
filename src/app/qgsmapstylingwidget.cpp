#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QSizePolicy>
#include <QUndoStack>

#include "qgsapplication.h"
#include "qgslabelingwidget.h"
#include "qgsmapstylingwidget.h"
#include "qgsrastertransparencywidget.h"
#include "qgsrendererv2propertiesdialog.h"
#include "qgsrendererrasterpropertieswidget.h"
#include "qgsrasterhistogramwidget.h"
#include "qgsrasterrendererwidget.h"
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
    , mLabelingWidget( nullptr )
    , mVectorStyleWidget( nullptr )
    , mRasterStyleWidget( nullptr )
{
  QBoxLayout* layout = new QVBoxLayout();
  layout->setContentsMargins( 0, 0, 0, 0 );
  this->setLayout( layout );

  connect( QgsMapLayerRegistry::instance(), SIGNAL( layerWillBeRemoved( QgsMapLayer* ) ), this, SLOT( layerAboutToBeRemoved( QgsMapLayer* ) ) );

  mAutoApplyTimer = new QTimer( this );
  mAutoApplyTimer->setSingleShot( true );
  connect( mAutoApplyTimer, SIGNAL( timeout() ), this, SLOT( apply() ) );

  mStackedWidget = new QStackedWidget( this );
  mStyleTabs = new QTabWidget( this );
  mStyleTabs->setDocumentMode( true );

  mNotSupportedPage = mStackedWidget->addWidget( new QLabel( "Not supported currently" ) );
  mLayerPage = mStackedWidget->addWidget( mStyleTabs );

  // create undo widget
  mUndoWidget = new QgsUndoWidget( this->mStyleTabs, mMapCanvas );
  mUndoWidget->setObjectName( "Undo Styles" );

  mButtonBox = new QDialogButtonBox( QDialogButtonBox::Apply );

  QHBoxLayout* titleLayout = new QHBoxLayout( );
  mLayerTitleLabel = new QLabel();
  mLayerTitleLabel->setAlignment( Qt::AlignLeft );
  QWidget* spacer = new QWidget();
  spacer->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
  titleLayout->addWidget( mLayerTitleLabel );
  titleLayout->addWidget( spacer );

  layout->addLayout( titleLayout );
  layout->addWidget( mStackedWidget );

  mLiveApplyCheck = new QCheckBox( "Live update" );
  mLiveApplyCheck->setChecked( true );
  titleLayout->addWidget( mLiveApplyCheck );

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
  layout->addLayout( bottomLayout );

  connect( mStyleTabs, SIGNAL( currentChanged( int ) ), this, SLOT( updateCurrentWidgetLayer() ) );

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
    whileBlocking( mStyleTabs )->clear();
    return;
  }

  // TODO Don't clear and reload tabs if the layer type is the same
  bool clearTabs = true;
  if ( mCurrentLayer )
  {
    clearTabs = mCurrentLayer->type() != layer->type();
  }

  mCurrentLayer = layer;

  if ( clearTabs )
  {
    whileBlocking( mStyleTabs )->clear();

    if ( layer->type() == QgsMapLayer::VectorLayer )
    {
      QScrollArea* stylescroll = new QScrollArea;
      stylescroll->setWidgetResizable( true );
      stylescroll->setFrameStyle( QFrame::NoFrame );

      QScrollArea* labelscroll = new QScrollArea;
      labelscroll->setWidgetResizable( true );
      labelscroll->setFrameStyle( QFrame::NoFrame );

      mVectorStyleTabIndex = mStyleTabs->addTab( stylescroll, QgsApplication::getThemeIcon( "propertyicons/symbology.png" ), "Style" );
      mVectorLabelTabIndex = mStyleTabs->addTab( labelscroll, QgsApplication::getThemeIcon( "labelingSingle.svg" ), "Labeling" );
      mStyleTabs->addTab( mUndoWidget, QgsApplication::getThemeIcon( "labelingSingle.svg" ), "History" );
//  int diagramTabIndex = mMapStyleTabs->addTab( new QWidget(), QgsApplication::getThemeIcon( "propertyicons/diagram.png" ), "Diagrams" );

      whileBlocking( mStyleTabs )->setCurrentIndex( 0 );
    }
    else if ( layer->type() == QgsMapLayer::RasterLayer )
    {
      QScrollArea* rasterstylescroll = new QScrollArea;
      rasterstylescroll->setWidgetResizable( true );
      rasterstylescroll->setFrameStyle( QFrame::NoFrame );

      mRasterStyleTabIndex = mStyleTabs->addTab( rasterstylescroll, QgsApplication::getThemeIcon( "propertyicons/symbology.png" ), "Style" );

      QScrollArea* rastertransscroll = new QScrollArea;
      rastertransscroll->setWidgetResizable( true );
      rastertransscroll->setFrameStyle( QFrame::NoFrame );

      mRasterTransTabIndex = mStyleTabs->addTab( rastertransscroll, QgsApplication::getThemeIcon( "propertyicons/symbology.png" ), "Transparency" );

      QScrollArea* rasterhistsscroll = new QScrollArea;
      rasterhistsscroll->setWidgetResizable( true );
      rasterhistsscroll->setFrameStyle( QFrame::NoFrame );

      mRasterHistogramTabIndex = mStyleTabs->addTab( rasterhistsscroll, QgsApplication::getThemeIcon( "propertyicons/symbology.png" ), "Histrogram" );

      whileBlocking( mStyleTabs )->setCurrentIndex( 0 );
    }

    mStyleTabs->addTab( mUndoWidget, QgsApplication::getThemeIcon( "labelingSingle.svg" ), "History" );

  }

  updateCurrentWidgetLayer();
}

void QgsMapStylingWidget::apply()
{
  QString undoName = "Style Change";
  if ( mCurrentLayer->type() == QgsMapLayer::VectorLayer )
  {
    int currentPage = mStyleTabs->currentIndex();
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
    pushUndoItem( undoName );
  }
  else if ( mCurrentLayer->type() == QgsMapLayer::RasterLayer )
  {
    int currentPage = mStyleTabs->currentIndex();

    // Note: Histogram set the values on the renderer so we need to update that
    if ( currentPage == mRasterStyleTabIndex || currentPage == mRasterHistogramTabIndex )
    {
      mRasterStyleWidget->apply();
      emit styleChanged( mCurrentLayer );
      QgsProject::instance()->setDirty( true );
      mMapCanvas->clearCache();
      mMapCanvas->refresh();
    }
    else if ( currentPage == mRasterTransTabIndex )
    {
      QScrollArea* area = qobject_cast<QScrollArea*>( mStyleTabs->currentWidget() );
      QgsRasterTransparencyWidget* widget = qobject_cast<QgsRasterTransparencyWidget*>( area->widget() );
      widget->apply();
      emit styleChanged( mCurrentLayer );
      QgsProject::instance()->setDirty( true );
      mMapCanvas->clearCache();
      mMapCanvas->refresh();
    }
    pushUndoItem( undoName );
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
    mStackedWidget->setCurrentIndex( mLayerPage );

    QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer*>( layer );
    QScrollArea* area = qobject_cast<QScrollArea*>( mStyleTabs->currentWidget() );

    int currentPage = mStyleTabs->currentIndex();
    if ( currentPage == mVectorLabelTabIndex )
    {
      // The label widget is super heavy so we only do it once for now.
      if ( !mLabelingWidget )
      {
        mLabelingWidget = new QgsLabelingWidget( 0, mMapCanvas, this );
        mLabelingWidget->setDockMode( true );
        connect( mLabelingWidget, SIGNAL( widgetChanged() ), this, SLOT( autoApply() ) );
        area->setWidget( mLabelingWidget );
      }
      mLabelingWidget->setLayer( vlayer );
    }
    if ( currentPage == mVectorStyleTabIndex )
    {
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
    mStackedWidget->setCurrentIndex( mLayerPage );

    QgsRasterLayer *rlayer = qobject_cast<QgsRasterLayer*>( layer );
    QScrollArea* area = qobject_cast<QScrollArea*>( mStyleTabs->currentWidget() );

    int currentPage = mStyleTabs->currentIndex();
    if ( currentPage == mRasterStyleTabIndex )
    {
      mRasterStyleWidget = new QgsRendererRasterPropertiesWidget( rlayer, mMapCanvas, area );
      connect( mRasterStyleWidget, SIGNAL( widgetChanged() ), this, SLOT( autoApply() ) );
      area->setWidget( mRasterStyleWidget );
    }
    else if ( currentPage == mRasterTransTabIndex )
    {
      QgsRasterTransparencyWidget* widget = new QgsRasterTransparencyWidget( rlayer, mMapCanvas, area );
      connect( widget, SIGNAL( widgetChanged() ), this, SLOT( autoApply() ) );
      area->setWidget( widget );
    }
    else if ( currentPage == mRasterHistogramTabIndex )
    {
      QgsRasterHistogramWidget* widget = new QgsRasterHistogramWidget( rlayer, area );
      connect( widget, SIGNAL( widgetChanged() ), this, SLOT( autoApply() ) );

      // Wat?! This is a bit gross just because we need the render name
      widget->setRendererWidget( mRasterStyleWidget->currentRenderWidget()->renderer()->type(), mRasterStyleWidget->currentRenderWidget() );
      area->setWidget( widget );
    }
    QString errorMsg;
    QDomDocument doc( "style" );
    mLastStyleXml = doc.createElement( "style" );
    doc.appendChild( mLastStyleXml );
    mCurrentLayer->writeSymbology( mLastStyleXml, doc, errorMsg );
  }
  else
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

void QgsMapStylingWidget::pushUndoItem( const QString &name )
{
  QString errorMsg;
  QDomDocument doc( "style" );
  QDomElement rootNode = doc.createElement( "qgis" );
  doc.appendChild( rootNode );
  mCurrentLayer->writeStyle( rootNode, doc, errorMsg );
  mCurrentLayer->undoStackStyles()->beginMacro( name );
  mCurrentLayer->undoStackStyles()->push( new QgsMapLayerStyleCommand( mCurrentLayer, rootNode, mLastStyleXml ) );
  mCurrentLayer->undoStackStyles()->endMacro();
  // Override the last style on the stack
  mLastStyleXml = rootNode.cloneNode();
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
