/***************************************************************************
                         qgslabelattributes.h - render vector labels
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
#ifndef QGSLABELATTRIBUTES_H
#define QGSLABELATTRIBUTES_H

#include <qnamespace.h>
#include <qpen.h>
class QString;
class QFont;
class QColor;
class QPen;
class QBrush;

/** This class can be used to store attributes needed for label rendering. 
 *
 *  Label attributes:
 *                         border (color, width, style)
 *                        /
 *                       /     text bounding box
 *                      /     /
 *               +-----------/--+  buffer (color, pattern)                
 *               |          /   | /                                       
 *               |    +----+    |/  --+                                   
 *               |    |Text|    /     |--- text size                      
 *               |    +----+   /|   --+
 *               |              |
 *               +--------------+
 *               |         |    |  
 *               |        >|----|<--- buffer width
 *               |
 *              >|<--- border width 
 *
 *   Text:            
 *   - font family
 *   - size type (map units, points)
 *   - size (in points - device independent)
 *   - bold
 *   - italic
 *   - underline
 *   - color
 *
 *   Position:
 *   - (coordinates)
 *   - offset type (map units, points)
 *   - x offset, y offset (measured in text coordinate system, not in map coordinate system)
 *   - angle
 *   - alignment (from a point calculated by angle and offset)
 *   
 *   Buffer:
 *   - buffer size type (map units, points)
 *   - buffer size
 *   - buffer color
 *   - buffer brush
 *
 *   Border:
 *   - border width
 *   - border color
 *   - border style
 *
 *   Each attribute is either se or unset.
 */

class QgsLabelAttributes
{
public:
    /** Constructor.
     *  @param def if true, defaults are set, if false all all attributes are unset
     */
    QgsLabelAttributes ( bool def );

    ~QgsLabelAttributes();

    /* Units type */
    enum Units { 
	MapUnits = 0,
	PointUnits
    };

    static QString unitsName ( int units );
    static int unitsCode ( const QString &name );
    
    static QString alignmentName ( int alignment );
    static int alignmentCode ( const QString &name );

    /* Text */
    void setText ( const QString & text );
    bool textIsSet ( void );
    const QString text ( void );

    /* Font */
    void setFamily ( const QString & family );
    bool familyIsSet ( void );
    const QString family ( void );

    void setBold ( bool enable );
    bool boldIsSet ( void ) ;
    bool bold ( void );

    void setItalic ( bool enable );
    bool italicIsSet ( void );
    bool italic ( void );

    void setUnderline ( bool enable );
    bool underlineIsSet ( void );
    bool underline ( void ); 
    
    void   setSize ( double size, int type );
    bool   sizeIsSet ( void );
    int    sizeType ( void );
    double size ( void );

    void  setColor ( const QColor &color );
    bool  colorIsSet ( void );
    const QColor & color ( void );

    /* Offset */
    void   setOffset ( double x, double y, int type );
    bool   offsetIsSet ( void );
    int    offsetType ( void );
    double xOffset ( void );
    double yOffset ( void );

    /* Angle */
    void   setAngle ( double angle );
    bool   angleIsSet ( void ) ;
    double angle ( void );

    /* Alignment */
    void setAlignment ( int alignment );
    bool alignmentIsSet ( void );
    int  alignment ( void ) ;

    /* Buffer */
    void   setBufferSize ( double size, int type );
    bool   bufferSizeIsSet ( void );
    int    bufferSizeType ( void );
    double bufferSize ( void );

    void  setBufferColor ( const QColor &color );
    bool  bufferColorIsSet ( void );
    const QColor & bufferColor ( void );

    void  setBufferStyle ( Qt::BrushStyle style );
    bool  bufferStyleIsSet ( void );
    Qt::BrushStyle bufferStyle ( void );

    /* Border */
    void  setBorderColor ( const QColor &color );
    bool  borderColorIsSet ( void );
    const QColor & borderColor ( void );

    void  setBorderWidth ( int width );
    bool  borderWidthIsSet ( void );
    int   borderWidth ( void );
    
    void  setBorderStyle ( Qt::PenStyle style );
    bool  borderStyleIsSet ( void );
    Qt::PenStyle   borderStyle ( void );
    
 protected:
    /* Text */
    QString mText;
    bool mTextIsSet;

    /** Font (family, weight, italic, underline) */
    QFont mFont;
    bool mFamilyIsSet;
    bool mBoldIsSet;
    bool mItalicIsSet;
    bool mUnderlineIsSet;

    /** Font size, size type */
    int  mSizeType;
    double mSize;
    bool   mSizeIsSet;

    /** Color */
    QColor mColor;
    bool   mColorIsSet;

    /** Offset */
    int    mOffsetType;
    double mXOffset;
    double mYOffset;
    bool   mOffsetIsSet;

    /** Angle (degrees) */
    double mAngle;
    bool   mAngleIsSet;

    /** Alignment */
    int  mAlignment;
    bool mAlignmentIsSet;            

    /** Buffer size, size type */
    int    mBufferSizeType;
    double mBufferSize;
    bool   mBufferSizeIsSet;
    
    /** Buffer brush (color, style) */
    QBrush mBufferBrush;
    bool   mBufferColorIsSet;
    bool   mBufferStyleIsSet;

    /** Border pen (color, width, style) */
    QPen mBorderPen;
    bool mBorderColorIsSet;
    bool mBorderWidthIsSet;
    bool mBorderStyleIsSet;
};

#endif
