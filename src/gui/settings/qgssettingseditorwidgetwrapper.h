/***************************************************************************
  qgssettingseditorwidgetwrapper.h
  --------------------------------------
  Date                 : February 2023
  Copyright            : (C) 2023 by Denis Rouzaud
  Email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSETTINGSEDITORWIDGETWRAPPER_H
#define QGSSETTINGSEDITORWIDGETWRAPPER_H

#include <QVariant>

#include "qgis_sip.h"
#include "qgis_gui.h"

class QgsSettingsEntryBase;

class QDialog;

/**
 * \ingroup gui
 * \brief Base class for settings editor wrappers
 *
 * \since QGIS 3.32
 */
class GUI_EXPORT QgsSettingsEditorWidgetWrapper : public QObject
{
    Q_OBJECT
  public:
    //! Creates a wrapper from the definition stored in a \a widget created by createEditor()
    static QgsSettingsEditorWidgetWrapper *fromWidget( const QWidget *widget );

    //! Constructor
    QgsSettingsEditorWidgetWrapper( QObject *parent = nullptr );

    virtual ~QgsSettingsEditorWidgetWrapper() = default;

    /**
     * This id of the type of settings it handles
     * \note This mostly correspond to the content of Qgis::SettingsType but it's a string since custom Python implementation are possible.
     */
    virtual QString id() const = 0;

    //! Creates a new instance of the editor wrapper so it can be configured for a widget and a setting
    virtual QgsSettingsEditorWidgetWrapper *createWrapper( QObject *parent = nullptr ) const = 0 SIP_FACTORY;

    //! Creates the editor widget for the given \a setting
    QWidget *createEditor( const QgsSettingsEntryBase *setting, const QStringList &dynamicKeyPartList = QStringList(), QWidget *parent = nullptr ) SIP_TRANSFERBACK;

    //! Configures the \a editor according the setting
    bool configureEditor( QWidget *editor, const QgsSettingsEntryBase *setting, const QStringList &dynamicKeyPartList = QStringList() );

    /**
     * Sets the widget value from the setting value
     * The wrapper must be configured before calling this medthod
     */
    virtual bool setWidgetFromSetting() const = 0;

    /**
     * Sets the setting value from the widget value
     * The wrapper must be configured before calling this medthod
     */
    virtual bool setSettingFromWidget() const = 0;

    /**
     * Returns the value from the widget as a variant
     * The wrapper must be configured before calling this medthod
     */
    virtual QVariant variantValueFromWidget() const = 0;

    /**
     * Sets the \a value of the widget
     * The wrapper must be configured before calling this medthod
     */
    virtual bool setWidgetFromVariant( const QVariant &value ) const = 0;

    /**
     * Configure the settings update behavior when a widget value is changed.
     *
     * If a \a dialog is provided, the setting will be updated when the dialog is accepted.
     * If not, the setting will be updated directly at each widget value change.
     *
     * \note This must called after createEditor() or configureEditor().
     *
     * \since QGIS 3.40
     */
    void configureAutomaticUpdate( QDialog *dialog = nullptr );

    /**
     * Returns the dynamic key parts
     * \since QGIS 3.40
     */
    QStringList dynamicKeyPartList() const { return mDynamicKeyPartList; }


  protected:
    //! Creates the widgets
    virtual QWidget *createEditorPrivate( QWidget *parent = nullptr ) const = 0 SIP_TRANSFERBACK;

    //! Configures an existing \a editor widget
    virtual bool configureEditorPrivate( QWidget *editor SIP_TRANSFERBACK, const QgsSettingsEntryBase *setting SIP_KEEPREFERENCE ) = 0;

    /**
     * Enables automatic update, which causes the setting to be updated immediately when the widget
     * value is changed.
     * \since QGIS 3.40
     */
    virtual void enableAutomaticUpdatePrivate() = 0;

    QStringList mDynamicKeyPartList;
};


#endif // QGSSETTINGSEDITORWIDGETWRAPPER_H
