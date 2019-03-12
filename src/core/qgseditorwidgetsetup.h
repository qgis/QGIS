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
 * Holder for the widget type and its configuration for a field.
 *
 * \since QGIS 3.0
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

    //! Constructor for QgsEditorWidgetSetup
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

  private:
    QString mType;
    QVariantMap mConfig;
};

#endif // QGSEDITORWIDGETSETUP_H
