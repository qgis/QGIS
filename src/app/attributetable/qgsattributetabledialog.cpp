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
#include <qgssearchstring.h>
#include <qgssearchtreenode.h>

#include "qgisapp.h"
#include "qgssearchquerybuilder.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsfieldcalculator.h"

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
      deleteLater();
    }
};


QgsAttributeTableDialog::QgsAttributeTableDialog( QgsVectorLayer *theLayer, QWidget *parent, Qt::WindowFlags flags )
    : QDialog( parent, flags ), mDock( NULL )
{
  mLayer = theLayer;

  setupUi( this );

  setAttribute( Qt::WA_DeleteOnClose );

  QSettings settings;
  restoreGeometry( settings.value( "/Windows/BetterAttributeTable/geometry" ).toByteArray() );

  mView->setLayer( mLayer );
  mFilterModel = ( QgsAttributeTableFilterModel * ) mView->model();
  mModel = ( QgsAttributeTableModel * )(( QgsAttributeTableFilterModel * )mView->model() )->sourceModel();

  mQuery = query;
  mColumnBox = columnBox;
  columnBoxInit();

  QSettings mySettings;
  bool myDockFlag = mySettings.value( "/qgis/dockAttributeTable", false ).toBool();
  if ( myDockFlag )
  {
    mDock = new QgsAttributeTableDock( tr( "Attribute table - %1" ).arg( mLayer->name() ), QgisApp::instance() );
    mDock->setAllowedAreas( Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea );
    mDock->setWidget( this );
    QgisApp::instance()->addDockWidget( Qt::BottomDockWidgetArea, mDock );
  }

  setWindowTitle( tr( "Attribute table - %1" ).arg( mLayer->name() ) );

  mRemoveSelectionButton->setIcon( getThemeIcon( "/mActionUnselectAttributes.png" ) );
  mSelectedToTopButton->setIcon( getThemeIcon( "/mActionSelectedToTop.png" ) );
  mCopySelectedRowsButton->setIcon( getThemeIcon( "/mActionCopySelected.png" ) );
  mZoomMapToSelectedRowsButton->setIcon( getThemeIcon( "/mActionZoomToSelected.png" ) );
  mInvertSelectionButton->setIcon( getThemeIcon( "/mActionInvertSelection.png" ) );
  mToggleEditingButton->setIcon( getThemeIcon( "/mActionToggleEditing.png" ) );
  mOpenFieldCalculator->setIcon( getThemeIcon( "/mActionCalculateField.png" ) );
  // toggle editing
  bool canChangeAttributes = mLayer->dataProvider()->capabilities() & QgsVectorDataProvider::ChangeAttributeValues;
  mToggleEditingButton->setCheckable( true );
  mToggleEditingButton->setEnabled( canChangeAttributes );
  mOpenFieldCalculator->setEnabled( canChangeAttributes && mLayer->isEditable() );

  // info from table to application
  connect( this, SIGNAL( editingToggled( QgsMapLayer * ) ), QgisApp::instance(), SLOT( toggleEditing( QgsMapLayer * ) ) );
  // info from layer to table
  connect( mLayer, SIGNAL( editingStarted() ), this, SLOT( editingToggled() ) );
  connect( mLayer, SIGNAL( editingStopped() ), this, SLOT( editingToggled() ) );

  connect( searchButton, SIGNAL( clicked() ), this, SLOT( search() ) );

  connect( mLayer, SIGNAL( selectionChanged() ), this, SLOT( updateSelectionFromLayer() ) );
  connect( mLayer, SIGNAL( layerDeleted() ), this, SLOT( close() ) );
  connect( mView->verticalHeader(), SIGNAL( sectionClicked( int ) ), this, SLOT( updateRowSelection( int ) ) );
  connect( mModel, SIGNAL( modelChanged() ), this, SLOT( updateSelection() ) );

  mLastClickedHeaderIndex = 0;
  mSelectionModel = new QItemSelectionModel( mFilterModel );
  updateSelectionFromLayer();

  //make sure to show all recs on first load
  on_cbxShowSelectedOnly_toggled( false );
}

QgsAttributeTableDialog::~QgsAttributeTableDialog()
{
  delete mSelectionModel;
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


QIcon QgsAttributeTableDialog::getThemeIcon( const QString theName )
{
  // copied from QgisApp::getThemeIcon. To be removed when merged to SVN

  QString myPreferredPath = QgsApplication::activeThemePath() + QDir::separator() + theName;
  QString myDefaultPath = QgsApplication::defaultThemePath() + QDir::separator() + theName;
  if ( QFile::exists( myPreferredPath ) )
  {
    return QIcon( myPreferredPath );
  }
  else if ( QFile::exists( myDefaultPath ) )
  {
    //could still return an empty icon if it
    //doesnt exist in the default theme either!
    return QIcon( myDefaultPath );
  }
  else
  {
    return QIcon();
  }
}

void QgsAttributeTableDialog::showAdvanced()
{
  mMenuActions->exec( QCursor::pos() );
}

void QgsAttributeTableDialog::on_mSelectedToTopButton_clicked()
{
  int freeIndex = 0;

  //QgsFeatureIds fids = mSelectedFeatures;
  //QgsFeatureIds::Iterator it = fids.begin();

  mModel->incomingChangeLayout();

  QgsFeatureIds::Iterator it = mSelectedFeatures.begin();
  for ( ; it != mSelectedFeatures.end(); ++it, ++freeIndex )
  {
    QModelIndex sourceIndex = mFilterModel->mapToSource( mFilterModel->index( freeIndex, 0 ) );
    mModel->swapRows( mModel->rowToId( sourceIndex.row() ), *it );
  }

  /*
    while (it != fids.end())
    { //map!!!!
      //mModel->swapRows(mModel->rowToId(freeIndex), *it);
      //QModelIndex index = mFilterModel->mapFromSource(mModel->index(mModel->idToRow(*it), 0));
      QModelIndex sourceIndex = mFilterModel->mapToSource(mFilterModel->index(freeIndex, 0));
      mModel->swapRows(mModel->rowToId(sourceIndex.row()), *it);
      //mModel->swapRows(freeIndex, *it);

      if (fids.empty())
        break;
      else
        ++it;

      ++freeIndex;
    }
  */
  // just select proper rows
  //mModel->reload(mModel->index(0,0), mModel->index(mModel->rowCount(), mModel->columnCount()));
  //mModel->changeLayout();
  mModel->resetModel();
  updateSelection();
}

void QgsAttributeTableDialog::on_mCopySelectedRowsButton_clicked()
{
  QgisApp::instance()->editCopy( mLayer );
}

void QgsAttributeTableDialog::on_mZoomMapToSelectedRowsButton_clicked()
{
  QgisApp::instance()->mapCanvas()->zoomToSelected( mLayer );
}

void QgsAttributeTableDialog::on_mInvertSelectionButton_clicked()
{
  mLayer->invertSelection();
}

void QgsAttributeTableDialog::on_mRemoveSelectionButton_clicked()
{
  mLayer->removeSelection();
}

void QgsAttributeTableDialog::on_cbxShowSelectedOnly_toggled( bool theFlag )
{
  mFilterModel->setHideUnselected( theFlag );
  mFilterModel->invalidate();
  //TODO: weird
  //mModel->changeLayout();
  updateSelection();
}

void QgsAttributeTableDialog::columnBoxInit()
{
  QgsFieldMap fieldMap = mLayer->dataProvider()->fields();
  QgsFieldMap::Iterator it = fieldMap.begin();

  for ( ; it != fieldMap.end(); ++it )
    mColumnBox->addItem( it.value().name() );
}

int QgsAttributeTableDialog::columnBoxColumnId()
{
  QgsFieldMap fieldMap = mLayer->dataProvider()->fields();
  QgsFieldMap::Iterator it = fieldMap.begin();

  for ( ; it != fieldMap.end(); ++it )
    if ( it.value().name() == mColumnBox->currentText() )
      return it.key();

  return 0;
}

void QgsAttributeTableDialog::updateSelection()
{
  QModelIndex index;
  mView->setSelectionMode( QAbstractItemView::MultiSelection );

  QItemSelection selection;

  QgsFeatureIds::Iterator it = mSelectedFeatures.begin();
  for ( ; it != mSelectedFeatures.end(); ++it )
  {
    QModelIndex leftUpIndex = mFilterModel->mapFromSource( mModel->index( mModel->idToRow( *it ), 0 ) );
    QModelIndex rightBottomIndex = mFilterModel->mapFromSource( mModel->index( mModel->idToRow( *it ), mModel->columnCount() - 1 ) );
    selection.append( QItemSelectionRange( leftUpIndex, rightBottomIndex ) );
    //selection.append(QItemSelectionRange(leftUpIndex, leftUpIndex));
  }

  mSelectionModel->select( selection, QItemSelectionModel::ClearAndSelect );// | QItemSelectionModel::Columns);
  mView->setSelectionModel( mSelectionModel );
  mView->setSelectionMode( QAbstractItemView::NoSelection );

  /*for (int i = 0; i < mModel->rowCount(); ++i)
  {
  int id = mModel->rowToId(i);
    QgsDebugMsg(id);
  }
  QgsDebugMsg("--------------");
  */
}

void QgsAttributeTableDialog::updateRowSelection( int index )
{
  // map index to filter model
  //index = mFilterModel->mapFromSource(mModel->index(index, 0)).row();

  if ( mView->shiftPressed() )
  {
    QgsDebugMsg( "shift" );
    // get the first and last index of the rows to be selected/deselected
    int first, last;
    if ( index > mLastClickedHeaderIndex )
    {
      first = mLastClickedHeaderIndex + 1;
      last = index;
    }
    else if ( index == mLastClickedHeaderIndex )
    {
      // row was selected and now it is shift-clicked
      // ignore the shift and deselect the row
      first = last = index;
    }
    else
    {
      first = index;
      last = mLastClickedHeaderIndex - 1;
    }

    // for all the rows update the selection, without starting a new selection
    if ( first <= last )
      updateRowSelection( first, last, false );

    mLastClickedHeaderIndex = last;
  }
  else if ( mView->ctrlPressed() )
  {
    QgsDebugMsg( "ctrl" );
    // update the single row selection, without starting a new selection
    updateRowSelection( index, index, false );

    // the next shift would start from here
    mLastClickedHeaderIndex = index;
  }
  else
  {
    QgsDebugMsg( "ordinary click" );
    // update the single row selection, start a new selection if the row was not selected
    updateRowSelection( index, index, true );

    // the next shift would start from here
    mLastClickedHeaderIndex = index;
  }
}

// fast row deselection needed
void QgsAttributeTableDialog::updateRowSelection( int first, int last, bool startNewSelection )
{
  disconnect( mLayer, SIGNAL( selectionChanged() ), this, SLOT( updateSelectionFromLayer() ) );

  //index = mFilterModel->mapFromSource(mModel->index(index, 0)).row();
  // Id must be mapped to table/view row
  QModelIndex index = mFilterModel->mapToSource( mFilterModel->index( first, 0 ) );
  int fid = mModel->rowToId( index.row() );
  bool wasSelected = mSelectedFeatures.contains( fid );

  // new selection should be created
  if ( startNewSelection )
  {
    mView->clearSelection();
    mSelectedFeatures.clear();

    if ( wasSelected )
    {
      mLayer->removeSelection();
      connect( mLayer, SIGNAL( selectionChanged() ), this, SLOT( updateSelectionFromLayer() ) );
      return;
    }

    // set clicked row to current
    mView->setCurrentIndex( mFilterModel->index( first, 0 ) );
    mView->setSelectionMode( QAbstractItemView::SingleSelection );

    //QModelIndex index = mFilterModel->mapFromSource(mModel->index(first, 0));

    mView->selectRow( first );
    mView->setSelectionMode( QAbstractItemView::NoSelection );

    mSelectedFeatures.insert( fid );
    //mLayer->setSelectedFeatures(mSelectedFeatures);
    mLayer->removeSelection();
    mLayer->select( fid );
    //mFilterModel->invalidate();
    connect( mLayer, SIGNAL( selectionChanged() ), this, SLOT( updateSelectionFromLayer() ) );
    return;
  }

  // existing selection should be updated
  for ( int i = first; i <= last; ++i )
  {
    if ( i > first )
    {
      // Id must be mapped to table/view row
      index = mFilterModel->mapToSource( mFilterModel->index( i, 0 ) );
      fid = mModel->rowToId( index.row() );
      wasSelected = mSelectedFeatures.contains( fid );
    }

    if ( wasSelected )
      mSelectedFeatures.remove( fid );
    else
      mSelectedFeatures.insert( fid );
  }
  //mFilterModel->invalidate();

  /*
  QItemSelection selection;
  QModelIndex leftUpIndex = mFilterModel->index(first, 0);
  QModelIndex rightBottomIndex = mFilterModel->index(last, mModel->columnCount() - 1);
  selection.append(QItemSelectionRange(leftUpIndex, rightBottomIndex));
  mSelectionModel->select(selection, QItemSelectionModel::Select);
  mView->setSelectionModel(mSelectionModel);
  */
  updateSelection();
  mLayer->setSelectedFeatures( mSelectedFeatures );
  connect( mLayer, SIGNAL( selectionChanged() ), this, SLOT( updateSelectionFromLayer() ) );
}

void QgsAttributeTableDialog::updateSelectionFromLayer()
{
  QgsDebugMsg( "updateFromLayer" );
  mSelectedFeatures = mLayer->selectedFeaturesIds();
  updateSelection();
}

void QgsAttributeTableDialog::doSearch( QString searchString )
{
  // parse search string and build parsed tree
  QgsSearchString search;
  if ( !search.setString( searchString ) )
  {
    QMessageBox::critical( this, tr( "Search string parsing error" ), search.parserErrorMsg() );
    return;
  }

  QgsSearchTreeNode* searchTree = search.tree();
  if ( searchTree == NULL )
  {
    QMessageBox::information( this, tr( "Search results" ), tr( "You've supplied an empty search string." ) );
    return;
  }

  QApplication::setOverrideCursor( Qt::WaitCursor );
  mSelectedFeatures.clear();

  if ( cbxSearchSelectedOnly->isChecked() )
  {
    QgsFeatureList selectedFeatures = mLayer->selectedFeatures();
    for ( QgsFeatureList::ConstIterator it = selectedFeatures.begin(); it != selectedFeatures.end(); ++it )
    {
      if ( searchTree->checkAgainst( mLayer->pendingFields(), it->attributeMap() ) )
        mSelectedFeatures << it->id();

      // check if there were errors during evaluating
      if ( searchTree->hasError() )
        break;
    }
  }
  else
  {
    mLayer->select( mLayer->pendingAllAttributesList(), QgsRectangle(), false );
    QgsFeature f;

    while ( mLayer->nextFeature( f ) )
    {
      if ( searchTree->checkAgainst( mLayer->pendingFields(), f.attributeMap() ) )
        mSelectedFeatures << f.id();

      // check if there were errors during evaluating
      if ( searchTree->hasError() )
        break;
    }
  }

  QApplication::restoreOverrideCursor();

  if ( searchTree->hasError() )
  {
    QMessageBox::critical( this, tr( "Error during search" ), searchTree->errorMsg() );
    return;
  }

  // update view
  updateSelection();

  disconnect( mLayer, SIGNAL( selectionChanged() ), this, SLOT( updateSelectionFromLayer() ) );
  mLayer->setSelectedFeatures( mSelectedFeatures );
  connect( mLayer, SIGNAL( selectionChanged() ), this, SLOT( updateSelectionFromLayer() ) );

  QString str;
  if ( mSelectedFeatures.size() )
    str.sprintf( tr( "Found %d matching features.", "", mSelectedFeatures.size() ).toUtf8(), mSelectedFeatures.size() );
  else
    str = tr( "No matching features found." );

  QgisApp::instance()->statusBar()->showMessage( str );
}

void QgsAttributeTableDialog::search()
{

  QString str = mColumnBox->currentText();

  const QgsFieldMap& flds = mLayer->dataProvider()->fields();
  int fldIndex = mLayer->dataProvider()->fieldNameIndex( str );
  QVariant::Type fldType = flds[fldIndex].type();
  bool numeric = ( fldType == QVariant::Int || fldType == QVariant::Double );

  if ( numeric )
    str += " = '";
  else
    str += " ~ '";

  str += mQuery->displayText();
  str += "'";

  doSearch( str );
}

void QgsAttributeTableDialog::on_mAdvancedSearchButton_clicked()
{
  QgsSearchQueryBuilder dlg( mLayer, this );
  dlg.setSearchString( mQuery->displayText() );

  if ( dlg.exec() )
    doSearch( dlg.searchString() );
}

void QgsAttributeTableDialog::on_mToggleEditingButton_toggled()
{
  emit editingToggled( mLayer );
}

void QgsAttributeTableDialog::editingToggled()
{
  mToggleEditingButton->blockSignals( true );
  mToggleEditingButton->setChecked( mLayer->isEditable() );
  mToggleEditingButton->blockSignals( false );

  bool canChangeAttributes = mLayer->dataProvider()->capabilities() & QgsVectorDataProvider::ChangeAttributeValues;
  mOpenFieldCalculator->setEnabled( canChangeAttributes && mLayer->isEditable() );

  // (probably reload data if user stopped editing - possible revert)
  mModel->reload( mModel->index( 0, 0 ), mModel->index( mModel->rowCount(), mModel->columnCount() ) );

  // not necessary to set table read only if layer is not editable
  // because model always reflects actual state when returning item flags
}

// not used now
void QgsAttributeTableDialog::startEditing()
{
  mLayer->startEditing();
}

// not used now
void QgsAttributeTableDialog::submit()
{
  mLayer->commitChanges();
}

// not used now
void QgsAttributeTableDialog::revert()
{
  mLayer->rollBack();
  mModel->revert();
  mModel->reload( mModel->index( 0, 0 ), mModel->index( mModel->rowCount(), mModel->columnCount() ) );
}

void QgsAttributeTableDialog::on_mOpenFieldCalculator_clicked()
{
  QgsFieldCalculator calc( mLayer );
  if ( calc.exec() )
  {
    // update model - a field has been added or updated
    mModel->reload( mModel->index( 0, 0 ), mModel->index( mModel->rowCount(), mModel->columnCount() ) );
  }
}
