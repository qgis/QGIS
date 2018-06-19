/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/auth/qgsauthmethodedit.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsAuthMethodEdit : QWidget
{
%Docstring
Abstract base class for the edit widget of authentication method plugins
%End

%TypeHeaderCode
#include "qgsauthmethodedit.h"
%End
  public:
    virtual bool validateConfig() = 0;
%Docstring
Validate the configuration of subclasses
%End

    virtual QgsStringMap configMap() const = 0;
%Docstring
The configuration key-vale map of subclasses
%End

  signals:
    void validityChanged( bool valid );
%Docstring
Emitted when the configuration validatity changes
%End

  public slots:

    virtual void loadConfig( const QgsStringMap &configmap ) = 0;
%Docstring
Load an existing config map into subclassed widget

:param configmap:
%End

    virtual void resetConfig() = 0;
%Docstring
Clear GUI controls in subclassed widget, optionally reloading any previously loaded config map
%End

    virtual void clearConfig() = 0;
%Docstring
Clear GUI controls in subclassed widget
%End

  protected:

    explicit QgsAuthMethodEdit( QWidget *parent = 0 );
%Docstring
Construct widget to edit an authentication method configuration

.. note::

   Non-public since this is an abstract base class

:param parent: Parent widget
%End

    ~QgsAuthMethodEdit();
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/auth/qgsauthmethodedit.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
