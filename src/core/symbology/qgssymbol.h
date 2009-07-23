/***************************************************************************
                          qgssymbol.h  -  description
                             -------------------
    begin                : Sat Jun 22 2002
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman at mrcc.com
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

#ifndef QGSSYMBOL_H
#define QGSSYMBOL_H

#include "qgis.h"
#include <QBrush>
#include <QPen>
#include <QImage>

class QDomNode;
class QDomDocument;
class QDomElement;

class QgsVectorLayer;

/**Encapsulates settings for drawing (QPen, QBrush, Point symbol) and classification
  (lower value, upper value)*/
class CORE_EXPORT QgsSymbol
{
  public:
    /**Constructor*/
    QgsSymbol( QGis::GeometryType t, QString lvalue = "", QString uvalue = "", QString label = "" );
    /**Constructor*/
    QgsSymbol( QGis::GeometryType t, QString lvalue, QString uvalue, QString label, QColor c );
    QgsSymbol( const QgsSymbol& );
    /**old constructors*/
    QgsSymbol();
    QgsSymbol( QColor c );
    /**Sets the brush*/
    virtual void setBrush( QBrush b );
    /**Gets a reference to m_brush, Don't use the brush to change color/style */
    virtual const QBrush& brush() const;
    /**Set the color*/
    virtual void setColor( QColor c );
    /**Get the current color*/
    virtual QColor color() const;
    /**Get the fill color*/
    virtual QColor fillColor() const;
    /**Sets the fill color*/
    virtual void setFillColor( QColor c );
    /**Get the line width*/
    virtual double lineWidth() const;
    /**Sets the line width*/
    virtual void setLineWidth( double w );
    /**Sets the pen*/
    virtual void setPen( QPen p );
    /**Gets a reference to m_pen. Don't use the pen to change color/style  */
    virtual const QPen& pen() const;

    /**Set the line (pen) style*/
    virtual void setLineStyle( Qt::PenStyle s );
    /**Set the fill (brush) style*/
    virtual void setFillStyle( Qt::BrushStyle s );

    /**Gets the path to the customs texture image*/
    virtual QString customTexture() const;
    /**Sets the path to the custom texture, and sets the brush to use TexturePattern */
    virtual void setCustomTexture( QString path );

    virtual void setLowerValue( QString value );
    virtual QString lowerValue() const;
    virtual void setUpperValue( QString value );
    virtual QString upperValue() const;
    virtual void setLabel( QString label );
    virtual QString label() const;

    /**Set point symbol from name*/
    virtual void setNamedPointSymbol( QString name );
    /**Get point symbol*/
    virtual QString pointSymbolName() const;

    /**Set point size*/
    virtual void setPointSize( double s );
    /**Get size*/
    virtual double pointSize() const;

    /**Set point size units*/
    virtual void setPointSizeUnits( bool sizeInMapUnits );
    /**get point size units*/
    virtual bool pointSizeUnits() const;

    //! Destructor
    virtual ~QgsSymbol();

    //! Get a little icon for the legend
    virtual QImage getLineSymbolAsImage();

    //! Get a little icon for the legend
    virtual QImage getPolygonSymbolAsImage();

    /** Get QImage representation of point symbol with current settings
    */
    virtual QImage getCachedPointSymbolAsImage( double widthScale = 1.0,
        bool selected = false, QColor selectionColor = Qt::yellow );

    /** Get QImage representation of point symbol with current settings
     * and scaled (can be slow when scale != 1.0)
     */
    virtual QImage getPointSymbolAsImage( double widthScale = 1.0,
                                          bool selected = false,
                                          QColor selectionColor = Qt::yellow,
                                          double scale = 1.0,
                                          double rotation = 0.0,
                                          double rasterScaleFactor = 1.0 );

    /**Writes the contents of the symbol to a configuration file
      @ return true in case of success*/
    virtual bool writeXML( QDomNode & item, QDomDocument & document, const QgsVectorLayer *vl ) const;
    /**Reads the contents of the symbol from a configuration file
      @ return true in case of success*/
    virtual bool readXML( QDomNode & symbol, const QgsVectorLayer *vl );
    /**Returns if this symbol is point/ line or polygon*/
    QGis::GeometryType type() const {return mType;}

    /**Returns the number of the rotation classification field*/
    int rotationClassificationField() const;
    /**Sets the number of the rotation classicifation field
    \param field the number of the field to classify for rotation*/
    void setRotationClassificationField( int field );

    /**Returns the number of the scale classification field*/
    int scaleClassificationField() const;
    /**Sets the number of the scale classicifation field
    \param field the number of the field to classify for scale*/
    void setScaleClassificationField( int field );

    /**Returns the number of the symbol field*/
    int symbolField() const;
    /**Sets the number of the symbol field
    \param field the number of the field to select the symbol*/
    void setSymbolField( int field );

  protected:

    /**Lower value for classification*/
    QString mLowerValue;
    /**Upper value for classification*/
    QString mUpperValue;
    QString mLabel;
    /**Vector type (point, line, polygon)*/
    QGis::GeometryType mType;

    QPen mPen;
    QBrush mBrush;
    QString mTextureFilePath;
    /* Point symbol name */
    QString mPointSymbolName;
    /* size */
    double mSize;
    /* units of size */
    bool mSizeInMapUnits;


    /* TODO Because for printing we always need a symbol without oversampling but with line width scale,
     *      we keep also separate picture with line width scale */

    //
    //
    // NOTE THE LOGIC OF THESE MEMBER VARS NEED TO BE REVISITED NOW THAT
    // I HAVE REMOVED SVG OVERSAMPLING (NEEDED IN QT3 WITH POOR SVG SUPPORT)
    // Tim Sutton 2006 XXX FIXME
    //
    //


    /* Point symbol cache  */
    QImage mPointSymbolImage;

    /* Point symbol cache  */
    QImage mPointSymbolImageSelected;

    /* Current line width scale used by mPointSymbolVectorImage. Equals -1.0 if not set */
    double mWidthScale;

    /* Point symbol cache but with line width scale mWidthScale */
    QImage mPointSymbolImage2;
    QImage mPointSymbolImageSelected2;

    /* Create point symbol mPointSymbolImage/mPointSymbolImage cache */
    void cache( QColor selectionColor );

    /* Create point symbol mPointSymbolImage2 cache */
    void cache2( double widthScale, QColor selectionColor );

    /* mPointSymbolImage/mPointSymbolImage cache updated */
    bool mCacheUpToDate;

    /* mPointSymbolImage2 cache updated */
    bool mCacheUpToDate2;

    /* Selection color used in cache */
    QColor mSelectionColor;
    QColor mSelectionColor2;

    /**Index of the classification fields (it must be a numerical field index)*/
    int mRotationClassificationField;
    int mScaleClassificationField;
    int mSymbolField;

  private:
    int readFieldName( QDomNode &synode, QString name, const QgsVectorLayer &vl );
    void appendField( QDomElement &symbol, QDomDocument &document, const QgsVectorLayer &vl, QString name, int idx ) const;
    void appendText( QDomElement &symbol, QDomDocument &document, QString name, QString value ) const;

};

inline void QgsSymbol::setBrush( QBrush b )
{
  mBrush = b;
}

inline const QBrush& QgsSymbol::brush() const
{
  return mBrush;
}

inline void QgsSymbol::setPen( QPen p )
{
  mPen = p;
}

inline const QPen& QgsSymbol::pen() const
{
  return mPen;
}

inline void QgsSymbol::setLowerValue( QString value )
{
  mLowerValue = value;
}

inline QString QgsSymbol::lowerValue() const
{
  return mLowerValue;
}

inline void QgsSymbol::setUpperValue( QString value )
{
  mUpperValue = value;
}

inline QString QgsSymbol::upperValue() const
{
  return mUpperValue;
}

inline void QgsSymbol::setLabel( QString label )
{
  mLabel = label;
}

inline QString QgsSymbol::label() const
{
  return mLabel;
}

#endif // QGSSYMBOL_H


