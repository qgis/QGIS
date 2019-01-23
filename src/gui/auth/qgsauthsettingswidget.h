/***************************************************************************
  qgsauthsettingswidget.h - QgsAuthSettingsWidget

 ---------------------
 begin                : 28.9.2017
 copyright            : (C) 2017 by Alessandro Pasotti
 email                : apasotti at boundlessgeo dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSAUTHSETTINGSWIDGET_H
#define QGSAUTHSETTINGSWIDGET_H

#include "qgis_gui.h"
#include "qgis_sip.h"

#include "ui_qgsauthsettingswidget.h"

#include <QWidget>

/**
 * \ingroup gui
 * Widget for entering authentication credentials both in the form username/password
 * and by using QGIS Authentication Database and its authentication configurations.
 *
 * The widget also offers the functionality to convert username/password credentials
 * to an authentication configuration.
 *
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsAuthSettingsWidget : public QWidget, private Ui::QgsAuthSettingsWidget
{

    Q_OBJECT

  public:

    /**
     * \brief The WarningType enum is used to determine the text
     * of the message shown to the user about the destination of
     * the stored clear-text credentials from the "Basic" tab:
     * depending on the provider or the settings, the credentials
     * are stored in the user settings and/or in the project file.
     */
    enum WarningType
    {
      ProjectFile,
      UserSettings
    };
    Q_ENUM( WarningType )

    /**
     * Create a dialog for setting an associated authentication config, either
     * from existing configs, or creating/removing them from auth database
     * \param parent Parent widget
     * \param configId authentication configuration id
     * \param username
     * \param password
     * \param dataprovider The key of the calling layer provider, if applicable
     */
    explicit QgsAuthSettingsWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr,
                                    const QString &configId = QString(),
                                    const QString &username = QString(),
                                    const QString &password = QString(),
                                    const QString &dataprovider = QString() );

    /**
     * \brief setWarningText set the text of the warning label
     * \param warningText the text of the warning label
     * \see formattedWarning()
     */
    void setWarningText( const QString &warningText );

    /**
     * \brief setBasicText set the text of the warning label
     * \param basicText the text of the basic tab label
     */
    void setBasicText( const QString &basicText );

    /**
     * \brief username
     * \return basic authentication username
     */
    const QString username( ) const;

    /**
     * \brief setUsername set the username
     * \param username the user name
     */
    void setUsername( const QString &username );

    /**
     * \brief password
     * \return basic authentication password
     */
    const QString password( ) const;

    /**
     * \brief setPassword set the password
     * \param password the password
     */
    void setPassword( const QString &password );

    /**
     * \brief configId
     * \return authentication configuration id
     */
    const QString configId( ) const;

    /**
     * \brief setConfigId set the authentication configuration id
     *  param configId the authentication configuration id
     */
    void setConfigId( const QString &configId );

    /**
     * \brief setDataprovider set the data provider key for filtering compatible authentication configurations
     * \param dataprovider data provider key
     */
    void setDataprovider( const QString &dataprovider );

    /**
     * \brief dataprovider
     * \return the data provider key used to filter compatible authentication configurations
     */
    const QString dataprovider( ) const;

    /**
     * \brief warning text message based upon where credentials are stored
     * \param warning enum of warning type
     * \return pre-formatted warning text
     */
    static const QString formattedWarning( WarningType warning );

    /**
     * \brief convertButtonEnabled, mainly useful for unit tests
     * \return true if the convert button is enabled
     */
    bool btnConvertToEncryptedIsEnabled( ) const;

    /**
     * \brief showStoreCheckboxes show the "Store" checkboxes for basic auth.
     *        Some connection configurations allow the user to enter credentials
     *        for testing the connection without storing them in the project.
     *        "Store" checkboxes are disabled by default.
     * \param enabled
     */
    void showStoreCheckboxes( bool enabled );

    /**
     * \brief setStoreUsernameChecked check the "Store" checkbox for the username
     * \param checked
     * \see showStoreCheckboxes
     */
    void setStoreUsernameChecked( bool checked );

    /**
     * \brief setStorePasswordCheched check the "Store" checkbox for the password
     * \param checked
     * \see showStoreCheckboxes
     */
    void setStorePasswordChecked( bool checked );

    /**
     * \brief storePassword
     * \return true if "Store" checkbox for the password is checked
     */
    bool storePasswordIsChecked( ) const;

    /**
     * \brief storeUsername
     * \return true if "Store" checkbox for the username is checked
     */
    bool storeUsernameIsChecked( ) const;

    /**
     * \brief configurationTabIsSelected
     * \return true if the configuration tab is the currently selected tab
     */
    bool configurationTabIsSelected( );

  public slots:

    /**
     * \brief convertToEncrypted is called when the convert to encrypted button is
     *        clicked and it creates a Basic authentication configuration from
     *        username and password specified in the Basic tab
     * \return return true on success
     */
    bool convertToEncrypted( );

    /**
     * Called when user name \a text is changed.
     * \note Not available in Python bindings
     */
    void userNameTextChanged( const QString &text ) SIP_SKIP;

    /**
     * Called when password \a text is changed.
     * \note Not available in Python bindings
     */
    void passwordTextChanged( const QString &text ) SIP_SKIP;


  private:

    // Mainly for tests
    QString mDataprovider;

    void updateConvertBtnState( );

    void updateSelectedTab( );

};

#endif // QGSAUTHSETTINGSWIDGET_H
