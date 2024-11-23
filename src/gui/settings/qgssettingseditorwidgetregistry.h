/***************************************************************************
    qgssettingseditorwidgetregistry.h
    ---------------------
    begin                : April 2023
    copyright            : (C) 2023 by Denis Rouzaud
    email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSETTINGSEDITORREGISTRY_H
#define QGSSETTINGSEDITORREGISTRY_H

#include <QObject>
#include <QMap>

#include "qgis_gui.h"
#include "qgis_sip.h"

class QWidget;
class QgsSettingsEntryBase;
class QgsSettingsEditorWidgetWrapper;

/**
 * \ingroup gui
 * \brief This class manages editor widgets for settings
 *
 * \since QGIS 3.32
 */
class GUI_EXPORT QgsSettingsEditorWidgetRegistry
{
  public:
    //! Constructor
    QgsSettingsEditorWidgetRegistry();
    ~QgsSettingsEditorWidgetRegistry();

    /**
     * Adds an editor widget \a wrapper to the registry
     * If an editor widget with same id already exists, the wrapper is deleted and FALSE is returned.
     */
    bool addWrapper( QgsSettingsEditorWidgetWrapper *wrapper SIP_TRANSFER );

    /**
     * Adds an editor widget \a wrapper for a specific setting to the registry
     * \since QGIS 3.40
     */
    void addWrapperForSetting( QgsSettingsEditorWidgetWrapper *wrapper SIP_TRANSFER, const QgsSettingsEntryBase *setting SIP_KEEPREFERENCE );

    //! Returns a new instance of the editor widget for the given \a id
    QgsSettingsEditorWidgetWrapper *createWrapper( const QString &id, QObject *parent ) const SIP_FACTORY;

    //! Creates an editor widget for the given \a setting using the corresponding registered wrapper
    QWidget *createEditor( const QgsSettingsEntryBase *setting, const QStringList &dynamicKeyPartList, QWidget *parent = nullptr ) const SIP_TRANSFERBACK;

  private:
    QMap<QString, QgsSettingsEditorWidgetWrapper *> mWrappers;
    QMap<const QgsSettingsEntryBase *, QgsSettingsEditorWidgetWrapper *> mSpecificWrappers;
};

#endif // QGSSETTINGSEDITORREGISTRY_H
