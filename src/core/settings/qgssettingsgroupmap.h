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

#ifndef QGSSETTINGSGROUPMAP_H
#define QGSSETTINGSGROUPMAP_H

#include <QString>
#include <QMap>
#include <limits>

#include "qgis_core.h"
#include "qgis_sip.h"

#include "qgssettingsgroup.h"

/**
 * \ingroup core
 * \class QgsSettingsGroupMap
 *
 * Represent a map of custom group of settings and it is the parent of inserted settings groups.
 * Template type T must be a subclass of QgsSettingsGroup
 *
 * \since QGIS 3.18
 */
template<typename T>
class CORE_EXPORT QgsSettingsGroupMap : public QgsSettingsGroup
{
  public:

    /**
     * Constructor for QgsSettingsGroupMap.
     *
     * The \a key argument specifies a part of the settings key.
     * The \a parentGroup argument specifies a parent group which is used to rebuild
     * the entiere settings key and to determine the settings section.
     * The \a description argument specifies a description for the settings group map.
     */
    QgsSettingsGroupMap( QString key = QString(),
                         QgsSettingsGroup *parentGroup = nullptr,
                         QString description = QString() )
      : QgsSettingsGroup( key, parentGroup, description )
    {
    }

    /**
     * Constructor for QgsSettingsGroupMap.
     *
     * The \a key argument specifies a part of the settings key.
     * The \a section argument specifies settings section for this group map and for children groups
     * and settings entries.
     * The \a description argument specifies a description for the settings group map.
     */
    QgsSettingsGroupMap( QString key,
                         QgsSettings::Section section,
                         QString description = QString() )
      : QgsSettingsGroup( key, section, description )
    {
    }

    /**
     * Get a settings group by key. If the key does not exist a new element is inserted in to the map.
     */
    T &operator[]( const QString &key )
    {
      if ( mMapKeySettingsGroup.contains( key ) == false )
      {
        T group( this );
        group.setKey( key );
        mMapKeySettingsGroup.insert( key, group );
      }

      return mMapKeySettingsGroup[ key ];
    }

  private:

    QMap<QString, T> mMapKeySettingsGroup;

};

#endif // QGSSETTINGSGROUPMAP_H
