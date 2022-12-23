/***************************************************************************
    qgsauthmethod.h
    ---------------------
    begin                : September 1, 2015
    copyright            : (C) 2015 by Boundless Spatial, Inc. USA
    author               : Larry Shaffer
    email                : lshaffer at boundlessgeo dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSAUTHMETHOD_H
#define QGSAUTHMETHOD_H

#include <QObject>
#include <QFlags>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QStringList>
#include <QUrl>
#include <QRecursiveMutex>

#include "qgsconfig.h"
#include "qgis_core.h"
#include "qgis_sip.h"

class QgsAuthMethodConfig;

/**
 * \ingroup core
 * \brief Abstract base class for authentication method plugins
 */
class CORE_EXPORT QgsAuthMethod : public QObject
{
    Q_OBJECT

  public:

    /**
     * Flags that represent the update points (where authentication configurations are expanded)
     * supported by an authentication method. These equate to the 'update*()' virtual functions
     * below, and allow for update point code to skip calling an unused update by a method, because
     * the base virtual function will always return TRUE, giving a false impression an update occurred.
     * \note When adding an 'update' member function, also add the corresponding Expansion flag.
     * \note These flags will be added to as new update points are added
     */
    enum Expansion
    {
      // TODO: Figure out all different authentication expansions current layer providers use
      NetworkRequest       = 0x1,
      NetworkReply         = 0x2,
      DataSourceUri        = 0x4,
      GenericDataSourceUri = 0x8,
      NetworkProxy                = 0x16,
      All = NetworkRequest | NetworkReply | DataSourceUri | GenericDataSourceUri | NetworkProxy
    };
    Q_DECLARE_FLAGS( Expansions, Expansion )

    //! A non-translated short name representing the auth method
    virtual QString key() const = 0;

    //! A non-translated short description representing the auth method for use in debug output and About dialog
    virtual QString description() const = 0;

    //! Translatable display version of the 'description()'
    virtual QString displayDescription() const = 0;

    //! Increment this if method is significantly updated, allow updater code to be written for previously stored authcfg
    int version() const { return mVersion; }


#ifdef HAVE_GUI
    SIP_IF_FEATURE( HAVE_GUI )

    /**
     * Constructs the configuration for the authentication method
     * \since QGIS 3.22
     */
    virtual QWidget *editWidget( QWidget *parent ) const;
    SIP_END
#endif

    /**
     * Flags that represent the update points (where authentication configurations are expanded)
     * supported by an authentication method.
     * \note These should directly correlate to existing 'update*()' member functions
     */
    QgsAuthMethod::Expansions supportedExpansions() const { return mExpansions; }

    /**
     * The data providers that the method supports, allowing for filtering out authcfgs that are not
     * applicable to a given provider, or where the updating code is not currently implemented.
     */
    QStringList supportedDataProviders() const { return mDataProviders; }

    /**
     * Update a network request with authentication components
     * \param request The network request to update
     * \param authcfg Authentication configuration ID
     * \param dataprovider Textual key for a data provider, e.g. 'postgres', that allows
     * for custom updater code specific to the provider
     * \returns Whether the update succeeded
     */
    virtual bool updateNetworkRequest( QNetworkRequest &request, const QString &authcfg,
                                       const QString &dataprovider = QString() )
    {
      Q_UNUSED( request )
      Q_UNUSED( authcfg )
      Q_UNUSED( dataprovider )
      return true; // noop
    }

    /**
     * Update a network reply with authentication components
     * \param reply The network reply object to update
     * \param authcfg Authentication configuration ID
     * \param dataprovider Textual key for a data provider, e.g. 'postgres', that allows
     * for custom updater code specific to the provider
     * \returns Whether the update succeeded
     */
    virtual bool updateNetworkReply( QNetworkReply *reply, const QString &authcfg,
                                     const QString &dataprovider = QString() )
    {
      Q_UNUSED( reply )
      Q_UNUSED( authcfg )
      Q_UNUSED( dataprovider )
      return true; // noop
    }

    /**
     * Update data source connection items with authentication components
     * \param connectionItems QStringlist of 'key=value' pairs, as utilized in QgsDataSourceUri::connectionInfo()
     * \param authcfg Authentication configuration ID
     * \param dataprovider Textual key for a data provider, e.g. 'postgres', that allows
     * for custom updater code specific to the provider
     * \returns Whether the update succeeded
     */
    virtual bool updateDataSourceUriItems( QStringList &connectionItems, const QString &authcfg,
                                           const QString &dataprovider = QString() )
    {
      Q_UNUSED( connectionItems )
      Q_UNUSED( authcfg )
      Q_UNUSED( dataprovider )
      return true; // noop
    }

    /**
     * Update proxy settings with authentication components
     * \param proxy
     * \param authcfg Authentication configuration ID
     * \param dataprovider Textual key for a data provider, e.g. 'proxy', that allows
     * for custom updater code specific to the provider
     * \returns Whether the update succeeded
     */
    virtual bool updateNetworkProxy( QNetworkProxy &proxy, const QString &authcfg,
                                     const QString &dataprovider = QString() )
    {
      Q_UNUSED( proxy )
      Q_UNUSED( authcfg )
      Q_UNUSED( dataprovider )
      return true; // noop
    }

    /**
     * Clear any cached configuration. Called when the QgsAuthManager deletes an authentication configuration (authcfg).
     * \note It is highly recommended that a cache of authentication components (per requested authcfg)
     * be implemented, to avoid excessive queries on the auth database. Such a cache could be as
     * simple as a QHash or QMap of authcfg -> QgsAuthMethodConfig. See 'Basic' auth method plugin for example.
     */
    virtual void clearCachedConfig( const QString &authcfg ) = 0;

    /**
     * Update an authentication configuration in place
     * \note Useful for updating previously stored authcfgs, when an authentication method has been significantly updated
     */
    virtual void updateMethodConfig( QgsAuthMethodConfig &mconfig ) = 0;

  protected:

    /**
     * Construct a default authentication method
     * \note Non-public since this is an abstract base class
     */
    explicit QgsAuthMethod();

    //! Tag signifying that this is an authentcation method (e.g. for use as title in message log panel output)
    static QString authMethodTag() { return QObject::tr( "Authentication method" ); }

    //! Sets the version of the auth method (useful for future upgrading)
    void setVersion( int version ) { mVersion = version; }

    //! Sets the support expansions (points in providers where the authentication is injected) of the auth method
    void setExpansions( QgsAuthMethod::Expansions expansions ) { mExpansions = expansions; }
    //! Sets list of data providers this auth method supports
    void setDataProviders( const QStringList &dataproviders ) { mDataProviders = dataproviders; }

    QgsAuthMethod::Expansions mExpansions = QgsAuthMethod::Expansions();
    QStringList mDataProviders;
    int mVersion = 0;
    QRecursiveMutex mMutex;
};
Q_DECLARE_OPERATORS_FOR_FLAGS( QgsAuthMethod::Expansions )

typedef QHash<QString, QgsAuthMethod *> QgsAuthMethodsMap;

#endif // QGSAUTHMETHOD_H
