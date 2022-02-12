/***************************************************************************
  QgsAttributeTableDialog.cpp
  -------------------
         date                 : Feb 2009
         copyright            : (C) 2009 by Vita Cizek
         email                : weetya (at) gmail.com

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QMessageBox>
#include <QGridLayout>
#include <QDialogButtonBox>
#include <QMenu>

#include "qgsattributetabledialog.h"
#include "qgsattributetablemodel.h"
#include "qgsattributetablefiltermodel.h"
#include "qgsattributetableview.h"
#include "qgsexpressioncontextutils.h"

#include "qgsapplication.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerutils.h"
#include "qgsvectordataprovider.h"
#include "qgsexpression.h"
#include "qgsexpressionbuilderwidget.h"
#include "qgsaddattrdialog.h"
#include "qgsdelattrdialog.h"
#include "qgsdockwidget.h"
#include "qgsfeatureiterator.h"
#include "qgssearchquerybuilder.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsproject.h"
#include "qgsfieldcalculator.h"
#include "qgsfeatureaction.h"
#include "qgsactionmanager.h"
#include "qgsmessagebar.h"
#include "qgsexpressionselectiondialog.h"
#include "qgsfeaturelistmodel.h"
#include "qgsrubberband.h"
#include "qgsfields.h"
#include "qgseditorwidgetregistry.h"
#include "qgsfieldproxymodel.h"
#include "qgsgui.h"
#include "qgsclipboard.h"
#include "qgsfeaturestore.h"
#include "qgsguiutils.h"
#include "qgsproxyprogresstask.h"
#include "qgisapp.h"
#include "qgsorganizetablecolumnsdialog.h"
#include "qgsvectorlayereditbuffer.h"
#include "qgstransactiongroup.h"

QgsExpressionContext QgsAttributeTableDialog::createExpressionContext() const
{
  QgsExpressionContext expContext;
  expContext << QgsExpressionContextUtils::globalScope()
             << QgsExpressionContextUtils::projectScope( QgsProject::instance() );

  if ( mLayer )
    expContext << QgsExpressionContextUtils::layerScope( mLayer );

  expContext.lastScope()->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "row_number" ), 1, true ) );

  expContext.setHighlightedVariables( QStringList() << QStringLiteral( "row_number" ) );

  return expContext;
}

void QgsAttributeTableDialog::updateMultiEditButtonState()
{
  if ( ! mLayer || ( mLayer->editFormConfig().layout() == QgsEditFormConfig::EditorLayout::UiFileLayout ) )
    return;

  mActionToggleMultiEdit->setEnabled( mLayer->isEditable() );

  if ( !mLayer->isEditable() || mMainView->view() != QgsDualView::AttributeEditor )
  {
    mActionToggleMultiEdit->setChecked( false );
  }
}

QgsAttributeTableDialog::QgsAttributeTableDialog( QgsVectorLayer *layer, QgsAttributeTableFilterModel::FilterMode initialMode, QWidget *parent, Qt::WindowFlags flags )
  : QDialog( parent, flags )
  , mLayer( layer )

{
  setObjectName( QStringLiteral( "QgsAttributeTableDialog/" ) + layer->id() );
  setupUi( this );
  connect( mActionCutSelectedRows, &QAction::triggered, this, &QgsAttributeTableDialog::mActionCutSelectedRows_triggered );
  connect( mActionCopySelectedRows, &QAction::triggered, this, &QgsAttributeTableDialog::mActionCopySelectedRows_triggered );
  connect( mActionPasteFeatures, &QAction::triggered, this, &QgsAttributeTableDialog::mActionPasteFeatures_triggered );
  connect( mActionToggleEditing, &QAction::toggled, this, &QgsAttributeTableDialog::mActionToggleEditing_toggled );
  connect( mActionSaveEdits, &QAction::triggered, this, &QgsAttributeTableDialog::mActionSaveEdits_triggered );
  connect( mActionReload, &QAction::triggered, this, &QgsAttributeTableDialog::mActionReload_triggered );
  connect( mActionInvertSelection, &QAction::triggered, this, &QgsAttributeTableDialog::mActionInvertSelection_triggered );
  connect( mActionRemoveSelection, &QAction::triggered, this, &QgsAttributeTableDialog::mActionRemoveSelection_triggered );
  connect( mActionSelectAll, &QAction::triggered, this, &QgsAttributeTableDialog::mActionSelectAll_triggered );
  connect( mActionZoomMapToSelectedRows, &QAction::triggered, this, &QgsAttributeTableDialog::mActionZoomMapToSelectedRows_triggered );
  connect( mActionPanMapToSelectedRows, &QAction::triggered, this, &QgsAttributeTableDialog::mActionPanMapToSelectedRows_triggered );
  connect( mActionSelectedToTop, &QAction::toggled, this, &QgsAttributeTableDialog::mActionSelectedToTop_toggled );
  connect( mActionAddAttribute, &QAction::triggered, this, &QgsAttributeTableDialog::mActionAddAttribute_triggered );
  connect( mActionRemoveAttribute, &QAction::triggered, this, &QgsAttributeTableDialog::mActionRemoveAttribute_triggered );
  connect( mActionOrganizeColumns, &QAction::triggered, this, &QgsAttributeTableDialog::mActionOrganizeColumns_triggered );
  connect( mActionOpenFieldCalculator, &QAction::triggered, this, &QgsAttributeTableDialog::mActionOpenFieldCalculator_triggered );
  connect( mActionDeleteSelected, &QAction::triggered, this, &QgsAttributeTableDialog::mActionDeleteSelected_triggered );
  connect( mMainView, &QgsDualView::currentChanged, this, &QgsAttributeTableDialog::mMainView_currentChanged );
  connect( mActionAddFeature, &QAction::triggered, this, &QgsAttributeTableDialog::mActionAddFeature_triggered );
  connect( mActionAddFeatureViaAttributeTable, &QAction::triggered, this, &QgsAttributeTableDialog::mActionAddFeatureViaAttributeTable_triggered );
  connect( mActionAddFeatureViaAttributeForm, &QAction::triggered, this, &QgsAttributeTableDialog::mActionAddFeatureViaAttributeForm_triggered );
  connect( mActionExpressionSelect, &QAction::triggered, this, &QgsAttributeTableDialog::mActionExpressionSelect_triggered );
  connect( mMainView, &QgsDualView::showContextMenuExternally, this, &QgsAttributeTableDialog::showContextMenu );

  mActionSelectAll->setShortcuts( QKeySequence::SelectAll );
  mActionSelectAll->setShortcutContext( Qt::WidgetWithChildrenShortcut );
  mMainView->addAction( mActionSelectAll );
  mActionCopySelectedRows->setShortcuts( QKeySequence::Copy );
  mActionCopySelectedRows->setShortcutContext( Qt::WidgetWithChildrenShortcut );
  mMainView->addAction( mActionCopySelectedRows );
  mActionCutSelectedRows->setShortcuts( QKeySequence::Cut );
  mActionCutSelectedRows->setShortcutContext( Qt::WidgetWithChildrenShortcut );
  mMainView->addAction( mActionCutSelectedRows );
  mActionPasteFeatures->setShortcuts( QKeySequence::Paste );
  mActionPasteFeatures->setShortcutContext( Qt::WidgetWithChildrenShortcut );
  mMainView->addAction( mActionPasteFeatures );

  QgsSettings settings;

  mActionAddFeature->setMenu( new QMenu( mActionAddFeature->parentWidget() ) );
  mActionAddFeature->menu()->addAction( mActionAddFeatureViaAttributeTable );
  mActionAddFeature->menu()->addAction( mActionAddFeatureViaAttributeForm );
  mActionAddFeature->setIcon(
    settings.value( QStringLiteral( "/qgis/attributeTableLastAddFeatureMethod" ) ) == QStringLiteral( "attributeForm" )
    ? mActionAddFeatureViaAttributeForm->icon()
    : mActionAddFeatureViaAttributeTable->icon() );

  const QgsFields fields = mLayer->fields();
  for ( const QgsField &field : fields )
  {
    mVisibleFields.append( field.name() );
  }

  // Fix selection color on losing focus (Windows)
  setStyleSheet( QgisApp::instance()->styleSheet() );

  setAttribute( Qt::WA_DeleteOnClose );

  layout()->setContentsMargins( 0, 0, 0, 0 );
  static_cast< QGridLayout * >( layout() )->setVerticalSpacing( 0 );

  int size = settings.value( QStringLiteral( "/qgis/iconSize" ), 16 ).toInt();
  if ( size > 32 )
  {
    size -= 16;
  }
  else if ( size == 32 )
  {
    size = 24;
  }
  else
  {
    size = 16;
  }
  mToolbar->setIconSize( QSize( size, size ) );

  // Initialize the window geometry
  restoreGeometry( settings.value( QStringLiteral( "Windows/BetterAttributeTable/geometry" ) ).toByteArray() );

  QgsDistanceArea da;
  da.setSourceCrs( mLayer->crs(), QgsProject::instance()->transformContext() );
  da.setEllipsoid( QgsProject::instance()->ellipsoid() );

  QgsAttributeEditorContext editorContext = QgisApp::instance()->createAttributeEditorContext();
  editorContext.setDistanceArea( da );

  QgsFeatureRequest r;
  bool needsGeom = false;
  if ( mLayer->geometryType() != QgsWkbTypes::NullGeometry &&
       initialMode == QgsAttributeTableFilterModel::ShowVisible )
  {
    QgsMapCanvas *mc = QgisApp::instance()->mapCanvas();
    QgsRectangle extent( mc->mapSettings().mapToLayerCoordinates( layer, mc->extent() ) );
    r.setFilterRect( extent );
    needsGeom = true;
  }
  else if ( initialMode == QgsAttributeTableFilterModel::ShowSelected )
  {
    r.setFilterFids( layer->selectedFeatureIds() );
  }
  else if ( initialMode == QgsAttributeTableFilterModel::ShowEdited )
  {
    r.setFilterFids( layer->editBuffer() ? layer->editBuffer()->allAddedOrEditedFeatures() : QgsFeatureIds() );
  }
  if ( !needsGeom )
    r.setFlags( QgsFeatureRequest::NoGeometry );

  // Initialize dual view
  mMainView->init( mLayer, QgisApp::instance()->mapCanvas(), r, editorContext, false );

  QgsAttributeTableConfig config = mLayer->attributeTableConfig();
  mMainView->setAttributeTableConfig( config );

  mFeatureFilterWidget->init( mLayer, editorContext, mMainView, QgisApp::instance()->messageBar(), QgsMessageBar::defaultMessageTimeout() );

  mActionFeatureActions = new QToolButton();
  mActionFeatureActions->setAutoRaise( false );
  mActionFeatureActions->setPopupMode( QToolButton::InstantPopup );
  mActionFeatureActions->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mAction.svg" ) ) );
  mActionFeatureActions->setText( tr( "Actions" ) );
  mActionFeatureActions->setToolTip( tr( "Actions" ) );

  mToolbar->addWidget( mActionFeatureActions );

  connect( mActionSetStyles, &QAction::triggered, this, &QgsAttributeTableDialog::openConditionalStyles );

  // info from layer to table
  connect( mLayer, &QgsVectorLayer::editingStarted, this, &QgsAttributeTableDialog::editingToggled );
  connect( mLayer, &QgsVectorLayer::editingStopped, this, &QgsAttributeTableDialog::editingToggled );
  connect( mLayer, &QObject::destroyed, mMainView, &QgsDualView::cancelProgress );
  connect( mLayer, &QgsVectorLayer::selectionChanged, this, &QgsAttributeTableDialog::updateTitle );
  connect( mLayer, &QgsVectorLayer::featureAdded, this, &QgsAttributeTableDialog::updateTitle );
  connect( mLayer, &QgsVectorLayer::featuresDeleted, this, &QgsAttributeTableDialog::updateTitle );
  connect( mLayer, &QgsVectorLayer::editingStopped, this, &QgsAttributeTableDialog::updateTitle );
  connect( mLayer, &QgsVectorLayer::readOnlyChanged, this, &QgsAttributeTableDialog::editingToggled );
  connect( mLayer, &QgsVectorLayer::layerModified, this, &QgsAttributeTableDialog::updateLayerModifiedActions );

  // When transaction group is enabled, collect related layers and connect modified actions to enable save action
  const auto relations { QgsProject::instance()->relationManager()->referencedRelations( mLayer ) };
  const auto transactionGroups = QgsProject::instance()->transactionGroups();
  for ( const auto &relation : std::as_const( relations ) )
  {
    for ( auto it = transactionGroups.constBegin(); it != transactionGroups.constEnd(); ++it )
    {
      if ( relation.isValid() && it.value()->layers().contains( { mLayer, relation.referencingLayer() } ) )
      {
        mReferencingLayers.push_back( relation.referencingLayer() );
        connect( relation.referencingLayer(), &QgsVectorLayer::layerModified, this, &QgsAttributeTableDialog::updateLayerModifiedActions );
      }
    }
  }

  // connect table info to window
  connect( mMainView, &QgsDualView::filterChanged, this, &QgsAttributeTableDialog::updateTitle );
  connect( mMainView, &QgsDualView::filterExpressionSet, this, &QgsAttributeTableDialog::formFilterSet );
  connect( mMainView, &QgsDualView::formModeChanged, this, &QgsAttributeTableDialog::viewModeChanged );

  // info from table to application
  connect( this, &QgsAttributeTableDialog::saveEdits, this, [ = ] { QgisApp::instance()->saveEdits(); } );

  const bool dockTable = settings.value( QStringLiteral( "qgis/dockAttributeTable" ), false ).toBool();
  if ( dockTable )
  {
    mDock = new QgsAttributeTableDock( QString(), QgisApp::instance() );
    mDock->setWidget( this );
    connect( this, &QObject::destroyed, mDock, &QWidget::close );
    QgisApp::instance()->addTabifiedDockWidget( Qt::BottomDockWidgetArea, mDock, QStringList(), true );
  }
  mActionDockUndock->setChecked( dockTable );
  connect( mActionDockUndock, &QAction::toggled, this, &QgsAttributeTableDialog::toggleDockMode );
  installEventFilter( this );

  updateTitle();

  mActionRemoveSelection->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionDeselectActiveLayer.svg" ) ) );
  mActionSelectAll->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionSelectAll.svg" ) ) );
  mActionSelectedToTop->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionSelectedToTop.svg" ) ) );
  mActionCopySelectedRows->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionEditCopy.svg" ) ) );
  mActionPasteFeatures->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionEditPaste.svg" ) ) );
  mActionZoomMapToSelectedRows->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionZoomToSelected.svg" ) ) );
  mActionPanMapToSelectedRows->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionPanToSelected.svg" ) ) );
  mActionInvertSelection->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionInvertSelection.svg" ) ) );
  mActionToggleEditing->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionToggleEditing.svg" ) ) );
  mActionSaveEdits->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionSaveEdits.svg" ) ) );
  mActionDeleteSelected->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionDeleteSelectedFeatures.svg" ) ) );
  mActionOpenFieldCalculator->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionCalculateField.svg" ) ) );
  mActionAddAttribute->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionNewAttribute.svg" ) ) );
  mActionRemoveAttribute->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionDeleteAttribute.svg" ) ) );
  mTableViewButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionOpenTable.svg" ) ) );
  mAttributeViewButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionFormView.svg" ) ) );
  mActionExpressionSelect->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mIconExpressionSelect.svg" ) ) );
  mActionAddFeature->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionNewTableRow.svg" ) ) );
  mActionFeatureActions->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mAction.svg" ) ) );

  // toggle editing
  QgsVectorDataProvider::Capabilities capabilities = mLayer->dataProvider()->capabilities();
  bool canChangeAttributes = capabilities & QgsVectorDataProvider::ChangeAttributeValues;
  bool canDeleteFeatures = capabilities & QgsVectorDataProvider::DeleteFeatures;
  bool canAddAttributes = capabilities & QgsVectorDataProvider::AddAttributes;
  bool canDeleteAttributes = capabilities & QgsVectorDataProvider::DeleteAttributes;
  bool canAddFeatures = capabilities & QgsVectorDataProvider::AddFeatures;
  bool canReload = capabilities & QgsVectorDataProvider::ReloadData;

  mActionToggleEditing->blockSignals( true );
  mActionToggleEditing->setCheckable( true );
  mActionToggleEditing->setChecked( mLayer->isEditable() );
  mActionToggleEditing->blockSignals( false );

  mActionSaveEdits->setEnabled( mActionToggleEditing->isEnabled() && mLayer->isEditable() && mLayer->isModified() );
  mActionReload->setEnabled( ! mLayer->isEditable() );
  mActionAddAttribute->setEnabled( ( canChangeAttributes || canAddAttributes ) && mLayer->isEditable() );
  mActionRemoveAttribute->setEnabled( canDeleteAttributes && mLayer->isEditable() );
  if ( !canDeleteFeatures )
  {
    mToolbar->removeAction( mActionDeleteSelected );
    mToolbar->removeAction( mActionCutSelectedRows );
  }
  mActionAddFeature->setEnabled( canAddFeatures && mLayer->isEditable() );
  mActionPasteFeatures->setEnabled( canAddFeatures && mLayer->isEditable() );
  if ( !canAddFeatures )
  {
    mToolbar->removeAction( mActionAddFeature );
    mToolbar->removeAction( mActionPasteFeatures );
  }

  if ( !canReload )
    mToolbar->removeAction( mActionReload );

  mMainViewButtonGroup->setId( mTableViewButton, QgsDualView::AttributeTable );
  mMainViewButtonGroup->setId( mAttributeViewButton, QgsDualView::AttributeEditor );

  switch ( initialMode )
  {
    case QgsAttributeTableFilterModel::ShowVisible:
      mFeatureFilterWidget->filterVisible();
      break;

    case QgsAttributeTableFilterModel::ShowSelected:
      mFeatureFilterWidget->filterSelected();
      break;

    case QgsAttributeTableFilterModel::ShowEdited:
      mFeatureFilterWidget->filterEdited();
      break;

    case QgsAttributeTableFilterModel::ShowAll:
    default:
      mFeatureFilterWidget->filterShowAll();
      break;
  }

  // Layer might have been destroyed while loading!
  if ( mLayer )
  {
    mUpdateExpressionText->registerExpressionContextGenerator( this );
    mFieldCombo->setFilters( QgsFieldProxyModel::AllTypes | QgsFieldProxyModel::HideReadOnly );
    mFieldCombo->setLayer( mLayer );

    connect( mRunFieldCalc, &QAbstractButton::clicked, this, &QgsAttributeTableDialog::updateFieldFromExpression );
    connect( mRunFieldCalcSelected, &QAbstractButton::clicked, this, &QgsAttributeTableDialog::updateFieldFromExpressionSelected );
    // NW TODO Fix in 2.6 - Doesn't work with field model for some reason.
    //  connect( mUpdateExpressionText, SIGNAL( returnPressed() ), this, SLOT( updateFieldFromExpression() ) );
    connect( mUpdateExpressionText, static_cast < void ( QgsFieldExpressionWidget::* )( const QString &, bool ) > ( &QgsFieldExpressionWidget::fieldChanged ), this, &QgsAttributeTableDialog::updateButtonStatus );
    mUpdateExpressionText->setLayer( mLayer );
    mUpdateExpressionText->setLeftHandButtonStyle( true );

    int initialView = settings.value( QStringLiteral( "qgis/attributeTableView" ), -1 ).toInt();
    if ( initialView < 0 )
    {
      initialView = settings.value( QStringLiteral( "qgis/attributeTableLastView" ), QgsDualView::AttributeTable ).toInt();
    }
    mMainView->setView( static_cast< QgsDualView::ViewMode >( initialView ) );
    mMainViewButtonGroup->button( initialView )->setChecked( true );

    connect( mActionToggleMultiEdit, &QAction::toggled, mMainView, &QgsDualView::setMultiEditEnabled );
    connect( mActionSearchForm, &QAction::toggled, mMainView, &QgsDualView::toggleSearchMode );
    updateMultiEditButtonState();

    if ( mLayer->editFormConfig().layout() == QgsEditFormConfig::EditorLayout::UiFileLayout )
    {
      //not supported with custom UI
      mActionToggleMultiEdit->setEnabled( false );
      mActionToggleMultiEdit->setToolTip( tr( "Multiedit is not supported when using custom UI forms" ) );
      mActionSearchForm->setEnabled( false );
      mActionSearchForm->setToolTip( tr( "Search is not supported when using custom UI forms" ) );
    }

    editingToggled();
    // Close and delete if the layer has been destroyed
    connect( mLayer, &QObject::destroyed, this, &QWidget::close );
  }
  else
  {
    QWidget::close();
  }
}

void QgsAttributeTableDialog::updateTitle()
{
  if ( ! mLayer )
  {
    return;
  }
  QWidget *w = mDock ? qobject_cast<QWidget *>( mDock )
               : mDialog ? qobject_cast<QWidget *>( mDialog )
               : qobject_cast<QWidget *>( this );
  w->setWindowTitle( tr( " %1 — Features Total: %L2, Filtered: %L3, Selected: %L4" )
                     .arg( mLayer->name() )
                     .arg( std::max( static_cast< long long >( mMainView->featureCount() ), mLayer->featureCount() ) ) // layer count may be estimated, so use larger of the two
                     .arg( mMainView->filteredFeatureCount() )
                     .arg( mLayer->selectedFeatureCount() )
                   );

  if ( mMainView->filterMode() == QgsAttributeTableFilterModel::ShowAll )
    mRunFieldCalc->setText( tr( "Update All" ) );
  else
    mRunFieldCalc->setText( tr( "Update Filtered" ) );

  bool canDeleteFeatures = mLayer->dataProvider()->capabilities() & QgsVectorDataProvider::DeleteFeatures;
  bool enabled = mLayer->selectedFeatureCount() > 0;
  mRunFieldCalcSelected->setEnabled( enabled );
  mActionDeleteSelected->setEnabled( canDeleteFeatures && mLayer->isEditable() && enabled );
  mActionCutSelectedRows->setEnabled( canDeleteFeatures && mLayer->isEditable() && enabled );
  mActionCopySelectedRows->setEnabled( enabled );
}

void QgsAttributeTableDialog::updateButtonStatus( const QString &fieldName, bool isValid )
{
  Q_UNUSED( fieldName )
  mRunFieldCalc->setEnabled( isValid );
}

void QgsAttributeTableDialog::keyPressEvent( QKeyEvent *event )
{
  QDialog::keyPressEvent( event );

  if ( ( event->key() == Qt::Key_Backspace || event->key() == Qt::Key_Delete ) && mActionDeleteSelected->isEnabled() )
  {
    QgisApp::instance()->deleteSelected( mLayer, this );
  }
}

bool QgsAttributeTableDialog::eventFilter( QObject *object, QEvent *ev )
{
  if ( ev->type() == QEvent::Close && !mDock && ( !mDialog || mDialog == object ) )
  {
    if ( QWidget *w = qobject_cast< QWidget * >( object ) )
    {
      QgsSettings settings;
      settings.setValue( QStringLiteral( "Windows/BetterAttributeTable/geometry" ), w->saveGeometry() );
    }
  }

  return QDialog::eventFilter( object, ev );
}

void QgsAttributeTableDialog::updateFieldFromExpression()
{
  bool filtered = mMainView->filterMode() != QgsAttributeTableFilterModel::ShowAll;
  QgsFeatureIds filteredIds = filtered ? mMainView->filteredFeatures() : QgsFeatureIds();
  runFieldCalculation( mLayer, mFieldCombo->currentField(), mUpdateExpressionText->asExpression(), filteredIds );
}

void QgsAttributeTableDialog::updateFieldFromExpressionSelected()
{
  QgsFeatureIds filteredIds = mLayer->selectedFeatureIds();
  runFieldCalculation( mLayer, mFieldCombo->currentField(), mUpdateExpressionText->asExpression(), filteredIds );
}

void QgsAttributeTableDialog::viewModeChanged( QgsAttributeEditorContext::Mode mode )
{
  if ( mode != QgsAttributeEditorContext::SearchMode )
    mActionSearchForm->setChecked( false );
}

void QgsAttributeTableDialog::formFilterSet( const QString &filter, QgsAttributeForm::FilterType type )
{
  setFilterExpression( filter, type, true );
}

void QgsAttributeTableDialog::runFieldCalculation( QgsVectorLayer *layer, const QString &fieldName, const QString &expression, const QgsFeatureIds &filteredIds )
{
  int fieldindex = layer->fields().indexFromName( fieldName );
  if ( fieldindex < 0 )
  {
    // this shouldn't happen... but it did. There's probably some deeper underlying issue
    // but we may as well play it safe here.
    QMessageBox::critical( nullptr, tr( "Update Attributes" ), tr( "An error occurred while trying to update the field %1" ).arg( fieldName ) );
    return;
  }

  QgsTemporaryCursorOverride cursorOverride( Qt::WaitCursor );
  mLayer->beginEditCommand( QStringLiteral( "Field calculator" ) );

  bool calculationSuccess = true;
  QString error;

  QgsExpression exp( expression );
  QgsDistanceArea da;
  da.setSourceCrs( mLayer->crs(), QgsProject::instance()->transformContext() );
  da.setEllipsoid( QgsProject::instance()->ellipsoid() );
  exp.setGeomCalculator( &da );
  exp.setDistanceUnits( QgsProject::instance()->distanceUnits() );
  exp.setAreaUnits( QgsProject::instance()->areaUnits() );
  bool useGeometry = exp.needsGeometry();

  QgsFeatureRequest request( mMainView->masterModel()->request() );
  useGeometry |= !( request.spatialFilterType() == Qgis::SpatialFilterType::NoFilter );
  request.setFlags( useGeometry ? QgsFeatureRequest::NoFlags : QgsFeatureRequest::NoGeometry );

  int rownum = 1;

  QgsExpressionContext context( QgsExpressionContextUtils::globalProjectLayerScopes( layer ) );
  exp.prepare( &context );

  QgsField fld = layer->fields().at( fieldindex );

  QSet< QString >referencedColumns = exp.referencedColumns();
  referencedColumns.insert( fld.name() ); // need existing column value to store old attribute when changing field values
  request.setSubsetOfAttributes( referencedColumns, layer->fields() );

  //go through all the features and change the new attributes
  QgsFeatureIterator fit = layer->getFeatures( request );

  std::unique_ptr< QgsScopedProxyProgressTask > task = std::make_unique< QgsScopedProxyProgressTask >( tr( "Calculating field" ) );

  long long count = !filteredIds.isEmpty() ? filteredIds.size() : layer->featureCount();
  long long i = 0;

  QgsFeature feature;
  while ( fit.nextFeature( feature ) )
  {
    if ( !filteredIds.isEmpty() && !filteredIds.contains( feature.id() ) )
    {
      continue;
    }

    i++;
    task->setProgress( i / static_cast< double >( count ) * 100 );

    context.setFeature( feature );
    context.lastScope()->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "row_number" ), rownum, true ) );

    QVariant value = exp.evaluate( &context );
    ( void )fld.convertCompatible( value );
    // Bail if we have a update error
    if ( exp.hasEvalError() )
    {
      calculationSuccess = false;
      error = exp.evalErrorString();
      break;
    }
    else
    {
      QVariant oldvalue = feature.attributes().value( fieldindex );
      mLayer->changeAttributeValue( feature.id(), fieldindex, value, oldvalue );
    }

    rownum++;
  }

  cursorOverride.release();
  task.reset();

  if ( !calculationSuccess )
  {
    QMessageBox::critical( nullptr, tr( "Update Attributes" ), tr( "An error occurred while evaluating the calculation string:\n%1" ).arg( error ) );
    mLayer->destroyEditCommand();
  }
  else
  {
    mLayer->endEditCommand();

    // refresh table with updated values
    // fixes https://github.com/qgis/QGIS/issues/25210
    QgsAttributeTableModel *masterModel = mMainView->masterModel();
    int modelColumn = masterModel->fieldCol( fieldindex );
    masterModel->reload( masterModel->index( 0, modelColumn ), masterModel->index( masterModel->rowCount() - 1, modelColumn ) );
  }
}

void QgsAttributeTableDialog::layerActionTriggered()
{
  QAction *qAction = qobject_cast<QAction *>( sender() );
  Q_ASSERT( qAction );

  QgsAction action = qAction->data().value<QgsAction>();

  QgsExpressionContext context = mLayer->createExpressionContext();
  QgsExpressionContextScope *scope = new QgsExpressionContextScope();
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "action_scope" ), "AttributeTable" ) );
  context.appendScope( scope );
  action.run( context );
}


void QgsAttributeTableDialog::mActionSelectedToTop_toggled( bool checked )
{
  if ( checked )
  {
    mMainView->setSelectedOnTop( true );
  }
  else
  {
    mMainView->setSelectedOnTop( false );
  }
}

void QgsAttributeTableDialog::mActionOpenFieldCalculator_triggered()
{
  QgsAttributeTableModel *masterModel = mMainView->masterModel();

  QgsFieldCalculator calc( mLayer, this );
  if ( calc.exec() == QDialog::Accepted )
  {
    int col = masterModel->fieldCol( calc.changedAttributeId() );

    if ( col >= 0 )
    {
      masterModel->reload( masterModel->index( 0, col ), masterModel->index( masterModel->rowCount() - 1, col ) );
    }
  }
}

void QgsAttributeTableDialog::mActionSaveEdits_triggered()
{
  QgisApp::instance()->saveEdits( mLayer, true, true );
  for ( const auto &referencingLayer : std::as_const( mReferencingLayers ) )
  {
    if ( referencingLayer )
    {
      QgisApp::instance()->saveEdits( referencingLayer, true, true );
    }
  }
}

void QgsAttributeTableDialog::mActionReload_triggered()
{
  mMainView->masterModel()->layer()->reload();
}

void QgsAttributeTableDialog::mActionAddFeature_triggered()
{
  QgsSettings s;

  if ( s.value( QStringLiteral( "/qgis/attributeTableLastAddFeatureMethod" ) ) == QLatin1String( "attributeForm" ) )
    mActionAddFeatureViaAttributeForm_triggered();
  else
    mActionAddFeatureViaAttributeTable_triggered();
}

void QgsAttributeTableDialog::mActionAddFeatureViaAttributeTable_triggered()
{
  if ( !mLayer->isEditable() )
    return;

  QgsSettings s;
  s.setValue( QStringLiteral( "/qgis/attributeTableLastAddFeatureMethod" ), QStringLiteral( "attributeTable" ) );
  mActionAddFeature->setIcon( mActionAddFeatureViaAttributeTable->icon() );

  QgsAttributeTableModel *masterModel = mMainView->masterModel();

  QgsFeature f;
  QgsFeatureAction action( tr( "Geometryless feature added" ), f, mLayer, QString(), -1, this );
  action.setForceSuppressFormPopup( true ); // we're already showing the table, allowing users to enter the new feature's attributes directly
  if ( action.addFeature() )
  {
    masterModel->reload( masterModel->index( 0, 0 ), masterModel->index( masterModel->rowCount() - 1, masterModel->columnCount() - 1 ) );
    mMainView->setCurrentEditSelection( QgsFeatureIds() << action.feature().id() );
    mMainView->tableView()->scrollToFeature( action.feature().id(), 0 );
  }
}

void QgsAttributeTableDialog::mActionAddFeatureViaAttributeForm_triggered()
{
  if ( !mLayer->isEditable() )
    return;

  QgsSettings s;
  s.setValue( QStringLiteral( "/qgis/attributeTableLastAddFeatureMethod" ), QStringLiteral( "attributeForm" ) );
  mActionAddFeature->setIcon( mActionAddFeatureViaAttributeForm->icon() );

  QgsFeature f;

  QgsFeatureAction action( tr( "Feature Added" ), f, mLayer, QString(), -1, this );
  QgsAttributeTableModel *masterModel = mMainView->masterModel();

  if ( action.addFeature() )
  {
    masterModel->reload( masterModel->index( 0, 0 ), masterModel->index( masterModel->rowCount() - 1, masterModel->columnCount() - 1 ) );
    mMainView->setCurrentEditSelection( QgsFeatureIds() << action.feature().id() );
    mMainView->tableView()->scrollToFeature( action.feature().id(), 0 );
  }
}


void QgsAttributeTableDialog::mActionExpressionSelect_triggered()
{
  QgsExpressionSelectionDialog *dlg = new QgsExpressionSelectionDialog( mLayer );
  dlg->setMessageBar( QgisApp::instance()->messageBar() );
  dlg->setAttribute( Qt::WA_DeleteOnClose );
  dlg->show();
}

void QgsAttributeTableDialog::mActionCutSelectedRows_triggered()
{
  QgisApp::instance()->cutSelectionToClipboard( mLayer );
}

void QgsAttributeTableDialog::mActionCopySelectedRows_triggered()
{
  if ( mMainView->view() == QgsDualView::AttributeTable )
  {
    const QList<QgsFeatureId> featureIds = mMainView->tableView()->selectedFeaturesIds();
    QgsFeatureStore featureStore;
    QgsFields fields = QgsFields( mLayer->fields() );
    QStringList fieldNames;

    const auto configs = mMainView->attributeTableConfig().columns();
    for ( const QgsAttributeTableConfig::ColumnConfig &columnConfig : configs )
    {
      if ( columnConfig.hidden )
      {
        int fieldIndex = fields.lookupField( columnConfig.name );
        fields.remove( fieldIndex );
        continue;
      }
      fieldNames << columnConfig.name;
    }
    featureStore.setFields( fields );

    QgsFeatureIterator it = mLayer->getFeatures( QgsFeatureRequest( qgis::listToSet( featureIds ) )
                            .setSubsetOfAttributes( fieldNames, mLayer->fields() ) );
    QgsFeatureMap featureMap;
    QgsFeature feature;
    while ( it.nextFeature( feature ) )
    {
      QgsVectorLayerUtils::matchAttributesToFields( feature, fields );
      featureMap[feature.id()] = feature;
    }
    for ( const QgsFeatureId &id : featureIds )
    {
      featureStore.addFeature( featureMap[id] );
    }

    featureStore.setCrs( mLayer->crs() );

    QgisApp::instance()->clipboard()->replaceWithCopyOf( featureStore );
  }
  else
  {
    QgisApp::instance()->copySelectionToClipboard( mLayer );
  }
}

void QgsAttributeTableDialog::mActionPasteFeatures_triggered()
{
  QgisApp::instance()->pasteFromClipboard( mLayer );
}


void QgsAttributeTableDialog::mActionZoomMapToSelectedRows_triggered()
{
  QgisApp::instance()->mapCanvas()->zoomToSelected( mLayer );
}

void QgsAttributeTableDialog::mActionPanMapToSelectedRows_triggered()
{
  QgisApp::instance()->mapCanvas()->panToSelected( mLayer );
}

void QgsAttributeTableDialog::mActionInvertSelection_triggered()
{
  mLayer->invertSelection();
}

void QgsAttributeTableDialog::mActionRemoveSelection_triggered()
{
  mLayer->removeSelection();
}

void QgsAttributeTableDialog::mActionSelectAll_triggered()
{
  mLayer->selectAll();
}

void QgsAttributeTableDialog::mActionDeleteSelected_triggered()
{
  QgisApp::instance()->deleteSelected( mLayer, this );
}

void QgsAttributeTableDialog::mMainView_currentChanged( int viewMode )
{
  mMainViewButtonGroup->button( viewMode )->click();
  updateMultiEditButtonState();

  if ( viewMode == 0 )
    mActionSearchForm->setChecked( false );

  QgsSettings s;
  s.setValue( QStringLiteral( "/qgis/attributeTableLastView" ), static_cast< int >( viewMode ) );
}

void QgsAttributeTableDialog::mActionToggleEditing_toggled( bool )
{
  if ( !mLayer )
    return;

  //this has to be done, because in case only one cell has been changed and is still enabled, the change
  //would not be added to the mEditBuffer. By disabling, it looses focus and the change will be stored.
  if ( mLayer->isEditable() && mMainView->tableView()->indexWidget( mMainView->tableView()->currentIndex() ) )
    mMainView->tableView()->indexWidget( mMainView->tableView()->currentIndex() )->setEnabled( false );

  if ( !QgisApp::instance()->toggleEditing( mLayer ) )
  {
    // restore gui state if toggling was canceled or layer commit/rollback failed
    editingToggled();
  }
}

void QgsAttributeTableDialog::editingToggled()
{
  mActionToggleEditing->blockSignals( true );
  mActionToggleEditing->setChecked( mLayer->isEditable() );
  mActionSaveEdits->setEnabled( mLayer->isEditable() && mLayer->isModified() );
  mActionReload->setEnabled( ! mLayer->isEditable() );
  updateMultiEditButtonState();
  if ( mLayer->isEditable() )
  {
    mActionSearchForm->setChecked( false );
  }
  mActionToggleEditing->blockSignals( false );

  bool canChangeAttributes = mLayer->dataProvider()->capabilities() & QgsVectorDataProvider::ChangeAttributeValues;
  bool canDeleteFeatures = mLayer->dataProvider()->capabilities() & QgsVectorDataProvider::DeleteFeatures;
  bool canAddAttributes = mLayer->dataProvider()->capabilities() & QgsVectorDataProvider::AddAttributes;
  bool canDeleteAttributes = mLayer->dataProvider()->capabilities() & QgsVectorDataProvider::DeleteAttributes;
  bool canAddFeatures = mLayer->dataProvider()->capabilities() & QgsVectorDataProvider::AddFeatures;
  mActionAddAttribute->setEnabled( ( canChangeAttributes || canAddAttributes ) && mLayer->isEditable() );
  mActionRemoveAttribute->setEnabled( canDeleteAttributes && mLayer->isEditable() );
  mActionDeleteSelected->setEnabled( canDeleteFeatures && mLayer->isEditable() && mLayer->selectedFeatureCount() > 0 );
  mActionCutSelectedRows->setEnabled( canDeleteFeatures && mLayer->isEditable() && mLayer->selectedFeatureCount() > 0 );
  mActionAddFeature->setEnabled( canAddFeatures && mLayer->isEditable() );
  mActionPasteFeatures->setEnabled( canAddFeatures && mLayer->isEditable() );
  mActionToggleEditing->setEnabled( ( canChangeAttributes || canDeleteFeatures || canAddAttributes || canDeleteAttributes || canAddFeatures ) && !mLayer->readOnly() );

  mUpdateExpressionBox->setVisible( mLayer->isEditable() );
  if ( mLayer->isEditable() && mFieldCombo->currentIndex() == -1 )
  {
    mFieldCombo->setCurrentIndex( 0 );
  }
  // not necessary to set table read only if layer is not editable
  // because model always reflects actual state when returning item flags

  QList<QgsAction> actions = mLayer->actions()->actions( QStringLiteral( "Layer" ) );

  if ( actions.isEmpty() )
  {
    mActionFeatureActions->setVisible( false );
  }
  else
  {
    QMenu *actionMenu = new QMenu();
    const auto constActions = actions;
    for ( const QgsAction &action : constActions )
    {
      if ( !mLayer->isEditable() && action.isEnabledOnlyWhenEditable() )
        continue;

      QAction *qAction = actionMenu->addAction( action.icon(), action.shortTitle() );
      qAction->setToolTip( action.name() );
      qAction->setData( QVariant::fromValue<QgsAction>( action ) );
      connect( qAction, &QAction::triggered, this, &QgsAttributeTableDialog::layerActionTriggered );
    }
    mActionFeatureActions->setMenu( actionMenu );
  }

}

void QgsAttributeTableDialog::mActionAddAttribute_triggered()
{
  if ( !mLayer )
  {
    return;
  }

  QgsAttributeTableModel *masterModel = mMainView->masterModel();

  QgsAddAttrDialog dialog( mLayer, this );
  if ( dialog.exec() == QDialog::Accepted )
  {
    mLayer->beginEditCommand( tr( "Attribute added" ) );
    if ( mLayer->addAttribute( dialog.field() ) )
    {
      mLayer->endEditCommand();

      if ( mLayer->displayExpression().isEmpty() )
      {
        mLayer->setDisplayExpression( dialog.field().name() );
      }
    }
    else
    {
      mLayer->destroyEditCommand();
      QMessageBox::critical( this, tr( "Add Field" ), tr( "Failed to add field '%1' of type '%2'. Is the field name unique?" ).arg( dialog.field().name(), dialog.field().typeName() ) );
    }

    // update model - a field has been added or updated
    masterModel->reload( masterModel->index( 0, 0 ), masterModel->index( masterModel->rowCount() - 1, masterModel->columnCount() - 1 ) );
  }
}

void QgsAttributeTableDialog::mActionRemoveAttribute_triggered()
{
  if ( !mLayer )
  {
    return;
  }

  QgsDelAttrDialog dialog( mLayer );
  if ( dialog.exec() == QDialog::Accepted )
  {
    QList<int> attributes = dialog.selectedAttributes();
    if ( attributes.empty() )
    {
      return;
    }

    // check whether display expression is a single field
    int fieldIdx = QgsExpression::expressionToLayerFieldIndex( mLayer->displayExpression(), mLayer );
    QgsAttributeTableModel *masterModel = mMainView->masterModel();

    mLayer->beginEditCommand( tr( "Deleted attribute" ) );
    if ( mLayer->deleteAttributes( attributes ) )
    {
      mLayer->endEditCommand();

      if ( fieldIdx != -1 && attributes.contains( fieldIdx ) )
        mLayer->setDisplayExpression( mLayer->fields().count() > 0 ? mLayer->fields().at( 0 ).name() : QString() );
    }
    else
    {
      QgisApp::instance()->messageBar()->pushMessage( tr( "Attribute error" ), tr( "The attribute(s) could not be deleted" ), Qgis::MessageLevel::Warning );
      mLayer->destroyEditCommand();
    }
    // update model - a field has been added or updated
    masterModel->reload( masterModel->index( 0, 0 ), masterModel->index( masterModel->rowCount() - 1, masterModel->columnCount() - 1 ) );
  }
}

void QgsAttributeTableDialog::mActionOrganizeColumns_triggered()
{
  if ( !mLayer )
  {
    return;
  }

  QgsOrganizeTableColumnsDialog dlg( mLayer, mLayer->attributeTableConfig(), this );
  if ( dlg.exec() == QDialog::Accepted )
  {
    QgsAttributeTableConfig config = dlg.config();
    mMainView->setAttributeTableConfig( config );
  }
}

void QgsAttributeTableDialog::openConditionalStyles()
{
  mMainView->openConditionalStyles();
}

void QgsAttributeTableDialog::setFilterExpression( const QString &filterString, QgsAttributeForm::FilterType type,
    bool alwaysShowFilter )
{
  mFeatureFilterWidget->setFilterExpression( filterString, type, alwaysShowFilter );
}

void QgsAttributeTableDialog::setView( QgsDualView::ViewMode mode )
{
  mMainView->setView( mode );
}

void QgsAttributeTableDialog::deleteFeature( const QgsFeatureId fid )
{
  QgsDebugMsg( QStringLiteral( "Delete %1" ).arg( fid ) );

  QgsVectorLayerUtils::QgsDuplicateFeatureContext infoContext;
  if ( QgsVectorLayerUtils::impactsCascadeFeatures( mLayer, QgsFeatureIds() << fid, QgsProject::instance(), infoContext, QgsVectorLayerUtils::IgnoreAuxiliaryLayers ) )
  {
    QString childrenInfo;
    int childrenCount = 0;
    const auto infoContextLayers = infoContext.layers();
    for ( QgsVectorLayer *chl : infoContextLayers )
    {
      childrenCount += infoContext.duplicatedFeatures( chl ).size();
      childrenInfo += ( tr( "%n feature(s) on layer \"%1\", ", nullptr, infoContext.duplicatedFeatures( chl ).size() ).arg( chl->name() ) );
    }

    // for extra safety to make sure we know that the delete can have impact on children and joins
    int res = QMessageBox::question( this, tr( "Delete at least %1 feature(s) on other layer(s)" ).arg( childrenCount ),
                                     tr( "Delete of feature on layer \"%1\", %2 as well\nand all of its other descendants.\nDelete these features?" ).arg( mLayer->name() ).arg( childrenInfo ),
                                     QMessageBox::Yes | QMessageBox::No );
    if ( res != QMessageBox::Yes )
      return;
  }

  QgsVectorLayer::DeleteContext context( true, QgsProject::instance() );
  mLayer->deleteFeature( fid, &context );
  const QList<QgsVectorLayer *> contextLayers = context.handledLayers( false );
  //if it effected more than one layer, print feedback for all descendants
  if ( contextLayers.size() > 1 )
  {
    int deletedCount = 0;
    QString feedbackMessage;
    for ( QgsVectorLayer *contextLayer : contextLayers )
    {
      feedbackMessage += tr( "%1 on layer %2. " ).arg( context.handledFeatures( contextLayer ).size() ).arg( contextLayer->name() );
      deletedCount += context.handledFeatures( contextLayer ).size();
    }
    QgisApp::instance()->messageBar()->pushMessage( tr( "%n feature(s) deleted: %1", nullptr, deletedCount ).arg( feedbackMessage ), Qgis::MessageLevel::Success );
  }
}

void QgsAttributeTableDialog::showContextMenu( QgsActionMenu *menu, const QgsFeatureId fid )
{
  if ( mLayer->isEditable() )
  {
    QAction *qAction = menu->addAction( QgsApplication::getThemeIcon( QStringLiteral( "/mActionDeleteSelectedFeatures.svg" ) ),  tr( "Delete Feature" ) );
    connect( qAction, &QAction::triggered, this, [this, fid]() { deleteFeature( fid ); } );
  }
}

void QgsAttributeTableDialog::toggleDockMode( bool docked )
{
  if ( docked )
  {
    // going from window -> dock, so save current window geometry
    QgsSettings().setValue( QStringLiteral( "Windows/BetterAttributeTable/geometry" ), mDialog ? mDialog->saveGeometry() : saveGeometry() );
    if ( mDialog )
    {
      mDialog->removeEventFilter( this );
      mDialog->setLayout( nullptr );
      mDialog->deleteLater();
      mDialog = nullptr;
    }

    mDock = new QgsAttributeTableDock( QString(), QgisApp::instance() );
    mDock->setWidget( this );
    connect( this, &QObject::destroyed, mDock, &QWidget::close );
    QgisApp::instance()->addTabifiedDockWidget( Qt::BottomDockWidgetArea, mDock, QStringList(), true );
    updateTitle();

    // To prevent "QAction::event: Ambiguous shortcut overload"
    QgsDebugMsgLevel( QStringLiteral( "Remove shortcuts from attribute table already defined in main window" ), 2 );
    mActionZoomMapToSelectedRows->setShortcut( QKeySequence() );
    mActionRemoveSelection->setShortcut( QKeySequence() );
    // duplicated on Main Window, with different semantics
    mActionPanMapToSelectedRows->setShortcut( QKeySequence() );
    mActionSearchForm->setShortcut( QKeySequence() );
  }
  else
  {
    // going from dock -> window

    mDialog = new QDialog( QgisApp::instance(), Qt::Window );
    mDialog->setAttribute( Qt::WA_DeleteOnClose );

    QVBoxLayout *vl = new QVBoxLayout();
    vl->setContentsMargins( 0, 0, 0, 0 );
    vl->addWidget( this );
    mDialog->setLayout( vl );

    if ( mDock )
    {
      mDock->setWidget( nullptr );
      disconnect( this, &QObject::destroyed, mDock, &QWidget::close );
      mDock->deleteLater();
      mDock = nullptr;
    }

    // subscribe to close events, so that we can save window geometry
    mDialog->installEventFilter( this );

    updateTitle();
    mDialog->restoreGeometry( QgsSettings().value( QStringLiteral( "Windows/BetterAttributeTable/geometry" ) ).toByteArray() );
    mDialog->show();

    // restore attribute table shortcuts in window mode
    QgsDebugMsgLevel( QStringLiteral( "Restore attribute table dialog shortcuts in window mode" ), 2 );
    // duplicated on Main Window
    mActionZoomMapToSelectedRows->setShortcut( QStringLiteral( "Ctrl+J" ) );
    mActionRemoveSelection->setShortcut( QStringLiteral( "Ctrl+Shift+A" ) );
    // duplicated on Main Window, with different semantics
    mActionPanMapToSelectedRows->setShortcut( QStringLiteral( "Ctrl+P" ) );
    mActionSearchForm->setShortcut( QStringLiteral( "Ctrl+F" ) );
  }
}

void QgsAttributeTableDialog::updateLayerModifiedActions()
{
  bool saveEnabled { mActionToggleEditing->isEnabled() &&mLayer->isEditable() &&mLayer->isModified() };
  if ( ! saveEnabled && mActionToggleEditing->isEnabled() )
  {
    for ( const auto &referencingLayer : std::as_const( mReferencingLayers ) )
    {
      if ( referencingLayer && referencingLayer->isEditable() && referencingLayer->isModified() )
      {
        saveEnabled = true;
        break;
      }
    }
  }
  mActionSaveEdits->setEnabled( saveEnabled );
}

//
// QgsAttributeTableDock
//

QgsAttributeTableDock::QgsAttributeTableDock( const QString &title, QWidget *parent, Qt::WindowFlags flags )
  : QgsDockWidget( title, parent, flags )
{
  setObjectName( QStringLiteral( "AttributeTable" ) ); // set object name so the position can be saved
}

void QgsAttributeTableDock::closeEvent( QCloseEvent *ev )
{
  Q_UNUSED( ev )
  deleteLater();
}
