/***************************************************************************
  qgsogrproviderconnection.h

 ---------------------
 begin                : 6.8.2019
 copyright            : (C) 2019 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSOGRPROVIDERCONNECTION_H
#define QGSOGRPROVIDERCONNECTION_H

#include "qgsabstractdatabaseproviderconnection.h"
#include "qgsogrutils.h"

///@cond PRIVATE
#define SIP_NO_FILE



struct QgsOgrProviderResultIterator: public QgsAbstractDatabaseProviderConnection::QueryResult::QueryResultIterator
{

    QgsOgrProviderResultIterator( gdal::ogr_datasource_unique_ptr hDS, OGRLayerH ogrLayer );

    ~QgsOgrProviderResultIterator();

    void setFields( const QgsFields &fields );
    void setGeometryColumnName( const QString &geometryColumnName );
    void setPrimaryKeyColumnName( const QString &primaryKeyColumnName );

  private:

    gdal::ogr_datasource_unique_ptr mHDS;
    OGRLayerH mOgrLayer;
    QgsFields mFields;
    QVariantList mNextRow;
    QString mGeometryColumnName;
    QString mPrimaryKeyColumnName;
    long long mRowCount = -1;

    QVariantList nextRowPrivate() override;
    bool hasNextRowPrivate() const override;
    long long rowCountPrivate() const override;
    QVariantList nextRowInternal();

};


/**
 * \ingroup core
 * \class QgsOgrProviderConnection
 *
 * \brief Base class for provider connections handled by OGR.
 *
 * This class is specialized in format-specific subclasses such as QgsGeoPackageProviderConnection
 */
class QgsOgrProviderConnection : public QgsAbstractDatabaseProviderConnection
{
  public:

    QgsOgrProviderConnection( const QString &name );
    QgsOgrProviderConnection( const QString &uri, const QVariantMap &configuration );

    // QgsAbstractProviderConnection interface
  public:
    void store( const QString &name ) const override;
    void remove( const QString &name ) const override;
    QString tableUri( const QString &schema, const QString &name ) const override;
    QList<QgsAbstractDatabaseProviderConnection::TableProperty> tables( const QString &schema = QString(),
        const TableFlags &flags = TableFlags() ) const override;
    QgsAbstractDatabaseProviderConnection::TableProperty table( const QString &schema, const QString &table ) const override;
    QueryResult execSql( const QString &sql, QgsFeedback *feedback = nullptr ) const override;
    QgsVectorLayer *createSqlVectorLayer( const SqlVectorLayerOptions &options ) const override;
    void createVectorTable( const QString &schema, const QString &name, const QgsFields &fields, QgsWkbTypes::Type wkbType, const QgsCoordinateReferenceSystem &srs, bool overwrite, const QMap<QString, QVariant> *options ) const override;
    void dropVectorTable( const QString &schema, const QString &name ) const override;
    void vacuum( const QString &schema, const QString &name ) const override;
    QList<QgsVectorDataProvider::NativeType> nativeTypes() const override;
    QStringList fieldDomainNames() const override;
    QList< Qgis::FieldDomainType > supportedFieldDomainTypes() const override;
    QgsFieldDomain *fieldDomain( const QString &name ) const override;
    void setFieldDomainName( const QString &fieldName, const QString &schema, const QString &tableName, const QString &domainName ) const override;
    void addFieldDomain( const QgsFieldDomain &domain, const QString &schema ) const override;
    void renameField( const QString &schema, const QString &tableName, const QString &name, const QString &newName ) const override;
    SqlVectorLayerOptions sqlOptions( const QString &layerSource ) override;
    QList< Qgis::RelationshipCardinality > supportedRelationshipCardinalities() const override;
    QList< Qgis::RelationshipStrength > supportedRelationshipStrengths() const override;
    Qgis::RelationshipCapabilities supportedRelationshipCapabilities() const override;
    QStringList relatedTableTypes() const override;
    QList< QgsWeakRelation > relationships( const QString &schema = QString(), const QString &tableName = QString() ) const override;
    void addRelationship( const QgsWeakRelation &relationship ) const override;
    void updateRelationship( const QgsWeakRelation &relationship ) const override;
    void deleteRelationship( const QgsWeakRelation &relationship ) const override;

  protected:

    void setDefaultCapabilities();

    virtual QString databaseQueryLogIdentifier() const;

    virtual QString primaryKeyColumnName( const QString &table ) const;

    //! Use GDAL to execute SQL
    QueryResult executeGdalSqlPrivate( const QString &sql, QgsFeedback *feedback = nullptr ) const;

  private:
    QString mDriverName;
    bool mSingleTableDataset = false;
    QList< Qgis::RelationshipCardinality > mSupportedRelationshipCardinality;
    QList< Qgis::RelationshipStrength > mSupportedRelationshipStrength;
    Qgis::RelationshipCapabilities mRelationshipCapabilities;
    QStringList mRelatedTableTypes;
};



///@endcond
#endif // QGSOGRPROVIDERCONNECTION_H
