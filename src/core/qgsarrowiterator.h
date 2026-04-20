/***************************************************************************
    qgsarrowiterator.h
    ---------------------
    begin                : November 2025
    copyright            : (C) 2025 by Dewey Dunnington
    email                : dewey at dunnington dot ca
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSARROWITERATOR_H
#define QGSARROWITERATOR_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsfeatureiterator.h"
#include "qgsvectorlayer.h"

#ifndef ARROW_C_DATA_INTERFACE
#define ARROW_C_DATA_INTERFACE

#define ARROW_FLAG_DICTIONARY_ORDERED 1
#define ARROW_FLAG_NULLABLE 2
#define ARROW_FLAG_MAP_KEYS_SORTED 4

#ifndef SIP_RUN
///@cond PRIVATE
struct ArrowSchema
{
    // Array type description
    const char *format;
    const char *name;
    const char *metadata;
    int64_t flags;
    int64_t n_children;
    struct ArrowSchema **children;
    struct ArrowSchema *dictionary;

    // Release callback
    void ( *release )( struct ArrowSchema * );
    // Opaque producer-specific data
    void *private_data;
};
/// @endcond
#endif

#ifndef SIP_RUN
///@cond PRIVATE
struct ArrowArray
{
    // Array data description
    int64_t length;
    int64_t null_count;
    int64_t offset;
    int64_t n_buffers;
    int64_t n_children;
    const void **buffers;
    struct ArrowArray **children;
    struct ArrowArray *dictionary;

    // Release callback
    void ( *release )( struct ArrowArray * );
    // Opaque producer-specific data
    void *private_data;
};
///@endcond
#endif

#endif // ARROW_C_DATA_INTERFACE

#ifndef ARROW_C_STREAM_INTERFACE
#define ARROW_C_STREAM_INTERFACE

#ifndef SIP_RUN
///@cond PRIVATE
struct ArrowArrayStream
{
    // Callbacks providing stream functionality
    int ( *get_schema )( struct ArrowArrayStream *, struct ArrowSchema *out );
    int ( *get_next )( struct ArrowArrayStream *, struct ArrowArray *out );
    const char *( *get_last_error )( struct ArrowArrayStream * );

    // Release callback
    void ( *release )( struct ArrowArrayStream * );

    // Opaque producer-specific data
    void *private_data;
};
///@endcond
#endif

#endif // ARROW_C_STREAM_INTERFACE

/**
 * \ingroup core
 * \brief Options for inferring an ArrowSchema from a feature source.
 * \since QGIS 4.0
 */
class CORE_EXPORT QgsArrowInferSchemaOptions
{
  public:
    //! Construct default options
    QgsArrowInferSchemaOptions();

    /**
     * Set the name that should be used to refer to the geometry column
     *
     * If empty, the geometry column will be derived from the layer or be named
     * geometry if the layer does not declare a geometry column name.
     */
    void setGeometryColumnName( const QString &geometryColumnName );

    /**
     * The name that should be used for a layer's geometry column
     *
     * If empty, the geometry column should be derived from the layer or be named
     * geometry if the layer does not declare a geometry column name.
     */
    QString geometryColumnName() const;

  private:
    QString mGeometryColumnName;
};

/**
 * \ingroup core
 * \brief Wrapper around an ArrowSchema.
 *
 * This object provides a helper to allow schemas to be passed to or returned from
 * QGIS functions in C++ or Python. See the documentation for the
 * Arrow C Data Interface for how to interact with the underlying ArrowSchema.
 * https://arrow.apache.org/docs/format/CDataInterface.html
 *
 * This object also stores the index of the geometry column that should be/will be
 * populated from the geometry of iterated-over features.
 *
 * \since QGIS 4.0
 */
class CORE_EXPORT QgsArrowSchema
{
  public:
    //! Construct invalid schema holder
    QgsArrowSchema();

    //! Copy constructor
    QgsArrowSchema( const QgsArrowSchema &other );

    //! Assignment operator
    QgsArrowSchema &operator=( const QgsArrowSchema &other );

    ~QgsArrowSchema();

#ifndef SIP_RUN
    //! Access the underlying ArrowSchema from C++
    struct ArrowSchema *schema();

    //! Access the underlying ArrowSchema immutably from C++
    const struct ArrowSchema *schema() const;
#endif

    /**
     * Returns the address of the underlying ArrowSchema for import or export across boundaries
     *
     * \warning This is intended for advanced usage and may cause a crash if used incorrectly.
     */
    unsigned long long cSchemaAddress() const;

    /**
     * Export this array to the address of an empty ArrowSchema for export across boundaries
     *
     * \warning This is intended for advanced usage and may cause a crash if used incorrectly.
     */
    void exportToAddress( unsigned long long otherAddress );

    //! Returns TRUE if this wrapper object holds a valid ArrowSchema
    bool isValid() const;

    /**
     * Returns the index of the column in this schema that should be populated with a feature geometry
     *
     * Returns -1 if this value has not been specified.
     */
    int geometryColumnIndex() const;

    //! Set the index of the column in this schema that should be populated with a feature geometry
    void setGeometryColumnIndex( int geometryColumnIndex );

    // clang-format off
#ifdef SIP_RUN
    /**
     * Export this schema as an Arrow PyCapsule.
     *
     * This implements the Arrow PyCapsule interface (__arrow_c_schema__),
     * allowing QgsArrowSchema to be consumed directly by pyarrow
     * and other Arrow-compatible libraries.
     *
     * \returns A PyCapsule containing the ArrowSchema pointer.
     * \since QGIS 4.2
     */
    SIP_PYOBJECT __arrow_c_schema__();
    % MethodCode
    struct ArrowSchema *exportedSchema = static_cast<struct ArrowSchema *>( malloc( sizeof( struct ArrowSchema ) ) );
    if ( !exportedSchema )
    {
      PyErr_SetString( PyExc_MemoryError, "Failed to allocate ArrowSchema" );
      sipIsErr = 1;
    }
    else
    {
      memcpy( exportedSchema, sipCpp->schema(), sizeof( struct ArrowSchema ) );
      sipCpp->schema()->release = nullptr;
      sipRes = PyCapsule_New( exportedSchema, "arrow_schema", []( PyObject *capsule )
      {
        struct ArrowSchema *schema = static_cast<struct ArrowSchema *>( PyCapsule_GetPointer( capsule, "arrow_schema" ) );
        if ( schema && schema->release )
        {
          schema->release( schema );
        }
        free( schema );
      } );
      if ( !sipRes )
      {
        if ( exportedSchema->release )
        {
          exportedSchema->release( exportedSchema );
        }
        free( exportedSchema );
        sipIsErr = 1;
      }
    }
    % End

    /**
     * Create a QgsArrowSchema from any object implementing __arrow_c_schema__().
     *
     * \param obj An object implementing the Arrow PyCapsule interface (e.g., pyarrow.Schema),
     *            or a PyCapsule directly.
     * \returns A new QgsArrowSchema
     * \throws TypeError if obj does not implement the Arrow PyCapsule interface.
     * \since QGIS 4.2
     */
    static SIP_PYOBJECT fromArrow( SIP_PYOBJECT obj ) SIP_TYPEHINT( QgsArrowSchema );
    % MethodCode
    if ( PyCapsule_CheckExact( a0 ) && PyCapsule_IsValid( a0, "arrow_schema" ) ) {
      struct ArrowSchema *capsuleSchema = static_cast<struct ArrowSchema *>( PyCapsule_GetPointer( a0, "arrow_schema" ) );
      QgsArrowSchema *newSchema = new QgsArrowSchema();
      memcpy(newSchema->schema(), capsuleSchema, sizeof(struct ArrowSchema));
      capsuleSchema->release = nullptr;
      sipRes = sipConvertFromNewType( newSchema, sipType_QgsArrowSchema, nullptr );
    }
    else if ( PyObject_HasAttrString( a0, "__arrow_c_schema__" ) )
    {
      PyObject *method = PyObject_GetAttrString( a0, "__arrow_c_schema__" );
      if ( method )
      {
        PyObject *capsule = PyObject_CallObject( method, nullptr );
        Py_DECREF( method );
        if ( capsule )
        {
          if ( PyCapsule_CheckExact( capsule ) && PyCapsule_IsValid( capsule, "arrow_schema" ) )
          {
            struct ArrowSchema *capsuleSchema = static_cast<struct ArrowSchema *>( PyCapsule_GetPointer( capsule, "arrow_schema" ) );
            QgsArrowSchema *newSchema = new QgsArrowSchema();
            memcpy(newSchema->schema(), capsuleSchema, sizeof(struct ArrowSchema));
            capsuleSchema->release = nullptr;
            sipRes = sipConvertFromNewType( newSchema, sipType_QgsArrowSchema, nullptr );
          }
          else
          {
            PyErr_SetString( PyExc_TypeError, "__arrow_c_schema__() did not return a valid arrow_schema PyCapsule" );
            sipIsErr = 1;
          }
          Py_DECREF( capsule );
        }
        else
        {
          sipIsErr = 1; // Exception already set by PyObject_CallObject
        }
      }
      else
      {
        sipIsErr = 1; // Exception already set
      }
    }
    else
    {
      PyErr_Format( PyExc_TypeError, "Expected an object implementing __arrow_c_schema__(), got %s", Py_TYPE( a0 )->tp_name );
      sipIsErr = 1;
    }
    % End
#endif

  private:
    struct ArrowSchema mSchema {};
    int mGeometryColumnIndex = -1;

    // clang-format on
};

/**
 * \ingroup core
 * \brief Wrapper around an ArrowArray.
 *
 * This object provides a helper to allow arrays to be passed to or returned from
 * QGIS functions in C++ or Python. See the documentation for the
 * Arrow C Data Interface for how to interact with the underlying ArrowArray:
 * https://arrow.apache.org/docs/format/CDataInterface.html
 *
 * \since QGIS 4.0
 */
class CORE_EXPORT QgsArrowArray
{
  public:
    //! Construct invalid array holder
    QgsArrowArray() = default;

    QgsArrowArray &operator=( QgsArrowArray &other ) = delete;
    QgsArrowArray( const QgsArrowArray &other ) = delete;

#ifndef SIP_RUN
    //! Move constructor
    QgsArrowArray( QgsArrowArray &&other );

    //! Move-assign constructor
    QgsArrowArray &operator=( QgsArrowArray &&other );
#endif

    ~QgsArrowArray();

#ifndef SIP_RUN
    //! Access the underlying ArrowArray from C++
    struct ArrowArray *array();

    //! Access the underlying ArrowArray immutably from C++
    const struct ArrowArray *array() const;
#endif

    /**
     * Returns the address of the underlying ArrowArray for import or export across boundaries
     *
     * \warning This is intended for advanced usage and may cause a crash if used incorrectly.
     */
    unsigned long long cArrayAddress() const;

    /**
     * Export this array to the address of an empty ArrowArray for export across boundaries
     *
     * \warning This is intended for advanced usage and may cause a crash if used incorrectly.
     */
    void exportToAddress( unsigned long long otherAddress );

    //! Returns TRUE if this wrapper object holds a valid ArrowArray
    bool isValid() const;

  private:
    struct ArrowArray mArray {};

#ifdef SIP_RUN
    QgsArrowArray( const QgsArrowArray &other );
#endif
};

/**
 * \ingroup core
 * \brief Wrapper around an ArrowArrayStream.
 *
 * This object provides a helper to allow array streams to be passed to or returned from
 * QGIS functions in C++ or Python. See the documentation for the
 * Arrow C Stream Interface for how to interact with the underlying ArrowArrayStream:
 * https://arrow.apache.org/docs/format/CStreamInterface.html
 *
 * \since QGIS 4.0
 */
class CORE_EXPORT QgsArrowArrayStream
{
  public:
    //! Construct invalid array stream holder
    QgsArrowArrayStream() = default;

    QgsArrowArrayStream &operator=( QgsArrowArrayStream &other ) = delete;
    QgsArrowArrayStream( const QgsArrowArrayStream &other ) = delete;

#ifndef SIP_RUN
    //! Move constructor
    QgsArrowArrayStream( QgsArrowArrayStream &&other );

    //! Move-assign constructor
    QgsArrowArrayStream &operator=( QgsArrowArrayStream &&other );
#endif

    ~QgsArrowArrayStream();

#ifndef SIP_RUN
    //! Access the underlying ArrowArray from C++
    struct ArrowArrayStream *arrayStream();
#endif

    /**
     * Returns the address of the underlying ArrowArrayStream for import or export across boundaries
     *
     * \warning This is intended for advanced usage and may cause a crash if used incorrectly.
     */
    unsigned long long cArrayStreamAddress() const;

    /**
     * Export this array to the address of an empty ArrowArrayStream for export across boundaries
     *
     * \warning This is intended for advanced usage and may cause a crash if used incorrectly.
     */
    void exportToAddress( unsigned long long otherAddress );

    //! Returns TRUE if this wrapper object holds a valid ArrowArray
    bool isValid() const;

    // clang-format off
#ifdef SIP_RUN
    /**
     * Export this stream as an Arrow PyCapsule.
     *
     * This implements the Arrow PyCapsule interface (__arrow_c_stream__),
     * allowing QgsArrowArrayStream to be consumed directly by pyarrow,
     * geopandas, and other Arrow-compatible libraries.
     *
     * \param requested_schema Optional schema to request (currently ignored).
     * \returns A PyCapsule containing the ArrowArrayStream pointer.
     * \since QGIS 4.2
     */
    SIP_PYOBJECT __arrow_c_stream__( SIP_PYOBJECT requested_schema = Py_None );
    % MethodCode
    Q_UNUSED( a0 ); // requested_schema is not used but required by the protocol signature
    struct ArrowArrayStream *exportedStream = static_cast<struct ArrowArrayStream *>( malloc( sizeof( struct ArrowArrayStream ) ) );
    if ( !exportedStream )
    {
      PyErr_SetString( PyExc_MemoryError, "Failed to allocate ArrowArrayStream" );
      sipIsErr = 1;
    }
    else
    {
      memcpy( exportedStream, sipCpp->arrayStream(), sizeof( struct ArrowArrayStream ) );
      sipCpp->arrayStream()->release = nullptr;
      sipRes = PyCapsule_New( exportedStream, "arrow_array_stream", []( PyObject *capsule )
      {
        struct ArrowArrayStream *stream = static_cast<struct ArrowArrayStream *>( PyCapsule_GetPointer( capsule, "arrow_array_stream" ) );
        if ( stream && stream->release )
        {
          stream->release( stream );
        }
        free( stream );
      } );
      if ( !sipRes )
      {
        if ( exportedStream->release )
        {
          exportedStream->release( exportedStream );
        }
        free( exportedStream );
        sipIsErr = 1;
      }
    }
    % End

    /**
     * Create a QgsArrowArrayStream from any object implementing __arrow_c_stream__().
     *
     * \param obj An object implementing the Arrow PyCapsule interface (e.g., pyarrow.RecordBatchReader)
     *            or a capsule directly.
     * \returns A new QgsArrowArrayStream
     * \throws TypeError if obj does not implement the Arrow PyCapsule interface.
     * \since QGIS 4.2
     */
    static SIP_PYOBJECT fromArrow( SIP_PYOBJECT obj ) SIP_TYPEHINT( QgsArrowArrayStream );
    % MethodCode
    if ( PyCapsule_CheckExact( a0 ) && PyCapsule_IsValid( a0, "arrow_array_stream" ) )
    {
      struct ArrowArrayStream *capsuleStream = static_cast<struct ArrowArrayStream *>( PyCapsule_GetPointer( a0, "arrow_array_stream" ) );
      QgsArrowArrayStream *newStream = new QgsArrowArrayStream();
      memcpy(newStream->arrayStream(), capsuleStream, sizeof(struct ArrowArrayStream));
      capsuleStream->release = nullptr;
      sipRes = sipConvertFromNewType( newStream, sipType_QgsArrowArrayStream, nullptr );
    }
    else if ( PyObject_HasAttrString( a0, "__arrow_c_stream__" ) )
    {
      PyObject *method = PyObject_GetAttrString( a0, "__arrow_c_stream__" );
      if ( method )
      {
        PyObject *capsule = PyObject_CallObject( method, nullptr );
        Py_DECREF( method );
        if ( capsule )
        {
          if ( PyCapsule_CheckExact( capsule ) && PyCapsule_IsValid( capsule, "arrow_array_stream" ) )
          {
            struct ArrowArrayStream *capsuleStream = static_cast<struct ArrowArrayStream *>( PyCapsule_GetPointer( capsule, "arrow_array_stream" ) );
            QgsArrowArrayStream *newStream = new QgsArrowArrayStream();
            memcpy(newStream->arrayStream(), capsuleStream, sizeof(struct ArrowArrayStream));
            capsuleStream->release = nullptr;
            sipRes = sipConvertFromNewType( newStream, sipType_QgsArrowArrayStream, nullptr );
          }
          else
          {
            PyErr_SetString( PyExc_TypeError, "__arrow_c_stream__() did not return a valid arrow_array_stream PyCapsule" );
            sipIsErr = 1;
          }
          Py_DECREF( capsule );
        }
        else
        {
          sipIsErr = 1; // Exception already set by PyObject_CallObject
        }
      }
      else
      {
        sipIsErr = 1; // Exception already set
      }
    }
    else
    {
      PyErr_Format( PyExc_TypeError, "Expected an object implementing __arrow_c_stream__(), got %s", Py_TYPE( a0 )->tp_name );
      sipIsErr = 1;
    }
    % End
#endif

  private:
    struct ArrowArrayStream mArrayStream {};

#ifdef SIP_RUN
    QgsArrowArrayStream( const QgsArrowArrayStream &other );
#endif

    // clang-format on
};

/**
 * \ingroup core
 * \brief Wrapper for an Arrow reader of features from vector data provider or vector layer.
 * \since QGIS 4.0
 */
class CORE_EXPORT QgsArrowIterator
{
  public:
    //! Construct invalid iterator
    QgsArrowIterator() = default;

    //! Construct iterator from an existing feature iterator
    explicit QgsArrowIterator( QgsFeatureIterator featureIterator );

#ifndef SIP_RUN
    //! Access the output ArrowSchema from C++
    struct ArrowSchema *schema();
#endif

    /**
     * Set the ArrowSchema for the output of all future batches
     *
     * This must be set before calling nextFeatures().
     */
    void setSchema( const QgsArrowSchema &schema );

    //! Export this iterator as an ArrowArrayStream
    QgsArrowArrayStream toArrayStream( int batchSize = 65536 ) const;

    /**
     * Build an ArrowArray using the next n features (or fewer depending on the number of features remaining)
     *
     * If no features remain, the returned array will be invalid (i.e., isValid() will return FALSE).
     *
     * \throws QgsException if a feature's attribute cannot be appended to an ArrowArray of the
     * requested type or on internal error when building the array.
     */
    QgsArrowArray nextFeatures( int n ) SIP_THROW( QgsException );

    /**
     * Infer the QgsArrowSchema for a given QgsVectorLayer
     *
     * \throws QgsException if one or more attribute fields is of an unsupported type.
     */
    static QgsArrowSchema inferSchema( const QgsVectorLayer &layer, const QgsArrowInferSchemaOptions &options = QgsArrowInferSchemaOptions() ) SIP_THROW( QgsException );

    /**
     * Infer the QgsArrowSchema from components
     *
     * \throws QgsException if one or more attribute fields is of an unsupported type.
     */
    static QgsArrowSchema inferSchema(
      const QgsFields &fields, bool hasGeometry = false, const QgsCoordinateReferenceSystem &crs = QgsCoordinateReferenceSystem(), const QgsArrowInferSchemaOptions &options = QgsArrowInferSchemaOptions()
    ) SIP_THROW( QgsException );

  private:
    QgsFeatureIterator mFeatureIterator;
    QgsArrowSchema mSchema;
};

#endif // QGSARROWITERATOR_H
