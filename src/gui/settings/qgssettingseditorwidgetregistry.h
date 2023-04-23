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
 * \brief This class manages editors for settings
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
     * Adds a editor to the registry
     * Returns FALSE if a editor with same id already exists.
     */
    bool addWrapper( QgsSettingsEditorWidgetWrapper *wrapper SIP_TRANSFER );

    //! Returns a new instance of the editor for the given id
    QgsSettingsEditorWidgetWrapper *createWrapper( const QString &id, QObject *parent ) const;

    //! Creates the editor for the given settings using the corresponding registered wrapper
    QWidget *createEditor( const QgsSettingsEntryBase *setting, const QStringList &dynamicKeyPartList, QWidget *parent = nullptr ) const SIP_FACTORY;

    //! Returns a map <name, id> of all registered editors.
    QMap<QString, QString> editorNames() const;


  private:
    QMap<QString, QgsSettingsEditorWidgetWrapper *> mWrappers;
};

#endif // QGSSETTINGSEDITORREGISTRY_H
