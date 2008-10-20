/***************************************************************************
                          QgsAttributeTableDisplay.cpp  -  description
                             -------------------
    begin                : Sat Nov 23 2002
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman at mrcc dot com
       Romans 3:23=>Romans 6:23=>Romans 5:8=>Romans 10:9,10=>Romans 12
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#include "qgsattributetabledisplay.h"

#include "qgisapp.h"
#include "qgsapplication.h"
#include "qgsfeature.h"
#include "qgsfield.h"
#include "qgslogger.h"
#include "qgssearchquerybuilder.h"
#include "qgssearchstring.h"
#include "qgssearchtreenode.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgscontexthelp.h"

#include <QCloseEvent>
#include <QMenuBar>
#include <QMessageBox>
#include <QIcon>
#include <QPixmap>
#include <QSettings>
#include <QToolButton>
#include <QDockWidget>

class QAttributeTableDock : public QDockWidget
{
  public:
    QAttributeTableDock( const QString & title, QWidget * parent = 0, Qt::WindowFlags flags = 0 )
        : QDockWidget( title, parent, flags )
    {
    }

    virtual void closeEvent( QCloseEvent * ev )
    {
      deleteLater();
    }
};

QgsAttributeTableDisplay::QgsAttributeTableDisplay( QgsVectorLayer* layer )
    : QDialog( 0, Qt::Window ),
    mLayer( layer ),
    mDock( NULL )
{
  setupUi( this );
  restorePosition();
  setTheme();

  mToggleEditingButton->setEnabled( layer->dataProvider()->capabilities() & QgsVectorDataProvider::ChangeAttributeValues );
  mToggleEditingButton->setChecked( layer->isEditable() );

  connect( mRemoveSelectionButton, SIGNAL( clicked() ), this, SLOT( removeSelection() ) );
  connect( mSelectedToTopButton, SIGNAL( clicked() ), this, SLOT( selectedToTop() ) );
  connect( mInvertSelectionButton, SIGNAL( clicked() ), this, SLOT( invertSelection() ) );
  connect( mCopySelectedRowsButton, SIGNAL( clicked() ), this, SLOT( copySelectedRowsToClipboard() ) );
  connect( mZoomMapToSelectedRowsButton, SIGNAL( clicked() ), this, SLOT( zoomMapToSelectedRows() ) );
  connect( mSearchButton, SIGNAL( clicked() ), this, SLOT( search() ) );
  connect( mSearchShowResults, SIGNAL( activated( int ) ), this, SLOT( searchShowResultsChanged( int ) ) );
  connect( btnAdvancedSearch, SIGNAL( clicked() ), this, SLOT( advancedSearch() ) );
  connect( buttonBox, SIGNAL( helpRequested() ), this, SLOT( showHelp() ) );
  connect( buttonBox->button( QDialogButtonBox::Close ), SIGNAL( clicked() ), this, SLOT( close() ) );

  connect( mToggleEditingButton, SIGNAL( clicked() ), this, SLOT( toggleEditing() ) );
  connect( this, SIGNAL( editingToggled( QgsMapLayer * ) ), QgisApp::instance(), SLOT( toggleEditing( QgsMapLayer * ) ) );

  // establish connection to table
  connect( tblAttributes, SIGNAL( cellChanged( int, int ) ), this, SLOT( changeFeatureAttribute( int, int ) ) );

  // establish connections to layer
  connect( mLayer, SIGNAL( layerDeleted() ), this, SLOT( close() ) );

  connect( mLayer, SIGNAL( selectionChanged() ), this, SLOT( selectionChanged() ) );

  connect( mLayer, SIGNAL( editingStarted() ), this, SLOT( editingToggled() ) );
  connect( mLayer, SIGNAL( editingStopped() ), this, SLOT( editingToggled() ) );

  connect( mLayer, SIGNAL( attributeAdded( int ) ), this, SLOT( attributeAdded( int ) ) );
  connect( mLayer, SIGNAL( attributeDeleted( int ) ), this, SLOT( attributeDeleted( int ) ) );

  connect( mLayer, SIGNAL( attributeValueChanged( int, int, const QVariant & ) ),
           tblAttributes, SLOT( attributeValueChanged( int, int, const QVariant & ) ) );

  connect( mLayer, SIGNAL( featureDeleted( int ) ),
           tblAttributes, SLOT( featureDeleted( int ) ) );

  // establish connections between table and vector layer
  connect( tblAttributes, SIGNAL( selected( int, bool ) ), mLayer, SLOT( select( int, bool ) ) );
  connect( tblAttributes, SIGNAL( selectionRemoved( bool ) ), mLayer, SLOT( removeSelection( bool ) ) );
  connect( tblAttributes, SIGNAL( repaintRequested() ), mLayer, SLOT( triggerRepaint() ) );

  // fill in mSearchColumns with available columns
  const QgsFieldMap& xfields = mLayer->pendingFields();
  QgsFieldMap::const_iterator fldIt;
  for ( fldIt = xfields.constBegin(); fldIt != xfields.constEnd(); ++fldIt )
  {
    mSearchColumns->addItem( fldIt->name() );
  }

  // TODO: create better labels
  mSearchShowResults->addItem( tr( "select" ) );
  mSearchShowResults->addItem( tr( "select and bring to top" ) );
  mSearchShowResults->addItem( tr( "show only matching" ) );

  QSettings mySettings;
  bool myDockFlag = mySettings.value( "/qgis/dockAttributeTable", false ).toBool();
  if ( myDockFlag )
  {
    mDock = new QAttributeTableDock( tr( "Attribute table - " ) + layer->name(), QgisApp::instance() );
    mDock->setAllowedAreas( Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea );
    mDock->setWidget( this );
    QgisApp::instance()->addDockWidget( Qt::BottomDockWidgetArea, mDock );
    buttonBox->setVisible( false );
  }

  setWindowTitle( tr( "Attribute table - " ) + layer->name() );

#ifdef Q_WS_MAC
  if ( !myDockFlag )
  {
    QMenuBar *menuBar = new QMenuBar( this );

    QMenu *appMenu = menuBar->addMenu( tr( "QGIS" ) );
    appMenu->addAction( QgisApp::instance()->actionAbout() );
    appMenu->addAction( QgisApp::instance()->actionOptions() );

    QMenu *fileMenu = menuBar->addMenu( tr( "File" ) );
    QAction *closeAction = fileMenu->addAction( tr( "Close" ), this, SLOT( close() ), tr( "Ctrl+W" ) );

    QMenu *editMenu = menuBar->addMenu( tr( "Edit" ) );
    QAction *undoAction = editMenu->addAction( tr( "&Undo" ), this, SLOT( undo() ), tr( "Ctrl+Z" ) );
    undoAction->setEnabled( false );
    editMenu->addSeparator();
    QAction *cutAction = editMenu->addAction( tr( "Cu&t" ), this, SLOT( cut() ), tr( "Ctrl+X" ) );
    cutAction->setEnabled( false );
    QAction *copyAction = editMenu->addAction(
                            mCopySelectedRowsButton->icon(), tr( "&Copy" ), this, SLOT( copySelectedRowsToClipboard() ), tr( "Ctrl+C" ) );
    QAction *pasteAction = editMenu->addAction( tr( "&Paste" ), this, SLOT( paste() ), tr( "Ctrl+V" ) );
    pasteAction->setEnabled( false );
    QAction *deleteAction = editMenu->addAction(
                              mRemoveSelectionButton->icon(), tr( "Delete" ), this, SLOT( removeSelection() ) );

    QMenu *layerMenu = menuBar->addMenu( tr( "Layer" ) );
    QAction *zoomToSelectedAction = layerMenu->addAction(
                                      mZoomMapToSelectedRowsButton->icon(), tr( "Zoom to Selection" ), this, SLOT( zoomMapToSelectedRows() ), tr( "Ctrl+J" ) );
    layerMenu->addSeparator();
    QAction *toggleEditingAction = layerMenu->addAction(
                                     mToggleEditingButton->icon(), tr( "Toggle Editing" ), this, SLOT( toggleEditing() ) );
    toggleEditingAction->setEnabled( mToggleEditingButton->isEnabled() );
    toggleEditingAction->setCheckable( true );
    toggleEditingAction->setChecked( mToggleEditingButton->isChecked() );
    connect( mToggleEditingButton, SIGNAL( toggled( bool ) ), toggleEditingAction, SLOT( setChecked( bool ) ) );

    QMenu *tableMenu = menuBar->addMenu( tr( "Table" ) );
    QAction *moveToTopAction = tableMenu->addAction(
                                 mSelectedToTopButton->icon(), tr( "Move to Top" ), this, SLOT( selectedToTop() ) );
    QAction *invertAction = tableMenu->addAction(
                              mInvertSelectionButton->icon(), tr( "Invert" ), this, SLOT( invertSelection() ) );

#ifndef Q_WS_MAC64 /* assertion failure in NSMenuItem setSubmenu (Qt 4.5.0-snapshot-20080830) */
    menuBar->addMenu( QgisApp::instance()->windowMenu() );

    menuBar->addMenu( QgisApp::instance()->helpMenu() );
#endif

    // Create action to select this window and add it to Window menu
    mWindowAction = new QAction( windowTitle(), this );
    connect( mWindowAction, SIGNAL( triggered() ), this, SLOT( activate() ) );
    QgisApp::instance()->addWindow( mWindowAction );
  }
#endif
}

QgsAttributeTableDisplay::~QgsAttributeTableDisplay()
{
  smTables.remove( mLayer );
}

#ifdef Q_WS_MAC
void QgsAttributeTableDisplay::changeEvent( QEvent* event )
{
  QDialog::changeEvent( event );
  switch ( event->type() )
  {
    case QEvent::ActivationChange:
      if ( QApplication::activeWindow() == this )
      {
        mWindowAction->setChecked( true );
      }
      break;

    default:
      break;
  }
}
#endif

void QgsAttributeTableDisplay::closeEvent( QCloseEvent *ev )
{
  QDialog::closeEvent( ev );

  if ( mDock == NULL )
    saveWindowLocation();

  deleteLater();
}

void QgsAttributeTableDisplay::fillTable()
{
  tblAttributes->fillTable( mLayer );
  tblAttributes->setReadOnly( !mLayer->isEditable() );

  selectionChanged();

  // Give the table the most recent copy of the actions for this layer.
  setAttributeActions( *mLayer->actions() );
}

void QgsAttributeTableDisplay::toggleEditing()
{
  emit editingToggled( mLayer );
}

void QgsAttributeTableDisplay::setAttributeActions( const QgsAttributeAction &action )
{
  tblAttributes->setAttributeActions( action );
}

void QgsAttributeTableDisplay::selectRowsWithId( const QgsFeatureIds &ids )
{
  tblAttributes->selectRowsWithId( ids );
}

void QgsAttributeTableDisplay::setTheme()
{
  mRemoveSelectionButton->setIcon( QgisApp::getThemeIcon( "/mActionUnselectAttributes.png" ) );
  mSelectedToTopButton->setIcon( QgisApp::getThemeIcon( "/mActionSelectedToTop.png" ) );
  mInvertSelectionButton->setIcon( QgisApp::getThemeIcon( "/mActionInvertSelection.png" ) );
  mCopySelectedRowsButton->setIcon( QgisApp::getThemeIcon( "/mActionCopySelected.png" ) );
  mZoomMapToSelectedRowsButton->setIcon( QgisApp::getThemeIcon( "/mActionZoomToSelected.png" ) );
  mToggleEditingButton->setIcon( QgisApp::getThemeIcon( "/mActionToggleEditing.png" ) );
}

void QgsAttributeTableDisplay::editingToggled()
{
  mToggleEditingButton->setChecked( mLayer->isEditable() );
  tblAttributes->setReadOnly( !mLayer->isEditable() );
}

void QgsAttributeTableDisplay::selectedToTop()
{
  tblAttributes->bringSelectedToTop();
}

void QgsAttributeTableDisplay::invertSelection()
{
  if ( !mLayer )
    return;

  QApplication::setOverrideCursor( Qt::WaitCursor );
  mLayer->invertSelection();
  QApplication::restoreOverrideCursor();
}

void QgsAttributeTableDisplay::removeSelection()
{
  tblAttributes->clearSelection();
  mLayer->triggerRepaint();
}

void QgsAttributeTableDisplay::copySelectedRowsToClipboard()
{
  QgisApp::instance()->editCopy( mLayer );
}

void QgsAttributeTableDisplay::zoomMapToSelectedRows()
{
  QgisApp::instance()->zoomToSelected();
}

void QgsAttributeTableDisplay::search()
{
  int type = tblAttributes->item( 0, mSearchColumns->currentIndex() )->data( QgsAttributeTable::AttributeType ).toInt();
  bool numeric = ( type == QVariant::Int || type == QVariant::Double );

  QString str;
  str = mSearchColumns->currentText();
  if ( numeric )
    str += " = '";
  else
    str += " ~ '";
  str += mSearchText->text();
  str += "'";

  doSearch( str );
}


void QgsAttributeTableDisplay::advancedSearch()
{
  QgsSearchQueryBuilder dlg( mLayer, this );
  dlg.setSearchString( mSearchString );
  if ( dlg.exec() )
  {
    doSearch( dlg.searchString() );
  }
}


void QgsAttributeTableDisplay::searchShowResultsChanged( int item )
{
  QApplication::setOverrideCursor( Qt::WaitCursor );

  if ( item == 2 ) // show only matching
  {
    tblAttributes->showRowsWithId( mSearchIds );
  }
  else
  {
    // make sure that all rows are shown
    tblAttributes->showAllRows();

    // select matching
    mLayer->setSelectedFeatures( mSearchIds );

    if ( item == 1 ) // select matching and bring to top
      tblAttributes->bringSelectedToTop();
  }

  QApplication::restoreOverrideCursor();
}


void QgsAttributeTableDisplay::doSearch( QString searchString )
{
  mSearchString = searchString;

  // parse search string (and build parsed tree)
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

  QgsDebugMsg( "Search by attribute: " + searchString + " parsed as: " + search.tree()->makeSearchString() );

  QApplication::setOverrideCursor( Qt::WaitCursor );

  mSearchIds.clear();

  mLayer->select( mLayer->pendingAllAttributesList(), QgsRect(), false );

  QgsFeature f;
  while ( mLayer->nextFeature( f ) )
  {
    if ( searchTree->checkAgainst( mLayer->pendingFields(), f.attributeMap() ) )
    {
      mSearchIds << f.featureId();
    }

    // check if there were errors during evaluating
    if ( searchTree->hasError() )
      break;
  }

  QApplication::restoreOverrideCursor();

  if ( searchTree->hasError() )
  {
    QMessageBox::critical( this, tr( "Error during search" ), searchTree->errorMsg() );
    return;
  }

  // update table
  searchShowResultsChanged( mSearchShowResults->currentIndex() );

  QString str;
  if ( mSearchIds.size() )
    str.sprintf( tr( "Found %d matching features.", "", mSearchIds.size() ).toUtf8(), mSearchIds.size() );
  else
    str = tr( "No matching features found." );
  QMessageBox::information( this, tr( "Search results" ), str );
}

void QgsAttributeTableDisplay::restorePosition()
{
  QSettings settings;
  restoreGeometry( settings.value( "/Windows/AttributeTable/geometry" ).toByteArray() );
}

void QgsAttributeTableDisplay::saveWindowLocation()
{
  QSettings settings;
  settings.setValue( "/Windows/AttributeTable/geometry", saveGeometry() );
}

void QgsAttributeTableDisplay::showHelp()
{
  QgsContextHelp::run( context_id );
}

void QgsAttributeTableDisplay::changeFeatureAttribute( int row, int column )
{
  if ( column == 0 )
    return;

  if ( !mLayer->isEditable() )
    return;

  mLayer->changeAttributeValue(
    tblAttributes->item( row, 0 )->text().toInt(),
    tblAttributes->horizontalHeaderItem( column )->data( QgsAttributeTable::AttributeIndex ).toInt(),
    tblAttributes->item( row, column )->text(),
    false
  );
}

QMap<QgsVectorLayer *, QgsAttributeTableDisplay *> QgsAttributeTableDisplay::smTables;

QgsAttributeTableDisplay *QgsAttributeTableDisplay::attributeTable( QgsVectorLayer *layer )
{
  if ( !layer )
    return NULL;

  if ( smTables.contains( layer ) )
  {
    QgsAttributeTableDisplay *td = smTables[layer];
    td->setAttributeActions( *layer->actions() );
    td->activate();

    return td;
  }

  QgsAttributeTableDisplay *td = new QgsAttributeTableDisplay( layer );
  if ( !td )
    return NULL;

  // display the attribute table
  QApplication::setOverrideCursor( Qt::WaitCursor );

  try
  {
    td->fillTable();
  }
  catch ( std::bad_alloc& ba )
  {
    Q_UNUSED( ba );
    QMessageBox::critical( 0, tr( "bad_alloc exception" ), tr( "Filling the attribute table has been stopped because there was no more virtual memory left" ) );
    delete td;
    td = NULL;
  }

  QApplication::restoreOverrideCursor();

  if ( !td )
    return NULL;

  smTables[layer] = td;
  td->show();

  return td;
}

void QgsAttributeTableDisplay::activate()
{
  raise();
  setWindowState( windowState() & ~Qt::WindowMinimized );
  activateWindow();
}

void QgsAttributeTableDisplay::selectionChanged()
{
  // select rows which should be selected
  selectRowsWithId( mLayer->selectedFeaturesIds() );
}

void QgsAttributeTableDisplay::attributeAdded( int attr )
{
  tblAttributes->addAttribute( attr, mLayer->pendingFields()[attr] );
}

void QgsAttributeTableDisplay::attributeDeleted( int attr )
{
  tblAttributes->deleteAttribute( attr );
}
