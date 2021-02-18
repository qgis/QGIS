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
#include "qgssettingsgroup.h"

/**
 * \ingroup core
 * \class QgsSettingsMap
 *
 * \since QGIS 3.17
 */
template<class T>
class CORE_EXPORT QgsSettingsMap //: public QMap
{
  public:

    /**
     * Constructor for QgsSettingsMap.
     */
    QgsSettingsMap( QString key = QString(),
                    QgsSettingsGroup *parentGroup = nullptr,
                    QString description = QString() )
      : mKey( key )
      , mSettingsGroupParent( parentGroup )
      , mDescription( description )
    {
    }
    virtual ~QgsSettingsMap()
    {
    }

    /**
     * Get settings group key.
     */
    QString key() const
    {
      if ( mSettingsGroupParent == nullptr )
        return mKey;

      return QString( "%1/%2" )
             .arg( mSettingsGroupParent->key() )
             .arg( mKey );
    }

    /**
     * Get settings group description.
     */
    QString description() const
    {
      return mDescription;
    }

  private:

    QString mKey;
    QgsSettingsGroup *mSettingsGroupParent;
    QString mDescription;

};

#endif // QGSSETTINGSMAP_H
