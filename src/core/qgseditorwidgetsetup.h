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

#include "qgseditorwidgetconfig.h"

/** \ingroup core
 * Holder for the widget type and its configuration for a field.
 *
 * @note added in QGIS 3.0
 */
class CORE_EXPORT QgsEditorWidgetSetup
{
  public:
    /**
     * Constructor
     */
    QgsEditorWidgetSetup( const QString& type, const QgsEditorWidgetConfig& config ) : mType( type ), mConfig( config ) {}
    QgsEditorWidgetSetup() {}

    /**
     * @return the widget type to use
     */
    QString type() const { return mType; }

    /**
     * @return the widget configuration to used
     */
    QgsEditorWidgetConfig config() const { return mConfig; }

    /**
     * @return true if there is no widget configured.
     */
    bool isNull() const { return mType.isEmpty(); }
  private:
    QString mType;
    QgsEditorWidgetConfig mConfig;
};

#endif // QGSEDITORWIDGETSETUP_H