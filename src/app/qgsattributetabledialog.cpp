/***************************************************************************
  QgsAttributeTableDialog.cpp
  -------------------
         date                 : Feb 2009
         copyright            : Vita Cizek
         email                : weetya (at) gmail.com

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QDockWidget>
#include <QMessageBox>

#include "qgsattributetabledialog.h"
#include "qgsattributetablemodel.h"
#include "qgsattributetablefiltermodel.h"
#include "qgsattributetableview.h"

#include <qgsapplication.h>
#include <qgsvectordataprovider.h>
#include <qgsvectorlayer.h>
#include <qgsexpression.h>

#include "qgisapp.h"
#include "qgsaddattrdialog.h"
#include "qgsdelattrdialog.h"
#include "qgssearchquerybuilder.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsproject.h"
#include "qgsfieldcalculator.h"
#include "qgsfeatureaction.h"
#include "qgsattributeaction.h"
#include "qgsexpressionbuilderdialog.h"
#include "qgsmessagebar.h"
#include "qgsexpressionselectiondialog.h"
#include "qgsfeaturelistmodel.h"
#include "qgsrubberband.h"
#include "qgsfield.h"
#include "qgseditorwidgetregistry.h"

class QgsAttributeTableDock : public QDockWidget
{
  public:
    QgsAttributeTableDock( const QString & title, QWidget * parent = 0, Qt::WindowFlags flags = 0 )
        : QDockWidget( title, parent, flags )
    {
      setObjectName( "AttributeTable" ); // set object name so the position can be saved
    }

    virtual void closeEvent( QCloseEvent * ev ) override
    {
      Q_UNUSED( ev );
      deleteLater();
    }
};

static QgsExpressionContext _getExpressionContext( const void* context )
{
  QgsExpressionContext expContext;
  expContext << QgsExpressionContextUtils::globalScope()
  << QgsExpressionContextUtils::projectScope();

  const QgsVectorLayer* layer = ( const QgsVectorLayer* ) context;
  if ( layer )
    expContext << QgsExpressionContextUtils::layerScope( layer );

  return expContext;
}

QgsAttributeTableDialog::QgsAttributeTableDialog( QgsVectorLayer *theLayer, QWidget *parent, Qt::WindowFlags flags )
    : QDialog( parent, flags )
    , mDock( 0 )
    , mLayer( theLayer )
    , mRubberBand( 0 )
    , mCurrentSearchWidgetWrapper( 0 )
{
  setupUi( this );

  // Fix selection color on loosing focus (Windows)
  setStyleSheet( QgisApp::instance()->styleSheet() );

  setAttribute( Qt::WA_DeleteOnClose );

  QSettings settings;

  // Initialize the window geometry
  restoreGeometry( settings.value( "/Windows/BetterAttributeTable/geometry" ).toByteArray() );

  QgsAttributeEditorContext context;

  myDa = new QgsDistanceArea();

  myDa->setSourceCrs( mLayer->crs() );
  myDa->setEllipsoidalMode( QgisApp::instance()->mapCanvas()->mapSettings().hasCrsTransformEnabled() );
  myDa->setEllipsoid( QgsProject::instance()->readEntry( "Measure", "/Ellipsoid", GEO_NONE ) );

  context.setDistanceArea( *myDa );
  context.setVectorLayerTools( QgisApp::instance()->vectorLayerTools() );

  QgsFeatureRequest r;
  if ( mLayer->geometryType() != QGis::NoGeometry &&
       settings.value( "/qgis/attributeTableBehaviour", QgsAttributeTableFilterModel::ShowAll ).toInt() == QgsAttributeTableFilterModel::ShowVisible )
  {
    QgsMapCanvas *mc = QgisApp::instance()->mapCanvas();
    QgsRectangle extent( mc->mapSettings().mapToLayerCoordinates( theLayer, mc->extent() ) );
    r.setFilterRect( extent );

    QgsGeometry *g = QgsGeometry::fromRect( extent );
    mRubberBand = new QgsRubberBand( mc, true );
    mRubberBand->setToGeometry( g, theLayer );
    delete g;

    mActionShowAllFilter->setText( tr( "Show All Features In Initial Canvas Extent" ) );
  }

  // Initialize dual view
  mMainView->init( mLayer, QgisApp::instance()->mapCanvas(), r, context );

  // Initialize filter gui elements
  mFilterActionMapper = new QSignalMapper( this );
  mFilterColumnsMenu = new QMenu( this );
  mActionFilterColumnsMenu->setMenu( mFilterColumnsMenu );
  mApplyFilterButton->setDefaultAction( mActionApplyFilter );

  // Set filter icon in a couple of places
  QIcon filterIcon = QgsApplication::getThemeIcon( "/mActionFilter.svg" );
  mActionShowAllFilter->setIcon( filterIcon );
  mActionAdvancedFilter->setIcon( filterIcon );
  mActionSelectedFilter->setIcon( filterIcon );
  mActionVisibleFilter->setIcon( filterIcon );
  mActionEditedFilter->setIcon( filterIcon );

  // Connect filter signals
  connect( mActionAdvancedFilter, SIGNAL( triggered() ), SLOT( filterExpressionBuilder() ) );
  connect( mActionShowAllFilter, SIGNAL( triggered() ), SLOT( filterShowAll() ) );
  connect( mActionSelectedFilter, SIGNAL( triggered() ), SLOT( filterSelected() ) );
  connect( mActionVisibleFilter, SIGNAL( triggered() ), SLOT( filterVisible() ) );
  connect( mActionEditedFilter, SIGNAL( triggered() ), SLOT( filterEdited() ) );
  connect( mFilterActionMapper, SIGNAL( mapped( QObject* ) ), SLOT( filterColumnChanged( QObject* ) ) );
  connect( mFilterQuery, SIGNAL( returnPressed() ), SLOT( filterQueryAccepted() ) );
  connect( mActionApplyFilter, SIGNAL( triggered() ), SLOT( filterQueryAccepted() ) );
  connect( mSetStyles, SIGNAL( pressed() ), SLOT( openConditionalStyles() ) );

  // info from layer to table
  connect( mLayer, SIGNAL( editingStarted() ), this, SLOT( editingToggled() ) );
  connect( mLayer, SIGNAL( editingStopped() ), this, SLOT( editingToggled() ) );
  connect( mLayer, SIGNAL( layerDeleted() ), this, SLOT( close() ) );
  connect( mLayer, SIGNAL( selectionChanged() ), this, SLOT( updateTitle() ) );
  connect( mLayer, SIGNAL( featureAdded( QgsFeatureId ) ), this, SLOT( updateTitle() ) );
  connect( mLayer, SIGNAL( featuresDeleted( QgsFeatureIds ) ), this, SLOT( updateTitle() ) );
  connect( mLayer, SIGNAL( attributeAdded( int ) ), this, SLOT( columnBoxInit() ) );
  connect( mLayer, SIGNAL( attributeDeleted( int ) ), this, SLOT( columnBoxInit() ) );

  // connect table info to window
  connect( mMainView, SIGNAL( filterChanged() ), this, SLOT( updateTitle() ) );

  // info from table to application
  connect( this, SIGNAL( saveEdits( QgsMapLayer * ) ), QgisApp::instance(), SLOT( saveEdits( QgsMapLayer * ) ) );

  bool myDockFlag = settings.value( "/qgis/dockAttributeTable", false ).toBool();
  if ( myDockFlag )
  {
    mDock = new QgsAttributeTableDock( tr( "Attribute table - %1 (%n Feature(s))", "feature count", mMainView->featureCount() ).arg( mLayer->name() ), QgisApp::instance() );
    mDock->setAllowedAreas( Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea );
    mDock->setWidget( this );
    connect( this, SIGNAL( destroyed() ), mDock, SLOT( close() ) );
    QgisApp::instance()->addDockWidget( Qt::BottomDockWidgetArea, mDock );
  }

  columnBoxInit();
  updateTitle();

  mRemoveSelectionButton->setIcon( QgsApplication::getThemeIcon( "/mActionUnselectAttributes.png" ) );
  mSelectedToTopButton->setIcon( QgsApplication::getThemeIcon( "/mActionSelectedToTop.png" ) );
  mCopySelectedRowsButton->setIcon( QgsApplication::getThemeIcon( "/mActionCopySelected.png" ) );
  mZoomMapToSelectedRowsButton->setIcon( QgsApplication::getThemeIcon( "/mActionZoomToSelected.svg" ) );
  mPanMapToSelectedRowsButton->setIcon( QgsApplication::getThemeIcon( "/mActionPanToSelected.svg" ) );
  mInvertSelectionButton->setIcon( QgsApplication::getThemeIcon( "/mActionInvertSelection.png" ) );
  mToggleEditingButton->setIcon( QgsApplication::getThemeIcon( "/mActionToggleEditing.svg" ) );
  mSaveEditsButton->setIcon( QgsApplication::getThemeIcon( "/mActionSaveEdits.svg" ) );
  mDeleteSelectedButton->setIcon( QgsApplication::getThemeIcon( "/mActionDeleteSelected.svg" ) );
  mOpenFieldCalculator->setIcon( QgsApplication::getThemeIcon( "/mActionCalculateField.png" ) );
  mAddAttribute->setIcon( QgsApplication::getThemeIcon( "/mActionNewAttribute.png" ) );
  mRemoveAttribute->setIcon( QgsApplication::getThemeIcon( "/mActionDeleteAttribute.png" ) );
  mTableViewButton->setIcon( QgsApplication::getThemeIcon( "/mActionOpenTable.png" ) );
  mAttributeViewButton->setIcon( QgsApplication::getThemeIcon( "/mActionPropertyItem.png" ) );
  mExpressionSelectButton->setIcon( QgsApplication::getThemeIcon( "/mIconExpressionSelect.svg" ) );
  mAddFeature->setIcon( QgsApplication::getThemeIcon( "/mActionNewTableRow.png" ) );

  // toggle editing
  bool canChangeAttributes = mLayer->dataProvider()->capabilities() & QgsVectorDataProvider::ChangeAttributeValues;
  bool canDeleteFeatures = mLayer->dataProvider()->capabilities() & QgsVectorDataProvider::DeleteFeatures;
  bool canAddAttributes = mLayer->dataProvider()->capabilities() & QgsVectorDataProvider::AddAttributes;
  bool canDeleteAttributes = mLayer->dataProvider()->capabilities() & QgsVectorDataProvider::DeleteAttributes;
  bool canAddFeatures = mLayer->dataProvider()->capabilities() & QgsVectorDataProvider::AddFeatures;

  mToggleEditingButton->blockSignals( true );
  mToggleEditingButton->setCheckable( true );
  mToggleEditingButton->setChecked( mLayer->isEditable() );
  mToggleEditingButton->setEnabled(( canChangeAttributes || canDeleteFeatures || canAddAttributes || canDeleteAttributes || canAddFeatures ) && !mLayer->isReadOnly() );
  mToggleEditingButton->blockSignals( false );

  mSaveEditsButton->setEnabled( mToggleEditingButton->isEnabled() && mLayer->isEditable() );
  mAddAttribute->setEnabled(( canChangeAttributes || canAddAttributes ) && mLayer->isEditable() );
  mDeleteSelectedButton->setEnabled( canDeleteFeatures && mLayer->isEditable() );
  mAddFeature->setEnabled( canAddFeatures && mLayer->isEditable() && mLayer->geometryType() == QGis::NoGeometry );
  mAddFeature->setHidden( !canAddFeatures || mLayer->geometryType() != QGis::NoGeometry );

  mMainViewButtonGroup->setId( mTableViewButton, QgsDualView::AttributeTable );
  mMainViewButtonGroup->setId( mAttributeViewButton, QgsDualView::AttributeEditor );

  // Load default attribute table filter
  QgsAttributeTableFilterModel::FilterMode defaultFilterMode = ( QgsAttributeTableFilterModel::FilterMode ) settings.value( "/qgis/attributeTableBehaviour", QgsAttributeTableFilterModel::ShowAll ).toInt();

  switch ( defaultFilterMode )
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

  mUpdateExpressionText->registerGetExpressionContextCallback( &_getExpressionContext, mLayer );

  mFieldModel = new QgsFieldModel();
  mFieldModel->setLayer( mLayer );
  mFieldCombo->setModel( mFieldModel );
  connect( mRunFieldCalc, SIGNAL( clicked() ), this, SLOT( updateFieldFromExpression() ) );
  connect( mRunFieldCalcSelected, SIGNAL( clicked() ), this, SLOT( updateFieldFromExpressionSelected() ) );
  // NW TODO Fix in 2.6 - Doesn't work with field model for some reason.
//  connect( mUpdateExpressionText, SIGNAL( returnPressed() ), this, SLOT( updateFieldFromExpression() ) );
  connect( mUpdateExpressionText, SIGNAL( fieldChanged( QString, bool ) ), this, SLOT( updateButtonStatus( QString, bool ) ) );
  mUpdateExpressionText->setLayer( mLayer );
  mUpdateExpressionText->setLeftHandButtonStyle( true );
  editingToggled();
}

QgsAttributeTableDialog::~QgsAttributeTableDialog()
{
  delete myDa;
  delete mRubberBand;
}

void QgsAttributeTableDialog::updateTitle()
{
  QWidget *w = mDock ? qobject_cast<QWidget*>( mDock ) : qobject_cast<QWidget*>( this );
  w->setWindowTitle( tr( "Attribute table - %1 :: Features total: %2, filtered: %3, selected: %4%5" )
                     .arg( mLayer->name() )
                     .arg( mMainView->featureCount() )
                     .arg( mMainView->filteredFeatureCount() )
                     .arg( mLayer->selectedFeatureCount() )
                     .arg( mRubberBand ? tr( ", spatially limited" ) : "" )
                   );

  if ( mMainView->filterMode() == QgsAttributeTableFilterModel::ShowAll )
    mRunFieldCalc->setText( tr( "Update All" ) );
  else
    mRunFieldCalc->setText( tr( "Update Filtered" ) );

  bool enabled = mLayer->selectedFeatureCount() > 0;
  mRunFieldCalcSelected->setEnabled( enabled );
}

void QgsAttributeTableDialog::updateButtonStatus( QString fieldName, bool isValid )
{
  Q_UNUSED( fieldName );
  mRunFieldCalc->setEnabled( isValid );
}

void QgsAttributeTableDialog::closeEvent( QCloseEvent* event )
{
  QDialog::closeEvent( event );

  if ( mDock == NULL )
  {
    QSettings settings;
    settings.setValue( "/Windows/BetterAttributeTable/geometry", saveGeometry() );
  }
}

void QgsAttributeTableDialog::keyPressEvent( QKeyEvent* event )
{
  QDialog::keyPressEvent( event );

  if (( event->key() == Qt::Key_Backspace || event->key() == Qt::Key_Delete ) && mDeleteSelectedButton->isEnabled() )
  {
    QgisApp::instance()->deleteSelected( mLayer, this );
  }
}

void QgsAttributeTableDialog::columnBoxInit()
{
  foreach ( QAction* a, mFilterColumnsMenu->actions() )
  {
    mFilterColumnsMenu->removeAction( a );
    mFilterActionMapper->removeMappings( a );
    mFilterButton->removeAction( a );
    delete a;
  }

  mFilterButton->addAction( mActionShowAllFilter );
  mFilterButton->addAction( mActionSelectedFilter );
  if ( mLayer->hasGeometryType() )
  {
    mFilterButton->addAction( mActionVisibleFilter );
  }
  mFilterButton->addAction( mActionEditedFilter );
  mFilterButton->addAction( mActionFilterColumnsMenu );
  mFilterButton->addAction( mActionAdvancedFilter );

  QList<QgsField> fields = mLayer->fields().toList();

  foreach ( const QgsField field, fields )
  {
    int idx = mLayer->fieldNameIndex( field.name() );
    if ( idx < 0 )
      continue;

    if ( mLayer->editorWidgetV2( idx ) != "Hidden" )
    {
      QIcon icon = QgsApplication::getThemeIcon( "/mActionNewAttribute.png" );
      QString text = field.name();

      // Generate action for the filter popup button
      QAction* filterAction = new QAction( icon, text, mFilterButton );
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
  runFieldCalculation( mLayer, mFieldCombo->currentText(), mUpdateExpressionText->currentField(), filteredIds );
}

void QgsAttributeTableDialog::updateFieldFromExpressionSelected()
{
  QgsFeatureIds filteredIds = mLayer->selectedFeaturesIds();
  runFieldCalculation( mLayer, mFieldCombo->currentText(), mUpdateExpressionText->currentField(), filteredIds );
}

void QgsAttributeTableDialog::runFieldCalculation( QgsVectorLayer* layer, QString fieldName, QString expression, QgsFeatureIds filteredIds )
{
  QApplication::setOverrideCursor( Qt::WaitCursor );

  mLayer->beginEditCommand( "Field calculator" );

  QModelIndex modelindex = mFieldModel->indexFromName( fieldName );
  int fieldindex = modelindex.data( QgsFieldModel::FieldIndexRole ).toInt();

  bool calculationSuccess = true;
  QString error;

  QgsExpression exp( expression );
  exp.setGeomCalculator( *myDa );
  bool useGeometry = exp.needsGeometry();

  QgsFeatureRequest request( mMainView->masterModel()->request() );
  useGeometry |= !request.filterRect().isNull();
  request.setFlags( useGeometry ? QgsFeatureRequest::NoFlags : QgsFeatureRequest::NoGeometry );

  int rownum = 1;

  QgsExpressionContext context;
  context << QgsExpressionContextUtils::globalScope()
  << QgsExpressionContextUtils::projectScope()
  << QgsExpressionContextUtils::layerScope( layer );

  QgsField fld = layer->fields()[ fieldindex ];

  //go through all the features and change the new attributes
  QgsFeatureIterator fit = layer->getFeatures( request );
  QgsFeature feature;
  while ( fit.nextFeature( feature ) )
  {
    if ( !filteredIds.isEmpty() && !filteredIds.contains( feature.id() ) )
    {
      continue;
    }

    context.setFeature( feature );
    context.lastScope()->setVariable( QString( "_rownum_" ), rownum );

    QVariant value = exp.evaluate( &context );
    fld.convertCompatible( value );
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

  QApplication::restoreOverrideCursor();

  if ( !calculationSuccess )
  {
    QMessageBox::critical( 0, tr( "Error" ), tr( "An error occured while evaluating the calculation string:\n%1" ).arg( error ) );
    mLayer->destroyEditCommand();
    return;
  }

  mLayer->endEditCommand();
}

void QgsAttributeTableDialog::replaceSearchWidget( QWidget* oldw, QWidget* neww )
{
  mFilterLayout->removeWidget( oldw );
  oldw->setVisible( false );
  mFilterLayout->addWidget( neww, 0, 0, 0 );
  neww->setVisible( true );
}

void QgsAttributeTableDialog::filterColumnChanged( QObject* filterAction )
{
  mFilterButton->setDefaultAction( qobject_cast<QAction *>( filterAction ) );
  mFilterButton->setPopupMode( QToolButton::InstantPopup );
  // replace the search line edit with a search widget that is suited to the selected field
  // delete previous widget
  if ( mCurrentSearchWidgetWrapper != 0 )
  {
    mCurrentSearchWidgetWrapper->widget()->setVisible( false );
    delete mCurrentSearchWidgetWrapper;
  }
  QString fieldName = mFilterButton->defaultAction()->text();
  // get the search widget
  int fldIdx = mLayer->fieldNameIndex( fieldName );
  if ( fldIdx < 0 )
    return;
  const QString widgetType = mLayer->editorWidgetV2( fldIdx );
  const QgsEditorWidgetConfig widgetConfig = mLayer->editorWidgetV2Config( fldIdx );
  mCurrentSearchWidgetWrapper = QgsEditorWidgetRegistry::instance()->
                                createSearchWidget( widgetType, mLayer, fldIdx, widgetConfig, mFilterContainer );
  if ( mCurrentSearchWidgetWrapper->applyDirectly() )
  {
    connect( mCurrentSearchWidgetWrapper, SIGNAL( expressionChanged( QString ) ), SLOT( filterQueryChanged( QString ) ) );
    mApplyFilterButton->setVisible( false );
  }
  else
  {
    mApplyFilterButton->setVisible( true );
  }

  replaceSearchWidget( mFilterQuery, mCurrentSearchWidgetWrapper->widget() );

}

void QgsAttributeTableDialog::filterExpressionBuilder()
{
  // Show expression builder
  QgsExpressionContext context;
  context << QgsExpressionContextUtils::globalScope()
  << QgsExpressionContextUtils::projectScope()
  << QgsExpressionContextUtils::layerScope( mLayer );

  QgsExpressionBuilderDialog dlg( mLayer, mFilterQuery->text(), this, "generic", context );
  dlg.setWindowTitle( tr( "Expression based filter" ) );

  QgsDistanceArea myDa;
  myDa.setSourceCrs( mLayer->crs().srsid() );
  myDa.setEllipsoidalMode( QgisApp::instance()->mapCanvas()->mapSettings().hasCrsTransformEnabled() );
  myDa.setEllipsoid( QgsProject::instance()->readEntry( "Measure", "/Ellipsoid", GEO_NONE ) );
  dlg.setGeomCalculator( myDa );

  if ( dlg.exec() == QDialog::Accepted )
  {
    setFilterExpression( dlg.expressionText() );
  }
}

void QgsAttributeTableDialog::filterShowAll()
{
  mFilterButton->setDefaultAction( mActionShowAllFilter );
  mFilterButton->setPopupMode( QToolButton::InstantPopup );
  mFilterQuery->setVisible( false );
  mApplyFilterButton->setVisible( false );
  mMainView->setFilterMode( QgsAttributeTableFilterModel::ShowAll );
  updateTitle();
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
  if ( !mLayer->hasGeometryType() )
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

void QgsAttributeTableDialog::on_mSelectedToTopButton_toggled()
{
  if ( mSelectedToTopButton->isChecked() )
  {
    mMainView->setSelectedOnTop( true );
  }
  else
  {
    mMainView->setSelectedOnTop( false );
  }
}

void QgsAttributeTableDialog::on_mOpenFieldCalculator_clicked()
{
  QgsAttributeTableModel* masterModel = mMainView->masterModel();

  QgsFieldCalculator calc( mLayer );
  if ( calc.exec() == QDialog::Accepted )
  {
    int col = masterModel->fieldCol( calc.changedAttributeId() );

    if ( col >= 0 )
    {
      masterModel->reload( masterModel->index( 0, col ), masterModel->index( masterModel->rowCount() - 1, col ) );
    }
  }
}

void QgsAttributeTableDialog::on_mSaveEditsButton_clicked()
{
  QgisApp::instance()->saveEdits( mLayer, true, true );
}

void QgsAttributeTableDialog::on_mAddFeature_clicked()
{
  if ( !mLayer->isEditable() )
    return;

  QgsAttributeTableModel* masterModel = mMainView->masterModel();

  QgsFeature f;
  QgsFeatureAction action( tr( "Geometryless feature added" ), f, mLayer, -1, -1, this );
  if ( action.addFeature() )
  {
    masterModel->reload( masterModel->index( 0, 0 ), masterModel->index( masterModel->rowCount() - 1, masterModel->columnCount() - 1 ) );
  }
}

void QgsAttributeTableDialog::on_mExpressionSelectButton_clicked()
{
  QgsExpressionSelectionDialog* dlg = new QgsExpressionSelectionDialog( mLayer );
  dlg->setAttribute( Qt::WA_DeleteOnClose );
  dlg->show();
}

void QgsAttributeTableDialog::on_mCopySelectedRowsButton_clicked()
{
  QgisApp::instance()->editCopy( mLayer );
}

void QgsAttributeTableDialog::on_mPasteFeatures_clicked()
{
  QgisApp::instance()->editPaste( mLayer );
}


void QgsAttributeTableDialog::on_mZoomMapToSelectedRowsButton_clicked()
{
  QgisApp::instance()->mapCanvas()->zoomToSelected( mLayer );
}

void QgsAttributeTableDialog::on_mPanMapToSelectedRowsButton_clicked()
{
  QgisApp::instance()->mapCanvas()->panToSelected( mLayer );
}

void QgsAttributeTableDialog::on_mInvertSelectionButton_clicked()
{
  mLayer->invertSelection();
}

void QgsAttributeTableDialog::on_mRemoveSelectionButton_clicked()
{
  mLayer->removeSelection();
}

void QgsAttributeTableDialog::on_mDeleteSelectedButton_clicked()
{
  QgisApp::instance()->deleteSelected( mLayer, this );
}

void QgsAttributeTableDialog::on_mMainView_currentChanged( int viewMode )
{
  mMainViewButtonGroup->button( viewMode )->click();
}

void QgsAttributeTableDialog::on_mToggleEditingButton_toggled()
{
  if ( !mLayer )
    return;
  if ( !QgisApp::instance()->toggleEditing( mLayer ) )
  {
    // restore gui state if toggling was canceled or layer commit/rollback failed
    editingToggled();
  }
}

void QgsAttributeTableDialog::editingToggled()
{
  mToggleEditingButton->blockSignals( true );
  mToggleEditingButton->setChecked( mLayer->isEditable() );
  mSaveEditsButton->setEnabled( mLayer->isEditable() );
  mToggleEditingButton->blockSignals( false );

  bool canChangeAttributes = mLayer->dataProvider()->capabilities() & QgsVectorDataProvider::ChangeAttributeValues;
  bool canDeleteFeatures = mLayer->dataProvider()->capabilities() & QgsVectorDataProvider::DeleteFeatures;
  bool canAddAttributes = mLayer->dataProvider()->capabilities() & QgsVectorDataProvider::AddAttributes;
  bool canAddFeatures = mLayer->dataProvider()->capabilities() & QgsVectorDataProvider::AddFeatures;
  mAddAttribute->setEnabled(( canChangeAttributes || canAddAttributes ) && mLayer->isEditable() );
  mDeleteSelectedButton->setEnabled( canDeleteFeatures && mLayer->isEditable() );
  mAddFeature->setEnabled( canAddFeatures && mLayer->isEditable() && mLayer->geometryType() == QGis::NoGeometry );

  mUpdateExpressionBox->setVisible( mLayer->isEditable() );
  // not necessary to set table read only if layer is not editable
  // because model always reflects actual state when returning item flags
}

void QgsAttributeTableDialog::on_mAddAttribute_clicked()
{
  if ( !mLayer )
  {
    return;
  }

  QgsAttributeTableModel* masterModel = mMainView->masterModel();

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
      QMessageBox::critical( this, tr( "Failed to add field" ), tr( "Failed to add field '%1' of type '%2'. Is the field name unique?" ).arg( dialog.field().name() ).arg( dialog.field().typeName() ) );
    }


    // update model - a field has been added or updated
    masterModel->reload( masterModel->index( 0, 0 ), masterModel->index( masterModel->rowCount() - 1, masterModel->columnCount() - 1 ) );
    columnBoxInit();
  }
}

void QgsAttributeTableDialog::on_mRemoveAttribute_clicked()
{
  if ( !mLayer )
  {
    return;
  }

  QgsDelAttrDialog dialog( mLayer );
  if ( dialog.exec() == QDialog::Accepted )
  {
    QList<int> attributes = dialog.selectedAttributes();
    if ( attributes.size() < 1 )
    {
      return;
    }

    QgsAttributeTableModel* masterModel = mMainView->masterModel();

    mLayer->beginEditCommand( tr( "Deleted attribute" ) );
    if ( mLayer->deleteAttributes( attributes ) )
    {
      mLayer->endEditCommand();
    }
    else
    {
      QgisApp::instance()->messageBar()->pushMessage( tr( "Attribute error" ), tr( "The attribute(s) could not be deleted" ), QgsMessageBar::WARNING, QgisApp::instance()->messageTimeout() );
      mLayer->destroyEditCommand();
    }
    // update model - a field has been added or updated
    masterModel->reload( masterModel->index( 0, 0 ), masterModel->index( masterModel->rowCount() - 1, masterModel->columnCount() - 1 ) );
    columnBoxInit();
  }
}

void QgsAttributeTableDialog::filterQueryChanged( const QString& query )
{
  QString str;
  if ( mFilterButton->defaultAction() == mActionAdvancedFilter )
  {
    str = query;
  }
  else
  {
    str = mCurrentSearchWidgetWrapper->expression();
  }

  setFilterExpression( str );
  updateTitle();
}

void QgsAttributeTableDialog::filterQueryAccepted()
{
  if (( mFilterQuery->isVisible() && mFilterQuery->text().isEmpty() ) ||
      ( mCurrentSearchWidgetWrapper != 0 && mCurrentSearchWidgetWrapper->widget()->isVisible()
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

void QgsAttributeTableDialog::setFilterExpression( QString filterString )
{
  if ( mCurrentSearchWidgetWrapper == 0 || !mCurrentSearchWidgetWrapper->applyDirectly() )
  {
    mFilterQuery->setText( filterString );
    mFilterButton->setDefaultAction( mActionAdvancedFilter );
    mFilterButton->setPopupMode( QToolButton::MenuButtonPopup );
    mFilterQuery->setVisible( true );
    mApplyFilterButton->setVisible( true );
    if ( mCurrentSearchWidgetWrapper != 0 )
    {
      // replace search widget widget with the normal filter query line edit
      replaceSearchWidget( mCurrentSearchWidgetWrapper->widget(), mFilterQuery );
    }
  }

  mMainView->setFilterMode( QgsAttributeTableFilterModel::ShowFilteredList );

  QgsFeatureIds filteredFeatures;
  QgsDistanceArea myDa;

  myDa.setSourceCrs( mLayer->crs().srsid() );
  myDa.setEllipsoidalMode( QgisApp::instance()->mapCanvas()->mapSettings().hasCrsTransformEnabled() );
  myDa.setEllipsoid( QgsProject::instance()->readEntry( "Measure", "/Ellipsoid", GEO_NONE ) );

  // parse search string and build parsed tree
  QgsExpression filterExpression( filterString );
  if ( filterExpression.hasParserError() )
  {
    QgisApp::instance()->messageBar()->pushMessage( tr( "Parsing error" ), filterExpression.parserErrorString(), QgsMessageBar::WARNING, QgisApp::instance()->messageTimeout() );
    return;
  }

  QgsExpressionContext context;
  context << QgsExpressionContextUtils::globalScope()
  << QgsExpressionContextUtils::projectScope()
  << QgsExpressionContextUtils::layerScope( mLayer );

  if ( ! filterExpression.prepare( &context ) )
  {
    QgisApp::instance()->messageBar()->pushMessage( tr( "Evaluation error" ), filterExpression.evalErrorString(), QgsMessageBar::WARNING, QgisApp::instance()->messageTimeout() );
  }

  bool fetchGeom = filterExpression.needsGeometry();

  QApplication::setOverrideCursor( Qt::WaitCursor );

  filterExpression.setGeomCalculator( myDa );
  QgsFeatureRequest request( mMainView->masterModel()->request() );
  request.setSubsetOfAttributes( filterExpression.referencedColumns(), mLayer->fields() );
  if ( !fetchGeom )
  {
    request.setFlags( QgsFeatureRequest::NoGeometry );
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
    QgisApp::instance()->messageBar()->pushMessage( tr( "Error filtering" ), filterExpression.evalErrorString(), QgsMessageBar::WARNING, QgisApp::instance()->messageTimeout() );
    return;
  }
}
