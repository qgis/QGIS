/***************************************************************************
  qgssettingsgroup.h
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

#ifndef QGSSETTINGSGROUP_H
#define QGSSETTINGSGROUP_H

#include <QString>
#include <QMap>
#include <limits>

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgssettings.h"

class QgsSettingsEntry;

/**
 * \ingroup core
 * \class QgsSettingsGroup
 *
 * Represent a group of settings and can be the parent of other settings groups or settings
 * entries.
 *
 * \since QGIS 3.18
 */
class CORE_EXPORT QgsSettingsGroup
{
  public:

    /**
     * Constructor for QgsSettingsGroup.
     *
     * The \a key argument specifies a part of the settings key.
     * The \a parentGroup argument specifies a parent group which is used to rebuild
     * the entiere settings key and to determine the settings section.
     * The \a description argument specifies a description for the settings group.
     */
    QgsSettingsGroup( QString key = QString(),
                      QgsSettingsGroup *parentGroup = nullptr,
                      QString description = QString() );

    /**
     * Constructor for QgsSettingsGroup.
     *
     * The \a key argument specifies a part of the settings key.
     * The \a section argument specifies settings section for this group and for children groups
     * and settings entries.
     * The \a description argument specifies a description for the settings group.
     */
    QgsSettingsGroup( QgsSettings::Section section,
                      QString description = QString() );

    QgsSettingsGroup *parentSettingsGroup() const;

    void setParentSettingsGroup( QgsSettingsGroup *parentSettingsGroup );

    QList<QgsSettingsGroup *> childSettingsGroups() const;

    QList<QgsSettingsEntry *> childSettingsEntries() const;

    /**
     * Set settings group key.
     */
    void setKey( const QString &key );

    /**
     * Get settings group key. The returned key is composed of this group key plus parent keys.
     */
    QString key() const;

    /**
     * Get settings group section. The settings section of the parent group is returned if available.
     */
    QgsSettings::Section section();

    /**
     * Get settings group description.
     */
    QString description() const;

    /**
     * Register \a childSettingsGroup to this settings group to enable instrospection capabilities.
     */
    void registerChildSettingsGroup( QgsSettingsGroup *childSettingsGroup );

    /**
     * Register \a childSettingsEntry to this settings group to enable instrospection capabilities.
     */
    void registerChildSettingsEntry( QgsSettingsEntry *childSettingsEntry );

  private:

    QString mKey;
    QgsSettings::Section mSection;
    QgsSettingsGroup *mSettingsGroupParent;
    QString mDescription;

    QList<QgsSettingsGroup *> mChildSettingsGroups;
    QList<QgsSettingsEntry *> mChildSettingsEntries;

};

#endif // QGSSETTINGSGROUP_H
