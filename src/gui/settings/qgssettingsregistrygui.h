/***************************************************************************
  qgssettingsregistrygui.h
  --------------------------------------
  Date                 : July 2021
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


#ifndef QGSSETTINGSREGISTRYGUI_H
#define QGSSETTINGSREGISTRYGUI_H

#include "qgis_gui.h"
#include "qgis_sip.h"
#include "qgssettingsregistry.h"
#include "qgssettingsentryimpl.h"

/**
 * \ingroup gui
 * \class QgsSettingsRegistryGui
 * \brief QgsSettingsRegistryGui is used for settings introspection and collects all
 * QgsSettingsEntry instances of gui.
 *
 * \since QGIS 3.22
 */
class GUI_EXPORT QgsSettingsRegistryGui : public QgsSettingsRegistry
{
  public:

    /**
     * Constructor for QgsSettingsRegistryGui.
     */
    QgsSettingsRegistryGui();

    /**
     * Destructor for QgsSettingsRegistryGui.
     */
    virtual ~QgsSettingsRegistryGui();

#ifndef SIP_RUN
    //! Settings entry respect screen dpi
    static const inline QgsSettingsEntryBool settingsRespectScreenDPI = QgsSettingsEntryBool( QStringLiteral( "respect_screen_dpi" ), QgsSettings::Prefix::GUI_QGIS, false );
#endif

};

#endif // QGSSETTINGSREGISTRYGUI_H
