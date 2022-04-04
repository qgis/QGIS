/***************************************************************************
    qgsdualview.cpp
     --------------------------------------
    Date                 : 10.2.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QClipboard>
#include <QDialog>
#include <QMenu>
#include <QMessageBox>
#include <QProgressDialog>
#include <QGroupBox>
#include <QInputDialog>
#include <QTimer>
#include <QShortcut>

#include "qgsapplication.h"
#include "qgsactionmanager.h"
#include "qgsattributetablemodel.h"
#include "qgsdualview.h"
#include "qgsexpressionbuilderdialog.h"
#include "qgsfeaturelistmodel.h"
#include "qgsifeatureselectionmanager.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayeractionregistry.h"
#include "qgsmessagelog.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayercache.h"
#include "qgsorganizetablecolumnsdialog.h"
#include "qgseditorwidgetregistry.h"
#include "qgssettings.h"
#include "qgsscrollarea.h"
#include "qgsgui.h"
#include "qgsexpressioncontextutils.h"
#include "qgsshortcutsmanager.h"
#include "qgsfieldconditionalformatwidget.h"
#include "qgsmapcanvasutils.h"
#include "qgsmessagebar.h"
#include "qgsvectorlayereditbuffer.h"


QgsDualView::QgsDualView( QWidget *parent )
  : QStackedWidget( parent )
{
  setupUi( this );
  connect( mFeatureListView, &QgsFeatureListView::aboutToChangeEditSelection, this, &QgsDualView::featureListAboutToChangeEditSelection );
  connect( mFeatureListView, &QgsFeatureListView::currentEditSelectionChanged, this, &QgsDualView::featureListCurrentEditSelectionChanged );
  connect( mFeatureListView, &QgsFeatureListView::currentEditSelectionProgressChanged, this, &QgsDualView::updateEditSelectionProgress );
  connect( mFeatureListView, &QgsFeatureListView::willShowContextMenu, this, &QgsDualView::widgetWillShowContextMenu );

  connect( mTableView, &QgsAttributeTableView::willShowContextMenu, this, &QgsDualView::viewWillShowContextMenu );
  mTableView->horizontalHeader()->setContextMenuPolicy( Qt::CustomContextMenu );
  connect( mTableView->horizontalHeader(), &QHeaderView::customContextMenuRequested, this, &QgsDualView::showViewHeaderMenu );
  connect( mTableView, &QgsAttributeTableView::columnResized, this, &QgsDualView::tableColumnResized );

  mConditionalFormatWidgetStack->hide();
  mConditionalFormatWidget = new QgsFieldConditionalFormatWidget( this );
  mConditionalFormatWidgetStack->setMainPanel( mConditionalFormatWidget );
  mConditionalFormatWidget->setDockMode( true );

  const QgsSettings settings;
  mConditionalSplitter->restoreState( settings.value( QStringLiteral( "/qgis/attributeTable/splitterState" ), QByteArray() ).toByteArray() );

  mPreviewColumnsMenu = new QMenu( this );
  mActionPreviewColumnsMenu->setMenu( mPreviewColumnsMenu );

  // Set preview icon
  mActionExpressionPreview->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mIconExpressionPreview.svg" ) ) );

  // Connect layer list preview signals
  connect( mActionExpressionPreview, &QAction::triggered, this, &QgsDualView::previewExpressionBuilder );
  connect( mFeatureListView, &QgsFeatureListView::displayExpressionChanged, this, &QgsDualView::previewExpressionChanged );

  // browsing toolbar
  connect( mNextFeatureButton, &QToolButton::clicked, mFeatureListView, &QgsFeatureListView::editNextFeature );
  connect( mPreviousFeatureButton, &QToolButton::clicked, mFeatureListView, &QgsFeatureListView::editPreviousFeature );
  connect( mFirstFeatureButton, &QToolButton::clicked, mFeatureListView, &QgsFeatureListView::editFirstFeature );
  connect( mLastFeatureButton, &QToolButton::clicked, mFeatureListView, &QgsFeatureListView::editLastFeature );

  auto createShortcuts = [ = ]( const QString & objectName, void ( QgsFeatureListView::* slot )() )
  {
    QShortcut *sc = QgsGui::shortcutsManager()->shortcutByName( objectName );
    // do not assert for sc as it would lead to crashes in testing
    // or when using custom widgets lib if built with Debug
    if ( sc )
      connect( sc, &QShortcut::activated, mFeatureListView, slot );
  };
  createShortcuts( QStringLiteral( "mAttributeTableFirstEditedFeature" ), &QgsFeatureListView::editFirstFeature );
  createShortcuts( QStringLiteral( "mAttributeTablePreviousEditedFeature" ), &QgsFeatureListView::editPreviousFeature );
  createShortcuts( QStringLiteral( "mAttributeTableNextEditedFeature" ), &QgsFeatureListView::editNextFeature );
  createShortcuts( QStringLiteral( "mAttributeTableLastEditedFeature" ), &QgsFeatureListView::editLastFeature );

  QButtonGroup *buttonGroup = new QButtonGroup( this );
  buttonGroup->setExclusive( false );
  buttonGroup->addButton( mAutoPanButton, PanToFeature );
  buttonGroup->addButton( mAutoZoomButton, ZoomToFeature );
  const FeatureListBrowsingAction action = QgsSettings().enumValue( QStringLiteral( "/qgis/attributeTable/featureListBrowsingAction" ), NoAction );
  QAbstractButton *bt = buttonGroup->button( static_cast<int>( action ) );
  if ( bt )
    bt->setChecked( true );
  connect( buttonGroup, qOverload< QAbstractButton *, bool >( &QButtonGroup::buttonToggled ), this, &QgsDualView::panZoomGroupButtonToggled );
  mFlashButton->setChecked( QgsSettings().value( QStringLiteral( "/qgis/attributeTable/featureListHighlightFeature" ), true ).toBool() );
  connect( mFlashButton, &QToolButton::clicked, this, &QgsDualView::flashButtonClicked );
}

QgsDualView::~QgsDualView()
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "/qgis/attributeTable/splitterState" ), mConditionalSplitter->saveState() );
}

void QgsDualView::init( QgsVectorLayer *layer, QgsMapCanvas *mapCanvas, const QgsFeatureRequest &request,
                        const QgsAttributeEditorContext &context, bool loadFeatures, bool showFirstFeature )
{
  if ( !layer )
    return;

  delete mAttributeForm;
  mAttributeForm = nullptr;

  mLayer = layer;

  // Keep fields order in sync: force config reset
  connect( mLayer, &QgsVectorLayer::updatedFields, this, [ = ]
  {
    mFilterModel->setAttributeTableConfig( attributeTableConfig(), /* force */ true );
  } );

  mEditorContext = context;

  // create an empty form to find out if it needs geometry or not
  const QgsAttributeForm emptyForm( mLayer, QgsFeature(), mEditorContext );

  const bool needsGeometry = !( request.flags() & QgsFeatureRequest::NoGeometry )
                             || ( request.spatialFilterType() != Qgis::SpatialFilterType::NoFilter )
                             || emptyForm.needsGeometry();

  initLayerCache( needsGeometry );
  initModels( mapCanvas, request, loadFeatures );

  mConditionalFormatWidget->setLayer( mLayer );

  mTableView->setModel( mFilterModel );
  mFeatureListView->setModel( mFeatureListModel );

  connect( mFilterModel, &QgsAttributeTableFilterModel::sortColumnChanged, this, &QgsDualView::onSortColumnChanged );

  if ( mFeatureListPreviewButton->defaultAction() )
    mFeatureListView->setDisplayExpression( mDisplayExpression );
  else
    columnBoxInit();

  // This slows down load of the attribute table heaps and uses loads of memory.
  //mTableView->resizeColumnsToContents();

  if ( showFirstFeature && mFeatureListModel->rowCount( ) > 0 )
    mFeatureListView->setEditSelection( QgsFeatureIds() << mFeatureListModel->data( mFeatureListModel->index( 0, 0 ), QgsFeatureListModel::Role::FeatureRole ).value<QgsFeature>().id() );
}

void QgsDualView::initAttributeForm( const QgsFeature &feature )
{
  Q_ASSERT( !mAttributeForm );

  mAttributeForm = new QgsAttributeForm( mLayer, feature, mEditorContext );
  if ( !mEditorContext.parentContext() )
  {
    mAttributeEditorScrollArea = new QgsScrollArea();
    mAttributeEditorScrollArea->setWidgetResizable( true );
    mAttributeEditor->layout()->addWidget( mAttributeEditorScrollArea );
    mAttributeEditorScrollArea->setWidget( mAttributeForm );
  }
  else
  {
    mAttributeEditor->layout()->addWidget( mAttributeForm );
  }

  setAttributeTableConfig( mLayer->attributeTableConfig() );

  connect( mAttributeForm, &QgsAttributeForm::widgetValueChanged, this, &QgsDualView::featureFormAttributeChanged );
  connect( mAttributeForm, &QgsAttributeForm::modeChanged, this, &QgsDualView::formModeChanged );
  connect( mAttributeForm, &QgsAttributeForm::filterExpressionSet, this, &QgsDualView::filterExpressionSet );
  connect( mAttributeForm, &QgsAttributeForm::flashFeatures, this, [ = ]( const QString & filter )
  {
    if ( QgsMapCanvas *canvas = mFilterModel->mapCanvas() )
    {
      QgsMapCanvasUtils::flashMatchingFeatures( canvas, mLayer, filter );
    }
  } );
  connect( mAttributeForm, &QgsAttributeForm::zoomToFeatures, this, [ = ]( const QString & filter )
  {
    if ( QgsMapCanvas *canvas = mFilterModel->mapCanvas() )
    {
      QgsMapCanvasUtils::zoomToMatchingFeatures( canvas, mLayer, filter );
    }
  } );

  connect( mMasterModel, &QgsAttributeTableModel::modelChanged, mAttributeForm, &QgsAttributeForm::refreshFeature );
}

void QgsDualView::columnBoxInit()
{
  // load fields
  const QList<QgsField> fields = mLayer->fields().toList();

  const QString defaultField;

  mFeatureListPreviewButton->addAction( mActionExpressionPreview );
  mFeatureListPreviewButton->addAction( mActionPreviewColumnsMenu );

  const auto constFields = fields;
  for ( const QgsField &field : constFields )
  {
    const int fieldIndex = mLayer->fields().lookupField( field.name() );
    if ( fieldIndex == -1 )
      continue;

    const QString fieldName = field.name();
    if ( QgsGui::editorWidgetRegistry()->findBest( mLayer, fieldName ).type() != QLatin1String( "Hidden" ) )
    {
      const QIcon icon = mLayer->fields().iconForField( fieldIndex );
      const QString text = mLayer->attributeDisplayName( fieldIndex );

      // Generate action for the preview popup button of the feature list
      QAction *previewAction = new QAction( icon, text, mFeatureListPreviewButton );
      connect( previewAction, &QAction::triggered, this, [ = ] { previewColumnChanged( previewAction, fieldName ); } );
      mPreviewColumnsMenu->addAction( previewAction );

      if ( text == defaultField )
      {
        mFeatureListPreviewButton->setDefaultAction( previewAction );
      }
    }
  }

  QMenu *sortMenu = new QMenu( this );
  QAction *sortMenuAction = new QAction( QgsApplication::getThemeIcon( QStringLiteral( "sort.svg" ) ), tr( "Sort…" ), this );
  sortMenuAction->setMenu( sortMenu );

  QAction *sortByPreviewExpressionAsc = new QAction( QgsApplication::getThemeIcon( QStringLiteral( "sort.svg" ) ), tr( "By Preview Expression (ascending)" ), this );
  connect( sortByPreviewExpressionAsc, &QAction::triggered, this, [ = ]()
  {
    mFeatureListModel->setSortByDisplayExpression( true, Qt::AscendingOrder );
  } );
  sortMenu->addAction( sortByPreviewExpressionAsc );
  QAction *sortByPreviewExpressionDesc = new QAction( QgsApplication::getThemeIcon( QStringLiteral( "sort-reverse.svg" ) ), tr( "By Preview Expression (descending)" ), this );
  connect( sortByPreviewExpressionDesc, &QAction::triggered, this, [ = ]()
  {
    mFeatureListModel->setSortByDisplayExpression( true, Qt::DescendingOrder );
  } );
  sortMenu->addAction( sortByPreviewExpressionDesc );
  QAction *sortByPreviewExpressionCustom = new QAction( QgsApplication::getThemeIcon( QStringLiteral( "mIconExpressionPreview.svg" ) ), tr( "By Custom Expression" ), this );
  connect( sortByPreviewExpressionCustom, &QAction::triggered, this, [ = ]()
  {
    if ( modifySort() )
      mFeatureListModel->setSortByDisplayExpression( false );
  } );
  sortMenu->addAction( sortByPreviewExpressionCustom );

  mFeatureListPreviewButton->addAction( sortMenuAction );

  QAction *separator = new QAction( mFeatureListPreviewButton );
  separator->setSeparator( true );
  mFeatureListPreviewButton->addAction( separator );
  restoreRecentDisplayExpressions();

  // If there is no single field found as preview
  if ( !mFeatureListPreviewButton->defaultAction() )
  {
    mFeatureListView->setDisplayExpression( mLayer->displayExpression() );
    mFeatureListPreviewButton->setDefaultAction( mActionExpressionPreview );
    const QString displayExpression = mFeatureListView->displayExpression();
    setDisplayExpression( displayExpression.isEmpty() ? tr( "'[Please define preview text]'" ) : displayExpression );
  }
  else
  {
    mFeatureListPreviewButton->defaultAction()->trigger();
  }
}

void QgsDualView::setView( QgsDualView::ViewMode view )
{
  setCurrentIndex( view );
}

QgsDualView::ViewMode QgsDualView::view() const
{
  return static_cast< QgsDualView::ViewMode >( currentIndex() );
}

void QgsDualView::setFilterMode( QgsAttributeTableFilterModel::FilterMode filterMode )
{
  // cleanup any existing connections
  switch ( mFilterModel->filterMode() )
  {
    case QgsAttributeTableFilterModel::ShowVisible:
      disconnect( mFilterModel->mapCanvas(), &QgsMapCanvas::extentsChanged, this, &QgsDualView::extentChanged );
      disconnect( mFilterModel, &QgsAttributeTableFilterModel::visibleReloaded, this, &QgsDualView::filterChanged );
      break;

    case QgsAttributeTableFilterModel::ShowAll:
    case QgsAttributeTableFilterModel::ShowFilteredList:
      disconnect( mFilterModel, &QgsAttributeTableFilterModel::featuresFiltered, this, &QgsDualView::filterChanged );
      disconnect( mFilterModel, &QgsAttributeTableFilterModel::filterError, this, &QgsDualView::filterError );
      break;

    case QgsAttributeTableFilterModel::ShowEdited:
      disconnect( mFilterModel, &QgsAttributeTableFilterModel::featuresFiltered, this, &QgsDualView::filterChanged );
      disconnect( mFilterModel, &QgsAttributeTableFilterModel::filterError, this, &QgsDualView::filterError );
      disconnect( masterModel()->layer(), &QgsVectorLayer::layerModified, this, &QgsDualView::updateEditedAddedFeatures );
      break;

    case QgsAttributeTableFilterModel::ShowSelected:
      disconnect( masterModel()->layer(), &QgsVectorLayer::selectionChanged, this, &QgsDualView::updateSelectedFeatures );
      break;
  }

  QgsFeatureRequest r = mMasterModel->request();
  const bool needsGeometry = filterMode == QgsAttributeTableFilterModel::ShowVisible;

  const bool requiresTableReload = ( r.filterType() != QgsFeatureRequest::FilterNone || r.spatialFilterType() != Qgis::SpatialFilterType::NoFilter ) // previous request was subset
                                   || ( needsGeometry && r.flags() & QgsFeatureRequest::NoGeometry ) // no geometry for last request
                                   || ( mMasterModel->rowCount() == 0 ); // no features

  if ( !needsGeometry )
    r.setFlags( r.flags() | QgsFeatureRequest::NoGeometry );
  else
    r.setFlags( r.flags() & ~( QgsFeatureRequest::NoGeometry ) );
  r.setFilterFids( QgsFeatureIds() );
  r.setFilterRect( QgsRectangle() );
  r.disableFilter();

  // setup new connections and filter request parameters
  switch ( filterMode )
  {
    case QgsAttributeTableFilterModel::ShowVisible:
      connect( mFilterModel->mapCanvas(), &QgsMapCanvas::extentsChanged, this, &QgsDualView::extentChanged );
      if ( mFilterModel->mapCanvas() )
      {
        const QgsRectangle rect = mFilterModel->mapCanvas()->mapSettings().mapToLayerCoordinates( mLayer, mFilterModel->mapCanvas()->extent() );
        r.setFilterRect( rect );
      }
      connect( mFilterModel, &QgsAttributeTableFilterModel::visibleReloaded, this, &QgsDualView::filterChanged );
      break;

    case QgsAttributeTableFilterModel::ShowEdited:
      r.setFilterFids( masterModel()->layer()->editBuffer() ? masterModel()->layer()->editBuffer()->allAddedOrEditedFeatures() : QgsFeatureIds() );
      connect( mFilterModel, &QgsAttributeTableFilterModel::featuresFiltered, this, &QgsDualView::filterChanged );
      connect( mFilterModel, &QgsAttributeTableFilterModel::filterError, this, &QgsDualView::filterError );
      connect( masterModel()->layer(), &QgsVectorLayer::layerModified, this, &QgsDualView::updateEditedAddedFeatures );
      break;

    case QgsAttributeTableFilterModel::ShowAll:
    case QgsAttributeTableFilterModel::ShowFilteredList:
      connect( mFilterModel, &QgsAttributeTableFilterModel::featuresFiltered, this, &QgsDualView::filterChanged );
      connect( mFilterModel, &QgsAttributeTableFilterModel::filterError, this, &QgsDualView::filterError );
      break;

    case QgsAttributeTableFilterModel::ShowSelected:
      connect( masterModel()->layer(), &QgsVectorLayer::selectionChanged, this, &QgsDualView::updateSelectedFeatures );
      r.setFilterFids( masterModel()->layer()->selectedFeatureIds() );
      break;
  }

  // disable the browsing auto pan/scale if the list only shows visible items
  switch ( filterMode )
  {
    case QgsAttributeTableFilterModel::ShowVisible:
      setBrowsingAutoPanScaleAllowed( false );
      break;

    case QgsAttributeTableFilterModel::ShowAll:
    case QgsAttributeTableFilterModel::ShowEdited:
    case QgsAttributeTableFilterModel::ShowFilteredList:
    case QgsAttributeTableFilterModel::ShowSelected:
      setBrowsingAutoPanScaleAllowed( true );
      break;
  }

  if ( requiresTableReload )
  {
    //disconnect the connections of the current (old) filtermode before reload
    mFilterModel->disconnectFilterModeConnections();

    mMasterModel->setRequest( r );
    whileBlocking( mLayerCache )->setCacheGeometry( needsGeometry );
    mMasterModel->loadLayer();
  }

  //update filter model
  mFilterModel->setFilterMode( filterMode );
  emit filterChanged();
}

void QgsDualView::setSelectedOnTop( bool selectedOnTop )
{
  mFilterModel->setSelectedOnTop( selectedOnTop );
}

void QgsDualView::initLayerCache( bool cacheGeometry )
{
  // Initialize the cache
  const QgsSettings settings;
  const int cacheSize = settings.value( QStringLiteral( "qgis/attributeTableRowCache" ), "10000" ).toInt();
  mLayerCache = new QgsVectorLayerCache( mLayer, cacheSize, this );
  mLayerCache->setCacheGeometry( cacheGeometry );
  if ( 0 == cacheSize || 0 == ( QgsVectorDataProvider::SelectAtId & mLayer->dataProvider()->capabilities() ) )
  {
    connect( mLayerCache, &QgsVectorLayerCache::invalidated, this, &QgsDualView::rebuildFullLayerCache );
    rebuildFullLayerCache();
  }
}

void QgsDualView::initModels( QgsMapCanvas *mapCanvas, const QgsFeatureRequest &request, bool loadFeatures )
{
  delete mFeatureListModel;
  delete mFilterModel;
  delete mMasterModel;

  mMasterModel = new QgsAttributeTableModel( mLayerCache, this );
  mMasterModel->setRequest( request );
  mMasterModel->setEditorContext( mEditorContext );
  mMasterModel->setExtraColumns( 1 ); // Add one extra column which we can "abuse" as an action column

  connect( mMasterModel, &QgsAttributeTableModel::progress, this, &QgsDualView::progress );
  connect( mMasterModel, &QgsAttributeTableModel::finished, this, &QgsDualView::finished );

  connect( mConditionalFormatWidget, &QgsFieldConditionalFormatWidget::rulesUpdated, mMasterModel, &QgsAttributeTableModel::fieldConditionalStyleChanged );

  if ( loadFeatures )
    mMasterModel->loadLayer();

  mFilterModel = new QgsAttributeTableFilterModel( mapCanvas, mMasterModel, mMasterModel );

  // The following connections to invalidate() are necessary to keep the filter model in sync
  // see regression https://github.com/qgis/QGIS/issues/23890
  connect( mMasterModel, &QgsAttributeTableModel::rowsRemoved, mFilterModel, &QgsAttributeTableFilterModel::invalidate );
  connect( mMasterModel, &QgsAttributeTableModel::rowsInserted, mFilterModel, &QgsAttributeTableFilterModel::invalidate );

  connect( mFeatureListView, &QgsFeatureListView::displayExpressionChanged, this, &QgsDualView::displayExpressionChanged );

  mFeatureListModel = new QgsFeatureListModel( mFilterModel, mFilterModel );
  mFeatureListModel->setSortByDisplayExpression( true );
}

void QgsDualView::restoreRecentDisplayExpressions()
{
  const QVariantList previewExpressions = mLayer->customProperty( QStringLiteral( "dualview/previewExpressions" ) ).toList();

  for ( const QVariant &previewExpression : previewExpressions )
    insertRecentlyUsedDisplayExpression( previewExpression.toString() );
}

void QgsDualView::saveRecentDisplayExpressions() const
{
  if ( ! mLayer )
  {
    return;
  }
  const QList<QAction *> actions = mFeatureListPreviewButton->actions();

  // Remove existing same action
  int index = actions.indexOf( mLastDisplayExpressionAction );
  if ( index != -1 )
  {
    QVariantList previewExpressions;
    for ( ; index < actions.length(); ++index )
    {
      QAction *action = actions.at( index );
      previewExpressions << action->property( "previewExpression" );
    }

    mLayer->setCustomProperty( QStringLiteral( "dualview/previewExpressions" ), previewExpressions );
  }
}

void QgsDualView::setDisplayExpression( const QString &expression )
{
  mDisplayExpression = expression;
  insertRecentlyUsedDisplayExpression( expression );
}

void QgsDualView::insertRecentlyUsedDisplayExpression( const QString &expression )
{
  const QList<QAction *> actions = mFeatureListPreviewButton->actions();

  // Remove existing same action
  const int index = actions.indexOf( mLastDisplayExpressionAction );
  if ( index != -1 )
  {
    for ( int i = 0; index + i < actions.length(); ++i )
    {
      QAction *action = actions.at( index );
      if ( action->text() == expression || i >= 9 )
      {
        if ( action == mLastDisplayExpressionAction )
          mLastDisplayExpressionAction = nullptr;
        mFeatureListPreviewButton->removeAction( action );
      }
      else
      {
        if ( !mLastDisplayExpressionAction )
          mLastDisplayExpressionAction = action;
      }
    }
  }

  QString name = expression;
  QIcon icon = QgsApplication::getThemeIcon( QStringLiteral( "/mIconExpressionPreview.svg" ) );
  if ( expression.startsWith( QLatin1String( "COALESCE( \"" ) ) && expression.endsWith( QLatin1String( ", '<NULL>' )" ) ) )
  {
    name = expression.mid( 11, expression.length() - 24 ); // Numbers calculated from the COALESCE / <NULL> parts

    const int fieldIndex = mLayer->fields().indexOf( name );
    if ( fieldIndex != -1 )
    {
      name = mLayer->attributeDisplayName( fieldIndex );
      icon = mLayer->fields().iconForField( fieldIndex );
    }
    else
    {
      name = expression;
    }
  }

  QAction *previewAction = new QAction( icon, name, mFeatureListPreviewButton );
  previewAction->setProperty( "previewExpression", expression );
  connect( previewAction, &QAction::triggered, this, [expression, this]( bool )
  {
    setDisplayExpression( expression );
    mFeatureListPreviewButton->setText( expression );
  }
         );

  mFeatureListPreviewButton->insertAction( mLastDisplayExpressionAction, previewAction );
  mLastDisplayExpressionAction = previewAction;
}

void QgsDualView::updateEditSelectionProgress( int progress, int count )
{
  mProgressCount->setText( QStringLiteral( "%1 / %2" ).arg( progress + 1 ).arg( count ) );
  mPreviousFeatureButton->setEnabled( progress > 0 );
  mNextFeatureButton->setEnabled( progress + 1 < count );
  mFirstFeatureButton->setEnabled( progress > 0 );
  mLastFeatureButton->setEnabled( progress + 1 < count );
  if ( mAttributeForm )
  {
    mAttributeForm->setVisible( count > 0 );
  }
}

void QgsDualView::panOrZoomToFeature( const QgsFeatureIds &featureset )
{
  QgsMapCanvas *canvas = mFilterModel->mapCanvas();
  if ( canvas && view() == AttributeEditor && featureset != mLastFeatureSet )
  {
    if ( mBrowsingAutoPanScaleAllowed )
    {
      if ( mAutoPanButton->isChecked() )
        QTimer::singleShot( 0, this, [ = ]()
      {
        canvas->panToFeatureIds( mLayer, featureset, false );
      } );
      else if ( mAutoZoomButton->isChecked() )
        QTimer::singleShot( 0, this, [ = ]()
      {
        canvas->zoomToFeatureIds( mLayer, featureset );
      } );
    }
    if ( mFlashButton->isChecked() )
      QTimer::singleShot( 0, this, [ = ]()
    {
      canvas->flashFeatureIds( mLayer, featureset );
    } );
    mLastFeatureSet = featureset;
  }
}

void QgsDualView::setBrowsingAutoPanScaleAllowed( bool allowed )
{
  if ( mBrowsingAutoPanScaleAllowed == allowed )
    return;

  mBrowsingAutoPanScaleAllowed = allowed;

  mAutoPanButton->setEnabled( allowed );
  mAutoZoomButton->setEnabled( allowed );

  const QString disabledHint = tr( "(disabled when attribute table only shows features visible in the current map canvas extent)" );

  mAutoPanButton->setToolTip( tr( "Automatically pan to the current feature" ) + ( allowed ? QString() : QString( ' ' ) + disabledHint ) );
  mAutoZoomButton->setToolTip( tr( "Automatically zoom to the current feature" ) + ( allowed ? QString() : QString( ' ' ) + disabledHint ) );
}

void QgsDualView::panZoomGroupButtonToggled( QAbstractButton *button, bool checked )
{
  if ( button == mAutoPanButton && checked )
  {
    QgsSettings().setEnumValue( QStringLiteral( "/qgis/attributeTable/featureListBrowsingAction" ), PanToFeature );
    mAutoZoomButton->setChecked( false );
  }
  else if ( button == mAutoZoomButton && checked )
  {
    QgsSettings().setEnumValue( QStringLiteral( "/qgis/attributeTable/featureListBrowsingAction" ), ZoomToFeature );
    mAutoPanButton->setChecked( false );
  }
  else
  {
    QgsSettings().setEnumValue( QStringLiteral( "/qgis/attributeTable/featureListBrowsingAction" ), NoAction );
  }

  if ( checked && mLayer->isSpatial() )
    panOrZoomToFeature( mFeatureListView->currentEditSelection() );
}

void QgsDualView::flashButtonClicked( bool clicked )
{
  QgsSettings().setValue( QStringLiteral( "/qgis/attributeTable/featureListHighlightFeature" ), clicked );
  if ( !clicked )
    return;

  QgsMapCanvas *canvas = mFilterModel->mapCanvas();

  if ( canvas )
    canvas->flashFeatureIds( mLayer, mFeatureListView->currentEditSelection() );
}

void QgsDualView::filterError( const QString &errorMessage )
{
  if ( mEditorContext.mainMessageBar() )
  {
    mEditorContext.mainMessageBar()->pushWarning( tr( "An error occurred while filtering features" ), errorMessage );
  }
}

void QgsDualView::featureListAboutToChangeEditSelection( bool &ok )
{
  if ( !mAttributeForm )
    return;

  if ( mLayer->isEditable() && !mAttributeForm->save() )
    ok = false;
}

void QgsDualView::featureListCurrentEditSelectionChanged( const QgsFeature &feat )
{
  if ( !mAttributeForm )
  {
    initAttributeForm( feat );
  }
  else if ( !mLayer->isEditable() || mAttributeForm->save() )
  {
    mAttributeForm->setFeature( feat );
    QgsFeatureIds featureset;
    featureset << feat.id();
    setCurrentEditSelection( featureset );

    if ( mLayer->isSpatial() )
      panOrZoomToFeature( featureset );

  }
  else
  {
    // Couldn't save feature
  }
}

void QgsDualView::setCurrentEditSelection( const QgsFeatureIds &fids )
{
  mFeatureListView->setCurrentFeatureEdited( false );
  mFeatureListView->setEditSelection( fids );
}

bool QgsDualView::saveEditChanges()
{
  return mAttributeForm ? mAttributeForm->save() : false;
}

void QgsDualView::openConditionalStyles()
{
  mConditionalFormatWidgetStack->setVisible( !mConditionalFormatWidgetStack->isVisible() );
}

void QgsDualView::setMultiEditEnabled( bool enabled )
{
  if ( !mAttributeForm )
    return;

  if ( enabled )
  {
    mPreviousView = view();
    setView( AttributeEditor );
  }
  else
  {
    setView( mPreviousView );
  }

  mAttributeForm->setMode( enabled ? QgsAttributeEditorContext::MultiEditMode : QgsAttributeEditorContext::SingleEditMode );
}

void QgsDualView::toggleSearchMode( bool enabled )
{
  if ( !mAttributeForm )
    return;

  if ( enabled )
  {
    setView( AttributeEditor );
    mAttributeForm->setMode( QgsAttributeEditorContext::SearchMode );
    mAttributeForm->setVisible( true );
  }
  else
  {
    mAttributeForm->setMode( QgsAttributeEditorContext::SingleEditMode );
    mAttributeForm->setVisible( mFilterModel->rowCount( ) > 0 );
  }

}

void QgsDualView::previewExpressionBuilder()
{
  // Show expression builder
  const QgsExpressionContext context( QgsExpressionContextUtils::globalProjectLayerScopes( mLayer ) );

  QgsExpressionBuilderDialog dlg( mLayer, mFeatureListView->displayExpression(), this, QStringLiteral( "generic" ), context );
  dlg.setWindowTitle( tr( "Expression Based Preview" ) );
  dlg.setExpressionText( mFeatureListView->displayExpression() );

  if ( dlg.exec() == QDialog::Accepted )
  {
    mFeatureListView->setDisplayExpression( dlg.expressionText() );
    mFeatureListPreviewButton->setDefaultAction( mActionExpressionPreview );
    mFeatureListPreviewButton->setPopupMode( QToolButton::MenuButtonPopup );
  }

  setDisplayExpression( mFeatureListView->displayExpression() );
}

void QgsDualView::previewColumnChanged( QAction *previewAction, const QString &expression )
{
  if ( !mFeatureListView->setDisplayExpression( QStringLiteral( "COALESCE( \"%1\", '<NULL>' )" ).arg( expression ) ) )
  {
    QMessageBox::warning( this,
                          tr( "Column Preview" ),
                          tr( "Could not set column '%1' as preview column.\nParser error:\n%2" )
                          .arg( previewAction->text(), mFeatureListView->parserErrorString() )
                        );
  }
  else
  {
    mFeatureListPreviewButton->setText( previewAction->text() );
    mFeatureListPreviewButton->setIcon( previewAction->icon() );
    mFeatureListPreviewButton->setPopupMode( QToolButton::InstantPopup );
  }

  setDisplayExpression( mFeatureListView->displayExpression() );
}

int QgsDualView::featureCount()
{
  return mMasterModel->rowCount();
}

int QgsDualView::filteredFeatureCount()
{
  return mFilterModel->rowCount();
}

void QgsDualView::copyCellContent() const
{
  const QModelIndex currentIndex = mTableView->currentIndex();
  if ( !currentIndex.isValid() )
  {
    return;
  }

  const QVariant var = mMasterModel->data( currentIndex, Qt::DisplayRole );
  QApplication::clipboard()->setText( var.toString() );
}

void QgsDualView::cancelProgress()
{
  if ( mProgressDlg )
    mProgressDlg->cancel();
}

void QgsDualView::parentFormValueChanged( const QString &attribute, const QVariant &newValue )
{
  if ( mAttributeForm )
  {
    mAttributeForm->parentFormValueChanged( attribute, newValue );
  }
}

void QgsDualView::hideEvent( QHideEvent *event )
{
  Q_UNUSED( event )
  saveRecentDisplayExpressions();
}

void QgsDualView::viewWillShowContextMenu( QMenu *menu, const QModelIndex &masterIndex )
{
  if ( !menu )
  {
    return;
  }

  QAction *copyContentAction = menu->addAction( tr( "Copy Cell Content" ) );
  menu->addAction( copyContentAction );
  connect( copyContentAction, &QAction::triggered, this, [masterIndex, this]
  {
    const QVariant var = mMasterModel->data( masterIndex, Qt::DisplayRole );
    QApplication::clipboard()->setText( var.toString() );
  } );

  QgsVectorLayer *vl = mFilterModel->layer();
  QgsMapCanvas *canvas = mFilterModel->mapCanvas();
  if ( canvas && vl && vl->geometryType() != QgsWkbTypes::NullGeometry )
  {
    QAction *zoomToFeatureAction = menu->addAction( tr( "Zoom to Feature" ) );
    connect( zoomToFeatureAction, &QAction::triggered, this, &QgsDualView::zoomToCurrentFeature );

    QAction *panToFeatureAction = menu->addAction( tr( "Pan to Feature" ) );
    connect( panToFeatureAction, &QAction::triggered, this, &QgsDualView::panToCurrentFeature );

    QAction *flashFeatureAction = menu->addAction( tr( "Flash Feature" ) );
    connect( flashFeatureAction, &QAction::triggered, this, &QgsDualView::flashCurrentFeature );
  }

  //add user-defined actions to context menu
  const QList<QgsAction> actions = mLayer->actions()->actions( QStringLiteral( "Field" ) );
  if ( !actions.isEmpty() )
  {
    QAction *a = menu->addAction( tr( "Run Layer Action" ) );
    a->setEnabled( false );

    for ( const QgsAction &action : actions )
    {
      if ( !action.runable() )
        continue;

      if ( vl && !vl->isEditable() && action.isEnabledOnlyWhenEditable() )
        continue;

      QgsAttributeTableAction *a = new QgsAttributeTableAction( action.name(), this, action.id(), masterIndex );
      menu->addAction( action.name(), a, &QgsAttributeTableAction::execute );
    }
  }
  const QModelIndex rowSourceIndex = mMasterModel->index( masterIndex.row(), 0 );
  if ( ! rowSourceIndex.isValid() )
  {
    return;
  }

  //add actions from QgsMapLayerActionRegistry to context menu
  const QList<QgsMapLayerAction *> registeredActions = QgsGui::mapLayerActionRegistry()->mapLayerActions( mLayer, QgsMapLayerAction::Layer | QgsMapLayerAction::SingleFeature );
  if ( !registeredActions.isEmpty() )
  {
    //add a separator between user defined and standard actions
    menu->addSeparator();

    for ( QgsMapLayerAction *action : registeredActions )
    {
      QgsAttributeTableMapLayerAction *a = new QgsAttributeTableMapLayerAction( action->text(), this, action, rowSourceIndex );
      menu->addAction( action->text(), a, &QgsAttributeTableMapLayerAction::execute );
    }
  }

  // entries for multiple features layer actions
  // only show if the context menu is shown over a selected row
  const QgsFeatureId currentFid = mMasterModel->rowToId( masterIndex.row() );
  if ( mLayer->selectedFeatureCount() > 1 && mLayer->selectedFeatureIds().contains( currentFid ) )
  {
    const QList<QgsMapLayerAction *> registeredActions = QgsGui::mapLayerActionRegistry()->mapLayerActions( mLayer, QgsMapLayerAction::MultipleFeatures );
    if ( !registeredActions.isEmpty() )
    {
      menu->addSeparator();
      QAction *action = menu->addAction( tr( "Actions on Selection (%1)" ).arg( mLayer->selectedFeatureCount() ) );
      action->setEnabled( false );

      for ( QgsMapLayerAction *action : registeredActions )
      {
        menu->addAction( action->text(), action, [ = ]() {action->triggerForFeatures( mLayer, mLayer->selectedFeatures() );} );
      }
    }
  }

  menu->addSeparator();
  QgsAttributeTableAction *a = new QgsAttributeTableAction( tr( "Open Form" ), this, QUuid(), rowSourceIndex );
  menu->addAction( tr( "Open Form…" ), a, &QgsAttributeTableAction::featureForm );
}


void QgsDualView::widgetWillShowContextMenu( QgsActionMenu *menu, const QModelIndex &atIndex )
{
  emit showContextMenuExternally( menu, mFilterModel->rowToId( atIndex ) );
}


void QgsDualView::showViewHeaderMenu( QPoint point )
{
  const int col = mTableView->columnAt( point.x() );

  delete mHorizontalHeaderMenu;
  mHorizontalHeaderMenu = new QMenu( this );

  QAction *hide = new QAction( tr( "&Hide Column" ), mHorizontalHeaderMenu );
  connect( hide, &QAction::triggered, this, &QgsDualView::hideColumn );
  hide->setData( col );
  mHorizontalHeaderMenu->addAction( hide );
  QAction *setWidth = new QAction( tr( "&Set Width…" ), mHorizontalHeaderMenu );
  connect( setWidth, &QAction::triggered, this, &QgsDualView::resizeColumn );
  setWidth->setData( col );
  mHorizontalHeaderMenu->addAction( setWidth );

  QAction *setWidthAllColumns = new QAction( tr( "&Set All Column Widths…" ), mHorizontalHeaderMenu );
  connect( setWidthAllColumns, &QAction::triggered, this, &QgsDualView::resizeAllColumns );
  setWidthAllColumns->setData( col );
  mHorizontalHeaderMenu->addAction( setWidthAllColumns );

  QAction *optimizeWidth = new QAction( tr( "&Autosize" ), mHorizontalHeaderMenu );
  connect( optimizeWidth, &QAction::triggered, this, &QgsDualView::autosizeColumn );
  optimizeWidth->setData( col );
  mHorizontalHeaderMenu->addAction( optimizeWidth );

  QAction *optimizeWidthAllColumns = new QAction( tr( "&Autosize All Columns" ), mHorizontalHeaderMenu );
  connect( optimizeWidthAllColumns, &QAction::triggered, this, &QgsDualView::autosizeAllColumns );
  mHorizontalHeaderMenu->addAction( optimizeWidthAllColumns );


  mHorizontalHeaderMenu->addSeparator();
  QAction *organize = new QAction( tr( "&Organize Columns…" ), mHorizontalHeaderMenu );
  connect( organize, &QAction::triggered, this, &QgsDualView::organizeColumns );
  mHorizontalHeaderMenu->addAction( organize );
  QAction *sort = new QAction( tr( "&Sort…" ), mHorizontalHeaderMenu );
  connect( sort, &QAction::triggered, this, [ = ]() {modifySort();} );
  mHorizontalHeaderMenu->addAction( sort );

  mHorizontalHeaderMenu->popup( mTableView->horizontalHeader()->mapToGlobal( point ) );
}

void QgsDualView::organizeColumns()
{
  if ( !mLayer )
  {
    return;
  }

  QgsOrganizeTableColumnsDialog dialog( mLayer, attributeTableConfig(), this );
  if ( dialog.exec() == QDialog::Accepted )
  {
    const QgsAttributeTableConfig config = dialog.config();
    setAttributeTableConfig( config );
  }
}

void QgsDualView::tableColumnResized( int column, int width )
{
  QgsAttributeTableConfig config = mConfig;
  const int sourceCol = config.mapVisibleColumnToIndex( column );
  if ( sourceCol >= 0 && config.columnWidth( sourceCol ) != width )
  {
    config.setColumnWidth( sourceCol, width );
    setAttributeTableConfig( config );
  }
}

void QgsDualView::hideColumn()
{
  QAction *action = qobject_cast<QAction *>( sender() );
  const int col = action->data().toInt();
  QgsAttributeTableConfig config = mConfig;
  const int sourceCol = mConfig.mapVisibleColumnToIndex( col );
  if ( sourceCol >= 0 )
  {
    config.setColumnHidden( sourceCol, true );
    setAttributeTableConfig( config );
  }
}

void QgsDualView::resizeColumn()
{
  QAction *action = qobject_cast<QAction *>( sender() );
  const int col = action->data().toInt();
  if ( col < 0 )
    return;

  QgsAttributeTableConfig config = mConfig;
  const int sourceCol = config.mapVisibleColumnToIndex( col );
  if ( sourceCol >= 0 )
  {
    bool ok = false;
    const int width = QInputDialog::getInt( this, tr( "Set column width" ), tr( "Enter column width" ),
                                            mTableView->columnWidth( col ),
                                            0, 1000, 10, &ok );
    if ( ok )
    {
      config.setColumnWidth( sourceCol, width );
      setAttributeTableConfig( config );
    }
  }
}

void QgsDualView::resizeAllColumns()
{
  QAction *action = qobject_cast<QAction *>( sender() );
  const int col = action->data().toInt();
  if ( col < 0 )
    return;

  QgsAttributeTableConfig config = mConfig;

  bool ok = false;
  const int width = QInputDialog::getInt( this, tr( "Set Column Width" ), tr( "Enter column width" ),
                                          mTableView->columnWidth( col ),
                                          1, 1000, 10, &ok );
  if ( ok )
  {
    const int colCount = mTableView->model()->columnCount();
    if ( colCount > 0 )
    {
      for ( int i = 0; i < colCount; i++ )
      {
        config.setColumnWidth( i, width );
      }
      setAttributeTableConfig( config );
    }
  }
}

void QgsDualView::autosizeColumn()
{
  QAction *action = qobject_cast<QAction *>( sender() );
  const int col = action->data().toInt();
  mTableView->resizeColumnToContents( col );
}

void QgsDualView::autosizeAllColumns()
{
  mTableView->resizeColumnsToContents();
}

bool QgsDualView::modifySort()
{
  if ( !mLayer )
    return false;

  QgsAttributeTableConfig config = mConfig;

  QDialog orderByDlg;
  orderByDlg.setWindowTitle( tr( "Configure Attribute Table Sort Order" ) );
  QDialogButtonBox *dialogButtonBox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel );
  QGridLayout *layout = new QGridLayout();
  connect( dialogButtonBox, &QDialogButtonBox::accepted, &orderByDlg, &QDialog::accept );
  connect( dialogButtonBox, &QDialogButtonBox::rejected, &orderByDlg, &QDialog::reject );
  orderByDlg.setLayout( layout );

  QGroupBox *sortingGroupBox = new QGroupBox();
  sortingGroupBox->setTitle( tr( "Defined sort order in attribute table" ) );
  sortingGroupBox->setCheckable( true );
  sortingGroupBox->setChecked( !sortExpression().isEmpty() );
  layout->addWidget( sortingGroupBox );
  sortingGroupBox->setLayout( new QGridLayout() );

  QgsExpressionBuilderWidget *expressionBuilder = new QgsExpressionBuilderWidget();
  const QgsExpressionContext context( QgsExpressionContextUtils::globalProjectLayerScopes( mLayer ) );

  expressionBuilder->initWithLayer( mLayer, context, QStringLiteral( "generic" ) );
  expressionBuilder->setExpressionText( sortExpression().isEmpty() ? mLayer->displayExpression() : sortExpression() );

  sortingGroupBox->layout()->addWidget( expressionBuilder );

  QCheckBox *cbxSortAscending = new QCheckBox( tr( "Sort ascending" ) );
  cbxSortAscending->setChecked( config.sortOrder() == Qt::AscendingOrder );
  sortingGroupBox->layout()->addWidget( cbxSortAscending );

  layout->addWidget( dialogButtonBox );
  if ( orderByDlg.exec() )
  {
    const Qt::SortOrder sortOrder = cbxSortAscending->isChecked() ? Qt::AscendingOrder : Qt::DescendingOrder;
    if ( sortingGroupBox->isChecked() )
    {
      setSortExpression( expressionBuilder->expressionText(), sortOrder );
      config.setSortExpression( expressionBuilder->expressionText() );
      config.setSortOrder( sortOrder );
    }
    else
    {
      setSortExpression( QString(), sortOrder );
      config.setSortExpression( QString() );
    }

    setAttributeTableConfig( config );
    return true;
  }
  else
  {
    return false;
  }

}

void QgsDualView::zoomToCurrentFeature()
{
  const QModelIndex currentIndex = mTableView->currentIndex();
  if ( !currentIndex.isValid() )
  {
    return;
  }

  QgsFeatureIds ids;
  ids.insert( mFilterModel->rowToId( currentIndex ) );
  QgsMapCanvas *canvas = mFilterModel->mapCanvas();
  if ( canvas )
  {
    canvas->zoomToFeatureIds( mLayer, ids );
  }
}

void QgsDualView::panToCurrentFeature()
{
  const QModelIndex currentIndex = mTableView->currentIndex();
  if ( !currentIndex.isValid() )
  {
    return;
  }

  QgsFeatureIds ids;
  ids.insert( mFilterModel->rowToId( currentIndex ) );
  QgsMapCanvas *canvas = mFilterModel->mapCanvas();
  if ( canvas )
  {
    canvas->panToFeatureIds( mLayer, ids );
  }
}

void QgsDualView::flashCurrentFeature()
{
  const QModelIndex currentIndex = mTableView->currentIndex();
  if ( !currentIndex.isValid() )
  {
    return;
  }

  QgsFeatureIds ids;
  ids.insert( mFilterModel->rowToId( currentIndex ) );
  QgsMapCanvas *canvas = mFilterModel->mapCanvas();
  if ( canvas )
  {
    canvas->flashFeatureIds( mLayer, ids );
  }
}

void QgsDualView::rebuildFullLayerCache()
{
  connect( mLayerCache, &QgsVectorLayerCache::progress, this, &QgsDualView::progress, Qt::UniqueConnection );
  connect( mLayerCache, &QgsVectorLayerCache::finished, this, &QgsDualView::finished, Qt::UniqueConnection );

  mLayerCache->setFullCache( true );
}

void QgsDualView::previewExpressionChanged( const QString &expression )
{
  mLayer->setDisplayExpression( expression );
}

void QgsDualView::onSortColumnChanged()
{
  QgsAttributeTableConfig cfg = attributeTableConfig();
  if ( cfg.sortExpression() != mFilterModel->sortExpression() ||
       cfg.sortOrder() != mFilterModel->sortOrder() )
  {
    cfg.setSortExpression( mFilterModel->sortExpression() );
    cfg.setSortOrder( mFilterModel->sortOrder() );
    setAttributeTableConfig( cfg );
  }
}

void QgsDualView::updateSelectedFeatures()
{
  QgsFeatureRequest r = mMasterModel->request();
  if ( r.filterType() == QgsFeatureRequest::FilterNone && r.spatialFilterType() == Qgis::SpatialFilterType::NoFilter )
    return; // already requested all features

  r.setFilterFids( masterModel()->layer()->selectedFeatureIds() );
  mMasterModel->setRequest( r );
  mMasterModel->loadLayer();
  emit filterChanged();
}

void QgsDualView::updateEditedAddedFeatures()
{
  QgsFeatureRequest r = mMasterModel->request();
  if ( r.filterType() == QgsFeatureRequest::FilterNone && r.spatialFilterType() == Qgis::SpatialFilterType::NoFilter )
    return; // already requested all features

  r.setFilterFids( masterModel()->layer()->editBuffer() ? masterModel()->layer()->editBuffer()->allAddedOrEditedFeatures() : QgsFeatureIds() );
  mMasterModel->setRequest( r );
  mMasterModel->loadLayer();
  emit filterChanged();
}

void QgsDualView::extentChanged()
{
  QgsFeatureRequest r = mMasterModel->request();
  if ( mFilterModel->mapCanvas() && ( r.filterType() != QgsFeatureRequest::FilterNone || !r.filterRect().isNull() ) )
  {
    const QgsRectangle rect = mFilterModel->mapCanvas()->mapSettings().mapToLayerCoordinates( mLayer, mFilterModel->mapCanvas()->extent() );
    r.setFilterRect( rect );
    mMasterModel->setRequest( r );
    mMasterModel->loadLayer();
  }
  emit filterChanged();
}

void QgsDualView::featureFormAttributeChanged( const QString &attribute, const QVariant &value, bool attributeChanged )
{
  Q_UNUSED( attribute )
  Q_UNUSED( value )
  if ( attributeChanged )
  {
    mFeatureListView->setCurrentFeatureEdited( true );
    mAttributeForm->save();
  }
}

void QgsDualView::setFilteredFeatures( const QgsFeatureIds &filteredFeatures )
{
  mFilterModel->setFilteredFeatures( filteredFeatures );
}

void QgsDualView::filterFeatures( const QgsExpression &filterExpression, const QgsExpressionContext &context )
{
  mFilterModel->setFilterExpression( filterExpression, context );
  mFilterModel->filterFeatures();
}


void QgsDualView::setRequest( const QgsFeatureRequest &request )
{
  mMasterModel->setRequest( request );
}

void QgsDualView::setFeatureSelectionManager( QgsIFeatureSelectionManager *featureSelectionManager )
{
  mTableView->setFeatureSelectionManager( featureSelectionManager );
  mFeatureListView->setFeatureSelectionManager( featureSelectionManager );

  if ( mFeatureSelectionManager && mFeatureSelectionManager->parent() == this )
    delete mFeatureSelectionManager;

  mFeatureSelectionManager = featureSelectionManager;
}

void QgsDualView::setAttributeTableConfig( const QgsAttributeTableConfig &config )
{
  mConfig = config;
  mConfig.update( mLayer->fields() );
  mLayer->setAttributeTableConfig( mConfig );
  mFilterModel->setAttributeTableConfig( mConfig );
  mTableView->setAttributeTableConfig( mConfig );
}

void QgsDualView::setSortExpression( const QString &sortExpression, Qt::SortOrder sortOrder )
{
  if ( sortExpression.isNull() )
    mFilterModel->sort( -1 );
  else
    mFilterModel->sort( sortExpression, sortOrder );

  mConfig.setSortExpression( sortExpression );
  mConfig.setSortOrder( sortOrder );
  setAttributeTableConfig( mConfig );
}

QString QgsDualView::sortExpression() const
{
  return mFilterModel->sortExpression();
}

QgsAttributeTableConfig QgsDualView::attributeTableConfig() const
{
  return mConfig;
}

void QgsDualView::progress( int i, bool &cancel )
{
  if ( !mProgressDlg )
  {
    mProgressDlg = new QProgressDialog( tr( "Loading features…" ), tr( "Abort" ), 0, 0, this );
    mProgressDlg->setWindowTitle( tr( "Attribute Table" ) );
    mProgressDlg->setWindowModality( Qt::WindowModal );
    mProgressDlg->show();
  }

  mProgressDlg->setLabelText( tr( "%L1 features loaded." ).arg( i ) );
  QCoreApplication::processEvents();

  cancel = mProgressDlg && mProgressDlg->wasCanceled();
}

void QgsDualView::finished()
{
  delete mProgressDlg;
  mProgressDlg = nullptr;
}

/*
 * QgsAttributeTableAction
 */

void QgsAttributeTableAction::execute()
{
  mDualView->masterModel()->executeAction( mAction, mFieldIdx );
}

void QgsAttributeTableAction::featureForm()
{
  QgsFeatureIds editedIds;
  editedIds << mDualView->masterModel()->rowToId( mFieldIdx.row() );
  mDualView->setCurrentEditSelection( editedIds );
  mDualView->setView( QgsDualView::AttributeEditor );
}

/*
 * QgsAttributeTableMapLayerAction
 */

void QgsAttributeTableMapLayerAction::execute()
{
  mDualView->masterModel()->executeMapLayerAction( mAction, mFieldIdx );
}
