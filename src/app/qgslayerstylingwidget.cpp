/***************************************************************************
    qgslayerstylingwidget.cpp
    ---------------------
    begin                : April 2016
    copyright            : (C) 2016 by Nathan Woodrow
    email                : woodrow dot nathan at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QSizePolicy>
#include <QUndoStack>
#include <QListWidget>

#include "qgsapplication.h"
#include "qgslabelingwidget.h"
#include "qgslayerstylingwidget.h"
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
#include "qgsmaplayerconfigwidget.h"
#include "qgsmaplayerstylemanagerwidget.h"
#include "qgsruntimeprofiler.h"


QgsLayerStylingWidget::QgsLayerStylingWidget( QgsMapCanvas* canvas, QList<QgsMapLayerConfigWidgetFactory*> pages, QWidget *parent )
    : QWidget( parent )
    , mNotSupportedPage( 0 )
    , mLayerPage( 1 )
    , mMapCanvas( canvas )
    , mBlockAutoApply( false )
    , mCurrentLayer( nullptr )
    , mLabelingWidget( nullptr )
    , mRasterStyleWidget( nullptr )
    , mPageFactories( pages )
{
  setupUi( this );

  connect( QgsMapLayerRegistry::instance(), SIGNAL( layerWillBeRemoved( QgsMapLayer* ) ), this, SLOT( layerAboutToBeRemoved( QgsMapLayer* ) ) );

  QSettings settings;
  mLiveApplyCheck->setChecked( settings.value( "UI/autoApplyStyling", true ).toBool() );

  mAutoApplyTimer = new QTimer( this );
  mAutoApplyTimer->setSingleShot( true );

  mUndoWidget = new QgsUndoWidget( this, mMapCanvas );
  mUndoWidget->setAutoDelete( false );
  mUndoWidget->setObjectName( "Undo Styles" );
  mUndoWidget->hide();

  mStyleManagerFactory = new QgsLayerStyleManagerWidgetFactory();

  QList<QgsMapLayerConfigWidgetFactory*> l;
  setPageFactories( pages );

  connect( mUndoButton, SIGNAL( pressed() ), this, SLOT( undo() ) );
  connect( mRedoButton, SIGNAL( pressed() ), this, SLOT( redo() ) );

  connect( mAutoApplyTimer, SIGNAL( timeout() ), this, SLOT( apply() ) );

  connect( mOptionsListWidget, SIGNAL( currentRowChanged( int ) ), this, SLOT( updateCurrentWidgetLayer() ) );
  connect( mButtonBox->button( QDialogButtonBox::Apply ), SIGNAL( clicked() ), this, SLOT( apply() ) );
  connect( mLayerCombo, SIGNAL( layerChanged( QgsMapLayer* ) ), this, SLOT( setLayer( QgsMapLayer* ) ) );
  connect( mLiveApplyCheck, SIGNAL( toggled( bool ) ), this, SLOT( liveApplyToggled( bool ) ) );

  mStackedWidget->setCurrentIndex( 0 );
}

QgsLayerStylingWidget::~QgsLayerStylingWidget()
{
  delete mStyleManagerFactory;
}

void QgsLayerStylingWidget::setPageFactories( QList<QgsMapLayerConfigWidgetFactory *> factories )
{
  mPageFactories = factories;
  // Always append the style manager factory at the bottom of the list
  mPageFactories.append( mStyleManagerFactory );
}

void QgsLayerStylingWidget::blockUpdates( bool blocked )
{
  if ( !mCurrentLayer )
    return;

  if ( blocked )
  {
    disconnect( mCurrentLayer, SIGNAL( styleChanged() ), this, SLOT( updateCurrentWidgetLayer() ) );
  }
  else
  {
    connect( mCurrentLayer, SIGNAL( styleChanged() ), this, SLOT( updateCurrentWidgetLayer() ) );
  }
}

void QgsLayerStylingWidget::setLayer( QgsMapLayer *layer )
{
  if ( layer == mCurrentLayer )
    return;

  if ( mCurrentLayer )
  {
    disconnect( mCurrentLayer, SIGNAL( styleChanged() ), this, SLOT( updateCurrentWidgetLayer() ) );
  }

  if ( !layer || !layer->isSpatial() )
  {
    mLayerCombo->setLayer( nullptr );
    mStackedWidget->setCurrentIndex( mNotSupportedPage );
    mLastStyleXml.clear();
    mCurrentLayer = nullptr;
    return;
  }

  bool sameLayerType = false;
  if ( mCurrentLayer )
  {
    sameLayerType =  mCurrentLayer->type() == layer->type();
  }

  mCurrentLayer = layer;

  mUndoWidget->setUndoStack( layer->undoStackStyles() );

  connect( mCurrentLayer, SIGNAL( styleChanged() ), this, SLOT( updateCurrentWidgetLayer() ) );

  int lastPage = mOptionsListWidget->currentIndex().row();
  mOptionsListWidget->blockSignals( true );
  mOptionsListWidget->clear();
  mUserPages.clear();
  if ( layer->type() == QgsMapLayer::VectorLayer )
  {
    QListWidgetItem* symbolItem = new QListWidgetItem( QgsApplication::getThemeIcon( "propertyicons/symbology.svg" ), QString() );
    symbolItem->setData( Qt::UserRole, Symbology );
    symbolItem->setToolTip( tr( "Symbology" ) );
    mOptionsListWidget->addItem( symbolItem );
    QListWidgetItem* labelItem = new QListWidgetItem( QgsApplication::getThemeIcon( "labelingSingle.svg" ), QString() );
    labelItem->setData( Qt::UserRole, VectorLabeling );
    labelItem->setToolTip( tr( "Labels" ) );
    mOptionsListWidget->addItem( labelItem );
  }
  else if ( layer->type() == QgsMapLayer::RasterLayer )
  {
    QListWidgetItem* symbolItem = new QListWidgetItem( QgsApplication::getThemeIcon( "propertyicons/symbology.svg" ), QString() );
    symbolItem->setData( Qt::UserRole, Symbology );
    symbolItem->setToolTip( tr( "Symbology" ) );
    mOptionsListWidget->addItem( symbolItem );
    QListWidgetItem* transparencyItem = new QListWidgetItem( QgsApplication::getThemeIcon( "propertyicons/transparency.png" ), QString() );
    transparencyItem->setToolTip( tr( "Transparency" ) );
    transparencyItem->setData( Qt::UserRole, RasterTransparency );
    mOptionsListWidget->addItem( transparencyItem );
    QListWidgetItem* histogramItem = new QListWidgetItem( QgsApplication::getThemeIcon( "propertyicons/histogram.png" ), QString() );
    histogramItem->setData( Qt::UserRole, RasterHistogram );
    mOptionsListWidget->addItem( histogramItem );
    histogramItem->setToolTip( tr( "Histogram" ) );
  }

  Q_FOREACH ( QgsMapLayerConfigWidgetFactory* factory, mPageFactories )
  {
    if ( factory->supportsStyleDock() && factory->supportsLayer( layer ) )
    {
      QListWidgetItem* item =  new QListWidgetItem( factory->icon(), QString() );
      item->setToolTip( factory->title() );
      mOptionsListWidget->addItem( item );
      int row = mOptionsListWidget->row( item );
      mUserPages[row] = factory;
    }
  }
  QListWidgetItem* historyItem = new QListWidgetItem( QgsApplication::getThemeIcon( "mActionHistory.svg" ), QString() );
  historyItem->setData( Qt::UserRole, History );
  historyItem->setToolTip( tr( "History" ) );
  mOptionsListWidget->addItem( historyItem );
  mOptionsListWidget->blockSignals( false );

  if ( sameLayerType )
  {
    mOptionsListWidget->setCurrentRow( lastPage );
  }
  else
  {
    mOptionsListWidget->setCurrentRow( 0 );
  }

  mStackedWidget->setCurrentIndex( 1 );

  QString errorMsg;
  QDomDocument doc( "style" );
  mLastStyleXml = doc.createElement( "style" );
  doc.appendChild( mLastStyleXml );
  mCurrentLayer->writeStyle( mLastStyleXml, doc, errorMsg );
}

void QgsLayerStylingWidget::apply()
{
  if ( !mCurrentLayer )
    return;

  disconnect( mCurrentLayer, SIGNAL( styleChanged() ), this, SLOT( updateCurrentWidgetLayer() ) );

  QString undoName = "Style Change";

  QWidget* current = mWidgetStack->mainWidget();

  bool styleWasChanged = false;
  if ( QgsLabelingWidget* widget = qobject_cast<QgsLabelingWidget*>( current ) )
  {
    widget->apply();
    styleWasChanged = true;
    undoName = "Label Change";
  }
  if ( QgsPanelWidgetWrapper* wrapper = qobject_cast<QgsPanelWidgetWrapper*>( current ) )
  {
    if ( QgsRendererV2PropertiesDialog* widget = qobject_cast<QgsRendererV2PropertiesDialog*>( wrapper->widget() ) )
    {
      widget->apply();
      QgsVectorLayer* layer = qobject_cast<QgsVectorLayer*>( mCurrentLayer );
      QgsRendererV2AbstractMetadata* m = QgsRendererV2Registry::instance()->rendererMetadata( layer->rendererV2()->type() );
      undoName = QString( "Style Change - %1" ).arg( m->visibleName() );
      styleWasChanged = true;
    }
  }
  else if ( QgsRasterTransparencyWidget* widget = qobject_cast<QgsRasterTransparencyWidget*>( current ) )
  {
    widget->apply();
    styleWasChanged = true;
  }
  else if ( qobject_cast<QgsRasterHistogramWidget*>( current ) )
  {
    mRasterStyleWidget->apply();
    styleWasChanged = true;
  }
  else if ( QgsMapLayerConfigWidget* widget = qobject_cast<QgsMapLayerConfigWidget*>( current ) )
  {
    widget->apply();
    styleWasChanged = true;
  }

  pushUndoItem( undoName );

  if ( styleWasChanged )
  {
    emit styleChanged( mCurrentLayer );
    QgsProject::instance()->setDirty( true );
    mMapCanvas->clearCache();
    mMapCanvas->refresh();
  }
  connect( mCurrentLayer, SIGNAL( styleChanged() ), this, SLOT( updateCurrentWidgetLayer() ) );
}

void QgsLayerStylingWidget::autoApply()
{
  if ( mLiveApplyCheck->isChecked() && !mBlockAutoApply )
  {
    mAutoApplyTimer->start( 100 );
  }
}

void QgsLayerStylingWidget::undo()
{
  mUndoWidget->undo();
  updateCurrentWidgetLayer();
}

void QgsLayerStylingWidget::redo()
{
  mUndoWidget->redo();
  updateCurrentWidgetLayer();
}

void QgsLayerStylingWidget::updateCurrentWidgetLayer()
{
  if ( !mCurrentLayer )
    return;  // non-spatial are ignored in setLayer()

  mBlockAutoApply = true;

  whileBlocking( mLayerCombo )->setLayer( mCurrentLayer );

  int row = mOptionsListWidget->currentIndex().row();

  mStackedWidget->setCurrentIndex( mLayerPage );

  QgsPanelWidget* current = mWidgetStack->takeMainWidget();
  if ( current )
  {
    if ( QgsLabelingWidget* widget = qobject_cast<QgsLabelingWidget*>( current ) )
    {
      mLabelingWidget = widget;
    }
    else if ( QgsUndoWidget* widget = qobject_cast<QgsUndoWidget*>( current ) )
    {
      mUndoWidget = widget;
    }
    else if ( QgsRendererRasterPropertiesWidget* widget = qobject_cast<QgsRendererRasterPropertiesWidget*>( current ) )
    {
      mRasterStyleWidget = widget;
    }

  }

  mWidgetStack->clear();
  // Create the user page widget if we are on one of those pages
  // TODO Make all widgets use this method.
  if ( mUserPages.contains( row ) )
  {
    QgsMapLayerConfigWidget* panel = mUserPages[row]->createWidget( mCurrentLayer, mMapCanvas, true, mWidgetStack );
    if ( panel )
    {
      connect( panel, SIGNAL( widgetChanged( QgsPanelWidget* ) ), this, SLOT( autoApply() ) );
      mWidgetStack->addMainPanel( panel );
    }
  }

  // The last widget is always the undo stack.
  if ( row == mOptionsListWidget->count() - 1 )
  {
    mWidgetStack->addMainPanel( mUndoWidget );
  }
  else if ( mCurrentLayer->type() == QgsMapLayer::VectorLayer )
  {
    QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer*>( mCurrentLayer );

    switch ( row )
    {
      case 0: // Style
      {
        QgsRendererV2PropertiesDialog* styleWidget = new QgsRendererV2PropertiesDialog( vlayer, QgsStyleV2::defaultStyle(), true, mStackedWidget );
        styleWidget->setMapCanvas( mMapCanvas );
        styleWidget->setDockMode( true );
        connect( styleWidget, SIGNAL( widgetChanged() ), this, SLOT( autoApply() ) );
        QgsPanelWidgetWrapper* wrapper = new QgsPanelWidgetWrapper( styleWidget, mStackedWidget );
        wrapper->setDockMode( true );
        connect( styleWidget, SIGNAL( showPanel( QgsPanelWidget* ) ), wrapper, SLOT( openPanel( QgsPanelWidget* ) ) );
        mWidgetStack->addMainPanel( wrapper );
        break;
      }
      case 1: // Labels
      {
        if ( !mLabelingWidget )
        {
          mLabelingWidget = new QgsLabelingWidget( 0, mMapCanvas, mWidgetStack );
          mLabelingWidget->setDockMode( true );
          connect( mLabelingWidget, SIGNAL( widgetChanged() ), this, SLOT( autoApply() ) );
        }
        mLabelingWidget->setLayer( vlayer );
        mWidgetStack->addMainPanel( mLabelingWidget );
        break;
      }
      default:
        break;
    }
  }
  else if ( mCurrentLayer->type() == QgsMapLayer::RasterLayer )
  {
    QgsRasterLayer *rlayer = qobject_cast<QgsRasterLayer*>( mCurrentLayer );

    switch ( row )
    {
      case 0: // Style
        mRasterStyleWidget = new QgsRendererRasterPropertiesWidget( rlayer, mMapCanvas, mWidgetStack );
        connect( mRasterStyleWidget, SIGNAL( widgetChanged() ), this, SLOT( autoApply() ) );
        mWidgetStack->addMainPanel( mRasterStyleWidget );
        break;
      case 1: // Transparency
      {
        QgsRasterTransparencyWidget* transwidget = new QgsRasterTransparencyWidget( rlayer, mMapCanvas, mWidgetStack );
        connect( transwidget, SIGNAL( widgetChanged() ), this, SLOT( autoApply() ) );
        mWidgetStack->addMainPanel( transwidget );
        break;
      }
      case 2: // Histogram
      {
        if ( mRasterStyleWidget )
        {
          mRasterStyleWidget->deleteLater();
          delete mRasterStyleWidget;
        }
        mRasterStyleWidget = new QgsRendererRasterPropertiesWidget( rlayer, mMapCanvas, mWidgetStack );
        mRasterStyleWidget->syncToLayer( rlayer );
        connect( mRasterStyleWidget, SIGNAL( widgetChanged() ), this, SLOT( autoApply() ) );

        QgsRasterHistogramWidget* widget = new QgsRasterHistogramWidget( rlayer, mWidgetStack );
        connect( widget, SIGNAL( widgetChanged() ), this, SLOT( autoApply() ) );
        QString name = mRasterStyleWidget->currentRenderWidget()->renderer()->type();
        widget->setRendererWidget( name, mRasterStyleWidget->currentRenderWidget() );

        mWidgetStack->addMainPanel( widget );
        break;
      }
      default:
        break;
    }
  }
  else
  {
    mStackedWidget->setCurrentIndex( mNotSupportedPage );
  }

  mBlockAutoApply = false;
}

void QgsLayerStylingWidget::setCurrentPage( QgsLayerStylingWidget::Page page )
{
  for ( int i = 0; i < mOptionsListWidget->count(); ++i )
  {
    int data = mOptionsListWidget->item( i )->data( Qt::UserRole ).toInt();
    if ( data == static_cast< int >( page ) )
    {
      mOptionsListWidget->setCurrentRow( i );
      return;
    }
  }
}

void QgsLayerStylingWidget::layerAboutToBeRemoved( QgsMapLayer* layer )
{
  if ( layer == mCurrentLayer )
  {
    mAutoApplyTimer->stop();
    setLayer( nullptr );
  }
}

void QgsLayerStylingWidget::liveApplyToggled( bool value )
{
  QSettings settings;
  settings.setValue( "UI/autoApplyStyling", value );
}

void QgsLayerStylingWidget::pushUndoItem( const QString &name )
{
  QString errorMsg;
  QDomDocument doc( "style" );
  QDomElement rootNode = doc.createElement( "qgis" );
  doc.appendChild( rootNode );
  mCurrentLayer->writeStyle( rootNode, doc, errorMsg );
  mCurrentLayer->undoStackStyles()->push( new QgsMapLayerStyleCommand( mCurrentLayer, name, rootNode, mLastStyleXml ) );
  // Override the last style on the stack
  mLastStyleXml = rootNode.cloneNode();
}


QgsMapLayerStyleCommand::QgsMapLayerStyleCommand( QgsMapLayer *layer, const QString& text, const QDomNode &current, const QDomNode &last )
    : QUndoCommand( text )
    , mLayer( layer )
    , mXml( current )
    , mLastState( last )
    , mTime( QTime::currentTime() )
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

bool QgsMapLayerStyleCommand::mergeWith( const QUndoCommand* other )
{
  if ( other->id() != id() ) // make sure other is also an QgsMapLayerStyleCommand
    return false;

  const QgsMapLayerStyleCommand* otherCmd = static_cast<const QgsMapLayerStyleCommand*>( other );
  if ( otherCmd->mLayer != mLayer )
    return false;  // should never happen though...

  // only merge commands if they are created shortly after each other
  // (e.g. user keeps modifying one property)
  QSettings settings;
  int timeout = settings.value( "/UI/styleUndoMergeTimeout", 500 ).toInt();
  if ( mTime.msecsTo( otherCmd->mTime ) > timeout )
    return false;

  mXml = otherCmd->mXml;
  mTime = otherCmd->mTime;
  return true;
}

QgsLayerStyleManagerWidgetFactory::QgsLayerStyleManagerWidgetFactory()
{
  setIcon( QgsApplication::getThemeIcon( "propertyicons/stylepreset.svg" ) );
  setTitle( QObject::tr( "Style Manager" ) );
}

QgsMapLayerConfigWidget *QgsLayerStyleManagerWidgetFactory::createWidget( QgsMapLayer *layer, QgsMapCanvas *canvas, bool dockMode, QWidget *parent ) const
{
  Q_UNUSED( dockMode );
  return new QgsMapLayerStyleManagerWidget( layer,  canvas, parent );

}

bool QgsLayerStyleManagerWidgetFactory::supportsLayer( QgsMapLayer *layer ) const
{
  return ( layer->type() == QgsMapLayer::VectorLayer || layer->type() == QgsMapLayer::RasterLayer );
}
