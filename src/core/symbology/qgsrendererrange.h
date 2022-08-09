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

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgssymbollayerutils.h"

#include <QRegularExpression>

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
    ~QgsRendererRange();

    /**
     * Creates a renderer symbol range
     * \param range The classification range
     * \param symbol The symbol for this renderer range
     * \param render If TRUE, it will be renderered
     */
    QgsRendererRange( const QgsClassificationRange &range, QgsSymbol *symbol SIP_TRANSFER, bool render = true );
    QgsRendererRange( double lowerValue, double upperValue, QgsSymbol *symbol SIP_TRANSFER, const QString &label, bool render = true );
    QgsRendererRange( const QgsRendererRange &range );

    // default dtor is OK
    QgsRendererRange &operator=( QgsRendererRange range );

    bool operator<( const QgsRendererRange &other ) const;

    /**
     * Returns the lower bound of the range.
     *
     * \see setLowerValue()
     * \see upperValue()
     */
    double lowerValue() const;

    /**
     * Returns the upper bound of the range.
     *
     * \see setUpperValue()
     * \see lowerValue()
     */
    double upperValue() const;

    /**
     * Returns the symbol used for the range.
     *
     * \see setSymbol()
     */
    QgsSymbol *symbol() const;

    /**
     * Returns the label used for the range.
     *
     * \see setLabel()
     */
    QString label() const;

    /**
     * Sets the symbol used for the range.
     *
     * Ownership of the symbol is transferred.
     *
     * \see symbol()
     */
    void setSymbol( QgsSymbol *s SIP_TRANSFER );

    /**
     * Sets the label used for the range.
     *
     * \see label()
     */
    void setLabel( const QString &label );

    /**
     * Sets the lower bound of the range.
     *
     * \see lowerValue()
     * \see setUpperValue()
     */
    void setLowerValue( double lowerValue );

    /**
     * Sets the upper bound of the range.
     *
     * \see upperValue()
     * \see setLowerValue()
     */
    void setUpperValue( double upperValue );

    /**
     * Returns TRUE if the range should be rendered.
     *
     * \see setRenderState()
     * \since QGIS 2.6
     */
    bool renderState() const;

    /**
     * Sets whether the range should be rendered.
     *
     * \see renderState()
     * \since QGIS 2.6
     */
    void setRenderState( bool render );

    /**
     * Dumps a string representation of the range.
     */
    QString dump() const;

    /**
     * Creates a DOM element representing the range in SLD format.
     * \param doc DOM document
     * \param element destination DOM element
     * \param props graduated renderer properties
     * \param firstRange set to TRUE if the range is the first range, where the lower value uses a <= test
     * rather than a < test.
     */
    void toSld( QDomDocument &doc, QDomElement &element, QVariantMap props, bool firstRange = false ) const;

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    const QString str = sipCpp->label().isEmpty()
                        ? QStringLiteral( "<QgsRendererRange: %1 - %2>" ).arg( sipCpp->lowerValue() ).arg( sipCpp->upperValue() )
                        : QStringLiteral( "<QgsRendererRange: %1 - %2 (%3)>" ).arg( sipCpp->lowerValue() ).arg( sipCpp->upperValue() ).arg( sipCpp->label() );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End

    SIP_PYOBJECT __getitem__( int );
    % MethodCode
    if ( a0 == 0 )
    {
      sipRes = Py_BuildValue( "d", sipCpp->lowerValue() );
    }
    else if ( a0 == 1 )
    {
      sipRes = Py_BuildValue( "d", sipCpp->upperValue() );
    }
    else
    {
      QString msg = QString( "Bad index: %1" ).arg( a0 );
      PyErr_SetString( PyExc_IndexError, msg.toLatin1().constData() );
    }
    % End
#endif

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
class CORE_DEPRECATED_EXPORT QgsRendererRangeLabelFormat SIP_DEPRECATED
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
    QRegularExpression mReTrailingZeroes;
    QRegularExpression mReNegativeZero;
};


#endif // QGSRENDERERRANGE_H
