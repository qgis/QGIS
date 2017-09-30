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
#include "qgis.h"

#include "ui_qgsauthsettingswidget.h"

#include <QWidget>

/** \ingroup gui
 * Widget for entering authentication credentials both in the form username/password
 * and by using QGIS Authentication Database and its authentication configurations.
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsAuthSettingsWidget : public QWidget, private Ui::QgsAuthSettingsWidget
{

    Q_OBJECT

  public:

    /**
     * Create a dialog for setting an associated authentication config, either
     * from existing configs, or creating/removing them from auth database
     * \param parent Parent widget
     * \param configId authentication configuration id
     * \param username
     * \param password
     * \param dataprovider The key of the calling layer provider, if applicable
     */
    explicit QgsAuthSettingsWidget( QWidget *parent SIP_TRANSFERTHIS = 0,
                                      const QString &configId = QString(),
                                      const QString &username = QString(),
                                      const QString &password = QString(),
                                      const QString &dataprovider = QString() );

    /**
     * \brief setWarningText set the text of the warning label
     * \param warningText the text of the warning label
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
     * \brief password
     * \return basic authentication password
     */
    const QString password( ) const;

    /**
     * \brief configId
     * \return authentication configuration id
     */
    const QString configId( ) const;

    /**
     * \brief currentTabIndex, mainly useful for unit tests
     * \return active tab index
     */
    int currentTabIndex( ) const;

    /**
     * \brief convertButtonEnabled, mainly useful for unit tests
     * \return true if the convert button is enabled
     */
    bool btnConvertToEncryptedIsEnabled( ) const;

  public slots:

    /**
     * \brief on_btnConvertToEncrypted_clicked create a Basic authentication configuration from
     *        username and password specified in the Basic tab
     * \return return true on success
     */
    bool on_btnConvertToEncrypted_clicked( );

    /**
     * \brief on_txtUserName_textChanged set convert button state
     * \param text the changet text
     * \param Not available in Python bindings
     */
    void on_txtUserName_textChanged( const QString &text ) SIP_SKIP;

    /**
     * \brief on_txtPassword_textChanged set convert button state
     * \param text the changed text
     * \note Not available in Python bindings
     */
    void on_txtPassword_textChanged( const QString &text ) SIP_SKIP;


  private:

    void updateConvertBtnState( );

};

#endif // QGSAUTHSETTINGSWIDGET_H
