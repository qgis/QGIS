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

#include "qgsfeatureiterator.h"

QgsArrowIterator::QgsArrowIterator( QgsFeatureIterator featureIterator, struct ArrowSchema *arrowSchema )
  : mFeatureIterator( featureIterator ) {}

QgsArrowIterator::~QgsArrowIterator() {}

void QgsArrowIterator::setSchema( const struct ArrowSchema *requestedSchema ) {}

void QgsArrowIterator::inferSchema( struct ArrowSchema *out ) {}

void QgsArrowIterator::nextFeatures( int64_t n, struct ArrowArray *out ) {}
