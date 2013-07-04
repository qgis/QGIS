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

#include <QtGui>

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

class QgsAttributeTableDock : public QDockWidget
{
  public:
    QgsAttributeTableDock( const QString & title, QWidget * parent = 0, Qt::WindowFlags flags = 0 )
        : QDockWidget( title, parent, flags )
    {
      setObjectName( "AttributeTable" ); // set object name so the position can be saved
    }

    virtual void closeEvent( QCloseEvent * ev )
    {
      Q_UNUSED( ev );
      deleteLater();
    }
};

QgsAttributeTableDialog::QgsAttributeTableDialog( QgsVectorLayer *theLayer, QWidget *parent, Qt::WindowFlags flags )
    : QDialog( parent, flags )
    , mDock( 0 )
    , mLayer( theLayer )
{
  setupUi( this );

  setAttribute( Qt::WA_DeleteOnClose );

  QSettings settings;

  // Initialize the window geometry
  restoreGeometry( settings.value( "/Windows/BetterAttributeTable/geometry" ).toByteArray() );


  QgsDistanceArea myDa;

  myDa.setSourceCrs( mLayer->crs().srsid() );
  myDa.setEllipsoidalMode( QgisApp::instance()->mapCanvas()->mapRenderer()->hasCrsTransformEnabled() );
  myDa.setEllipsoid( QgsProject::instance()->readEntry( "Measure", "/Ellipsoid", GEO_NONE ) );

  // Initialize dual view
  mMainView->init( mLayer, QgisApp::instance()->mapCanvas(), myDa );

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

  // info from layer to table
  connect( mLayer, SIGNAL( editingStarted() ), this, SLOT( editingToggled() ) );
  connect( mLayer, SIGNAL( editingStopped() ), this, SLOT( editingToggled() ) );
  connect( mLayer, SIGNAL( layerDeleted() ), this, SLOT( close() ) );
  connect( mLayer, SIGNAL( selectionChanged() ), this, SLOT( updateTitle() ) );
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

  // toggle editing
  bool canChangeAttributes = mLayer->dataProvider()->capabilities() & QgsVectorDataProvider::ChangeAttributeValues;
  bool canDeleteFeatures = mLayer->dataProvider()->capabilities() & QgsVectorDataProvider::DeleteFeatures;
  bool canAddAttributes = mLayer->dataProvider()->capabilities() & QgsVectorDataProvider::AddAttributes;
  bool canDeleteAttributes = mLayer->dataProvider()->capabilities() & QgsVectorDataProvider::DeleteAttributes;
  bool canAddFeatures = mLayer->dataProvider()->capabilities() & QgsVectorDataProvider::AddFeatures;

  mToggleEditingButton->blockSignals( true );
  mToggleEditingButton->setCheckable( true );
  mToggleEditingButton->setChecked( mLayer->isEditable() );
  mToggleEditingButton->setEnabled( canChangeAttributes && !mLayer->isReadOnly() );
  mToggleEditingButton->blockSignals( false );

  mSaveEditsButton->setEnabled( canChangeAttributes && mLayer->isEditable() );
  mOpenFieldCalculator->setEnabled(( canChangeAttributes || canAddAttributes ) && mLayer->isEditable() );
  mDeleteSelectedButton->setEnabled( canDeleteFeatures && mLayer->isEditable() );
  mAddAttribute->setEnabled( canAddAttributes && mLayer->isEditable() );
  mRemoveAttribute->setEnabled( canDeleteAttributes && mLayer->isEditable() );
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
}

QgsAttributeTableDialog::~QgsAttributeTableDialog()
{
}

void QgsAttributeTableDialog::updateTitle()
{
  QWidget *w = mDock ? qobject_cast<QWidget*>( mDock ) : qobject_cast<QWidget*>( this );
  w->setWindowTitle( tr( "Attribute table - %1 :: Features total: %2, filtered: %3, selected: %4" )
                     .arg( mLayer->name() )
                     .arg( mMainView->featureCount() )
                     .arg( mMainView->filteredFeatureCount() )
                     .arg( mLayer->selectedFeatureCount() )
                   );
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

  QList<QgsField> fields = mLayer->pendingFields().toList();

  foreach ( const QgsField field, fields )
  {
    if ( mLayer->editType( mLayer->fieldNameIndex( field.name() ) ) != QgsVectorLayer::Hidden )
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

void QgsAttributeTableDialog::filterColumnChanged( QObject* filterAction )
{
  mFilterButton->setDefaultAction( qobject_cast<QAction *>( filterAction ) );
  mFilterButton->setPopupMode( QToolButton::InstantPopup );
  mCbxCaseSensitive->setVisible( true );
  mFilterQuery->setVisible( true );
  mApplyFilterButton->setVisible( true );
}

void QgsAttributeTableDialog::filterExpressionBuilder()
{
  // Show expression builder
  QgsExpressionBuilderDialog dlg( mLayer, mFilterQuery->text() , this );
  dlg.setWindowTitle( tr( "Expression based filter" ) );

  QgsDistanceArea myDa;
  myDa.setSourceCrs( mLayer->crs().srsid() );
  myDa.setEllipsoidalMode( QgisApp::instance()->mapCanvas()->mapRenderer()->hasCrsTransformEnabled() );
  myDa.setEllipsoid( QgsProject::instance()->readEntry( "Measure", "/Ellipsoid", GEO_NONE ) );
  dlg.setGeomCalculator( myDa );

  if ( dlg.exec() == QDialog::Accepted )
  {
    mFilterQuery->setText( dlg.expressionText() );
    mFilterButton->setDefaultAction( mActionAdvancedFilter );
    mFilterButton->setPopupMode( QToolButton::MenuButtonPopup );
    mCbxCaseSensitive->setVisible( false );
    mFilterQuery->setVisible( true );
    mApplyFilterButton->setVisible( true );
    mMainView->setFilterMode( QgsAttributeTableFilterModel::ShowFilteredList );
    setFilterExpression( dlg.expressionText() );
  }
}

void QgsAttributeTableDialog::filterShowAll()
{
  mFilterButton->setDefaultAction( mActionShowAllFilter );
  mFilterButton->setPopupMode( QToolButton::InstantPopup );
  mCbxCaseSensitive->setVisible( false );
  mFilterQuery->setVisible( false );
  mApplyFilterButton->setVisible( false );
  mMainView->setFilterMode( QgsAttributeTableFilterModel::ShowAll );
}

void QgsAttributeTableDialog::filterSelected()
{
  mFilterButton->setDefaultAction( mActionSelectedFilter );
  mFilterButton->setPopupMode( QToolButton::InstantPopup );
  mCbxCaseSensitive->setVisible( false );
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
  mCbxCaseSensitive->setVisible( false );
  mFilterQuery->setVisible( false );
  mApplyFilterButton->setVisible( false );
  mMainView->setFilterMode( QgsAttributeTableFilterModel::ShowVisible );
}

void QgsAttributeTableDialog::filterEdited()
{
  mFilterButton->setDefaultAction( mActionEditedFilter );
  mFilterButton->setPopupMode( QToolButton::InstantPopup );
  mCbxCaseSensitive->setVisible( false );
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
  bool canDeleteAttributes = mLayer->dataProvider()->capabilities() & QgsVectorDataProvider::DeleteAttributes;
  bool canAddFeatures = mLayer->dataProvider()->capabilities() & QgsVectorDataProvider::AddFeatures;
  mOpenFieldCalculator->setEnabled(( canChangeAttributes || canAddAttributes ) && mLayer->isEditable() );
  mDeleteSelectedButton->setEnabled( canDeleteFeatures && mLayer->isEditable() );
  mAddAttribute->setEnabled( canAddAttributes && mLayer->isEditable() );
  mRemoveAttribute->setEnabled( canDeleteAttributes && mLayer->isEditable() );
  mAddFeature->setEnabled( canAddFeatures && mLayer->isEditable() && mLayer->geometryType() == QGis::NoGeometry );

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
      QMessageBox::critical( 0, tr( "Attribute Error" ), tr( "The attribute could not be added to the layer" ) );
      mLayer->destroyEditCommand();
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
    QString fieldName = mFilterButton->defaultAction()->text();

    const QgsFields& flds = mLayer->pendingFields();
    int fldIndex = mLayer->fieldNameIndex( fieldName );
    QVariant::Type fldType = flds[fldIndex].type();
    bool numeric = ( fldType == QVariant::Int || fldType == QVariant::Double );

    QString sensString = "ILIKE";
    if ( mCbxCaseSensitive->isChecked() )
    {
      sensString = "LIKE";
    }

    QSettings settings;
    QString nullValue = settings.value( "qgis/nullValue", "NULL" ).toString();

    if ( mFilterQuery->displayText() == nullValue )
    {
      str = QString( "%1 IS NULL" ).arg( QgsExpression::quotedColumnRef( fieldName ) );
    }
    else
    {
      str = QString( "%1 %2 '%3'" )
            .arg( QgsExpression::quotedColumnRef( fieldName ) )
            .arg( numeric ? "=" : sensString )
            .arg( numeric
                  ? mFilterQuery->displayText().replace( "'", "''" )
                  :
                  "%" + mFilterQuery->displayText().replace( "'", "''" ) + "%" ); // escape quotes
    }
  }

  setFilterExpression( str );
  updateTitle();
}

void QgsAttributeTableDialog::filterQueryAccepted()
{
  if ( mFilterQuery->text().isEmpty() )
  {
    filterShowAll();
    return;
  }
  filterQueryChanged( mFilterQuery->text() );
}

void QgsAttributeTableDialog::setFilterExpression( QString filterString )
{
  QgsFeatureIds filteredFeatures;
  QgsDistanceArea myDa;

  myDa.setSourceCrs( mLayer->crs().srsid() );
  myDa.setEllipsoidalMode( QgisApp::instance()->mapCanvas()->mapRenderer()->hasCrsTransformEnabled() );
  myDa.setEllipsoid( QgsProject::instance()->readEntry( "Measure", "/Ellipsoid", GEO_NONE ) );

  // parse search string and build parsed tree
  QgsExpression filterExpression( filterString );
  if ( filterExpression.hasParserError() )
  {
    QgisApp::instance()->messageBar()->pushMessage( tr( "Parsing error" ), filterExpression.parserErrorString(), QgsMessageBar::WARNING, QgisApp::instance()->messageTimeout() );
    return;
  }

  if ( ! filterExpression.prepare( mLayer->pendingFields() ) )
  {
    QgisApp::instance()->messageBar()->pushMessage( tr( "Evaluation error" ), filterExpression.evalErrorString(), QgsMessageBar::WARNING, QgisApp::instance()->messageTimeout() );
  }

  // TODO: fetch only necessary columns
  // QStringList columns = search.referencedColumns();
  bool fetchGeom = filterExpression.needsGeometry();

  QApplication::setOverrideCursor( Qt::WaitCursor );

  filterExpression.setGeomCalculator( myDa );
  QgsFeatureRequest request;
  if ( !fetchGeom )
  {
    request.setFlags( QgsFeatureRequest::NoGeometry );
  }
  QgsFeatureIterator featIt = mLayer->getFeatures( request );

  QgsFeature f;

  while ( featIt.nextFeature( f ) )
  {
    if ( filterExpression.evaluate( &f ).toInt() != 0 )
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
