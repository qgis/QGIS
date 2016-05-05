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
#include "qgsmapcanvas.h"
#include "qgsmaplayer.h"
#include "qgsstylev2.h"
#include "qgsvectorlayer.h"
#include "qgsproject.h"
#include "qgsundowidget.h"

QgsMapStylingWidget::QgsMapStylingWidget( QgsMapCanvas* canvas, QWidget *parent ) :
    QWidget( parent )
    , mMapCanvas( canvas )
    , mBlockAutoApply( false )
    , mCurrentLayer( nullptr )
{
  QBoxLayout* layout = new QVBoxLayout();
  layout->setContentsMargins( 0, 0, 0, 0 );
  this->setLayout( layout );

  mAutoApplyTimer = new QTimer( this );
  mAutoApplyTimer->setSingleShot( true );
  connect( mAutoApplyTimer, SIGNAL( timeout()), this, SLOT(apply()));

  mStackedWidget = new QStackedWidget( this );
  mMapStyleTabs = new QTabWidget( this );
  mMapStyleTabs->setDocumentMode( true );
  mNotSupportedPage = mStackedWidget->addWidget( new QLabel( "Not supported currently" ) );
  mVectorPage = mStackedWidget->addWidget( mMapStyleTabs );

  // create undo widget
  mUndoWidget = new QgsUndoWidget( nullptr, mMapCanvas );
  mUndoWidget->setObjectName( "Undo Styles" );

  mLayerTitleLabel = new QLabel();
  mLayerTitleLabel->setAlignment( Qt::AlignHCenter );
  layout->addWidget( mLayerTitleLabel );
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
  QScrollArea* stylescroll = new QScrollArea;
  stylescroll->setWidgetResizable( true );
  stylescroll->setFrameStyle( QFrame::NoFrame );
  QScrollArea* labelscroll = new QScrollArea;
  labelscroll->setWidgetResizable( true );
  labelscroll->setFrameStyle( QFrame::NoFrame );
  labelscroll->setWidget( mLabelingWidget );

  mStyleTabIndex = mMapStyleTabs->addTab( stylescroll, QgsApplication::getThemeIcon( "propertyicons/symbology.png" ), "Styles" );
  mLabelTabIndex = mMapStyleTabs->addTab( labelscroll, QgsApplication::getThemeIcon( "labelingSingle.svg" ), "Labeling" );
  mMapStyleTabs->addTab( mUndoWidget, QgsApplication::getThemeIcon( "labelingSingle.svg" ), "History" );
//  int diagramTabIndex = mMapStyleTabs->addTab( new QWidget(), QgsApplication::getThemeIcon( "propertyicons/diagram.png" ), "Diagrams" );
//  mMapStyleTabs->setTabEnabled( styleTabIndex, false );
//  mMapStyleTabs->setTabEnabled( diagramTabIndex, false );
  mMapStyleTabs->setCurrentIndex( mStyleTabIndex );

  connect( mMapStyleTabs, SIGNAL( currentChanged( int ) ), this, SLOT( updateCurrentWidgetLayer( int ) ) );

  connect( mLiveApplyCheck, SIGNAL( toggled( bool ) ), mButtonBox->button( QDialogButtonBox::Apply ), SLOT( setDisabled( bool ) ) );

  connect( mButtonBox->button( QDialogButtonBox::Apply ), SIGNAL( clicked() ), this, SLOT( apply() ) );
  connect( mButtonBox->button( QDialogButtonBox::Reset ), SIGNAL( clicked() ), this, SLOT( resetSettings() ) );

  mButtonBox->button( QDialogButtonBox::Apply )->setEnabled( false );
  mButtonBox->button( QDialogButtonBox::Reset )->setEnabled( false );

}

void QgsMapStylingWidget::setLayer( QgsMapLayer *layer )
{
  if ( !layer )
  {
    mLayerTitleLabel->setText( "" );
    mStackedWidget->setCurrentIndex( mNotSupportedPage );
    return;
  }

  mCurrentLayer = layer;

  // TODO Adjust for raster
  updateCurrentWidgetLayer( mMapStyleTabs->currentIndex() );
}

void QgsMapStylingWidget::apply()
{
  if ( mStackedWidget->currentIndex() == mVectorPage )
  {
      int currentPage = mMapStyleTabs->currentIndex();
      if ( currentPage == mLabelTabIndex )
      {
        mLabelingWidget->apply();
        mButtonBox->button( QDialogButtonBox::Reset )->setEnabled( true );
        emit styleChanged( mCurrentLayer );
    QgsDebugMsg("Label Style");
      }
      else if ( currentPage == mStyleTabIndex )
      {
        mVectorStyleWidget->apply();
        mButtonBox->button( QDialogButtonBox::Reset )->setEnabled( true );
        QgsProject::instance()->setDirty( true );
        mMapCanvas->clearCache();
        mMapCanvas->refresh();
        emit styleChanged( mCurrentLayer );
    QgsDebugMsg("Map Style");
      }
      QString errorMsg;
      QDomDocument doc( "style" );
      QDomElement rootNode = doc.createElement( "qgis" );
      doc.appendChild( rootNode );
      mCurrentLayer->writeSymbology( rootNode, doc, errorMsg );
      mCurrentLayer->undoStackStyles()->push( new QgsMapLayerStyleCommand( mCurrentLayer, rootNode, mLastStyleXml ) );
      // Override the last style on the stack
      mLastStyleXml = rootNode.cloneNode();
  }
}

void QgsMapStylingWidget::autoApply()
{
  if ( mLiveApplyCheck->isChecked() && !mBlockAutoApply )
  {
      mAutoApplyTimer->start(100);
  }
}

void QgsMapStylingWidget::resetSettings()
{
  if ( mStackedWidget->currentIndex() == mVectorPage &&
       mMapStyleTabs->currentIndex() == mLabelTabIndex )
  {
    mLabelingWidget->resetSettings();
  }
}

void QgsMapStylingWidget::updateCurrentWidgetLayer( int currentPage )
{
  mBlockAutoApply = true;

  QgsMapLayer* layer = mCurrentLayer;
  mUndoWidget->setUndoStack( layer->undoStackStyles() );

  mLayerTitleLabel->setText( layer->name() );

  if ( layer->type() == QgsMapLayer::VectorLayer )
  {
    mStackedWidget->setCurrentIndex( mVectorPage );
    if ( currentPage == mLabelTabIndex )
    {
      mLabelingWidget->setLayer( layer );
    }
    if ( currentPage == mStyleTabIndex )
    {
      // TODO Refactor props dialog so we don't have to do this
      QScrollArea* area = qobject_cast<QScrollArea*>( mMapStyleTabs->widget( mStyleTabIndex ) );
      QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer*>( layer );
      mVectorStyleWidget = new QgsRendererV2PropertiesDialog( vlayer, QgsStyleV2::defaultStyle(), true );
      connect( mVectorStyleWidget, SIGNAL( widgetChanged() ), this, SLOT( autoApply() ) );
      area->setWidget( mVectorStyleWidget );
    }
    QString errorMsg;
    QDomDocument doc( "style" );
    mLastStyleXml = doc.createElement("style");
    doc.appendChild( mLastStyleXml );
    mCurrentLayer->writeSymbology( mLastStyleXml, doc, errorMsg );
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


QgsMapLayerStyleCommand::QgsMapLayerStyleCommand(QgsMapLayer *layer, const QDomNode &current, const QDomNode &last)
    : QUndoCommand( "Style chanage" )
    , mLayer( layer )
    , mXml( current )
    , mLastState( last )
{
}

void QgsMapLayerStyleCommand::undo()
{
   QString error;
   mLayer->readSymbology( mLastState, error);
   mLayer->triggerRepaint();
   QgsDebugMsg( error );
}

void QgsMapLayerStyleCommand::redo()
{
   QString error;
   mLayer->readSymbology( mXml, error);
   mLayer->triggerRepaint();
   QgsDebugMsg( error );
}
