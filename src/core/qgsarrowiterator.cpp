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

#include "qgsarrowiterator.h"

#include <nanoarrow/nanoarrow.hpp>

#include "qgsfeatureiterator.h"



QgsArrowIterator::QgsArrowIterator( QgsFeatureIterator featureIterator )
  : mFeatureIterator( featureIterator ) {

  }

QgsArrowIterator::~QgsArrowIterator() {
    if (mSchema.release != nullptr) {
        ArrowSchemaRelease(&mSchema);
    }
}

void QgsArrowIterator::setSchema( const struct ArrowSchema *requestedSchema ) {
    if (requestedSchema == nullptr || requestedSchema->release == nullptr) {
        throw QgsException("Invalid or null ArrowSchema provided");
    }

    if (mSchema.release != nullptr) {
        ArrowSchemaRelease(&mSchema);
    }

    ArrowSchemaDeepCopy(requestedSchema, &mSchema);
}

void QgsArrowIterator::inferSchema( struct ArrowSchema *out ) {
    if (out == nullptr) {
        throw QgsException("null output ArrowSchema provided");
    }

    throw QgsException("ArrowSchema inference not yet implemented");
}

void QgsArrowIterator::nextFeatures( int64_t n, struct ArrowArray *out ) {
    if (out == nullptr) {
        throw QgsException("null output ArrowSchema provided");
    }

    if (n < 1) {
        throw QgsException("QgsArrowIterator can't iterate over less than one feature");
    }
}
