/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/auth/qgsauthenticationwidget.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsAuthenticationWidget : QWidget
{
%Docstring
 Widget for entering authentication credentials both in the form username/password
 and by using QGIS Authentication Database and its authentication configurations.
%End

%TypeHeaderCode
#include "qgsauthenticationwidget.h"
%End
  public:

    explicit QgsAuthenticationWidget( QWidget *parent /TransferThis/ = 0, const QString &dataprovider = QString() );
%Docstring
 Create a dialog for setting an associated authentication config, either
 from existing configs, or creating/removing them from auth database
 \param parent Parent widget
 \param dataprovider The key of the calling layer provider, if applicable
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/auth/qgsauthenticationwidget.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
