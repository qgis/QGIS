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
    : QCanvasPolygonalItem(0)
{
    std::cout << "QgsComposerLabel::QgsComposerLabel()" << std::endl;

    mComposition = composition;
    mId  = id;

    mText = text;

    // Font and pen 
    mFont.setPointSize ( fontSize );

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

QgsComposerLabel::~QgsComposerLabel()
{
}

void QgsComposerLabel::draw ( QPainter & painter )
{
    std::cout << "QgsComposerLabel::render" << std::endl;

    int size = (int) ( 25.4 * mComposition->scale() * mFont.pointSize() / 72);
    QFont font ( mFont );
    font.setPointSize ( size );

    QFontMetrics metrics ( font );

    // Fonts for rendering

    // It seems that font pointSize is used in points in Postscript, that means it depends 
    // on resolution!
    if ( plotStyle() == QgsComposition::Print ) {
	size = (int) ( 72.0 * size / mComposition->resolution() );
    }
    
    font.setPointSize ( size );

    // Not sure about Style Strategy, QFont::PreferMatch ?
    font.setStyleStrategy ( (QFont::StyleStrategy) (QFont::PreferOutline | QFont::PreferAntialias ) );

    painter.setPen ( mPen );
    painter.setFont ( font );
    
    int x = (int) QCanvasPolygonalItem::x();
    int y = (int) QCanvasPolygonalItem::y();
    int w = metrics.width ( mText );
    int h = metrics.height();

    //painter.drawText( (int)(x-w/2), (int)(y+h/2), mText );	
   
    QRect r = boundingRect();
    painter.drawText ( r, Qt::AlignCenter|Qt::SingleLine , mText );

    // Show selected / Highlight
    if ( mSelected && plotStyle() == QgsComposition::Preview ) {
        painter.setPen( mComposition->selectionPen() );
        painter.setBrush( mComposition->selectionBrush() );
	int s = mComposition->selectionBoxSize();
	QRect r = boundingRect();
	
	painter.drawRect ( r.x(), r.y(), s, s );
	painter.drawRect ( r.x()+r.width()-s, r.y(), s, s );
	painter.drawRect ( r.x()+r.width()-s, r.y()+r.height()-s, s, s );
	painter.drawRect ( r.x(), r.y()+r.height()-s, s, s );
    }
}

void QgsComposerLabel::changeFont ( void ) 
{
    bool result;

    mFont = QFontDialog::getFont(&result, mFont, this );

    if ( result ) {
	QCanvasPolygonalItem::update();
	QCanvasPolygonalItem::canvas()->update();
    }
    writeSettings();
}

QRect QgsComposerLabel::boundingRect ( void ) const
{
    // Recalculate sizes according to current font size
    
    int size = (int) ( 25.4 * mComposition->scale() * mFont.pointSize() / 72);
    QFont font ( mFont );
    font.setPointSize ( size );

    QFontMetrics metrics ( font );

    int x = (int) QCanvasPolygonalItem::x();
    int y = (int) QCanvasPolygonalItem::y();
    int w = metrics.width ( mText );
    int h = metrics.height();
    QRect r ( (int)(x - w/2), (int) (y - h/2), w, h );

    return r;
}

void QgsComposerLabel::drawShape(QPainter&)
{
    std::cout << "QgsComposerLabel::drawShape" << std::endl;
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
    
}

void QgsComposerLabel::textChanged ( void )
{ 
    mText = mTextLineEdit->text();
    QCanvasPolygonalItem::update();
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
    QgsProject::instance()->writeEntry( "Compositions", path+"x", (int)QCanvasPolygonalItem::x() );
    QgsProject::instance()->writeEntry( "Compositions", path+"y", (int)QCanvasPolygonalItem::y() );

    QgsProject::instance()->writeEntry( "Compositions", path+"font/size", mFont.pointSize() );
    QgsProject::instance()->writeEntry( "Compositions", path+"font/family", mFont.family() );
    
    return true; 
}

bool QgsComposerLabel::readSettings ( void )
{
    bool ok;
    QString path;
    path.sprintf("/composition_%d/label_%d/", mComposition->id(), mId );

    QCanvasPolygonalItem::setX( mComposition->fromMM(QgsProject::instance()->readDoubleEntry( "Compositions", path+"x", 0, &ok)) );
    QCanvasPolygonalItem::setY( mComposition->fromMM(QgsProject::instance()->readDoubleEntry( "Compositions", path+"y", 0, &ok)) );

    mFont.setFamily ( QgsProject::instance()->readEntry("Compositions", path+"font/family", "", &ok) );
    mFont.setPointSize ( QgsProject::instance()->readNumEntry("Compositions", path+"font/size", 10, &ok) );

    QCanvasPolygonalItem::update();
    
    return true;
}

bool QgsComposerLabel::writeXML( QDomNode & node, QDomDocument & document, bool temp )
{
    return true;
}

bool QgsComposerLabel::readXML( QDomNode & node )
{
    return true;
}
