/***************************************************************************
    qgsmapstylingwidget.cpp
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
#include "qgsmapstylepanel.h"
#include "qgsmaplayerstylemanagerwidget.h"

QgsMapStylingWidget::QgsMapStylingWidget( QgsMapCanvas* canvas, QList<QgsMapStylePanelFactory*> pages, QWidget *parent )
    : QWidget( parent )
    , mNotSupportedPage( 0 )
    , mLayerPage( 1 )
    , mMapCanvas( canvas )
    , mBlockAutoApply( false )
    , mCurrentLayer( nullptr )
    , mLabelingWidget( nullptr )
    , mVectorStyleWidget( nullptr )
    , mRasterStyleWidget( nullptr )
    , mPageFactories( pages )
{
  setupUi( this );

  connect( QgsMapLayerRegistry::instance(), SIGNAL( layerWillBeRemoved( QgsMapLayer* ) ), this, SLOT( layerAboutToBeRemoved( QgsMapLayer* ) ) );

  mAutoApplyTimer = new QTimer( this );
  mAutoApplyTimer->setSingleShot( true );

  mUndoWidget = new QgsUndoWidget( this, mMapCanvas );
  mUndoWidget->setObjectName( "Undo Styles" );
  mUndoWidget->hide();

  mStyleManagerFactory = new QgsMapLayerStyleManagerWidgetFactory();

  connect( mUndoButton, SIGNAL( pressed() ), mUndoWidget, SLOT( undo() ) );
  connect( mRedoButton, SIGNAL( pressed() ), mUndoWidget, SLOT( redo() ) );

  connect( mAutoApplyTimer, SIGNAL( timeout() ), this, SLOT( apply() ) );

  connect( mOptionsListWidget, SIGNAL( currentRowChanged( int ) ), this, SLOT( updateCurrentWidgetLayer() ) );
  connect( mLiveApplyCheck, SIGNAL( toggled( bool ) ), mButtonBox->button( QDialogButtonBox::Apply ), SLOT( setDisabled( bool ) ) );
  connect( mButtonBox->button( QDialogButtonBox::Apply ), SIGNAL( clicked() ), this, SLOT( apply() ) );
  connect( mLayerCombo, SIGNAL( layerChanged( QgsMapLayer* ) ), this, SLOT( setLayer( QgsMapLayer* ) ) );

  mButtonBox->button( QDialogButtonBox::Apply )->setEnabled( false );
}

QgsMapStylingWidget::~QgsMapStylingWidget()
{
  delete mStyleManagerFactory;
}

void QgsMapStylingWidget::setPageFactories( QList<QgsMapStylePanelFactory *> factories )
{
  mPageFactories = factories;
  // Always append the style manager factory at the bottom of the list
  mPageFactories.append( mStyleManagerFactory );
}

void QgsMapStylingWidget::setLayer( QgsMapLayer *layer )
{
  if ( !layer || !layer->isSpatial() )
  {
    mLayerCombo->setLayer( nullptr );
    mStackedWidget->setCurrentIndex( mNotSupportedPage );
    return;
  }

  bool sameLayerType = false;
  if ( mCurrentLayer )
  {
    sameLayerType =  mCurrentLayer->type() == layer->type();
  }

  mCurrentLayer = layer;

  int lastPage = mOptionsListWidget->currentIndex().row();
  mOptionsListWidget->clear();
  mUserPages.clear();
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

  Q_FOREACH ( QgsMapStylePanelFactory* factory, mPageFactories )
  {
    if ( factory->layerType().testFlag( layer->type() ) )
    {
      QListWidgetItem* item =  new QListWidgetItem( factory->icon(), "" );
      mOptionsListWidget->addItem( item );
      int row = mOptionsListWidget->row( item );
      mUserPages[row] = factory;
      QgsDebugMsg( QString( "ROW IS %1" ).arg( row ) );
    }
  }
  mOptionsListWidget->addItem( new QListWidgetItem( QgsApplication::getThemeIcon( "mIconTreeView.png" ), "" ) );

  if ( sameLayerType )
  {
    mOptionsListWidget->setCurrentRow( lastPage );
  }
  else
  {
    mOptionsListWidget->setCurrentRow( 0 );
  }

  mStackedWidget->setCurrentIndex( 1 );
}

void QgsMapStylingWidget::apply()
{
  QString undoName = "Style Change";
  if ( !mCurrentLayer )
    return;

  QWidget* current = mWidgetArea->widget();

  bool styleWasChanged = false;
  if ( QgsMapStylePanel* widget = qobject_cast<QgsMapStylePanel*>( current ) )
  {
    widget->apply();
    styleWasChanged = true;
  }
  else if ( QgsLabelingWidget* widget = qobject_cast<QgsLabelingWidget*>( current ) )
  {
    widget->apply();
    styleWasChanged = true;
    undoName = "Label Change";
  }
  else if ( QgsRendererV2PropertiesDialog* widget = qobject_cast<QgsRendererV2PropertiesDialog*>( current ) )
  {
    widget->apply();
    QgsVectorLayer* layer = qobject_cast<QgsVectorLayer*>( mCurrentLayer );
    QgsRendererV2AbstractMetadata* m = QgsRendererV2Registry::instance()->rendererMetadata( layer->rendererV2()->type() );
    undoName = QString( "Style Change - %1" ).arg( m->visibleName() );
    styleWasChanged = true;
  }
  else if ( QgsRendererRasterPropertiesWidget* widget = qobject_cast<QgsRendererRasterPropertiesWidget*>( current ) )
  {
    widget->apply();
    styleWasChanged = true;
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
  pushUndoItem( undoName );

  if ( styleWasChanged )
  {
    emit styleChanged( mCurrentLayer );
    QgsProject::instance()->setDirty( true );
    mMapCanvas->clearCache();
    mMapCanvas->refresh();
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

  whileBlocking( mLayerCombo )->setLayer( layer );

  int row = mOptionsListWidget->currentIndex().row();

  mStackedWidget->setCurrentIndex( mLayerPage );
  QWidget* current = mWidgetArea->takeWidget();

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
  else
  {
    current->deleteLater();
  }

  // Create the user page widget if we are on one of those pages
  // TODO Make all widgets use this method.
  if ( mUserPages.contains( row ) )
  {
    QgsMapStylePanel* panel = mUserPages[row]->createPanel( layer, mMapCanvas, this );
    if ( panel )
    {
      QgsDebugMsg( "Made me a widget!!" );
      connect( panel, SIGNAL( widgetChanged() ), this, SLOT( autoApply() ) );
      mWidgetArea->setWidget( panel );
    }
  }

  // The last widget is always the undo stack.
  if ( row == mOptionsListWidget->count() - 1 )
  {
    mWidgetArea->setWidget( mUndoWidget );
  }
  else if ( layer->type() == QgsMapLayer::VectorLayer )
  {
    QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer*>( layer );

    switch ( row )
    {
      case 0: // Style
      {
        mVectorStyleWidget = new QgsRendererV2PropertiesDialog( vlayer, QgsStyleV2::defaultStyle(), true );
        connect( mVectorStyleWidget, SIGNAL( widgetChanged() ), this, SLOT( autoApply() ) );
        mWidgetArea->setWidget( mVectorStyleWidget );
        break;
      }
      case 1: // Labels
      {
        if ( !mLabelingWidget )
        {
          mLabelingWidget = new QgsLabelingWidget( 0, mMapCanvas, this );
          mLabelingWidget->setDockMode( true );
          connect( mLabelingWidget, SIGNAL( widgetChanged() ), this, SLOT( autoApply() ) );
        }
        mLabelingWidget->setLayer( vlayer );
        mWidgetArea->setWidget( mLabelingWidget );
        break;
      }
      default:
        break;
    }
  }
  else if ( layer->type() == QgsMapLayer::RasterLayer )
  {
    QgsRasterLayer *rlayer = qobject_cast<QgsRasterLayer*>( layer );

    switch ( row )
    {
      case 0: // Style
        mRasterStyleWidget = new QgsRendererRasterPropertiesWidget( mMapCanvas, mWidgetArea );
        mRasterStyleWidget->syncToLayer( rlayer );
        connect( mRasterStyleWidget, SIGNAL( widgetChanged() ), this, SLOT( autoApply() ) );

        mWidgetArea->setWidget( mRasterStyleWidget );
        break;
      case 1: // Transparency
      {
        QgsRasterTransparencyWidget* transwidget = new QgsRasterTransparencyWidget( rlayer, mMapCanvas, mWidgetArea );
        connect( transwidget, SIGNAL( widgetChanged() ), this, SLOT( autoApply() ) );
        mWidgetArea->setWidget( transwidget );
        break;
      }
      case 2: // Transparency
      {
        if ( mRasterStyleWidget )
        {
          mRasterStyleWidget->deleteLater();
          delete mRasterStyleWidget;
        }
        mRasterStyleWidget = new QgsRendererRasterPropertiesWidget( mMapCanvas, mWidgetArea );
        mRasterStyleWidget->syncToLayer( rlayer );
        connect( mRasterStyleWidget, SIGNAL( widgetChanged() ), this, SLOT( autoApply() ) );

        QgsRasterHistogramWidget* widget = new QgsRasterHistogramWidget( rlayer, mWidgetArea );
        connect( widget, SIGNAL( widgetChanged() ), this, SLOT( autoApply() ) );
        QString name = mRasterStyleWidget->currentRenderWidget()->renderer()->type();
        widget->setRendererWidget( name, mRasterStyleWidget->currentRenderWidget() );

        mWidgetArea->setWidget( widget );
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

  QString errorMsg;
  QDomDocument doc( "style" );
  mLastStyleXml = doc.createElement( "style" );
  doc.appendChild( mLastStyleXml );
  mCurrentLayer->writeSymbology( mLastStyleXml, doc, errorMsg );

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

QIcon QgsMapLayerStyleManagerWidgetFactory::icon()
{
  return  QgsApplication::getThemeIcon( "propertyicons/symbology.png" );
}

QString QgsMapLayerStyleManagerWidgetFactory::title()
{
  return QString();
}

QgsMapStylePanel *QgsMapLayerStyleManagerWidgetFactory::createPanel( QgsMapLayer *layer, QgsMapCanvas *canvas, QWidget *parent )
{
  return new QgsMapLayerStyleManagerWidget( layer,  canvas, parent );

}

QgsMapStylePanelFactory::LayerTypesFlags QgsMapLayerStyleManagerWidgetFactory::layerType()
{
  return QgsMapLayer::VectorLayer;
}
