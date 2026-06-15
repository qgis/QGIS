/***************************************************************************
  qgsqgisprofileimporter.h
  ------------------------
  begin                : June 2026
  copyright            : (C) 2026 by Francesco Mazzi
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSQGISPROFILEIMPORTER_H
#define QGSQGISPROFILEIMPORTER_H

#include "qgis_app.h"
#include "qgserror.h"

#include <QList>
#include <QString>
#include <QStringList>

/**
 * \ingroup app
 * \brief Imports existing QGIS user profiles into Strata user profiles.
 *
 * This class is intentionally an app-level helper: it understands desktop
 * profile folders, QGIS settings ini names, and Strata onboarding state.
 */
class APP_EXPORT QgsQgisProfileImporter
{
  public:
    struct Candidate
    {
        QString profileName;
        QString profilePath;
        QString sourceRoot;
        QString sourceVersionLabel;
        int pluginCount = 0;
    };

    struct ImportResult
    {
        QStringList importedProfileNames;
        QString activeProfileName;
        QgsError errors;
    };

    static QList<Candidate> detectCandidates( const QString &configLocalStorageLocation, const QString &targetRootProfileFolder );

    static bool shouldOfferFirstRunImport( const QString &targetRootProfileFolder, const QList<Candidate> &candidates, bool guiAvailable, bool explicitProfile, bool preventSettingsMigration );

    static ImportResult importProfiles( const QList<Candidate> &candidates, const QString &targetRootProfileFolder, const QString &activeProfileHint = QString() );

    static ImportResult importProfileAsNewProfile( const Candidate &candidate, const QString &targetRootProfileFolder, const QString &targetProfileName = QString() );

    static QString preferredActiveProfileName( const QList<Candidate> &candidates );

    static QString uniqueProfileName( const QString &requestedName, const QString &targetRootProfileFolder, const QStringList &reservedNames = QStringList() );

    static void markFirstRunImportDeclined( const QString &targetRootProfileFolder );
};

#endif // QGSQGISPROFILEIMPORTER_H
