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

#include "qgsattributetabledialog.h"
#include "qgsattributetablemodel.h"
#include "qgsattributetablefiltermodel.h"
#include "qgsattributetableview.h"


#include "qgsapplication.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerutils.h"
#include "qgsvectordataprovider.h"
#include "qgsexpression.h"
#include "qgsexpressionbuilderwidget.h"
#include "qgisapp.h"
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
#include "qgsexpressionbuilderdialog.h"
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
  if ( ! mLayer || ( mLayer->editFormConfig().layout() == QgsEditFormConfig::UiFileLayout ) )
    return;

  mActionToggleMultiEdit->setEnabled( mLayer->isEditable() );

  if ( !mLayer->isEditable() || ( mLayer->isEditable() && mMainView->view() != QgsDualView::AttributeEditor ) )
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
  connect( mActionOpenFieldCalculator, &QAction::triggered, this, &QgsAttributeTableDialog::mActionOpenFieldCalculator_triggered );
  connect( mActionDeleteSelected, &QAction::triggered, this, &QgsAttributeTableDialog::mActionDeleteSelected_triggered );
  connect( mMainView, &QgsDualView::currentChanged, this, &QgsAttributeTableDialog::mMainView_currentChanged );
  connect( mActionAddFeature, &QAction::triggered, this, &QgsAttributeTableDialog::mActionAddFeature_triggered );
  connect( mActionExpressionSelect, &QAction::triggered, this, &QgsAttributeTableDialog::mActionExpressionSelect_triggered );
  connect( mMainView, &QgsDualView::showContextMenuExternally, this, &QgsAttributeTableDialog::showContextMenu );

  const QgsFields fields = mLayer->fields();
  for ( const QgsField &field : fields )
  {
    mVisibleFields.append( field.name() );
  }

  // Fix selection color on losing focus (Windows)
  setStyleSheet( QgisApp::instance()->styleSheet() );

  setAttribute( Qt::WA_DeleteOnClose );

  layout()->setMargin( 0 );
  layout()->setContentsMargins( 0, 0, 0, 0 );
  static_cast< QGridLayout * >( layout() )->setVerticalSpacing( 0 );

  QgsSettings settings;

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

  myDa = new QgsDistanceArea();

  myDa->setSourceCrs( mLayer->crs(), QgsProject::instance()->transformContext() );
  myDa->setEllipsoid( QgsProject::instance()->ellipsoid() );

  mEditorContext.setDistanceArea( *myDa );
  mEditorContext.setVectorLayerTools( QgisApp::instance()->vectorLayerTools() );
  mEditorContext.setMapCanvas( QgisApp::instance()->mapCanvas() );

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
  if ( !needsGeom )
    r.setFlags( QgsFeatureRequest::NoGeometry );

  // Initialize dual view
  mMainView->init( mLayer, QgisApp::instance()->mapCanvas(), r, mEditorContext, false );

  QgsAttributeTableConfig config = mLayer->attributeTableConfig();
  mMainView->setAttributeTableConfig( config );

  // Initialize filter gui elements
  mFilterActionMapper = new QSignalMapper( this );
  mFilterColumnsMenu = new QMenu( this );
  mActionFilterColumnsMenu->setMenu( mFilterColumnsMenu );
  mApplyFilterButton->setDefaultAction( mActionApplyFilter );

  // Set filter icon in a couple of places
  QIcon filterIcon = QgsApplication::getThemeIcon( "/mActionFilter2.svg" );
  mActionShowAllFilter->setIcon( filterIcon );
  mActionAdvancedFilter->setIcon( filterIcon );
  mActionSelectedFilter->setIcon( filterIcon );
  mActionVisibleFilter->setIcon( filterIcon );
  mActionEditedFilter->setIcon( filterIcon );

  mActionFeatureActions = new QToolButton();
  mActionFeatureActions->setAutoRaise( false );
  mActionFeatureActions->setPopupMode( QToolButton::InstantPopup );
  mActionFeatureActions->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mAction.svg" ) ) );
  mActionFeatureActions->setText( tr( "Actions" ) );
  mActionFeatureActions->setToolTip( tr( "Actions" ) );
  mToolbar->addWidget( mActionFeatureActions );

  // Connect filter signals
  connect( mActionAdvancedFilter, &QAction::triggered, this, &QgsAttributeTableDialog::filterExpressionBuilder );
  connect( mActionShowAllFilter, &QAction::triggered, this, &QgsAttributeTableDialog::filterShowAll );
  connect( mActionSelectedFilter, &QAction::triggered, this, &QgsAttributeTableDialog::filterSelected );
  connect( mActionVisibleFilter, &QAction::triggered, this, &QgsAttributeTableDialog::filterVisible );
  connect( mActionEditedFilter, &QAction::triggered, this, &QgsAttributeTableDialog::filterEdited );
  connect( mFilterActionMapper, SIGNAL( mapped( QObject * ) ), SLOT( filterColumnChanged( QObject * ) ) );
  connect( mFilterQuery, &QLineEdit::returnPressed, this, &QgsAttributeTableDialog::filterQueryAccepted );
  connect( mActionApplyFilter, &QAction::triggered, this, &QgsAttributeTableDialog::filterQueryAccepted );
  connect( mActionSetStyles, &QAction::triggered, this, &QgsAttributeTableDialog::openConditionalStyles );

  // info from layer to table
  connect( mLayer, &QgsVectorLayer::editingStarted, this, &QgsAttributeTableDialog::editingToggled );
  connect( mLayer, &QgsVectorLayer::editingStopped, this, &QgsAttributeTableDialog::editingToggled );
  connect( mLayer, &QObject::destroyed, mMainView, &QgsDualView::cancelProgress );
  connect( mLayer, &QgsVectorLayer::selectionChanged, this, &QgsAttributeTableDialog::updateTitle );
  connect( mLayer, &QgsVectorLayer::featureAdded, this, &QgsAttributeTableDialog::updateTitle );
  connect( mLayer, &QgsVectorLayer::featuresDeleted, this, &QgsAttributeTableDialog::updateTitle );
  connect( mLayer, &QgsVectorLayer::editingStopped, this, &QgsAttributeTableDialog::updateTitle );
  connect( mLayer, &QgsVectorLayer::attributeAdded, this, &QgsAttributeTableDialog::columnBoxInit );
  connect( mLayer, &QgsVectorLayer::attributeDeleted, this, &QgsAttributeTableDialog::columnBoxInit );
  connect( mLayer, &QgsVectorLayer::readOnlyChanged, this, &QgsAttributeTableDialog::editingToggled );

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
    QgisApp::instance()->addDockWidget( Qt::BottomDockWidgetArea, mDock );
  }
  mActionDockUndock->setChecked( dockTable );
  connect( mActionDockUndock, &QAction::toggled, this, &QgsAttributeTableDialog::toggleDockMode );
  installEventFilter( this );

  columnBoxInit();
  updateTitle();

  mActionRemoveSelection->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionDeselectAll.svg" ) ) );
  mActionSelectAll->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionSelectAll.svg" ) ) );
  mActionSelectedToTop->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionSelectedToTop.svg" ) ) );
  mActionCopySelectedRows->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionEditCopy.svg" ) ) );
  mActionPasteFeatures->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionEditPaste.svg" ) ) );
  mActionZoomMapToSelectedRows->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionZoomToSelected.svg" ) ) );
  mActionPanMapToSelectedRows->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionPanToSelected.svg" ) ) );
  mActionInvertSelection->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionInvertSelection.svg" ) ) );
  mActionToggleEditing->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionToggleEditing.svg" ) ) );
  mActionSaveEdits->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionSaveEdits.svg" ) ) );
  mActionDeleteSelected->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionDeleteSelected.svg" ) ) );
  mActionOpenFieldCalculator->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionCalculateField.svg" ) ) );
  mActionAddAttribute->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionNewAttribute.svg" ) ) );
  mActionRemoveAttribute->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionDeleteAttribute.svg" ) ) );
  mTableViewButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionOpenTable.svg" ) ) );
  mAttributeViewButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionFormView.svg" ) ) );
  mActionExpressionSelect->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mIconExpressionSelect.svg" ) ) );
  mActionAddFeature->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionNewTableRow.svg" ) ) );
  mActionFeatureActions->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mAction.svg" ) ) );

  // toggle editing
  bool canChangeAttributes = mLayer->dataProvider()->capabilities() & QgsVectorDataProvider::ChangeAttributeValues;
  bool canDeleteFeatures = mLayer->dataProvider()->capabilities() & QgsVectorDataProvider::DeleteFeatures;
  bool canAddAttributes = mLayer->dataProvider()->capabilities() & QgsVectorDataProvider::AddAttributes;
  bool canDeleteAttributes = mLayer->dataProvider()->capabilities() & QgsVectorDataProvider::DeleteAttributes;
  bool canAddFeatures = mLayer->dataProvider()->capabilities() & QgsVectorDataProvider::AddFeatures;

  mActionToggleEditing->blockSignals( true );
  mActionToggleEditing->setCheckable( true );
  mActionToggleEditing->setChecked( mLayer->isEditable() );
  mActionToggleEditing->blockSignals( false );

  mActionSaveEdits->setEnabled( mActionToggleEditing->isEnabled() && mLayer->isEditable() );
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

  mMainViewButtonGroup->setId( mTableViewButton, QgsDualView::AttributeTable );
  mMainViewButtonGroup->setId( mAttributeViewButton, QgsDualView::AttributeEditor );

  switch ( initialMode )
  {
    case QgsAttributeTableFilterModel::ShowVisible:
      filterVisible();
      break;

    case QgsAttributeTableFilterModel::ShowSelected:
      filterSelected();
      break;

    case QgsAttributeTableFilterModel::ShowAll:
    default:
      filterShowAll();
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

    if ( mLayer->editFormConfig().layout() == QgsEditFormConfig::UiFileLayout )
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

QgsAttributeTableDialog::~QgsAttributeTableDialog()
{
  delete myDa;
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
  w->setWindowTitle( tr( " %1 :: Features Total: %2, Filtered: %3, Selected: %4" )
                     .arg( mLayer->name() )
                     .arg( std::max( static_cast< long >( mMainView->featureCount() ), mLayer->featureCount() ) ) // layer count may be estimated, so use larger of the two
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
  Q_UNUSED( fieldName );
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

void QgsAttributeTableDialog::columnBoxInit()
{
  Q_FOREACH ( QAction *a, mFilterColumnsMenu->actions() )
  {
    mFilterColumnsMenu->removeAction( a );
    mFilterActionMapper->removeMappings( a );
    mFilterButton->removeAction( a );
    delete a;
  }

  mFilterButton->addAction( mActionShowAllFilter );
  mFilterButton->addAction( mActionSelectedFilter );
  if ( mLayer->isSpatial() )
  {
    mFilterButton->addAction( mActionVisibleFilter );
  }
  mFilterButton->addAction( mActionEditedFilter );
  mFilterButton->addAction( mActionFilterColumnsMenu );
  mFilterButton->addAction( mActionAdvancedFilter );

  const QList<QgsField> fields = mLayer->fields().toList();

  Q_FOREACH ( const QgsField &field, fields )
  {
    int idx = mLayer->fields().lookupField( field.name() );
    if ( idx < 0 )
      continue;

    if ( QgsGui::editorWidgetRegistry()->findBest( mLayer, field.name() ).type() != QLatin1String( "Hidden" ) )
    {
      QIcon icon = mLayer->fields().iconForField( idx );
      QString alias = mLayer->attributeDisplayName( idx );

      // Generate action for the filter popup button
      QAction *filterAction = new QAction( icon, alias, mFilterButton );
      filterAction->setData( field.name() );
      mFilterActionMapper->setMapping( filterAction, filterAction );
      connect( filterAction, SIGNAL( triggered() ), mFilterActionMapper, SLOT( map() ) );
      mFilterColumnsMenu->addAction( filterAction );
    }
  }
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
  exp.setGeomCalculator( myDa );
  exp.setDistanceUnits( QgsProject::instance()->distanceUnits() );
  exp.setAreaUnits( QgsProject::instance()->areaUnits() );
  bool useGeometry = exp.needsGeometry();

  QgsFeatureRequest request( mMainView->masterModel()->request() );
  useGeometry |= !request.filterRect().isNull();
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

  std::unique_ptr< QgsScopedProxyProgressTask > task = qgis::make_unique< QgsScopedProxyProgressTask >( tr( "Calculating field" ) );

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
    // fixes https://issues.qgis.org/issues/17312
    QgsAttributeTableModel *masterModel = mMainView->masterModel();
    int modelColumn = masterModel->fieldCol( fieldindex );
    masterModel->reload( masterModel->index( 0, modelColumn ), masterModel->index( masterModel->rowCount() - 1, modelColumn ) );
  }
}

void QgsAttributeTableDialog::replaceSearchWidget( QWidget *oldw, QWidget *neww )
{
  mFilterLayout->removeWidget( oldw );
  oldw->setVisible( false );
  mFilterLayout->addWidget( neww, 0, 0, nullptr );
  neww->setVisible( true );
  neww->setFocus();
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

void QgsAttributeTableDialog::filterColumnChanged( QObject *filterAction )
{
  mFilterButton->setDefaultAction( qobject_cast<QAction *>( filterAction ) );
  mFilterButton->setPopupMode( QToolButton::InstantPopup );
  // replace the search line edit with a search widget that is suited to the selected field
  // delete previous widget
  if ( mCurrentSearchWidgetWrapper )
  {
    mCurrentSearchWidgetWrapper->widget()->setVisible( false );
    delete mCurrentSearchWidgetWrapper;
  }
  QString fieldName = mFilterButton->defaultAction()->data().toString();
  // get the search widget
  int fldIdx = mLayer->fields().lookupField( fieldName );
  if ( fldIdx < 0 )
    return;
  const QgsEditorWidgetSetup setup = QgsGui::editorWidgetRegistry()->findBest( mLayer, fieldName );
  mCurrentSearchWidgetWrapper = QgsGui::editorWidgetRegistry()->
                                createSearchWidget( setup.type(), mLayer, fldIdx, setup.config(), mFilterContainer, mEditorContext );
  if ( mCurrentSearchWidgetWrapper->applyDirectly() )
  {
    connect( mCurrentSearchWidgetWrapper, &QgsSearchWidgetWrapper::expressionChanged, this, &QgsAttributeTableDialog::filterQueryChanged );
    mApplyFilterButton->setVisible( false );
  }
  else
  {
    connect( mCurrentSearchWidgetWrapper, &QgsSearchWidgetWrapper::expressionChanged, this, &QgsAttributeTableDialog::filterQueryAccepted );
    mApplyFilterButton->setVisible( true );
  }

  replaceSearchWidget( mFilterQuery, mCurrentSearchWidgetWrapper->widget() );
}

void QgsAttributeTableDialog::filterExpressionBuilder()
{
  // Show expression builder
  QgsExpressionContext context( QgsExpressionContextUtils::globalProjectLayerScopes( mLayer ) );

  QgsExpressionBuilderDialog dlg( mLayer, mFilterQuery->text(), this, QStringLiteral( "generic" ), context );
  dlg.setWindowTitle( tr( "Expression Based Filter" ) );

  QgsDistanceArea myDa;
  myDa.setSourceCrs( mLayer->crs(), QgsProject::instance()->transformContext() );
  myDa.setEllipsoid( QgsProject::instance()->ellipsoid() );
  dlg.setGeomCalculator( myDa );

  if ( dlg.exec() == QDialog::Accepted )
  {
    setFilterExpression( dlg.expressionText(), QgsAttributeForm::ReplaceFilter, true );
  }
}

void QgsAttributeTableDialog::filterShowAll()
{
  mFilterButton->setDefaultAction( mActionShowAllFilter );
  mFilterButton->setPopupMode( QToolButton::InstantPopup );
  mFilterQuery->setVisible( false );
  mFilterQuery->setText( QString() );
  if ( mCurrentSearchWidgetWrapper )
  {
    mCurrentSearchWidgetWrapper->widget()->setVisible( false );
  }
  mApplyFilterButton->setVisible( false );
  mMainView->setFilterMode( QgsAttributeTableFilterModel::ShowAll );
}

void QgsAttributeTableDialog::filterSelected()
{
  mFilterButton->setDefaultAction( mActionSelectedFilter );
  mFilterButton->setPopupMode( QToolButton::InstantPopup );
  mFilterQuery->setVisible( false );
  mApplyFilterButton->setVisible( false );
  mMainView->setFilterMode( QgsAttributeTableFilterModel::ShowSelected );
}

void QgsAttributeTableDialog::filterVisible()
{
  if ( !mLayer->isSpatial() )
  {
    filterShowAll();
    return;
  }

  mFilterButton->setDefaultAction( mActionVisibleFilter );
  mFilterButton->setPopupMode( QToolButton::InstantPopup );
  mFilterQuery->setVisible( false );
  mApplyFilterButton->setVisible( false );
  mMainView->setFilterMode( QgsAttributeTableFilterModel::ShowVisible );
}

void QgsAttributeTableDialog::filterEdited()
{
  mFilterButton->setDefaultAction( mActionEditedFilter );
  mFilterButton->setPopupMode( QToolButton::InstantPopup );
  mFilterQuery->setVisible( false );
  mApplyFilterButton->setVisible( false );
  mMainView->setFilterMode( QgsAttributeTableFilterModel::ShowEdited );
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
}

void QgsAttributeTableDialog::mActionReload_triggered()
{
  mMainView->masterModel()->layer()->dataProvider()->forceReload();
}

void QgsAttributeTableDialog::mActionAddFeature_triggered()
{
  if ( !mLayer->isEditable() )
    return;

  QgsAttributeTableModel *masterModel = mMainView->masterModel();

  QgsFeature f;
  QgsFeatureAction action( tr( "Geometryless feature added" ), f, mLayer, QString(), -1, this );
  action.setForceSuppressFormPopup( true ); // we're already showing the table, allowing users to enter the new feature's attributes directly
  if ( action.addFeature() )
  {
    masterModel->reload( masterModel->index( 0, 0 ), masterModel->index( masterModel->rowCount() - 1, masterModel->columnCount() - 1 ) );
  }
}

void QgsAttributeTableDialog::mActionExpressionSelect_triggered()
{
  QgsExpressionSelectionDialog *dlg = new QgsExpressionSelectionDialog( mLayer );
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

    QgsFeatureIterator it = mLayer->getFeatures( QgsFeatureRequest( featureIds.toSet() )
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
  mActionSaveEdits->setEnabled( mLayer->isEditable() );
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
    Q_FOREACH ( const QgsAction &action, actions )
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
    }
    else
    {
      mLayer->destroyEditCommand();
      QMessageBox::critical( this, tr( "Add Field" ), tr( "Failed to add field '%1' of type '%2'. Is the field name unique?" ).arg( dialog.field().name(), dialog.field().typeName() ) );
    }


    // update model - a field has been added or updated
    masterModel->reload( masterModel->index( 0, 0 ), masterModel->index( masterModel->rowCount() - 1, masterModel->columnCount() - 1 ) );
    columnBoxInit();
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

    QgsAttributeTableModel *masterModel = mMainView->masterModel();

    mLayer->beginEditCommand( tr( "Deleted attribute" ) );
    if ( mLayer->deleteAttributes( attributes ) )
    {
      mLayer->endEditCommand();
    }
    else
    {
      QgisApp::instance()->messageBar()->pushMessage( tr( "Attribute error" ), tr( "The attribute(s) could not be deleted" ), Qgis::Warning, QgisApp::instance()->messageTimeout() );
      mLayer->destroyEditCommand();
    }
    // update model - a field has been added or updated
    masterModel->reload( masterModel->index( 0, 0 ), masterModel->index( masterModel->rowCount() - 1, masterModel->columnCount() - 1 ) );
    columnBoxInit();
  }
}

void QgsAttributeTableDialog::filterQueryChanged( const QString &query )
{
  QString str;
  if ( mFilterButton->defaultAction() == mActionAdvancedFilter )
  {
    str = query;
  }
  else if ( mCurrentSearchWidgetWrapper )
  {
    str = mCurrentSearchWidgetWrapper->expression();
  }

  setFilterExpression( str );
}

void QgsAttributeTableDialog::filterQueryAccepted()
{
  if ( ( mFilterQuery->isVisible() && mFilterQuery->text().isEmpty() ) ||
       ( mCurrentSearchWidgetWrapper && mCurrentSearchWidgetWrapper->widget()->isVisible()
         && mCurrentSearchWidgetWrapper->expression().isEmpty() ) )
  {
    filterShowAll();
    return;
  }
  filterQueryChanged( mFilterQuery->text() );
}

void QgsAttributeTableDialog::openConditionalStyles()
{
  mMainView->openConditionalStyles();
}

void QgsAttributeTableDialog::setFilterExpression( const QString &filterString, QgsAttributeForm::FilterType type,
    bool alwaysShowFilter )
{
  QString filter;
  if ( !mFilterQuery->text().isEmpty() && !filterString.isEmpty() )
  {
    switch ( type )
    {
      case QgsAttributeForm::ReplaceFilter:
        filter = filterString;
        break;

      case QgsAttributeForm::FilterAnd:
        filter = QStringLiteral( "(%1) AND (%2)" ).arg( mFilterQuery->text(), filterString );
        break;

      case QgsAttributeForm::FilterOr:
        filter = QStringLiteral( "(%1) OR (%2)" ).arg( mFilterQuery->text(), filterString );
        break;
    }
  }
  else if ( !filterString.isEmpty() )
  {
    filter = filterString;
  }
  else
  {
    filterShowAll();
    return;
  }

  mFilterQuery->setText( filter );

  if ( alwaysShowFilter || !mCurrentSearchWidgetWrapper || !mCurrentSearchWidgetWrapper->applyDirectly() )
  {

    mFilterButton->setDefaultAction( mActionAdvancedFilter );
    mFilterButton->setPopupMode( QToolButton::MenuButtonPopup );
    mFilterQuery->setVisible( true );
    mApplyFilterButton->setVisible( true );
    if ( mCurrentSearchWidgetWrapper )
    {
      // replace search widget widget with the normal filter query line edit
      replaceSearchWidget( mCurrentSearchWidgetWrapper->widget(), mFilterQuery );
    }
  }

  QgsFeatureIds filteredFeatures;
  QgsDistanceArea myDa;

  myDa.setSourceCrs( mLayer->crs(), QgsProject::instance()->transformContext() );
  myDa.setEllipsoid( QgsProject::instance()->ellipsoid() );

  // parse search string and build parsed tree
  QgsExpression filterExpression( filter );
  if ( filterExpression.hasParserError() )
  {
    QgisApp::instance()->messageBar()->pushMessage( tr( "Parsing error" ), filterExpression.parserErrorString(), Qgis::Warning, QgisApp::instance()->messageTimeout() );
    return;
  }

  QgsExpressionContext context( QgsExpressionContextUtils::globalProjectLayerScopes( mLayer ) );

  if ( !filterExpression.prepare( &context ) )
  {
    QgisApp::instance()->messageBar()->pushMessage( tr( "Evaluation error" ), filterExpression.evalErrorString(), Qgis::Warning, QgisApp::instance()->messageTimeout() );
  }

  bool fetchGeom = filterExpression.needsGeometry();

  QApplication::setOverrideCursor( Qt::WaitCursor );

  filterExpression.setGeomCalculator( &myDa );
  filterExpression.setDistanceUnits( QgsProject::instance()->distanceUnits() );
  filterExpression.setAreaUnits( QgsProject::instance()->areaUnits() );
  QgsFeatureRequest request( mMainView->masterModel()->request() );
  request.setSubsetOfAttributes( filterExpression.referencedColumns(), mLayer->fields() );
  if ( !fetchGeom )
  {
    request.setFlags( QgsFeatureRequest::NoGeometry );
  }
  else
  {
    // force geometry extraction if the filter requests it
    request.setFlags( request.flags() & ~QgsFeatureRequest::NoGeometry );
  }
  QgsFeatureIterator featIt = mLayer->getFeatures( request );

  QgsFeature f;

  while ( featIt.nextFeature( f ) )
  {
    context.setFeature( f );
    if ( filterExpression.evaluate( &context ).toInt() != 0 )
      filteredFeatures << f.id();

    // check if there were errors during evaluating
    if ( filterExpression.hasEvalError() )
      break;
  }

  featIt.close();

  mMainView->setFilteredFeatures( filteredFeatures );

  QApplication::restoreOverrideCursor();

  if ( filterExpression.hasEvalError() )
  {
    QgisApp::instance()->messageBar()->pushMessage( tr( "Error filtering" ), filterExpression.evalErrorString(), Qgis::Warning, QgisApp::instance()->messageTimeout() );
    return;
  }
  mMainView->setFilterMode( QgsAttributeTableFilterModel::ShowFilteredList );
}


void QgsAttributeTableDialog::deleteFeature( const QgsFeatureId fid )
{
  QgsDebugMsg( QStringLiteral( "Delete %1" ).arg( fid ) );
  mLayer->deleteFeature( fid );
}

void QgsAttributeTableDialog::showContextMenu( QgsActionMenu *menu, const QgsFeatureId fid )
{
  if ( mLayer->isEditable() )
  {
    QAction *qAction = menu->addAction( QgsApplication::getThemeIcon( QStringLiteral( "/mActionDeleteSelected.svg" ) ),  tr( "Delete feature" ) );
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
    QgisApp::instance()->addDockWidget( Qt::BottomDockWidgetArea, mDock );
    updateTitle();
  }
  else
  {
    // going from dock -> window

    mDialog = new QDialog( QgisApp::instance(), Qt::Window );
    mDialog->setAttribute( Qt::WA_DeleteOnClose );

    QVBoxLayout *vl = new QVBoxLayout();
    vl->setContentsMargins( 0, 0, 0, 0 );
    vl->setMargin( 0 );
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
  }
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
  Q_UNUSED( ev );
  deleteLater();
}
