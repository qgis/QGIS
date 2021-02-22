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

#ifndef QGSSETTINGSGROUP_H
#define QGSSETTINGSGROUP_H

#include <QString>
#include <QMap>
#include <limits>

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgssettings.h"

/**
 * \ingroup core
 * \class QgsSettingsGroup
 *
 * \since QGIS 3.18
 */
class CORE_EXPORT QgsSettingsGroup
{
  public:

    /**
     * Constructor for QgsSettingsGroup.
     */
    QgsSettingsGroup( QString key = QString(),
                      QgsSettingsGroup *parentGroup = nullptr,
                      QString description = QString() );

    /**
     * Constructor for QgsSettingsGroup.
     */
    QgsSettingsGroup( QString key,
                      QgsSettings::Section section,
                      QString description = QString() );

    /**
     * Destructor for QgsSettingsGroup.
     */
    virtual ~QgsSettingsGroup();

    /**
     * Set settings group key.
     */
    void setKey( const QString &key );

    /**
     * Get settings group key.
     */
    QString key() const;

    /**
     * Get settings group description.
     */
    QString description() const;

  private:

    QString mKey;
    QgsSettings::Section mSection;
    QgsSettingsGroup *mSettingsGroupParent;
    QString mDescription;

};

#endif // QGSSETTINGSGROUP_H
