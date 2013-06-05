/***************************************************************************
                      qgsfeature.h - Spatial Feature Class
                     --------------------------------------
Date                 : 09-Sep-2003
Copyright            : (C) 2003 by Gary E.Sherman
email                : sherman at mrcc.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSFEATURE_H
#define QGSFEATURE_H

#include <QMap>
#include <QString>
#include <QVariant>
#include <QList>
#include <QHash>
#include <QVector>
#include <QSet>

class QgsGeometry;
class QgsRectangle;
class QgsFeature;
class QgsFields;

// feature id class (currently 64 bit)
#if 0
#include <limits>

class QgsFeatureId
{
  public:
    QgsFeatureId( qint64 id = 0 ) : mId( id ) {}
    QgsFeatureId( QString str ) : mId( str.toLongLong() ) {}
    QgsFeatureId &operator=( const QgsFeatureId &other ) { mId = other.mId; return *this; }
    QgsFeatureId &operator++() { mId++; return *this; }
    QgsFeatureId operator++( int ) { QgsFeatureId pId = mId; ++( *this ); return pId; }

    bool operator==( const QgsFeatureId &id ) const { return mId == id.mId; }
    bool operator!=( const QgsFeatureId &id ) const { return mId != id.mId; }
    bool operator<( const QgsFeatureId &id ) const { return mId < id.mId; }
    bool operator>( const QgsFeatureId &id ) const { return mId > id.mId; }
    operator QString() const { return QString::number( mId ); }

    bool isNew() const
    {
      return mId < 0;
    }

    qint64 toLongLong() const
    {
      return mId;
    }

  private:
    qint64 mId;

    friend uint qHash( const QgsFeatureId &id );
};

inline uint qHash( const QgsFeatureId &id )
{
  return qHash( id.mId );
}

#define FID_IS_NEW(fid)     (fid).isNew()
#define FID_TO_NUMBER(fid)  (fid).toLongLong()
#define FID_TO_STRING(fid)  static_cast<QString>(fid)
#define STRING_TO_FID(str)  QgsFeatureId(str)
#endif

// 64 bit feature ids
#if 1
typedef qint64 QgsFeatureId;
#define FID_IS_NEW(fid)     (fid<0)
#define FID_TO_NUMBER(fid)  static_cast<qint64>(fid)
#define FID_TO_STRING(fid)  QString::number( fid )
#define STRING_TO_FID(str)  (str).toLongLong()
#endif

// 32 bit feature ids
#if 0
typedef int QgsFeatureId;
#define FID_IS_NEW(fid)     (fid<0)
#define FID_TO_NUMBER(fid)  static_cast<int>(fid)
#define FID_TO_STRING(fid)  QString::number( fid )
#define STRING_TO_FID(str)  (str).toLong()
#endif


// key = field index, value = field value
typedef QMap<int, QVariant> QgsAttributeMap;

typedef QVector<QVariant> QgsAttributes;

class QgsField;
typedef QMap<int, QgsField> QgsFieldMap;




/** \ingroup core
 * The feature class encapsulates a single feature including its id,
 * geometry and a list of field/values attributes.
 *
 * @author Gary E.Sherman
 */
class CORE_EXPORT QgsFeature
{
  public:
    //! Constructor
    QgsFeature( QgsFeatureId id = QgsFeatureId() );

    QgsFeature( const QgsFields& fields, QgsFeatureId id = QgsFeatureId() );

    /** copy ctor needed due to internal pointer */
    QgsFeature( const QgsFeature & rhs );

    /** assignment operator needed due to internal pointer */
    QgsFeature & operator=( QgsFeature const & rhs );

    //! Destructor
    ~QgsFeature();

    /**
     * Get the feature id for this feature
     * @return Feature id
     */
    QgsFeatureId id() const;

    /**
     * Set the feature id for this feature
     * @param id Feature id
     */
    void setFeatureId( QgsFeatureId id );

    const QgsAttributes& attributes() const { return mAttributes; }
    QgsAttributes& attributes() { return mAttributes; }
    void setAttributes( const QgsAttributes& attrs ) { mAttributes = attrs; }
    bool setAttribute( int field, const QVariant& attr );
    void initAttributes( int fieldCount );

    /**Deletes an attribute and its value*/
    void deleteAttribute( int field );

    /**
     * Return the validity of this feature. This is normally set by
     * the provider to indicate some problem that makes the feature
     * invalid or to indicate a null feature.
     */
    bool isValid() const;

    /**
     * Set the validity of the feature.
     */
    void setValid( bool validity );

    /**
     * Get the geometry object associated with this feature
     */
    QgsGeometry *geometry() const;

    /**
     * Get the geometry object associated with this feature
     * The caller assumes responsibility for the QgsGeometry*'s destruction.
     */
    QgsGeometry *geometryAndOwnership();

    /** Set this feature's geometry from another QgsGeometry object (deep copy)
     */
    void setGeometry( const QgsGeometry& geom );

    /** Set this feature's geometry (takes geometry ownership)
     * @note not available in python bindings
     */
    void setGeometry( QgsGeometry* geom );

    /**
     * Set this feature's geometry from WKB
     *
     * This feature assumes responsibility for destroying geom.
     */
    void setGeometryAndOwnership( unsigned char * geom, size_t length );

    /** Assign a field map with the feature to allow attribute access by attribute name
     *
     *  @param fields         The attribute fields which this feature holds. When used from python, make sure
     *                        a copy of the fields is held by python, as ownership stays there.
     *                        I.e. Do not call feature.setFields( myDataProvider.fields() ) but instead call
     *                        myFields = myDataProvider.fields()
     *                        feature.setFields( myFields )
     *  @param initAttributes If true, attributes are initialized. Clears any data previously assigned.
     *                        C++: Defaults to false
     *                        Python: Defaults to true
     *  @note added in 2.0
     */
    void setFields( const QgsFields* fields, bool initAttributes = false );

    /** Get associated field map. may be NULL
     *  @note added in 2.0
     */
    const QgsFields* fields() const { return mFields; }

    /** Insert a value into attribute. Returns false if attribute name could not be converted to index.
     *  Field map must be associated to make this work.
     *  @note added in 2.0
     */
    bool setAttribute( const QString& name, QVariant value );

    /** Remove an attribute value. Returns false if attribute name could not be converted to index.
     *  Field map must be associated to make this work.
     *  @note added in 2.0
     */
    bool deleteAttribute( const QString& name );

    /** Lookup attribute value from attribute name. Returns invalid variant if attribute name could not be converted to index.
     *  Field map must be associated to make this work.
     *  @note added in 2.0
     */
    QVariant attribute( const QString& name ) const;

    /** Lookup attribute value from its index. Returns invalid variant if the index does not exist.
     *  @note added in 2.0
     */
    QVariant attribute( int fieldIdx ) const;

    /** Utility method to get attribute index from name. Returns -1 if field does not exist or field map is not associated.
     *  Field map must be associated to make this work.
     *  @note added in 2.0
     */
    int fieldNameIndex( const QString& fieldName ) const;

  private:

    //! feature id
    QgsFeatureId mFid;

    /** attributes accessed by field index */
    QgsAttributes mAttributes;

    /** pointer to geometry in binary WKB format

       This is usually set by a call to OGRGeometry::exportToWkb()
     */
    QgsGeometry *mGeometry;

    /** Indicator if the mGeometry is owned by this QgsFeature.
        If so, this QgsFeature takes responsibility for the mGeometry's destruction.
     */
    bool mOwnsGeometry;

    //! Flag to indicate if this feature is valid
    bool mValid;

    //! Optional field map for name-based attribute lookups
    const QgsFields* mFields;

}; // class QgsFeature

// key = feature id, value = changed attributes
typedef QMap<QgsFeatureId, QgsAttributeMap> QgsChangedAttributesMap;

// key = feature id, value = changed geometry
typedef QMap<QgsFeatureId, QgsGeometry> QgsGeometryMap;

typedef QSet<QgsFeatureId> QgsFeatureIds;

// key = field index, value = field name
typedef QMap<int, QString> QgsFieldNameMap;

typedef QList<QgsFeature> QgsFeatureList;

Q_DECLARE_METATYPE( QgsFeatureList );

#endif
