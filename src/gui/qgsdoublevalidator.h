#ifndef QGSDOUBLEVALIDATOR_H
#define QGSDOUBLEVALIDATOR_H

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

#include <limits>
#include <QRegExpValidator>
#include <QLocale>
#include "qgis_gui.h"
#include "qgis_sip.h"

/**
 * \ingroup gui
 *
 * QgsDoubleValidator is a QLineEdit Validator that combines QDoubleValidator
 * and QRegularExpressionValidator to allow user to enter double with both
 * local and C interpretation.
 *
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsDoubleValidator : public QRegularExpressionValidator
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsDoubleValidator.
     * \since QGIS 3.14
     */
    explicit QgsDoubleValidator( QObject *parent );

    /**
     * Constructor for QgsDoubleValidator.
     * \since QGIS 3.14
     */
    QgsDoubleValidator( QRegularExpression reg, double bottom, double top, QObject *parent );

    /**
     * Constructor for QgsDoubleValidator.
     * \since QGIS 3.14
     */
    QgsDoubleValidator( double bottom, double top, QObject *parent );

    /**
     * Constructor for QgsDoubleValidator.
     * \since QGIS 3.14
     */
    QgsDoubleValidator( double bottom, double top, int dec, QObject *parent );

    /**
     * Evaluates input QString validity according to QRegularExpression
     * and ability to be converted in double value.
     * \since QGIS 3.14
     */
    QValidator::State validate( QString &input, int & ) const override;

    /**
     * Evaluates input QString validity according to QRegularExpression
     * and ability to be converted in double value.
     * \since QGIS 3.14
     */
    QValidator::State validate( QString input ) const;

    /**
     * Converts QString to double value.
     * It used locale interpretation first
     * and C locale interpretation as fallback
     * \since QGIS 3.14
     */
    static double toDouble( QString input, bool *ok ) SIP_SKIP;

    /**
     * Converts QString to double value.
     * It used locale interpretation first
     * and C locale interpretation as fallback
     * \since QGIS 3.14
     */
    static double toDouble( QString input );

    /**
     * Set top range limit
     * \see setTop
     * \see setRange
     * \since QGIS 3.14
     */
    void setBottom( double bottom ) { b = bottom; }

    /**
     * Set top range limit
     * \see setBottom
     * \see setRange
     * \since QGIS 3.14
     */
    void setTop( double top ) { t = top; }

    /**
     * Set bottom and top range limits
     * \see setBottom
     * \see setTop
     * \since QGIS 3.14
     */
    virtual void setRange( double bottom, double top )
    {
      b = bottom;
      t = top;
    }

    /**
     * Returns top range limit
     * \see setBottom
     * \since QGIS 3.14
     */
    double bottom() const { return b; }

    /**
     * Returns top range limit
     * \see setTop
     * \since QGIS 3.14
     */
    double top() const { return t; }

  private:
    // Disables copy constructing
    Q_DISABLE_COPY( QgsDoubleValidator )

    /**
     * Bottom range limit
     */
    double b;

    /**
     * Top range limit
     */
    double t;
};

#endif // QGSDOUBLEVALIDATOR_H
