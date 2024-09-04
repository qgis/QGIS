/***************************************************************************
  qgsauthconfigurationstoragesqlite.h - QgsAuthConfigurationStorageSqlite

 ---------------------
 begin                : 20.6.2024
 copyright            : (C) 2024 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSAUTHCONFIGURATIONSTORAGESQLITE_H
#define QGSAUTHCONFIGURATIONSTORAGESQLITE_H

///@cond PRIVATE

#define SIP_NO_FILE

#include "qgsauthconfigurationstoragedb.h"

#include <QObject>
#include <QRecursiveMutex>
#include <QSqlDatabase>

/**
 * This class is not part of the API: is is just a thin convenience wrapper
 * around a QSQLITE implementation of a QgsAuthConfigurationStorageDb.
 */
class QgsAuthConfigurationStorageSqlite : public QgsAuthConfigurationStorageDb
{
    Q_OBJECT

    //  QgsAuthConfigurationStorageDb interface
  public:

    QgsAuthConfigurationStorageSqlite( const QString &databasePath );

    bool initialize() override;
    QList<QgsAuthConfigurationStorage::SettingParameter> settingsParameters() const override;
    QString description() const override;
    QString type( ) const override;

  private:

    bool tableExists( const QString &table ) const override;
    void checkCapabilities() override;

};
/// @endcond

#endif // QGSAUTHCONFIGURATIONSTORAGESQLITE_H
