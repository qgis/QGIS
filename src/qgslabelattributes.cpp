/***************************************************************************
                         qgslabel.cpp - render vector labels
                             -------------------
    begin                : August 2004
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

#include <qapplication.h>
#include <qstring.h>
#include <qfont.h>
#include <qcolor.h>
#include <qpen.h>
#include <qbrush.h>

#include "qgslabelattributes.h"

QgsLabelAttributes::QgsLabelAttributes( bool def ) 
{
    #ifdef QGISDEBUG
    std::cerr << "QgsLabelAttributes::QgsLabelAttributes()" << std::endl;
    #endif

    if ( def ) { // set defaults
	setText ( "Label" );

	mFont = QApplication::font();
	mFamilyIsSet = true;
	mBoldIsSet = true;
	mItalicIsSet = true;
	mUnderlineIsSet = true;

	setSize(12.0, PointUnits);
	
	setOffset ( 0, 0, PointUnits );
	setAngle ( 0 );
	
	setAlignment ( Qt::AlignCenter );
	setColor ( QColor(0,0,0) );

	setBufferSize ( 0, PointUnits );
	setBufferColor ( QColor(255,255,255) );
	setBufferStyle ( Qt::NoBrush );
	
	setBorderWidth ( 0 );
	setBorderColor ( QColor(0,0,0) );
	setBorderStyle ( Qt::NoPen );
    }
}

QgsLabelAttributes::~QgsLabelAttributes()
{
}
  /* Text */
void QgsLabelAttributes::setText ( const QString & text )
{
    mText = text;
    mTextIsSet = true;
}

bool QgsLabelAttributes::textIsSet ( void )
{
    return mTextIsSet;
}

const QString QgsLabelAttributes::text ( void )
{
    return mText;
}


  /* Offset */
void QgsLabelAttributes::setOffset ( double x, double y, int type )
{
    mOffsetType = type;
    mXOffset = x;
    mYOffset = y;
    mOffsetIsSet = true;
}

bool QgsLabelAttributes::offsetIsSet ( void )
{
    return mOffsetIsSet;
}

int QgsLabelAttributes::offsetType ( void )
{
    return mOffsetType;
}

double QgsLabelAttributes::xOffset ( void )
{
    return mXOffset;
}

double QgsLabelAttributes::yOffset ( void )
{
    return mYOffset;
}

  /* Angle */
void QgsLabelAttributes::setAngle ( double angle )
{
    mAngle = angle;
    mAngleIsSet = true;
}

bool QgsLabelAttributes::angleIsSet ( void )
{
    return mAngleIsSet;
}

double QgsLabelAttributes::angle ( void )
{
    return mAngle;
}

  /* Alignment */
void QgsLabelAttributes::setAlignment ( int alignment )
{
    mAlignment = alignment;
    mAlignmentIsSet = true;
}

bool QgsLabelAttributes::alignmentIsSet ( void )
{
    return mAlignmentIsSet;
}

int QgsLabelAttributes::alignment ( void )
{
    return mAlignment;
}

  /* Font */
void QgsLabelAttributes::setFamily ( const QString & family )
{
    mFont.setFamily ( family );
    mFamilyIsSet = true;
}

bool QgsLabelAttributes::familyIsSet ( void )
{
    return mFamilyIsSet;
}

const QString QgsLabelAttributes::family ( void )
{
    return mFont.family();
}


void QgsLabelAttributes::setBold ( bool enable )
{
    mFont.setBold ( enable );
    mBoldIsSet = true;
}

bool QgsLabelAttributes::boldIsSet ( void )
{
    return mBoldIsSet;
}

bool QgsLabelAttributes::bold ( void )
{
    return mFont.bold();
}


void QgsLabelAttributes::setItalic ( bool enable )
{
    mFont.setItalic ( enable );
    mItalicIsSet = true;
}

bool QgsLabelAttributes::italicIsSet ( void )
{
    return mItalicIsSet;
}

bool QgsLabelAttributes::italic ( void )
{
    return mFont.italic();
}


void QgsLabelAttributes::setUnderline ( bool enable )
{
    mFont.setUnderline ( enable );
    mUnderlineIsSet = true;
}

bool QgsLabelAttributes::underlineIsSet ( void )
{
    return mUnderlineIsSet;
}

bool QgsLabelAttributes::underline ( void )
{
    return mFont.underline();
}


void QgsLabelAttributes::setSize ( double size, int type )
{
    mSizeType = type;
    mSize = size;
    mSizeIsSet = true;
}

bool QgsLabelAttributes::sizeIsSet ( void )
{
    return mSizeIsSet;
}

int QgsLabelAttributes::sizeType ( void )
{
    return mSizeType;
}

double QgsLabelAttributes::size ( void )
{
    return mSize;
}


void QgsLabelAttributes::setColor ( const QColor &color )
{
    mColor = color;
    mColorIsSet = true;
}

bool QgsLabelAttributes::colorIsSet ( void )
{
    return mColorIsSet;
}

const QColor & QgsLabelAttributes::color ( void )
{
    return mColor;
}

  /* Buffer */
void QgsLabelAttributes::setBufferSize ( double size, int type )
{
    mBufferSizeType = type;
    mBufferSize = size;
    mBufferSizeIsSet = true;
}

bool QgsLabelAttributes::bufferSizeIsSet ( void )
{
    return mBufferSizeIsSet;
}

int QgsLabelAttributes::bufferSizeType ( void )
{
    return mBufferSizeType;
}

double QgsLabelAttributes::bufferSize ( void )
{
    return mBufferSize;
}


void QgsLabelAttributes::setBufferColor ( const QColor &color )
{
    mBufferBrush.setColor ( color );
    mBufferColorIsSet = true;
}

bool QgsLabelAttributes::bufferColorIsSet ( void )
{
    return mColorIsSet;
}

const QColor & QgsLabelAttributes::bufferColor ( void )
{
    return mBufferBrush.color();
}


void QgsLabelAttributes::setBufferStyle ( Qt::BrushStyle style )
{
    mBufferBrush.setStyle ( style );
    mBufferStyleIsSet = true;
}

bool QgsLabelAttributes::bufferStyleIsSet ( void )
{
    return mBufferStyleIsSet;
}

Qt::BrushStyle QgsLabelAttributes::bufferStyle ( void )
{
    return mBufferBrush.style();
}

  /* Border */
void QgsLabelAttributes::setBorderColor ( const QColor &color )
{
    mBorderPen.setColor ( color );
    mBorderColorIsSet = true;
}

bool QgsLabelAttributes::borderColorIsSet ( void )
{
    return mBorderColorIsSet;
}

const QColor & QgsLabelAttributes::borderColor ( void )
{
    return mBorderPen.color();
}

void QgsLabelAttributes::setBorderWidth ( int width )
{
    mBorderPen.setWidth ( width );
    mBorderWidthIsSet = true;
}

bool QgsLabelAttributes::borderWidthIsSet ( void ) 
{
    return mBorderWidthIsSet;
}

int QgsLabelAttributes::borderWidth ( void )
{
    return mBorderPen.width();
}


void QgsLabelAttributes::setBorderStyle ( Qt::PenStyle style )
{
     mBorderPen.setStyle ( style );
     mBorderStyleIsSet = true;
}

bool QgsLabelAttributes::borderStyleIsSet ( void )
{
    return mBorderStyleIsSet;
}

Qt::PenStyle QgsLabelAttributes::borderStyle ( void )
{
    return mBorderPen.style();
}

QString QgsLabelAttributes::unitsName ( int units )
{
    if ( units == MapUnits ){ 
	return QString("mu");
    }
	
    return QString("pt");
}

int QgsLabelAttributes::unitsCode ( const QString &name )
{
    if ( name.compare("mu") == 0 ) {
	return MapUnits;
    }
	
    return PointUnits;
}

QString QgsLabelAttributes::alignmentName ( int alignment )
{
  std::cout << "QString QgsLabelAttributes::alignmentName (" << alignment << ")" << std::endl;
  if (!alignment)                                       return  QString("center");
  if (alignment == (Qt::AlignRight | Qt::AlignBottom )) return  QString("aboveleft");
  if (alignment == (Qt::AlignRight | Qt::AlignTop    )) return  QString("belowleft"); 
  if (alignment == (Qt::AlignLeft  | Qt::AlignBottom )) return  QString("aboveright");
  if (alignment == (Qt::AlignLeft  | Qt::AlignTop    )) return  QString("belowright");
  if (alignment == (Qt::AlignRight | Qt::AlignVCenter)) return  QString("left");
  if (alignment == (Qt::AlignLeft  | Qt::AlignVCenter)) return  QString("right");
  if (alignment == (Qt::AlignBottom| Qt::AlignHCenter)) return  QString("above"); 
  if (alignment == (Qt::AlignTop   | Qt::AlignHCenter)) return  QString("below"); 
  if (alignment == (Qt::AlignCenter                  )) return  QString("center");
  return QString("center");
}

int QgsLabelAttributes::alignmentCode ( const QString &name )
{
  QString lname = name.lower();
  if (lname.compare("aboveleft")  == 0)  return Qt::AlignRight | Qt::AlignBottom     ;
  if (lname.compare("belowleft")  == 0)  return Qt::AlignRight | Qt::AlignTop        ; 
  if (lname.compare("aboveright")  == 0) return Qt::AlignLeft  | Qt::AlignBottom     ;
  if (lname.compare("belowright")  == 0) return Qt::AlignLeft  | Qt::AlignTop        ;
  if (lname.compare("left")  == 0)       return Qt::AlignRight | Qt::AlignVCenter    ;
  if (lname.compare("right")  == 0)      return Qt::AlignLeft  | Qt::AlignVCenter    ;
  if (lname.compare("above")  == 0)      return Qt::AlignBottom| Qt::AlignHCenter    ; 
  if (lname.compare("below")  == 0)      return Qt::AlignTop   | Qt::AlignHCenter    ; 
  if (lname.compare("center")  == 0)       return Qt::AlignCenter                      ;  


  return Qt::AlignCenter;
}
