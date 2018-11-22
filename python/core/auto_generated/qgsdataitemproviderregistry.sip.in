/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsdataitemproviderregistry.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsDataItemProviderRegistry
{
%Docstring
This class keeps a list of data item providers that may add items to the browser tree.
When created, it automatically adds providers from provider plugins (e.g. PostGIS, WMS, ...)

QgsDataItemProviderRegistry is not usually directly created, but rather accessed through
:py:func:`QgsApplication.dataItemProviderRegistry()`

.. versionadded:: 2.10
%End

%TypeHeaderCode
#include "qgsdataitemproviderregistry.h"
%End
  public:

    QgsDataItemProviderRegistry();

    ~QgsDataItemProviderRegistry();


    QList<QgsDataItemProvider *> providers() const;
%Docstring
Returns the list of available providers.
%End

    void addProvider( QgsDataItemProvider *provider /Transfer/ );
%Docstring
Adds a ``provider`` implementation to the registry. Ownership of the provider
is transferred to the registry.
%End

    void removeProvider( QgsDataItemProvider *provider );
%Docstring
Removes a ``provider`` implementation from the registry.
The provider object is automatically deleted.
%End

  private:
    QgsDataItemProviderRegistry( const QgsDataItemProviderRegistry &rh );
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsdataitemproviderregistry.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
