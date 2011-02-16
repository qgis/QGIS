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
#include "qgsaddattrdialog.h"
#include "qgsdelattrdialog.h"
#include "qgssearchquerybuilder.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsfieldcalculator.h"
#include "qgsfeatureaction.h"

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

  bool myDockFlag = settings.value( "/qgis/dockAttributeTable", false ).toBool();
  if ( myDockFlag )
  {
    mDock = new QgsAttributeTableDock( tr( "Attribute table - %1 (%n Feature(s))", "feature count", mModel->rowCount() ).arg( mLayer->name() ), QgisApp::instance() );
    mDock->setAllowedAreas( Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea );
    mDock->setWidget( this );
    connect( this, SIGNAL( destroyed() ), mDock, SLOT( close() ) );
    QgisApp::instance()->addDockWidget( Qt::BottomDockWidgetArea, mDock );
  }

  updateTitle();

  mRemoveSelectionButton->setIcon( getThemeIcon( "/mActionUnselectAttributes.png" ) );
  mSelectedToTopButton->setIcon( getThemeIcon( "/mActionSelectedToTop.png" ) );
  mCopySelectedRowsButton->setIcon( getThemeIcon( "/mActionCopySelected.png" ) );
  mZoomMapToSelectedRowsButton->setIcon( getThemeIcon( "/mActionZoomToSelected.png" ) );
  mInvertSelectionButton->setIcon( getThemeIcon( "/mActionInvertSelection.png" ) );
  mToggleEditingButton->setIcon( getThemeIcon( "/mActionToggleEditing.png" ) );
  mDeleteSelectedButton->setIcon( getThemeIcon( "/mActionDeleteSelected.png" ) );
  mOpenFieldCalculator->setIcon( getThemeIcon( "/mActionCalculateField.png" ) );
  mAddAttribute->setIcon( getThemeIcon( "/mActionNewAttribute.png" ) );
  mRemoveAttribute->setIcon( getThemeIcon( "/mActionDeleteAttribute.png" ) );

  // toggle editing
  bool canChangeAttributes = mLayer->dataProvider()->capabilities() & QgsVectorDataProvider::ChangeAttributeValues;
  bool canDeleteFeatures = mLayer->dataProvider()->capabilities() & QgsVectorDataProvider::DeleteFeatures;
  bool canAddAttributes = mLayer->dataProvider()->capabilities() & QgsVectorDataProvider::AddAttributes;
  bool canDeleteAttributes = mLayer->dataProvider()->capabilities() & QgsVectorDataProvider::DeleteAttributes;
  bool canAddFeatures = mLayer->dataProvider()->capabilities() & QgsVectorDataProvider::AddFeatures;
  mToggleEditingButton->setCheckable( true );
  mToggleEditingButton->setChecked( mLayer->isEditable() );
  mToggleEditingButton->setEnabled( canChangeAttributes && !mLayer->isReadOnly() );
  mOpenFieldCalculator->setEnabled( canChangeAttributes && mLayer->isEditable() );
  mDeleteSelectedButton->setEnabled( canDeleteFeatures && mLayer->isEditable() );
  mAddAttribute->setEnabled( canAddAttributes && mLayer->isEditable() );
  mRemoveAttribute->setEnabled( canDeleteAttributes && mLayer->isEditable() );
  mAddFeature->setEnabled( canAddFeatures && mLayer->isEditable() && mLayer->geometryType() == QGis::NoGeometry );
  mAddFeature->setHidden( !canAddFeatures || mLayer->geometryType() != QGis::NoGeometry );

  // info from table to application
  connect( this, SIGNAL( editingToggled( QgsMapLayer * ) ), QgisApp::instance(), SLOT( toggleEditing( QgsMapLayer * ) ) );
  // info from layer to table
  connect( mLayer, SIGNAL( editingStarted() ), this, SLOT( editingToggled() ) );
  connect( mLayer, SIGNAL( editingStopped() ), this, SLOT( editingToggled() ) );

  connect( searchButton, SIGNAL( clicked() ), this, SLOT( search() ) );
  connect( mAddFeature, SIGNAL( clicked() ), this, SLOT( addFeature() ) );

  connect( mLayer, SIGNAL( selectionChanged() ), this, SLOT( updateSelectionFromLayer() ) );
  connect( mLayer, SIGNAL( layerDeleted() ), this, SLOT( close() ) );
  connect( mView->verticalHeader(), SIGNAL( sectionClicked( int ) ), this, SLOT( updateRowSelection( int ) ) );
  connect( mView->verticalHeader(), SIGNAL( sectionPressed( int ) ), this, SLOT( updateRowPressed( int ) ) );
  connect( mModel, SIGNAL( modelChanged() ), this, SLOT( updateSelection() ) );

  if ( settings.value( "/qgis/attributeTableBehaviour", 0 ).toInt() == 2 )
  {
    connect( QgisApp::instance()->mapCanvas(), SIGNAL( extentsChanged() ), mModel, SLOT( layerModified() ) );
  }

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

void QgsAttributeTableDialog::updateTitle()
{
  setWindowTitle( tr( "Attribute table - %1 :: %n / %2 feature(s) selected",
                      "feature count",
                      mView->selectionModel()->selectedRows().size()
                    )
                  .arg( mLayer->name() )
                  .arg( mModel->rowCount() )
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
  QSettings settings;
  bool canvasFeatures = settings.value( "/qgis/attributeTableBehaviour", 0 ).toInt() == 2;
  if ( canvasFeatures )
  {
    disconnect( QgisApp::instance()->mapCanvas(), SIGNAL( extentsChanged() ), mModel, SLOT( layerModified() ) );
  }

  QgisApp::instance()->mapCanvas()->zoomToSelected( mLayer );

  if ( canvasFeatures )
  {
    connect( QgisApp::instance()->mapCanvas(), SIGNAL( extentsChanged() ), mModel, SLOT( layerModified() ) );
  }
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
  QgisApp::instance()->deleteSelected( mLayer );
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
  QgsFieldMap fieldMap = mLayer->pendingFields();
  QgsFieldMap::Iterator it = fieldMap.begin();

  for ( ; it != fieldMap.end(); ++it )
    if ( mLayer->editType( it.key() ) != QgsVectorLayer::Hidden )
      mColumnBox->addItem( it.value().name() );

  mColumnBox->setCurrentIndex( mColumnBox->findText( mLayer->displayField() ) );
}

int QgsAttributeTableDialog::columnBoxColumnId()
{
  QgsFieldMap fieldMap = mLayer->pendingFields();
  QgsFieldMap::Iterator it = fieldMap.begin();

  for ( ; it != fieldMap.end(); ++it )
    if ( it.value().name() == mColumnBox->currentText() )
      return it.key();

  return 0;
}

void QgsAttributeTableDialog::updateSelection()
{
  QModelIndex index;
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
  updateTitle();

#if 0
  for ( int i = 0; i < mModel->rowCount(); ++i )
  {
    int id = mModel->rowToId( i );
    QgsDebugMsg( id );
  }
  QgsDebugMsg( "--------------" );
#endif
}

void QgsAttributeTableDialog::updateRowPressed( int index )
{
  mView->setSelectionMode( QAbstractItemView::ExtendedSelection );
  mIndexPressed = index;
}

void QgsAttributeTableDialog::updateRowSelection( int index )
{
  bool bDrag;

  if ( mIndexPressed == index )
    bDrag = false;
  else
    bDrag = true;

  QString key = "";
  if ( QApplication::keyboardModifiers() == Qt::ControlModifier )
    key = "Control";
  else if ( QApplication::keyboardModifiers() == Qt::ShiftModifier )
    key = "Shift";

  int first, last;

  if ( bDrag )
  {
    if ( mIndexPressed < index )
    {
      first = mIndexPressed;
      mLastClickedHeaderIndex = last = index;
    }
    else
    {
      last = mIndexPressed;
      mLastClickedHeaderIndex = first = index;
    }

    updateRowSelection( first, last, 3 );
    updateTitle();

    mView->setSelectionMode( QAbstractItemView::NoSelection );
    return;
  }
  else // No drag
  {
    if ( key == "Shift" )
    {
      QgsDebugMsg( "shift" );
      // get the first and last index of the rows to be selected/deselected
      first = last = 0;

      if ( index > mLastClickedHeaderIndex )
      {
        first = mLastClickedHeaderIndex;
        last = index;
      }
      else if ( index == mLastClickedHeaderIndex )
      {
        // row was selected and now it is shift-clicked
        first = last = index;
      }
      else
      {
        first = index;
        last = mLastClickedHeaderIndex;
      }

      // for all the rows update the selection, without starting a new selection
      if ( first <= last )
        updateRowSelection( first, last, 1 );
    }
    else if ( key == "Control" )
    {
      QgsDebugMsg( "ctrl" );
      // update the single row selection, without starting a new selection
      updateRowSelection( index, index, 2 );

      // the next shift would start from here
      mLastClickedHeaderIndex = index;
    }
    else // Single click
    {
      // Start a new selection if the row was not selected
      updateRowSelection( index, index, 0 );

      // the next shift would start from here
      mLastClickedHeaderIndex = index;
    }
  }
  mView->setSelectionMode( QAbstractItemView::NoSelection );
  updateTitle();
}

void QgsAttributeTableDialog::updateRowSelection( int first, int last, int clickType )
{
  // clickType= 0:Single click, 1:Shift, 2:Ctrl, 3: Dragged click
  disconnect( mLayer, SIGNAL( selectionChanged() ), this, SLOT( updateSelectionFromLayer() ) );

  // Id must be mapped to table/view row
  QModelIndex index = mFilterModel->mapToSource( mFilterModel->index( first, 0 ) );
  int fid = mModel->rowToId( index.row() );
  bool wasSelected = mSelectedFeatures.contains( fid );

  // new selection should be created
  if ( clickType == 0 ) // Single click
  {
    if ( mSelectedFeatures.size() == 1 && wasSelected ) // One item selected
      return; // Click over a selected item doesn't do anything

    mView->setCurrentIndex( mFilterModel->index( first, 0 ) );
    mView->selectRow( first );

    mSelectedFeatures.clear();
    mSelectedFeatures.insert( fid );
    mLayer->removeSelection();
    mLayer->select( fid );
    connect( mLayer, SIGNAL( selectionChanged() ), this, SLOT( updateSelectionFromLayer() ) );
    return;
  }
  else if ( clickType == 1 ) // Shift
  {
    QgsFeatureIds newSelection;

    for ( int i = first; i <= last; ++i )
    {
      if ( i >= first )
      {
        // Id must be mapped to table/view row
        index = mFilterModel->mapToSource( mFilterModel->index( i, 0 ) );
        fid = mModel->rowToId( index.row() );
      }
      newSelection.insert( fid );
    }

    // Remove items in mSelectedFeatures if they aren't in mNewSelection
    QgsFeatureIds::Iterator it = mSelectedFeatures.begin();
    while ( it != mSelectedFeatures.end() )
    {
      if ( !newSelection.contains( *it ) )
      {
        it = mSelectedFeatures.erase( it );
      }
      else
      {
        ++it;
      }
    }

    // Append the other fids in range first-last to mSelectedFeatures
    QgsFeatureIds::Iterator itNew = newSelection.begin();
    for ( ; itNew != newSelection.end(); ++itNew )
    {
      if ( !mSelectedFeatures.contains( *itNew ) )
      {
        mSelectedFeatures.insert( *itNew );
      }
    }
  }
  else if ( clickType == 2 ) // Ctrl
  {
    // existing selection should be updated
    if ( wasSelected )
      mSelectedFeatures.remove( fid );
    else
      mSelectedFeatures.insert( fid );
  }
  else if ( clickType == 3 ) // Dragged click
  {
    QgsFeatureIds newSelection;

    for ( int i = first; i <= last; ++i )
    {
      if ( i >= first )
      {
        // Id must be mapped to table/view row
        index = mFilterModel->mapToSource( mFilterModel->index( i, 0 ) );
        fid = mModel->rowToId( index.row() );
      }
      newSelection.insert( fid );
    }

    // Remove items in mSelectedFeatures if they aren't in mNewSelection
    QgsFeatureIds::Iterator it = mSelectedFeatures.begin();
    while ( it != mSelectedFeatures.end() )
    {
      if ( !newSelection.contains( *it ) )
      {
        it = mSelectedFeatures.erase( it );
      }
      else
      {
        ++it;
      }
    }

    // Append the other fids in range first-last to mSelectedFeatures
    QgsFeatureIds::Iterator itNew = newSelection.begin();
    for ( ; itNew != newSelection.end(); ++itNew )
    {
      if ( !mSelectedFeatures.contains( *itNew ) )
      {
        mSelectedFeatures.insert( *itNew );
      }
    }
  }

  updateSelection();
  mLayer->setSelectedFeatures( mSelectedFeatures );
  connect( mLayer, SIGNAL( selectionChanged() ), this, SLOT( updateSelectionFromLayer() ) );
}

void QgsAttributeTableDialog::updateSelectionFromLayer()
{
  QgsDebugMsg( "updateFromLayer" );
  mSelectedFeatures = mLayer->selectedFeaturesIds();

  if ( cbxShowSelectedOnly->isChecked() )
    mFilterModel->invalidate();

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

  // TODO: fetch only necessary columns
  // QStringList columns = searchTree->referencedColumns();
  bool fetchGeom = searchTree->needsGeometry();

  QApplication::setOverrideCursor( Qt::WaitCursor );
  mSelectedFeatures.clear();

  if ( cbxSearchSelectedOnly->isChecked() )
  {
    QgsFeatureList selectedFeatures = mLayer->selectedFeatures();
    for ( QgsFeatureList::Iterator it = selectedFeatures.begin(); it != selectedFeatures.end(); ++it )
    {
      if ( searchTree->checkAgainst( mLayer->pendingFields(), *it ) )
        mSelectedFeatures << it->id();

      // check if there were errors during evaluating
      if ( searchTree->hasError() )
        break;
    }
  }
  else
  {
    mLayer->select( mLayer->pendingAllAttributesList(), QgsRectangle(), fetchGeom );
    QgsFeature f;

    while ( mLayer->nextFeature( f ) )
    {
      if ( searchTree->checkAgainst( mLayer->pendingFields(), f ) )
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
  QWidget *w = mDock ? qobject_cast<QWidget*>( mDock ) : qobject_cast<QWidget*>( this );
  if ( mSelectedFeatures.size() )
  {
    w->setWindowTitle( tr( "Attribute table - %1 (%n matching features)", "matching features", mSelectedFeatures.size() ).arg( mLayer->name() ) );
  }
  else
  {
    w->setWindowTitle( tr( "Attribute table - %1 (No matching features)" ).arg( mLayer->name() ) );
  }
}

void QgsAttributeTableDialog::search()
{

  QString fieldName = mColumnBox->currentText();
  const QgsFieldMap& flds = mLayer->pendingFields();
  int fldIndex = mLayer->fieldNameIndex( fieldName );
  QVariant::Type fldType = flds[fldIndex].type();
  bool numeric = ( fldType == QVariant::Int || fldType == QVariant::Double );
  QString sensString = "ILIKE";
  if ( cbxCaseSensitive->isChecked() )
  {
    sensString = "LIKE";
  }

  QSettings settings;
  QString nullValue = settings.value( "qgis/nullValue", "NULL" ).toString();

  QString str;
  if ( mQuery->displayText() == nullValue )
  {
    str = QString( "%1 IS NULL" ).arg( QgsSearchTreeNode::quotedColumnRef( fieldName ) );
  }
  else
  {
    str = QString( "%1 %2 '%3'" )
          .arg( QgsSearchTreeNode::quotedColumnRef( fieldName ) )
          .arg( numeric ? "=" : sensString )
          .arg( numeric
                ? mQuery->displayText().replace( "'", "''" )
                :
                "%" + mQuery->displayText().replace( "'", "''" ) + "%" ); // escape quotes
  }

  doSearch( str );
}

void QgsAttributeTableDialog::on_mAdvancedSearchButton_clicked()
{
  QgsSearchQueryBuilder dlg( mLayer, this );
  dlg.setSearchString( mQuery->displayText() );

  if ( dlg.exec() == QDialog::Accepted )
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
  bool canDeleteFeatures = mLayer->dataProvider()->capabilities() & QgsVectorDataProvider::DeleteFeatures;
  bool canAddAttributes = mLayer->dataProvider()->capabilities() & QgsVectorDataProvider::AddAttributes;
  bool canDeleteAttributes = mLayer->dataProvider()->capabilities() & QgsVectorDataProvider::DeleteAttributes;
  bool canAddFeatures = mLayer->dataProvider()->capabilities() & QgsVectorDataProvider::AddFeatures;
  mOpenFieldCalculator->setEnabled( canChangeAttributes && mLayer->isEditable() );
  mDeleteSelectedButton->setEnabled( canDeleteFeatures && mLayer->isEditable() );
  mAddAttribute->setEnabled( canAddAttributes && mLayer->isEditable() );
  mRemoveAttribute->setEnabled( canDeleteAttributes && mLayer->isEditable() );
  mAddFeature->setEnabled( canAddFeatures && mLayer->isEditable() && mLayer->geometryType() == QGis::NoGeometry );

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

void QgsAttributeTableDialog::on_mAddAttribute_clicked()
{
  if ( !mLayer )
  {
    return;
  }

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
    mModel->reload( mModel->index( 0, 0 ), mModel->index( mModel->rowCount(), mModel->columnCount() ) );
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

    mLayer->beginEditCommand( tr( "Deleted attribute" ) );
    bool deleted = false;
    QList<int>::const_iterator it = attributes.constBegin();
    for ( ; it != attributes.constEnd(); ++it )
    {
      if ( mLayer->deleteAttribute( *it ) )
      {
        deleted = true;
      }
    }

    if ( deleted )
    {
      mLayer->endEditCommand();
    }
    else
    {
      QMessageBox::critical( 0, tr( "Attribute Error" ), tr( "The attribute(s) could not be deleted" ) );
      mLayer->destroyEditCommand();
    }
    // update model - a field has been added or updated
    mModel->reload( mModel->index( 0, 0 ), mModel->index( mModel->rowCount(), mModel->columnCount() ) );
  }
}

void QgsAttributeTableDialog::on_mOpenFieldCalculator_clicked()
{
  QgsFieldCalculator calc( mLayer );
  if ( calc.exec() == QDialog::Accepted )
  {
    int col = mModel->fieldCol( calc.changedAttributeId() );

    if ( col >= 0 )
    {
      mModel->reload( mModel->index( 0, col ),
                      mModel->index( mModel->rowCount(), col ) );
    }
  }
}


void QgsAttributeTableDialog::addFeature()
{
  if ( !mLayer->isEditable() )
    return;

  QgsFeature f;
  QgsFeatureAction action( tr( "Geometryless feature added" ), f, mLayer, -1, this );
  if ( action.addFeature() )
  {
    mModel->reload( mModel->index( 0, 0 ), mModel->index( mModel->rowCount(), mModel->columnCount() ) );
  }
}
