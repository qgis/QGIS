/***************************************************************************
  qgssettingsregistry.h
  --------------------------------------
  Date                 : February 2021
  Copyright            : (C) 2021 by Damiano Lombardi
  Email                : damiano at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSSETTINGSREGISTRY_H
#define QGSSETTINGSREGISTRY_H

#include <QString>

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgssettings.h"
#include "qgssettingsentry.h"

/**
 * \ingroup core
 * \class QgsSettingsRegistry
 *
 * \since QGIS 3.17
 */
class CORE_EXPORT QgsSettingsRegistry : public QObject
{
    Q_OBJECT
  public:

    /**
     * Constructor for QgsSettingsRegistry.
     */
    QgsSettingsRegistry( QgsSettings::Section settingsSection,
                         QObject *parent = nullptr );

    void registerValue( const QString &settingsName,
                        QVariant::Type type,
                        const QVariant &defaultValue = QVariant(),
                        const QString &description = QString() );

    void unregister( const QString &settingsName );

    bool setValue( const QString &settingsName,
                   const QVariant &value );

    QVariant value( const QString &settingsName ) const;

  private:

    QgsSettings::Section mSettingsSection;

    QMap<QString, QgsSettingsEntry> mMapSettingsEntry;
};

#endif // QGSSETTINGSREGISTRY_H
