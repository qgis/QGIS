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
#endif

#ifndef SIP_RUN
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
#endif

#endif // ARROW_C_DATA_INTERFACE

#ifndef ARROW_C_STREAM_INTERFACE
#define ARROW_C_STREAM_INTERFACE

#ifndef SIP_RUN
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
#endif

#endif // ARROW_C_STREAM_INTERFACE


/**
 * \ingroup core
 * \brief Wrapper around an ArrowSchema.
 */
class CORE_EXPORT QgsArrowSchema
{
  public:
    //! Construct invalid schema holder
    QgsArrowSchema() = default;

    //! Copy constructor
    QgsArrowSchema( const QgsArrowSchema &other );

    //! Assignment operator
    QgsArrowSchema &operator=( const QgsArrowSchema &other );

    ~QgsArrowSchema();

#ifndef SIP_RUN
    struct ArrowSchema *schema();

    const struct ArrowSchema *schema() const;
#endif

    //! Returns the address of the underlying ArrowSchema for export to or import from other systems
    uintptr_t cSchemaAddress();

    //! Export this schema to the address of a similar object
    void exportToAddress( uintptr_t otherAddress );

    //! Returns true if this wrapper object holds a valid ArrowSchema
    bool isValid() const;

  private:
    struct ArrowSchema mSchema {};
};


/**
 * \ingroup core
 * \brief Wrapper for an Arrow reader of features from vector data provider or vector layer.
 */
class CORE_EXPORT QgsArrowIterator
{
  public:
    //! Construct invalid iterator
    QgsArrowIterator() = default;

    //! Construct iterator from an existing feature iterator
    QgsArrowIterator( QgsFeatureIterator featureIterator );

    //! Request a specific Arrow schema for this output
    void setSchema( const QgsArrowSchema &schema );

    //! Populate out with the next n features (or fewer depending on the number of features remaining)
    void nextFeatures( int64_t n, struct ArrowArray *out ) SIP_SKIP;

    //! Guess the schema for a given QgsVectorLayer and return the geometry column index
    static QgsArrowSchema inferSchema( const QgsVectorLayer &layer );

  private:
    QgsFeatureIterator mFeatureIterator;
    QgsArrowSchema mSchema;
    int64_t mSchemaGeometryColumnIndex;
};

#endif // QGSARROWITERATOR_H
