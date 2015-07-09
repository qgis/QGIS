/***************************************************************************
    qgsgrassattributes.cpp  -  GRASS Attributes
                             -------------------
    begin                : July, 2004
    copyright            : (C) 2004 by Radim Blazek
    email                : blazek@itc.it
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgrassattributes.h"
#include "qgsgrassprovider.h"

#include "qgslogger.h"

#include <QHeaderView>
#include <QKeyEvent>
#include <QMessageBox>
#include <QSettings>
#include <QTableWidget>


QgsGrassAttributesKeyPress::QgsGrassAttributesKeyPress( QTableWidget *tab )
{
  mTable = tab;
}

QgsGrassAttributesKeyPress::~QgsGrassAttributesKeyPress() {}

bool QgsGrassAttributesKeyPress::eventFilter( QObject *o, QEvent *e )
{
  Q_UNUSED( o );
  if ( e->type() == QEvent::KeyPress )
  {
    QKeyEvent *k = ( QKeyEvent * )e;

    if ( k->key() == Qt::Key_Tab )
    {
      if ( mTable->currentRow() < mTable->rowCount() - 1 )
      {
        mTable->setCurrentCell( mTable->currentRow() + 1, mTable->currentColumn() );
      }
      return true; // eat event
    }
  }
  return false; // standard event processing
}

QgsGrassAttributes::QgsGrassAttributes( QgsGrassEdit *edit, QgsGrassProvider *provider, int line,
                                        QWidget * parent, const char * name, Qt::WindowFlags f )
    : QDialog( parent, f ), QgsGrassAttributesBase()
{
  Q_UNUSED( name );
  QgsDebugMsg( "QgsGrassAttributes()" );

  setupUi( this );

  mEdit = edit;
  mProvider = provider;
  mLine = line;

  resultLabel->setText( "" );

  // Remove old
  while ( tabCats->count() )
  {
    tabCats->removeTab( tabCats->currentIndex() );
  }

  connect( this, SIGNAL( destroyed() ), mEdit, SLOT( attributesClosed() ) );

  // TODO: does not work:
  connect( tabCats, SIGNAL( currentChanged( int ) ), this, SLOT( tabChanged( int ) ) );

  resetButtons();
  restorePosition();
}

QgsGrassAttributes::~QgsGrassAttributes()
{
  QgsDebugMsg( "~QgsGrassAttributes()" );

  saveWindowLocation();
}

void QgsGrassAttributes::restorePosition()
{
  QgsDebugMsg( "entered." );
  QSettings settings;
  restoreGeometry( settings.value( "/GRASS/windows/attributes/geometry" ).toByteArray() );
}

void QgsGrassAttributes::saveWindowLocation()
{
  QgsDebugMsg( "entered." );
  QSettings settings;
  settings.setValue( "/GRASS/windows/attributes/geometry", saveGeometry() );
}

void QgsGrassAttributes::setRowReadOnly( QTableWidget* table, int row, bool ro )
{
  for ( int i = 0; i < table->columnCount(); ++i )
  {
    QTableWidgetItem *item = table->item( row, i );
    item->setFlags( ro ? item->flags() & ~Qt::ItemIsEditable : item->flags() | Qt::ItemIsEditable );
  }
}

int QgsGrassAttributes::addTab( const QString & label )
{
  QgsDebugMsg( "entered." );

  QTableWidget *tb = new QTableWidget( 2, 3 );

  tb->setHorizontalHeaderLabels( QStringList()
                                 << tr( "Column" )
                                 << tr( "Value" )
                                 << tr( "Type" ) );  // Internal use

  tb->verticalHeader()->hide();

  tabCats->addTab( tb, label );

  // Move down with Tab, unfortunately it does not work if the cell is edited
  // TODO: catch Tab also if the cell is edited
  QgsGrassAttributesKeyPress *ef = new QgsGrassAttributesKeyPress( tb );
  tb->installEventFilter( ef );

  resetButtons();

  QSettings settings;
  QString path = "/GRASS/windows/attributes/columnWidth/";
  for ( int i = 0; i < 2; i++ )
  {
    bool ok = settings.contains( path + QString::number( i ) );
    int cw = settings.value( path + QString::number( i ), 30 ).toInt();
    if ( ok )
      tb->setColumnWidth( i, cw );
  }

  connect( tb->horizontalHeader(), SIGNAL( sectionResized( int, int, int ) ),
           this, SLOT( columnSizeChanged( int, int, int ) ) );

  return ( tabCats->count() - 1 );
}

void QgsGrassAttributes::setField( int tab, int field )
{
  QgsDebugMsg( "entered." );

  QTableWidget *tb = static_cast<QTableWidget *>( tabCats->widget( tab ) );

  tb->setItem( 0, 0, new QTableWidgetItem( tr( "Layer" ) ) );

  QString str;
  str.sprintf( "%d", field );

  tb->setItem( 0, 1, new QTableWidgetItem( str ) );

  tb->setItem( 0, 2, new QTableWidgetItem() );

  setRowReadOnly( tb, 0, true );
}

void QgsGrassAttributes::setCat( int tab, const QString & name, int cat )
{
  QgsDebugMsg( "entered." );

  QTableWidget *tb = static_cast<QTableWidget *>( tabCats->widget( tab ) );

  tb->setItem( 1, 0, new QTableWidgetItem( name ) );

  QString str;
  str.sprintf( "%d", cat );

  tb->setItem( 1, 1, new QTableWidgetItem( str ) );

  tb->setItem( 1, 2, new QTableWidgetItem() );

  setRowReadOnly( tb, 1, true );
}

void QgsGrassAttributes::addAttribute( int tab, const QString &name, const QString &value,
                                       const QString &type )
{
  QgsDebugMsg( QString( "name=%1 value=%2" ).arg( name ).arg( value ) );

  QTableWidget *tb = static_cast<QTableWidget *>( tabCats->widget( tab ) );

  tb->setRowCount( tb->rowCount() + 1 );

  int row = tb->rowCount() - 1;
  tb->setItem( row, 0, new QTableWidgetItem( name ) );
  tb->item( row, 0 )->setFlags( tb->item( row, 0 )->flags() & ~Qt::ItemIsEditable );
  tb->setItem( row, 1, new QTableWidgetItem( value ) );
  tb->setItem( row, 2, new QTableWidgetItem( type ) );
  tb->item( row, 2 )->setFlags( tb->item( row, 2 )->flags() & ~Qt::ItemIsEditable );

  resetButtons();
}

void QgsGrassAttributes::addTextRow( int tab, const QString &text )
{
  QgsDebugMsg( "entered." );

  QTableWidget *tb = static_cast<QTableWidget *>( tabCats->widget( tab ) );

  tb->setRowCount( tb->rowCount() + 1 );

  int row = tb->rowCount() - 1;
  tb->setItem( row, 0, new QTableWidgetItem( text ) );
  tb->item( row, 0 )->setFlags( tb->item( row, 0 )->flags() & ~Qt::ItemIsEditable );
  tb->setSpan( row, 0, 1, 3 );
}

void QgsGrassAttributes::updateAttributes()
{
  QgsDebugMsg( "entered." );

  if ( tabCats->count() == 0 )
    return;

  QTableWidget *tb = static_cast<QTableWidget *>( tabCats->currentWidget() );

  if ( tb->rowCount() > 2 )
  {
    // Stop editing
    QWidget *w = QApplication::focusWidget();
    if ( w )
      w->clearFocus();

    QString sql;

    for ( int i = 2; i < tb->rowCount(); i++ )
    {
      if ( i > 2 )
        sql.append( ", " );

      QString val = tb->item( i, 1 )->text().trimmed();

      if ( val.isEmpty() )
      {
        sql.append( tb->item( i, 0 )->text() + " = null" );
      }
      else
      {
        if ( tb->item( i, 2 )->text() == "int" || tb->item( i, 2 )->text() == "double" )
        {
          sql.append( tb->item( i, 0 )->text() + " = " + val );
        }
        else
        {
          val.replace( "'", "''" );
          sql.append( tb->item( i, 0 )->text() + " = '" + val + "'" );
        }
      }
    }

    QgsDebugMsg( QString( "sql: %1" ).arg( sql ) );

    QString error = mProvider->updateAttributes( tb->item( 0, 1 )->text().toInt(), tb->item( 1, 1 )->text().toInt(), sql );
    if ( !error.isEmpty() )
    {
      QMessageBox::warning( 0, tr( "Warning" ), error );
      resultLabel->setText( tr( "ERROR" ) );
    }
    else
    {
      resultLabel->setText( tr( "OK" ) );
    }
  }
}

void QgsGrassAttributes::addCat()
{
  QgsDebugMsg( "entered." );

  mEdit->addCat( mLine );

  // Select new tab
  tabCats->setCurrentIndex( tabCats->count() - 1 );

  resetButtons();
}

void QgsGrassAttributes::deleteCat()
{
  QgsDebugMsg( "entered." );

  if ( tabCats->count() == 0 )
    return;

  QTableWidget *tb = static_cast<QTableWidget *>( tabCats->currentWidget() );

  int field = tb->item( 0, 1 )->text().toInt();
  int cat = tb->item( 1, 1 )->text().toInt();

  mEdit->deleteCat( mLine, field, cat );

  tabCats->removeTab( tabCats->indexOf( tb ) );
  delete tb;
  resetButtons();
}

void QgsGrassAttributes::clear()
{
  QgsDebugMsg( "entered." );

  while ( tabCats->count() > 0 )
  {
    QTableWidget *tb = static_cast<QTableWidget *>( tabCats->currentWidget() );
    tabCats->removeTab( tabCats->indexOf( tb ) );
    delete tb;
  }
  resetButtons();
}

void QgsGrassAttributes::tabChanged( int index )
{
  Q_UNUSED( index );
  QgsDebugMsg( "entered." );

  resultLabel->setText( "" );
}

void QgsGrassAttributes::setLine( int line )
{
  mLine = line;
}

void QgsGrassAttributes::resetButtons()
{
  if ( tabCats->count() == 0 )
  {
    deleteButton->setEnabled( false );
    updateButton->setEnabled( false );
  }
  else
  {
    deleteButton->setEnabled( true );
    updateButton->setEnabled( true );
  }
}

void QgsGrassAttributes::columnSizeChanged( int section, int oldSize, int newSize )
{
  Q_UNUSED( oldSize );
  QSettings settings;
  QString path = "/GRASS/windows/attributes/columnWidth/"
                 + QString::number( section );
  QgsDebugMsg( QString( "path = %1 newSize = %2" ).arg( path ).arg( newSize ) );
  settings.setValue( path, newSize );
}

void QgsGrassAttributes::setCategoryMode( QgsGrassEdit::CatMode mode, const QString &cat )
{
  if ( mode == QgsGrassEdit::CAT_MODE_NOCAT || ( mode == QgsGrassEdit::CAT_MODE_MANUAL && cat.isEmpty() ) )
  {
    newButton->setEnabled( false );
  }
  else
  {
    newButton->setEnabled( true );
  }
}
