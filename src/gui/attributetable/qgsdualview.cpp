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

#include <QClipboard>
#include <QDialog>
#include <QMenu>
#include <QMessageBox>
#include <QProgressDialog>
#include <QSettings>
#include <QGroupBox>
#include <QInputDialog>

QgsDualView::QgsDualView( QWidget* parent )
    : QStackedWidget( parent )
    , mEditorContext()
    , mMasterModel( nullptr )
    , mFilterModel( nullptr )
    , mFeatureListModel( nullptr )
    , mAttributeForm( nullptr )
    , mHorizontalHeaderMenu( nullptr )
    , mLayerCache( nullptr )
    , mProgressDlg( nullptr )
    , mFeatureSelectionManager( nullptr )
    , mAttributeEditorScrollArea( nullptr )
    , mMapCanvas( nullptr )
{
  setupUi( this );

  mConditionalFormatWidget->hide();

  mPreviewActionMapper = new QSignalMapper( this );

  mPreviewColumnsMenu = new QMenu( this );
  mActionPreviewColumnsMenu->setMenu( mPreviewColumnsMenu );

  // Set preview icon
  mActionExpressionPreview->setIcon( QgsApplication::getThemeIcon( "/mIconExpressionPreview.svg" ) );

  // Connect layer list preview signals
  connect( mActionExpressionPreview, SIGNAL( triggered() ), SLOT( previewExpressionBuilder() ) );
  connect( mPreviewActionMapper, SIGNAL( mapped( QObject* ) ), SLOT( previewColumnChanged( QObject* ) ) );
  connect( mFeatureList, SIGNAL( displayExpressionChanged( QString ) ), this, SLOT( previewExpressionChanged( QString ) ) );
}

void QgsDualView::init( QgsVectorLayer *layer, QgsMapCanvas *mapCanvas, const QgsFeatureRequest &request, const QgsAttributeEditorContext &context, bool loadFeatures )
{
  mMapCanvas = mapCanvas;

  if ( !layer )
    return;

  mLayer = layer;

  mEditorContext = context;

  connect( mTableView, SIGNAL( willShowContextMenu( QMenu*, QModelIndex ) ), this, SLOT( viewWillShowContextMenu( QMenu*, QModelIndex ) ) );
  mTableView->horizontalHeader()->setContextMenuPolicy( Qt::CustomContextMenu );
  connect( mTableView->horizontalHeader(), SIGNAL( customContextMenuRequested( QPoint ) ), this, SLOT( showViewHeaderMenu( QPoint ) ) );
  connect( mTableView, SIGNAL( columnResized( int, int ) ), this, SLOT( tableColumnResized( int, int ) ) );

  initLayerCache( !( request.flags() & QgsFeatureRequest::NoGeometry ) || !request.filterRect().isNull() );
  initModels( mapCanvas, request, loadFeatures );

  mConditionalFormatWidget->setLayer( mLayer );

  mTableView->setModel( mFilterModel );
  mFeatureList->setModel( mFeatureListModel );
  delete mAttributeForm;
  mAttributeForm = new QgsAttributeForm( mLayer, QgsFeature(), mEditorContext );
  if ( !context.parentContext() )
  {
    mAttributeEditorScrollArea = new QScrollArea();
    mAttributeEditorScrollArea->setWidgetResizable( true );
    mAttributeEditor->layout()->addWidget( mAttributeEditorScrollArea );
    mAttributeEditorScrollArea->setWidget( mAttributeForm );
  }
  else
  {
    mAttributeEditor->layout()->addWidget( mAttributeForm );
  }

  connect( mAttributeForm, SIGNAL( attributeChanged( QString, QVariant ) ), this, SLOT( featureFormAttributeChanged() ) );
  connect( mAttributeForm, SIGNAL( modeChanged( QgsAttributeForm::Mode ) ), this, SIGNAL( formModeChanged( QgsAttributeForm::Mode ) ) );
  connect( mMasterModel, SIGNAL( modelChanged() ), mAttributeForm, SLOT( refreshFeature() ) );
  connect( mAttributeForm, SIGNAL( filterExpressionSet( QString, QgsAttributeForm::FilterType ) ), this, SIGNAL( filterExpressionSet( QString, QgsAttributeForm::FilterType ) ) );
  connect( mFilterModel, SIGNAL( sortColumnChanged( int, Qt::SortOrder ) ), this, SLOT( onSortColumnChanged() ) );
  if ( mFeatureListPreviewButton->defaultAction() )
    mFeatureList->setDisplayExpression( mDisplayExpression );
  else
    columnBoxInit();

  // This slows down load of the attribute table heaps and uses loads of memory.
  //mTableView->resizeColumnsToContents();

  mFeatureList->setEditSelection( QgsFeatureIds() << mFeatureListModel->idxToFid( mFeatureListModel->index( 0, 0 ) ) );
}

void QgsDualView::columnBoxInit()
{
  // load fields
  QList<QgsField> fields = mLayer->fields().toList();

  QString defaultField;

  // default expression: saved value
  QString displayExpression = mLayer->displayExpression();

  // if no display expression is saved: use display field instead
  if ( displayExpression.isEmpty() )
  {
    if ( !mLayer->displayField().isEmpty() )
    {
      defaultField = mLayer->displayField();
      displayExpression = QString( "COALESCE(\"%1\", '<NULL>')" ).arg( defaultField );
    }
  }

  // if neither display expression nor display field is saved...
  if ( displayExpression.isEmpty() )
  {
    QgsAttributeList pkAttrs = mLayer->pkAttributeList();

    if ( !pkAttrs.isEmpty() )
    {
      if ( pkAttrs.size() == 1 )
        defaultField = pkAttrs.at( 0 );

      // ... If there are primary key(s) defined
      QStringList pkFields;

      Q_FOREACH ( int attr, pkAttrs )
      {
        pkFields.append( "COALESCE(\"" + fields[attr].name() + "\", '<NULL>')" );
      }

      displayExpression = pkFields.join( "||', '||" );
    }
    else if ( !fields.isEmpty() )
    {
      if ( fields.size() == 1 )
        defaultField = fields.at( 0 ).name();

      // ... concat all fields
      QStringList fieldNames;
      Q_FOREACH ( const QgsField& field, fields )
      {
        fieldNames.append( "COALESCE(\"" + field.name() + "\", '<NULL>')" );
      }

      displayExpression = fieldNames.join( "||', '||" );
    }
    else
    {
      // ... there isn't really much to display
      displayExpression = "'[Please define preview text]'";
    }
  }

  mFeatureListPreviewButton->addAction( mActionExpressionPreview );
  mFeatureListPreviewButton->addAction( mActionPreviewColumnsMenu );

  Q_FOREACH ( const QgsField& field, fields )
  {
    int fieldIndex = mLayer->fieldNameIndex( field.name() );
    if ( fieldIndex == -1 )
      continue;

    if ( mLayer->editFormConfig()->widgetType( fieldIndex ) != "Hidden" )
    {
      QIcon icon = mLayer->fields().iconForField( fieldIndex );
      QString text = field.name();

      // Generate action for the preview popup button of the feature list
      QAction* previewAction = new QAction( icon, text, mFeatureListPreviewButton );
      mPreviewActionMapper->setMapping( previewAction, previewAction );
      connect( previewAction, SIGNAL( triggered() ), mPreviewActionMapper, SLOT( map() ) );
      mPreviewColumnsMenu->addAction( previewAction );

      if ( text == defaultField )
      {
        mFeatureListPreviewButton->setDefaultAction( previewAction );
      }
    }
  }

  // If there is no single field found as preview
  if ( !mFeatureListPreviewButton->defaultAction() )
  {
    mFeatureList->setDisplayExpression( displayExpression );
    mFeatureListPreviewButton->setDefaultAction( mActionExpressionPreview );
    mDisplayExpression = mFeatureList->displayExpression();
  }
  else
  {
    mFeatureListPreviewButton->defaultAction()->trigger();
  }

  QAction* sortByPreviewExpression = new QAction( QgsApplication::getThemeIcon( "sort.svg" ), tr( "Sort by preview expression" ), this );
  connect( sortByPreviewExpression, SIGNAL( triggered( bool ) ), this, SLOT( sortByPreviewExpression() ) );
  mFeatureListPreviewButton->addAction( sortByPreviewExpression );
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
      disconnect( mMapCanvas, SIGNAL( extentsChanged() ), this, SLOT( extentChanged() ) );
      break;

    case QgsAttributeTableFilterModel::ShowAll:
    case QgsAttributeTableFilterModel::ShowEdited:
    case QgsAttributeTableFilterModel::ShowFilteredList:
      break;

    case QgsAttributeTableFilterModel::ShowSelected:
      disconnect( masterModel()->layer(), SIGNAL( selectionChanged() ), this,
                  SLOT( updateSelectedFeatures() ) );
      break;
  }

  QgsFeatureRequest r = mMasterModel->request();
  bool needsGeometry = filterMode == QgsAttributeTableFilterModel::ShowVisible;

  bool requiresTableReload = ( r.filterType() != QgsFeatureRequest::FilterNone || !r.filterRect().isNull() ) // previous request was subset
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
      connect( mMapCanvas, SIGNAL( extentsChanged() ), this, SLOT( extentChanged() ) );
      if ( mMapCanvas )
      {
        QgsRectangle rect = mMapCanvas->mapSettings().mapToLayerCoordinates( mLayer, mMapCanvas->extent() );
        r.setFilterRect( rect );
      }
      break;

    case QgsAttributeTableFilterModel::ShowAll:
    case QgsAttributeTableFilterModel::ShowEdited:
    case QgsAttributeTableFilterModel::ShowFilteredList:
      break;

    case QgsAttributeTableFilterModel::ShowSelected:
      connect( masterModel()->layer(), SIGNAL( selectionChanged() ), this, SLOT( updateSelectedFeatures() ) );
      if ( masterModel()->layer()->selectedFeatureCount() > 0 )
        r.setFilterFids( masterModel()->layer()->selectedFeaturesIds() );
      break;
  }

  if ( requiresTableReload )
  {
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
  QSettings settings;
  int cacheSize = settings.value( "/qgis/attributeTableRowCache", "10000" ).toInt();
  mLayerCache = new QgsVectorLayerCache( mLayer, cacheSize, this );
  mLayerCache->setCacheGeometry( cacheGeometry );
  if ( 0 == cacheSize || 0 == ( QgsVectorDataProvider::SelectAtId & mLayer->dataProvider()->capabilities() ) )
  {
    connect( mLayerCache, SIGNAL( invalidated() ), this, SLOT( rebuildFullLayerCache() ) );
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

  connect( mMasterModel, SIGNAL( progress( int, bool & ) ), this, SLOT( progress( int, bool & ) ) );
  connect( mMasterModel, SIGNAL( finished() ), this, SLOT( finished() ) );

  connect( mConditionalFormatWidget, SIGNAL( rulesUpdated( QString ) ), mMasterModel, SLOT( fieldConditionalStyleChanged( QString ) ) );

  if ( loadFeatures )
    mMasterModel->loadLayer();

  mFilterModel = new QgsAttributeTableFilterModel( mapCanvas, mMasterModel, mMasterModel );

  connect( mFeatureList, SIGNAL( displayExpressionChanged( QString ) ), this, SIGNAL( displayExpressionChanged( QString ) ) );

  mFeatureListModel = new QgsFeatureListModel( mFilterModel, mFilterModel );
}

void QgsDualView::on_mFeatureList_aboutToChangeEditSelection( bool& ok )
{
  if ( mLayer->isEditable() && !mAttributeForm->save() )
    ok = false;
}

void QgsDualView::on_mFeatureList_currentEditSelectionChanged( const QgsFeature &feat )
{
  if ( !mLayer->isEditable() || mAttributeForm->save() )
  {
    mAttributeForm->setFeature( feat );
    setCurrentEditSelection( QgsFeatureIds() << feat.id() );
  }
  else
  {
    // Couldn't save feature
  }
}

void QgsDualView::setCurrentEditSelection( const QgsFeatureIds& fids )
{
  mFeatureList->setCurrentFeatureEdited( false );
  mFeatureList->setEditSelection( fids );
}

bool QgsDualView::saveEditChanges()
{
  return mAttributeForm->save();
}

void QgsDualView::openConditionalStyles()
{
  mConditionalFormatWidget->setVisible( !mConditionalFormatWidget->isVisible() );
  mConditionalFormatWidget->viewRules();
}

void QgsDualView::setMultiEditEnabled( bool enabled )
{
  if ( enabled )
    setView( AttributeEditor );

  mAttributeForm->setMode( enabled ? QgsAttributeForm::MultiEditMode : QgsAttributeForm::SingleEditMode );
}

void QgsDualView::toggleSearchMode( bool enabled )
{
  if ( enabled )
  {
    setView( AttributeEditor );
    mAttributeForm->setMode( QgsAttributeForm::SearchMode );
  }
  else
  {
    mAttributeForm->setMode( QgsAttributeForm::SingleEditMode );
  }
}

void QgsDualView::previewExpressionBuilder()
{
  // Show expression builder
  QgsExpressionContext context;
  context << QgsExpressionContextUtils::globalScope()
  << QgsExpressionContextUtils::projectScope()
  << QgsExpressionContextUtils::layerScope( mLayer );

  QgsExpressionBuilderDialog dlg( mLayer, mFeatureList->displayExpression(), this, "generic", context );
  dlg.setWindowTitle( tr( "Expression based preview" ) );
  dlg.setExpressionText( mFeatureList->displayExpression() );

  if ( dlg.exec() == QDialog::Accepted )
  {
    mFeatureList->setDisplayExpression( dlg.expressionText() );
    mFeatureListPreviewButton->setDefaultAction( mActionExpressionPreview );
    mFeatureListPreviewButton->setPopupMode( QToolButton::MenuButtonPopup );
  }

  mDisplayExpression = mFeatureList->displayExpression();
}

void QgsDualView::previewColumnChanged( QObject* action )
{
  QAction* previewAction = qobject_cast< QAction* >( action );

  if ( previewAction )
  {
    if ( !mFeatureList->setDisplayExpression( QString( "COALESCE( \"%1\", '<NULL>' )" ).arg( previewAction->text() ) ) )
    {
      QMessageBox::warning( this,
                            tr( "Could not set preview column" ),
                            tr( "Could not set column '%1' as preview column.\nParser error:\n%2" )
                            .arg( previewAction->text(), mFeatureList->parserErrorString() )
                          );
    }
    else
    {
      mFeatureListPreviewButton->setDefaultAction( previewAction );
      mFeatureListPreviewButton->setPopupMode( QToolButton::InstantPopup );
    }
  }

  mDisplayExpression = mFeatureList->displayExpression();

  Q_ASSERT( previewAction );
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
  QAction* action = qobject_cast<QAction*>( sender() );

  if ( action && action->data().isValid() && action->data().canConvert<QModelIndex>() )
  {
    QModelIndex index = action->data().value<QModelIndex>();
    QVariant var = masterModel()->data( index, Qt::DisplayRole );
    QApplication::clipboard()->setText( var.toString() );
  }
}

void QgsDualView::viewWillShowContextMenu( QMenu* menu, const QModelIndex& atIndex )
{
  if ( !menu )
  {
    return;
  }


  QModelIndex sourceIndex = mFilterModel->mapToSource( atIndex );

  QAction *copyContentAction = new QAction( tr( "Copy cell content" ), this );
  copyContentAction->setData( QVariant::fromValue<QModelIndex>( sourceIndex ) );
  menu->addAction( copyContentAction );
  connect( copyContentAction, SIGNAL( triggered() ), this, SLOT( copyCellContent() ) );

  QgsVectorLayer* vl = mFilterModel->layer();
  QgsMapCanvas* canvas = mFilterModel->mapCanvas();
  if ( canvas && vl && vl->geometryType() != QGis::NoGeometry )
  {
    menu->addAction( tr( "Zoom to feature" ), this, SLOT( zoomToCurrentFeature() ) );
  }

  //add user-defined actions to context menu
  if ( mLayer->actions()->size() != 0 )
  {

    QAction *a = menu->addAction( tr( "Run layer action" ) );
    a->setEnabled( false );

    for ( int i = 0; i < mLayer->actions()->size(); i++ )
    {
      const QgsAction &action = mLayer->actions()->at( i );

      if ( !action.runable() )
        continue;

      QgsAttributeTableAction *a = new QgsAttributeTableAction( action.name(), this, i, sourceIndex );
      menu->addAction( action.name(), a, SLOT( execute() ) );
    }
  }

  //add actions from QgsMapLayerActionRegistry to context menu
  QList<QgsMapLayerAction *> registeredActions = QgsMapLayerActionRegistry::instance()->mapLayerActions( mLayer );
  if ( !registeredActions.isEmpty() )
  {
    //add a separator between user defined and standard actions
    menu->addSeparator();

    QList<QgsMapLayerAction*>::iterator actionIt;
    for ( actionIt = registeredActions.begin(); actionIt != registeredActions.end(); ++actionIt )
    {
      QgsAttributeTableMapLayerAction *a = new QgsAttributeTableMapLayerAction(( *actionIt )->text(), this, ( *actionIt ), sourceIndex );
      menu->addAction(( *actionIt )->text(), a, SLOT( execute() ) );
    }
  }

  menu->addSeparator();
  QgsAttributeTableAction *a = new QgsAttributeTableAction( tr( "Open form" ), this, -1, sourceIndex );
  menu->addAction( tr( "Open form" ), a, SLOT( featureForm() ) );
}

void QgsDualView::showViewHeaderMenu( QPoint point )
{
  int col = mTableView->columnAt( point.x() );

  delete mHorizontalHeaderMenu;
  mHorizontalHeaderMenu = new QMenu( this );

  QAction* hide = new QAction( tr( "&Hide column" ), mHorizontalHeaderMenu );
  connect( hide, SIGNAL( triggered( bool ) ), this, SLOT( hideColumn() ) );
  hide->setData( col );
  mHorizontalHeaderMenu->addAction( hide );
  QAction* setWidth = new QAction( tr( "&Set width..." ), mHorizontalHeaderMenu );
  connect( setWidth, SIGNAL( triggered( bool ) ), this, SLOT( resizeColumn() ) );
  setWidth->setData( col );
  mHorizontalHeaderMenu->addAction( setWidth );
  QAction* optimizeWidth = new QAction( tr( "&Autosize" ), mHorizontalHeaderMenu );
  connect( optimizeWidth, SIGNAL( triggered( bool ) ), this, SLOT( autosizeColumn() ) );
  optimizeWidth->setData( col );
  mHorizontalHeaderMenu->addAction( optimizeWidth );

  mHorizontalHeaderMenu->addSeparator();
  QAction* organize = new QAction( tr( "&Organize columns..." ), mHorizontalHeaderMenu );
  connect( organize, SIGNAL( triggered( bool ) ), this, SLOT( organizeColumns() ) );
  mHorizontalHeaderMenu->addAction( organize );
  QAction* sort = new QAction( tr( "&Sort..." ), mHorizontalHeaderMenu );
  connect( sort, SIGNAL( triggered( bool ) ), this, SLOT( modifySort() ) );
  mHorizontalHeaderMenu->addAction( sort );

  mHorizontalHeaderMenu->popup( mTableView->horizontalHeader()->mapToGlobal( point ) );
}

void QgsDualView::organizeColumns()
{
  if ( !mLayer )
  {
    return;
  }

  QgsOrganizeTableColumnsDialog dialog( mLayer, this );
  if ( dialog.exec() == QDialog::Accepted )
  {
    QgsAttributeTableConfig config = dialog.config();
    setAttributeTableConfig( config );
  }
}

void QgsDualView::tableColumnResized( int column, int width )
{
  QgsAttributeTableConfig config = mConfig;
  int sourceCol = config.mapVisibleColumnToIndex( column );
  if ( sourceCol >= 0 )
  {
    config.setColumnWidth( sourceCol, width );
    setAttributeTableConfig( config );
  }
}

void QgsDualView::hideColumn()
{
  QAction* action = qobject_cast<QAction*>( sender() );
  int col = action->data().toInt();
  QgsAttributeTableConfig config = mConfig;
  int sourceCol = mConfig.mapVisibleColumnToIndex( col );
  if ( sourceCol >= 0 )
  {
    config.setColumnHidden( sourceCol, true );
    setAttributeTableConfig( config );
  }
}

void QgsDualView::resizeColumn()
{
  QAction* action = qobject_cast<QAction*>( sender() );
  int col = action->data().toInt();
  if ( col < 0 )
    return;

  QgsAttributeTableConfig config = mConfig;
  int sourceCol = config.mapVisibleColumnToIndex( col );
  if ( sourceCol >= 0 )
  {
    bool ok = false;
    int width = QInputDialog::getInt( this, tr( "Set column width" ), tr( "Enter column width" ),
                                      mTableView->columnWidth( col ),
                                      0, 1000, 10, &ok );
    if ( ok )
    {
      config.setColumnWidth( sourceCol, width );
      setAttributeTableConfig( config );
    }
  }
}

void QgsDualView::autosizeColumn()
{
  QAction* action = qobject_cast<QAction*>( sender() );
  int col = action->data().toInt();
  mTableView->resizeColumnToContents( col );
}

void QgsDualView::modifySort()
{
  if ( !mLayer )
    return;

  QgsAttributeTableConfig config = mConfig;

  QDialog orderByDlg;
  orderByDlg.setWindowTitle( tr( "Configure attribute table sort order" ) );
  QDialogButtonBox* dialogButtonBox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel );
  QGridLayout* layout = new QGridLayout();
  connect( dialogButtonBox, SIGNAL( accepted() ), &orderByDlg, SLOT( accept() ) );
  connect( dialogButtonBox, SIGNAL( rejected() ), &orderByDlg, SLOT( reject() ) );
  orderByDlg.setLayout( layout );

  QGroupBox* sortingGroupBox = new QGroupBox();
  sortingGroupBox->setTitle( tr( "Defined sort order in attribute table" ) );
  sortingGroupBox->setCheckable( true );
  sortingGroupBox->setChecked( !sortExpression().isEmpty() );
  layout->addWidget( sortingGroupBox );
  sortingGroupBox->setLayout( new QGridLayout() );

  QgsExpressionBuilderWidget* expressionBuilder = new QgsExpressionBuilderWidget();
  QgsExpressionContext context;
  context << QgsExpressionContextUtils::globalScope()
  << QgsExpressionContextUtils::projectScope()
  << QgsExpressionContextUtils::layerScope( mLayer );
  expressionBuilder->setExpressionContext( context );
  expressionBuilder->setLayer( mLayer );
  expressionBuilder->loadFieldNames();
  expressionBuilder->loadRecent( "generic" );
  expressionBuilder->setExpressionText( sortExpression().isEmpty() ? mLayer->displayExpression() : sortExpression() );

  sortingGroupBox->layout()->addWidget( expressionBuilder );

  QCheckBox* cbxSortAscending = new QCheckBox( tr( "Sort ascending" ) );
  cbxSortAscending->setChecked( config.sortOrder() == Qt::AscendingOrder );
  sortingGroupBox->layout()->addWidget( cbxSortAscending );

  layout->addWidget( dialogButtonBox );
  if ( orderByDlg.exec() )
  {
    Qt::SortOrder sortOrder = cbxSortAscending->isChecked() ? Qt::AscendingOrder : Qt::DescendingOrder;
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
  }
}

void QgsDualView::zoomToCurrentFeature()
{
  QModelIndex currentIndex = mTableView->currentIndex();
  if ( !currentIndex.isValid() )
  {
    return;
  }

  QgsFeatureIds ids;
  ids.insert( mFilterModel->rowToId( currentIndex ) );
  QgsMapCanvas* canvas = mFilterModel->mapCanvas();
  if ( canvas )
  {
    canvas->zoomToFeatureIds( mLayer, ids );
  }
}

void QgsDualView::rebuildFullLayerCache()
{
  connect( mLayerCache, SIGNAL( progress( int, bool& ) ), this, SLOT( progress( int, bool& ) ), Qt::UniqueConnection );
  connect( mLayerCache, SIGNAL( finished() ), this, SLOT( finished() ), Qt::UniqueConnection );

  mLayerCache->setFullCache( true );
}

void QgsDualView::previewExpressionChanged( const QString& expression )
{
  mLayer->setDisplayExpression( expression );
}

void QgsDualView::onSortColumnChanged()
{
  QgsAttributeTableConfig cfg = mLayer->attributeTableConfig();
  cfg.setSortExpression( mFilterModel->sortExpression() );
  cfg.setSortOrder( mFilterModel->sortOrder() );
  setAttributeTableConfig( cfg );
}

void QgsDualView::sortByPreviewExpression()
{
  Qt::SortOrder sortOrder = Qt::AscendingOrder;
  if ( mFeatureList->displayExpression() == sortExpression() )
  {
    sortOrder = mConfig.sortOrder() == Qt::AscendingOrder ? Qt::DescendingOrder : Qt::AscendingOrder;
  }
  setSortExpression( mFeatureList->displayExpression(), sortOrder );
}

void QgsDualView::updateSelectedFeatures()
{
  QgsFeatureRequest r = mMasterModel->request();
  if ( r.filterType() == QgsFeatureRequest::FilterNone && r.filterRect().isNull() )
    return; // already requested all features

  if ( masterModel()->layer()->selectedFeatureCount() > 0 )
    r.setFilterFids( masterModel()->layer()->selectedFeaturesIds() );
  else
    r.disableFilter();
  mMasterModel->setRequest( r );
  mMasterModel->loadLayer();
  emit filterChanged();
}

void QgsDualView::extentChanged()
{
  QgsFeatureRequest r = mMasterModel->request();
  if ( mMapCanvas && ( r.filterType() != QgsFeatureRequest::FilterNone || !r.filterRect().isNull() ) )
  {
    QgsRectangle rect = mMapCanvas->mapSettings().mapToLayerCoordinates( mLayer, mMapCanvas->extent() );
    r.setFilterRect( rect );
    mMasterModel->setRequest( r );
    mMasterModel->loadLayer();
  }
  emit filterChanged();
}

void QgsDualView::featureFormAttributeChanged()
{
  mFeatureList->setCurrentFeatureEdited( true );
}

void QgsDualView::setFilteredFeatures( const QgsFeatureIds& filteredFeatures )
{
  mFilterModel->setFilteredFeatures( filteredFeatures );
}

void QgsDualView::setRequest( const QgsFeatureRequest& request )
{
  mMasterModel->setRequest( request );
}

void QgsDualView::setFeatureSelectionManager( QgsIFeatureSelectionManager* featureSelectionManager )
{
  mTableView->setFeatureSelectionManager( featureSelectionManager );
  mFeatureList->setFeatureSelectionManager( featureSelectionManager );

  if ( mFeatureSelectionManager && mFeatureSelectionManager->parent() == this )
    delete mFeatureSelectionManager;

  mFeatureSelectionManager = featureSelectionManager;
}

void QgsDualView::setAttributeTableConfig( const QgsAttributeTableConfig& config )
{
  mConfig = config;
  mLayer->setAttributeTableConfig( config );
  mFilterModel->setAttributeTableConfig( config );
  mTableView->setAttributeTableConfig( config );
}

void QgsDualView::setSortExpression( const QString& sortExpression, Qt::SortOrder sortOrder )
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

void QgsDualView::progress( int i, bool& cancel )
{
  if ( !mProgressDlg )
  {
    mProgressDlg = new QProgressDialog( tr( "Loading features..." ), tr( "Abort" ), 0, 0, this );
    mProgressDlg->setWindowTitle( tr( "Attribute table" ) );
    mProgressDlg->setWindowModality( Qt::WindowModal );
    mProgressDlg->show();
  }

  mProgressDlg->setLabelText( tr( "%1 features loaded." ).arg( i ) );
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
