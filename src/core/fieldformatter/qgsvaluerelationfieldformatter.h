/***************************************************************************
  qgsvaluerelationfieldformatter.h - QgsValueRelationFieldFormatter

 ---------------------
 begin                : 3.12.2016
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
#ifndef QGSVALUERELATIONFIELDKIT_H
#define QGSVALUERELATIONFIELDKIT_H

#include "qgis_core.h"
#include "qgsfieldformatter.h"
#include "qgsexpression.h"
#include "qgsexpressioncontext.h"

#include <QVector>
#include <QVariant>


/**
 * \ingroup core
 * Field formatter for a value relation field.
 * A value relation field formatter looks up the values from
 * features on another layer.
 *
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsValueRelationFieldFormatter : public QgsFieldFormatter
{
  public:
    struct ValueRelationItem
    {
      //! Constructor for ValueRelationItem
      ValueRelationItem( const QVariant &key, const QString &value )
        : key( key )
        , value( value )
      {}

      //! Constructor for ValueRelationItem
      ValueRelationItem() = default;

      QVariant key;
      QString value;
    };

    typedef QVector < QgsValueRelationFieldFormatter::ValueRelationItem > ValueRelationCache;

    /**
     * Constructor for QgsValueRelationFieldFormatter.
     */
    QgsValueRelationFieldFormatter() = default;

    QString id() const override;
    QString representValue( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config, const QVariant &cache, const QVariant &value ) const override;

    QVariant sortValue( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config, const QVariant &cache, const QVariant &value ) const override;

    QVariant createCache( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config ) const override;

    /**
     * Utility to convert an array or a string representation of an array \a value to a string list
     * \since QGIS 3.2
     */
    static QStringList valueToStringList( const QVariant &value );

    /**
     * Create a cache for a value relation field.
     * This can be used to keep the value map in the local memory
     * if doing multiple lookups in a loop.
     * \param config The widget configuration
     * \param formFeature The feature currently being edited with current attribute values
     * \return A kvp list of values for the widget
     *
     * \since QGIS 3.0
     */
    static QgsValueRelationFieldFormatter::ValueRelationCache createCache( const QVariantMap &config, const QgsFeature &formFeature = QgsFeature() );

    /**
     * Check if the \a expression requires a form scope (i.e. if it uses fields
     * or geometry of the currently edited feature).
     *
     * \param expression The widget's filter expression
     * \return TRUE if the expression requires a form scope
     * \since QGIS 3.2
     */
    static bool expressionRequiresFormScope( const QString &expression );

    /**
     * Returns a list of attributes required by the form context \a expression
     *
     * \param expression Form filter expression
     * \return list of attributes required by the expression
     * \since QGIS 3.2
     */
    static QSet<QString> expressionFormAttributes( const QString &expression );

    /**
     * Returns a list of variables required by the form context \a expression
     *
     * \param expression Form filter expression
     * \return list of variables required by the expression
     * \since QGIS 3.2
     */
    static QSet<QString> expressionFormVariables( const QString &expression );

    /**
     * Check whether the \a feature has all values required by the \a expression
     *
     * \return TRUE if the expression can be used
     * \since QGIS 3.2
     */
    static bool expressionIsUsable( const QString &expression, const QgsFeature &feature );

};

Q_DECLARE_METATYPE( QgsValueRelationFieldFormatter::ValueRelationCache )

#endif // QGSVALUERELATIONFIELDKIT_H
