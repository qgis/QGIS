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
#include <qfiledialog.h> 
#include <qsettings.h>
#include <qpixmap.h>
#include <qlistbox.h>
#include <qstringlist.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qmessagebox.h>
#include <qinputdialog.h>
#include <qsettings.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qpen.h>
#include <qpointarray.h>
#include <qcursor.h>
#include <qnamespace.h>
#include <qtabwidget.h>
#include <qtable.h>
#include <qsettings.h>
#include <qevent.h>

#include "../../src/qgis.h"
#include "../../src/qgsmapcanvas.h"
#include "../../src/qgsmaplayer.h"
#include "../../src/qgsvectorlayer.h"
#include "../../src/qgsdataprovider.h"
#include "../../src/qgsmaptopixel.h"
#include "../../src/qgsfeatureattribute.h"

extern "C" {
#include <gis.h>
#include <Vect.h>
}

#include "../../providers/grass/qgsgrass.h"
#include "../../providers/grass/qgsgrassprovider.h"
#include "qgsgrassedit.h"
#include "qgsgrassattributes.h"

QgsGrassAttributesKeyPress::QgsGrassAttributesKeyPress ( QTable *tab )
{
    mTable = tab;
}

QgsGrassAttributesKeyPress::~QgsGrassAttributesKeyPress () {};

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
        QWidget * parent, const char * name, WFlags f ) :QgsGrassAttributesBase ( parent, name, f)
{
    #ifdef QGISDEBUG
    std::cerr << "QgsGrassAttributes()" << std::endl;
    #endif

    mEdit = edit;
    mProvider = provider;
    mLine = line;
      
    resultLabel->setText ( "" );

    // Remove old
    while ( tabCats->count() ) {
        tabCats->removePage( tabCats->currentPage() );
    }

    // TODO: does not work: 
    connect( tabCats, SIGNAL(void currentChanged(QWidget *)), this, SLOT(tabChanged(QWidget *)));

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
  QSettings settings;
  int ww = settings.readNumEntry("/qgis/grass/windows/attributes/w", 250);
  int wh = settings.readNumEntry("/qgis/grass/windows/attributes/h", 350);
  int wx = settings.readNumEntry("/qgis/grass/windows/attributes/x", 100);
  int wy = settings.readNumEntry("/qgis/grass/windows/attributes/y", 100);
  resize(ww,wh);
  move(wx,wy);
}

void QgsGrassAttributes::saveWindowLocation()
{
  QSettings settings;
  QPoint p = this->pos();
  QSize s = this->size();
  settings.writeEntry("/qgis/grass/windows/attributes/x", p.x());
  settings.writeEntry("/qgis/grass/windows/attributes/y", p.y());
  settings.writeEntry("/qgis/grass/windows/attributes/w", s.width());
  settings.writeEntry("/qgis/grass/windows/attributes/h", s.height());
} 

int QgsGrassAttributes::addTab ( const QString & label )
{
    #ifdef QGISDEBUG
    std::cerr << "QgsGrassAttributes::addTab()" << std::endl;
    #endif

    QTable *tb = new QTable ( 2, 3 );
    tb->setColumnReadOnly ( 0, TRUE );
    tb->setColumnReadOnly ( 2, TRUE );
    tb->setRowReadOnly ( 0, TRUE );
    tb->setRowReadOnly ( 1, TRUE );
    
    tb->horizontalHeader()->setLabel( 0, "Column" );
    tb->horizontalHeader()->setLabel( 1, "Value" );
    tb->horizontalHeader()->setLabel( 2, "Type" );  // Internal use

    tb->setLeftMargin(0); // hide row labels

    tb->setText ( 0, 0, "Field" );
    tb->setText ( 1, 0, "Cat" );

    tabCats->addTab ( tb, label );

    // Move down with Tab, unfortunately it does not work if the cell is edited
    // TODO: catch Tab also if the cell is edited
    QgsGrassAttributesKeyPress *ef = new QgsGrassAttributesKeyPress ( tb );
    tb->installEventFilter( ef );

    return ( tabCats->count() - 1 );
}

void QgsGrassAttributes::setField ( int tab, int field )
{
    #ifdef QGISDEBUG
    std::cerr << "QgsGrassAttributes::setField()" << std::endl;
    #endif

    QTable *tb = (QTable *) tabCats->page(tab);

    QString str;
    str.sprintf("%d", field);
    
    tb->setText ( 0, 1, str );
}

void QgsGrassAttributes::setCat ( int tab, const QString & name, int cat )
{
    #ifdef QGISDEBUG
    std::cerr << "QgsGrassAttributes::setField()" << std::endl;
    #endif

    QTable *tb = (QTable *) tabCats->page(tab);
    
    tb->setText ( 1, 0, name );

    QString str;
    str.sprintf("%d", cat);
    
    tb->setText ( 1, 1, str );
}

void QgsGrassAttributes::addAttribute ( int tab, const QString &name, const QString &value, 
                                  const QString &type )
{
    #ifdef QGISDEBUG
    std::cerr << "QgsGrassAttributes::addAttribute(): " << name << ": " << value << std::endl;
    #endif

    QTable *tb = (QTable *) tabCats->page(tab);

    tb->setNumRows ( tb->numRows()+1 );

    int row = tb->numRows()-1;
    tb->setText ( row, 0, name );

    // I have no rational explanation why fromLocal8Bit is necessary, value should be in unicode
    // because QgsGrassProvider::attributes is using mEncoding->toUnicode() 
    tb->setText ( row, 1, QString::fromLocal8Bit(value) );
    tb->setText ( row, 2, type );
}

void QgsGrassAttributes::addTextRow ( int tab, const QString &text )
{
    #ifdef QGISDEBUG
    std::cerr << "QgsGrassAttributes::addTextRow()" << std::endl;
    #endif

    QTable *tb = (QTable *) tabCats->page(tab);

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

    QTable *tb = (QTable *) tabCats->currentPage();

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
      std::cerr << "sql: " << sql << std::endl;
      #endif

      QString *error = mProvider->updateAttributes ( tb->text(0,1).toInt(), tb->text(1,1).toInt(), sql );

      if ( !error->isEmpty() ) {
	  QMessageBox::warning( 0, "Warning", *error );
	  resultLabel->setText ( "ERROR" );
      } else {
	  resultLabel->setText ( "OK" );
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
    
}

void QgsGrassAttributes::deleteCat ( )
{
    #ifdef QGISDEBUG
    std::cerr << "QgsGrassAttributes::deleteCat()" << std::endl;
    #endif
    
    QTable *tb = (QTable *) tabCats->currentPage();

    int field = tb->text(0,1).toInt();
    int cat = tb->text(1,1).toInt();

    mEdit->deleteCat(mLine, field, cat);

    tabCats->removePage( tb );
    delete tb;
}

void QgsGrassAttributes::tabChanged ( QWidget *widget )
{
    #ifdef QGISDEBUG
    std::cerr << "QgsGrassAttributes::tabChanged()" << std::endl;
    #endif
    
    QTable *tb = (QTable *) tabCats->currentPage();

    resultLabel->setText ( "" );
}

void QgsGrassAttributes::setLine ( int line )
{
    mLine = line;
}
