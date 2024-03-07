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
 * \brief Field formatter for a value relation field.
 *
 * A value relation field formatter looks up the values from
 * features on another layer.
 *
 */
class CORE_EXPORT QgsValueRelationFieldFormatter : public QgsFieldFormatter
{
  public:
    struct ValueRelationItem
    {
      //! Constructor for ValueRelationItem
      ValueRelationItem( const QVariant &key, const QString &value, const QString &description = QString(), const QVariant group = QVariant() )
        : key( key )
        , value( value )
        , description( description )
        , group( group )
      {}

      //! Constructor for ValueRelationItem
      ValueRelationItem() = default;

      QVariant key;
      QString value;
      QString description;
      //! Value used to regroup items during sorting (since QGIS 3.38)
      QVariant group;
    };

    typedef QVector < QgsValueRelationFieldFormatter::ValueRelationItem > ValueRelationCache;

    /**
     * Constructor for QgsValueRelationFieldFormatter.
     */
    QgsValueRelationFieldFormatter();

    QString id() const override;
    QString representValue( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config, const QVariant &cache, const QVariant &value ) const override;

    QVariant sortValue( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config, const QVariant &cache, const QVariant &value ) const override;

    QVariant createCache( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config ) const override;

    /**
     * Utility to convert a list or a string representation of an (hstore style: {1,2...}) list in \a value to a string list
     * \since QGIS 3.2
     */
    static QStringList valueToStringList( const QVariant &value );

    /**
     * Create a cache for a value relation field.
     * This can be used to keep the value map in the local memory
     * if doing multiple lookups in a loop.
     * \param config The widget configuration
     * \param formFeature The feature currently being edited with current attribute values
     * \param parentFormFeature For embedded forms only, the feature currently being edited in the parent form with current attribute values
     * \return A kvp list of values for the widget
     *
     */
    static QgsValueRelationFieldFormatter::ValueRelationCache createCache( const QVariantMap &config, const QgsFeature &formFeature = QgsFeature(), const QgsFeature &parentFormFeature = QgsFeature() );

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
     * Check if the \a expression requires a parent form scope (i.e. if it uses fields
     * or geometry of the parent form's currently edited feature).
     *
     * \param expression The widget's filter expression
     * \return TRUE if the expression requires a parent form scope
     * \since QGIS 3.14
     */
    static bool expressionRequiresParentFormScope( const QString &expression );

    /**
     * Returns a list of attributes required by the parent form's form context \a expression
     *
     * \param expression Form filter expression
     * \return list of parent attributes required by the expression
     * \since QGIS 3.14
     */
    static QSet<QString> expressionParentFormAttributes( const QString &expression );

    /**
     * Returns a list of variables required by the parent form's form context \a expression
     *
     * \param expression Form filter expression
     * \return list of parent variables required by the expression
     * \since QGIS 3.14
     */
    static QSet<QString> expressionParentFormVariables( const QString &expression );


    /**
     * Check whether the \a feature has all values required by the \a expression,
     * optionally checks for \a parentFeature
     *
     * \return TRUE if the expression can be used
     * \since QGIS 3.2
     */
    static bool expressionIsUsable( const QString &expression, const QgsFeature &feature, const QgsFeature &parentFeature = QgsFeature() );

    /**
     * Returns the (possibly NULL) layer from the widget's \a config and \a project
     * \since QGIS 3.8
     */
    static QgsVectorLayer *resolveLayer( const QVariantMap &config, const QgsProject *project );

    QList<QgsVectorLayerRef> layerDependencies( const QVariantMap &config ) const override SIP_SKIP;

    QVariantList availableValues( const QVariantMap &config, int countLimit, const QgsFieldFormatterContext &context ) const override;
};

Q_DECLARE_METATYPE( QgsValueRelationFieldFormatter::ValueRelationCache )

#endif // QGSVALUERELATIONFIELDKIT_H
