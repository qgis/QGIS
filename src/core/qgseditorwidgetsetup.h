/***************************************************************************
               qgseditorwidgetsetup.h - Holder for the widget configuration.
                     --------------------------------------
               Date                 : 01-Sep-2016
               Copyright            : (C) 2016 by Patrick Valsecchi
               email                : patrick.valsecchi at camptocamp.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSEDITORWIDGETSETUP_H
#define QGSEDITORWIDGETSETUP_H

#include "qgis_core.h"
#include <QVariantMap>

/**
 * \ingroup core
 * \brief Holder for the widget type and its configuration for a field.
 *
 */
class CORE_EXPORT QgsEditorWidgetSetup
{
  public:

    /**
     * Constructor
     */
    QgsEditorWidgetSetup( const QString &type, const QVariantMap &config )
      : mType( type )
      , mConfig( config )
    {}

    QgsEditorWidgetSetup() = default;

    /**
     * \returns the widget type to use
     */
    QString type() const { return mType; }

    /**
     * \returns the widget configuration to used
     */
    QVariantMap config() const { return mConfig; }

    /**
     * \returns TRUE if there is no widget configured.
     */
    bool isNull() const { return mType.isEmpty(); }

    // TODO c++20 - replace with = default
    bool operator==( const QgsEditorWidgetSetup &other ) const
    {
      return mType == other.mType && mConfig == other.mConfig;
    }

  private:
    QString mType;
    QVariantMap mConfig;
};

#endif // QGSEDITORWIDGETSETUP_H
