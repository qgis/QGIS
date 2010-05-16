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
/* $Id$ */

#include <QApplication>
#include <QString>
#include <QFont>
#include <QColor>
#include <QPen>
#include <QBrush>

#include "qgslabelattributes.h"
#include "qgslogger.h"

QgsLabelAttributes::QgsLabelAttributes( bool def )
    : mTextIsSet( false )
    , mFamilyIsSet( false )
    , mBoldIsSet( false )
    , mItalicIsSet( false )
    , mUnderlineIsSet( false )
    , mStrikeOutIsSet( false )
    , mSizeType( 0 )
    , mSize( 0.0 )
    , mSizeIsSet( false )
    , mColorIsSet( false )
    , mOffsetType( 0 )
    , mXOffset( 0 )
    , mYOffset( 0 )
    , mOffsetIsSet( false )
    , mAngle( 0.0 )
    , mAngleIsSet( false )
    , mAngleIsAuto( false )
    , mAlignment( 0 )
    , mAlignmentIsSet( false )
    , mBufferEnabledFlag( false )
    , mBufferSizeType( 0 )
    , mBufferSize( 0.0 )
    , mBufferSizeIsSet( false )
    , mBufferColorIsSet( false )
    , mBufferStyleIsSet( false )
    , mBorderColorIsSet( false )
    , mBorderWidthIsSet( false )
    , mBorderStyleIsSet( false )
    , mMultilineEnabledFlag( false )
    , mSelectedOnly( false )
{

  if ( def )   // set defaults
  {
    setText( QObject::tr( "Label" ) );

    mFont = QApplication::font();
    mFamilyIsSet = true;
    mBoldIsSet = true;
    mItalicIsSet = true;
    mUnderlineIsSet = true;

    setSize( 12.0, PointUnits );

    setOffset( 0, 0, PointUnits );
    setAngle( 0 );
    setAutoAngle( false );

    setAlignment( Qt::AlignCenter );
    setColor( QColor( 0, 0, 0 ) );

    setBufferSize( 1, PointUnits );
    setBufferColor( QColor( 255, 255, 255 ) );
    setBufferStyle( Qt::NoBrush );

    setBorderWidth( 0 );
    setBorderColor( QColor( 0, 0, 0 ) );
    setBorderStyle( Qt::NoPen );
  }
}

QgsLabelAttributes::~QgsLabelAttributes()
{
}
/* Text */
void QgsLabelAttributes::setText( const QString & text )
{
  mText = text;
  mTextIsSet = true;
}

bool QgsLabelAttributes::textIsSet( void ) const
{
  return mTextIsSet;
}

const QString QgsLabelAttributes::text( void ) const
{
  return mText;
}


/* Offset */
void QgsLabelAttributes::setOffset( double x, double y, int type )
{
  mOffsetType = type;
  mXOffset = x;
  mYOffset = y;
  mOffsetIsSet = true;
}

bool QgsLabelAttributes::offsetIsSet( void ) const
{
  return mOffsetIsSet;
}

int QgsLabelAttributes::offsetType( void ) const
{
  return mOffsetType;
}

double QgsLabelAttributes::xOffset( void ) const
{
  return mXOffset;
}

double QgsLabelAttributes::yOffset( void ) const
{
  return mYOffset;
}

/* Angle */
void QgsLabelAttributes::setAngle( double angle )
{
  mAngle = angle;
  mAngleIsSet = true;
}

bool QgsLabelAttributes::angleIsSet( void ) const
{
  return mAngleIsSet;
}

double QgsLabelAttributes::angle( void ) const
{
  return mAngle;
}

bool QgsLabelAttributes::angleIsAuto() const
{
  return mAngleIsAuto;
}

void QgsLabelAttributes::setAutoAngle( bool state )
{
  mAngleIsAuto = state;
}

/* Alignment */
void QgsLabelAttributes::setAlignment( int alignment )
{
  mAlignment = alignment;
  mAlignmentIsSet = true;
}

bool QgsLabelAttributes::alignmentIsSet( void ) const
{
  return mAlignmentIsSet;
}

int QgsLabelAttributes::alignment( void ) const
{
  return mAlignment;
}

/* Font */
void QgsLabelAttributes::setFamily( const QString & family )
{
  mFont.setFamily( family );
  mFamilyIsSet = true;
}

bool QgsLabelAttributes::familyIsSet( void ) const
{
  return mFamilyIsSet;
}

const QString QgsLabelAttributes::family( void ) const
{
  return mFont.family();
}


void QgsLabelAttributes::setBold( bool enable )
{
  mFont.setBold( enable );
  mBoldIsSet = true;
}

bool QgsLabelAttributes::boldIsSet( void ) const
{
  return mBoldIsSet;
}

bool QgsLabelAttributes::bold( void ) const
{
  return mFont.bold();
}


void QgsLabelAttributes::setItalic( bool enable )
{
  mFont.setItalic( enable );
  mItalicIsSet = true;
}

bool QgsLabelAttributes::italicIsSet( void ) const
{
  return mItalicIsSet;
}

bool QgsLabelAttributes::italic( void ) const
{
  return mFont.italic();
}


void QgsLabelAttributes::setUnderline( bool enable )
{
  mFont.setUnderline( enable );
  mUnderlineIsSet = true;
}

bool QgsLabelAttributes::underlineIsSet( void ) const
{
  return mUnderlineIsSet;
}

bool QgsLabelAttributes::underline( void ) const
{
  return mFont.underline();
}

void QgsLabelAttributes::setStrikeOut( bool enable )
{
  mFont.setStrikeOut( enable );
  mStrikeOutIsSet = true;
}

bool QgsLabelAttributes::strikeOutIsSet( void ) const
{
  return mStrikeOutIsSet;
}

bool QgsLabelAttributes::strikeOut( void ) const
{
  return mFont.strikeOut();
}


void QgsLabelAttributes::setSize( double size, int type )
{
  mSizeType = type;
  mSize = size;
  mSizeIsSet = true;
}

bool QgsLabelAttributes::sizeIsSet( void ) const
{
  return mSizeIsSet;
}

int QgsLabelAttributes::sizeType( void ) const
{
  return mSizeType;
}

double QgsLabelAttributes::size( void ) const
{
  return mSize;
}


void QgsLabelAttributes::setColor( const QColor &color )
{
  mColor = color;
  mColorIsSet = true;
}

bool QgsLabelAttributes::colorIsSet( void ) const
{
  return mColorIsSet;
}

const QColor & QgsLabelAttributes::color( void ) const
{
  return mColor;
}

/* Buffer */
bool QgsLabelAttributes::bufferEnabled() const
{
  return mBufferEnabledFlag;
}
void QgsLabelAttributes::setBufferEnabled( bool useBufferFlag )
{
  mBufferEnabledFlag = useBufferFlag;
}
void QgsLabelAttributes::setBufferSize( double size, int type )
{
  mBufferSizeType = type;
  mBufferSize = size;
  mBufferSizeIsSet = true;
}

bool QgsLabelAttributes::bufferSizeIsSet( void ) const
{
  return mBufferSizeIsSet;
}

int QgsLabelAttributes::bufferSizeType( void ) const
{
  return mBufferSizeType;
}

double QgsLabelAttributes::bufferSize( void ) const
{
  return mBufferSize;
}


void QgsLabelAttributes::setBufferColor( const QColor &color )
{
  mBufferBrush.setColor( color );
  mBufferColorIsSet = true;
}

bool QgsLabelAttributes::bufferColorIsSet( void ) const
{
  return mColorIsSet;
}

QColor QgsLabelAttributes::bufferColor( void ) const
{
  return mBufferBrush.color();
}


void QgsLabelAttributes::setBufferStyle( Qt::BrushStyle style )
{
  mBufferBrush.setStyle( style );
  mBufferStyleIsSet = true;
}

bool QgsLabelAttributes::bufferStyleIsSet( void ) const
{
  return mBufferStyleIsSet;
}

Qt::BrushStyle QgsLabelAttributes::bufferStyle( void ) const
{
  return mBufferBrush.style();
}

/* Border */
void QgsLabelAttributes::setBorderColor( const QColor &color )
{
  mBorderPen.setColor( color );
  mBorderColorIsSet = true;
}

bool QgsLabelAttributes::borderColorIsSet( void ) const
{
  return mBorderColorIsSet;
}

QColor QgsLabelAttributes::borderColor( void ) const
{
  return mBorderPen.color();
}

void QgsLabelAttributes::setBorderWidth( int width )
{
  mBorderPen.setWidth( width );
  mBorderWidthIsSet = true;
}

bool QgsLabelAttributes::borderWidthIsSet( void ) const
{
  return mBorderWidthIsSet;
}

int QgsLabelAttributes::borderWidth( void ) const
{
  return mBorderPen.width();
}


void QgsLabelAttributes::setBorderStyle( Qt::PenStyle style )
{
  mBorderPen.setStyle( style );
  mBorderStyleIsSet = true;
}

bool QgsLabelAttributes::borderStyleIsSet( void ) const
{
  return mBorderStyleIsSet;
}

Qt::PenStyle QgsLabelAttributes::borderStyle( void ) const
{
  return mBorderPen.style();
}

/* Multiline */
bool QgsLabelAttributes::multilineEnabled() const
{
  return mMultilineEnabledFlag;
}
void QgsLabelAttributes::setMultilineEnabled( bool useMultilineFlag )
{
  mMultilineEnabledFlag = useMultilineFlag;
}

/* selected only */
bool QgsLabelAttributes::selectedOnly() const
{
  return mSelectedOnly;
}
void QgsLabelAttributes::setSelectedOnly( bool selectedOnly )
{
  mSelectedOnly = selectedOnly;
}

/* units */
QString QgsLabelAttributes::unitsName( int units )
{
  if ( units == MapUnits )
  {
    return QString( "mu" );
  }

  return QString( "pt" );
}

int QgsLabelAttributes::unitsCode( const QString &name )
{
  if ( name.compare( "mu" ) == 0 )
  {
    return MapUnits;
  }

  return PointUnits;
}

/* alignment */
QString QgsLabelAttributes::alignmentName( int alignment )
{
  QgsDebugMsg( QString( "alignment=%1" ).arg( alignment ) );
  if ( !alignment )                                       return  QString( "center" );
  if ( alignment == ( Qt::AlignRight | Qt::AlignBottom ) ) return  QString( "aboveleft" );
  if ( alignment == ( Qt::AlignRight | Qt::AlignTop ) ) return  QString( "belowleft" );
  if ( alignment == ( Qt::AlignLeft  | Qt::AlignBottom ) ) return  QString( "aboveright" );
  if ( alignment == ( Qt::AlignLeft  | Qt::AlignTop ) ) return  QString( "belowright" );
  if ( alignment == ( Qt::AlignRight | Qt::AlignVCenter ) ) return  QString( "left" );
  if ( alignment == ( Qt::AlignLeft  | Qt::AlignVCenter ) ) return  QString( "right" );
  if ( alignment == ( Qt::AlignBottom | Qt::AlignHCenter ) ) return  QString( "above" );
  if ( alignment == ( Qt::AlignTop   | Qt::AlignHCenter ) ) return  QString( "below" );
  if ( alignment == ( Qt::AlignCenter ) ) return  QString( "center" );
  return QString( "center" );
}

int QgsLabelAttributes::alignmentCode( const QString &name )
{
  QString lname = name.toLower();
  if ( lname.compare( "aboveleft" )  == 0 )  return Qt::AlignRight | Qt::AlignBottom     ;
  if ( lname.compare( "belowleft" )  == 0 )  return Qt::AlignRight | Qt::AlignTop        ;
  if ( lname.compare( "aboveright" )  == 0 ) return Qt::AlignLeft  | Qt::AlignBottom     ;
  if ( lname.compare( "belowright" )  == 0 ) return Qt::AlignLeft  | Qt::AlignTop        ;
  if ( lname.compare( "left" )  == 0 )       return Qt::AlignRight | Qt::AlignVCenter    ;
  if ( lname.compare( "right" )  == 0 )      return Qt::AlignLeft  | Qt::AlignVCenter    ;
  if ( lname.compare( "above" )  == 0 )      return Qt::AlignBottom | Qt::AlignHCenter    ;
  if ( lname.compare( "below" )  == 0 )      return Qt::AlignTop   | Qt::AlignHCenter    ;
  if ( lname.compare( "center" )  == 0 )       return Qt::AlignCenter                      ;


  return Qt::AlignCenter;
}
