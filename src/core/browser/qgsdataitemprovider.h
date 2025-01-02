/***************************************************************************
  qgsdataitemprovider.h
  --------------------------------------
  Date                 : March 2015
  Copyright            : (C) 2015 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSDATAITEMPROVIDER_H
#define QGSDATAITEMPROVIDER_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgis.h"
#include <QString>
#include <QVector>

class QgsDataItem;

//! handlesDirectoryPath function
typedef bool handlesDirectoryPath_t( const QString &path ) SIP_SKIP;


/**
 * \ingroup core
 * \brief This is the interface for those who want to add custom data items to the browser tree.
 *
 * The method createDataItem() is ever called only if capabilities() return non-zero value.
 * There are two occasions when createDataItem() is called:
 *
 * - to create root items (passed path is empty, parent item is NULLPTR).
 * - to create items in directory structure. For this capabilities have to return at least
 *   of the following: QgsDataProvider::Dir or QgsDataProvider::File. Passed path is the file
 *   or directory being inspected, parent item is a valid QgsDirectoryItem
 *
 */
class CORE_EXPORT QgsDataItemProvider
{
  public:
    virtual ~QgsDataItemProvider() = default;

    //! Human-readable name of the provider name
    virtual QString name() = 0;

    /**
     * Returns the data provider key (if the data item provider is associated with a data provider),
     * the default implementation returns an empty string.
     *
     * \since QGIS 3.14
     */
    virtual QString dataProviderKey() const { return QString(); };

    //! Returns combination of flags from QgsDataProvider::DataCapabilities
    virtual Qgis::DataItemProviderCapabilities capabilities() const = 0;

    /**
     * Create a new instance of QgsDataItem (or NULLPTR) for given path and parent item.
     * Caller takes responsibility of deleting created items.
     */
    virtual QgsDataItem *createDataItem( const QString &path, QgsDataItem *parentItem ) = 0 SIP_FACTORY;

    /**
     * Create a vector of instances of QgsDataItem (or NULLPTR) for given path and parent item.
     * Caller takes responsibility of deleting created items.
     */
    virtual QVector<QgsDataItem *> createDataItems( const QString &path, QgsDataItem *parentItem );

    /**
     * Returns TRUE if the provider will handle the directory at the specified \a path.
     *
     * If the provider indicates that it will handle the directory, the default creation and
     * population of directory items for the path will be avoided and it is left to the
     * provider to correctly populate relevant entries for the path.
     *
     * The default implementation returns FALSE for all paths.
     *
     */
    virtual bool handlesDirectoryPath( const QString &path );
};

#endif // QGSDATAITEMPROVIDER_H
