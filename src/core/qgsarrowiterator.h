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
#include "qgsfeatureiterator.h"

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

    //! Set the name that should be used to refer to the geometry column (default: "geometry")
    void setGeometryColumnName( const QString &geometryColumnName );

    //! The name that should be used for a layer's geometry column
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
    unsigned long long cSchemaAddress();

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

  private:
    struct ArrowSchema mSchema;
    int mGeometryColumnIndex = -1;
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

    /**
     * Copy constructor
     *
     * ArrowArrays are not copyable objects, so this wrapper will throw an
     * exception if there is an attempt to copy it while it holds a valid array
     * to prevent leaking the array.
     */
    QgsArrowArray( const QgsArrowArray &other );

#ifndef SIP_RUN
    //! Move constructor
    QgsArrowArray( QgsArrowArray &&other );
#endif

    /**
     * Assignment operator
     *
     * ArrowArrays are not copyable objects, so this wrapper will throw an
     * exception if there is an attempt to copy it while it holds a valid array
     * to prevent leaking the array.
     */
    QgsArrowArray &operator=( const QgsArrowArray &other );

    ~QgsArrowArray();

#ifndef SIP_RUN
    struct ArrowArray *array();

    const struct ArrowArray *array() const;
#endif

    /**
     * Returns the address of the underlying ArrowArray for import or export across boundaries
     *
     * \warning This is intended for advanced usage and may cause a crash if used incorrectly.
     */
    unsigned long long cArrayAddress();

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
    QgsArrowIterator( QgsFeatureIterator featureIterator );

    /**
     * Set the ArrowSchema for the output of all future batches
     *
     * This must be set before calling nextFeatures().
     */
    void setSchema( const QgsArrowSchema &schema );

    /**
     * Build an ArrowArray using the next n features (or fewer depending on the number of features remaining)
     *
     * If no features remain, the returned array will be invalid (i.e., isValid() will return false).
     *
     * \throws QgsException when XXX
     */
    QgsArrowArray nextFeatures( int n ) SIP_THROW( QgsException );

    //! Infer the QgsArrowSchema for a given QgsVectorLayer
    static QgsArrowSchema inferSchema( const QgsVectorLayer &layer, const QgsArrowInferSchemaOptions &options = QgsArrowInferSchemaOptions() );

    //! Infer the QgsArrowSchema from components
    static QgsArrowSchema inferSchema( const QgsFields &fields, bool hasGeometry = false, const QgsCoordinateReferenceSystem &crs = QgsCoordinateReferenceSystem(), const QgsArrowInferSchemaOptions &options = QgsArrowInferSchemaOptions() );

  private:
    QgsFeatureIterator mFeatureIterator;
    QgsArrowSchema mSchema;
};

#endif // QGSARROWITERATOR_H
