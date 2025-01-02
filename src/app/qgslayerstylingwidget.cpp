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
#include "qgsmaskingwidget.h"
#include "qgsdiagramwidget.h"
#include "qgslayerstylingwidget.h"
#include "moc_qgslayerstylingwidget.cpp"
#include "qgsrastertransparencywidget.h"
#include "qgsrendererpropertiesdialog.h"
#include "qgsrendererrasterpropertieswidget.h"
#include "qgsrenderermeshpropertieswidget.h"
#include "qgsrasterhistogramwidget.h"
#include "qgsrasterattributetablewidget.h"
#include "qgsrasterrenderer.h"
#include "qgsrasterrendererwidget.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerstylemanager.h"
#include "qgsstyle.h"
#include "qgsvectorlayer.h"
#include "qgsvectortilelayer.h"
#include "qgsvectortilebasiclabelingwidget.h"
#include "qgsvectortilebasicrendererwidget.h"
#include "qgsmeshlayer.h"
#include "qgsmeshlabelingwidget.h"
#include "qgsproject.h"
#include "qgsundowidget.h"
#include "qgsreadwritecontext.h"
#include "qgsrenderer.h"
#include "qgsrendererregistry.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterlayer.h"
#include "qgsmaplayerconfigwidget.h"
#include "qgsmaplayerstylemanagerwidget.h"
#include "qgsrasterminmaxwidget.h"
#include "qgisapp.h"
#include "qgssymbolwidgetcontext.h"
#include "qgsannotationlayer.h"
#include "qgsrasterlabelingwidget.h"

#ifdef HAVE_3D
#include "qgsvectorlayer3drendererwidget.h"
#include "qgsmeshlayer3drendererwidget.h"
#endif


QgsLayerStylingWidget::QgsLayerStylingWidget( QgsMapCanvas *canvas, QgsMessageBar *messageBar, const QList<const QgsMapLayerConfigWidgetFactory *> &pages, QWidget *parent )
  : QWidget( parent )
  , mNotSupportedPage( 0 )
  , mLayerPage( 1 )
  , mMapCanvas( canvas )
  , mMessageBar( messageBar )
  , mBlockAutoApply( false )
  , mPageFactories( pages )
{
  setupUi( this );

  mContext.setMapCanvas( canvas );
  mContext.setMessageBar( messageBar );

  mOptionsListWidget->setIconSize( QgisApp::instance()->iconSize( false ) );
  mOptionsListWidget->setMaximumWidth( static_cast<int>( mOptionsListWidget->iconSize().width() * 1.18 ) );

  connect( QgsProject::instance(), static_cast<void ( QgsProject::* )( QgsMapLayer * )>( &QgsProject::layerWillBeRemoved ), this, &QgsLayerStylingWidget::layerAboutToBeRemoved );

  QgsSettings settings;
  mLiveApplyCheck->setChecked( settings.value( QStringLiteral( "UI/autoApplyStyling" ), true ).toBool() );
  mButtonBox->button( QDialogButtonBox::Apply )->setEnabled( !mLiveApplyCheck->isChecked() );

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

  mLayerCombo->setFilters( Qgis::LayerFilter::HasGeometry | Qgis::LayerFilter::RasterLayer | Qgis::LayerFilter::PluginLayer | Qgis::LayerFilter::MeshLayer | Qgis::LayerFilter::VectorTileLayer | Qgis::LayerFilter::PointCloudLayer | Qgis::LayerFilter::TiledSceneLayer | Qgis::LayerFilter::AnnotationLayer );
  mLayerCombo->setAdditionalLayers( { QgsProject::instance()->mainAnnotationLayer() } );

  mStackedWidget->setCurrentIndex( 0 );
}

QgsLayerStylingWidget::~QgsLayerStylingWidget()
{
  delete mStyleManagerFactory;
}

void QgsLayerStylingWidget::setPageFactories( const QList<const QgsMapLayerConfigWidgetFactory *> &factories )
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


  // when current layer is changed, apply the main panel stack to allow it to gracefully clean up
  mWidgetStack->acceptAllPanels();

  if ( mCurrentLayer )
  {
    disconnect( mCurrentLayer, &QgsMapLayer::styleChanged, this, &QgsLayerStylingWidget::updateCurrentWidgetLayer );
    disconnect( mCurrentLayer->styleManager(), &QgsMapLayerStyleManager::currentStyleChanged, this, &QgsLayerStylingWidget::emitLayerStyleChanged );
    disconnect( mCurrentLayer->styleManager(), &QgsMapLayerStyleManager::styleRenamed, this, &QgsLayerStylingWidget::emitLayerStyleRenamed );
  }

  if ( !layer || !layer->isSpatial() || !QgsProject::instance()->layerIsEmbedded( layer->id() ).isEmpty() )
  {
    mLayerCombo->setLayer( nullptr );
    mStackedWidget->setCurrentIndex( mNotSupportedPage );
    mLastStyleXml.clear();
    mCurrentLayer = nullptr;
    emitLayerStyleChanged( QString() );
    return;
  }

  bool sameLayerType = false;
  if ( mCurrentLayer )
  {
    sameLayerType = mCurrentLayer->type() == layer->type();
  }

  mCurrentLayer = layer;
  mContext.setLayerTreeGroup( nullptr );

  mUndoWidget->setUndoStack( layer->undoStackStyles() );

  connect( mCurrentLayer, &QgsMapLayer::styleChanged, this, &QgsLayerStylingWidget::updateCurrentWidgetLayer );
  connect( mCurrentLayer->styleManager(), &QgsMapLayerStyleManager::currentStyleChanged, this, &QgsLayerStylingWidget::emitLayerStyleChanged );
  connect( mCurrentLayer->styleManager(), &QgsMapLayerStyleManager::styleRenamed, this, &QgsLayerStylingWidget::emitLayerStyleRenamed );

  int lastPage = mOptionsListWidget->currentIndex().row();
  mOptionsListWidget->blockSignals( true );
  mOptionsListWidget->clear();
  mUserPages.clear();

  switch ( layer->type() )
  {
    case Qgis::LayerType::Vector:
    {
      QListWidgetItem *symbolItem = new QListWidgetItem( QgsApplication::getThemeIcon( QStringLiteral( "propertyicons/symbology.svg" ) ), QString() );
      symbolItem->setData( Qt::UserRole, Symbology );
      symbolItem->setToolTip( tr( "Symbology" ) );
      mOptionsListWidget->addItem( symbolItem );
      QListWidgetItem *labelItem = new QListWidgetItem( QgsApplication::getThemeIcon( QStringLiteral( "labelingSingle.svg" ) ), QString() );
      labelItem->setData( Qt::UserRole, VectorLabeling );
      labelItem->setToolTip( tr( "Labels" ) );
      mOptionsListWidget->addItem( labelItem );
      QListWidgetItem *maskItem = new QListWidgetItem( QgsApplication::getThemeIcon( QStringLiteral( "propertyicons/labelmask.svg" ) ), QString() );
      maskItem->setData( Qt::UserRole, VectorLabeling );
      maskItem->setToolTip( tr( "Masks" ) );
      mOptionsListWidget->addItem( maskItem );

#ifdef HAVE_3D
      QListWidgetItem *symbol3DItem = new QListWidgetItem( QgsApplication::getThemeIcon( QStringLiteral( "3d.svg" ) ), QString() );
      symbol3DItem->setData( Qt::UserRole, Symbology3D );
      symbol3DItem->setToolTip( tr( "3D View" ) );
      mOptionsListWidget->addItem( symbol3DItem );
#endif

      QListWidgetItem *diagramItem = new QListWidgetItem( QgsApplication::getThemeIcon( QStringLiteral( "/propertyicons/diagram.svg" ) ), QString() );
      diagramItem->setData( Qt::UserRole, VectorDiagram );
      diagramItem->setToolTip( tr( "Diagrams" ) );
      mOptionsListWidget->addItem( diagramItem );
      break;
    }
    case Qgis::LayerType::Raster:
    {
      QListWidgetItem *symbolItem = new QListWidgetItem( QgsApplication::getThemeIcon( QStringLiteral( "propertyicons/symbology.svg" ) ), QString() );
      symbolItem->setData( Qt::UserRole, Symbology );
      symbolItem->setToolTip( tr( "Symbology" ) );
      mOptionsListWidget->addItem( symbolItem );
      QListWidgetItem *transparencyItem = new QListWidgetItem( QgsApplication::getThemeIcon( QStringLiteral( "propertyicons/transparency.svg" ) ), QString() );
      transparencyItem->setToolTip( tr( "Transparency" ) );
      transparencyItem->setData( Qt::UserRole, RasterTransparency );
      mOptionsListWidget->addItem( transparencyItem );

      QListWidgetItem *labelItem = new QListWidgetItem( QgsApplication::getThemeIcon( QStringLiteral( "labelingSingle.svg" ) ), QString() );
      labelItem->setData( Qt::UserRole, VectorLabeling );
      labelItem->setToolTip( tr( "Labels" ) );
      mOptionsListWidget->addItem( labelItem );

      if ( static_cast<QgsRasterLayer *>( layer )->dataProvider() && static_cast<QgsRasterLayer *>( layer )->dataProvider()->capabilities() & Qgis::RasterInterfaceCapability::Size )
      {
        QListWidgetItem *histogramItem = new QListWidgetItem( QgsApplication::getThemeIcon( QStringLiteral( "propertyicons/histogram.svg" ) ), QString() );
        histogramItem->setData( Qt::UserRole, RasterHistogram );
        mOptionsListWidget->addItem( histogramItem );
        histogramItem->setToolTip( tr( "Histogram" ) );
      }

      QListWidgetItem *rasterAttributeTableItem = new QListWidgetItem( QgsApplication::getThemeIcon( QStringLiteral( "propertyicons/attributes.svg" ) ), QString() );
      rasterAttributeTableItem->setToolTip( tr( "Raster Attribute Tables" ) );
      rasterAttributeTableItem->setData( Qt::UserRole, RasterAttributeTables );
      mOptionsListWidget->addItem( rasterAttributeTableItem );
      break;
    }
    case Qgis::LayerType::Mesh:
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

    case Qgis::LayerType::VectorTile:
    {
      QListWidgetItem *symbolItem = new QListWidgetItem( QgsApplication::getThemeIcon( QStringLiteral( "propertyicons/symbology.svg" ) ), QString() );
      symbolItem->setData( Qt::UserRole, Symbology );
      symbolItem->setToolTip( tr( "Symbology" ) );
      mOptionsListWidget->addItem( symbolItem );
      QListWidgetItem *labelItem = new QListWidgetItem( QgsApplication::getThemeIcon( QStringLiteral( "labelingSingle.svg" ) ), QString() );
      labelItem->setData( Qt::UserRole, VectorLabeling );
      labelItem->setToolTip( tr( "Labels" ) );
      mOptionsListWidget->addItem( labelItem );
      break;
    }

    case Qgis::LayerType::PointCloud:
    case Qgis::LayerType::Plugin:
    case Qgis::LayerType::Annotation:
    case Qgis::LayerType::Group:
    case Qgis::LayerType::TiledScene:
      break;
  }

  for ( const QgsMapLayerConfigWidgetFactory *factory : std::as_const( mPageFactories ) )
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
  emit layerStyleChanged( mCurrentLayer->styleManager()->currentStyle() );
}

void QgsLayerStylingWidget::apply()
{
  if ( mCurrentLayer )
  {
    disconnect( mCurrentLayer, &QgsMapLayer::styleChanged, this, &QgsLayerStylingWidget::updateCurrentWidgetLayer );
  }

  QString undoName = QStringLiteral( "Style Change" );

  QWidget *current = mWidgetStack->mainPanel();

  bool styleWasChanged = false;
  bool triggerRepaint = false; // whether the change needs the layer to be repainted
  if ( QgsMaskingWidget *widget = qobject_cast<QgsMaskingWidget *>( current ) )
  {
    widget->apply();
    styleWasChanged = true;
    undoName = QStringLiteral( "Mask Change" );
  }
  if ( QgsPanelWidgetWrapper *wrapper = qobject_cast<QgsPanelWidgetWrapper *>( current ) )
  {
    if ( mCurrentLayer )
    {
      if ( QgsRendererPropertiesDialog *widget = qobject_cast<QgsRendererPropertiesDialog *>( wrapper->widget() ) )
      {
        widget->apply();
        QgsVectorLayer *layer = qobject_cast<QgsVectorLayer *>( mCurrentLayer );
        QgsRendererAbstractMetadata *m = QgsApplication::rendererRegistry()->rendererMetadata( layer->renderer()->type() );
        undoName = QStringLiteral( "Style Change - %1" ).arg( m->visibleName() );
        styleWasChanged = true;
        triggerRepaint = true;
      }
    }
  }
  else if ( QgsRasterTransparencyWidget *widget = qobject_cast<QgsRasterTransparencyWidget *>( current ) )
  {
    widget->apply();
    styleWasChanged = true;
    triggerRepaint = true;
  }
  else if ( qobject_cast<QgsRasterHistogramWidget *>( current ) )
  {
    mRasterStyleWidget->apply();
    styleWasChanged = true;
    triggerRepaint = true;
  }
  else if ( QgsLabelingWidget *widget = qobject_cast<QgsLabelingWidget *>( current ) )
  {
    widget->apply();
    styleWasChanged = true;
    undoName = QStringLiteral( "Label Change" );
  }
  else if ( QgsDiagramWidget *widget = qobject_cast<QgsDiagramWidget *>( current ) )
  {
    widget->apply();
    styleWasChanged = true;
    undoName = QStringLiteral( "Diagram Change" );
  }
  else if ( QgsMapLayerConfigWidget *widget = qobject_cast<QgsMapLayerConfigWidget *>( current ) )
  {
    // Warning: All classes inheriting from QgsMapLayerConfigWidget
    // should come in the current if block, before this else-if
    // clause, to avoid duplicate calls to apply()!
    widget->apply();
    styleWasChanged = true;
    triggerRepaint = widget->shouldTriggerLayerRepaint();
  }

  if ( mCurrentLayer )
    pushUndoItem( undoName, triggerRepaint );

  if ( mCurrentLayer && styleWasChanged )
  {
    emit styleChanged( mCurrentLayer );
    QgsProject::instance()->setDirty( true );
  }

  if ( mCurrentLayer )
  {
    connect( mCurrentLayer, &QgsMapLayer::styleChanged, this, &QgsLayerStylingWidget::updateCurrentWidgetLayer );
  }
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
  if ( !mCurrentLayer && !mContext.layerTreeGroup() )
    return; // non-spatial are ignored in setLayer()

  mBlockAutoApply = true;

  if ( mCurrentLayer )
    whileBlocking( mLayerCombo )->setLayer( mCurrentLayer );

  int row = mOptionsListWidget->currentIndex().row();

  mStackedWidget->setCurrentIndex( mLayerPage );

  if ( QgsPanelWidget *current = mWidgetStack->takeMainPanel() )
  {
    if ( QgsLabelingWidget *widget = qobject_cast<QgsLabelingWidget *>( current ) )
    {
      mLabelingWidget = widget;
    }
    else if ( QgsMaskingWidget *widget = qobject_cast<QgsMaskingWidget *>( current ) )
    {
      mMaskingWidget = widget;
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
    else if ( QgsDiagramWidget *widget = qobject_cast<QgsDiagramWidget *>( current ) )
    {
      mDiagramWidget = widget;
    }
  }

  mWidgetStack->clear();
  // Create the user page widget if we are on one of those pages
  // TODO Make all widgets use this method.
  if ( mUserPages.contains( row ) )
  {
    QgsMapLayerConfigWidget *panel = mUserPages[row]->createWidget( mCurrentLayer, mMapCanvas, true, mWidgetStack );
    if ( panel )
    {
      panel->setDockMode( true );
      panel->setMapLayerConfigWidgetContext( mContext );
      connect( panel, &QgsPanelWidget::widgetChanged, this, &QgsLayerStylingWidget::autoApply );
      mWidgetStack->setMainPanel( panel );
      mBlockAutoApply = false;
      return;
    }
  }

  // The last widget is always the undo stack.
  if ( mCurrentLayer && row == mOptionsListWidget->count() - 1 )
  {
    mWidgetStack->setMainPanel( mUndoWidget );
  }
  else if ( mCurrentLayer )
  {
    switch ( mCurrentLayer->type() )
    {
      case Qgis::LayerType::Vector:
      {
        QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( mCurrentLayer );

#ifdef HAVE_3D
        const int tabShift = 1; // To move subsequent tabs
#else
        const int tabShift = 0;
#endif
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
              mLabelingWidget = new QgsLabelingWidget( nullptr, mMapCanvas, mWidgetStack, mMessageBar );
              mLabelingWidget->setDockMode( true );
              connect( mLabelingWidget, &QgsLabelingWidget::widgetChanged, this, &QgsLayerStylingWidget::autoApply );
            }
            mLabelingWidget->setLayer( vlayer );
            mWidgetStack->setMainPanel( mLabelingWidget );
            break;
          }
          case 2: // Masks
          {
            if ( !mMaskingWidget )
            {
              mMaskingWidget = new QgsMaskingWidget( mWidgetStack );
              mMaskingWidget->layout()->setContentsMargins( 0, 0, 0, 0 );
              connect( mMaskingWidget, &QgsMaskingWidget::widgetChanged, this, &QgsLayerStylingWidget::autoApply );
            }
            mMaskingWidget->setLayer( vlayer );
            mWidgetStack->setMainPanel( mMaskingWidget );
            break;
          }
#ifdef HAVE_3D
          case 3: // 3D View
          {
            if ( !mVector3DWidget )
            {
              mVector3DWidget = new QgsVectorLayer3DRendererWidget( vlayer, mMapCanvas, mWidgetStack );
              mVector3DWidget->setDockMode( true );
              connect( mVector3DWidget, &QgsVectorLayer3DRendererWidget::widgetChanged, this, &QgsLayerStylingWidget::autoApply );
            }
            mVector3DWidget->syncToLayer( vlayer );
            mWidgetStack->setMainPanel( mVector3DWidget );
            break;
          }
#endif
          case 3 + tabShift: // Diagrams
          {
            mDiagramWidget = new QgsDiagramWidget( vlayer, mMapCanvas, mWidgetStack );
            mDiagramWidget->setDockMode( true );
            connect( mDiagramWidget, &QgsDiagramWidget::widgetChanged, this, &QgsLayerStylingWidget::autoApply );
            mDiagramWidget->syncToOwnLayer();
            mWidgetStack->setMainPanel( mDiagramWidget );
            break;
          }
          default:
            break;
        }
        break;
      }

      case Qgis::LayerType::Raster:
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

            QgsSymbolWidgetContext context;
            context.setMapCanvas( mMapCanvas );
            context.setMessageBar( mMessageBar );
            transwidget->setContext( context );

            connect( transwidget, &QgsPanelWidget::widgetChanged, this, &QgsLayerStylingWidget::autoApply );
            mWidgetStack->setMainPanel( transwidget );
            break;
          }

          case 2: // Labeling
          {
            if ( !mRasterLabelingWidget )
            {
              mRasterLabelingWidget = new QgsRasterLabelingWidget( rlayer, mMapCanvas, mWidgetStack, mMessageBar );
              mRasterLabelingWidget->setDockMode( true );
              connect( mRasterLabelingWidget, &QgsPanelWidget::widgetChanged, this, &QgsLayerStylingWidget::autoApply );
            }
            else
            {
              mRasterLabelingWidget->setLayer( rlayer );
            }
            mWidgetStack->setMainPanel( mRasterLabelingWidget );
            break;
          }

          case 3: // Histogram
          {
            if ( rlayer->dataProvider()->capabilities() & Qgis::RasterInterfaceCapability::Size )
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

          case 4: // Attribute Tables
          {
            if ( rlayer->attributeTableCount() > 0 )
            {
              if ( !mRasterAttributeTableWidget )
              {
                mRasterAttributeTableWidget = new QgsRasterAttributeTableWidget( mWidgetStack, rlayer );
                mRasterAttributeTableWidget->setDockMode( true );
              }
              else
              {
                mRasterAttributeTableWidget->setRasterLayer( rlayer );
              }

              mWidgetStack->setMainPanel( mRasterAttributeTableWidget );
            }
            else
            {
              if ( !mRasterAttributeTableDisabledWidget )
              {
                mRasterAttributeTableDisabledWidget = new QgsPanelWidget { mWidgetStack };
                QVBoxLayout *layout = new QVBoxLayout { mRasterAttributeTableDisabledWidget };
                mRasterAttributeTableDisabledWidget->setLayout( layout );
                QLabel *label { new QLabel( tr( "There are no raster attribute tables associated with this data source.<br>"
                                                "If the current symbology can be converted to an attribute table you "
                                                "can create a new attribute table using the context menu available in the "
                                                "layer tree or in the layer properties dialog." ) ) };
                label->setWordWrap( true );
                mRasterAttributeTableDisabledWidget->layout()->addWidget( label );
                layout->addStretch();
                mRasterAttributeTableDisabledWidget->setDockMode( true );
              }
              mWidgetStack->setMainPanel( mRasterAttributeTableDisabledWidget );
            }

            break;
          }
          default:
            break;
        }
        break;
      }

      case Qgis::LayerType::Mesh:
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

            connect( meshLayer, &QgsMeshLayer::reloaded, this, [this] { mMeshStyleWidget->syncToLayer( mCurrentLayer ); } );
            break;
          }
          case 1: // Labeling
          {
            mMeshLabelingWidget = new QgsMeshLabelingWidget( meshLayer, mMapCanvas, mWidgetStack, mMessageBar );
            mMeshLabelingWidget->setDockMode( true );
            connect( mMeshLabelingWidget, &QgsPanelWidget::widgetChanged, this, &QgsLayerStylingWidget::autoApply );
            mWidgetStack->setMainPanel( mMeshLabelingWidget );
            break;
          }
#ifdef HAVE_3D
          case 2: // 3D View
          {
            if ( !mMesh3DWidget )
            {
              mMesh3DWidget = new QgsMeshLayer3DRendererWidget( nullptr, mMapCanvas, mWidgetStack );
              mMesh3DWidget->setDockMode( true );
              connect( mMesh3DWidget, &QgsMeshLayer3DRendererWidget::widgetChanged, this, &QgsLayerStylingWidget::autoApply );
            }
            mMesh3DWidget->syncToLayer( meshLayer );
            mWidgetStack->setMainPanel( mMesh3DWidget );

            connect( meshLayer, &QgsMeshLayer::reloaded, this, [this] { mMesh3DWidget->syncToLayer( mCurrentLayer ); } );
            break;
          }
#endif
          default:
            break;
        }
        break;
      }

      case Qgis::LayerType::VectorTile:
      {
        QgsVectorTileLayer *vtLayer = qobject_cast<QgsVectorTileLayer *>( mCurrentLayer );
        switch ( row )
        {
          case 0: // Style
          {
            mVectorTileStyleWidget = new QgsVectorTileBasicRendererWidget( vtLayer, mMapCanvas, mMessageBar, mWidgetStack );
            mVectorTileStyleWidget->setDockMode( true );
            connect( mVectorTileStyleWidget, &QgsPanelWidget::widgetChanged, this, &QgsLayerStylingWidget::autoApply );
            mWidgetStack->setMainPanel( mVectorTileStyleWidget );
            break;
          }
          case 1: // Labeling
          {
            mVectorTileLabelingWidget = new QgsVectorTileBasicLabelingWidget( vtLayer, mMapCanvas, mMessageBar, mWidgetStack );
            mVectorTileLabelingWidget->setDockMode( true );
            connect( mVectorTileLabelingWidget, &QgsPanelWidget::widgetChanged, this, &QgsLayerStylingWidget::autoApply );
            mWidgetStack->setMainPanel( mVectorTileLabelingWidget );
            break;
          }
          default:
            break;
        }
        break;
      }

      case Qgis::LayerType::PointCloud:
      case Qgis::LayerType::Annotation:
      case Qgis::LayerType::Group:
      case Qgis::LayerType::TiledScene:
      {
        break;
      }

      case Qgis::LayerType::Plugin:
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
    if ( data == static_cast<int>( page ) )
    {
      mOptionsListWidget->setCurrentRow( i );
      return;
    }
  }
}

void QgsLayerStylingWidget::setAnnotationItem( QgsAnnotationLayer *layer, const QString &itemId )
{
  mContext.setAnnotationId( itemId );
  if ( layer )
  {
    setLayer( layer );
    mStackedWidget->setCurrentIndex( mLayerPage );
  }

  if ( QgsMapLayerConfigWidget *configWidget = qobject_cast<QgsMapLayerConfigWidget *>( mWidgetStack->mainPanel() ) )
  {
    configWidget->setMapLayerConfigWidgetContext( mContext );
  }
}

void QgsLayerStylingWidget::setLayerTreeGroup( QgsLayerTreeGroup *group )
{
  mOptionsListWidget->blockSignals( true );
  mOptionsListWidget->clear();
  mUserPages.clear();

  for ( const QgsMapLayerConfigWidgetFactory *factory : std::as_const( mPageFactories ) )
  {
    if ( factory->supportsStyleDock() && factory->supportsLayerTreeGroup( group ) )
    {
      QListWidgetItem *item = new QListWidgetItem( factory->icon(), QString() );
      item->setToolTip( factory->title() );
      mOptionsListWidget->addItem( item );
      int row = mOptionsListWidget->row( item );
      mUserPages[row] = factory;
    }
  }

  mContext.setLayerTreeGroup( group );
  setLayer( nullptr );

  mOptionsListWidget->setCurrentRow( 0 );
  mOptionsListWidget->blockSignals( false );
  updateCurrentWidgetLayer();

  mStackedWidget->setCurrentIndex( 1 );

  if ( QgsMapLayerConfigWidget *configWidget = qobject_cast<QgsMapLayerConfigWidget *>( mWidgetStack->mainPanel() ) )
  {
    configWidget->setMapLayerConfigWidgetContext( mContext );
  }
}

void QgsLayerStylingWidget::focusDefaultWidget()
{
  if ( QgsMapLayerConfigWidget *configWidget = qobject_cast<QgsMapLayerConfigWidget *>( mWidgetStack->mainPanel() ) )
  {
    configWidget->focusDefaultWidget();
  }
}

void QgsLayerStylingWidget::layerAboutToBeRemoved( QgsMapLayer *layer )
{
  if ( layer == mCurrentLayer )
  {
    // when current layer is removed, apply the main panel stack to allow it to gracefully clean up
    mWidgetStack->acceptAllPanels();

    mAutoApplyTimer->stop();
    setLayer( nullptr );
  }
}

void QgsLayerStylingWidget::liveApplyToggled( bool liveUpdateEnabled )
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "UI/autoApplyStyling" ), liveUpdateEnabled );

  mButtonBox->button( QDialogButtonBox::Apply )->setEnabled( !liveUpdateEnabled );
}

void QgsLayerStylingWidget::pushUndoItem( const QString &name, bool triggerRepaint )
{
  QString errorMsg;
  QDomDocument doc( QStringLiteral( "style" ) );
  QDomElement rootNode = doc.createElement( QStringLiteral( "qgis" ) );
  doc.appendChild( rootNode );
  mCurrentLayer->writeStyle( rootNode, doc, errorMsg, QgsReadWriteContext() );
  mCurrentLayer->undoStackStyles()->push( new QgsMapLayerStyleCommand( mCurrentLayer, name, rootNode, mLastStyleXml, triggerRepaint ) );
  // Override the last style on the stack
  mLastStyleXml = rootNode.cloneNode();
}


void QgsLayerStylingWidget::emitLayerStyleRenamed()
{
  emit layerStyleChanged( mCurrentLayer->styleManager()->currentStyle() );
}


QgsMapLayerStyleCommand::QgsMapLayerStyleCommand( QgsMapLayer *layer, const QString &text, const QDomNode &current, const QDomNode &last, bool triggerRepaint )
  : QUndoCommand( text )
  , mLayer( layer )
  , mXml( current )
  , mLastState( last )
  , mTime( QTime::currentTime() )
  , mTriggerRepaint( triggerRepaint )
{
}

void QgsMapLayerStyleCommand::undo()
{
  QString error;
  QgsReadWriteContext context = QgsReadWriteContext();
  mLayer->readStyle( mLastState, error, context );
  if ( mTriggerRepaint )
    mLayer->triggerRepaint();
}

void QgsMapLayerStyleCommand::redo()
{
  QString error;
  QgsReadWriteContext context = QgsReadWriteContext();
  mLayer->readStyle( mXml, error, context );
  if ( mTriggerRepaint )
    mLayer->triggerRepaint();
}

bool QgsMapLayerStyleCommand::mergeWith( const QUndoCommand *other )
{
  if ( other->id() != id() ) // make sure other is also an QgsMapLayerStyleCommand
    return false;

  const QgsMapLayerStyleCommand *otherCmd = static_cast<const QgsMapLayerStyleCommand *>( other );
  if ( otherCmd->mLayer != mLayer )
    return false; // should never happen though...

  // only merge commands if they are created shortly after each other
  // (e.g. user keeps modifying one property)
  QgsSettings settings;
  int timeout = settings.value( QStringLiteral( "UI/styleUndoMergeTimeout" ), 500 ).toInt();
  if ( mTime.msecsTo( otherCmd->mTime ) > timeout )
    return false;

  mXml = otherCmd->mXml;
  mTime = otherCmd->mTime;
  mTriggerRepaint |= otherCmd->mTriggerRepaint;
  return true;
}

QgsLayerStyleManagerWidgetFactory::QgsLayerStyleManagerWidgetFactory()
{
  setIcon( QgsApplication::getThemeIcon( QStringLiteral( "propertyicons/stylepreset.svg" ) ) );
  setTitle( QObject::tr( "Style Manager" ) );
}

QgsMapLayerConfigWidget *QgsLayerStyleManagerWidgetFactory::createWidget( QgsMapLayer *layer, QgsMapCanvas *canvas, bool dockMode, QWidget *parent ) const
{
  Q_UNUSED( dockMode )
  return new QgsMapLayerStyleManagerWidget( layer, canvas, parent );
}

bool QgsLayerStyleManagerWidgetFactory::supportsLayer( QgsMapLayer *layer ) const
{
  switch ( layer->type() )
  {
    case Qgis::LayerType::Vector:
    case Qgis::LayerType::Raster:
    case Qgis::LayerType::Mesh:
    case Qgis::LayerType::VectorTile:
      return true;

    case Qgis::LayerType::PointCloud:
    case Qgis::LayerType::Plugin:
    case Qgis::LayerType::Annotation:
    case Qgis::LayerType::Group:
    case Qgis::LayerType::TiledScene:
      return false;
  }
  return false; // no warnings
}
