/***************************************************************************
    qgssensorthingsutils.h
    --------------------
    begin                : November 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSSENSORTHINGSUTILS_H
#define QGSSENSORTHINGSUTILS_H

#include "qgis_sip.h"
#include "qgis_core.h"
#include "qgis.h"

class QgsFields;
class QgsFeedback;
class QgsRectangle;
class QgsSensorThingsExpansionDefinition;

/**
 * \ingroup core
 * \brief Utility functions for working with OGC SensorThings API services.
 *
 * \since QGIS 3.36
 */
class CORE_EXPORT QgsSensorThingsUtils
{

  public:

    //! Default page size
    static constexpr int DEFAULT_PAGE_SIZE = 200; SIP_SKIP

    //! Default limit on number of features fetched
    static constexpr int DEFAULT_FEATURE_LIMIT = 10000; SIP_SKIP

    //! Default limit on number of expanded features fetched
    static constexpr int DEFAULT_EXPANSION_LIMIT = 100; SIP_SKIP

    /**
     * Converts a string value to a Qgis::SensorThingsEntity type.
     *
     * Returns Qgis::SensorThingsEntity::Invalid if the string could not be converted to a known entity type.
     */
    static Qgis::SensorThingsEntity stringToEntity( const QString &type );

    /**
     * Converts a Qgis::SensorThingsEntity \a type to a user-friendly translated string.
     *
     * If \a plural is TRUE then a plural string is returned (ie "Things" instead of "Thing").
    */
    static QString displayString( Qgis::SensorThingsEntity type, bool plural = false );

    /**
     * Converts a string value corresponding to a SensorThings entity set to a Qgis::SensorThingsEntity type.
     *
     * Returns Qgis::SensorThingsEntity::Invalid if the string could not be converted to a known entity set type.
    */
    static Qgis::SensorThingsEntity entitySetStringToEntity( const QString &type );

    /**
     * Converts a SensorThings entity set to a SensorThings entity set string.
     *
     * \since QGIS 3.38
    */
    static QString entityToSetString( Qgis::SensorThingsEntity type );

    /**
     * Returns the SensorThings properties which correspond to a specified entity \a type.
     *
     * \since QGIS 3.38
     */
    static QStringList propertiesForEntityType( Qgis::SensorThingsEntity type );

    /**
     * Returns the fields which correspond to a specified entity \a type.
     */
    static QgsFields fieldsForEntityType( Qgis::SensorThingsEntity type );

    /**
     * Returns the fields which correspond to a specified entity \a baseType, expanded
     * using the specified list of \a expandedTypes.
     *
     * \since QGIS 3.38
     */
    static QgsFields fieldsForExpandedEntityType( Qgis::SensorThingsEntity baseType, const QList< Qgis::SensorThingsEntity > &expandedTypes );

    /**
     * Returns the geometry field for a specified entity \a type.
     */
    static QString geometryFieldForEntityType( Qgis::SensorThingsEntity type );

    /**
     * Returns TRUE if the specified entity \a type can have geometry attached.
     */
    static bool entityTypeHasGeometry( Qgis::SensorThingsEntity type );

    /**
     * Returns the geometry type for if the specified entity \a type.
     *
     * If there are no restrictions on the geometry type an ntity can have Qgis::GeometryType::Unknown will be returned.
     *
     * \since QGIS 3.38
     */
    static Qgis::GeometryType geometryTypeForEntity( Qgis::SensorThingsEntity type );

    /**
     * Returns a filter string which restricts results to those matching the specified
     * \a entityType and \a wkbType.
     */
    static QString filterForWkbType( Qgis::SensorThingsEntity entityType, Qgis::WkbType wkbType );

    /**
     * Returns a filter string which restricts results to those within the specified
     * \a extent.
     *
     * The \a extent should always be specified in EPSG:4326.
     *
     * \since QGIS 3.38
     */
    static QString filterForExtent( const QString &geometryField, const QgsRectangle &extent );

    /**
     * Combines a set of SensorThings API filter operators.
     *
     * See https://docs.ogc.org/is/18-088/18-088.html#requirement-request-data-filter
     *
     * \since QGIS 3.38
     */
    static QString combineFilters( const QStringList &filters );

    /**
     * Returns a list of available geometry types for the server at the specified \a uri
     * and entity \a type.
     *
     * This method will block while network requests are made to the server.
     */
    static QList< Qgis::GeometryType > availableGeometryTypes( const QString &uri, Qgis::SensorThingsEntity type, QgsFeedback *feedback = nullptr, const QString &authCfg = QString() );

    /**
     * Returns a list of permissible expand targets for a given base entity \a type.
     *
     * \since QGIS 3.38
     */
    static QList< Qgis::SensorThingsEntity > expandableTargets( Qgis::SensorThingsEntity type );

    /**
     * Returns the cardinality of the relationship between a base entity type and a related entity type.
     *
     * \param baseType base entity type
     * \param relatedType related entity type
     * \param valid will be set to TRUE if a relationship exists between the entity types, or FALSE if no relationship exists
     *
     * \returns relationship cardinality
     *
     * \since QGIS 3.38
     */
    static Qgis::RelationshipCardinality relationshipCardinality( Qgis::SensorThingsEntity baseType, Qgis::SensorThingsEntity relatedType, bool &valid SIP_OUT );

    /**
     * Returns a list of \a expansions as a valid SensorThings API query string, eg "$expand=Locations($orderby=id desc;$top=3;$expand=Datastreams($top=101))".
     *
     * The base entity type for the query must be specified.
     *
     * \since QGIS 3.38
     */
    static QString asQueryString( Qgis::SensorThingsEntity baseType, const QList< QgsSensorThingsExpansionDefinition > &expansions );

};


/**
 * \ingroup core
 * \brief Encapsulates information about how relationships in a SensorThings API service should be expanded.
 *
 * \since QGIS 3.38
 */
class CORE_EXPORT QgsSensorThingsExpansionDefinition
{
  public:

    /**
     * Constructor for QgsSensorThingsExpansionDefinition, targeting the specified child entity type.
     */
    QgsSensorThingsExpansionDefinition( Qgis::SensorThingsEntity childEntity = Qgis::SensorThingsEntity::Invalid,
                                        const QString &orderBy = QString(),
                                        Qt::SortOrder sortOrder = Qt::SortOrder::AscendingOrder,
                                        int limit = QgsSensorThingsUtils::DEFAULT_EXPANSION_LIMIT,
                                        const QString &filter = QString() );

    /**
     * Returns an expansion definition for the specified \a entity type, populated with reasonable
     * defaults which make sense for that entity type.
     */
    static QgsSensorThingsExpansionDefinition defaultDefinitionForEntity( Qgis::SensorThingsEntity entity );

    /**
     * Returns TRUE if the definition is valid.
     */
    bool isValid() const;

    /**
     * Returns the target child entity which should be expanded.
     *
     * \see setChildEntity()
     */
    Qgis::SensorThingsEntity childEntity() const;

    /**
    * Sets the target child \a entity which should be expanded.
    *
    * \see childEntity()
    */
    void setChildEntity( Qgis::SensorThingsEntity entity );

    /**
     * Returns the field name to order the expanded child entities by.
     *
     * \see sortOrder()
     * \see setOrderBy()
     */
    QString orderBy() const;

    /**
     * Sets the \a field name to order the expanded child entities by.
     *
     * \see orderBy()
     * \see setSortOrder()
     */
    void setOrderBy( const QString &field );

    /**
     * Returns the sort order for the expanded child entities.
     *
     * \see orderBy()
     * \see setSortOrder()
     */
    Qt::SortOrder sortOrder() const;

    /**
     * Sets the sort order for the expanded child entities.
     *
     * \see setOrderBy()
     * \see sortOrder()
     */
    void setSortOrder( Qt::SortOrder order );

    /**
     * Returns the limit on the number of child features to fetch.
     *
     * Returns -1 if no limit is defined.
     *
     * \see setLimit()
     */
    int limit() const;

    /**
     * Sets the \a limit on the number of child features to fetch.
     *
     * Set to -1 if no limit is desired.
     *
     * \see limit()
     */
    void setLimit( int limit );

    /**
     * Returns the the string filter to filter expanded child entities by.
     *
     * \see setFilter()
     */
    QString filter() const;

    /**
     * Returns the the string \a filter to filter expanded child entities by.
     *
     * \see filter()
     */
    void setFilter( const QString &filter );

    /**
     * Returns a string encapsulation of the expansion definition.
     *
     * \see fromString()
     */
    QString toString() const;

    /**
     * Returns a QgsSensorThingsExpansionDefinition from a string representation.
     *
     * \see toString()
     */
    static QgsSensorThingsExpansionDefinition fromString( const QString &string );

    /**
     * Returns the expansion as a valid SensorThings API query string, eg "$expand=Observations($orderby=phenomenonTime desc;$top=10)".
     *
     * The parent entity type for the expansion must be specified.
     *
     * Optionally a list of additional query options can be specified for the expansion.
     */
    QString asQueryString( Qgis::SensorThingsEntity parentEntityType, const QStringList &additionalOptions = QStringList() ) const;

    bool operator==( const QgsSensorThingsExpansionDefinition &other ) const;
    bool operator!=( const QgsSensorThingsExpansionDefinition &other ) const;

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    if ( !sipCpp->isValid() )
    {
      sipRes = PyUnicode_FromString( "<QgsSensorThingsExpansionDefinition: invalid>" );
    }
    else
    {
      QString innerDefinition;
      if ( !sipCpp->orderBy().isEmpty() )
      {
        innerDefinition = QStringLiteral( "by %1 (%2)" ).arg( sipCpp->orderBy(), sipCpp->sortOrder() == Qt::SortOrder::AscendingOrder ? QStringLiteral( "asc" ) : QStringLiteral( "desc" ) );
      }
      if ( sipCpp->limit() >= 0 )
      {
        if ( !innerDefinition.isEmpty() )
          innerDefinition = QStringLiteral( "%1, limit %2" ).arg( innerDefinition ).arg( sipCpp->limit() );
        else
          innerDefinition = QStringLiteral( "limit %1" ).arg( sipCpp->limit() );
      }
      if ( !sipCpp->filter().isEmpty() )
      {
        if ( !innerDefinition.isEmpty() )
          innerDefinition = QStringLiteral( "%1, filter '%2'" ).arg( innerDefinition ).arg( sipCpp->filter() );
        else
          innerDefinition = QStringLiteral( "filter '%1'" ).arg( sipCpp->filter() );
      }

      QString str = QStringLiteral( "<QgsSensorThingsExpansionDefinition: %1%2>" ).arg( qgsEnumValueToKey( sipCpp->childEntity() ), innerDefinition.isEmpty() ? QString() : ( QStringLiteral( " " ) + innerDefinition ) );
      sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    }
    % End
#endif

  private:

    Qgis::SensorThingsEntity mChildEntity = Qgis::SensorThingsEntity::Invalid;
    QString mOrderBy;
    Qt::SortOrder mSortOrder = Qt::SortOrder::AscendingOrder;
    int mLimit = QgsSensorThingsUtils::DEFAULT_EXPANSION_LIMIT;
    QString mFilter;

};
Q_DECLARE_METATYPE( QgsSensorThingsExpansionDefinition )

#endif // QGSSENSORTHINGSUTILS_H
