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

#ifndef QGSSETTINGSMAP_H
#define QGSSETTINGSMAP_H

#include <QString>
#include <QMap>
#include <limits>

#include "qgis_core.h"
#include "qgis_sip.h"

#include "qgssettingsgroup.h"

/**
 * \ingroup core
 * \class QgsSettingsMap
 *
 * \since QGIS 3.18
 */
template<typename T>
class CORE_EXPORT QgsSettingsMap : public QgsSettingsGroup
{
  public:

    /**
     * Constructor for QgsSettingsMap.
     */
    QgsSettingsMap( QString key = QString(),
                    QgsSettingsGroup *parentGroup = nullptr,
                    QString description = QString(),
                    bool createElementsOnAccess = true )
      : QgsSettingsGroup( key, parentGroup, description )
    {
    }

    virtual ~QgsSettingsMap()
    {
    }

    T &operator[]( const QString &key )
    {
      return mMapKeySettingsGroup[ key ];
    }

  private:

    QMap<QString, T> mMapKeySettingsGroup;

    bool mCreateElementsOnAccess;

};

#endif // QGSSETTINGSMAP_H
