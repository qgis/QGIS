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

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgssourceselectproviderregistry.h"
%End
  public:
    QgsSourceSelectProviderRegistry();

    ~QgsSourceSelectProviderRegistry();


    QList< QgsSourceSelectProvider *> providers() const;
%Docstring
Get list of available providers
 :rtype: list of QgsSourceSelectProvider
%End

    void addProvider( QgsSourceSelectProvider *provider /Transfer/ );
%Docstring
Add a provider implementation. Takes ownership of the object.
%End

    void removeProvider( QgsSourceSelectProvider *provider );
%Docstring
Remove provider implementation from the list (provider object is deleted)
%End

    QgsSourceSelectProvider *providerByName( const QString &name ) const;
%Docstring
Return a provider by name or None if not found
 :rtype: QgsSourceSelectProvider
%End

    QList<QgsSourceSelectProvider *> providersByKey( const QString &providerKey ) const;
%Docstring
Return a (possibly empty) list of providers by data provider's key
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
