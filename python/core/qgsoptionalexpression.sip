/***************************************************************************
  qgsoptionalexpression.sip - QgsOptionalExpression

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

/**
 * \ingroup core
 *
 * QgsOptionalExpression is a container for an expression with an additional enabled/disabled flag.
 *
 * @note Added in QGIS 3.0
 */
class QgsOptionalExpression
{
%TypeHeaderCode
#include <qgsoptionalexpression.h>
%End
  public:
    /**
     * A QgsOptionalExpression is disabled by default if default constructed.
     */
    QgsOptionalExpression();

    /**
     * A QgsOptionalExpression is enabled by default if constructed with an expression.
     */
    QgsOptionalExpression( const QgsExpression& data );

    /**
     * A QgsOptionalExptression constructed with enabled status and data
     */
    QgsOptionalExpression( const QgsExpression& data, bool enabled );

    /**
     * Compare this QgsOptionalExptression to another one.
     *
     * This will compare the enabled flag and call the == operator
     * of the contained class.
     *
     * @note Added in QGIS 3.0
     */
    bool operator== ( const QgsOptionalExpression& other ) const;
    operator bool () const;

    /**
     * Check if this optional is enabled
     *
     * @note Added in QGIS 3.0
     */
    bool enabled() const;

    /**
     * Set if this optional is enabled
     *
     * @note Added in QGIS 3.0
     */
    void setEnabled( bool enabled );

    /**
     * Access the payload data
     *
     * @note Added in QGIS 3.0
     */
    QgsExpression data() const;

    /**
     * Set the payload data
     *
     * @note Added in QGIS 3.0
     */
    void setData( const QgsExpression& data );

    /**
     * Save the optional expression to the provided QDomElement.
     *
     * The caller is responsible to pass an empty QDomElement and to append it to
     * a parent element.
     *
     * @note Added in QGIS 2.18
     */
    void writeXml(QDomElement& parent );

    /**
     * Read the optional expression from the provided QDomElement.
     *
     * @note Added in QGIS 2.18
     */
    void readXml(const QDomElement& parent );
};
