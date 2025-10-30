/***************************************************************************
    qgsarrowreader.h
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

#include "qgsfeatureiterator.h"

#ifndef ARROW_C_DATA_INTERFACE
#define ARROW_C_DATA_INTERFACE

#define ARROW_FLAG_DICTIONARY_ORDERED 1
#define ARROW_FLAG_NULLABLE 2
#define ARROW_FLAG_MAP_KEYS_SORTED 4

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

#endif // ARROW_C_DATA_INTERFACE

#ifndef ARROW_C_STREAM_INTERFACE
#define ARROW_C_STREAM_INTERFACE

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

#endif // ARROW_C_STREAM_INTERFACE

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
    QgsArrowIterator( QgsFeatureIterator featureIterator, struct ArrowSchema *arrowSchema = nullptr );

    ~QgsArrowIterator();

    //! Request a specific Arrow schema for this output
    void setSchema( const struct ArrowSchema *requestedSchema );

    //! Guess the schema for this feature iterator
    //!
    //! The schema populated by inferSchema() is not affected by any previous call to
    //! setSchema().
    void inferSchema( struct ArrowSchema *out );

    //! Populate out with the next n features (or fewer depending on the number of features remaining)
    void nextFeatures( int64_t n, struct ArrowArray *out );


  private:
    QgsFeatureIterator mFeatureIterator;
    struct ArrowSchema mSchema {};
};
