#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QSizePolicy>
#include <QUndoStack>
#include <QListWidget>

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
    , mNotSupportedPage( 0 )
    , mLayerPage( 1 )
    , mMapCanvas( canvas )
    , mBlockAutoApply( false )
    , mCurrentLayer( nullptr )
    , mLabelingWidget( nullptr )
    , mVectorStyleWidget( nullptr )
    , mRasterStyleWidget( nullptr )
{
  setupUi( this );

  connect( QgsMapLayerRegistry::instance(), SIGNAL( layerWillBeRemoved( QgsMapLayer* ) ), this, SLOT( layerAboutToBeRemoved( QgsMapLayer* ) ) );

  mAutoApplyTimer = new QTimer( this );
  mAutoApplyTimer->setSingleShot( true );
  connect( mAutoApplyTimer, SIGNAL( timeout() ), this, SLOT( apply() ) );

  mStackedWidget = new QStackedWidget( this );

  connect( mOptionsListWidget, SIGNAL( currentRowChanged( int ) ), this, SLOT( updateCurrentWidgetLayer() ) );
  connect( mLiveApplyCheck, SIGNAL( toggled( bool ) ), mButtonBox->button( QDialogButtonBox::Apply ), SLOT( setDisabled( bool ) ) );
  connect( mButtonBox->button( QDialogButtonBox::Apply ), SIGNAL( clicked() ), this, SLOT( apply() ) );

  mButtonBox->button( QDialogButtonBox::Apply )->setEnabled( false );

  mStackedWidget->setCurrentIndex( 1 );
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
  mOptionsListWidget->clear();

  if ( layer->type() == QgsMapLayer::VectorLayer )
  {
    mOptionsListWidget->addItem( new QListWidgetItem( QgsApplication::getThemeIcon( "propertyicons/symbology.png" ), "" ) );
    mOptionsListWidget->addItem( new QListWidgetItem( QgsApplication::getThemeIcon( "labelingSingle.svg" ), "" ) );
  }
  else if ( layer->type() == QgsMapLayer::RasterLayer )
  {
    mOptionsListWidget->addItem( new QListWidgetItem( QgsApplication::getThemeIcon( "propertyicons/symbology.png" ), "" ) );
    mOptionsListWidget->addItem( new QListWidgetItem( QgsApplication::getThemeIcon( "propertyicons/transparency.png" ), "" ) );
    mOptionsListWidget->addItem( new QListWidgetItem( QgsApplication::getThemeIcon( "propertyicons/histogram.png" ), "" ) );
  }

  mOptionsListWidget->addItem( new QListWidgetItem( QgsApplication::getThemeIcon( "mIconTreeView.png" ), "" ) );

  mOptionsListWidget->setCurrentRow( 0 );
}

void QgsMapStylingWidget::apply()
{
  QString undoName = "Style Change";
  if ( mCurrentLayer->type() == QgsMapLayer::VectorLayer )
  {
    QWidget* current = mWidgetArea->widget();
    if ( QgsLabelingWidget* widget = qobject_cast<QgsLabelingWidget*>( current ) )
    {
      widget->apply();
      emit styleChanged( mCurrentLayer );
      undoName = "Label Change";
    }
    else if ( QgsRendererV2PropertiesDialog* widget = qobject_cast<QgsRendererV2PropertiesDialog*>( current ) )
    {
      widget->apply();
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
    QWidget* current = mWidgetArea->widget();

    if ( QgsRendererRasterPropertiesWidget* rasterproperties = qobject_cast<QgsRendererRasterPropertiesWidget*>( current ) )
    {
      rasterproperties->apply();
      emit styleChanged( mCurrentLayer );
      QgsProject::instance()->setDirty( true );
      mMapCanvas->clearCache();
      mMapCanvas->refresh();
    }
    else if ( QgsRasterTransparencyWidget* transwidget = qobject_cast<QgsRasterTransparencyWidget*>( current ) )
    {
      transwidget->apply();
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

  mLayerTitleLabel->setText( layer->name() );

  QScrollArea* area = mWidgetArea;

  if ( layer->type() == QgsMapLayer::VectorLayer )
  {
    mStackedWidget->setCurrentIndex( mLayerPage );

    QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer*>( layer );
    QWidget* current = area->takeWidget();

    if ( QgsLabelingWidget* labelWidget = qobject_cast<QgsLabelingWidget*>( current ) )
    {
      mLabelingWidget = labelWidget;
    }
    else if ( QgsRendererRasterPropertiesWidget* rasterproperties = qobject_cast<QgsRendererRasterPropertiesWidget*>( current ) )
    {
      mRasterStyleWidget = rasterproperties;
    }

    switch ( mOptionsListWidget->currentIndex().row() )
    {
      case 0: // Style
      {
        mVectorStyleWidget = new QgsRendererV2PropertiesDialog( vlayer, QgsStyleV2::defaultStyle(), true );
        connect( mVectorStyleWidget, SIGNAL( widgetChanged() ), this, SLOT( autoApply() ) );
        area->setWidget( mVectorStyleWidget );
        break;
      }
      case 1: // Labels
      {
        if ( !mLabelingWidget )
        {
          mLabelingWidget = new QgsLabelingWidget( 0, mMapCanvas, this );
          mLabelingWidget->setDockMode( true );
          mLabelingWidget->setLayer( vlayer );
          connect( mLabelingWidget, SIGNAL( widgetChanged() ), this, SLOT( autoApply() ) );
        }
        area->setWidget( mLabelingWidget );
        break;
      }
      case 2: // History
        mUndoWidget = new QgsUndoWidget( mOptionsListWidget, mMapCanvas );
        mUndoWidget->setObjectName( "Undo Styles" );
        mUndoWidget->setUndoStack( layer->undoStackStyles() );
        connect( mUndoButton, SIGNAL( pressed() ), mUndoWidget, SLOT( undo() ) );
        connect( mRedoButton, SIGNAL( pressed() ), mUndoWidget, SLOT( redo() ) );
        area->setWidget( mUndoWidget );
        break;
      default:
        break;
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

    switch ( mOptionsListWidget->currentIndex().row() )
    {
      case 0: // Style
        mRasterStyleWidget = new QgsRendererRasterPropertiesWidget( rlayer, mMapCanvas, area );
        connect( mRasterStyleWidget, SIGNAL( widgetChanged() ), this, SLOT( autoApply() ) );
        area->setWidget( mRasterStyleWidget );
        break;
      case 1: // Transparency
      {
        QgsRasterTransparencyWidget* transwidget = new QgsRasterTransparencyWidget( rlayer, mMapCanvas, area );
        connect( transwidget, SIGNAL( widgetChanged() ), this, SLOT( autoApply() ) );
        area->setWidget( transwidget );
        break;
      }
      case 2: // Transparency
      {
        QgsRasterHistogramWidget* histowidget = new QgsRasterHistogramWidget( rlayer, area );
        connect( histowidget, SIGNAL( widgetChanged() ), this, SLOT( autoApply() ) );

        // Wat?! This is a bit gross just because we need the render name
//        histowidget->setRendererWidget( mRasterStyleWidget->currentRenderWidget()->renderer()->type(), mRasterStyleWidget->currentRenderWidget() );
        area->setWidget( histowidget );
        break;
      }
      case 3: // History
        mUndoWidget = new QgsUndoWidget( mOptionsListWidget, mMapCanvas );
        mUndoWidget->setObjectName( "Undo Styles" );
        mUndoWidget->setUndoStack( layer->undoStackStyles() );

        connect( mUndoButton, SIGNAL( pressed() ), mUndoWidget, SLOT( undo() ) );
        connect( mRedoButton, SIGNAL( pressed() ), mUndoWidget, SLOT( redo() ) );
        area->setWidget( mUndoWidget );
        break;
      default:
        break;
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
