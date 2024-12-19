/***************************************************************************
                         qgsdoublevalidator.h  -  description
                             -------------------
    begin                : June 2020
    copyright            : (C) 2020 by Sebastien Peillet
    email                : sebastien.peillet@oslandia.com

  adapted version of Qgslonglongvalidator + QgsFieldValidator
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSDOUBLEVALIDATOR_H
#define QGSDOUBLEVALIDATOR_H

#include <limits>
#include <QRegularExpressionValidator>
#include <QLocale>
#include "qgis_gui.h"
#include "qgis_sip.h"

class QRegularExpression;

/**
 * \ingroup gui
 *
 * \brief QgsDoubleValidator is a QLineEdit Validator that combines QDoubleValidator
 * and QRegularExpressionValidator to allow users to enter doubles with both
 * local and C interpretation as a fallback.
 *
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsDoubleValidator : public QRegularExpressionValidator
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsDoubleValidator.
     */
    explicit QgsDoubleValidator( QObject *parent );

    /**
     * Constructor for QgsDoubleValidator.
     *
     * \param bottom the minimal range limit accepted by the validator
     * \param top the maximal range limit accepted by the validator
     * \param parent parent object
     * \param expression custom regular expression
     */
    QgsDoubleValidator( const QRegularExpression &expression, double bottom, double top, QObject *parent );

    /**
     * Constructor for QgsDoubleValidator.
     *
     * \param bottom the minimal range limit accepted by the validator
     * \param top the maximal range limit accepted by the validator
     * \param parent parent object
     */
    QgsDoubleValidator( double bottom, double top, QObject *parent );

    /**
     * Constructor for QgsDoubleValidator.
     *
     * \param bottom the minimal range limit accepted by the validator
     * \param top the maximal range limit accepted by the validator
     * \param decimal the number of decimals accepted by the validator
     * \param parent parent object
     */
    QgsDoubleValidator( double bottom, double top, int decimal, QObject *parent );

    /**
     * Constructor for QgsDoubleValidator.
     *
     * \param decimal the number of decimals accepted by the validator
     * \param parent parent object
     * \since QGIS 3.16
     */
    QgsDoubleValidator( int decimal, QObject *parent );

    /**
     * Sets the number of decimals accepted by the validator to \a maxDecimals.
     * \warning setting decimals overrides any custom regular expression that was previously set
     * \since QGIS 3.22
     */
    void setMaxDecimals( int maxDecimals );


    QValidator::State validate( QString &input, int & ) const override SIP_SKIP;

    /**
     * Evaluates \a input string validity according to QRegularExpression
     * and ability to be converted in double value.
     */
    QValidator::State validate( QString &input ) const;

    /**
     * Converts \a input string to double value.
     * It uses locale interpretation first
     * and C locale interpretation as fallback
     */
    static double toDouble( const QString &input, bool *ok ) SIP_SKIP;

    /**
     * Converts \a input string to double value.
     * It uses locale interpretation first
     * and C locale interpretation as fallback
     */
    static double toDouble( const QString &input );

    /**
     * Set top range limit
     * \see setTop
     * \see setRange
     */
    void setBottom( double bottom ) { mMinimum = bottom; }

    /**
     * Set top range limit
     * \see setBottom
     * \see setRange
     */
    void setTop( double top ) { mMaximum = top; }

    /**
     * Set bottom and top range limits
     * \see setBottom
     * \see setTop
     */
    virtual void setRange( double bottom, double top )
    {
      mMinimum = bottom;
      mMaximum = top;
    }

    /**
     * Returns top range limit
     * \see setBottom
     */
    double bottom() const { return mMinimum; }

    /**
     * Returns top range limit
     * \see setTop
     */
    double top() const { return mMaximum; }

  private:
    // Disables copy constructing
    Q_DISABLE_COPY( QgsDoubleValidator )

    /**
     * Bottom range limit
     */
    double mMinimum;

    /**
     * Top range limit
     */
    double mMaximum;
};

#endif // QGSDOUBLEVALIDATOR_H
