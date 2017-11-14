/***************************************************************************
  qgsversionmigration.h - QgsVersionMigration

 ---------------------
 begin                : 30.7.2017
 copyright            : (C) 2017 by nathan
 email                : woodrow.nathan at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSVERSIONMIGRATION_H
#define QGSVERSIONMIGRATION_H

#include "qgis_app.h"
#include "qgserror.h"

#include <QSettings>

class QgsSettings;

/**
 * Version migration class used to transfer settings, etc between major versions.
 * \note Not everything can be translated and depends on the from and to versions.
 */
class APP_EXPORT QgsVersionMigration
{
  public:
    QgsVersionMigration() = default;
    virtual ~QgsVersionMigration() = default;

    /**
     * Check if two version has a migration options.
     * @param fromVersion The version migrating from.
     * @param toVersion The version migrating to.
     * @return
     */
    static QgsVersionMigration *canMigrate( int fromVersion, int toVersion );

    /**
     * Run the version migration to convert between versions.
     * @return QgsError containing any error messages when running the conversion.
     */
    virtual QgsError runMigration() = 0;
    virtual bool requiresMigration() = 0;
};


class Qgs2To3Migration : public QgsVersionMigration
{
  public:
    virtual QgsError runMigration() override;
    virtual bool requiresMigration() override;
  private:
    QgsError migrateStyles();
    QgsError migrateSettings();
    QgsError migrateAuthDb();

    QList<QPair<QString, QString>> walk( QString group, QString newkey );
    QPair<QString, QString> transformKey( QString fullOldKey, QString newKeyPart );

    QString migrationFilePath();

    int mMigrationFileVersion;

    QSettings *mOldSettings;

};

#endif // QGSVERSIONMIGRATION_H
