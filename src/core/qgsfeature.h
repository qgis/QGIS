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

class QgsGeometry;
class QgsRectangle;
class QgsFeature;

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
    QgsFeature( QgsFeatureId id = QgsFeatureId(), QString typeName = "" );

    /** copy ctor needed due to internal pointer */
    QgsFeature( QgsFeature const & rhs );

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


    /** returns the feature's type name
     */
    QString typeName() const;


    /** sets the feature's type name
     */
    void setTypeName( QString typeName );

    /**
     * Get the attributes for this feature.
     * @return A std::map containing the field name/value mapping
     */
    const QgsAttributeMap& attributeMap() const;

    /**Sets all the attributes in one go*/
    void setAttributeMap( const QgsAttributeMap& attributeMap );

    /** Clear attribute map
     * added in 1.5
     */
    void clearAttributeMap();

    /**
     * Add an attribute to the map
     */
    void addAttribute( int field, QVariant attr );

    /**Deletes an attribute and its value*/
    void deleteAttribute( int field );

    /**Changes an existing attribute value
       @param field index of the field
       @param attr attribute name and value to be set */
    void changeAttribute( int field, QVariant attr );

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
     * Return the dirty state of this feature.
     * Dirty is set if (e.g.) the feature's geometry has been modified in-memory.
     */
    bool isDirty() const;

    /**
     * Reset the dirtiness of the feature.  (i.e. make clean)
     * You would normally do this after it's saved to permanent storage (e.g. disk, an ACID-compliant database)
     */
    void clean();

    /**
     * Get the geometry object associated with this feature
     */
    QgsGeometry *geometry();

    /**
     * Get the geometry object associated with this feature
     * The caller assumes responsibility for the QgsGeometry*'s destruction.
     */
    QgsGeometry *geometryAndOwnership();

    /** Set this feature's geometry from another QgsGeometry object (deep copy)
     */
    void setGeometry( const QgsGeometry& geom );

    /** Set this feature's geometry (takes geometry ownership)
     */
    void setGeometry( QgsGeometry* geom );

    /**
     * Set this feature's geometry from WKB
     *
     * This feature assumes responsibility for destroying geom.
     */
    void setGeometryAndOwnership( unsigned char * geom, size_t length );

  private:

    //! feature id
    QgsFeatureId mFid;

    /** map of attributes accessed by field index */
    QgsAttributeMap mAttributes;

    /** pointer to geometry in binary WKB format

       This is usually set by a call to OGRGeometry::exportToWkb()
     */
    QgsGeometry *mGeometry;

    /** Indicator if the mGeometry is owned by this QgsFeature.
        If so, this QgsFeature takes responsibility for the mGeometry's destruction.
     */
    bool mOwnsGeometry;

    //! Flag to indicate if this feature is valid
    // TODO: still applies? [MD]
    bool mValid;

    //! Flag to indicate if this feature is dirty (e.g. geometry has been modified in-memory)
    // TODO: still applies? [MD]
    bool mDirty;

    /// feature type name
    QString mTypeName;


}; // class QgsFeature

// key = feature id, value = changed attributes
typedef QMap<QgsFeatureId, QgsAttributeMap> QgsChangedAttributesMap;

// key = feature id, value = changed geometry
typedef QMap<QgsFeatureId, QgsGeometry> QgsGeometryMap;

typedef QSet<QgsFeatureId> QgsFeatureIds;

// key = field index, value = field name
typedef QMap<int, QString> QgsFieldNameMap;

typedef QList<QgsFeature> QgsFeatureList;

#endif
