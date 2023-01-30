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

#include "qgis_core.h"
#include "qgis_sip.h"

#include <QExplicitlySharedDataPointer>
#include <QList>
#include <QMap>
#include <QSet>
#include <QString>
#include <QVariant>
#include <QVector>

#include "qgsattributes.h"
#include "qgsfields.h"
#include "qgsfeatureid.h"
#include <memory>
class QgsFeature;
class QgsFeaturePrivate;
class QgsField;
class QgsGeometry;
class QgsRectangle;
class QgsAbstractGeometry;
class QgsSymbol;

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in testqgsfeature.cpp.
 * See details in QEP #17
 ****************************************************************************/


/**
 * \ingroup core
 * \brief The feature class encapsulates a single feature including its unique ID,
 * geometry and a list of field/values attributes.
 * \note QgsFeature objects are implicitly shared.
 */
class CORE_EXPORT QgsFeature
{
#ifdef SIP_RUN
#if (SIP_VERSION >= 0x040900 && SIP_VERSION < 0x040c01)
#define sipType_QVariant ((sipWrapperType *) sipTypeAsPyTypeObject (sipType_QVariant))
#endif
#endif
    Q_GADGET

    Q_PROPERTY( QgsFeatureId id READ id WRITE setId )
    Q_PROPERTY( QgsAttributes attributes READ attributes WRITE setAttributes )
    Q_PROPERTY( QgsFields fields READ fields WRITE setFields )
    Q_PROPERTY( QgsGeometry geometry READ geometry WRITE setGeometry )

  public:

#ifdef SIP_RUN
    SIP_PYOBJECT __iter__();
    % MethodCode
    QgsAttributes attributes = sipCpp->attributes();
    PyObject *attrs = sipConvertFromType( &attributes, sipType_QgsAttributes, Py_None );
    sipRes = PyObject_GetIter( attrs );
    % End

    SIP_PYOBJECT __getitem__( int key );
    % MethodCode
    QgsAttributes attrs = sipCpp->attributes();
    if ( a0 < 0 || a0 >= attrs.count() )
    {
      PyErr_SetString( PyExc_KeyError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    else
    {
      QVariant *v = new QVariant( attrs.at( a0 ) );
      sipRes = sipConvertFromNewType( v, sipType_QVariant, Py_None );
    }
    % End

    SIP_PYOBJECT __getitem__( const QString &name );
    % MethodCode
    int fieldIdx = sipCpp->fieldNameIndex( *a0 );
    if ( fieldIdx == -1 )
    {
      PyErr_SetString( PyExc_KeyError, a0->toLatin1() );
      sipIsErr = 1;
    }
    else
    {
      QVariant *v = new QVariant( sipCpp->attribute( fieldIdx ) );
      sipRes = sipConvertFromNewType( v, sipType_QVariant, Py_None );
    }
    % End

    void __setitem__( int key, QVariant value / GetWrapper / );
    % MethodCode
    bool rv;

    if ( a1Wrapper == Py_None )
    {
      rv = sipCpp->setAttribute( a0, QVariant( QVariant::Int ) );
    }
    else
    {
      rv = sipCpp->setAttribute( a0, *a1 );
    }

    if ( !rv )
    {
      PyErr_SetString( PyExc_KeyError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    % End

    void __setitem__( const QString &key, QVariant value / GetWrapper / );
    % MethodCode
    int fieldIdx = sipCpp->fieldNameIndex( *a0 );
    if ( fieldIdx == -1 )
    {
      PyErr_SetString( PyExc_KeyError, a0->toLatin1() );
      sipIsErr = 1;
    }
    else
    {
      if ( a1Wrapper == Py_None )
      {
        sipCpp->setAttribute( *a0, QVariant( QVariant::Int ) );
      }
      else
      {
        sipCpp->setAttribute( fieldIdx, *a1 );
      }
    }
    % End

    void __delitem__( int key );
    % MethodCode
    if ( a0 >= 0 && a0 < sipCpp->attributes().count() )
      sipCpp->deleteAttribute( a0 );
    else
    {
      PyErr_SetString( PyExc_KeyError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    % End

    void __delitem__( const QString &name );
    % MethodCode
    int fieldIdx = sipCpp->fieldNameIndex( *a0 );
    if ( fieldIdx == -1 )
    {
      PyErr_SetString( PyExc_KeyError, a0->toLatin1() );
      sipIsErr = 1;
    }
    else
      sipCpp->deleteAttribute( fieldIdx );
    % End

    long __hash__() const;
    % MethodCode
    sipRes = qHash( *sipCpp );
    % End
#endif

    /**
     * Constructor for QgsFeature
     * \param id unique feature ID
     */
#ifndef SIP_RUN
    QgsFeature( QgsFeatureId id = FID_NULL );
#else
    QgsFeature( qint64 id = FID_NULL );
#endif

    /**
     * Constructor for QgsFeature
     * \param fields feature's fields
     * \param id unique feature ID
     */
#ifndef SIP_RUN
    QgsFeature( const QgsFields &fields, QgsFeatureId id = FID_NULL );
#else
    QgsFeature( const QgsFields &fields, qint64 id = FID_NULL );
#endif

    /**
     * Copy constructor
     */
    QgsFeature( const QgsFeature &rhs );

    /**
     * Assignment operator
     */
    QgsFeature &operator=( const QgsFeature &rhs );

    /**
     * Compares two features
     */
    bool operator==( const QgsFeature &other ) const;

    /**
     * Compares two features
     */
    bool operator!=( const QgsFeature &other ) const;

    virtual ~QgsFeature();

    /**
     * Returns the feature ID for this feature.
     * \see setId()
     */
    QgsFeatureId id() const;

    /**
     * Sets the feature \a id for this feature.
     * \param id feature id
     * \see id()
     * \warning Feature IDs will be automatically changed whenever a feature is added to vector layer or data provider.
     *  This method is not designed to allow a specific feature ID to be assigned to a feature which will be added to a
     *  layer or data provider, and the results will be unpredictable
     */
    void setId( QgsFeatureId id );

    /**
     * Returns the feature's attributes.
     *
     * Alternatively, in Python it is possible to directly iterate over a feature in order to retrieve
     * its attributes:
     *
     * \code{.py}
     *   feature = QgsFeature()
     *   feature.setAttributes([11, 'string value', 55.5])
     *   for attr in feature:
     *       print(attr)
     * \endcode
     *
     * \see setAttributes()
     * \see attributeMap()
     * \since QGIS 2.9
     */
    QgsAttributes attributes() const;

#ifndef SIP_RUN

    /**
     * Returns the feature's attributes as a map of field name to value.
     *
     * \note The fields definition must be associated with the feature using setFields() before this method can be used.
     *
     * \see attributes()
     * \see setAttributes()
     * \since QGIS 3.22.2
     */
    QVariantMap attributeMap() const;
#else

    /**
     * Returns the feature's attributes as a map of field name to value.
     *
     * \note The fields definition must be associated with the feature using setFields() before this method can be used.
     *
     * \throws ValueError if the field definition is unset or the size of the fields does not match the size of the feature's attributes()
     *
     * \see attributes()
     * \see setAttributes()
     * \since QGIS 3.22.2
     */
    SIP_PYOBJECT attributeMap() const SIP_TYPEHINT( Dict[str, Optional[object]] );
    % MethodCode
    const int fieldSize = sipCpp->fields().size();
    const int attributeSize = sipCpp->attributes().size();
    if ( fieldSize == 0 && attributeSize != 0 )
    {
      PyErr_SetString( PyExc_ValueError, QStringLiteral( "Field definition has not been set for feature" ).toUtf8().constData() );
      sipIsErr = 1;
    }
    else if ( fieldSize != attributeSize )
    {
      PyErr_SetString( PyExc_ValueError, QStringLiteral( "Feature attribute size (%1) does not match number of fields (%2)" ).arg( attributeSize ).arg( fieldSize ).toUtf8().constData() );
      sipIsErr = 1;
    }
    else
    {
      QVariantMap *v = new QVariantMap( sipCpp->attributeMap() );
      sipRes = sipConvertFromNewType( v, sipType_QVariantMap, Py_None );
    }
    % End
#endif

    /**
     * Returns the number of attributes attached to the feature.
     * \since QGIS 3.18
     */
    int attributeCount() const;

    /**
     * Sets the feature's attributes.
     *
     * Calling this method will automatically set the feature as valid (see isValid()).
     *
     * The number of provided attributes need to exactly match the number of the feature's fields.
     *
     * \param attrs List of attribute values
     *
     * \warning If the number of provided attributes does not exactly match
     * the number of the feature's fields then it will not be possible to add this feature to the corresponding data
     * provider.
     *
     * \see setAttribute()
     * \see attributes()
     */
    void setAttributes( const QgsAttributes &attrs );

#ifndef SIP_RUN

    /**
     * Sets an attribute's value by field index.
     *
     * If the attribute was successfully set then the feature will be automatically marked as valid (see isValid()).
     *
     * \param field the index of the field to set
     * \param attr the value of the attribute
     * \returns FALSE, if the field index does not exist
     * \see setAttributes()
     */
    bool setAttribute( int field, const QVariant &attr );
#else

    /**
     * Sets an attribute's value by field index.
     *
     * If the attribute was successfully set then the feature will be automatically marked as valid (see isValid()).
     *
     * Alternatively, in Python it is possible to directly set a field's value via the field's index:
     *
     * \code{.py}
     *   fields = QgsFields()
     *   fields.append(QgsField('my_id', QVariant.Int))
     *   fields.append(QgsField('name', QVariant.String))
     *   feature = QgsFeature(fields)
     *
     *   # set the "name" field value
     *   feature[1] = "my name"
     *   # set the "my_id" field value
     *   feature[0] = 55
     * \endcode
     *
     * \param field the index of the field to set
     * \param attr the value of the attribute
     * \throws KeyError if the field index does not exist
     * \see setAttributes()
     */
    bool setAttribute( int field, const QVariant &attr / GetWrapper / );
    % MethodCode
    bool rv;

    if ( a1Wrapper == Py_None )
    {
      rv = sipCpp->setAttribute( a0, QVariant( QVariant::Int ) );
    }
    else
    {
      rv = sipCpp->setAttribute( a0, *a1 );
    }

    if ( !rv )
    {
      PyErr_SetString( PyExc_KeyError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }

    sipRes = rv;
    % End
#endif

    /**
     * Initialize this feature with the given number of fields.
     *
     * Discards any previously set attribute data.
     *
     * \param fieldCount Number of fields to initialize
     *
     * \see resizeAttributes()
     */
    void initAttributes( int fieldCount );

    /**
     * Resizes the attributes attached to this feature to the given number of fields.
     *
     * If the new \a fieldCount is greater than the original number of fields then the additional attributes will
     * be filled with NULL values. All existing attributes will remain unchanged.
     *
     * If the new \a fieldCount is less than the original number of fields then the unwanted values will be discarded from the
     * end of the existing attributes.
     *
     * \see initAttributes()
     * \see padAttributes()
     * \since QGIS 3.18
     */
    void resizeAttributes( int fieldCount );

    /**
     * Resizes the attributes attached to this feature by appending the specified \a count of NULL values to the end of the existing attributes.
     *
     * \see resizeAttributes()
     * \since QGIS 3.18
     */
    void padAttributes( int count );

#ifndef SIP_RUN

    /**
     * Clear's an attribute's value by its index.
     *
     * \param field the index of the field
     *
     * \see setAttribute()
     */
    void deleteAttribute( int field );
#else

    /**
     * Clear's an attribute's value by its index.
     *
     * \param field the index of the field
     *
     * Alternatively, in Python it is possible to directly `del` an attribute via its index:
     *
     * \code{.py}
     *   feature = QgsFeature()
     *   feature.setAttributes([11, 'my feature', 55.5])
     *
     *   # will print [11, 'my feature', 55.5]
     *   print(feature.attributes())
     *
     *   # clear the second attribute
     *   del feature[1]
     *
     *   # will now print [11, NONE]
     *   print(feature.attributes())
     * \endcode
     *
     * \throws KeyError if the field is not found
     * \see setAttribute()
     */
    void deleteAttribute( int field );
    % MethodCode
    if ( a0 >= 0 && a0 < sipCpp->attributes().count() )
      sipCpp->deleteAttribute( a0 );
    else
    {
      PyErr_SetString( PyExc_KeyError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    % End
#endif

    /**
     * Returns the validity of this feature.
     *
     * This is normally set by the provider to indicate some problem that makes the feature
     * invalid or to indicate a null feature.
     *
     * \see setValid()
     */
    bool isValid() const;

    /**
     * Sets the validity of the feature.
     *
     * \param validity set to TRUE if feature is valid
     *
     * \see isValid()
     */
    void setValid( bool validity );

    /**
     * Returns TRUE if the feature has an associated geometry.
     * \see geometry()
     * \since QGIS 3.0.
     */
    bool hasGeometry() const;

    /**
     * Returns the geometry associated with this feature. If the feature has no geometry,
     * an empty QgsGeometry object will be returned.
     * \see hasGeometry()
     * \see setGeometry()
     */
    QgsGeometry geometry() const;

    /**
     * Set the feature's geometry.
     *
     * Calling this method will automatically set the feature as valid (see isValid()).
     *
     * \param geometry new feature geometry
     * \see geometry()
     * \see clearGeometry()
     */
    void setGeometry( const QgsGeometry &geometry );

    /**
     * Set the feature's \a geometry.
     *
     * Ownership of the geometry is transferred to the feature.
     *
     * Calling this method will automatically set the feature as valid (see isValid()).
     *
     * This method is a shortcut for calling:
     * \code{.py}
     *   feature.setGeometry( QgsGeometry( geometry ) )
     * \endcode
     *
     * ### Example
     *
     * \code{.py}
     *   # Sets a feature's geometry to a point geometry
     *   feature.setGeometry( QgsPoint( 210, 41 ) )
     *   print(feature.geometry())
     *   # output: <QgsGeometry: Point (210 41)>
     *
     *   # Sets a feature's geometry to a line string
     *   feature.setGeometry( QgsLineString( [ QgsPoint( 210, 41 ), QgsPoint( 301, 55 ) ] ) )
     *   print(feature.geometry())
     *   # output: <QgsGeometry: LineString (210 41, 301 55)>
     * \endcode
     *
     * \see geometry()
     * \see clearGeometry()
     * \since QGIS 3.6
     */
#ifndef SIP_RUN
    void setGeometry( std::unique_ptr< QgsAbstractGeometry > geometry );
#else
    void setGeometry( QgsAbstractGeometry *geometry SIP_TRANSFER );
    % MethodCode
    sipCpp->setGeometry( std::unique_ptr< QgsAbstractGeometry>( a0 ) );
    % End
#endif

    /**
     * Removes any geometry associated with the feature.
     * \see setGeometry()
     * \see hasGeometry()
     * \since QGIS 3.0
     */
    void clearGeometry();

    /**
     * Assigns a field map with the feature to allow attribute access by attribute name.
     * \param fields The attribute fields which this feature holds
     * \param initAttributes If TRUE, attributes are initialized. Clears any data previously assigned.
     * \see fields()
     * \since QGIS 2.9
     */
    void setFields( const QgsFields &fields, bool initAttributes = false SIP_PYARGDEFAULT( true ) );

    /**
     * Returns the field map associated with the feature.
     * \see setFields()
     */
    QgsFields fields() const;

#ifndef SIP_RUN

    /**
     * Insert a value into attribute, by field \a name.
     *
     * Returns FALSE if field \a name could not be matched.
     *
     * Field map must be associated using setFields() before this method can be used.
     *
     * Calling this method will automatically set the feature as valid (see isValid()).
     *
     * \param name The name of the field to set
     * \param value The value to set
     * \returns FALSE if attribute name could not be converted to index
     * \see setFields()
     */
    bool setAttribute( const QString &name, const QVariant &value );
#else

    /**
     * Insert a value into attribute, by field \a name.
     *
     * Field map must be associated using setFields() before this method can be used.
     *
     * Calling this method will automatically set the feature as valid (see isValid()).
     *
     * Alternatively, in Python it is possible to directly set a field's value via the field's name:
     *
     * \code{.py}
     *   fields = QgsFields()
     *   fields.append(QgsField('my_id', QVariant.Int))
     *   fields.append(QgsField('name', QVariant.String))
     *   feature = QgsFeature(fields)
     *
     *   # set the "name" field value
     *   feature['name'] = "my name"
     *   # set the "my_id" field value
     *   feature['my_id'] = 55
     * \endcode
     *
     * \param name The name of the field to set
     * \param value The value to set
     * \throws KeyError if the attribute name could not could not be matched.
     * \see setFields()
     */
    void setAttribute( const QString &name, const QVariant &value / GetWrapper / );
    % MethodCode
    int fieldIdx = sipCpp->fieldNameIndex( *a0 );
    if ( fieldIdx == -1 )
    {
      PyErr_SetString( PyExc_KeyError, a0->toLatin1() );
      sipIsErr = 1;
    }
    else
    {
      if ( a1Wrapper == Py_None )
      {
        sipCpp->setAttribute( *a0, QVariant( QVariant::Int ) );
      }
      else
      {
        sipCpp->setAttribute( fieldIdx, *a1 );
      }
    }
    % End
#endif

#ifndef SIP_RUN

    /**
     * Clear's an attribute's value by its field \a name.
     *
     * Field map must be associated using setFields() before this method can be used.
     *
     * \param name The name of the field to clear
     * \returns FALSE if attribute name could not be converted to index
     * \see setFields()
     */
    bool deleteAttribute( const QString &name );
#else

    /**
     * Clear's an attribute's value by its field \a name.
     *
     * Field map must be associated using setFields() before this method can be used.
     *
     * Alternatively, in Python it is possible to directly `del` an attribute via its name:
     *
     * \code{.py}
     *   fields = QgsFields()
     *   fields.append(QgsField('my_id', QVariant.Int))
     *   fields.append(QgsField('name', QVariant.String))
     *
     *   feature = QgsFeature(fields)
     *   feature.setAttributes([11, 'my feature'])
     *
     *   # will print [11, 'my feature']
     *   print(feature.attributes())
     *
     *   # clear the 'name' attribute
     *   del feature['name']
     *
     *   # will now print [11, NULL]
     *   print(feature.attributes())
     * \endcode
     *
     * \param name The name of the field to clear
     * \throws KeyError if attribute name could not be matched.
     * \see setFields()
     */
    bool deleteAttribute( const QString &name );
    % MethodCode
    int fieldIdx = sipCpp->fieldNameIndex( *a0 );
    if ( fieldIdx == -1 )
    {
      PyErr_SetString( PyExc_KeyError, a0->toLatin1() );
      sipIsErr = 1;
      sipRes = false;
    }
    else
    {
      sipCpp->deleteAttribute( fieldIdx );
      sipRes = true;
    }
    % End
#endif

#ifndef SIP_RUN

    /**
     * Lookup attribute value by attribute \a name.
     *
     * Field map must be associated using setFields() before this method can be used.
     *
     * \param name The name of the attribute to get
     * \returns The value of the attribute, or an invalid/null variant if no such name exists
     * \see setFields
     */
    QVariant attribute( const QString &name ) const;
#else

    /**
     * Lookup attribute value by attribute \a name.
     *
     * Field map must be associated using setFields() before this method can be used.
     *
     * Alternatively, in Python it is possible to directly retrieve a field's value via the field's name:
     *
     * \code{.py}
     *   fields = QgsFields()
     *   fields.append(QgsField('my_id', QVariant.Int))
     *   fields.append(QgsField('name', QVariant.String))
     *   feature = QgsFeature(fields)
     *   feature.setAttributes([11, 'my feature'])
     *
     *   # print the "name" field value
     *   print(feature['name'])
     *   # print the "my_id" field value
     *   print(feature['my_id'])
     * \endcode
     *
     * \param name The name of the attribute to get
     * \returns The value of the attribute
     * \throws KeyError if the field is not found
     * \see setFields
     */
    SIP_PYOBJECT attribute( const QString &name ) const;
    % MethodCode
    int fieldIdx = sipCpp->fieldNameIndex( *a0 );
    if ( fieldIdx == -1 )
    {
      PyErr_SetString( PyExc_KeyError, a0->toLatin1() );
      sipIsErr = 1;
    }
    else
    {
      QVariant *v = new QVariant( sipCpp->attribute( fieldIdx ) );
      sipRes = sipConvertFromNewType( v, sipType_QVariant, Py_None );
    }
    % End
#endif

#ifndef SIP_RUN

    /**
     * Lookup attribute value from its index.
     *
     * \param fieldIdx The index of the attribute to get
     * \returns The value of the attribute, or an invalid/null variant if no such name exists
     * \see setFields()
     */
    QVariant attribute( int fieldIdx ) const;
#else

    /**
     * Lookup attribute value from its index.
     *
     * Alternatively, in Python it is possible to directly retrieve a field's value via its index:
     *
     * \code{.py}
     *   feature = QgsFeature()
     *   feature.setAttributes([11, 'my feature', 55.5])
     *
     *   # will print 11
     *   print(feature[0])
     *
     *   # will print 'my feature'
     *   print(feature[1])
     *
     *   # will print 55.5
     *   print(feature[2])
     * \endcode
     *
     * \param fieldIdx The index of the attribute to get
     * \returns The value of the attribute
     * \throws KeyError if the field is not found
     * \see setFields()
     */
    SIP_PYOBJECT attribute( int fieldIdx ) const;
    % MethodCode
    {
      if ( a0 < 0 || a0 >= sipCpp->attributes().count() )
      {
        PyErr_SetString( PyExc_KeyError, QByteArray::number( a0 ) );
        sipIsErr = 1;
      }
      else
      {
        QVariant *v = new QVariant( sipCpp->attribute( a0 ) );
        sipRes = sipConvertFromNewType( v, sipType_QVariant, Py_None );
      }
    }
    % End
#endif


#ifndef SIP_RUN

    /**
     * Returns TRUE if the attribute at the specified index is an unset value.
     *
     * \see QgsUnsetAttributeValue
     * \since QGIS 3.28
     */
    bool isUnsetValue( int fieldIdx ) const;
#else

    /**
     * Returns TRUE if the attribute at the specified index is an unset value.
     *
     * \throws KeyError if the field is not found
     * \see QgsUnsetAttributeValue
     * \since QGIS 3.28
     */
    bool isUnsetValue( int fieldIdx ) const;
    % MethodCode
    {
      if ( a0 < 0 || a0 >= sipCpp->attributes().count() )
      {
        PyErr_SetString( PyExc_KeyError, QByteArray::number( a0 ) );
        sipIsErr = 1;
      }
      else
      {
        sipRes = sipCpp->isUnsetValue( a0 );
      }
    }
    % End
#endif

    /**
     * Returns the feature's embedded symbology, or NULLPTR if the feature has no embedded symbol.
     *
     * \since QGIS 3.20
     */
    const QgsSymbol *embeddedSymbol() const;

    /**
     * Sets the feature's embedded \a symbol.
     *
     * Ownership of \a symbol is transferred to the feature.
     *
     * \since QGIS 3.20
     */
    void setEmbeddedSymbol( QgsSymbol *symbol SIP_TRANSFER );

    /**
     * Utility method to get attribute index from name.
     *
     * Field map must be associated using setFields() before this method can be used.
     *
     * \param fieldName name of field to get attribute index of
     * \returns -1 if field does not exist or field map is not associated.
     * \see setFields()
     */
    int fieldNameIndex( const QString &fieldName ) const;

    /**
     * Returns the approximate RAM usage of the feature, in bytes.
     *
     * This method takes into account the size of variable elements (strings,
     * geometry, ...), but the value returned should be considered as a lower
     * bound estimation.
     *
     * \since QGIS 3.16
     */
    int approximateMemoryUsage() const;

    //! Allows direct construction of QVariants from features.
    operator QVariant() const
    {
      return QVariant::fromValue( *this );
    }

  private:

    QExplicitlySharedDataPointer<QgsFeaturePrivate> d;

}; // class QgsFeature

//! Writes the feature to stream out. QGIS version compatibility is not guaranteed.
CORE_EXPORT QDataStream &operator<<( QDataStream &out, const QgsFeature &feature )  SIP_SKIP;
//! Reads a feature from stream in into feature. QGIS version compatibility is not guaranteed.
CORE_EXPORT QDataStream &operator>>( QDataStream &in, QgsFeature &feature )  SIP_SKIP;

// key = feature id, value = changed attributes
#ifndef SIP_RUN
typedef QMap<QgsFeatureId, QgsAttributeMap> QgsChangedAttributesMap;
#else
typedef QMap<qint64, QMap<int, QVariant> > QgsChangedAttributesMap;
#endif

#include "qgsgeometry.h"

// key = feature id, value = changed geometry
#ifndef SIP_RUN
typedef QMap<QgsFeatureId, QgsGeometry> QgsGeometryMap;
#else
typedef QMap<qint64, QgsGeometry> QgsGeometryMap;
#endif

typedef QList<QgsFeature> QgsFeatureList;

CORE_EXPORT uint qHash( const QgsFeature &key, uint seed = 0 )  SIP_SKIP;

Q_DECLARE_METATYPE( QgsFeature )
Q_DECLARE_METATYPE( QgsFeatureList )

#endif
