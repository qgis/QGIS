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
 * \since QGIS 3.18
 */
template<typename T>
class CORE_EXPORT QgsSettingsGroupMap : public QgsSettingsGroup
{
  public:

    /**
     * Constructor for QgsSettingsGroupMap.
     */
    QgsSettingsGroupMap( QString key = QString(),
                         QgsSettingsGroup *parentGroup = nullptr,
                         QString description = QString() )
      : QgsSettingsGroup( key, parentGroup, description )
    {
    }

    /**
     * Constructor for QgsSettingsGroupMap.
     */
    QgsSettingsGroupMap( QString key,
                         QgsSettings::Section section,
                         QString description = QString() )
      : QgsSettingsGroup( key, section, description )
    {
    }

    virtual ~QgsSettingsGroupMap()
    {
    }

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
