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
      deleteConnectionsPrivate( connectionNames, deleteConnection, firstParent );
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

#endif

  private:
    static void deleteConnectionsPrivate( const QStringList &connectionNames, const std::function<void( const QString & )> &deleteConnection, QPointer<QgsDataItem> firstParent );
};

#endif // QGSDATAITEMGUIPROVIDERUTILS_H
