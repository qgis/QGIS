/***************************************************************************
    qgsrendererrange.h
    ---------------------
    begin                : September 2019
    copyright            : (C) 2019 by Denis Rouzaud
    email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRENDERERRANGE_H
#define QGSRENDERERRANGE_H

#include <QRegExp>

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgssymbollayerutils.h"

class QDomDocument;
class QDomElement;

class QgsSymbol;
class QgsClassificationRange;


/**
 * \ingroup core
 * \class QgsRendererRange
 */
class CORE_EXPORT QgsRendererRange
{
  public:

    /**
     * Constructor for QgsRendererRange.
     */
    QgsRendererRange() = default;

    /**
     * Creates a renderer symbol range
     * \param range The classification range
     * \param symbol The symbol for this renderer range
     * \param render If true, it will be renderered
     */
    QgsRendererRange( const QgsClassificationRange &range, QgsSymbol *symbol SIP_TRANSFER, bool render = true );
    QgsRendererRange( double lowerValue, double upperValue, QgsSymbol *symbol SIP_TRANSFER, const QString &label, bool render = true );
    QgsRendererRange( const QgsRendererRange &range );

    // default dtor is OK
    QgsRendererRange &operator=( QgsRendererRange range );

    bool operator<( const QgsRendererRange &other ) const;

    double lowerValue() const;
    double upperValue() const;

    QgsSymbol *symbol() const;
    QString label() const;

    void setSymbol( QgsSymbol *s SIP_TRANSFER );
    void setLabel( const QString &label );
    void setLowerValue( double lowerValue );
    void setUpperValue( double upperValue );

    // \since QGIS 2.5
    bool renderState() const;
    void setRenderState( bool render );

    // debugging
    QString dump() const;

    /**
     * Creates a DOM element representing the range in SLD format.
     * \param doc DOM document
     * \param element destination DOM element
     * \param props graduated renderer properties
     * \param firstRange set to TRUE if the range is the first range, where the lower value uses a <= test
     * rather than a < test.
     */
    void toSld( QDomDocument &doc, QDomElement &element, QgsStringMap props, bool firstRange = false ) const;

  protected:
    double mLowerValue = 0, mUpperValue = 0;
    std::unique_ptr<QgsSymbol> mSymbol;
    QString mLabel;
    bool mRender = true;

    // for cpy+swap idiom
    void swap( QgsRendererRange &other );
};

typedef QList<QgsRendererRange> QgsRangeList;


/**
 * \ingroup core
 * \class QgsRendererRangeLabelFormat
 * \since QGIS 2.6
 * \deprecated since QGIS 3.10, use QgsClassificationMethod instead
 */
class Q_DECL_DEPRECATED CORE_EXPORT QgsRendererRangeLabelFormat SIP_DEPRECATED
{
  public:
    QgsRendererRangeLabelFormat();
    QgsRendererRangeLabelFormat( const QString &format, int precision = 4, bool trimTrailingZeroes = false );

    bool operator==( const QgsRendererRangeLabelFormat &other ) const;
    bool operator!=( const QgsRendererRangeLabelFormat &other ) const;

    QString format() const { return mFormat; }
    void setFormat( const QString &format ) { mFormat = format; }

    int precision() const { return mPrecision; }
    void setPrecision( int precision );

    bool trimTrailingZeroes() const { return mTrimTrailingZeroes; }
    void setTrimTrailingZeroes( bool trimTrailingZeroes ) { mTrimTrailingZeroes = trimTrailingZeroes; }

    //! \note labelForLowerUpper in Python bindings
    QString labelForRange( double lower, double upper ) const SIP_PYNAME( labelForLowerUpper );
    QString labelForRange( const QgsRendererRange &range ) const;
    QString formatNumber( double value ) const;

    void setFromDomElement( QDomElement &element );
    void saveToDomElement( QDomElement &element );

    static const int MAX_PRECISION;
    static const int MIN_PRECISION;

  protected:
    QString mFormat;
    int mPrecision = 4;
    bool mTrimTrailingZeroes = false;
    // values used to manage number formatting - precision and trailing zeroes
    double mNumberScale = 1.0;
    QString mNumberSuffix;
    QRegExp mReTrailingZeroes;
    QRegExp mReNegativeZero;
};


#endif // QGSRENDERERRANGE_H
