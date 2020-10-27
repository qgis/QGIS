/***************************************************************************
                         qgspointcloudblock.h
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

#ifndef QGSPOINTCLOUDBLOCK_H
#define QGSPOINTCLOUDBLOCK_H

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
 */
class CORE_EXPORT QgsPointCloudAttributeCollection
{
  public:
    QgsPointCloudAttributeCollection();
    QgsPointCloudAttributeCollection( const QVector<QgsPointCloudAttribute> &attributes );

    void push_back( const QgsPointCloudAttribute &attribute );

    QVector<QgsPointCloudAttribute> attributes() const;

    //! Total size of record
    int pointRecordSize() const { return mSize; }
  private:
    int mSize = 0;
    QVector<QgsPointCloudAttribute> mAttributes;
};

/**
 * \ingroup core
 * Base class for storing raw data from point cloud nodes
 *
 * \note The API is considered EXPERIMENTAL and can be changed without a notice
 *
 * \since QGIS 3.18
 */
class CORE_EXPORT QgsPointCloudBlock
{
  public:
    //! Ctor
    QgsPointCloudBlock( int count,
                        const QgsPointCloudAttributeCollection &attributes,
                        const QByteArray &data );
    //! Dtor
    ~QgsPointCloudBlock();

    //! Returns raw pointer to data
    const char *data() const;

    //! Returns number of points that are stored in the block
    int pointCount() const;

    //! Returns the attributes that are stored in the data block, along with their size
    QgsPointCloudAttributeCollection attributes() const;

  private:
    int mPointCount;
    QgsPointCloudAttributeCollection mAttributes;
    QByteArray mStorage;
};

#endif // QGSPOINTCLOUDBLOCK_H
