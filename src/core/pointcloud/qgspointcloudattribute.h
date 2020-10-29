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
#include <QString>
#include <QVector>

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
    //! Systems of unit measurement
    enum DataType
    {
      Char, //!< Char 1 byte
      Short, //!< Short int 2 bytes
      Int32, //!< Int32 4 bytes
      Float, //!< Float 4 bytes
      Double, //!< Double 8 bytes
    };

    //! Ctor
    QgsPointCloudAttribute();
    //! Ctor
    QgsPointCloudAttribute( const QString &name, DataType type );

    //! Returns name of the attribute
    QString name() const;

    //! Returns size of the attribute in bytes
    size_t size() const;

    //! Returns the data type
    DataType type() const;

  private:
    void updateSize();

    QString mName;
    size_t mSize = 0;
    DataType mType;
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

    /**
     * Finds the attribute with the name
     *
     * Returns nullptr if not found
     */
    const QgsPointCloudAttribute *find( const QString &attributeName, int &offset ) const;

    //! Returns total size of record
    int pointRecordSize() const { return mSize; }

  private:
    int mSize = 0;
    QVector<QgsPointCloudAttribute> mAttributes;
};

#endif // QGSPOINTCLOUDATTRIBUTE_H
