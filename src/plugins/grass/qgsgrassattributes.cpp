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
#include <iostream>
#include <qdir.h>
#include <qfile.h>
#include <qsettings.h>
#include <qpixmap.h>
#include <q3listbox.h>
#include <qstringlist.h>
#include <qlabel.h>
#include <QComboBox>
#include <qspinbox.h>
#include <qmessagebox.h>
#include <qinputdialog.h>
#include <qsettings.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qpen.h>
#include <q3pointarray.h>
#include <qcursor.h>
#include <qnamespace.h>
#include <qtabwidget.h>
#include <q3table.h>
#include <qsettings.h>
#include <qevent.h>
//Added by qt3to4:
#include <QKeyEvent>

#include "qgis.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayer.h"
#include "qgsvectorlayer.h"
#include "qgsdataprovider.h"
#include "qgsmaptopixel.h"

extern "C" {
#include <grass/gis.h>
#include <grass/Vect.h>
}

#include "qgsgrass.h"
#include "qgsgrassprovider.h"
#include "qgsgrassedit.h"
#include "qgsgrassattributes.h"

QgsGrassAttributesKeyPress::QgsGrassAttributesKeyPress ( Q3Table *tab )
{
  mTable = tab;
}

QgsGrassAttributesKeyPress::~QgsGrassAttributesKeyPress () {}

bool QgsGrassAttributesKeyPress::eventFilter( QObject *o, QEvent *e )
{
  if ( e->type() == QEvent::KeyPress ) 
  {
    QKeyEvent *k = (QKeyEvent *)e;

    if ( k->key() == Qt::Key_Tab ) {
      if ( mTable->currentRow() < mTable->numRows()-1 ) 
      {
        mTable->setCurrentCell( mTable->currentRow()+1, mTable->currentColumn() );
      }
      return TRUE; // eat event
    }
  }
  return FALSE; // standard event processing
}

QgsGrassAttributes::QgsGrassAttributes ( QgsGrassEdit *edit, QgsGrassProvider *provider, int line, 
        QWidget * parent, const char * name, Qt::WFlags f ) 
    : QDialog(parent, f ), QgsGrassAttributesBase ()
{
#ifdef QGISDEBUG
  std::cerr << "QgsGrassAttributes()" << std::endl;
#endif

  setupUi(this);

  mEdit = edit;
  mProvider = provider;
  mLine = line;

  resultLabel->setText ( "" );

  // Remove old
  while ( tabCats->count() ) {
    tabCats->removePage( tabCats->currentPage() );
  }

  connect ( this, SIGNAL(destroyed()), mEdit, SLOT(attributesClosed()) );

  // TODO: does not work: 
  connect( tabCats, SIGNAL(void currentChanged(QWidget *)), this, SLOT(tabChanged(QWidget *)));

  resetButtons();
  restorePosition();
}

QgsGrassAttributes::~QgsGrassAttributes ()
{
#ifdef QGISDEBUG
  std::cerr << "~QgsGrassAttributes()" << std::endl;
#endif

  saveWindowLocation();
}

void QgsGrassAttributes::restorePosition()
{
#ifdef QGISDEBUG
  std::cerr << "QgsGrassAttributes::restorePosition()" << std::endl;
#endif
  QSettings settings;
  restoreGeometry(settings.value("/GRASS/windows/attributes/geometry").toByteArray());
}

void QgsGrassAttributes::saveWindowLocation()
{
#ifdef QGISDEBUG
  std::cerr << "QgsGrassAttributes::saveWindowLocation()" << std::endl;
#endif
  QSettings settings;
  settings.setValue("/GRASS/windows/attributes/geometry", saveGeometry());
} 

int QgsGrassAttributes::addTab ( const QString & label )
{
#ifdef QGISDEBUG
  std::cerr << "QgsGrassAttributes::addTab()" << std::endl;
#endif

  Q3Table *tb = new Q3Table ( 2, 3 );
  tb->setColumnReadOnly ( 0, TRUE );
  tb->setColumnReadOnly ( 2, TRUE );
  tb->setRowReadOnly ( 0, TRUE );
  tb->setRowReadOnly ( 1, TRUE );

  tb->horizontalHeader()->setLabel( 0, tr("Column") );
  tb->horizontalHeader()->setLabel( 1, tr("Value") );
  tb->horizontalHeader()->setLabel( 2, tr("Type") );  // Internal use

  tb->setLeftMargin(0); // hide row labels

  tb->setText ( 0, 0, tr("Layer") );
  tb->setText ( 1, 0, "Cat" );

  tabCats->addTab ( tb, label );

  // Move down with Tab, unfortunately it does not work if the cell is edited
  // TODO: catch Tab also if the cell is edited
  QgsGrassAttributesKeyPress *ef = new QgsGrassAttributesKeyPress ( tb );
  tb->installEventFilter( ef );

  resetButtons();

  QSettings settings;
  QString path = "/GRASS/windows/attributes/columnWidth/";
  for ( int i = 0; i < 2; i++ ) 
  {
    bool ok;
    int cw = settings.readNumEntry( path+QString::number(i), 30, &ok);
    if ( ok ) tb->setColumnWidth (i, cw );
  }

  connect ( tb->horizontalHeader(), SIGNAL(sizeChange(int,int,int)),
    this, SLOT(columnSizeChanged(int,int,int)) );

  return ( tabCats->count() - 1 );
}

void QgsGrassAttributes::setField ( int tab, int field )
{
#ifdef QGISDEBUG
  std::cerr << "QgsGrassAttributes::setField()" << std::endl;
#endif

  Q3Table *tb = (Q3Table *) tabCats->page(tab);

  QString str;
  str.sprintf("%d", field);

  tb->setText ( 0, 1, str );
}

void QgsGrassAttributes::setCat ( int tab, const QString & name, int cat )
{
#ifdef QGISDEBUG
  std::cerr << "QgsGrassAttributes::setField()" << std::endl;
#endif

  Q3Table *tb = (Q3Table *) tabCats->page(tab);

  tb->setText ( 1, 0, name );

  QString str;
  str.sprintf("%d", cat);

  tb->setText ( 1, 1, str );
}

void QgsGrassAttributes::addAttribute ( int tab, const QString &name, const QString &value,
                                        const QString &type )
{
#ifdef QGISDEBUG
  std::cerr << "QgsGrassAttributes::addAttribute(): " << name.toLocal8Bit().data() << ": " << value.toLocal8Bit().data() << std::endl;
#endif

  Q3Table *tb = (Q3Table *) tabCats->page(tab);

  tb->setNumRows ( tb->numRows()+1 );

  int row = tb->numRows()-1;
  tb->setText ( row, 0, name );

  // I have no rational explanation why fromLocal8Bit is necessary, value should be in unicode
  // because QgsGrassProvider::attributes is using mEncoding->toUnicode() 
  //    tb->setText ( row, 1, QString::fromLocal8Bit(value) );
  tb->setText ( row, 1, value );
  tb->setText ( row, 2, type );

  resetButtons();
}

void QgsGrassAttributes::addTextRow ( int tab, const QString &text )
{
#ifdef QGISDEBUG
  std::cerr << "QgsGrassAttributes::addTextRow()" << std::endl;
#endif

  Q3Table *tb = (Q3Table *) tabCats->page(tab);

  tb->setNumRows ( tb->numRows()+1 );

  int row = tb->numRows()-1;
  tb->setText ( row, 0, text );
  tb->item(row,0)->setSpan(1,3); // must be after setText() whe item exists
}

void QgsGrassAttributes::updateAttributes ( )
{
#ifdef QGISDEBUG
  std::cerr << "QgsGrassAttributes::updateAttributes()" << std::endl;
#endif

  if ( tabCats->count() == 0 ) return;

  Q3Table *tb = (Q3Table *) tabCats->currentPage();

  if ( tb->numRows() > 2 ) {

    // Stop editing (trick)
    tb->setColumnReadOnly ( 1, TRUE );
    tb->setColumnReadOnly ( 1, FALSE );
    tb->setRowReadOnly ( 0, TRUE );
    tb->setRowReadOnly ( 1, TRUE );

    QString sql;

    for ( int i = 2; i < tb->numRows(); i++ ) {
      if ( i > 2 ) sql.append (", ");

      QString val = tb->text(i, 1).stripWhiteSpace();

      if ( val.isEmpty() ) 
      {
        sql.append ( tb->text(i, 0) + " = null" );
      } 
      else
      {
        if ( tb->text(i, 2) == "int" || tb->text(i, 2) == "double" ) {
          sql.append ( tb->text(i, 0) + " = " + val );
        } else {
          val.replace("'","''");
          sql.append ( tb->text(i, 0) + " = '" + val + "'" );
        }
      }
    }

#ifdef QGISDEBUG
    std::cerr << "sql: " << sql.toLocal8Bit().data() << std::endl;
#endif

    QString *error = mProvider->updateAttributes ( tb->text(0,1).toInt(), tb->text(1,1).toInt(), sql );

    if ( !error->isEmpty() ) {
      QMessageBox::warning( 0, tr("Warning"), *error );
      resultLabel->setText ( tr("ERROR") );
    } else {
      resultLabel->setText ( tr("OK") );
    }

    delete error;
  }
}

void QgsGrassAttributes::addCat ( )
{
#ifdef QGISDEBUG
  std::cerr << "QgsGrassAttributes::addCat()" << std::endl;
#endif

  mEdit->addCat( mLine );

  // Select new tab
  tabCats->setCurrentPage( tabCats->count()-1 );

  resetButtons();
}

void QgsGrassAttributes::deleteCat ( )
{
#ifdef QGISDEBUG
  std::cerr << "QgsGrassAttributes::deleteCat()" << std::endl;
#endif

  if ( tabCats->count() == 0 ) return;

  Q3Table *tb = (Q3Table *) tabCats->currentPage();

  int field = tb->text(0,1).toInt();
  int cat = tb->text(1,1).toInt();

  mEdit->deleteCat(mLine, field, cat);

  tabCats->removePage( tb );
  delete tb;
  resetButtons();
}

void QgsGrassAttributes::clear ( )
{
#ifdef QGISDEBUG
  std::cerr << "QgsGrassAttributes::clear()" << std::endl;
#endif

  while ( tabCats->count() > 0 )
  {
    Q3Table *tb = (Q3Table *) tabCats->currentPage();
    tabCats->removePage( tb );
    delete tb;
  }
  resetButtons();
}

void QgsGrassAttributes::tabChanged ( QWidget *widget )
{
#ifdef QGISDEBUG
  std::cerr << "QgsGrassAttributes::tabChanged()" << std::endl;
#endif

  //Q3Table *tb = (Q3Table *) tabCats->currentPage();

  resultLabel->setText ( "" );
}

void QgsGrassAttributes::setLine ( int line )
{
  mLine = line;
}

void QgsGrassAttributes::resetButtons ( )
{
  if ( tabCats->count() == 0 )
  {
    deleteButton->setEnabled(false);
    updateButton->setEnabled(false);
  }
  else
  {
    deleteButton->setEnabled(true);
    updateButton->setEnabled(true);
  }
}

void QgsGrassAttributes::columnSizeChanged ( int section, int oldSize, int newSize )
{
  QSettings settings;
  QString path = "/GRASS/windows/attributes/columnWidth/"
    + QString::number(section);
  std::cerr << "path = " << path.ascii() << " newSize = " << newSize << std::endl;
  settings.writeEntry( path, newSize);
}
