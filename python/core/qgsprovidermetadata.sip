/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsprovidermetadata.h                                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsProviderMetadata
{
%Docstring
Holds data provider key, description, and associated shared library file or function pointer information.

Provider metadata refers either to providers which are loaded via libraries or
which are native providers that are included in the core QGIS installation
and accessed through function pointers.

For library based providers, the metadata class is used in a lazy load
implementation in QgsProviderRegistry.  To save memory, data providers
are only actually loaded via QLibrary calls if they're to be used.  (Though they're all
iteratively loaded once to get their metadata information, and then
unloaded when the QgsProviderRegistry is created.)  QgsProviderMetadata
supplies enough information to be able to later load the associated shared
library object.
%End

%TypeHeaderCode
#include "qgsprovidermetadata.h"
%End
  public:


    QgsProviderMetadata( const QString &_key, const QString &_description, const QString &_library );


    QString key() const;
%Docstring
This returns the unique key associated with the provider

This key string is used for the associative container in :py:class:`QgsProviderRegistry`
%End

    QString description() const;
%Docstring
This returns descriptive text for the provider

This is used to provide a descriptive list of available data providers.
%End

    QString library() const;
%Docstring
This returns the library file name

This is used to QLibrary calls to load the data provider.
%End


};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsprovidermetadata.h                                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
