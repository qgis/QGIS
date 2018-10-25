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
#include "qgseditorwidgetregistry.h"
#include "qgssettings.h"
#include "qgsscrollarea.h"
#include "qgsgui.h"

#include <QClipboard>
#include <QDialog>
#include <QMenu>
#include <QMessageBox>
#include <QProgressDialog>
#include <QGroupBox>
#include <QInputDialog>

QgsDualView::QgsDualView( QWidget *parent )
  : QStackedWidget( parent )
{
  setupUi( this );
  connect( mFeatureList, &QgsFeatureListView::aboutToChangeEditSelection, this, &QgsDualView::mFeatureList_aboutToChangeEditSelection );
  connect( mFeatureList, &QgsFeatureListView::currentEditSelectionChanged, this, &QgsDualView::mFeatureList_currentEditSelectionChanged );

  mConditionalFormatWidget->hide();

  mPreviewColumnsMenu = new QMenu( this );
  mActionPreviewColumnsMenu->setMenu( mPreviewColumnsMenu );

  // Set preview icon
  mActionExpressionPreview->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mIconExpressionPreview.svg" ) ) );

  // Connect layer list preview signals
  connect( mActionExpressionPreview, &QAction::triggered, this, &QgsDualView::previewExpressionBuilder );
  connect( mFeatureList, &QgsFeatureListView::displayExpressionChanged, this, &QgsDualView::previewExpressionChanged );
}

void QgsDualView::init( QgsVectorLayer *layer, QgsMapCanvas *mapCanvas, const QgsFeatureRequest &request, const QgsAttributeEditorContext &context, bool loadFeatures )
{
  if ( !layer )
    return;

  mLayer = layer;

  mEditorContext = context;

  connect( mTableView, &QgsAttributeTableView::willShowContextMenu, this, &QgsDualView::viewWillShowContextMenu );
  mTableView->horizontalHeader()->setContextMenuPolicy( Qt::CustomContextMenu );
  connect( mTableView->horizontalHeader(), &QHeaderView::customContextMenuRequested, this, &QgsDualView::showViewHeaderMenu );
  connect( mTableView, &QgsAttributeTableView::columnResized, this, &QgsDualView::tableColumnResized );
  connect( mFeatureList, &QgsFeatureListView::willShowContextMenu, this, &QgsDualView::widgetWillShowContextMenu );

  initLayerCache( !( request.flags() & QgsFeatureRequest::NoGeometry ) || !request.filterRect().isNull() );
  initModels( mapCanvas, request, loadFeatures );

  mConditionalFormatWidget->setLayer( mLayer );

  mTableView->setModel( mFilterModel );
  mFeatureList->setModel( mFeatureListModel );
  delete mAttributeForm;
  mAttributeForm = new QgsAttributeForm( mLayer, mTempAttributeFormFeature, mEditorContext );
  mTempAttributeFormFeature = QgsFeature();
  if ( !context.parentContext() )
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

  connect( mAttributeForm, &QgsAttributeForm::widgetValueChanged, this, &QgsDualView::featureFormAttributeChanged );
  connect( mAttributeForm, &QgsAttributeForm::modeChanged, this, &QgsDualView::formModeChanged );
  connect( mMasterModel, &QgsAttributeTableModel::modelChanged, mAttributeForm, &QgsAttributeForm::refreshFeature );
  connect( mAttributeForm, &QgsAttributeForm::filterExpressionSet, this, &QgsDualView::filterExpressionSet );
  connect( mFilterModel, &QgsAttributeTableFilterModel::sortColumnChanged, this, &QgsDualView::onSortColumnChanged );

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

  if ( displayExpression.isEmpty() )
  {
    // ... there isn't really much to display
    displayExpression = QStringLiteral( "'[Please define preview text]'" );
  }

  mFeatureListPreviewButton->addAction( mActionExpressionPreview );
  mFeatureListPreviewButton->addAction( mActionPreviewColumnsMenu );

  Q_FOREACH ( const QgsField &field, fields )
  {
    int fieldIndex = mLayer->fields().lookupField( field.name() );
    if ( fieldIndex == -1 )
      continue;

    QString fieldName = field.name();
    if ( QgsGui::editorWidgetRegistry()->findBest( mLayer, fieldName ).type() != QLatin1String( "Hidden" ) )
    {
      QIcon icon = mLayer->fields().iconForField( fieldIndex );
      QString text = mLayer->attributeDisplayName( fieldIndex );

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

  QAction *sortByPreviewExpression = new QAction( QgsApplication::getThemeIcon( QStringLiteral( "sort.svg" ) ), tr( "Sort by preview expression" ), this );
  connect( sortByPreviewExpression, &QAction::triggered, this, &QgsDualView::sortByPreviewExpression );
  mFeatureListPreviewButton->addAction( sortByPreviewExpression );

  QAction *separator = new QAction( mFeatureListPreviewButton );
  separator->setSeparator( true );
  mFeatureListPreviewButton->addAction( separator );
  restoreRecentDisplayExpressions();

  // If there is no single field found as preview
  if ( !mFeatureListPreviewButton->defaultAction() )
  {
    mFeatureList->setDisplayExpression( displayExpression );
    mFeatureListPreviewButton->setDefaultAction( mActionExpressionPreview );
    setDisplayExpression( mFeatureList->displayExpression() );
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
      break;

    case QgsAttributeTableFilterModel::ShowAll:
    case QgsAttributeTableFilterModel::ShowEdited:
    case QgsAttributeTableFilterModel::ShowFilteredList:
      break;

    case QgsAttributeTableFilterModel::ShowSelected:
      disconnect( masterModel()->layer(), &QgsVectorLayer::selectionChanged, this, &QgsDualView::updateSelectedFeatures );
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
      connect( mFilterModel->mapCanvas(), &QgsMapCanvas::extentsChanged, this, &QgsDualView::extentChanged );
      if ( mFilterModel->mapCanvas() )
      {
        QgsRectangle rect = mFilterModel->mapCanvas()->mapSettings().mapToLayerCoordinates( mLayer, mFilterModel->mapCanvas()->extent() );
        r.setFilterRect( rect );
      }
      break;

    case QgsAttributeTableFilterModel::ShowAll:
    case QgsAttributeTableFilterModel::ShowEdited:
    case QgsAttributeTableFilterModel::ShowFilteredList:
      break;

    case QgsAttributeTableFilterModel::ShowSelected:
      connect( masterModel()->layer(), &QgsVectorLayer::selectionChanged, this, &QgsDualView::updateSelectedFeatures );
      r.setFilterFids( masterModel()->layer()->selectedFeatureIds() );
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
  QgsSettings settings;
  int cacheSize = settings.value( QStringLiteral( "qgis/attributeTableRowCache" ), "10000" ).toInt();
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
  // see regression https://issues.qgis.org/issues/15974
  connect( mMasterModel, &QgsAttributeTableModel::rowsRemoved, mFilterModel, &QgsAttributeTableFilterModel::invalidate );
  connect( mMasterModel, &QgsAttributeTableModel::rowsInserted, mFilterModel, &QgsAttributeTableFilterModel::invalidate );

  connect( mFeatureList, &QgsFeatureListView::displayExpressionChanged, this, &QgsDualView::displayExpressionChanged );

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
  QList<QAction *> actions = mFeatureListPreviewButton->actions();

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
  QList<QAction *> actions = mFeatureListPreviewButton->actions();

  // Remove existing same action
  int index = actions.indexOf( mLastDisplayExpressionAction );
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

    int fieldIndex = mLayer->fields().indexOf( name );
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

void QgsDualView::mFeatureList_aboutToChangeEditSelection( bool &ok )
{
  if ( mLayer->isEditable() && !mAttributeForm->save() )
    ok = false;
}

void QgsDualView::mFeatureList_currentEditSelectionChanged( const QgsFeature &feat )
{
  if ( !mAttributeForm )
  {
    mTempAttributeFormFeature = feat;
  }
  else if ( !mLayer->isEditable() || mAttributeForm->save() )
  {
    mAttributeForm->setFeature( feat );
    setCurrentEditSelection( QgsFeatureIds() << feat.id() );
  }
  else
  {
    // Couldn't save feature
  }
}

void QgsDualView::setCurrentEditSelection( const QgsFeatureIds &fids )
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

  mAttributeForm->setMode( enabled ? QgsAttributeEditorContext::MultiEditMode : QgsAttributeEditorContext::SingleEditMode );
}

void QgsDualView::toggleSearchMode( bool enabled )
{
  if ( enabled )
  {
    setView( AttributeEditor );
    mAttributeForm->setMode( QgsAttributeEditorContext::SearchMode );
  }
  else
  {
    mAttributeForm->setMode( QgsAttributeEditorContext::SingleEditMode );
  }
}

void QgsDualView::previewExpressionBuilder()
{
  // Show expression builder
  QgsExpressionContext context( QgsExpressionContextUtils::globalProjectLayerScopes( mLayer ) );

  QgsExpressionBuilderDialog dlg( mLayer, mFeatureList->displayExpression(), this, QStringLiteral( "generic" ), context );
  dlg.setWindowTitle( tr( "Expression Based Preview" ) );
  dlg.setExpressionText( mFeatureList->displayExpression() );

  if ( dlg.exec() == QDialog::Accepted )
  {
    mFeatureList->setDisplayExpression( dlg.expressionText() );
    mFeatureListPreviewButton->setDefaultAction( mActionExpressionPreview );
    mFeatureListPreviewButton->setPopupMode( QToolButton::MenuButtonPopup );
  }

  setDisplayExpression( mFeatureList->displayExpression() );
}

void QgsDualView::previewColumnChanged( QAction *previewAction, const QString &expression )
{
  if ( !mFeatureList->setDisplayExpression( QStringLiteral( "COALESCE( \"%1\", '<NULL>' )" ).arg( expression ) ) )
  {
    QMessageBox::warning( this,
                          tr( "Column Preview" ),
                          tr( "Could not set column '%1' as preview column.\nParser error:\n%2" )
                          .arg( previewAction->text(), mFeatureList->parserErrorString() )
                        );
  }
  else
  {
    mFeatureListPreviewButton->setText( previewAction->text() );
    mFeatureListPreviewButton->setIcon( previewAction->icon() );
    mFeatureListPreviewButton->setPopupMode( QToolButton::InstantPopup );
  }

  setDisplayExpression( mFeatureList->displayExpression() );
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
  QAction *action = qobject_cast<QAction *>( sender() );

  if ( action && action->data().isValid() && action->data().canConvert<QModelIndex>() )
  {
    QModelIndex index = action->data().toModelIndex();
    QVariant var = masterModel()->data( index, Qt::DisplayRole );
    QApplication::clipboard()->setText( var.toString() );
  }
}

void QgsDualView::cancelProgress()
{
  if ( mProgressDlg )
    mProgressDlg->cancel();
}

void QgsDualView::hideEvent( QHideEvent *event )
{
  Q_UNUSED( event )
  saveRecentDisplayExpressions();
}

void QgsDualView::viewWillShowContextMenu( QMenu *menu, const QModelIndex &atIndex )
{
  if ( !menu )
  {
    return;
  }

  QModelIndex sourceIndex = mFilterModel->mapToSource( atIndex );

  QAction *copyContentAction = new QAction( tr( "Copy Cell Content" ), this );
  copyContentAction->setData( QVariant::fromValue<QModelIndex>( sourceIndex ) );
  menu->addAction( copyContentAction );
  connect( copyContentAction, &QAction::triggered, this, &QgsDualView::copyCellContent );

  QgsVectorLayer *vl = mFilterModel->layer();
  QgsMapCanvas *canvas = mFilterModel->mapCanvas();
  if ( canvas && vl && vl->geometryType() != QgsWkbTypes::NullGeometry )
  {
    menu->addAction( tr( "Zoom to Feature" ), this, SLOT( zoomToCurrentFeature() ) );
    menu->addAction( tr( "Pan to Feature" ), this, SLOT( panToCurrentFeature() ) );
    menu->addAction( tr( "Flash Feature" ), this, SLOT( flashCurrentFeature() ) );
  }

  //add user-defined actions to context menu
  QList<QgsAction> actions = mLayer->actions()->actions( QStringLiteral( "Field" ) );
  if ( !actions.isEmpty() )
  {
    QAction *a = menu->addAction( tr( "Run Layer Action" ) );
    a->setEnabled( false );

    Q_FOREACH ( const QgsAction &action, actions )
    {
      if ( !action.runable() )
        continue;

      if ( vl && !vl->isEditable() && action.isEnabledOnlyWhenEditable() )
        continue;

      QgsAttributeTableAction *a = new QgsAttributeTableAction( action.name(), this, action.id(), sourceIndex );
#if QT_VERSION < QT_VERSION_CHECK(5, 6, 0)
      menu->addAction( action.name(), a, SLOT( execute() ) );
#else
      menu->addAction( action.name(), a, &QgsAttributeTableAction::execute );
#endif
    }
  }

  //add actions from QgsMapLayerActionRegistry to context menu
  QList<QgsMapLayerAction *> registeredActions = QgsGui::mapLayerActionRegistry()->mapLayerActions( mLayer );
  if ( !registeredActions.isEmpty() )
  {
    //add a separator between user defined and standard actions
    menu->addSeparator();

    Q_FOREACH ( QgsMapLayerAction *action, registeredActions )
    {
      QgsAttributeTableMapLayerAction *a = new QgsAttributeTableMapLayerAction( action->text(), this, action, sourceIndex );
#if QT_VERSION < QT_VERSION_CHECK(5, 6, 0)
      menu->addAction( action->text(), a, SLOT( execut() ) );
#else
      menu->addAction( action->text(), a, &QgsAttributeTableMapLayerAction::execute );
#endif
    }
  }

  menu->addSeparator();
  QgsAttributeTableAction *a = new QgsAttributeTableAction( tr( "Open Form" ), this, QString(), sourceIndex );
#if QT_VERSION < QT_VERSION_CHECK(5, 6, 0)
  menu->addAction( tr( "Open Form" ), a, SLOT( featureForm() ) );
#else
  menu->addAction( tr( "Open Form" ), a, &QgsAttributeTableAction::featureForm );
#endif
}


void QgsDualView::widgetWillShowContextMenu( QgsActionMenu *menu, const QModelIndex &atIndex )
{
  emit showContextMenuExternally( menu, mFilterModel->rowToId( atIndex ) );
}


void QgsDualView::showViewHeaderMenu( QPoint point )
{
  int col = mTableView->columnAt( point.x() );

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
  QAction *optimizeWidth = new QAction( tr( "&Autosize" ), mHorizontalHeaderMenu );
  connect( optimizeWidth, &QAction::triggered, this, &QgsDualView::autosizeColumn );
  optimizeWidth->setData( col );
  mHorizontalHeaderMenu->addAction( optimizeWidth );

  mHorizontalHeaderMenu->addSeparator();
  QAction *organize = new QAction( tr( "&Organize Columns…" ), mHorizontalHeaderMenu );
  connect( organize, &QAction::triggered, this, &QgsDualView::organizeColumns );
  mHorizontalHeaderMenu->addAction( organize );
  QAction *sort = new QAction( tr( "&Sort…" ), mHorizontalHeaderMenu );
  connect( sort, &QAction::triggered, this, &QgsDualView::modifySort );
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
    QgsAttributeTableConfig config = dialog.config();
    setAttributeTableConfig( config );
  }
}

void QgsDualView::tableColumnResized( int column, int width )
{
  QgsAttributeTableConfig config = mConfig;
  int sourceCol = config.mapVisibleColumnToIndex( column );
  if ( sourceCol >= 0 && config.columnWidth( sourceCol ) != width )
  {
    config.setColumnWidth( sourceCol, width );
    setAttributeTableConfig( config );
  }
}

void QgsDualView::hideColumn()
{
  QAction *action = qobject_cast<QAction *>( sender() );
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
  QAction *action = qobject_cast<QAction *>( sender() );
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
  QAction *action = qobject_cast<QAction *>( sender() );
  int col = action->data().toInt();
  mTableView->resizeColumnToContents( col );
}

void QgsDualView::modifySort()
{
  if ( !mLayer )
    return;

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
  QgsExpressionContext context( QgsExpressionContextUtils::globalProjectLayerScopes( mLayer ) );
  expressionBuilder->setExpressionContext( context );
  expressionBuilder->setLayer( mLayer );
  expressionBuilder->loadFieldNames();
  expressionBuilder->loadRecent( QStringLiteral( "generic" ) );
  expressionBuilder->setExpressionText( sortExpression().isEmpty() ? mLayer->displayExpression() : sortExpression() );

  sortingGroupBox->layout()->addWidget( expressionBuilder );

  QCheckBox *cbxSortAscending = new QCheckBox( tr( "Sort ascending" ) );
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
  QgsMapCanvas *canvas = mFilterModel->mapCanvas();
  if ( canvas )
  {
    canvas->zoomToFeatureIds( mLayer, ids );
  }
}

void QgsDualView::panToCurrentFeature()
{
  QModelIndex currentIndex = mTableView->currentIndex();
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
  QModelIndex currentIndex = mTableView->currentIndex();
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

  r.setFilterFids( masterModel()->layer()->selectedFeatureIds() );
  mMasterModel->setRequest( r );
  mMasterModel->loadLayer();
  emit filterChanged();
}

void QgsDualView::extentChanged()
{
  QgsFeatureRequest r = mMasterModel->request();
  if ( mFilterModel->mapCanvas() && ( r.filterType() != QgsFeatureRequest::FilterNone || !r.filterRect().isNull() ) )
  {
    QgsRectangle rect = mFilterModel->mapCanvas()->mapSettings().mapToLayerCoordinates( mLayer, mFilterModel->mapCanvas()->extent() );
    r.setFilterRect( rect );
    mMasterModel->setRequest( r );
    mMasterModel->loadLayer();
  }
  emit filterChanged();
}

void QgsDualView::featureFormAttributeChanged( const QString &attribute, const QVariant &value, bool attributeChanged )
{
  Q_UNUSED( attribute );
  Q_UNUSED( value );
  if ( attributeChanged )
    mFeatureList->setCurrentFeatureEdited( true );
}

void QgsDualView::setFilteredFeatures( const QgsFeatureIds &filteredFeatures )
{
  mFilterModel->setFilteredFeatures( filteredFeatures );
}

void QgsDualView::setRequest( const QgsFeatureRequest &request )
{
  mMasterModel->setRequest( request );
}

void QgsDualView::setFeatureSelectionManager( QgsIFeatureSelectionManager *featureSelectionManager )
{
  mTableView->setFeatureSelectionManager( featureSelectionManager );
  mFeatureList->setFeatureSelectionManager( featureSelectionManager );

  if ( mFeatureSelectionManager && mFeatureSelectionManager->parent() == this )
    delete mFeatureSelectionManager;

  mFeatureSelectionManager = featureSelectionManager;
}

void QgsDualView::setAttributeTableConfig( const QgsAttributeTableConfig &config )
{
  mConfig = config;
  mConfig.update( mLayer->fields() );
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
