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
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsauthsettingswidget.h"
%End
  public:

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

    const QString password( ) const;
%Docstring
 password
 :return: basic authentication password
 :rtype: str
%End

    const QString configId( ) const;
%Docstring
 configId
 :return: authentication configuration id
 :rtype: str
%End

    int currentTabIndex( ) const;
%Docstring
 currentTabIndex, mainly useful for unit tests
 :return: active tab index
 :rtype: int
%End

    bool btnConvertToEncryptedIsEnabled( ) const;
%Docstring
 convertButtonEnabled, mainly useful for unit tests
 :return: true if the convert button is enabled
 :rtype: bool
%End

  public slots:

    bool on_btnConvertToEncrypted_clicked( );
%Docstring
 on_btnConvertToEncrypted_clicked create a Basic authentication configuration from
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
