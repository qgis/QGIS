/***************************************************************************
                         qgscomposerlabel.cpp
                             -------------------
    begin                : January 2005
    copyright            : (C) 2005 by Radim Blazek
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
#include <math.h>
#include <iostream>
#include <typeinfo>
#include <map>

#include <qwidget.h>
#include <qrect.h>
#include <qcombobox.h>
#include <qdom.h>
#include <qcanvas.h>
#include <qpainter.h>
#include <qstring.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qlineedit.h>
#include <qpointarray.h>
#include <qfont.h>
#include <qfontmetrics.h>
#include <qfontdialog.h>
#include <qpen.h>
#include <qrect.h>
#include <qcheckbox.h>

#include "qgsrect.h"
#include "qgsmaptopixel.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayer.h"
#include "qgsvectorlayer.h"
#include "qgsdlgvectorlayerproperties.h"
#include "qgscomposition.h"
#include "qgscomposermap.h"
#include "qgscomposerlabel.h"

QgsComposerLabel::QgsComposerLabel ( QgsComposition *composition, int id, 
	                                            int x, int y, QString text, int fontSize )
    : QCanvasPolygonalItem(0), mBox(false)
{
    std::cout << "QgsComposerLabel::QgsComposerLabel()" << std::endl;

    mComposition = composition;
    mId  = id;

    //mText = text;
    mText = "Quantum GIS";

    // Font and pen 
    mFont.setPointSize ( fontSize );
    mPen.setWidth (1);

    QCanvasPolygonalItem::setX(x);
    QCanvasPolygonalItem::setY(y);

    mSelected = false;

    setOptions();

    // Add to canvas
    setCanvas(mComposition->canvas());
    QCanvasPolygonalItem::setZ(100);
    setActive(true);
    QCanvasPolygonalItem::show();
    QCanvasPolygonalItem::update(); // ?

    writeSettings();
}

QgsComposerLabel::QgsComposerLabel ( QgsComposition *composition, int id ) 
    : QCanvasPolygonalItem(0)
{
    std::cout << "QgsComposerLabel::QgsComposerLabel()" << std::endl;

    mComposition = composition;
    mId  = id;
    mSelected = false;

    readSettings();
    
    setOptions();

    // Add to canvas
    setCanvas(mComposition->canvas());
    QCanvasPolygonalItem::setZ(100);
    setActive(true);
    QCanvasPolygonalItem::show();
    QCanvasPolygonalItem::update(); // ?

}

QgsComposerLabel::~QgsComposerLabel()
{
    std::cout << "QgsComposerLabel::~QgsComposerLabel" << std::endl;
    QCanvasItem::hide();
}

void QgsComposerLabel::drawShape ( QPainter & painter )
{
    std::cout << "QgsComposerLabel::drawShape" << std::endl;
    draw ( painter );
}

void QgsComposerLabel::draw ( QPainter & painter )
{
    std::cout << "QgsComposerLabel::render" << std::endl;

    float size =  25.4 * mComposition->scale() * mFont.pointSizeFloat() / 72;
    mBoxBuffer = (int) ( size / 10 * mComposition->scale() );

    QFont font ( mFont );
    font.setPointSizeFloat ( size );
    QFontMetrics metrics ( font );

    // Not sure about Style Strategy, QFont::PreferMatch ?
    //font.setStyleStrategy ( (QFont::StyleStrategy) (QFont::PreferOutline | QFont::PreferAntialias ) );

    painter.setPen ( mPen );
    painter.setFont ( font );
    
    int x = (int) QCanvasPolygonalItem::x();
    int y = (int) QCanvasPolygonalItem::y();
    
    int w = metrics.width ( mText );
    int h = metrics.height() ;

    QRect r ( (int)(x - w/2), (int) (y - h/2), w, h );
    
    QRect boxRect;
    if ( mBox ) {
	// I don't know why, but the box seems to be too short -> add 1 * mBoxBuffer to width
	boxRect.setRect ( (int)(r.x()-1.5*mBoxBuffer), r.y()-mBoxBuffer, (int)(r.width()+3*mBoxBuffer), r.height()+2*mBoxBuffer );
	QBrush brush ( QColor(255,255,255) );
	painter.setBrush ( brush );
	painter.drawRect ( boxRect );
    }
    
    // The width is not sufficient in postscript
    QRect tr = r;
    tr.setWidth ( r.width() );

    if ( plotStyle() == QgsComposition::Postscript ) {
	// TODO: For output to Postscript the font must be scaled. But how?  
	//       The factor is an empirical value.
	//       In any case, each font scales in in different way even if painter.scale()
	//       is used instead of font size!!! -> Postscript is never exactly the same as 
	//       in preview.
	double factor = 2.45;
	
	double pssize = factor * 72.0 * mFont.pointSizeFloat() / mComposition->resolution();
	double psscale = pssize/size;
	
	painter.save();
	//painter.translate(x-w/2,(int)(y+metrics.height()/2-metrics.descent()));
	painter.translate(x,y);

	painter.scale ( psscale, psscale );
    
	/// rect can be too small in PS -> add buf
	int buf = metrics.width ( "x" );
	QRect psr ( (int)( -1.*(w+2*buf)/2/psscale), (int) (-1.*h/2/psscale),(int)(1.*(w+2*buf)/psscale), (int)(1.*h/psscale) );

        //painter.drawText ( 0, 0, mText );	
	painter.drawText ( psr, Qt::AlignCenter|Qt::SingleLine , mText );
	
	painter.restore();
    } else {
	//painter.drawText ( tr, Qt::AlignCenter|Qt::SingleLine , mText );
	painter.drawText ( x-w/2,(int)(y+metrics.height()/2-metrics.descent()), mText );
    } 

    // Show selected / Highlight
    if ( mSelected && plotStyle() == QgsComposition::Preview ) {
	QRect hr;
	if ( mBox ) {
	    hr = boxRect;
	} else {
	    hr = r;
	}
        painter.setPen( mComposition->selectionPen() );
        painter.setBrush( mComposition->selectionBrush() );
	int s = mComposition->selectionBoxSize();
	
	painter.drawRect ( hr.x(), hr.y(), s, s );
	painter.drawRect ( hr.x()+hr.width()-s, hr.y(), s, s );
	painter.drawRect ( hr.x()+hr.width()-s, hr.y()+hr.height()-s, s, s );
	painter.drawRect ( hr.x(), hr.y()+hr.height()-s, s, s );
    }
}

void QgsComposerLabel::changeFont ( void ) 
{
    bool result;

    QRect r = boundingRect();

    mFont = QFontDialog::getFont(&result, mFont, this );

    if ( result ) {
	QCanvasPolygonalItem::invalidate();
    	QCanvasPolygonalItem::canvas()->setChanged(r);
	QCanvasPolygonalItem::update();
	QCanvasPolygonalItem::canvas()->update();
    }
    writeSettings();
}

void QgsComposerLabel::boxChanged ()
{
    QRect r = boundingRect();
    
    mBox = mBoxCheckBox->isChecked();

    QCanvasPolygonalItem::invalidate();
    QCanvasPolygonalItem::canvas()->setChanged(r);
    QCanvasPolygonalItem::update();
    QCanvasPolygonalItem::canvas()->update();

    writeSettings();
}

QRect QgsComposerLabel::boundingRect ( void ) const
{
    // Recalculate sizes according to current font size
    
    float size = 25.4 * mComposition->scale() * mFont.pointSize() / 72;
    
    QFont font ( mFont );
    font.setPointSizeFloat ( size );
    
    QFontMetrics metrics ( font );
    
    int x = (int) QCanvasPolygonalItem::x();
    int y = (int) QCanvasPolygonalItem::y();
    int w = metrics.width ( mText );
    int h = metrics.height() ;
    
    int buf = 0;
    int width;
    
    if ( mBox ) {
	buf = (int) ( size / 10 * mComposition->scale() + 2 ); // 2 is for line width
    }
    
    QRect r ( (int)(x - w/2 - 1.5*buf), (int) (y - h/2 - buf), (int)(w+3*buf), h+2*buf );

    return r;
}

QPointArray QgsComposerLabel::areaPoints() const
{
    std::cout << "QgsComposerLabel::areaPoints" << std::endl;
    QRect r = boundingRect();

    QPointArray pa(4);
    pa[0] = QPoint( r.x(), r.y() );
    pa[1] = QPoint( r.x()+r.width(), r.y() );
    pa[2] = QPoint( r.x()+r.width(), r.y()+r.height() );
    pa[3] = QPoint( r.x(), r.y()+r.height() );

    return pa ;
}

void QgsComposerLabel::setOptions ( void )
{ 
    mTextLineEdit->setText ( mText );
    mBoxCheckBox->setChecked ( mBox );
    
}

void QgsComposerLabel::textChanged ( void )
{ 
    QRect r = boundingRect();
    mText = mTextLineEdit->text();
    QCanvasPolygonalItem::invalidate();
    QCanvasPolygonalItem::canvas()->setChanged(r);
    QCanvasPolygonalItem::update();
    QCanvasPolygonalItem::canvas()->update();
    writeSettings();
}

void QgsComposerLabel::setSelected (  bool s ) 
{
    std::cout << "QgsComposerLabel::setSelected" << std::endl;
    mSelected = s;
    QCanvasPolygonalItem::update(); // show highlight
            
    std::cout << "mSelected = " << mSelected << std::endl;
}    

bool QgsComposerLabel::selected( void )
{
    return mSelected;
}

QWidget *QgsComposerLabel::options ( void )
{
    setOptions ();
    return ( dynamic_cast <QWidget *> (this) );
}

bool QgsComposerLabel::writeSettings ( void )  
{
    QString path;
    path.sprintf("/composition_%d/label_%d/", mComposition->id(), mId ); 
    
    QgsProject::instance()->writeEntry( "Compositions", path+"text", mText );

    QgsProject::instance()->writeEntry( "Compositions", path+"x", mComposition->toMM((int)QCanvasPolygonalItem::x()) );
    QgsProject::instance()->writeEntry( "Compositions", path+"y", mComposition->toMM((int)QCanvasPolygonalItem::y()) );

    QgsProject::instance()->writeEntry( "Compositions", path+"font/size", mFont.pointSize() );
    QgsProject::instance()->writeEntry( "Compositions", path+"font/family", mFont.family() );
    QgsProject::instance()->writeEntry( "Compositions", path+"font/weight", mFont.weight() );
    QgsProject::instance()->writeEntry( "Compositions", path+"font/underline", mFont.underline() );
    QgsProject::instance()->writeEntry( "Compositions", path+"font/strikeout", mFont.strikeOut() );

    QgsProject::instance()->writeEntry( "Compositions", path+"box", mBox );
    
    return true; 
}

bool QgsComposerLabel::readSettings ( void )
{
    std::cout << "QgsComposerLabel::readSettings mId = " << mId << std::endl;
    bool ok;

    QString path;
    path.sprintf("/composition_%d/label_%d/", mComposition->id(), mId );

    mText = QgsProject::instance()->readEntry("Compositions", path+"text", "???", &ok);

    int x = mComposition->fromMM( QgsProject::instance()->readDoubleEntry( "Compositions", path+"x", 0, &ok) );
    QCanvasPolygonalItem::setX( x );
    int y = mComposition->fromMM(QgsProject::instance()->readDoubleEntry( "Compositions", path+"y", 0, &ok) );
    QCanvasPolygonalItem::setY( y );

    mFont.setFamily ( QgsProject::instance()->readEntry("Compositions", path+"font/family", "", &ok) );
    mFont.setPointSize ( QgsProject::instance()->readNumEntry("Compositions", path+"font/size", 10, &ok) );
    mFont.setWeight(  QgsProject::instance()->readNumEntry("Compositions", path+"font/weight", (int)QFont::Normal, &ok) );
    mFont.setUnderline(  QgsProject::instance()->readBoolEntry("Compositions", path+"font/underline", false, &ok) );
    mFont.setStrikeOut(  QgsProject::instance()->readBoolEntry("Compositions", path+"font/strikeout", false, &ok) );

    mBox = QgsProject::instance()->readBoolEntry("Compositions", path+"box", false, &ok);

    QCanvasPolygonalItem::update();

    return true;
}

bool QgsComposerLabel::removeSettings ( void )
{
    QString path;
    path.sprintf("/composition_%d/label_%d", mComposition->id(), mId );
    return QgsProject::instance()->removeEntry ( "Compositions", path );
}

bool QgsComposerLabel::writeXML( QDomNode & node, QDomDocument & document, bool temp )
{
    return true;
}

bool QgsComposerLabel::readXML( QDomNode & node )
{
    return true;
}
