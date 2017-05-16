/***************************************************************************
  qgsoptionalexpression.h - QgsOptionalExpression

 ---------------------
 begin                : 8.9.2016
 copyright            : (C) 2016 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSOPTIONALEXPRESSION_H
#define QGSOPTIONALEXPRESSION_H

#include "qgsoptional.h"
#include "qgsexpression.h"

#include "qgis_core.h"

/**
 * \ingroup core
 *
 * An expression with an additional enabled flag.
 *
 * This can be used for configuration options where an expression can be enabled
 * or disabled but when disabled it shouldn't lose it's information for the case
 * it gets re-enabled later on and a user shouldn't be force to redo the configuration.
 *
 * \since QGIS 2.18
 */

class CORE_EXPORT QgsOptionalExpression : public QgsOptional<QgsExpression>
{
  public:

    /**
     * Construct a default optional expression.
     * It will be disabled and with an empty expression.
     */
    QgsOptionalExpression();

    /**
     * Construct an optional expression with the provided expression.
     * It will be enabled.
     */
    QgsOptionalExpression( const QgsExpression &expression );

    /**
     * Construct an optional expression with the provided expression and enabled state.
     */
    QgsOptionalExpression( const QgsExpression &expression, bool enabled );


    // SIP does not handle properly template class Inheritance at the moment
    // following is copied from QgsOptional header
#ifdef SIP_RUN

    /**
     * Compare this QgsOptionalExpression to another one.
     *
     * This will compare the enabled flag and call the == operator
     * of the contained class.
     *
     * @note Added in QGIS 3.0
     */
    int operator== ( const QgsOptionalExpression &other ) const;
    % MethodCode
    sipRes = *sipCpp == *a0;
    % End


    int __bool__() const;
    % MethodCode
    sipRes = sipCpp->enabled();
    % End

    /**
     * Check if this optional is enabled
     *
     * \since QGIS 3.0
     */
    bool enabled() const;

    /**
     * Set if this optional is enabled
     *
     * \since QGIS 3.0
     */
    void setEnabled( bool enabled );

    /**
     * Access the payload data
     *
     * \since QGIS 3.0
     */
    QgsExpression data() const;

    /**
     * Set the payload data
     *
     * \since QGIS 3.0
     */
    void setData( const QgsExpression &data );
#endif

    /**
     * Save the optional expression to the provided QDomElement.
     *
     * The caller is responsible to pass an empty QDomElement and to append it to
     * a parent element.
     *
     * \since QGIS 2.18
     */
    void writeXml( QDomElement &element );

    /**
     * Read the optional expression from the provided QDomElement.
     *
     * \since QGIS 2.18
     */
    void readXml( const QDomElement &element );
};


#if defined(Q_OS_WIN)
template CORE_EXPORT QgsOptional<QgsExpression>;
#endif


#endif // QGSOPTIONALEXPRESSION_H
