/***************************************************************************
                     qgsidentifyresults.cpp  -  description
                              -------------------
      begin                : Fri Oct 25 2002
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

#include "qgsidentifyresults.h"
#include "qgscontexthelp.h"
#include "qgsapplication.h"
#include "qgisapp.h"

#include <QCloseEvent>
#include <QLabel>
#include <QAction>
#include <QTreeWidgetItem>
#include <QPixmap>
#include <QSettings>
#include <QMenu>
#include <QClipboard>

#include "qgslogger.h"

QgsIdentifyResults::QgsIdentifyResults( const QgsAttributeAction& actions,
                                        QWidget *parent, Qt::WFlags f )
    : QDialog( parent, f ),
    mActions( actions ),
    mClickedOnValue( 0 ),
    mActionPopup( 0 ),
    mCurrentFeatureId( 0 )
{
  setupUi( this );
  lstResults->setColumnCount( 2 );
  setColumnText( 0, tr( "Feature" ) );
  setColumnText( 1, tr( "Value" ) );

  connect( buttonCancel, SIGNAL( clicked() ),
           this, SLOT( close() ) );
  connect( lstResults, SIGNAL( itemClicked( QTreeWidgetItem*, int ) ),
           this, SLOT( clicked( QTreeWidgetItem * ) ) );
  connect( lstResults, SIGNAL( itemExpanded( QTreeWidgetItem* ) ),
           this, SLOT( itemExpanded( QTreeWidgetItem* ) ) );

  connect( lstResults, SIGNAL( currentItemChanged( QTreeWidgetItem*, QTreeWidgetItem* ) ),
           this, SLOT( handleCurrentItemChanged( QTreeWidgetItem*, QTreeWidgetItem* ) ) );

  // The label to use for the Derived node in the identify results
  mDerivedLabel = tr( "(Derived)" );
}

QgsIdentifyResults::~QgsIdentifyResults()
{
  delete mActionPopup;
}

// Call to show the dialog box.
void QgsIdentifyResults::show()
{
  // Enfore a few things before showing the dialog box
  lstResults->sortItems( 0, Qt::AscendingOrder );
  expandColumnsToFit();

  QDialog::show();
}
// Slot called when user clicks the Close button
// (saves the current window size/position)
void QgsIdentifyResults::close()
{
  saveWindowLocation();
  done( 0 );
}
// Save the current window size/position before closing
// from window menu or X in titlebar
void QgsIdentifyResults::closeEvent( QCloseEvent *e )
{
  // We'll close in our own good time thanks...
  e->ignore();
  close();
}

// Popup (create if necessary) a context menu that contains a list of
// actions that can be applied to the data in the identify results
// dialog box.

void QgsIdentifyResults::contextMenuEvent( QContextMenuEvent* event )
{
  QTreeWidgetItem* item = lstResults->itemAt( lstResults->viewport()->mapFrom( this, event->pos() ) );
  // if the user clicked below the end of the attribute list, just return
  if ( item == NULL )
    return;

  if ( mActionPopup == 0 )
  {
    mActionPopup = new QMenu();

    QAction *a;
    a = mActionPopup->addAction( tr( "Copy attribute value" ) );
    a->setEnabled( true );
    a->setData( QVariant::fromValue( -2 ) );

    a = mActionPopup->addAction( tr( "Copy feature attributes" ) );
    a->setEnabled( true );
    a->setData( QVariant::fromValue( -1 ) );

    if ( mActions.size() > 0 )
    {
      // The assumption is made that an instance of QgsIdentifyResults is
      // created for each new Identify Results dialog box, and that the
      // contents of the popup menu doesn't change during the time that
      // such a dialog box is around.
      a = mActionPopup->addAction( tr( "Run action" ) );
      a->setEnabled( false );
      mActionPopup->addSeparator();

      QgsAttributeAction::aIter iter = mActions.begin();
      for ( int j = 0; iter != mActions.end(); ++iter, ++j )
      {
        QAction* a = mActionPopup->addAction( iter->name() );
        // The menu action stores an integer that is used later on to
        // associate an menu action with an actual qgis action.
        a->setData( QVariant::fromValue( j ) );
      }
    }

    connect( mActionPopup, SIGNAL( triggered( QAction* ) ),
             this, SLOT( popupItemSelected( QAction* ) ) );
  }

  // Save the attribute values as these are needed for substituting into
  // the action.
  extractAllItemData( item );

  mActionPopup->popup( event->globalPos() );
}

// Restore last window position/size and show the window
void QgsIdentifyResults::restorePosition()
{
  QSettings settings;
  restoreGeometry( settings.value( "/Windows/Identify/geometry" ).toByteArray() );
  show();
}

// Save the current window location (store in ~/.qt/qgisrc)
void QgsIdentifyResults::saveWindowLocation()
{
  QSettings settings;
  settings.setValue( "/Windows/Identify/geometry", saveGeometry() );
}

/** add an attribute and its value to the list */
void QgsIdentifyResults::addAttribute( QTreeWidgetItem * fnode, QString field, QString value )
{
  QStringList labels;
  labels << field << value;
  new QTreeWidgetItem( fnode, labels );
}

void QgsIdentifyResults::addAttribute( QString field, QString value )
{
  QStringList labels;
  labels << field << value;
  new QTreeWidgetItem( lstResults, labels );
}

void QgsIdentifyResults::addDerivedAttribute( QTreeWidgetItem * fnode, QString field, QString value )
{
  QTreeWidgetItem * daRootNode;

  // Determine if this is the first derived attribute for this feature or not
  if ( mDerivedAttributeRootNodes.find( fnode ) != mDerivedAttributeRootNodes.end() )
  {
    // Reuse existing derived-attribute root node
    daRootNode = mDerivedAttributeRootNodes[fnode];
  }
  else
  {
    // Create new derived-attribute root node
    daRootNode = new QTreeWidgetItem( fnode, QStringList( mDerivedLabel ) );
    QFont font = daRootNode->font( 0 );
    font.setItalic( true );
    daRootNode->setFont( 0, font );
    mDerivedAttributeRootNodes[fnode] = daRootNode;
  }

  QStringList labels;
  labels << field << value;
  new QTreeWidgetItem( daRootNode, labels );
}

void QgsIdentifyResults::addEdit( QTreeWidgetItem * fnode, int id )
{
  QStringList labels;
  labels << "edit" << QString::number( id );
  QTreeWidgetItem *item = new QTreeWidgetItem( fnode, labels );

  item->setIcon( 0, QgisApp::getThemeIcon( "/mIconEditable.png" ) );
}

void QgsIdentifyResults::addAction( QTreeWidgetItem * fnode, int id, QString field, QString value )
{
  QStringList labels;
  labels << field << value << "action" << QString::number( id );
  QTreeWidgetItem *item = new QTreeWidgetItem( fnode, labels );

  item->setIcon( 0, QgisApp::getThemeIcon( "/mAction.png" ) );
}

/** Add a feature node to the list */
QTreeWidgetItem *QgsIdentifyResults::addNode( QString label )
{
  return new QTreeWidgetItem( lstResults, QStringList( label ) );
}

void QgsIdentifyResults::setTitle( QString title )
{
  setWindowTitle( tr( "Identify Results - %1" ).arg( title ) );
}

void QgsIdentifyResults::setColumnText( int column, const QString & label )
{
  QTreeWidgetItem* header = lstResults->headerItem();
  header->setText( column, label );
}

// Run the action that was selected in the popup menu
void QgsIdentifyResults::popupItemSelected( QAction* menuAction )
{
  int id = menuAction->data().toInt();

  if ( id < 0 )
  {
    QClipboard *clipboard = QApplication::clipboard();
    QString text;

    if ( id == -2 )
    {
      text = mValues[ mClickedOnValue ].second;
    }
    else
    {
      for ( std::vector< std::pair<QString, QString> >::const_iterator it = mValues.begin(); it != mValues.end(); it++ )
      {
        text += QString( "%1: %2\n" ).arg( it->first ).arg( it->second );
      }
    }

    QgsDebugMsg( QString( "set clipboard: %1" ).arg( text ) );
    clipboard->setText( text );
  }
  else
  {
    mActions.doAction( id, mValues, mClickedOnValue );
  }
}

/** Expand all the identified features (show their attributes). */
void QgsIdentifyResults::showAllAttributes()
{
  // Easy now with Qt 4.2...
  lstResults->expandAll();
}

void QgsIdentifyResults::expandColumnsToFit()
{
  lstResults->resizeColumnToContents( 0 );
  lstResults->resizeColumnToContents( 1 );
}

void QgsIdentifyResults::clear()
{
  mDerivedAttributeRootNodes.clear();
  lstResults->clear();
}

void QgsIdentifyResults::setMessage( QString shortMsg, QString longMsg )
{
  QStringList labels;
  labels << shortMsg << longMsg;
  new QTreeWidgetItem( lstResults, labels );
}

void QgsIdentifyResults::setActions( const QgsAttributeAction& actions )
{
  mActions = actions;
}

void QgsIdentifyResults::clicked( QTreeWidgetItem *item )
{
  if ( !item )
    return;

  if ( item->text( 2 ) == "action" )
  {
    int id = item->text( 3 ).toInt();

    extractAllItemData( item );

    mActions.doAction( id, mValues, mClickedOnValue );
  }
  else if ( item->text( 0 ) == "edit" )
  {
    emit editFeature( item->text( 1 ).toInt() );
  }
}
void QgsIdentifyResults::on_buttonHelp_clicked()
{
  QgsContextHelp::run( context_id );
}

void QgsIdentifyResults::itemExpanded( QTreeWidgetItem* item )
{
  expandColumnsToFit();
}

void QgsIdentifyResults::handleCurrentItemChanged( QTreeWidgetItem* current, QTreeWidgetItem* previous )
{
  if ( lstResults->model()->rowCount() <= 1 )
    return;

  if ( current == NULL )
  {
    mCurrentFeatureId = 0;
    emit selectedFeatureChanged( 0 );
    return;
  }

  // move to node where is saved feature ID
  QTreeWidgetItem* topLevelItem = current;
  while ( topLevelItem->parent() != NULL )
  {
    topLevelItem = topLevelItem->parent();
  }

  QVariant fid = topLevelItem->data( 0, Qt::UserRole );

  // no data saved...
  if ( fid.type() != QVariant::Int )
    return;
  int fid2 = fid.toInt();

  if ( fid2 == mCurrentFeatureId )
    return;

  mCurrentFeatureId = fid2;
  emit selectedFeatureChanged( mCurrentFeatureId );
}

void QgsIdentifyResults::extractAllItemData( QTreeWidgetItem* item )
{
  // Extracts the name/value pairs from the given item. This includes data
  // under the (Derived) item.

  // A little bit complicated because the user could of right-clicked
  // on any item in the dialog box. We want a toplevel item, so walk upwards
  // as far as possible.
  // We also want to keep track of which row in the identify results table was
  // actually clicked on. This is stored as an index into the mValues vector.

  QTreeWidgetItem* child = item;
  QTreeWidgetItem* parent = child->parent();
  while ( parent != 0 )
  {
    child = parent;
    parent = parent->parent();
  }
  parent = child;

  mValues.clear();

  // For the code below we
  // need to do the comparison on the text strings rather than the
  // pointers because if the user clicked on the parent, we need
  // to pick up which child that actually is (the parent in the
  // identify results dialog box is just one of the children
  // that has been chosen by some method).

  int valuesIndex = 0;

  for ( int j = 0; j < parent->childCount(); ++j )
  {
    // For derived attributes, build up a virtual name
    if ( parent->child( j )->text( 0 ) == mDerivedLabel )
    {
      for ( int k = 0; k < parent->child( j )->childCount(); ++k )
      {
        mValues.push_back(
          std::make_pair( mDerivedLabel + "."
                          + parent->child( j )->child( k )->text( 0 ),
                          parent->child( j )->child( k )->text( 1 ) ) );

        if ( item == parent->child( j )->child( k ) )
        {
          mClickedOnValue = valuesIndex;
        }

        valuesIndex++;
      }
    }
    else // do the actual feature attributes
    {
      mValues.push_back( std::make_pair( parent->child( j )->text( 0 ),
                                         parent->child( j )->text( 1 ) ) );

      if ( item == parent->child( j ) )
      {
        mClickedOnValue = valuesIndex;
      }

      valuesIndex++;
    }
  }
}
