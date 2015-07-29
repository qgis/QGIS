/***************************************************************************
    qgsdatadefined.h - Data defined container class
     --------------------------------------
    Date                 : 9-May-2013
    Copyright            : (C) 2013 by Larry Shaffer
    Email                : larrys at dakcarto dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSDATADEFINED_H
#define QGSDATADEFINED_H

#include <QStringList>
#include <QDomElement>
#include <QMap>
#include <QExplicitlySharedDataPointer>
#include "qgis.h"
#include "qgsfield.h"

class QgsExpression;
class QgsVectorLayer;
class QgsDataDefinedPrivate;


/** \ingroup core
 * \class QgsDataDefined
 * A container class for data source field mapping or expression.
 * \note QgsDataDefined objects are implicitly shared.
 */

class CORE_EXPORT QgsDataDefined
{
  public:
    /**
     * Construct a new data defined object
     *
     * @param active Whether the current data defined is active
     * @param useexpr Whether to use expression instead of field
     * @param expr Expression string
     * @param field Field name string
     */
    QgsDataDefined( bool active = false,
                    bool useexpr = false,
                    const QString& expr = QString(),
                    const QString& field = QString() );

    /**
     * Construct a new data defined object, analysing the expression to determine
     * if it's a simple field reference or an expression.
     * @param expression can be null
     */
    explicit QgsDataDefined( const QgsExpression * expression );

    /**
     * Construct a new data defined object, analysing the string to determine
     * if it's a simple field reference or an expression
     * @param string field reference or an expression, can be empty
     * @note added in QGIS 2.9
     */
    explicit QgsDataDefined( const QString& string );

    /**
     * Copy constructor. Note that copies of data defined objects with expressions
     * will not be prepared.
     */
    QgsDataDefined( const QgsDataDefined& other );

    /** Creates a QgsDataDefined from a decoded QgsStringMap.
     * @param map string map encoding of QgsDataDefined
     * @param baseName base name for values in the string map
     * @returns new QgsDataDefined if string map was successfully interpreted
     * @see toMap
     * @note added in QGIS 2.9
     */
    static QgsDataDefined* fromMap( const QgsStringMap& map, const QString& baseName = QString() );

    virtual ~QgsDataDefined();

    /** Returns whether the data defined container is set to all the default
     * values, ie, disabled, with empty expression and no assigned field
     * @returns true if data defined container is set to default values
     * @note added in QGIS 2.7
     */
    bool hasDefaultValues() const;

    bool isActive() const;
    void setActive( bool active );

    bool useExpression() const;
    void setUseExpression( bool use );

    QString expressionString() const;
    void setExpressionString( const QString& expr );

    // @note not available in python bindings
    QMap<QString, QVariant> expressionParams() const;
    // @note not available in python bindings
    void setExpressionParams( QMap<QString, QVariant> params );
    void insertExpressionParam( QString key, QVariant param );

    /** Prepares the expression using a vector layer
     * @param layer vector layer
     * @returns true if expression was successfully prepared
     */
    bool prepareExpression( QgsVectorLayer* layer );

    /** Prepares the expression using a fields collection
     * @param fields
     * @returns true if expression was successfully prepared
     * @note added in QGIS 2.9
     */
    bool prepareExpression( const QgsFields &fields = QgsFields() );

    /** Returns whether the data defined object's expression is prepared
     * @returns true if expression is prepared
     */
    bool expressionIsPrepared() const;

    QgsExpression* expression();

    /** Returns the columns referenced by the QgsDataDefined
     * @param layer vector layer, used for preparing the expression if required
     */
    QStringList referencedColumns( QgsVectorLayer* layer );

    /** Returns the columns referenced by the QgsDataDefined
     * @param fields vector layer, used for preparing the expression if required
     * @note added in QGIS 2.9
     */
    QStringList referencedColumns( const QgsFields& fields = QgsFields() );

    QString field() const;
    void setField( const QString& field );

    /** Encodes the QgsDataDefined into a string map.
     * @param baseName optional base name for values in the string map. Can be used
     * to differentiate multiple QgsDataDefineds encoded in the same string map.
     * @see fromMap
     */
    QgsStringMap toMap( const QString& baseName = QString() ) const;

    /** Returns a DOM element containing the properties of the data defined container.
     * @param document DOM document
     * @param elementName name for DOM element
     * @returns DOM element corresponding to data defined container
     * @note added in QGIS 2.7
     * @see setFromXmlElement
     */
    QDomElement toXmlElement( QDomDocument &document, const QString &elementName ) const;

    /** Sets the properties of the data defined container from an XML element. Calling
     * this will overwrite all the current properties of the container.
     * @param element DOM element
     * @returns true if properties were successfully read from element
     * @note added in QGIS 2.7
     * @see toXmlElement
     */
    bool setFromXmlElement( const QDomElement& element );

    bool operator==( const QgsDataDefined &other ) const;
    bool operator!=( const QgsDataDefined &other ) const;

    /** Assignment operator. Note that after assignment the data defined
     * object's expression will not be prepared.
     */
    QgsDataDefined& operator=( QgsDataDefined const & rhs );

  private:

    QExplicitlySharedDataPointer<QgsDataDefinedPrivate> d;

};

#endif // QGSDATADEFINED_H
