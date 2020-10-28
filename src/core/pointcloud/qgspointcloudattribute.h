/***************************************************************************
                         qgspointcloudattribute.h
                         ---------------------
    begin                : October 2020
    copyright            : (C) 2020 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPOINTCLOUDATTRIBUTE_H
#define QGSPOINTCLOUDATTRIBUTE_H

#include "qgis.h"
#include "qgis_core.h"
#include <QPair>
#include <QString>
#include <QVector>
#include <QByteArray>

#define SIP_NO_FILE

/**
 * \ingroup core
 *
 * Attribute for point cloud data
 * pair of name and size in bytes
 *
 * \since QGIS 3.18
 */
class CORE_EXPORT QgsPointCloudAttribute
{
  public:
    //! Ctor
    QgsPointCloudAttribute( const QString &name, int size );

    //! Returns name of the attribute
    QString name() const;

    //! Returns size of the attribute in bytes
    int size() const;

  private:
    QString mName;
    int mSize;
};

/**
 * \ingroup core
 *
 * Collection of point cloud attributes
 *
 * \since QGIS 3.18
 */
class CORE_EXPORT QgsPointCloudAttributeCollection
{
  public:
    //! Ctor
    QgsPointCloudAttributeCollection();
    //! Ctor with given attributes
    QgsPointCloudAttributeCollection( const QVector<QgsPointCloudAttribute> &attributes );
    //! Adds extra attribute
    void push_back( const QgsPointCloudAttribute &attribute );

    //! Returns all attributes
    QVector<QgsPointCloudAttribute> attributes() const;

    //! Returns true for error
    bool offset( const QString &attributeName, int &offset, int &size ) const;

    //! Returns total size of record
    int pointRecordSize() const { return mSize; }
  private:
    int mSize = 0;
    QVector<QgsPointCloudAttribute> mAttributes;
};

#endif // QGSPOINTCLOUDATTRIBUTE_H
