/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/auth/qgsauthsettingswidget.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsAuthSettingsWidget : QWidget
{
%Docstring
 Widget for entering authentication credentials both in the form username/password
 and by using QGIS Authentication Database and its authentication configurations.

 The widget also offers the functionality to convert username/password credentials
 to an authentication configuration.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsauthsettingswidget.h"
%End
  public:

    enum WarningType
    {
      ProjectFile,
      UserSettings
    };

    explicit QgsAuthSettingsWidget( QWidget *parent /TransferThis/ = 0,
                                    const QString &configId = QString(),
                                    const QString &username = QString(),
                                    const QString &password = QString(),
                                    const QString &dataprovider = QString() );
%Docstring
 Create a dialog for setting an associated authentication config, either
 from existing configs, or creating/removing them from auth database
 \param parent Parent widget
 \param configId authentication configuration id
 \param username
 \param password
 \param dataprovider The key of the calling layer provider, if applicable
%End

    void setWarningText( const QString &warningText );
%Docstring
 setWarningText set the text of the warning label
 \param warningText the text of the warning label
.. seealso:: formattedWarning()
%End

    void setBasicText( const QString &basicText );
%Docstring
 setBasicText set the text of the warning label
 \param basicText the text of the basic tab label
%End

    const QString username( ) const;
%Docstring
 username
 :return: basic authentication username
 :rtype: str
%End

    void setUsername( const QString &username );
%Docstring
 setUsername set the username
 \param username the user name
%End

    const QString password( ) const;
%Docstring
 password
 :return: basic authentication password
 :rtype: str
%End

    void setPassword( const QString &password );
%Docstring
 setPassword set the password
 \param password the password
%End

    const QString configId( ) const;
%Docstring
 configId
 :return: authentication configuration id
 :rtype: str
%End

    void setConfigId( const QString &configId );
%Docstring
 setConfigId set the authentication configuration id
  param configId the authentication configuration id
%End

    void setDataprovider( const QString &dataprovider );
%Docstring
 setDataprovider set the data provider key for filtering compatible authentication configurations
 \param dataprovider data provider key
%End

    const QString dataprovider( ) const;
%Docstring
 dataprovider
 :return: the data provider key used to filter compatible authentication configurations
 :rtype: str
%End

    static const QString formattedWarning( WarningType warning );
%Docstring
 warning text message based upon where credentials are stored
 \param warning enum of warning type
 :return: pre-formatted warning text
 :rtype: str
%End

    bool btnConvertToEncryptedIsEnabled( ) const;
%Docstring
 convertButtonEnabled, mainly useful for unit tests
 :return: true if the convert button is enabled
 :rtype: bool
%End

    void showStoreCheckboxes( bool enabled );
%Docstring
 showStoreCheckboxes show the "Store" checkboxes for basic auth.
        Some connection configurations allow the user to enter credentials
        for testing the connection without storing them in the project.
        "Store" checkboxes are disabled by default.
 \param enabled
%End

    void setStoreUsernameChecked( bool checked );
%Docstring
 setStoreUsernameChecked check the "Store" checkbox for the username
 \param checked
.. seealso:: showStoreCheckboxes
%End

    void setStorePasswordChecked( bool checked );
%Docstring
 setStorePasswordCheched check the "Store" checkbox for the password
 \param checked
.. seealso:: showStoreCheckboxes
%End

    bool storePasswordIsChecked( ) const;
%Docstring
 storePassword
 :return: true if "Store" checkbox for the password is checked
 :rtype: bool
%End

    bool storeUsernameIsChecked( ) const;
%Docstring
 storeUsername
 :return: true if "Store" checkbox for the username is checked
 :rtype: bool
%End

    bool configurationTabIsSelected( );
%Docstring
 configurationTabIsSelected
 :return: true if the configuration tab is the currently selected tab
 :rtype: bool
%End

  public slots:

    bool convertToEncrypted( );
%Docstring
 convertToEncrypted is called when the convert to encrypted button is
        clicked and it creates a Basic authentication configuration from
        username and password specified in the Basic tab
 :return: return true on success
 :rtype: bool
%End




};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/auth/qgsauthsettingswidget.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
