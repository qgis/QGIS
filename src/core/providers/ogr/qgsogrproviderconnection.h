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
    void createVectorTable( const QString &schema, const QString &name, const QgsFields &fields, QgsWkbTypes::Type wkbType, const QgsCoordinateReferenceSystem &srs, bool overwrite, const QMap<QString, QVariant> *options ) const override;
    void dropVectorTable( const QString &schema, const QString &name ) const override;
    QList<QgsVectorDataProvider::NativeType> nativeTypes() const override;
    QStringList fieldDomainNames() const override;
    QgsFieldDomain *fieldDomain( const QString &name ) const override;
    void setFieldDomainName( const QString &fieldName, const QString &schema, const QString &tableName, const QString &domainName ) const override;
    void addFieldDomain( const QgsFieldDomain &domain, const QString &schema ) const override;

  protected:

    void setDefaultCapabilities();

};



///@endcond
#endif // QGSOGRPROVIDERCONNECTION_H
