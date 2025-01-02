/***************************************************************************
  qgssettingsentrygroup.h
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

#ifndef QGSSETTINGSENTRYGROUP_H
#define QGSSETTINGSENTRYGROUP_H

#include <QString>
#include <QColor>
#include <limits>

#include "qgis.h"
#include "qgis_core.h"
#include "qgis_sip.h"

class QgsSettingsEntryBase;


/**
 * \ingroup core
 * \class QgsSettingsEntryGroup
 * \brief Creates a group of setting which have a common definition of base key
 *
 * \since QGIS 3.26
 * \deprecated QGIS 3.30. Use QgsSettingsTreeNode instead.
 */
class CORE_DEPRECATED_EXPORT QgsSettingsEntryGroup SIP_DEPRECATED
{
  public:
    //! Constructor
    QgsSettingsEntryGroup( QList<const QgsSettingsEntryBase *> settings );
#ifdef SIP_RUN
    % MethodCode
    sipCpp = new QgsSettingsEntryGroup( *a0, false );
    sipIsErr = sipCpp->isValid() ? 0 : 1;
    if ( sipIsErr )
      PyErr_SetString( PyExc_ValueError, QStringLiteral( "Settings do not share the same base definition key for this group. This will lead to unpredictable results." ).toUtf8().constData() );
    % End
#endif

    //! Constructor
    QgsSettingsEntryGroup( QList<const QgsSettingsEntryBase *> settings, bool fatalErrorIfInvalid ) SIP_SKIP;

    //! Returns if the group is valid (if settings share the same base key)
    bool isValid() const {return mIsValid;}

    //! Returns the base key for the given \a dynamicKeyPartList
    QString baseKey( const QStringList &dynamicKeyPartList = QStringList() ) const;

    //! Returns all the settings
    const QList<const QgsSettingsEntryBase *> settings() const {return mSettings;}

    /**
     * Removes all the settings at the base key for the given \a dynamicKeyPartList
     * This means it might remove more settings than the ones registered in the group, use with caution
     */
    void removeAllSettingsAtBaseKey( const QStringList &dynamicKeyPartList = QStringList() ) const;

    /**
     * Removes all the settings from this group
     * The \a dynamicKeyPart argument specifies the dynamic part of the settings key.
     */
    void removeAllChildrenSettings( const QString &dynamicKeyPart = QString() ) const;

    /**
     * Removes all the settings from this group
     * The \a dynamicKeyPartList argument specifies the dynamic part of the settings key.
     */
    void removeAllChildrenSettings( const QStringList &dynamicKeyPartList ) const;

  private:
    bool hasDynamicKey() const;

    QList<const QgsSettingsEntryBase *> mSettings;
    QString mDefinitionBaseKey;
    bool mIsValid = true;
};


#endif // QGSSETTINGSENTRYGROUP_H
