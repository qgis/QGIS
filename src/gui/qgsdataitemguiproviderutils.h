/***************************************************************************
  qgsdataitemguiproviderutils.h
  --------------------------------------
  Date                 : June 2024
  Copyright            : (C) 2024 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSDATAITEMGUIPROVIDERUTILS_H
#define QGSDATAITEMGUIPROVIDERUTILS_H

#include "qgis_gui.h"
#include "qgis_sip.h"
#include "qgis.h"
#include "qgsdataitem.h"
#include "qgsdataitemguiprovider.h"
#include <QPointer>
#include <functional>

class QgsDataItem;

/**
 * \class QgsDataItemGuiProviderUtils
 * \ingroup gui
 *
 * \brief Utility functions for QgsDataItemGuiProviders.
 *
 * \since QGIS 3.38
 */
class GUI_EXPORT QgsDataItemGuiProviderUtils
{
  public:
#ifndef SIP_RUN

    /**
     * Handles deletion of a list of connection \a items.
     *
     * \note Not available in Python bindings
     */
    template<class T>
    static void deleteConnections( const QList<T *> &items, const std::function<void( const QString & )> &deleteConnection, QgsDataItemGuiContext context )
    {
      ( void ) context;
      if ( items.empty() )
        return;

      QStringList connectionNames;
      connectionNames.reserve( items.size() );
      for ( T *item : items )
      {
        connectionNames << item->name();
      }
      QPointer<QgsDataItem> firstParent( items.at( 0 )->parent() );
      deleteConnectionsPrivate( connectionNames, deleteConnection, std::move( firstParent ) );
    }

    /**
     * Check if connection with \a name exists in \a connectionNames list and then try to
     * append a number to it to get a unique name.
     *
     * \note Not available in Python bindings
     *
     * \since QGIS 3.40
     */
    static const QString uniqueName( const QString &name, const QStringList &connectionNames );

    /**
     * Handles dropping a vector layer for \a connection items.
     *
     * \note Not available in Python bindings
     */
    static bool handleDropUriForConnection(
      std::unique_ptr< QgsAbstractDatabaseProviderConnection > connection,
      const QgsMimeDataUtils::Uri &sourceUri,
      const QString &destinationSchema,
      QgsDataItemGuiContext context,
      const QString &shortTitle,
      const QString &longTitle,
      const QVariantMap &destinationProviderOptions,
      const std::function<void()> &onSuccessfulCompletion,
      const std::function<void( Qgis::VectorExportResult error, const QString &errorMessage )> &onError,
      QObject *connectionContext
    );

    /**
     * Handles importing a vector layer for \a connection items.
     *
     * \note Not available in Python bindings
     */
    static void handleImportVectorLayerForConnection(
      std::unique_ptr< QgsAbstractDatabaseProviderConnection > connection,
      const QString &destinationSchema,
      QgsDataItemGuiContext context,
      const QString &shortTitle,
      const QString &longTitle,
      const QVariantMap &destinationProviderOptions,
      const std::function<void()> &onSuccessfulCompletion,
      const std::function<void( Qgis::VectorExportResult error, const QString &errorMessage )> &onError,
      QObject *connectionContext
    );

#endif

    /**
     * Add an \a actionToAdd to the sub menu with \a subMenuName in \a mainMenu. If the sub menu with given name does not exist it will be created.
     *
     * \param mainMenu The menu in which sub menu is search for or created.
     * \param actionToAdd The action to add.
     * \param subMenuName Translated name of the sub menu that is searched for or created.
     *
     * \since QGIS 4.0
     */
    static void addToSubMenu( QMenu *mainMenu, QAction *actionToAdd, const QString &subMenuName );

  private:
    static void deleteConnectionsPrivate( const QStringList &connectionNames, const std::function<void( const QString & )> &deleteConnection, QPointer<QgsDataItem> firstParent );
};

#endif // QGSDATAITEMGUIPROVIDERUTILS_H
