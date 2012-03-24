/***************************************************************************
      qgsclipboard.h  -  QGIS internal clipboard for storage of features
      ------------------------------------------------------------------
    begin                : 20 May, 2005
    copyright            : (C) 2005 by Brendan Morley
    email                : morb at ozemail dot com dot au
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCLIPBOARD_H
#define QGSCLIPBOARD_H

#include <QList>
#include <QMap>

#include "qgsfield.h"
#include "qgsfeature.h"
#include "qgscoordinatereferencesystem.h"

/**
  \brief QGIS internal clipboard for features.

  An internal clipboard is required so that features can be retained in
  their original fidelity.

  The internal clipboard makes a copy of features that are presented to it,
  therefore the original objects can safely be destructed independent of
  the lifetime of the internal clipboard.

  As this class matures it should also be able to accept CSV repesentations
  of features in and out of the system clipboard (QClipboard).

  TODO: Make it work
*/

class QgsClipboard
{
  public:
    /**
    * Constructor for the clipboard.
    */
    QgsClipboard();

    //! Destructor
    virtual ~QgsClipboard();

    /*
     *  Place a copy of features on the internal clipboard,
     *  destroying the previous contents.
     */
    void replaceWithCopyOf( const QgsFieldMap& fields, QgsFeatureList& features );

    /*
     *  Returns a copy of features on the internal clipboard,
     *  the caller assumes responsibility for destroying the contents
     *  when it's done with it.
     */
    QgsFeatureList copyOf();

    /*
     *  Clears the internal clipboard.
     */
    void clear();

    /*
     *  Inserts a copy of the feature on the internal clipboard.
     */
    void insert( QgsFeature& feature );

    /*
     *  Returns true if the internal clipboard is empty, else false.
     */
    bool empty();

    /*
     *  Returns a copy of features on the internal clipboard, transformed
     *  from the clipboard CRS to the destCRS.
     *  The caller assumes responsibility for destroying the contents
     *  when it's done with it.
     */
    QgsFeatureList transformedCopyOf( QgsCoordinateReferenceSystem destCRS );

    /*
     *  Set the clipboard CRS
     */
    void setCRS( QgsCoordinateReferenceSystem crs );

    /*
     *  Get the clipboard CRS
     */
    QgsCoordinateReferenceSystem crs();

  private:

    /** QGIS-internal vector feature clipboard.
        Stored as values not pointers as each clipboard operation
        involves a deep copy anyway.
     */
    QgsFeatureList mFeatureClipboard;

    QgsCoordinateReferenceSystem mCRS;
};

#endif
