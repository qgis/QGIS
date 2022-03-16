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
#include "qgsfields.h"
#include <QString>
#include <QVector>
#include <QSet>

#include "qgsvector3d.h"

class QgsPointCloudAttributeCollection;

/**
 * \ingroup core
 *
 * \brief Attribute for point cloud data
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
      UChar, //!< Unsigned char 1 byte
      Short, //!< Short int 2 bytes
      UShort, //!< Unsigned short int 2 bytes
      Int32, //!< Int32 4 bytes
      UInt32, //!< Unsigned int32 4 bytes
      Int64, //!< Int64 8 bytes
      UInt64, //!< Unsigned int64 8 bytes
      Float, //!< Float 4 bytes
      Double, //!< Double 8 bytes
    };

    //! Ctor
    QgsPointCloudAttribute();
    //! Ctor
    QgsPointCloudAttribute( const QString &name, DataType type );

    //! Returns name of the attribute
    QString name() const { return mName; }

    //! Returns size of the attribute in bytes
    int size() const { return mSize; }

    /**
     * Returns the data type
     *
     * \see variantType()
     */
    DataType type() const { return mType; }

    /**
     * Returns the most suitable equivalent QVariant data type to this attribute type.
     *
     * \see type()
     */
    QVariant::Type variantType() const;

    /**
     * Returns the type to use when displaying this field.
     *
     * This will be used when the full datatype with details has to displayed to the user.
     *
     * \see type()
     */
    QString displayType() const;

    /**
     * Returns the attribute's value as a double for data pointed to by \a ptr
     *
     * \note Not available in Python binding
     * \since QGIS 3.26
     */
    double convertValueToDouble( const char *ptr ) const SIP_SKIP;

    /**
     * Returns TRUE if the specified data \a type is numeric.
     */
    static bool isNumeric( DataType type );

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str = QStringLiteral( "<QgsPointCloudAttribute: %1 (%2)>" ).arg( sipCpp->name() ).arg( sipCpp->displayType() );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

    /**
    * Retrieves the x, y, z values for the point at index \a i.
    */
    static void getPointXYZ( const char *ptr, int i, std::size_t pointRecordSize, int xOffset, QgsPointCloudAttribute::DataType xType,
                             int yOffset, QgsPointCloudAttribute::DataType yType,
                             int zOffset, QgsPointCloudAttribute::DataType zType,
                             const QgsVector3D &indexScale, const QgsVector3D &indexOffset, double &x, double &y, double &z ) SIP_SKIP;

    /**
    * Retrieves all the attributes of a point
    */
    static QVariantMap getAttributeMap( const char *data, std::size_t recordOffset, const QgsPointCloudAttributeCollection &attributeCollection ) SIP_SKIP;

  private:
    void updateSize();

    QString mName;
    int mSize = 0;
    DataType mType;
};

/**
 * \ingroup core
 *
 * \brief Collection of point cloud attributes
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

    /**
     *  Adds specific missing attributes from another QgsPointCloudAttributeCollection
     * \param otherCollection a QgsPointCloudAttributeCollection with more attributes
     * \param matchingNames the names of the attributes to be added
     * \since QGIS 3.26
     */
    void extend( const QgsPointCloudAttributeCollection &otherCollection, const QSet<QString> &matchingNames );

    //! Returns all attributes
    QVector<QgsPointCloudAttribute> attributes() const;

    /**
     * Returns the number of attributes present in the collection.
     */
    int count() const { return mAttributes.size(); }

    /**
     * Returns the attribute at the specified \a index.
     */
    const QgsPointCloudAttribute &at( int index ) const { return mAttributes.at( index ); }

    /**
     * Finds the attribute with the name
     *
     * Returns NULLPTR if not found.
     */
    const QgsPointCloudAttribute *find( const QString &attributeName, int &offset ) const;

    /**
     * Returns the index of the attribute with the specified \a name.
     *
     * Returns -1 if a matching attribute was not found.
     */
    int indexOf( const QString &name ) const;

    //! Returns total size of record
    int pointRecordSize() const { return mSize; }

    /**
     * Converts the attribute collection to an equivalent QgsFields collection.
     */
    QgsFields toFields() const;

  private:
    int mSize = 0;
    QVector<QgsPointCloudAttribute> mAttributes;

    struct CachedAttributeData
    {
      int index;
      int offset;
      CachedAttributeData( int index, int offset )
        : index( index )
        , offset( offset )
      {}
    };

    QMap< QString, CachedAttributeData > mCachedAttributes;
};

#endif // QGSPOINTCLOUDATTRIBUTE_H
