/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgssourceselectproviderregistry.h                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsSourceSelectProviderRegistry
{
%Docstring
 This class keeps a list of source select providers that may add items to the QgsDataSourceManagerDialog
 When created, it automatically adds providers from data provider plugins (e.g. PostGIS, WMS, ...)

 QgsSourceSelectProviderRegistry is not usually directly created, but rather accessed through
 QgsGui.sourceSelectProviderRegistry().

.. note::

   This class access to QgsProviderRegistry instance to initialize, but QgsProviderRegistry is
 typically initialized after QgsGui is constructed, for this reason a delayed initialization has been
 implemented in the class.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgssourceselectproviderregistry.h"
%End
  public:

    QgsSourceSelectProviderRegistry();
%Docstring
 Constructor for QgsSourceSelectProviderRegistry.
%End

    ~QgsSourceSelectProviderRegistry();


    QList< QgsSourceSelectProvider *> providers();
%Docstring
Get list of available providers
 :rtype: list of QgsSourceSelectProvider
%End

    void addProvider( QgsSourceSelectProvider *provider /Transfer/ );
%Docstring
Add a ``provider`` implementation. Takes ownership of the object.
%End

    bool removeProvider( QgsSourceSelectProvider *provider /Transfer/ );
%Docstring
 Remove ``provider`` implementation from the list (``provider`` object is deleted)
 :return: true if the provider was actually removed and deleted
 :rtype: bool
%End

    QgsSourceSelectProvider *providerByName( const QString &name );
%Docstring
Return a provider by ``name`` or None if not found
 :rtype: QgsSourceSelectProvider
%End

    QList<QgsSourceSelectProvider *> providersByKey( const QString &providerKey );
%Docstring
Return a (possibly empty) list of providers by data ``providerkey``
 :rtype: list of QgsSourceSelectProvider
%End


  private:
    QgsSourceSelectProviderRegistry( const QgsSourceSelectProviderRegistry &rh );
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgssourceselectproviderregistry.h                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
