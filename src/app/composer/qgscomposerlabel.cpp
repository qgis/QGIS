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
#include "qgscomposerlabel.h"
#include "qgsproject.h"

#include <QGraphicsScene>
#include <QAbstractGraphicsShapeItem>
#include <QPolygonF>
#include <QFontDialog>
#include <QPainter>

#include <iostream>

QgsComposerLabel::QgsComposerLabel ( QgsComposition *composition, int id, 
	                                            int x, int y, QString text, int fontSize )
    : QWidget(composition), QAbstractGraphicsShapeItem(0), mBox(false)
{
    setupUi(this);

#ifdef QGISDEBUG
    std::cout << "QgsComposerLabel::QgsComposerLabel()" << std::endl;
#endif

    mComposition = composition;
    mId  = id;

    //mText = text;
    mText = "Quantum GIS";

    // Font and pen 
    mFont.setPointSize ( fontSize );

    // Could make this user variable in the future
    mPen.setWidthF (0.5);

    QAbstractGraphicsShapeItem::setPos(x, y);

    mSelected = false;

    setOptions();

    // Add to canvas
    mComposition->canvas()->addItem(this);

    QAbstractGraphicsShapeItem::setZValue(100);
    QAbstractGraphicsShapeItem::show();
    QAbstractGraphicsShapeItem::update();

    writeSettings();
}

QgsComposerLabel::QgsComposerLabel ( QgsComposition *composition, int id ) 
    : QAbstractGraphicsShapeItem(0)
{
#ifdef QGISDEBUG
    std::cout << "QgsComposerLabel::QgsComposerLabel()" << std::endl;
#endif

    setupUi(this);

    mComposition = composition;
    mId  = id;
    mSelected = false;

    readSettings();
    
    setOptions();

    // Add to canvas
    mComposition->canvas()->addItem(this);
    QAbstractGraphicsShapeItem::setZValue(100);
    QAbstractGraphicsShapeItem::show();
    QAbstractGraphicsShapeItem::update();

}

QgsComposerLabel::~QgsComposerLabel()
{
#ifdef QGISDEBUG
    std::cout << "QgsComposerLabel::~QgsComposerLabel" << std::endl;
#endif
    //make ourselves disappear so there aren't any interesting effects when we get destroyed
    QGraphicsItem::hide();
}

#define FONT_WORKAROUND_SCALE 10
// This partially resolves a problem with the bounding rectangle for the text
// being too short for most font sizes. 
#define WIDTH_EXTENSION 1.0
void QgsComposerLabel::paint ( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget )
{
#ifdef QGISDEBUG
    std::cout << "QgsComposerLabel::paint" << std::endl;
#endif

    float size =  25.4 * mComposition->scale() * mFont.pointSizeF() / 72;

    //mBoxBuffer = size / 10 * mComposition->scale();
    mBoxBuffer = 0;

    QFont font ( mFont );
    font.setStyleStrategy ( QFont::StyleStrategy(QFont::PreferOutline | QFont::PreferAntialias) );
    font.setPointSizeF ( size );

    QFontMetricsF metrics ( font );
    QSizeF textSize = metrics.size(0, mText);
    qreal w = textSize.width() + WIDTH_EXTENSION;
    qreal h = textSize.height();

    QRectF r (0, -h, w, h); //used as the rectangle to draw the selection boxes on the corners of if there is no box
    
    QRectF boxRect;
    if ( mBox ) {
        boxRect.setRect(-mBoxBuffer, -(h+mBoxBuffer), w + (mBoxBuffer * 2)+1, h + (mBoxBuffer * 2));
        QBrush brush ( QColor(255,255,255) );
        painter->setBrush ( brush );
        painter->setPen(QPen(QColor(0, 0, 0), .2));
        painter->drawRect ( boxRect );
    }
    
    font.setPointSizeF ( size * FONT_WORKAROUND_SCALE );
    painter->setFont ( font );
    painter->setPen ( mPen );

    if ( plotStyle() == QgsComposition::Postscript ) 
    {
        // This metrics.ascent() is empirical
        size = metrics.ascent() * 72.0 / mComposition->resolution() * FONT_WORKAROUND_SCALE; 
        font.setPointSizeF ( size );
        painter->setFont ( font );
        //std::cout << "label using PS render size" << std::endl;
    }

    //Hack to work around the Qt font bug

    painter->save();
    painter->scale(1./FONT_WORKAROUND_SCALE, 1./FONT_WORKAROUND_SCALE);
    QRectF scaledBox(r.x() * FONT_WORKAROUND_SCALE,
                     r.y() * FONT_WORKAROUND_SCALE,
                     (r.width() + WIDTH_EXTENSION) * FONT_WORKAROUND_SCALE,
                     r.height() * FONT_WORKAROUND_SCALE);
    painter->drawText(scaledBox, Qt::AlignLeft|Qt::AlignVCenter, mText);
    painter->restore(); //undo our scaling of painter - End of the font workaround

    // Show selected / Highlight
    if ( mSelected && plotStyle() == QgsComposition::Preview ) {
        QRectF hr;
        if ( mBox ) {
            hr = boxRect;
        } else {
            hr = r;
        }
        hr.setWidth(hr.width() + WIDTH_EXTENSION);
        painter->setPen( mComposition->selectionPen() );
        painter->setBrush( mComposition->selectionBrush() );

        double s = mComposition->selectionBoxSize();
	
        painter->drawRect (QRectF(hr.x(), hr.y(), s, s ));
        painter->drawRect (QRectF(hr.x()+hr.width()-s, hr.y(), s, s ));
        painter->drawRect (QRectF(hr.x()+hr.width()-s, hr.y()+hr.height()-s, s, s ));
        painter->drawRect (QRectF(hr.x(), hr.y()+hr.height()-s, s, s ));
    }
}

void QgsComposerLabel::on_mFontButton_clicked() 
{
    bool result;

    QRectF r = boundingRect();

    mFont = QFontDialog::getFont(&result, mFont, this );

    if ( result ) {
	    QAbstractGraphicsShapeItem::prepareGeometryChange();
	    QAbstractGraphicsShapeItem::update();
    }
    writeSettings();
}

void QgsComposerLabel::on_mBoxCheckBox_clicked()
{
    QRectF r = boundingRect();
    
    mBox = mBoxCheckBox->isChecked();

    QAbstractGraphicsShapeItem::prepareGeometryChange();
    QAbstractGraphicsShapeItem::update();
    writeSettings();
}

QRectF QgsComposerLabel::boundingRect ( void ) const
{
    // Recalculate sizes according to current font size
    
    float size =  25.4 * mComposition->scale() * mFont.pointSizeF() / 72;

    QFont font ( mFont );
    font.setPointSizeF ( size );
    QFontMetricsF metrics ( font );

    QSizeF textSize = metrics.size(0, mText);
    double w = textSize.width() + WIDTH_EXTENSION;
    double h = textSize.height();

/*    
    int buf = 0;
    
    if ( mBox ) {
	buf = (int) ( size / 10 * mComposition->scale() + 2 ); // 2 is for line width
    }
    
    QRectF r ( (int)(x - w/2 - 1.5*buf), (int) (y - h/2 - buf), (int)(w+3*buf), h+2*buf );
*/

    QRectF r;

    if(mBox){
		//what happens if we haven't called paint() first?
        r.setRect(-mBoxBuffer, -(h+mBoxBuffer), w + (mBoxBuffer * 2), h + (mBoxBuffer * 2));
    }
    else{
        r.setRect(0, -h, w, h);
    }

    return r;

}

QPolygonF QgsComposerLabel::areaPoints() const
{
#ifdef QGISDEBUG
    std::cout << "QgsComposerLabel::areaPoints" << std::endl;
#endif
    QRectF r = boundingRect();

    QPolygonF pa;
    pa << QPointF( r.x(), r.y() );
    pa << QPointF( r.x()+r.width(), r.y() );
    pa << QPointF( r.x()+r.width(), r.y()+r.height() );
    pa << QPointF( r.x(), r.y()+r.height() );

    return pa ;
}

void QgsComposerLabel::setOptions ( void )
{ 
    mTextEdit->setText ( mText );
    mBoxCheckBox->setChecked ( mBox );
    
}

void QgsComposerLabel::on_mTextEdit_textChanged()
{ 
    QRectF r = boundingRect();
#if QT_VERSION < 0x040300
    mText = mTextEdit->text();
#else
    mText = mTextEdit->toPlainText();
#endif
    QAbstractGraphicsShapeItem::prepareGeometryChange();
    QAbstractGraphicsShapeItem::update();
    writeSettings();
}

void QgsComposerLabel::setSelected (  bool s ) 
{
#ifdef QGISDEBUG
    std::cout << "QgsComposerLabel::setSelected" << std::endl;
#endif

    mSelected = s;
    QAbstractGraphicsShapeItem::update(); // show highlight
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

    QgsProject::instance()->writeEntry( "Compositions", path+"x", mComposition->toMM((int)QAbstractGraphicsShapeItem::x()) );
    QgsProject::instance()->writeEntry( "Compositions", path+"y", mComposition->toMM((int)QAbstractGraphicsShapeItem::y()) );

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
    int y = mComposition->fromMM(QgsProject::instance()->readDoubleEntry( "Compositions", path+"y", 0, &ok) );
    QAbstractGraphicsShapeItem::setPos(x,y);

    mFont.setFamily ( QgsProject::instance()->readEntry("Compositions", path+"font/family", "", &ok) );
    mFont.setPointSize ( QgsProject::instance()->readNumEntry("Compositions", path+"font/size", 10, &ok) );
    mFont.setWeight(  QgsProject::instance()->readNumEntry("Compositions", path+"font/weight", (int)QFont::Normal, &ok) );
    mFont.setUnderline(  QgsProject::instance()->readBoolEntry("Compositions", path+"font/underline", false, &ok) );
    mFont.setStrikeOut(  QgsProject::instance()->readBoolEntry("Compositions", path+"font/strikeout", false, &ok) );

    mBox = QgsProject::instance()->readBoolEntry("Compositions", path+"box", false, &ok);

    QAbstractGraphicsShapeItem::update();

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
