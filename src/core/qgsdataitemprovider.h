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

class QgsDataItem;

class QString;

/** \ingroup core
 * This is the interface for those who want to add custom data items to the browser tree.
 *
 * The method createDataItem() is ever called only if capabilities() return non-zero value.
 * There are two occasions when createDataItem() is called:
 * 1. to create root items (passed path is empty, parent item is null).
 * 2. to create items in directory structure. For this capabilities have to return at least
 *    of the following: QgsDataProider::Dir or QgsDataProvider::File. Passed path is the file
 *    or directory being inspected, parent item is a valid QgsDirectoryItem
 *
 * @note added in 2.10
 */
class CORE_EXPORT QgsDataItemProvider
{
  public:
    virtual ~QgsDataItemProvider() {}

    //! Human-readable name of the provider name
    virtual QString name() = 0;

    //! Return combination of flags from QgsDataProvider::DataCapabilities
    virtual int capabilities() = 0;

    //! Create a new instance of QgsDataItem (or null) for given path and parent item.
    //! Caller takes responsibility of deleting created items.
    virtual QgsDataItem* createDataItem( const QString& path, QgsDataItem* parentItem ) = 0;

};

#endif // QGSDATAITEMPROVIDER_H
