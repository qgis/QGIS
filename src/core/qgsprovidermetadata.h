/***************************************************************************
                    qgsprovidermetadata.h  -  Metadata class for
                    describing a data provider.
                             -------------------
    begin                : Sat Jan 10 2004
    copyright            : (C) 2004 by Gary E.Sherman
    email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROVIDERMETADATA_H
#define QGSPROVIDERMETADATA_H


#include <QString>
#include "qgis_sip.h"
#include "qgsdataprovider.h"
#include "qgis_core.h"
#include <functional>

/**
 * \ingroup core
 * Holds data provider key, description, and associated shared library file or function pointer information.
 *
 * Provider metadata refers either to providers which are loaded via libraries or
 * which are native providers that are included in the core QGIS installation
 * and accessed through function pointers.
 *
 * For library based providers, the metadata class is used in a lazy load
 * implementation in QgsProviderRegistry.  To save memory, data providers
 * are only actually loaded via QLibrary calls if they're to be used.  (Though they're all
 * iteratively loaded once to get their metadata information, and then
 * unloaded when the QgsProviderRegistry is created.)  QgsProviderMetadata
 * supplies enough information to be able to later load the associated shared
 * library object.
 *
 */
class CORE_EXPORT QgsProviderMetadata
{
  public:

    /**
     * Typedef for data provider creation function.
     * \since QGIS 3.0
     */
    SIP_SKIP typedef std::function < QgsDataProvider*( const QString &, const QgsDataProvider::ProviderOptions & ) > CreateDataProviderFunction;

    QgsProviderMetadata( const QString &_key, const QString &_description, const QString &_library );

    /**
     * Metadata for provider with direct provider creation function pointer, where
     * no library is involved.
     * \since QGIS 3.0
     */
#ifndef SIP_RUN
    QgsProviderMetadata( const QString &key, const QString &description, const QgsProviderMetadata::CreateDataProviderFunction &createFunc );
#else
    QgsProviderMetadata( const QString &key, const QString &description, SIP_PYCALLABLE / AllowNone / );
    % MethodCode

    // Make sure the callable doesn't get garbage collected, this is needed because refcount for a2 is 0
    // and the creation function pointer is passed to the metadata and it needs to be kept in memory.
    Py_INCREF( a2 );

    Py_BEGIN_ALLOW_THREADS

    sipCpp = new QgsProviderMetadata( *a0, *a1, [a2]( const QString &dataSource, const QgsDataProvider::ProviderOptions &providerOptions ) -> QgsDataProvider*
    {
      QgsDataProvider *provider;
      provider = nullptr;
      PyObject *sipResObj;
      SIP_BLOCK_THREADS

      sipResObj = sipCallMethod( nullptr, a2, "DD", new QString( dataSource ), sipType_QString, nullptr, new QgsDataProvider::ProviderOptions( providerOptions ), sipType_QgsDataProvider_ProviderOptions, NULL );

      if ( sipResObj )
      {
        if ( sipCanConvertToType( sipResObj, sipType_QgsDataProvider, SIP_NOT_NONE ) )
        {
          int state0;
          int sipIsErr = 0;
          provider = reinterpret_cast<QgsDataProvider *>( sipConvertToType( sipResObj, sipType_QgsDataProvider, nullptr, SIP_NOT_NONE, &state0, &sipIsErr ) );
          if ( sipIsErr != 0 )
          {
            sipReleaseType( provider, sipType_QgsDataProvider, state0 );
            provider = nullptr;
          }
        }
      }
      SIP_UNBLOCK_THREADS
      return provider;
    } );

    Py_END_ALLOW_THREADS

    % End
#endif

    /**
     * This returns the unique key associated with the provider

        This key string is used for the associative container in QgsProviderRegistry
     */
    QString key() const;

    /**
     * This returns descriptive text for the provider

        This is used to provide a descriptive list of available data providers.
     */
    QString description() const;

    /**
     * This returns the library file name

        This is used to QLibrary calls to load the data provider.
     */
    QString library() const;

    /**
     * Returns a pointer to the direct provider creation function, if supported
     * by the provider.
     * \note not available in Python bindings
     * \since QGIS 3.0
     */
    SIP_SKIP CreateDataProviderFunction createFunction() const;

  private:

    /// unique key for data provider
    QString key_;

    /// associated terse description
    QString description_;

    /// file path
    QString library_;

    CreateDataProviderFunction mCreateFunc = nullptr;

};

#endif //QGSPROVIDERMETADATA_H

