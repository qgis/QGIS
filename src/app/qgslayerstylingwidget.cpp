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
#include "qgsrendererpropertiesdialog.h"
#include "qgsrendererrasterpropertieswidget.h"
#include "qgsrenderermeshpropertieswidget.h"
#include "qgsrasterhistogramwidget.h"
#include "qgsrasterrenderer.h"
#include "qgsrasterrendererwidget.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayer.h"
#include "qgsstyle.h"
#include "qgsvectorlayer.h"
#include "qgsmeshlayer.h"
#include "qgsproject.h"
#include "qgsundowidget.h"
#include "qgsreadwritecontext.h"
#include "qgsrenderer.h"
#include "qgsrendererregistry.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterlayer.h"
#include "qgsmaplayerconfigwidget.h"
#include "qgsmaplayerstylemanagerwidget.h"
#include "qgsruntimeprofiler.h"
#include "qgsrasterminmaxwidget.h"
#include "qgisapp.h"
#include "qgssymbolwidgetcontext.h"

#ifdef HAVE_3D
#include "qgsvectorlayer3drendererwidget.h"
#include "qgsmeshlayer3drendererwidget.h"
#endif


QgsLayerStylingWidget::QgsLayerStylingWidget( QgsMapCanvas *canvas, QgsMessageBar *messageBar, const QList<QgsMapLayerConfigWidgetFactory *> &pages, QWidget *parent )
  : QWidget( parent )
  , mNotSupportedPage( 0 )
  , mLayerPage( 1 )
  , mMapCanvas( canvas )
  , mMessageBar( messageBar )
  , mBlockAutoApply( false )
  , mPageFactories( pages )
{
  setupUi( this );

  mOptionsListWidget->setIconSize( QgisApp::instance()->iconSize( false ) );
  mOptionsListWidget->setMaximumWidth( static_cast< int >( mOptionsListWidget->iconSize().width() * 1.18 ) );

  connect( QgsProject::instance(), static_cast < void ( QgsProject::* )( QgsMapLayer * ) > ( &QgsProject::layerWillBeRemoved ), this, &QgsLayerStylingWidget::layerAboutToBeRemoved );

  QgsSettings settings;
  mLiveApplyCheck->setChecked( settings.value( QStringLiteral( "UI/autoApplyStyling" ), true ).toBool() );

  mAutoApplyTimer = new QTimer( this );
  mAutoApplyTimer->setSingleShot( true );

  mUndoWidget = new QgsUndoWidget( this, mMapCanvas );
  mUndoWidget->setButtonsVisible( false );
  mUndoWidget->setAutoDelete( false );
  mUndoWidget->setObjectName( QStringLiteral( "Undo Styles" ) );
  mUndoWidget->hide();

  mStyleManagerFactory = new QgsLayerStyleManagerWidgetFactory();

  setPageFactories( pages );

  connect( mUndoButton, &QAbstractButton::pressed, this, &QgsLayerStylingWidget::undo );
  connect( mRedoButton, &QAbstractButton::pressed, this, &QgsLayerStylingWidget::redo );

  connect( mAutoApplyTimer, &QTimer::timeout, this, &QgsLayerStylingWidget::apply );

  connect( mOptionsListWidget, &QListWidget::currentRowChanged, this, &QgsLayerStylingWidget::updateCurrentWidgetLayer );
  connect( mButtonBox->button( QDialogButtonBox::Apply ), &QAbstractButton::clicked, this, &QgsLayerStylingWidget::apply );
  connect( mLayerCombo, &QgsMapLayerComboBox::layerChanged, this, &QgsLayerStylingWidget::setLayer );
  connect( mLiveApplyCheck, &QAbstractButton::toggled, this, &QgsLayerStylingWidget::liveApplyToggled );

  mStackedWidget->setCurrentIndex( 0 );
}

QgsLayerStylingWidget::~QgsLayerStylingWidget()
{
  delete mStyleManagerFactory;
}

void QgsLayerStylingWidget::setPageFactories( const QList<QgsMapLayerConfigWidgetFactory *> &factories )
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
    disconnect( mCurrentLayer, &QgsMapLayer::styleChanged, this, &QgsLayerStylingWidget::updateCurrentWidgetLayer );
  }
  else
  {
    connect( mCurrentLayer, &QgsMapLayer::styleChanged, this, &QgsLayerStylingWidget::updateCurrentWidgetLayer );
  }
}

void QgsLayerStylingWidget::setLayer( QgsMapLayer *layer )
{
  if ( layer == mCurrentLayer )
    return;

  if ( mCurrentLayer )
  {
    disconnect( mCurrentLayer, &QgsMapLayer::styleChanged, this, &QgsLayerStylingWidget::updateCurrentWidgetLayer );
  }

  if ( !layer || !layer->isSpatial() || !QgsProject::instance()->layerIsEmbedded( layer->id() ).isEmpty() )
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
    sameLayerType = mCurrentLayer->type() == layer->type();
  }

  mCurrentLayer = layer;

  mUndoWidget->setUndoStack( layer->undoStackStyles() );

  connect( mCurrentLayer, &QgsMapLayer::styleChanged, this, &QgsLayerStylingWidget::updateCurrentWidgetLayer );

  int lastPage = mOptionsListWidget->currentIndex().row();
  mOptionsListWidget->blockSignals( true );
  mOptionsListWidget->clear();
  mUserPages.clear();

  switch ( layer->type() )
  {
    case QgsMapLayerType::VectorLayer:
    {
      QListWidgetItem *symbolItem = new QListWidgetItem( QgsApplication::getThemeIcon( QStringLiteral( "propertyicons/symbology.svg" ) ), QString() );
      symbolItem->setData( Qt::UserRole, Symbology );
      symbolItem->setToolTip( tr( "Symbology" ) );
      mOptionsListWidget->addItem( symbolItem );
      QListWidgetItem *labelItem = new QListWidgetItem( QgsApplication::getThemeIcon( QStringLiteral( "labelingSingle.svg" ) ), QString() );
      labelItem->setData( Qt::UserRole, VectorLabeling );
      labelItem->setToolTip( tr( "Labels" ) );
      mOptionsListWidget->addItem( labelItem );

#ifdef HAVE_3D
      QListWidgetItem *symbol3DItem = new QListWidgetItem( QgsApplication::getThemeIcon( QStringLiteral( "3d.svg" ) ), QString() );
      symbol3DItem->setData( Qt::UserRole, Symbology3D );
      symbol3DItem->setToolTip( tr( "3D View" ) );
      mOptionsListWidget->addItem( symbol3DItem );
#endif
      break;
    }
    case QgsMapLayerType::RasterLayer:
    {
      QListWidgetItem *symbolItem = new QListWidgetItem( QgsApplication::getThemeIcon( QStringLiteral( "propertyicons/symbology.svg" ) ), QString() );
      symbolItem->setData( Qt::UserRole, Symbology );
      symbolItem->setToolTip( tr( "Symbology" ) );
      mOptionsListWidget->addItem( symbolItem );
      QListWidgetItem *transparencyItem = new QListWidgetItem( QgsApplication::getThemeIcon( QStringLiteral( "propertyicons/transparency.svg" ) ), QString() );
      transparencyItem->setToolTip( tr( "Transparency" ) );
      transparencyItem->setData( Qt::UserRole, RasterTransparency );
      mOptionsListWidget->addItem( transparencyItem );

      if ( static_cast<QgsRasterLayer *>( layer )->dataProvider()->capabilities() & QgsRasterDataProvider::Size )
      {
        QListWidgetItem *histogramItem = new QListWidgetItem( QgsApplication::getThemeIcon( QStringLiteral( "propertyicons/histogram.svg" ) ), QString() );
        histogramItem->setData( Qt::UserRole, RasterHistogram );
        mOptionsListWidget->addItem( histogramItem );
        histogramItem->setToolTip( tr( "Histogram" ) );
      }
      break;
    }
    case QgsMapLayerType::MeshLayer:
    {
      QListWidgetItem *symbolItem = new QListWidgetItem( QgsApplication::getThemeIcon( QStringLiteral( "propertyicons/symbology.svg" ) ), QString() );
      symbolItem->setData( Qt::UserRole, Symbology );
      symbolItem->setToolTip( tr( "Symbology" ) );
      mOptionsListWidget->addItem( symbolItem );

#ifdef HAVE_3D
      QListWidgetItem *symbol3DItem = new QListWidgetItem( QgsApplication::getThemeIcon( QStringLiteral( "3d.svg" ) ), QString() );
      symbol3DItem->setData( Qt::UserRole, Symbology3D );
      symbol3DItem->setToolTip( tr( "3D View" ) );
      mOptionsListWidget->addItem( symbol3DItem );
#endif
      break;
    }

    case QgsMapLayerType::PluginLayer:
      break;
  }

  Q_FOREACH ( QgsMapLayerConfigWidgetFactory *factory, mPageFactories )
  {
    if ( factory->supportsStyleDock() && factory->supportsLayer( layer ) )
    {
      QListWidgetItem *item = new QListWidgetItem( factory->icon(), QString() );
      item->setToolTip( factory->title() );
      mOptionsListWidget->addItem( item );
      int row = mOptionsListWidget->row( item );
      mUserPages[row] = factory;
    }
  }
  QListWidgetItem *historyItem = new QListWidgetItem( QgsApplication::getThemeIcon( QStringLiteral( "mActionHistory.svg" ) ), QString() );
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
  QDomDocument doc( QStringLiteral( "style" ) );
  mLastStyleXml = doc.createElement( QStringLiteral( "style" ) );
  doc.appendChild( mLastStyleXml );
  mCurrentLayer->writeStyle( mLastStyleXml, doc, errorMsg, QgsReadWriteContext() );
}

void QgsLayerStylingWidget::apply()
{
  if ( !mCurrentLayer )
    return;

  disconnect( mCurrentLayer, &QgsMapLayer::styleChanged, this, &QgsLayerStylingWidget::updateCurrentWidgetLayer );

  QString undoName = QStringLiteral( "Style Change" );

  QWidget *current = mWidgetStack->mainPanel();

  bool styleWasChanged = false;
  if ( QgsLabelingWidget *widget = qobject_cast<QgsLabelingWidget *>( current ) )
  {
    widget->apply();
    styleWasChanged = true;
    undoName = QStringLiteral( "Label Change" );
  }
  if ( QgsPanelWidgetWrapper *wrapper = qobject_cast<QgsPanelWidgetWrapper *>( current ) )
  {
    if ( QgsRendererPropertiesDialog *widget = qobject_cast<QgsRendererPropertiesDialog *>( wrapper->widget() ) )
    {
      widget->apply();
      QgsVectorLayer *layer = qobject_cast<QgsVectorLayer *>( mCurrentLayer );
      QgsRendererAbstractMetadata *m = QgsApplication::rendererRegistry()->rendererMetadata( layer->renderer()->type() );
      undoName = QStringLiteral( "Style Change - %1" ).arg( m->visibleName() );
      styleWasChanged = true;
    }
  }
  else if ( QgsRasterTransparencyWidget *widget = qobject_cast<QgsRasterTransparencyWidget *>( current ) )
  {
    widget->apply();
    styleWasChanged = true;
  }
  else if ( qobject_cast<QgsRasterHistogramWidget *>( current ) )
  {
    mRasterStyleWidget->apply();
    styleWasChanged = true;
  }
  else if ( QgsMapLayerConfigWidget *widget = qobject_cast<QgsMapLayerConfigWidget *>( current ) )
  {
    widget->apply();
    styleWasChanged = true;
  }

  pushUndoItem( undoName );

  if ( styleWasChanged )
  {
    emit styleChanged( mCurrentLayer );
    QgsProject::instance()->setDirty( true );
    mCurrentLayer->triggerRepaint();
  }
  connect( mCurrentLayer, &QgsMapLayer::styleChanged, this, &QgsLayerStylingWidget::updateCurrentWidgetLayer );
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

  QgsPanelWidget *current = mWidgetStack->takeMainPanel();
  if ( current )
  {
    if ( QgsLabelingWidget *widget = qobject_cast<QgsLabelingWidget *>( current ) )
    {
      mLabelingWidget = widget;
    }
    else if ( QgsUndoWidget *widget = qobject_cast<QgsUndoWidget *>( current ) )
    {
      mUndoWidget = widget;
    }
    else if ( QgsRendererRasterPropertiesWidget *widget = qobject_cast<QgsRendererRasterPropertiesWidget *>( current ) )
    {
      mRasterStyleWidget = widget;
    }
#ifdef HAVE_3D
    else if ( QgsVectorLayer3DRendererWidget *widget = qobject_cast<QgsVectorLayer3DRendererWidget *>( current ) )
    {
      mVector3DWidget = widget;
    }
#endif
    else if ( QgsRendererMeshPropertiesWidget *widget = qobject_cast<QgsRendererMeshPropertiesWidget *>( current ) )
    {
      mMeshStyleWidget = widget;
    }
#ifdef HAVE_3D
    else if ( QgsMeshLayer3DRendererWidget *widget = qobject_cast<QgsMeshLayer3DRendererWidget *>( current ) )
    {
      mMesh3DWidget = widget;
    }
#endif
  }

  mWidgetStack->clear();
  // Create the user page widget if we are on one of those pages
  // TODO Make all widgets use this method.
  if ( mUserPages.contains( row ) )
  {
    QgsMapLayerConfigWidget *panel = mUserPages[row]->createWidget( mCurrentLayer, mMapCanvas, true, mWidgetStack );
    if ( panel )
    {
      connect( panel, &QgsPanelWidget::widgetChanged, this, &QgsLayerStylingWidget::autoApply );
      mWidgetStack->setMainPanel( panel );
    }
  }

  // The last widget is always the undo stack.
  if ( row == mOptionsListWidget->count() - 1 )
  {
    mWidgetStack->setMainPanel( mUndoWidget );
  }
  else
  {
    switch ( mCurrentLayer->type() )
    {
      case QgsMapLayerType::VectorLayer:
      {
        QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( mCurrentLayer );

        switch ( row )
        {
          case 0: // Style
          {
            QgsRendererPropertiesDialog *styleWidget = new QgsRendererPropertiesDialog( vlayer, QgsStyle::defaultStyle(), true, mStackedWidget );
            QgsSymbolWidgetContext context;
            context.setMapCanvas( mMapCanvas );
            context.setMessageBar( mMessageBar );
            styleWidget->setContext( context );
            styleWidget->setDockMode( true );
            connect( styleWidget, &QgsRendererPropertiesDialog::widgetChanged, this, &QgsLayerStylingWidget::autoApply );
            QgsPanelWidgetWrapper *wrapper = new QgsPanelWidgetWrapper( styleWidget, mStackedWidget );
            wrapper->setDockMode( true );
            connect( styleWidget, &QgsRendererPropertiesDialog::showPanel, wrapper, &QgsPanelWidget::openPanel );
            mWidgetStack->setMainPanel( wrapper );
            break;
          }
          case 1: // Labels
          {
            if ( !mLabelingWidget )
            {
              mLabelingWidget = new QgsLabelingWidget( nullptr, mMapCanvas, mWidgetStack );
              mLabelingWidget->setDockMode( true );
              connect( mLabelingWidget, &QgsLabelingWidget::widgetChanged, this, &QgsLayerStylingWidget::autoApply );
            }
            mLabelingWidget->setLayer( vlayer );
            mWidgetStack->setMainPanel( mLabelingWidget );
            break;
          }
#ifdef HAVE_3D
          case 2:  // 3D View
          {
            if ( !mVector3DWidget )
            {
              mVector3DWidget = new QgsVectorLayer3DRendererWidget( nullptr, mMapCanvas, mWidgetStack );
              mVector3DWidget->setDockMode( true );
              connect( mVector3DWidget, &QgsVectorLayer3DRendererWidget::widgetChanged, this, &QgsLayerStylingWidget::autoApply );
            }
            mVector3DWidget->setLayer( vlayer );
            mWidgetStack->setMainPanel( mVector3DWidget );
            break;
          }
#endif
          default:
            break;
        }
        break;
      }

      case QgsMapLayerType::RasterLayer:
      {
        QgsRasterLayer *rlayer = qobject_cast<QgsRasterLayer *>( mCurrentLayer );
        bool hasMinMaxCollapsedState = false;
        bool minMaxCollapsed = false;

        switch ( row )
        {
          case 0: // Style
          {
            // Backup collapsed state of min/max group so as to restore it
            // on the new widget.
            if ( mRasterStyleWidget )
            {
              QgsRasterRendererWidget *currentRenderWidget = mRasterStyleWidget->currentRenderWidget();
              if ( currentRenderWidget )
              {
                QgsRasterMinMaxWidget *mmWidget = currentRenderWidget->minMaxWidget();
                if ( mmWidget )
                {
                  hasMinMaxCollapsedState = true;
                  minMaxCollapsed = mmWidget->isCollapsed();
                }
              }
            }
            mRasterStyleWidget = new QgsRendererRasterPropertiesWidget( rlayer, mMapCanvas, mWidgetStack );
            if ( hasMinMaxCollapsedState )
            {
              QgsRasterRendererWidget *currentRenderWidget = mRasterStyleWidget->currentRenderWidget();
              if ( currentRenderWidget )
              {
                QgsRasterMinMaxWidget *mmWidget = currentRenderWidget->minMaxWidget();
                if ( mmWidget )
                {
                  mmWidget->setCollapsed( minMaxCollapsed );
                }
              }
            }
            mRasterStyleWidget->setDockMode( true );
            connect( mRasterStyleWidget, &QgsPanelWidget::widgetChanged, this, &QgsLayerStylingWidget::autoApply );
            mWidgetStack->setMainPanel( mRasterStyleWidget );
            break;
          }

          case 1: // Transparency
          {
            QgsRasterTransparencyWidget *transwidget = new QgsRasterTransparencyWidget( rlayer, mMapCanvas, mWidgetStack );
            transwidget->setDockMode( true );
            connect( transwidget, &QgsPanelWidget::widgetChanged, this, &QgsLayerStylingWidget::autoApply );
            mWidgetStack->setMainPanel( transwidget );
            break;
          }
          case 2: // Histogram
          {
            if ( rlayer->dataProvider()->capabilities() & QgsRasterDataProvider::Size )
            {
              if ( !mRasterStyleWidget )
              {
                mRasterStyleWidget = new QgsRendererRasterPropertiesWidget( rlayer, mMapCanvas, mWidgetStack );
                mRasterStyleWidget->syncToLayer( rlayer );
              }
              connect( mRasterStyleWidget, &QgsPanelWidget::widgetChanged, this, &QgsLayerStylingWidget::autoApply );

              QgsRasterHistogramWidget *widget = new QgsRasterHistogramWidget( rlayer, mWidgetStack );
              connect( widget, &QgsPanelWidget::widgetChanged, this, &QgsLayerStylingWidget::autoApply );
              QString name = mRasterStyleWidget->currentRenderWidget()->renderer()->type();
              widget->setRendererWidget( name, mRasterStyleWidget->currentRenderWidget() );
              widget->setDockMode( true );

              mWidgetStack->setMainPanel( widget );
            }
            break;
          }
          default:
            break;
        }
        break;
      }

      case QgsMapLayerType::MeshLayer:
      {
        QgsMeshLayer *meshLayer = qobject_cast<QgsMeshLayer *>( mCurrentLayer );
        switch ( row )
        {
          case 0: // Style
          {
            mMeshStyleWidget = new QgsRendererMeshPropertiesWidget( meshLayer, mMapCanvas, mWidgetStack );

            mMeshStyleWidget->setDockMode( true );
            connect( mMeshStyleWidget, &QgsPanelWidget::widgetChanged, this, &QgsLayerStylingWidget::autoApply );
            mWidgetStack->setMainPanel( mMeshStyleWidget );
            break;
          }
#ifdef HAVE_3D
          case 1:  // 3D View
          {
            if ( !mMesh3DWidget )
            {
              mMesh3DWidget = new QgsMeshLayer3DRendererWidget( nullptr, mMapCanvas, mWidgetStack );
              mMesh3DWidget->setDockMode( true );
              connect( mMesh3DWidget, &QgsMeshLayer3DRendererWidget::widgetChanged, this, &QgsLayerStylingWidget::autoApply );
            }
            mMesh3DWidget->setLayer( meshLayer );
            mWidgetStack->setMainPanel( mMesh3DWidget );
            break;
          }
#endif
          default:
            break;
        }
        break;
      }

      case QgsMapLayerType::PluginLayer:
      {
        mStackedWidget->setCurrentIndex( mNotSupportedPage );
        break;
      }
    }
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

void QgsLayerStylingWidget::layerAboutToBeRemoved( QgsMapLayer *layer )
{
  if ( layer == mCurrentLayer )
  {
    mAutoApplyTimer->stop();
    setLayer( nullptr );
  }
}

void QgsLayerStylingWidget::liveApplyToggled( bool value )
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "UI/autoApplyStyling" ), value );
}

void QgsLayerStylingWidget::pushUndoItem( const QString &name )
{
  QString errorMsg;
  QDomDocument doc( QStringLiteral( "style" ) );
  QDomElement rootNode = doc.createElement( QStringLiteral( "qgis" ) );
  doc.appendChild( rootNode );
  mCurrentLayer->writeStyle( rootNode, doc, errorMsg, QgsReadWriteContext() );
  mCurrentLayer->undoStackStyles()->push( new QgsMapLayerStyleCommand( mCurrentLayer, name, rootNode, mLastStyleXml ) );
  // Override the last style on the stack
  mLastStyleXml = rootNode.cloneNode();
}


QgsMapLayerStyleCommand::QgsMapLayerStyleCommand( QgsMapLayer *layer, const QString &text, const QDomNode &current, const QDomNode &last )
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
  QgsReadWriteContext context = QgsReadWriteContext();
  mLayer->readStyle( mLastState, error, context );
  mLayer->triggerRepaint();
}

void QgsMapLayerStyleCommand::redo()
{
  QString error;
  QgsReadWriteContext context = QgsReadWriteContext();
  mLayer->readStyle( mXml, error, context );
  mLayer->triggerRepaint();
}

bool QgsMapLayerStyleCommand::mergeWith( const QUndoCommand *other )
{
  if ( other->id() != id() ) // make sure other is also an QgsMapLayerStyleCommand
    return false;

  const QgsMapLayerStyleCommand *otherCmd = static_cast<const QgsMapLayerStyleCommand *>( other );
  if ( otherCmd->mLayer != mLayer )
    return false;  // should never happen though...

  // only merge commands if they are created shortly after each other
  // (e.g. user keeps modifying one property)
  QgsSettings settings;
  int timeout = settings.value( QStringLiteral( "UI/styleUndoMergeTimeout" ), 500 ).toInt();
  if ( mTime.msecsTo( otherCmd->mTime ) > timeout )
    return false;

  mXml = otherCmd->mXml;
  mTime = otherCmd->mTime;
  return true;
}

QgsLayerStyleManagerWidgetFactory::QgsLayerStyleManagerWidgetFactory()
{
  setIcon( QgsApplication::getThemeIcon( QStringLiteral( "propertyicons/stylepreset.svg" ) ) );
  setTitle( QObject::tr( "Style Manager" ) );
}

QgsMapLayerConfigWidget *QgsLayerStyleManagerWidgetFactory::createWidget( QgsMapLayer *layer, QgsMapCanvas *canvas, bool dockMode, QWidget *parent ) const
{
  Q_UNUSED( dockMode );
  return new QgsMapLayerStyleManagerWidget( layer,  canvas, parent );

}

bool QgsLayerStyleManagerWidgetFactory::supportsLayer( QgsMapLayer *layer ) const
{
  switch ( layer->type() )
  {
    case QgsMapLayerType::VectorLayer:
    case QgsMapLayerType::RasterLayer:
    case QgsMapLayerType::MeshLayer:
      return true;

    case QgsMapLayerType::PluginLayer:
      return false;
  }
  return false; // no warnings
}
