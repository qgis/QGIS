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

#include "qgslayerstylingwidget.h"

#include "annotations/qgsannotationitempropertieswidget.h"
#include "qgisapp.h"
#include "qgsannotationlayer.h"
#include "qgsapplication.h"
#include "qgsdiagramwidget.h"
#include "qgsgui.h"
#include "qgslabelingwidget.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerconfigwidget.h"
#include "qgsmaplayerstylemanager.h"
#include "qgsmaplayerstylemanagerwidget.h"
#include "qgsmaskingwidget.h"
#include "qgsmeshlabelingwidget.h"
#include "qgsmeshlayer.h"
#include "qgsproject.h"
#include "qgsrasterattributetablewidget.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterhistogramwidget.h"
#include "qgsrasterlabelingwidget.h"
#include "qgsrasterlayer.h"
#include "qgsrasterminmaxwidget.h"
#include "qgsrasterrenderer.h"
#include "qgsrasterrendererwidget.h"
#include "qgsrastertransparencywidget.h"
#include "qgsreadwritecontext.h"
#include "qgsrenderer.h"
#include "qgsrenderermeshpropertieswidget.h"
#include "qgsrendererpropertiesdialog.h"
#include "qgsrendererrasterpropertieswidget.h"
#include "qgsrendererregistry.h"
#include "qgssettingsentryimpl.h"
#include "qgssettingstree.h"
#include "qgsstyle.h"
#include "qgssymbolwidgetcontext.h"
#include "qgsundowidget.h"
#include "qgsvectorlayer.h"
#include "qgsvectortilebasiclabelingwidget.h"
#include "qgsvectortilebasicrendererwidget.h"
#include "qgsvectortilelayer.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QSizePolicy>
#include <QString>
#include <QUndoStack>
#include <QVBoxLayout>
#include <QWidget>

#include "moc_qgslayerstylingwidget.cpp"

using namespace Qt::StringLiterals;

#ifdef HAVE_3D
#include "qgsvectorlayer3drendererwidget.h"
#include "qgsmeshlayer3drendererwidget.h"
#endif


QgsLayerStylingWidget::QgsLayerStylingWidget( QgsMapCanvas *canvas, QgsMessageBar *messageBar, const QList<const QgsMapLayerConfigWidgetFactory *> &pages, QWidget *parent )
  : QWidget( parent )
  , mMapCanvas( canvas )
  , mMessageBar( messageBar )
  , mPageFactories( pages )
{
  setupUi( this );

  mContext.setMapCanvas( canvas );
  mContext.setMessageBar( messageBar );

  mOptionsListWidget->setIconSize( QgsGui::iconSize( Qgis::UserInterfaceIconType::MainWindowToolbar ) );
  mOptionsListWidget->setMaximumWidth( static_cast<int>( mOptionsListWidget->iconSize().width() * 1.18 ) );

  connect( QgsProject::instance(), static_cast<void ( QgsProject::* )( QgsMapLayer * )>( &QgsProject::layerWillBeRemoved ), this, &QgsLayerStylingWidget::layerAboutToBeRemoved );

  QgsSettings settings;
  mLiveApplyCheck->setChecked( settings.value( u"UI/autoApplyStyling"_s, true ).toBool() );
  mButtonBox->button( QDialogButtonBox::Apply )->setEnabled( !mLiveApplyCheck->isChecked() );

  mAutoApplyTimer = new QTimer( this );
  mAutoApplyTimer->setSingleShot( true );

  mUndoWidget = new QgsUndoWidget( this, mMapCanvas );
  mUndoWidget->setButtonsVisible( false );
  mUndoWidget->setAutoDelete( false );
  mUndoWidget->setObjectName( u"Undo Styles"_s );
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

  mLayerCombo->setFilters(
    Qgis::LayerFilter::HasGeometry
    | Qgis::LayerFilter::RasterLayer
    | Qgis::LayerFilter::PluginLayer
    | Qgis::LayerFilter::MeshLayer
    | Qgis::LayerFilter::VectorTileLayer
    | Qgis::LayerFilter::PointCloudLayer
    | Qgis::LayerFilter::TiledSceneLayer
    | Qgis::LayerFilter::AnnotationLayer
  );
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
      QListWidgetItem *symbolItem = new QListWidgetItem( QgsApplication::getThemeIcon( u"propertyicons/symbology.svg"_s ), QString() );
      symbolItem->setData( Qt::UserRole, QVariant::fromValue( Page::VectorRenderer ) );
      symbolItem->setToolTip( tr( "Symbology" ) );
      mOptionsListWidget->addItem( symbolItem );
      QListWidgetItem *labelItem = new QListWidgetItem( QgsApplication::getThemeIcon( u"labelingSingle.svg"_s ), QString() );
      labelItem->setData( Qt::UserRole, QVariant::fromValue( Page::VectorLabeling ) );
      labelItem->setToolTip( tr( "Labels" ) );
      mOptionsListWidget->addItem( labelItem );
      QListWidgetItem *maskItem = new QListWidgetItem( QgsApplication::getThemeIcon( u"propertyicons/labelmask.svg"_s ), QString() );
      maskItem->setData( Qt::UserRole, QVariant::fromValue( Page::VectorMasks ) );
      maskItem->setToolTip( tr( "Masks" ) );
      mOptionsListWidget->addItem( maskItem );

#ifdef HAVE_3D
      QListWidgetItem *symbol3DItem = new QListWidgetItem( QgsApplication::getThemeIcon( u"3d.svg"_s ), QString() );
      symbol3DItem->setData( Qt::UserRole, QVariant::fromValue( Page::Vector3D ) );
      symbol3DItem->setToolTip( tr( "3D View" ) );
      mOptionsListWidget->addItem( symbol3DItem );
#endif

      QListWidgetItem *diagramItem = new QListWidgetItem( QgsApplication::getThemeIcon( u"/propertyicons/diagram.svg"_s ), QString() );
      diagramItem->setData( Qt::UserRole, QVariant::fromValue( Page::VectorDiagrams ) );
      diagramItem->setToolTip( tr( "Diagrams" ) );
      mOptionsListWidget->addItem( diagramItem );
      break;
    }
    case Qgis::LayerType::Raster:
    {
      QListWidgetItem *symbolItem = new QListWidgetItem( QgsApplication::getThemeIcon( u"propertyicons/symbology.svg"_s ), QString() );
      symbolItem->setData( Qt::UserRole, QVariant::fromValue( Page::RasterRenderer ) );
      symbolItem->setToolTip( tr( "Symbology" ) );
      mOptionsListWidget->addItem( symbolItem );
      QListWidgetItem *transparencyItem = new QListWidgetItem( QgsApplication::getThemeIcon( u"propertyicons/transparency.svg"_s ), QString() );
      transparencyItem->setToolTip( tr( "Transparency" ) );
      transparencyItem->setData( Qt::UserRole, QVariant::fromValue( Page::RasterTransparency ) );
      mOptionsListWidget->addItem( transparencyItem );

      QListWidgetItem *labelItem = new QListWidgetItem( QgsApplication::getThemeIcon( u"labelingSingle.svg"_s ), QString() );
      labelItem->setData( Qt::UserRole, QVariant::fromValue( Page::RasterLabeling ) );
      labelItem->setToolTip( tr( "Labels" ) );
      mOptionsListWidget->addItem( labelItem );

      QgsRasterDataProvider *provider = qobject_cast<QgsRasterDataProvider *>( layer->dataProvider() );
      if ( provider && ( provider->capabilities() & Qgis::RasterInterfaceCapability::Size ) )
      {
        QListWidgetItem *histogramItem = new QListWidgetItem( QgsApplication::getThemeIcon( u"propertyicons/histogram.svg"_s ), QString() );
        histogramItem->setData( Qt::UserRole, QVariant::fromValue( Page::RasterHistogram ) );
        mOptionsListWidget->addItem( histogramItem );
        histogramItem->setToolTip( tr( "Histogram" ) );
      }

      QListWidgetItem *rasterAttributeTableItem = new QListWidgetItem( QgsApplication::getThemeIcon( u"propertyicons/attributes.svg"_s ), QString() );
      rasterAttributeTableItem->setToolTip( tr( "Raster Attribute Tables" ) );
      rasterAttributeTableItem->setData( Qt::UserRole, QVariant::fromValue( Page::RasterAttributeTable ) );
      mOptionsListWidget->addItem( rasterAttributeTableItem );
      break;
    }
    case Qgis::LayerType::Mesh:
    {
      QListWidgetItem *symbolItem = new QListWidgetItem( QgsApplication::getThemeIcon( u"propertyicons/symbology.svg"_s ), QString() );
      symbolItem->setData( Qt::UserRole, QVariant::fromValue( Page::MeshRenderer ) );
      symbolItem->setToolTip( tr( "Symbology" ) );
      mOptionsListWidget->addItem( symbolItem );
      QListWidgetItem *labelItem = new QListWidgetItem( QgsApplication::getThemeIcon( u"labelingSingle.svg"_s ), QString() );
      labelItem->setData( Qt::UserRole, QVariant::fromValue( Page::MeshLabeling ) );
      labelItem->setToolTip( tr( "Labels" ) );
      mOptionsListWidget->addItem( labelItem );

#ifdef HAVE_3D
      QListWidgetItem *symbol3DItem = new QListWidgetItem( QgsApplication::getThemeIcon( u"3d.svg"_s ), QString() );
      symbol3DItem->setData( Qt::UserRole, QVariant::fromValue( Page::Mesh3D ) );
      symbol3DItem->setToolTip( tr( "3D View" ) );
      mOptionsListWidget->addItem( symbol3DItem );
#endif
      break;
    }

    case Qgis::LayerType::VectorTile:
    {
      QListWidgetItem *symbolItem = new QListWidgetItem( QgsApplication::getThemeIcon( u"propertyicons/symbology.svg"_s ), QString() );
      symbolItem->setData( Qt::UserRole, QVariant::fromValue( Page::VectorTileRenderer ) );
      symbolItem->setToolTip( tr( "Symbology" ) );
      mOptionsListWidget->addItem( symbolItem );
      QListWidgetItem *labelItem = new QListWidgetItem( QgsApplication::getThemeIcon( u"labelingSingle.svg"_s ), QString() );
      labelItem->setData( Qt::UserRole, QVariant::fromValue( Page::VectorTileLabeling ) );
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
      item->setData( Qt::UserRole, QVariant::fromValue( Page::Custom ) );
      mOptionsListWidget->addItem( item );
      int row = mOptionsListWidget->row( item );
      mUserPages[row] = factory;
    }
  }
  QListWidgetItem *historyItem = new QListWidgetItem( QgsApplication::getThemeIcon( u"mActionHistory.svg"_s ), QString() );
  historyItem->setData( Qt::UserRole, QVariant::fromValue( Page::History ) );
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
  QDomDocument doc( u"style"_s );
  mLastStyleXml = doc.createElement( u"style"_s );
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

  QString undoName = u"Style Change"_s;

  QWidget *current = mWidgetStack->mainPanel();

  bool styleWasChanged = false;
  bool triggerRepaint = false; // whether the change needs the layer to be repainted
  if ( QgsMaskingWidget *widget = qobject_cast<QgsMaskingWidget *>( current ) )
  {
    widget->apply();
    styleWasChanged = true;
    undoName = u"Mask Change"_s;
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
        undoName = u"Style Change - %1"_s.arg( m->visibleName() );
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
    triggerRepaint = true;
    undoName = u"Label Change"_s;
  }
  else if ( QgsDiagramWidget *widget = qobject_cast<QgsDiagramWidget *>( current ) )
  {
    widget->apply();
    styleWasChanged = true;
    triggerRepaint = true;
    undoName = u"Diagram Change"_s;
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
    return;

  mBlockAutoApply = true;

  if ( mCurrentLayer )
    whileBlocking( mLayerCombo )->setLayer( mCurrentLayer );

  const int row = mOptionsListWidget->currentIndex().row();
  const Page rowPage = mOptionsListWidget->item( row )->data( Qt::UserRole ).value< Page >();

  // make sure we're not set to the "not supported" page
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
#ifdef HAVE_3D
    else if ( QgsMeshLayer3DRendererWidget *widget = qobject_cast<QgsMeshLayer3DRendererWidget *>( current ) )
    {
      mMesh3DWidget = widget;
    }
#endif
    else if ( QgsRasterAttributeTableWidget *widget = qobject_cast<QgsRasterAttributeTableWidget *>( current ) )
    {
      mRasterAttributeTableWidget = widget;
    }
    else
    {
      delete current;
      current = nullptr;
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
      connect( panel, &QgsPanelWidget::changed, this, &QgsLayerStylingWidget::autoApply );
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
        switch ( rowPage )
        {
          case Page::VectorRenderer:
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
          case Page::VectorLabeling:
          {
            if ( !mLabelingWidget )
            {
              mLabelingWidget = new QgsLabelingWidget( nullptr, mMapCanvas, mWidgetStack, mMessageBar );
              mLabelingWidget->setDockMode( true );
              connect( mLabelingWidget, &QgsLabelingWidget::changed, this, &QgsLayerStylingWidget::autoApply );
            }
            mLabelingWidget->setLayer( vlayer );
            mWidgetStack->setMainPanel( mLabelingWidget );
            break;
          }
          case Page::VectorMasks:
          {
            if ( !mMaskingWidget )
            {
              mMaskingWidget = new QgsMaskingWidget( mWidgetStack );
              mMaskingWidget->layout()->setContentsMargins( 0, 0, 0, 0 );
              connect( mMaskingWidget, &QgsMaskingWidget::changed, this, &QgsLayerStylingWidget::autoApply );
            }
            mMaskingWidget->setLayer( vlayer );
            mWidgetStack->setMainPanel( mMaskingWidget );
            break;
          }
#ifdef HAVE_3D
          case Page::Vector3D:
          {
            if ( !mVector3DWidget )
            {
              mVector3DWidget = new QgsVectorLayer3DRendererWidget( vlayer, mMapCanvas, mWidgetStack );
              mVector3DWidget->setDockMode( true );
              connect( mVector3DWidget, &QgsVectorLayer3DRendererWidget::changed, this, &QgsLayerStylingWidget::autoApply );
            }
            mVector3DWidget->syncToLayer( vlayer );
            mWidgetStack->setMainPanel( mVector3DWidget );
            break;
          }
#endif
          case Page::VectorDiagrams:
          {
            auto widget = new QgsDiagramWidget( vlayer, mMapCanvas, mWidgetStack );
            widget->setDockMode( true );
            connect( widget, &QgsDiagramWidget::changed, this, &QgsLayerStylingWidget::autoApply );
            widget->syncToOwnLayer();
            mWidgetStack->setMainPanel( widget );
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

        switch ( rowPage )
        {
          case Page::RasterRenderer:
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
            connect( mRasterStyleWidget, &QgsPanelWidget::changed, this, &QgsLayerStylingWidget::autoApply );
            mWidgetStack->setMainPanel( mRasterStyleWidget );
            break;
          }

          case Page::RasterTransparency:
          {
            QgsRasterTransparencyWidget *transwidget = new QgsRasterTransparencyWidget( rlayer, mMapCanvas, mWidgetStack );
            transwidget->setDockMode( true );

            QgsSymbolWidgetContext context;
            context.setMapCanvas( mMapCanvas );
            context.setMessageBar( mMessageBar );
            transwidget->setContext( context );

            connect( transwidget, &QgsPanelWidget::changed, this, &QgsLayerStylingWidget::autoApply );
            mWidgetStack->setMainPanel( transwidget );
            break;
          }

          case Page::RasterLabeling:
          {
            if ( !mRasterLabelingWidget )
            {
              mRasterLabelingWidget = new QgsRasterLabelingWidget( rlayer, mMapCanvas, mWidgetStack, mMessageBar );
              mRasterLabelingWidget->setDockMode( true );
              connect( mRasterLabelingWidget, &QgsPanelWidget::changed, this, &QgsLayerStylingWidget::autoApply );
            }
            else
            {
              mRasterLabelingWidget->setLayer( rlayer );
            }
            mWidgetStack->setMainPanel( mRasterLabelingWidget );
            break;
          }

          case Page::RasterHistogram:
          {
            QgsRasterDataProvider *provider = qobject_cast<QgsRasterDataProvider *>( rlayer->dataProvider() );
            if ( provider && ( provider->capabilities() & Qgis::RasterInterfaceCapability::Size ) )
            {
              if ( !mRasterStyleWidget )
              {
                mRasterStyleWidget = new QgsRendererRasterPropertiesWidget( rlayer, mMapCanvas, mWidgetStack );
                mRasterStyleWidget->syncToLayer( rlayer );
              }
              connect( mRasterStyleWidget, &QgsPanelWidget::changed, this, &QgsLayerStylingWidget::autoApply );

              QgsRasterHistogramWidget *widget = new QgsRasterHistogramWidget( rlayer, mWidgetStack );
              connect( widget, &QgsPanelWidget::changed, this, &QgsLayerStylingWidget::autoApply );
              QString name = mRasterStyleWidget->currentRenderWidget()->renderer()->type();
              widget->setRendererWidget( name, mRasterStyleWidget->currentRenderWidget() );
              widget->setDockMode( true );

              mWidgetStack->setMainPanel( widget );
            }
            break;
          }

          case Page::RasterAttributeTable:
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
              QgsPanelWidget *widget = new QgsPanelWidget { mWidgetStack };
              QVBoxLayout *layout = new QVBoxLayout { widget };
              widget->setLayout( layout );
              QLabel *label { new QLabel( tr(
                "There are no raster attribute tables associated with this data source.<br>"
                "If the current symbology can be converted to an attribute table you "
                "can create a new attribute table using the context menu available in the "
                "layer tree or in the layer properties dialog."
              ) ) };
              label->setWordWrap( true );
              widget->layout()->addWidget( label );
              layout->addStretch();
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

      case Qgis::LayerType::Mesh:
      {
        QgsMeshLayer *meshLayer = qobject_cast<QgsMeshLayer *>( mCurrentLayer );
        switch ( rowPage )
        {
          case Page::MeshRenderer:
          {
            auto widget = new QgsRendererMeshPropertiesWidget( meshLayer, mMapCanvas, mWidgetStack );

            widget->setDockMode( true );
            connect( widget, &QgsPanelWidget::changed, this, &QgsLayerStylingWidget::autoApply );
            mWidgetStack->setMainPanel( widget );

            connect( meshLayer, &QgsMeshLayer::reloaded, widget, [this, widget] { widget->syncToLayer( mCurrentLayer ); } );
            break;
          }
          case Page::MeshLabeling:
          {
            auto widget = new QgsMeshLabelingWidget( meshLayer, mMapCanvas, mWidgetStack, mMessageBar );
            widget->setDockMode( true );
            connect( widget, &QgsPanelWidget::changed, this, &QgsLayerStylingWidget::autoApply );
            mWidgetStack->setMainPanel( widget );
            break;
          }
#ifdef HAVE_3D
          case Page::Mesh3D:
          {
            if ( !mMesh3DWidget )
            {
              mMesh3DWidget = new QgsMeshLayer3DRendererWidget( nullptr, mMapCanvas, mWidgetStack );
              mMesh3DWidget->setDockMode( true );
              connect( mMesh3DWidget, &QgsMeshLayer3DRendererWidget::changed, this, &QgsLayerStylingWidget::autoApply );
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
        switch ( rowPage )
        {
          case Page::VectorTileRenderer:
          {
            auto widget = new QgsVectorTileBasicRendererWidget( vtLayer, mMapCanvas, mMessageBar, mWidgetStack );
            widget->setDockMode( true );
            connect( widget, &QgsPanelWidget::changed, this, &QgsLayerStylingWidget::autoApply );
            mWidgetStack->setMainPanel( widget );
            break;
          }
          case Page::VectorTileLabeling:
          {
            auto widget = new QgsVectorTileBasicLabelingWidget( vtLayer, mMapCanvas, mMessageBar, mWidgetStack );
            widget->setDockMode( true );
            connect( widget, &QgsPanelWidget::changed, this, &QgsLayerStylingWidget::autoApply );
            mWidgetStack->setMainPanel( widget );
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

void QgsLayerStylingWidget::setCurrentPage( Page page )
{
  for ( int i = 0; i < mOptionsListWidget->count(); ++i )
  {
    const Page thisPage = mOptionsListWidget->item( i )->data( Qt::UserRole ).value< Page >();
    if ( thisPage == page )
    {
      mOptionsListWidget->setCurrentRow( i );
      return;
    }
  }
}

void QgsLayerStylingWidget::setAnnotationItem( QgsAnnotationLayer *layer, const QString &itemId, bool multipleItems )
{
  const bool matchingPreviousItem = layer == mCurrentLayer && mContext.annotationId() == itemId;
  if ( !matchingPreviousItem )
  {
    mContext.setAnnotationId( itemId );
    if ( layer )
    {
      setLayer( layer );
    }
  }

  if ( layer )
  {
    mStackedWidget->setCurrentIndex( mLayerPage );
  }

  if ( QgsAnnotationItemPropertiesWidget *configWidget = qobject_cast<QgsAnnotationItemPropertiesWidget *>( mWidgetStack->mainPanel() ) )
  {
    if ( !matchingPreviousItem )
    {
      mWidgetStack->acceptAllPanels();
      configWidget->setMapLayerConfigWidgetContext( mContext );
    }

    if ( itemId.isEmpty() )
    {
      configWidget->setLabelMessage( multipleItems ? tr( "Multiple items selected." ) : tr( "No item selected." ) );
    }
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
      item->setData( Qt::UserRole, QVariant::fromValue( Page::Custom ) );
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
  settings.setValue( u"UI/autoApplyStyling"_s, liveUpdateEnabled );

  mButtonBox->button( QDialogButtonBox::Apply )->setEnabled( !liveUpdateEnabled );
}

void QgsLayerStylingWidget::pushUndoItem( const QString &name, bool triggerRepaint )
{
  QString errorMsg;
  QDomDocument doc( u"style"_s );
  QDomElement rootNode = doc.createElement( u"qgis"_s );
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


const QgsSettingsEntryInteger *QgsMapLayerStyleCommand::settingsStyleUndoMergeTimeout
  = new QgsSettingsEntryInteger( u"style-undo-merge-timeout"_s, QgsSettingsTree::sTreeGui, 500, u"Timeout in milliseconds for merging successive style undo commands"_s );


QgsMapLayerStyleCommand::QgsMapLayerStyleCommand( QgsMapLayer *layer, const QString &text, const QDomNode &current, const QDomNode &last, bool triggerRepaint )
  : QUndoCommand( text )
  , mLayer( layer )
  , mXml( current )
  , mLastState( last )
  , mTime( QTime::currentTime() )
  , mTriggerRepaint( triggerRepaint )
{}

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
  int timeout = settingsStyleUndoMergeTimeout->value();
  if ( mTime.msecsTo( otherCmd->mTime ) > timeout )
    return false;

  mXml = otherCmd->mXml;
  mTime = otherCmd->mTime;
  mTriggerRepaint |= otherCmd->mTriggerRepaint;
  return true;
}

QgsLayerStyleManagerWidgetFactory::QgsLayerStyleManagerWidgetFactory()
{
  setIcon( QgsApplication::getThemeIcon( u"propertyicons/stylepreset.svg"_s ) );
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
    case Qgis::LayerType::PointCloud:
    case Qgis::LayerType::TiledScene:
      return true;

    case Qgis::LayerType::Plugin:
    case Qgis::LayerType::Annotation:
    case Qgis::LayerType::Group:
      return false;
  }
  return false; // no warnings
}
