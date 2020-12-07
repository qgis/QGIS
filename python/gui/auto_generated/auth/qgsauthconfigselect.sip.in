/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/auth/qgsauthconfigselect.h                                   *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsAuthConfigSelect : QWidget
{
%Docstring
Selector widget for authentication configs
%End

%TypeHeaderCode
#include "qgsauthconfigselect.h"
%End
  public:

    explicit QgsAuthConfigSelect( QWidget *parent /TransferThis/ = 0, const QString &dataprovider = QString() );
%Docstring
Create a dialog for setting an associated authentication config, either
from existing configs, or creating/removing them from auth database

:param parent: Parent widget
:param dataprovider: The key of the calling layer provider, if applicable
%End

    void setConfigId( const QString &authcfg );
%Docstring
Sets the authentication config id for the resource
%End

    const QString configId() const;
%Docstring
Gets the authentication config id for the resource
%End

    void setDataProviderKey( const QString &key );
%Docstring
Sets key of layer provider, if applicable
%End

  signals:
    void selectedConfigIdChanged( const QString &authcfg );
%Docstring
Emitted when authentication config is changed or missing
%End

    void selectedConfigIdRemoved( const QString &authcfg );
%Docstring
Emitted when authentication config is removed
%End

  public slots:
    void showMessage( const QString &msg );
%Docstring
Show a small message bar with a close button
%End

    void clearMessage();
%Docstring
Clear and hide small message bar
%End

};





class QgsAuthConfigUriEdit : QDialog
{
%Docstring
Dialog wrapper of select widget to edit an authcfg in a data source URI
%End

%TypeHeaderCode
#include "qgsauthconfigselect.h"
%End
  public:

    explicit QgsAuthConfigUriEdit( QWidget *parent /TransferThis/ = 0,
                                   const QString &datauri = QString(),
                                   const QString &dataprovider = QString() );
%Docstring
Construct wrapper dialog for select widget to edit an authcfg in a data source URI

:param parent: Parent widget
:param datauri: URI QString with of without an authcfg=ID string
:param dataprovider: The key of the calling layer provider, if applicable
%End

    void setDataSourceUri( const QString &datauri );
%Docstring
Sets the data source URI to parse
%End

    QString dataSourceUri();
%Docstring
The returned, possibly edited data source URI
%End

    static bool hasConfigId( const QString &txt );
%Docstring
Whether a string contains an authcfg ID
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/auth/qgsauthconfigselect.h                                   *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
