/***************************************************************************
    qgsgrassselect.cpp  -  Select GRASS layer dialog
                             -------------------
    begin                : March, 2004
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
#include <qlistview.h>
#include <qcolordialog.h>
#include <qtable.h>

#include "../../src/qgis.h"
#include "../../src/qgsmapcanvas.h"
#include "../../src/qgsmaplayer.h"
#include "../../src/qgsvectorlayer.h"
#include "../../src/qgsdataprovider.h"
#include "../../src/qgsmaptopixel.h"
#include "../../src/qgsfield.h"
#include "../../src/qgsfeatureattribute.h"

extern "C" {
#include <gis.h>
#include <Vect.h>
}

#include "../../providers/grass/qgsgrass.h"
#include "../../providers/grass/qgsgrassprovider.h"
#include "qgsgrassattributes.h"
#include "qgsgrassedit.h"

bool QgsGrassEdit::mRunning = false;

QgsGrassEdit::QgsGrassEdit ( QgisApp *qgisApp, QgisIface *iface, 
	                     QWidget * parent, const char * name, WFlags f )
             :QgsGrassEditBase ( parent, name, f )
{
    #ifdef QGISDEBUG
    std::cerr << "QgsGrassEdit()" << std::endl;
    #endif

    mRunning = true;
    mValid = false;
    mTool = QgsGrassEdit::NONE;
    mSuspend = false;
    mQgisApp = qgisApp;
    mIface = iface;

    mCanvas = mIface->getMapCanvas();

    // TODO QGIS: crash if canvas is empty
    QgsMapLayer *layer = (QgsMapLayer *) mIface->activeLayer();

    if ( !layer ) {
	std::cerr << "No layer is selected." << std::endl;
	QMessageBox::warning( 0, "Warning", "No layer is selected." );
	return;
    }

    std::cerr << "layer name: " << layer->name() << std::endl;

    if ( layer->type() != QgsMapLayer::VECTOR ) {
	std::cerr << "The selected layer is not vector." << std::endl;
	QMessageBox::warning( 0, "Warning", "The selected layer is not vector." );
	return;
    }

    //TODO dynamic_cast ?
    QgsVectorLayer *vector = (QgsVectorLayer*)layer;

    std::cerr << "Vector layer type: " << vector->providerType() << std::endl;

    if ( vector->providerType() != "grass" ) {
	QMessageBox::warning( 0, "Warning", "The selected vector is not in GRASS format." );
	return;
    }

    //TODO dynamic_cast ?
    mProvider = (QgsGrassProvider *) vector->getDataProvider();

    if ( !(mProvider->isGrassEditable()) ) {
	QMessageBox::warning( 0, "Warning", "You are not owner of the mapset, "
		                 "cannot open the vector for editing." );
	return;
    }
    
    std::cerr << "Vector layer type: " << vector->providerType() << std::endl;
    if ( !(mProvider->startEdit()) ) {
	QMessageBox::warning( 0, "Warning", "Cannot open vector for update." );
	return;
    }

    mEditPoints = Vect_new_line_struct ();
    mPoints = Vect_new_line_struct ();
    mCats = Vect_new_cats_struct ();

    // Set lines symbology from map
    int nlines = mProvider->numLines(); 
    mLineSymb.resize(nlines+1000);
    for ( int line = 1; line <= nlines; line++ ) {
	mLineSymb[line] = lineSymbFromMap ( line );
    }
	
    // Set nodes symbology from map
    int nnodes = mProvider->numNodes(); 
    mNodeSymb.resize(nnodes+1000); 
    for ( int node = 1; node <= nnodes; node++ ) {
	mNodeSymb[node] = nodeSymbFromMap ( node );
    }
    
    // Set default colors
    mSymb.resize(SYMB_COUNT);
    mSymb[SYMB_BACKGROUND].setColor    ( QColor ( 255, 255, 255 ) );  // white
    mSymb[SYMB_HIGHLIGHT].setColor     ( QColor ( 255, 255,   0 ) );  // yellow
    mSymb[SYMB_DYNAMIC].setColor       ( QColor ( 125, 125, 125 ) );  // grey
    mSymb[SYMB_POINT].setColor         ( QColor (   0,   0,   0 ) );  // black
    mSymb[SYMB_LINE].setColor          ( QColor (   0,   0,   0 ) );  // black
    mSymb[SYMB_BOUNDARY_0].setColor    ( QColor ( 255,   0,   0 ) );  // red
    mSymb[SYMB_BOUNDARY_1].setColor    ( QColor ( 255, 125,   0 ) );  // orange
    mSymb[SYMB_BOUNDARY_2].setColor    ( QColor (   0, 255,   0 ) );  // green
    mSymb[SYMB_CENTROID_IN].setColor   ( QColor (   0, 255,   0 ) );  // green
    mSymb[SYMB_CENTROID_OUT].setColor  ( QColor ( 255,   0,   0 ) );  // red
    mSymb[SYMB_CENTROID_DUPL].setColor ( QColor ( 255,   0, 255 ) );  // magenta
    mSymb[SYMB_NODE_1].setColor        ( QColor ( 255,   0,   0 ) );  // red
    mSymb[SYMB_NODE_2].setColor        ( QColor (   0, 255,   0 ) );  // green

    // Set mSymbDisplay
    mSymbDisplay.resize(SYMB_COUNT);
    mSymbDisplay[SYMB_BACKGROUND] = true;
    mSymbDisplay[SYMB_HIGHLIGHT] = true;
    mSymbDisplay[SYMB_DYNAMIC] = true;
    mSymbDisplay[SYMB_POINT] = true;
    mSymbDisplay[SYMB_LINE] = true;
    mSymbDisplay[SYMB_BOUNDARY_0] = true;
    mSymbDisplay[SYMB_BOUNDARY_1] = true;
    mSymbDisplay[SYMB_BOUNDARY_2] = true;
    mSymbDisplay[SYMB_CENTROID_IN] = true;
    mSymbDisplay[SYMB_CENTROID_OUT] = true;
    mSymbDisplay[SYMB_CENTROID_DUPL] = true;
    mSymbDisplay[SYMB_NODE_1] = true;
    mSymbDisplay[SYMB_NODE_2] = true;

    // Set symbology names
    mSymbName.resize(SYMB_COUNT);
    mSymbName[SYMB_BACKGROUND]    = "Background";
    mSymbName[SYMB_HIGHLIGHT]     = "Highlight";
    mSymbName[SYMB_DYNAMIC]       = "Dynamic (XOR mode)";
    mSymbName[SYMB_POINT]         = "Point";
    mSymbName[SYMB_LINE]          = "Line";
    mSymbName[SYMB_BOUNDARY_0]    = "Boundary (no area)";
    mSymbName[SYMB_BOUNDARY_1]    = "Boundary (1 area)";
    mSymbName[SYMB_BOUNDARY_2]    = "Boundary (2 areas)";
    mSymbName[SYMB_CENTROID_IN]   = "Centroid (in area)";
    mSymbName[SYMB_CENTROID_OUT]  = "Centroid (outside area)";
    mSymbName[SYMB_CENTROID_DUPL] = "Centroid (duplicate in area)";
    mSymbName[SYMB_NODE_1]        = "Node (1 line)";
    mSymbName[SYMB_NODE_2]        = "Node (2 lines)";

    // Restore symbology
    QSettings settings;
    for ( int i = 0; i < SYMB_COUNT; i++ ) {
	bool ok;
	QString sn;
        sn.sprintf( "/qgis/grass/edit/symb/display/%d", i );
        bool displ = settings.readBoolEntry (sn, true, &ok );
	if ( ok ) {
	    mSymbDisplay[i] = displ;
	}
        
        sn.sprintf( "/qgis/grass/edit/symb/color/%d", i );
	QString colorName = settings.readEntry (sn, "", &ok );
	if ( ok ) {
            QColor color( colorName );
	    mSymb[i].setColor( color );
	}
    }

    // Set Symbology in dialog
    symbologyList->setColumnText(0,"Disp");
    symbologyList->setColumnWidth(0,20);
    symbologyList->addColumn("Color");
    symbologyList->setColumnWidth(0,50);
    symbologyList->addColumn("Type");
    symbologyList->setColumnWidthMode(2,QListView::Maximum);
    symbologyList->addColumn("Index", 0);
    symbologyList->clear();
    symbologyList->setSorting(-1);

    for ( int i = SYMB_COUNT-1; i >= 0; i-- ) {
	if ( i == SYMB_NODE_0 ) continue;

	QPixmap pm ( 40, 15 );
	pm.fill( mSymb[i].color() );
	QString index;
	index.sprintf ("%d", i );

        if ( i == SYMB_BACKGROUND || i == SYMB_HIGHLIGHT || i == SYMB_DYNAMIC ) {	
            QListViewItem *lvi = new QListViewItem ( symbologyList , "", "", mSymbName[i] );
	    lvi->setPixmap ( 1, pm );
	    lvi->setText ( 3, index );
        } else {
            QCheckListItem *clvi = new QCheckListItem ( symbologyList , "", QCheckListItem::CheckBox );
	    clvi->setText ( 2, mSymbName[i] );
	    clvi->setPixmap ( 1, pm );
	    clvi->setOn ( mSymbDisplay[i] );
	    clvi->setText ( 3, index );
	}
    }
    
    connect( symbologyList, SIGNAL(pressed(QListViewItem *, const QPoint &, int)), 
	                           this, SLOT(changeSymbology(QListViewItem *, const QPoint &, int)));

    // Init table tab
    mAttributeTable->setLeftMargin(0); // hide row labels
    mAttributeTable->horizontalHeader()->setLabel( 0, "Column" );
    mAttributeTable->horizontalHeader()->setLabel( 1, "Type" );
    mAttributeTable->horizontalHeader()->setLabel( 2, "Length" );

    int ndblinks = mProvider->numDbLinks();

    if ( ndblinks > 0 ) {
	for ( int i = 0; i < ndblinks; i++ ) {
	    int f = mProvider->dbLinkField ( i ); 

	    QString str;
	    str.sprintf ( "%d", f );	
	    mTableField->insertItem ( str );
            mFieldBox->insertItem( str );
	    if ( i == 0 ) {
		setAttributeTable( f );
	    }
	}
	mTableField->setCurrentItem ( 0 ); 
        mFieldBox->setCurrentItem ( 0 );
    } else { 
	mTableField->insertItem ( "1" );
	setAttributeTable ( 1 );
    
	mFieldBox->insertItem("1");
    }
    
    connect( mAttributeTable, SIGNAL(valueChanged(int,int)), this, SLOT(columnTypeChanged(int,int)) );

    // Set variables
    mSize = 9;
    mLastDynamicIcon = ICON_NONE;
    mLastDynamicPoints.resize(0);
    mSelectedLine = 0;
    mAttributes = 0;

    // Read max cats
    for (int i = 0; i < mProvider->cidxGetNumFields(); i++ ) {
	int field = mProvider->cidxGetFieldNumber(i);
	if ( field > 0 ) {
	    int cat = mProvider->cidxGetMaxCat(i);
	    MaxCat mc;
	    mc.field = field;
	    mc.maxCat = cat;
	    mMaxCats.push_back(mc);
	}
    }

    connect( mCanvas, SIGNAL(xyClickCoordinates(QgsPoint &)), 
	     this, SLOT(mouseEventReceiverClick(QgsPoint &)));
    connect( mCanvas, SIGNAL(xyCoordinates(QgsPoint &)), 
	     this, SLOT(mouseEventReceiverMove(QgsPoint &)));
    connect( mCanvas, SIGNAL(renderComplete(QPainter *)), this, SLOT(postRender(QPainter *)));
    
    mPixmap = mCanvas->canvasPixmap();

    // Init GUI values
    mCatModeBox->insertItem( "Next not used", CAT_MODE_NEXT );
    mCatModeBox->insertItem( "Manual entry", CAT_MODE_MANUAL );
    mCatModeBox->insertItem( "No category", CAT_MODE_NOCAT );
    catModeChanged ( );
	
    // TODO: how to get keyboard events from canvas (shortcuts)
    
    restorePosition();
    
    mValid = true; 
}

void QgsGrassEdit::attributeTableFieldChanged ( void )
{
    int field = mTableField->currentText().toInt();

    setAttributeTable ( field );
}

void QgsGrassEdit::setAttributeTable ( int field )
{
    mAttributeTable->setNumRows ( 0 );
    
    QString *key = mProvider->key ( field );

    if ( !key->isEmpty() ) { // Database link defined
	std::vector<QgsField> *cols = mProvider->columns ( field );

	mAttributeTable->setNumRows ( cols->size() );


	for ( int c = 0; c < cols->size(); c++ ) {
	    QgsField col = (*cols)[c];

	    QTableItem *ti;
	    
	    ti = new QTableItem( mAttributeTable, QTableItem::Never, col.name() );
	    ti->setEnabled( false );
	    mAttributeTable->setItem ( c, 0, ti );
	    
	    ti = new QTableItem( mAttributeTable, QTableItem::Never, col.type() );
	    ti->setEnabled( false );
	    mAttributeTable->setItem ( c, 1, ti );
	    
	    QString str;
	    str.sprintf("%d", col.length() );
	    ti = new QTableItem( mAttributeTable, QTableItem::Never, str );
	    ti->setEnabled( false );
	    mAttributeTable->setItem ( c, 2, ti );
	}
    } else {
        mAttributeTable->setNumRows ( 1 );

	QTableItem *ti;
	
	ti = new QTableItem( mAttributeTable, QTableItem::Always, "cat" );
	mAttributeTable->setItem ( 0, 0, ti );

	ti = new QTableItem( mAttributeTable, QTableItem::Never, "integer" );
	ti->setEnabled( false );
	mAttributeTable->setItem ( 0, 1, ti );
	
	ti = new QTableItem( mAttributeTable, QTableItem::Never, "" );
	ti->setEnabled( false );
	mAttributeTable->setItem ( 0, 2, ti );
    }
}

void QgsGrassEdit::addColumn ( void )
{
    int r = mAttributeTable->numRows();
    mAttributeTable->setNumRows( r+1 );
    mAttributeTable->setRowReadOnly ( r, false );

    QString cn;
    cn.sprintf ( "column%d", r+1 );

    QTableItem *ti;
    
    ti = new QTableItem( mAttributeTable, QTableItem::Always, cn );
    mAttributeTable->setItem ( r, 0, ti );

    QStringList types;
    types.push_back ( "integer" );
    types.push_back ( "double precision" );
    types.push_back ( "varchar" );

    QComboTableItem *cti = new QComboTableItem ( mAttributeTable, types ); 
    cti->setCurrentItem(0);
    mAttributeTable->setItem ( r, 1, cti );

    ti = new QTableItem( mAttributeTable, QTableItem::Never, "20" );
    ti->setEnabled(false);
    mAttributeTable->setItem ( r, 2, ti );
}

void QgsGrassEdit::columnTypeChanged ( int row, int col )
{
    #ifdef QGISDEBUG
    std::cerr << "QgsGrassEdit::columnChanged() row = " << row << " col = " << col << std::endl;
    #endif

    if ( col != 1 ) return;

    QComboTableItem *cti = (QComboTableItem *) mAttributeTable->item ( row, 1 ); 
    
    QTableItem *ti = mAttributeTable->item ( row, 2 );

    if ( cti->currentText().compare( "varchar" ) != 0 ) {
        QTableItem *nti = new QTableItem( mAttributeTable, QTableItem::Never, ti->text() );
	nti->setEnabled(false);
        mAttributeTable->setItem ( row, 2, nti );
	//delete ti;
    } else {
        QTableItem *nti = new QTableItem( mAttributeTable, QTableItem::Always, ti->text() );
	nti->setEnabled(true);
        mAttributeTable->setItem ( row, 2, nti );
	//delete ti;
    }
    mAttributeTable->updateCell ( row, 2 );
}

void QgsGrassEdit::alterTable ( void )
{
    #ifdef QGISDEBUG
    std::cerr << "QgsGrassEdit::alterTable()" << std::endl;
    #endif

    // Create new table if first column name is editable otherwise alter table
    int field = mTableField->currentText().toInt();

    QTableItem *ti;
    ti = mAttributeTable->item ( 0, 0 );

    QString sql;
    
    if ( mAttributeTable->item(0,0)->isEnabled() ) {
	#ifdef QGISDEBUG
	std::cerr << "Create new table" << std::endl;
	#endif

	for ( int i = 0; i < mAttributeTable->numRows(); i++ ) {
	    if ( i > 0 ) sql.append(", " );

	    
	    sql.append ( mAttributeTable->item(i,0)->text() + " " + mAttributeTable->item(i,1)->text() );

	    if ( mAttributeTable->item(i,1)->text().compare("varchar") == 0 ) {
		sql.append ( " (" + mAttributeTable->item(i,2)->text() + ")" );
	    }
	}
	
	QString *error = mProvider->createTable ( field, mAttributeTable->item(0,0)->text(), sql );  

	if ( !error->isEmpty() ) {
	    QMessageBox::warning( 0, "Warning", *error );
	} else {
	    QMessageBox::information( 0, "Info", "The table was created" );
	    QString str;
	    str.sprintf ( "%d", field );
            mFieldBox->insertItem( str );
	}
	delete error;
    } else { 
	#ifdef QGISDEBUG
	std::cerr << "Alter table" << std::endl;
	#endif

	for ( int i = 0; i < mAttributeTable->numRows(); i++ ) {
       	    if ( !(mAttributeTable->item(i,0)->isEnabled()) ) continue;
	    
	    sql = mAttributeTable->item(i,0)->text() + " " + mAttributeTable->item(i,1)->text();

	    if ( mAttributeTable->item(i,1)->text().compare("varchar") == 0 ) {
		sql.append ( " (" + mAttributeTable->item(i,2)->text() + ")" );
	    }

	    QString *error = mProvider->addColumn ( field, sql );  

	    if ( !error->isEmpty() ) {
		QMessageBox::warning( 0, "Warning", *error );
	    }
	    delete error;
	}
    }

    setAttributeTable ( field );
}

void QgsGrassEdit::changeSymbology(QListViewItem * item, const QPoint & pnt, int col)
{
    #ifdef QGISDEBUG
    std::cerr << "QgsGrassEdit::changeSymbology() col = " << col << std::endl;
    #endif
  
    QSettings settings;

    if ( !item ) return;

    int index = item->text(3).toInt();
    
    if ( col == 0 ) { 
        if ( index == SYMB_BACKGROUND || index == SYMB_HIGHLIGHT || index == SYMB_DYNAMIC ) return;	

    	QCheckListItem *clvi = (QCheckListItem *) item;
	mSymbDisplay[index] = clvi->isOn();
  
	int ww = settings.readNumEntry("/qgis/grass/windows/edit/w", 420);
	QString sn;
	// TODO use a name instead of index
        sn.sprintf( "/qgis/grass/edit/symb/display/%d", index );
  	settings.writeEntry ( sn, mSymbDisplay[index] );
    } else if ( col == 1 ) {
        QColor color = QColorDialog::getColor ( mSymb[index].color(), this );
	mSymb[index].setColor( color );

	QPixmap pm ( 40, 15 );
	pm.fill( mSymb[index].color() );
	item->setPixmap ( 1, pm );

	QString sn;
	// TODO use a name instead of index
        sn.sprintf( "/qgis/grass/edit/symb/color/%d", index );
  	settings.writeEntry ( sn, mSymb[index].color().name() );
    }
}

void QgsGrassEdit::restorePosition()
{
  QSettings settings;
  int ww = settings.readNumEntry("/qgis/grass/windows/edit/w", 420);
  int wh = settings.readNumEntry("/qgis/grass/windows/edit/h", 150);
  int wx = settings.readNumEntry("/qgis/grass/windows/edit/x", 100);
  int wy = settings.readNumEntry("/qgis/grass/windows/edit/y", 100);
  resize(ww,wh);
  move(wx,wy);
}

void QgsGrassEdit::saveWindowLocation()
{
  QSettings settings;
  QPoint p = this->pos();
  QSize s = this->size();
  settings.writeEntry("/qgis/grass/windows/edit/x", p.x());
  settings.writeEntry("/qgis/grass/windows/edit/y", p.y());
  settings.writeEntry("/qgis/grass/windows/edit/w", s.width());
  settings.writeEntry("/qgis/grass/windows/edit/h", s.height());
} 

void QgsGrassEdit::updateSymb ( void )
{
    #ifdef QGISDEBUG
    std::cerr << "QgsGrassEdit::updateSymb" << std::endl;
    #endif

    // Set lines symbology from map
    int nlines = mProvider->numLines(); 
    if ( nlines+1 >= mLineSymb.size() )
        mLineSymb.resize(nlines+1000); 

    nlines = mProvider->numUpdatedLines();
    for ( int i = 0; i < nlines; i++ ) {
	int line = mProvider->updatedLine(i);
	std::cerr << "updated line = " << line << std::endl;
	if ( !(mProvider->lineAlive(line)) ) continue;
	mLineSymb[line] = lineSymbFromMap ( line );
    }
	
    // Set nodes symbology from map
    int nnodes = mProvider->numNodes(); 
    if ( nnodes+1 >= mNodeSymb.size() )
        mNodeSymb.resize(nnodes+1000); 
    
    nnodes = mProvider->numUpdatedNodes(); 
    for ( int i = 0; i < nnodes; i++ ) {
	int node = mProvider->updatedNode(i);
	if ( !(mProvider->nodeAlive(node)) ) continue;
	mNodeSymb[node] = nodeSymbFromMap ( node );
	std::cerr << "node = " << node << " mNodeSymb = " << mNodeSymb[node] << std::endl;
    }
}

int QgsGrassEdit::nodeSymbFromMap ( int node )
{
    #ifdef QGISDEBUG
    std::cerr << "QgsGrassEdit::nodeSymbFromMap() node = " <<  node << std::endl;
    #endif

    int nlines = mProvider->nodeNLines ( node );

    int count = 0;
    
    for ( int i = 0; i < nlines; i++ ) {
	int line = abs (  mProvider->nodeLine(node,i) );
	int type = mProvider->readLine ( NULL, NULL, line );
	
	if ( type & GV_LINES )
	    count++;
    }

    if ( count == 0 ) 
	return SYMB_NODE_0;
    else if ( count == 1 )
	return SYMB_NODE_1;
    
    return SYMB_NODE_2;
}

int QgsGrassEdit::lineSymbFromMap ( int line )
{
    #ifdef QGISDEBUG
    std::cerr << "QgsGrassEdit::lineSymbFromMap() line = " << line << std::endl;
    #endif
    
    int type = mProvider->readLine ( NULL, NULL, line );

    if ( type < 0 ) return 0;

    switch ( type ) {
	case GV_POINT:
	    return SYMB_POINT;
	    break;

	case GV_LINE:
	    return SYMB_LINE;
	    break;

	case GV_BOUNDARY:
            int left, right, nareas;
	    
	    if ( !(mProvider->lineAreas(line, &left, &right)) ) return 0;

	    /* Count areas/isles on both sides */
	    nareas = 0;
	    if ( left != 0 ) nareas++;
	    if ( right != 0 ) nareas++;
	    if ( nareas == 0 ) return SYMB_BOUNDARY_0;
	    else if ( nareas == 1 ) return SYMB_BOUNDARY_1;
	    else return SYMB_BOUNDARY_2;
	    break;

	case GV_CENTROID:
	    int area = mProvider->centroidArea ( line );
	    if ( area == 0 ) return SYMB_CENTROID_OUT;
	    else if ( area > 0 ) return SYMB_CENTROID_IN;  
	    else return SYMB_CENTROID_DUPL; /* area < 0 */ 
	    break;
    }

    return 0; // Should not happen
}

QgsGrassEdit::~QgsGrassEdit()
{
    #ifdef QGISDEBUG
    std::cerr << "QgsGrassEdit::~QgsGrassEdit()" << std::endl;
    #endif

    if ( mValid ) 
        eraseDynamic();

    saveWindowLocation();
    mRunning = false;
}

bool QgsGrassEdit::isRunning(void)
{
    return mRunning;
}

bool QgsGrassEdit::isValid(void)
{
    return mValid;
}

void QgsGrassEdit::closeEdit(void)
{
    #ifdef QGISDEBUG
    std::cerr << "QgsGrassEdit::close()" << std::endl;
    #endif
    
    if ( mAttributes ) {
	delete mAttributes;
    }

    mProvider->closeEdit();
    
    close();

    delete this; 
}

void QgsGrassEdit::catModeChanged ( void )
{
    int mode = mCatModeBox->currentItem();

    int field = mFieldBox->currentText().toInt();

    if ( mode == CAT_MODE_NEXT ) { // Find next not used
	QString c = "1"; // Default for new field
	for (int i = 0; i < mMaxCats.size(); i++ ) {
	    if ( mMaxCats[i].field == field ) {
		c.sprintf("%d", mMaxCats[i].maxCat+1);
		break;
	    }
	}
	mCatEntry->setText ( c );
	mCatEntry->setEnabled(false);
	mFieldBox->setDisabled(false);
    } else if ( mode == CAT_MODE_MANUAL ) {
	mCatEntry->setEnabled(true);
	mFieldBox->setDisabled(false);
    } else { // CAT_MODE_NOCAT
	mCatEntry->clear ();
	mCatEntry->setEnabled(false);
	mFieldBox->setDisabled(true);
    }
}

void QgsGrassEdit::fieldChanged ( void )
{
    int mode = mCatModeBox->currentItem();
    int field = mFieldBox->currentText().toInt();

    if ( mode == CAT_MODE_NEXT ) { // Find next not used
	QString c = "1"; // Default for new field
	for (int i = 0; i < mMaxCats.size(); i++ ) {
	    if ( mMaxCats[i].field == field ) {
		c.sprintf("%d", mMaxCats[i].maxCat+1);
		break;
	    }
	}
	mCatEntry->setText ( c );
    }
}

int QgsGrassEdit::writeLine ( int type, struct line_pnts *Points )
{
    int mode = mCatModeBox->currentItem();
    int field = mFieldBox->currentText().toInt();
    int cat = mCatEntry->text().toInt();

    Vect_reset_cats ( mCats );
    if ( mode == CAT_MODE_NEXT || mode == CAT_MODE_MANUAL ) {
	Vect_cat_set ( mCats, field, cat );

	// Insert new DB record if link is defined and the record for this cat does not exist
	QString *key = mProvider->key ( field );
	
	if ( !key->isEmpty() ) { // Database link defined 
	    std::vector<QgsFeatureAttribute> *atts = mProvider->attributes ( field, cat );

	    if ( atts->size() == 0 ) { // Nothing selected
		QString *error = mProvider->insertAttributes ( field, cat );

		if ( !error->isEmpty() ) {
		     QMessageBox::warning( 0, "Warning", *error );
		}
		delete error;
	    }

	    delete atts;
	}
    }
    Vect_line_prune ( Points );
    int line = mProvider->writeLine ( type, Points, mCats );

    increaseMaxCat();
    return line;
}

void QgsGrassEdit::increaseMaxCat ( void )
{
    int mode = mCatModeBox->currentItem();
    int field = mFieldBox->currentText().toInt();
    int cat = mCatEntry->text().toInt();

    if ( mode == CAT_MODE_NEXT || mode == CAT_MODE_MANUAL ) {
	int found = 0;
	for (int i = 0; i < mMaxCats.size(); i++ ) {
	    if ( mMaxCats[i].field == field ) {
		if ( cat > mMaxCats[i].maxCat ) {
		   mMaxCats[i].maxCat = cat;
		}
		found = 1;
		break;
	    }
	}
	if ( !found ) { 
	    MaxCat mc;
	    mc.field = field;
	    mc.maxCat = cat;
	    mMaxCats.push_back(mc);
	}

	if ( mode == CAT_MODE_NEXT ) { 
	    QString c; 
	    c.sprintf("%d", cat+1);
	    mCatEntry->setText ( c );
	}
    }

}

double QgsGrassEdit::threshold ( void )
{
    int snapPixels = mSnapPixels->text().toInt();

    // Convert to map units (not nice)
    mTransform = mCanvas->getCoordinateTransform();
    double x1 = mTransform->toMapCoordinates( 0, 0 ).x();
    double x2 = mTransform->toMapCoordinates( snapPixels, 0 ).x();
    
    return ( x2 - x1 );
}

void QgsGrassEdit::snap (  double *x, double *y )
{
    double thresh = threshold();

    int node = mProvider->findNode ( *x, *y, thresh );

    if ( node > 0 ) {
	mProvider->nodeCoor ( node, x, y );
    }
}

void QgsGrassEdit::snap (  QgsPoint & point )
{
    double x = point.x();
    double y = point.y();

    snap ( &x, &y );
    
    point.setX(x);
    point.setY(y);
}

void QgsGrassEdit::newPoint(void) { startTool(QgsGrassEdit::NEW_POINT); }
void QgsGrassEdit::newLine(void) { 
    std::cerr << "QgsGrassEdit::newLine" << std::endl;
    startTool(QgsGrassEdit::NEW_LINE); 
}
void QgsGrassEdit::newBoundary(void) { 
    std::cerr << "QgsGrassEdit::newBoundary" << std::endl;
    startTool(QgsGrassEdit::NEW_BOUNDARY); 
}
void QgsGrassEdit::newCentroid(void) { startTool(QgsGrassEdit::NEW_CENTROID); }
void QgsGrassEdit::moveVertex(void) { startTool(QgsGrassEdit::MOVE_VERTEX); }
void QgsGrassEdit::addVertex(void) { startTool(QgsGrassEdit::ADD_VERTEX); }
void QgsGrassEdit::deleteVertex(void) { startTool(QgsGrassEdit::DELETE_VERTEX); }
void QgsGrassEdit::splitLine(void) { startTool(QgsGrassEdit::SPLIT_LINE); }
void QgsGrassEdit::moveLine(void) { startTool(QgsGrassEdit::MOVE_LINE); }
void QgsGrassEdit::deleteLine(void) { startTool(QgsGrassEdit::DELETE_LINE); }
void QgsGrassEdit::editCats(void) { startTool(QgsGrassEdit::EDIT_CATS); }
void QgsGrassEdit::editAttributes(void) { startTool(QgsGrassEdit::EDIT_ATTRIBUTES); }

void QgsGrassEdit::startTool(int tool)
{
    #ifdef QGISDEBUG
    std::cerr << "QgsGrassEdit::startTool() tool = " << tool << std::endl;
    #endif
    
    // Delete last dynamic drawing from canvas
    eraseDynamic();
    if ( mSelectedLine > 0 )
	displayElement ( mSelectedLine, mSymb[mLineSymb[mSelectedLine]], mSize );

    /* Close old tool */
    switch ( mTool ) {
	case QgsGrassEdit::NEW_LINE:
	case QgsGrassEdit::NEW_BOUNDARY:
	    /* Write the line to vector */
	    if ( mEditPoints->n_points > 1 ) {
		int type;
		
		if ( mTool == QgsGrassEdit::NEW_LINE )
		    type = GV_LINE;
		else // boundary
		    type = GV_BOUNDARY;
		
		int line;
	        line = writeLine ( type, mEditPoints );
		updateSymb();
	        displayUpdated();

		if ( mAttributes ) delete mAttributes;
		mAttributes = new QgsGrassAttributes ( this, mProvider, line, mQgisApp );
		for ( int i = 0; i < mCats->n_cats; i++ ) {
		    addAttributes ( mCats->field[i], mCats->cat[i] );
		}
		mAttributes->show();
	    }
	    break;
	    
	case QgsGrassEdit::NONE:
	case QgsGrassEdit::NEW_POINT:
	case QgsGrassEdit::NEW_CENTROID:
	case QgsGrassEdit::MOVE_VERTEX:
	case QgsGrassEdit::ADD_VERTEX:
	case QgsGrassEdit::DELETE_VERTEX:
	case QgsGrassEdit::SPLIT_LINE:
	case QgsGrassEdit::MOVE_LINE:
	case QgsGrassEdit::DELETE_LINE:
	case QgsGrassEdit::EDIT_CATS:
	case QgsGrassEdit::EDIT_ATTRIBUTES:
	    // Nothing to do
	    break;
	default:
	    std::cerr << "Unknown tool" << std::endl;
	    break;
    }

    // All necessary data were written -> reset mEditPoints etc.
    Vect_reset_line ( mEditPoints );
    mSelectedLine = 0;
    
    // TODO: mTool != NEW_LINE is a hack for lines until more buttons can be recieved
    if ( mAttributes && mTool != QgsGrassEdit::NEW_LINE && mTool != QgsGrassEdit::NEW_BOUNDARY ) {
	delete mAttributes;
	mAttributes = 0;
    }

    // Start new tool
    mTool = tool;

    switch ( mTool ) {
	case QgsGrassEdit::NONE:
	case QgsGrassEdit::NEW_POINT:
	case QgsGrassEdit::NEW_LINE:
	case QgsGrassEdit::NEW_BOUNDARY:
	case QgsGrassEdit::NEW_CENTROID:
	case QgsGrassEdit::MOVE_VERTEX:
	case QgsGrassEdit::ADD_VERTEX:
	case QgsGrassEdit::DELETE_VERTEX:
	case QgsGrassEdit::MOVE_LINE:
	case QgsGrassEdit::SPLIT_LINE:
	case QgsGrassEdit::DELETE_LINE:
	case QgsGrassEdit::EDIT_ATTRIBUTES:
	    break;
	case QgsGrassEdit::EDIT_CATS:
	    mTool = QgsGrassEdit::NONE;
	    QMessageBox::warning( 0, "Warning", "Tool not yet implemented." );
	    break;
	default:
	    std::cerr << "Unknown tool" << std::endl;
	    break;
    }
    
    mCanvas->setMapTool ( QGis::CapturePoint );
    mCanvas->setCursor (  Qt::CrossCursor );
}

void QgsGrassEdit::mouseEventReceiverClick( QgsPoint & point )
{
    #ifdef QGISDEBUG
    std::cerr << "QgsGrassEdit::mouseEventReceiverClick()" << std::endl;
    #endif

    double thresh = threshold();
    QPen pen;

    switch ( mTool ) {
	case QgsGrassEdit::NEW_POINT:
	case QgsGrassEdit::NEW_CENTROID:
	    Vect_reset_line ( mEditPoints );
	    snap ( point ); 
	    Vect_append_point ( mEditPoints, point.x(), point.y(), 0.0 );
	    
	    int type;
	    if ( mTool == QgsGrassEdit::NEW_POINT )
		type = GV_POINT;
	    else // centroid
		type = GV_CENTROID;

	    int line;
	    line = writeLine ( type, mEditPoints );
	    updateSymb();
	    displayUpdated();

	    if ( mAttributes ) delete mAttributes;
	    mAttributes = new QgsGrassAttributes ( this, mProvider, line, mQgisApp );
	    for ( int i = 0; i < mCats->n_cats; i++ ) {
		addAttributes ( mCats->field[i], mCats->cat[i] );
	    }
	    mAttributes->show();
	    break;

	case QgsGrassEdit::NEW_LINE:
	case QgsGrassEdit::NEW_BOUNDARY:
	    snap ( point ); 
	    Vect_append_point ( mEditPoints, point.x(), point.y(), 0.0 );
	    break;
	    
	case QgsGrassEdit::DELETE_LINE:
	    // Delete previously selected line 
	    if ( mSelectedLine > 0 ) {
		eraseElement ( mSelectedLine );
		mProvider->deleteLine ( mSelectedLine );
		updateSymb();
		displayUpdated();
	    }

	    // Select new/next line
            mSelectedLine = mProvider->findLine ( point.x(), point.y(), GV_POINT|GV_CENTROID, thresh );

            if ( mSelectedLine == 0 ) 
                mSelectedLine = mProvider->findLine ( point.x(), point.y(), GV_LINE|GV_BOUNDARY, thresh );

	    if ( mSelectedLine ) { // highlite
		displayElement ( mSelectedLine, mSymb[SYMB_HIGHLIGHT], mSize );
	    }
	    
	    break;

	case QgsGrassEdit::MOVE_LINE:
	    // Move previously selected line 
	    if ( mSelectedLine > 0 ) {
		eraseDynamic();
		eraseElement ( mSelectedLine );

		// Transform coordinates
		int type = mProvider->readLine ( mPoints, mCats, mSelectedLine );
		for ( int i = 0; i < mPoints->n_points; i++ ) {
		   mPoints->x[i] += point.x() - mLastPoint.x(); 
		   mPoints->y[i] += point.y() - mLastPoint.y(); 
		}
		
		mProvider->rewriteLine ( mSelectedLine, type, mPoints, mCats );
		updateSymb();
		displayUpdated();

		mSelectedLine = 0;
		Vect_reset_line ( mEditPoints );
	    } else {
		// Select new/next line
		mSelectedLine = mProvider->findLine ( point.x(), point.y(), GV_POINT|GV_CENTROID, thresh );

		if ( mSelectedLine == 0 ) 
		    mSelectedLine = mProvider->findLine ( point.x(), point.y(), GV_LINE|GV_BOUNDARY, thresh );

		if ( mSelectedLine ) { // highlite
		    mProvider->readLine ( mEditPoints, NULL, mSelectedLine );
		    displayElement ( mSelectedLine, mSymb[SYMB_HIGHLIGHT], mSize );
		}
	    }
	    break;
	    
	case QgsGrassEdit::MOVE_VERTEX:
	    // Move previously selected vertex 
	    if ( mSelectedLine > 0 ) {
		eraseDynamic();
		eraseElement ( mSelectedLine );

		// Move vertex
		int type = mProvider->readLine ( mPoints, mCats, mSelectedLine );
	        snap ( point ); 
		mPoints->x[mSelectedPart] = point.x();
		mPoints->y[mSelectedPart] = point.y();

                Vect_line_prune ( mPoints );
		mProvider->rewriteLine ( mSelectedLine, type, mPoints, mCats );
		updateSymb();
		displayUpdated();

		mSelectedLine = 0;
		Vect_reset_line ( mEditPoints );
	    } else {
		// Select new line
		mSelectedLine = mProvider->findLine ( point.x(), point.y(), GV_LINES, thresh );

		if ( mSelectedLine ) { // highlite
		    mProvider->readLine ( mEditPoints, NULL, mSelectedLine );
		    displayElement ( mSelectedLine, mSymb[SYMB_HIGHLIGHT], mSize );

		    double xl, yl; // nearest point on the line

		    // Note first segment is 1!
		    mSelectedPart = Vect_line_distance ( mEditPoints, point.x(), point.y(), 0.0, 0, 
 		                                         &xl, &yl, NULL, NULL, NULL, NULL );
		    
		    double dist1 = Vect_points_distance ( xl, yl, 0.0, mEditPoints->x[mSelectedPart-1], 
			                                  mEditPoints->y[mSelectedPart-1], 0.0, 0);
		    double dist2 = Vect_points_distance ( xl, yl, 0.0, mEditPoints->x[mSelectedPart], 
			                                  mEditPoints->y[mSelectedPart], 0.0, 0);
			
		    if ( dist1 < dist2 ) mSelectedPart--;
		}
	    }
	    break;
	    
	case QgsGrassEdit::ADD_VERTEX:
	    // Add vertex to previously selected line
	    if ( mSelectedLine > 0 ) {
		eraseDynamic();
		eraseElement ( mSelectedLine );

		// Move vertex
		int type = mProvider->readLine ( mPoints, mCats, mSelectedLine );

		if ( mAddVertexEnd && mSelectedPart == mEditPoints->n_points-1 ) {
	            snap ( point ); 
		    Vect_append_point ( mPoints, point.x(), point.y(), 0.0 );
		} else {
		    Vect_line_insert_point ( mPoints, mSelectedPart, point.x(), point.y(), 0.0 );
		}

                Vect_line_prune ( mPoints );
		mProvider->rewriteLine ( mSelectedLine, type, mPoints, mCats );
		updateSymb();
		displayUpdated();

		mSelectedLine = 0;
		Vect_reset_line ( mEditPoints );
	    } else {
		// Select new line
		mSelectedLine = mProvider->findLine ( point.x(), point.y(), GV_LINES, thresh );

		if ( mSelectedLine ) { // highlite
		    mProvider->readLine ( mEditPoints, NULL, mSelectedLine );
		    displayElement ( mSelectedLine, mSymb[SYMB_HIGHLIGHT], mSize );

		    double xl, yl; // nearest point on the line

		    // Note first segment is 1!
		    mSelectedPart = Vect_line_distance ( mEditPoints, point.x(), point.y(), 0.0, 0, 
 		                                         &xl, &yl, NULL, NULL, NULL, NULL );
		    
		    double dist1 = Vect_points_distance ( xl, yl, 0.0, mEditPoints->x[mSelectedPart-1], 
			                                  mEditPoints->y[mSelectedPart-1], 0.0, 0);
		    double dist2 = Vect_points_distance ( xl, yl, 0.0, mEditPoints->x[mSelectedPart], 
			                                  mEditPoints->y[mSelectedPart], 0.0, 0);
			
		    double maxdist = (dist1 + dist2)/4;
		    
		    if ( mSelectedPart == 1 && dist1 < maxdist ) {
			mSelectedPart = 0;
			mAddVertexEnd = true;
		    } else if ( mSelectedPart == mEditPoints->n_points-1 && dist2 < maxdist ) {
			mAddVertexEnd = true;
		    } else {
			mAddVertexEnd = false;
		    }
		}
	    }
	    break;
	    
	    
	case QgsGrassEdit::DELETE_VERTEX:
	    // Delete previously selected vertex 
	    if ( mSelectedLine > 0 ) {
		eraseDynamic();
		eraseElement ( mSelectedLine );

		// Move vertex
		int type = mProvider->readLine ( mPoints, mCats, mSelectedLine );
		Vect_line_delete_point ( mPoints, mSelectedPart );

		mProvider->rewriteLine ( mSelectedLine, type, mPoints, mCats );
		updateSymb();
		displayUpdated();

		mSelectedLine = 0;
		Vect_reset_line ( mEditPoints );
	    } else {
		// Select new/next line
		mSelectedLine = mProvider->findLine ( point.x(), point.y(), GV_LINES, thresh );

		if ( mSelectedLine ) { // highlite
		    mProvider->readLine ( mEditPoints, NULL, mSelectedLine );
	    
		    displayElement ( mSelectedLine, mSymb[SYMB_HIGHLIGHT], mSize );

		    double xl, yl; // nearest point on the line

		    // Note first segment is 1!
		    mSelectedPart = Vect_line_distance ( mEditPoints, point.x(), point.y(), 0.0, 0, 
 		                                         &xl, &yl, NULL, NULL, NULL, NULL );
		    
		    double dist1 = Vect_points_distance ( xl, yl, 0.0, mEditPoints->x[mSelectedPart-1], 
			                                  mEditPoints->y[mSelectedPart-1], 0.0, 0);
		    double dist2 = Vect_points_distance ( xl, yl, 0.0, mEditPoints->x[mSelectedPart], 
			                                  mEditPoints->y[mSelectedPart], 0.0, 0);
			
		    if ( dist1 < dist2 ) mSelectedPart--;
	
		    displayDynamic ( mEditPoints->x[mSelectedPart], mEditPoints->y[mSelectedPart], 
			             QgsGrassEdit::ICON_BOX, mSize );
		}

	    }
	    break;

	case QgsGrassEdit::SPLIT_LINE:
	    // Split previously selected line 
	    if ( mSelectedLine > 0 ) {
		eraseDynamic();
		eraseElement ( mSelectedLine );

		int type = mProvider->readLine ( mPoints, mCats, mSelectedLine );

		double xl, yl;
		Vect_line_distance ( mPoints, mLastPoint.x(), mLastPoint.y(), 0.0, 0, 
 		                     &xl, &yl, NULL, NULL, NULL, NULL );
		
		mPoints->n_points = mSelectedPart;
		Vect_append_point ( mPoints, xl, yl, 0.0 );
		mProvider->rewriteLine ( mSelectedLine, type, mPoints, mCats );
		updateSymb();
		displayUpdated();

		Vect_reset_line ( mPoints );
		Vect_append_point ( mPoints, xl, yl, 0.0 );
		for ( int i = mSelectedPart; i < mEditPoints->n_points; i++ ) {
		    Vect_append_point ( mPoints, mEditPoints->x[i], mEditPoints->y[i], 0.0 );
		}

		mProvider->writeLine ( type, mPoints, mCats );
		
		updateSymb();
		displayUpdated();

		mSelectedLine = 0;
		Vect_reset_line ( mEditPoints );
	    } else {
		// Select new/next line
		mSelectedLine = mProvider->findLine ( point.x(), point.y(), GV_LINES, thresh );

		if ( mSelectedLine ) { // highlite
		    mProvider->readLine ( mEditPoints, NULL, mSelectedLine );
	    
		    displayElement ( mSelectedLine, mSymb[SYMB_HIGHLIGHT], mSize );

		    double xl, yl; // nearest point on the line

		    // Note first segment is 1!
		    mSelectedPart = Vect_line_distance ( mEditPoints, point.x(), point.y(), 0.0, 0, 
 		                                         &xl, &yl, NULL, NULL, NULL, NULL );
		    
		    displayDynamic ( xl, yl, QgsGrassEdit::ICON_X, mSize );
		}

	    }
	    break;
	    
	case QgsGrassEdit::EDIT_ATTRIBUTES:
	    // Redraw previously selected line 
	    if ( mSelectedLine > 0 ) {
		displayElement ( mSelectedLine, mSymb[mLineSymb[mSelectedLine]], mSize );
	    }

	    // Select new/next line
            mSelectedLine = mProvider->findLine ( point.x(), point.y(), GV_POINT|GV_CENTROID, thresh );

            if ( mSelectedLine == 0 ) 
                mSelectedLine = mProvider->findLine ( point.x(), point.y(), GV_LINE|GV_BOUNDARY, thresh );
	    
	    #ifdef QGISDEBUG
	    std::cerr << "mSelectedLine = " << mSelectedLine << std::endl;
	    #endif

	    if ( mAttributes ) {
		delete mAttributes;
		mAttributes = 0;
	    }

	    if ( mSelectedLine ) { // highlite
		displayElement ( mSelectedLine, mSymb[SYMB_HIGHLIGHT], mSize );
		
		mProvider->readLine ( NULL, mCats, mSelectedLine );

		mAttributes = new QgsGrassAttributes ( this, mProvider, mSelectedLine, mQgisApp );
		for ( int i = 0; i < mCats->n_cats; i++ ) {
		    addAttributes ( mCats->field[i], mCats->cat[i] );
		}
		mAttributes->show();
	    }
	    
	    break;

	case QgsGrassEdit::EDIT_CATS:
	    std::cerr << "Tool not yet implemented." << std::endl;
	    break;

	case QgsGrassEdit::NONE:
	    break;
	    // nothing to do
	    
	default:
	    std::cerr << "Unknown tool" << std::endl;
	    break;
    }

    // Set last click
    mLastPoint = point;
    
    #ifdef QGISDEBUG
    std::cerr << "n_points = " << mEditPoints->n_points << std::endl;
    #endif

}

void QgsGrassEdit::addAttributes ( int field, int cat )
{
    QString *key = mProvider->key ( field );

    QString lab;
    lab.sprintf ( "%d:%d", field, cat );
    int tab = mAttributes->addTab( lab );
    mAttributes->setField ( tab, field );

    QString catLabel;
    if ( key->isEmpty() ) {
	catLabel = "Category";
    } else {
	catLabel = *key;
    }
    mAttributes->setCat ( tab, catLabel, cat );

    if ( !key->isEmpty() ) { // Database link defined 
	std::vector<QgsField> *cols = mProvider->columns ( field );

	if ( cols->size() == 0 ) {
	    QString str;
	    str.setNum( field );
	    QMessageBox::warning( 0, "Warning", "Cannot describe table for field " + str );
	} else {
	    std::vector<QgsFeatureAttribute> *atts = 
			 mProvider->attributes ( field, cat );

	    if ( atts->size() == 0 ) { // cannot select attributes
		 mAttributes->addTextRow ( tab, "WARNING: ATTRIBUTES MISSING" );
	    } else {
		int size;
		if ( atts->size() < cols->size() )
		    size = atts->size();
		else
		    size = cols->size();
		    
		for ( int j = 0; j < cols->size(); j++ ) {
		    QgsField col = (*cols)[j];
		    QgsFeatureAttribute att = (*atts)[j];
		    std::cerr << " name = " << col.name() <<  std::endl;

		    if ( col.name() != *key ) {
			std::cerr << " value = " << att.fieldValue() <<  std::endl;
			mAttributes->addAttribute ( tab, col.name(), att.fieldValue(), col.type() );
		    }
		}
	    }
	    delete atts;
	}
	delete cols;
    }
}

void QgsGrassEdit::addCat ( int line )
{
    int mode = mCatModeBox->currentItem();
    int field = mFieldBox->currentText().toInt();
    int cat = mCatEntry->text().toInt();

    int type = mProvider->readLine ( mPoints, mCats, line );
    if ( mode == CAT_MODE_NEXT || mode == CAT_MODE_MANUAL ) {
	Vect_cat_set ( mCats, field, cat );
    }

    line = mProvider->rewriteLine ( line, type, mPoints, mCats );
    if ( mAttributes ) mAttributes->setLine ( line );
    updateSymb();
    increaseMaxCat();

    // Insert new DB record if link is defined and the record for this cat does not exist
    QString *key = mProvider->key ( field );
    
    if ( !key->isEmpty() ) { // Database link defined 
	std::vector<QgsFeatureAttribute> *atts = mProvider->attributes ( field, cat );

	if ( atts->size() == 0 ) { // Nothing selected
            QString *error = mProvider->insertAttributes ( field, cat );

	    if ( !error->isEmpty() ) {
		 QMessageBox::warning( 0, "Warning", *error );
	    }
	    delete error;
	}

	delete atts;
    }

    addAttributes( field, cat );
}

void QgsGrassEdit::deleteCat ( int line, int field, int cat )
{
    #ifdef QGISDEBUG
    std::cerr << "QgsGrassEdit::deleteCat" << std::endl;
    #endif

    int type = mProvider->readLine ( mPoints, mCats, line );
    Vect_field_cat_del ( mCats, field, cat );

    line = mProvider->rewriteLine ( line, type, mPoints, mCats );
    if ( mAttributes ) mAttributes->setLine ( line );
    updateSymb();
}

void QgsGrassEdit::mouseEventReceiverMove ( QgsPoint & newPoint )
{
    #if QGISDEBUG > 3
    std::cerr << "QgsGrassEdit::mouseEventReceiverMove() mTool = " << mTool << std::endl;
    #endif
    
    switch ( mTool ) {
	case QgsGrassEdit::NEW_LINE:
	case QgsGrassEdit::NEW_BOUNDARY:
	    if ( mEditPoints->n_points > 0 ) {
		/* Draw the line with new segment */
		Vect_reset_line ( mPoints );
		Vect_append_points ( mPoints, mEditPoints, GV_FORWARD );
		Vect_append_point ( mPoints, static_cast<int>(newPoint.x()), 
                                             static_cast<int>(newPoint.y()), 0.0 );
                displayDynamic ( mPoints );
	    }
	    break;

	case QgsGrassEdit::MOVE_LINE:
	    // Move previously selected line 
	    if ( mSelectedLine > 0 ) {
		// Transform coordinates
		Vect_reset_line ( mPoints );
		Vect_append_points ( mPoints, mEditPoints, GV_FORWARD );

		for ( int i = 0; i < mPoints->n_points; i++ ) {
		   mPoints->x[i] += newPoint.x() - mLastPoint.x(); 
		   mPoints->y[i] += newPoint.y() - mLastPoint.y(); 
		}
		
                displayDynamic ( mPoints );
	    }
	    break;

	case QgsGrassEdit::MOVE_VERTEX:
	    if ( mSelectedLine > 0 ) {
		// Transform coordinates
		Vect_reset_line ( mPoints );
		std::cerr << "mSelectedPart = " << mSelectedPart << std::endl;
		if ( mSelectedPart == 0 ) {
		    Vect_append_point ( mPoints, mEditPoints->x[1], mEditPoints->y[1], 0.0 );
		    Vect_append_point ( mPoints, newPoint.x(), newPoint.y(), 0.0 );
		} else if ( mSelectedPart == mEditPoints->n_points-1 ) {
		    Vect_append_point ( mPoints, mEditPoints->x[mSelectedPart-1], 
			                mEditPoints->y[mSelectedPart-1], 0.0 );
		    Vect_append_point ( mPoints, newPoint.x(), newPoint.y(), 0.0 );
		} else {
		    Vect_append_point ( mPoints, mEditPoints->x[mSelectedPart-1], 
			                mEditPoints->y[mSelectedPart-1], 0.0 );
		    Vect_append_point ( mPoints, newPoint.x(), newPoint.y(), 0.0 );
		    Vect_append_point ( mPoints, mEditPoints->x[mSelectedPart+1], 
			                mEditPoints->y[mSelectedPart+1], 0.0 );
		}
		std::cerr << "npoints = " << mPoints->n_points << std::endl;
		for (int i = 0; i < mPoints->n_points; i++ ) {
		    std::cerr << mPoints->x[i] << " " << mPoints->y[i] << std::endl;
		}
		
                displayDynamic ( mPoints );
	    }
	    break;

	case QgsGrassEdit::ADD_VERTEX:
	    if ( mSelectedLine > 0 ) {
		Vect_reset_line ( mPoints );
		if ( mAddVertexEnd ) {
		    Vect_append_point ( mPoints, mEditPoints->x[mSelectedPart], 
			                         mEditPoints->y[mSelectedPart], 0.0 );
		    Vect_append_point ( mPoints, newPoint.x(), newPoint.y(), 0.0 );
		} else {
		    Vect_append_point ( mPoints, mEditPoints->x[mSelectedPart-1], 
			                mEditPoints->y[mSelectedPart-1], 0.0 );
		    Vect_append_point ( mPoints, newPoint.x(), newPoint.y(), 0.0 );
		    Vect_append_point ( mPoints, mEditPoints->x[mSelectedPart], 
			                mEditPoints->y[mSelectedPart], 0.0 );
		}
		std::cerr << "npoints = " << mPoints->n_points << std::endl;
		for (int i = 0; i < mPoints->n_points; i++ ) {
		    std::cerr << mPoints->x[i] << " " << mPoints->y[i] << std::endl;
		}
		
                displayDynamic ( mPoints );
	    }
	    break;

	case QgsGrassEdit::NONE:
	case QgsGrassEdit::NEW_POINT:
	case QgsGrassEdit::NEW_CENTROID:
	case QgsGrassEdit::DELETE_VERTEX:
	case QgsGrassEdit::SPLIT_LINE:
	case QgsGrassEdit::DELETE_LINE:
	case QgsGrassEdit::EDIT_CATS:
	case QgsGrassEdit::EDIT_ATTRIBUTES:
	    // Nothing to do
	    break;
	default:
	    std::cerr << "Unknown tool" << std::endl;
	    break;
    }
    mCanvas->repaint(false);
}

void QgsGrassEdit::postRender(QPainter *painter)
{
    #ifdef QGISDEBUG
    std::cerr << "QgsGrassEdit::postRender" << std::endl;
    #endif

    displayMap();

    // Redisplay highlighted
    if ( mSelectedLine ) {
	displayElement ( mSelectedLine, mSymb[SYMB_HIGHLIGHT], mSize );
    }
    // Redisplay current dynamics
    displayLastDynamic ( );
}

void QgsGrassEdit::displayMap (void)
{
    #ifdef QGISDEBUG
    std::cerr << "QgsGrassEdit::displayMap" << std::endl;
    #endif

    mTransform = mCanvas->getCoordinateTransform();


    QPainter *painter = new QPainter();
    painter->begin(mPixmap);

    // Display lines
    int nlines = mProvider->numLines(); 

    QPen pen;

    // TODO?: 2 loops, first lines, then points
    for ( int line = 1; line <= nlines; line++ ) {
	displayElement ( line, mSymb[mLineSymb[line]], mSize, painter );
    }

    // Display nodes
    int nnodes = mProvider->numNodes(); 

    pen.setColor(QColor(255,0,0));

    if ( mSymbDisplay[SYMB_NODE_1] || mSymbDisplay[SYMB_NODE_2] ) {
	for ( int node = 1; node <= nnodes; node++ ) {
	    if ( mNodeSymb[node] == SYMB_NODE_0 ) continue; // do not display nodes with points only
	    displayNode ( node, mSymb[mNodeSymb[node]], mSize, painter ); 
	}
    }
    
    painter->end();
    mCanvas->repaint(false);
}

void QgsGrassEdit::displayUpdated (void)
{
    #ifdef QGISDEBUG
    std::cerr << "QgsGrassEdit::displayUpdated" << std::endl;
    #endif

    mTransform = mCanvas->getCoordinateTransform();

    QPainter *painter = new QPainter();
    painter->begin(mPixmap);

    // Display lines
    int nlines = mProvider->numUpdatedLines();

    for ( int i = 0; i < nlines; i++ ) {
	int line = mProvider->updatedLine(i);
	if ( !(mProvider->lineAlive(line)) ) continue;
	
	displayElement ( line, mSymb[mLineSymb[line]], mSize, painter );
    }

    // Display nodes
    int nnodes = mProvider->numUpdatedNodes(); 
    for ( int i = 0; i < nnodes; i++ ) {
	int node = mProvider->updatedNode(i);
	if ( !(mProvider->nodeAlive(node)) ) continue;
	if ( mNodeSymb[node] == SYMB_NODE_0 ) continue; // do not display nodes with points only
	displayNode ( node, mSymb[mNodeSymb[node]], mSize, painter ); 
    }
    
    painter->end();
    mCanvas->repaint(false);
}

void QgsGrassEdit::displayElement ( int line, const QPen & pen, int size, QPainter *painter)
{
    #if QGISDEBUG > 3
    std::cerr << "QgsGrassEdit::displayElement() line = " << line << std::endl;
    #endif
	
    if ( !mSymbDisplay[mLineSymb[line]] ) return;
    
    int type = mProvider->readLine ( mPoints, NULL, line );
    if ( type < 0 ) return;

    QPainter *myPainter;
    if ( !painter ) {
	myPainter = new QPainter();
	myPainter->begin(mPixmap);
    } else {
	myPainter = painter;
    }

    if ( type & GV_POINTS ) {
	displayIcon ( mPoints->x[0], mPoints->y[0], pen, QgsGrassEdit::ICON_CROSS, size, painter );
    } else { // line
	QgsPoint point;
	QPointArray pointArray(mPoints->n_points);
	
	for ( int i = 0; i < mPoints->n_points; i++ ) {
	    point.setX(mPoints->x[i]);
	    point.setY(mPoints->y[i]);
	    mTransform->transform(&point);
	    pointArray.setPoint( i, static_cast<int>(point.x()), 
                                    static_cast<int>(point.y()) ); 
	}

	myPainter->setPen ( pen );
	myPainter->drawPolyline ( pointArray );
    }

    if ( !painter ) {
	myPainter->end();
	mCanvas->repaint(false);
	delete myPainter;
    }
}

void QgsGrassEdit::eraseElement ( int line )
{
    #ifdef QGISDEBUG
    std::cerr << "QgsGrassEdit::eraseElement() line = " << line << std::endl;
    #endif
    
    int type = mProvider->readLine ( NULL, NULL, line );
    if ( type < 0 ) return;

    // Erase line
    displayElement ( line, mSymb[SYMB_BACKGROUND], mSize );

    // Erase nodes
    if ( type & GV_LINES ) {
	int node1, node2;
	mProvider->lineNodes( line, &node1, &node2 );

	double x, y;
	mProvider->nodeCoor( node1, &x, &y );
	displayIcon ( x, y, mSymb[SYMB_BACKGROUND], QgsGrassEdit::ICON_X, mSize );

	mProvider->nodeCoor( node2, &x, &y );
	displayIcon ( x, y, mSymb[SYMB_BACKGROUND], QgsGrassEdit::ICON_X, mSize );
    }
}

void QgsGrassEdit::eraseDynamic ( void )
{
    displayDynamic ( 0, 0.0, 0.0, ICON_NONE, 0 );
}

void QgsGrassEdit::displayDynamic ( struct line_pnts *Points )
{
    displayDynamic ( Points, 0.0, 0.0, ICON_NONE, 0 );
}

void QgsGrassEdit::displayDynamic ( double x, double y, int type, int size )
{
    #if QGISDEBUG > 3
    std::cerr << "QgsGrassEdit::displayDynamic icon" << std::endl;
    #endif

    displayDynamic ( 0, x, y, type, size );
}
    
void QgsGrassEdit::displayDynamic ( struct line_pnts *Points, double x, double y, int type, int size )
{
    #if QGISDEBUG > 3
    std::cerr << "QgsGrassEdit::displayDynamic Points = " << Points << " type = " << type  << std::endl;
    #endif

    mTransform = mCanvas->getCoordinateTransform();
    
    // Delete last drawing
    displayLastDynamic ( );

    if ( Points ) {
	mLastDynamicPoints.resize (Points->n_points);

	for ( int i = 0; i < Points->n_points; i++ ) {
	    QgsPoint point;
	    point.setX(Points->x[i]);
	    point.setY(Points->y[i]);
	    mTransform->transform(&point);
	    mLastDynamicPoints.setPoint( i, static_cast<int>(point.x()), 
                                            static_cast<int>(point.y()) ); 
	}
    } else {
	mLastDynamicPoints.resize(0);
    }

    if ( type != ICON_NONE ) {
	mLastDynamicIconX = x;
	mLastDynamicIconY = y;
    }
    mLastDynamicIcon = type;

    displayLastDynamic ( );
}

void QgsGrassEdit::displayLastDynamic ( void ) 
{
    #if QGISDEBUG > 3
    std::cerr << "QgsGrassEdit::displayLastDynamic" << std::endl;
    #endif

    QPainter myPainter;
    myPainter.begin(mPixmap);

    // Use of XOR can result in repeated :
    //  'QPainter: Internal error; no available GC '
    // which is probably only false warning, see qt/src/kernel/qpainter_x11.cpp
    myPainter.setRasterOp(Qt::XorROP);  // Must be after begin()
    myPainter.setPen ( mSymb[SYMB_DYNAMIC] );
		
    myPainter.drawPolyline ( mLastDynamicPoints );

    if ( mLastDynamicIcon != ICON_NONE ) {
        displayIcon ( mLastDynamicIconX, mLastDynamicIconY, mSymb[SYMB_DYNAMIC], mLastDynamicIcon, 
		      mSize, &myPainter );
    }

    myPainter.end();
}

void QgsGrassEdit::displayNode ( int node, const QPen & pen, int size, QPainter *painter )
{
    #if QGISDEBUG > 3
    std::cerr << "QgsGrassEdit::displayNode() node = " << node << std::endl;
    #endif

    if ( !mSymbDisplay[mNodeSymb[node]] ) return;

    double x, y;

    if ( !(mProvider->nodeCoor(node,&x,&y )) ) return;

    displayIcon ( x, y, pen, QgsGrassEdit::ICON_X, size, painter );
}

void QgsGrassEdit::displayIcon ( double x, double y, const QPen & pen, 
	                         int type, int size, QPainter *painter )
{
    #if QGISDEBUG > 3
    std::cerr << "QgsGrassEdit::displayIcon()" << std::endl;
    #endif
		
    QgsPoint point;
    QPointArray pointArray(2);

    point.setX(x);
    point.setY(y);
    mTransform->transform(&point);

    int px = static_cast<int>(point.x());
    int py = static_cast<int>(point.y());
    int m = (size-1)/2;

    QPainter *myPainter;
    if ( !painter ) {
	myPainter = new QPainter();
        myPainter->begin(mPixmap);
    } else {
	myPainter = painter;
    }

    myPainter->setPen ( pen );

    switch ( type ) {
	case QgsGrassEdit::ICON_CROSS :
	    pointArray.setPoint( 0, px-m, py ); 
	    pointArray.setPoint( 1, px+m, py ); 
	    myPainter->drawPolyline ( pointArray );

	    pointArray.setPoint( 0, px, py+m ); 
	    pointArray.setPoint( 1, px, py-m ); 
	    myPainter->drawPolyline ( pointArray );
	    break;
	case QgsGrassEdit::ICON_X :
	    pointArray.setPoint( 0, px-m, py+m ); 
	    pointArray.setPoint( 1, px+m, py-m ); 
	    myPainter->drawPolyline ( pointArray );

	    pointArray.setPoint( 0, px-m, py-m ); 
	    pointArray.setPoint( 1, px+m, py+m ); 
	    myPainter->drawPolyline ( pointArray );
	    break;
	case QgsGrassEdit::ICON_BOX :
	    pointArray.resize(5);
	    pointArray.setPoint( 0, px-m, py-m ); 
	    pointArray.setPoint( 1, px+m, py-m ); 
	    pointArray.setPoint( 2, px+m, py+m ); 
	    pointArray.setPoint( 3, px-m, py+m ); 
	    pointArray.setPoint( 4, px-m, py-m ); 
	    myPainter->drawPolyline ( pointArray );
	    break;
    }

    if ( !painter ) {
	myPainter->end();
	mCanvas->repaint(false);
	delete myPainter;
    }
}

