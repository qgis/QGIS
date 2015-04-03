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
#ifndef QGSLABELATTRIBUTES_H
#define QGSLABELATTRIBUTES_H

#include <QBrush>
#include <QFont>
#include <QPen>

class QString;
class QColor;

/** \ingroup core
 * A class to store attributes needed for label rendering.
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
 *   Each attribute is either set or unset.
 */

class  CORE_EXPORT QgsLabelAttributes
{
  public:
    /** Constructor.
     *  @param def if true, defaults are set, if false all all attributes are unset
     */
    QgsLabelAttributes( bool def = true );

    ~QgsLabelAttributes();

    /* Units type */
    enum Units
    {
      MapUnits = 0,
      PointUnits
    };

    static QString unitsName( int units );
    static int unitsCode( const QString &name );

    static QString alignmentName( int alignment );
    static int alignmentCode( const QString &name );

    /* Text */
    void setText( const QString & text );
    bool textIsSet() const;
    const QString text() const;

    /* Font */
    void setFamily( const QString & family );
    bool familyIsSet() const;
    const QString family() const;

    void setBold( bool enable );
    bool boldIsSet() const;
    bool bold() const;

    void setItalic( bool enable );
    bool italicIsSet() const;
    bool italic() const;

    void setUnderline( bool enable );
    bool underlineIsSet() const;
    bool underline() const;

    void setStrikeOut( bool enable );
    bool strikeOutIsSet() const;
    bool strikeOut() const;

    void   setSize( double size, int type );
    bool   sizeIsSet() const;
    int    sizeType() const;
    double size() const;

    void  setColor( const QColor &color );
    bool  colorIsSet() const;
    const QColor & color() const;

    /* Offset */
    void   setOffset( double x, double y, int type );
    bool   offsetIsSet() const;
    int    offsetType() const;
    double xOffset() const;
    double yOffset() const;

    /* Angle */
    void   setAngle( double angle );
    bool   angleIsSet() const;
    double angle() const;

    bool   angleIsAuto() const;
    void   setAutoAngle( bool state );

    /* Alignment */
    void setAlignment( int alignment );
    bool alignmentIsSet() const;
    int  alignment() const;

    /* Buffer */
    bool   bufferEnabled() const;
    void   setBufferEnabled( bool useBufferFlag );
    void   setBufferSize( double size, int type );
    bool   bufferSizeIsSet() const;
    int    bufferSizeType() const;
    double bufferSize() const;

    void  setBufferColor( const QColor &color );
    bool  bufferColorIsSet() const;
    QColor bufferColor() const;

    void  setBufferStyle( Qt::BrushStyle style );
    bool  bufferStyleIsSet() const;
    Qt::BrushStyle bufferStyle() const;

    /* Border */
    void  setBorderColor( const QColor &color );
    bool  borderColorIsSet() const;
    QColor borderColor() const;

    void  setBorderWidth( int width );
    bool  borderWidthIsSet() const;
    int   borderWidth() const;

    void  setBorderStyle( Qt::PenStyle style );
    bool  borderStyleIsSet() const;
    Qt::PenStyle   borderStyle() const;

    bool  multilineEnabled() const;
    void  setMultilineEnabled( bool useMultiline );

    /* label only selected features */
    bool  selectedOnly() const;
    void  setSelectedOnly( bool selectedonly );

  protected:
    /* Text */
    QString mText;
    bool mTextIsSet;

    /** Font (family, weight, italic, underline, strikeout) */
    QFont mFont;
    bool mFamilyIsSet;
    bool mBoldIsSet;
    bool mItalicIsSet;
    bool mUnderlineIsSet;
    bool mStrikeOutIsSet;

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
    bool   mAngleIsAuto;

    /** Alignment */
    int  mAlignment;
    bool mAlignmentIsSet;

    /** Buffer enablement */
    bool mBufferEnabledFlag;
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

    /** Multiline enablement */
    bool mMultilineEnabledFlag;

    /** Label only selected */
    bool mSelectedOnly;
};

#endif
