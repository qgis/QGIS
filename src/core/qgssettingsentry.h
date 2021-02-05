/***************************************************************************
  qgssettingsentry.h
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

#ifndef QGSSETTINGSENTRY_H
#define QGSSETTINGSENTRY_H

#include <QString>

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgssettings.h"

/**
 * \ingroup core
 * \class QgsSettingsEntry
 *
 * \since QGIS 3.17
 */
class CORE_EXPORT QgsSettingsEntry : public QObject
{
    Q_OBJECT
  public:

    /**
     * Constructor for QgsSettingsEntry.
     */
    QgsSettingsEntry( QString settingsName = QString(),
                      QgsSettings::Section settingsSection = QgsSettings::NoSection,
                      QVariant defaultValue = QVariant(),
                      QString description = QString(),
                      QObject *parent = nullptr );

    QgsSettingsEntry( const QgsSettingsEntry &other );

    QgsSettingsEntry &operator=( const QgsSettingsEntry &other );

    void setValue( const QVariant &value );
    QVariant value() const;

  private:

    QString mSettingsName;
    QVariant mDefaultValue;
    QgsSettings::Section mSettingsSection;
    QString mDescription;
};

#endif // QGSSETTINGSENTRY_H
