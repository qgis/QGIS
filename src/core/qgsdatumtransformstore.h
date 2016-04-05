/***************************************************************************
    qgsdatumtransformstore.h
    ---------------------
    begin                : June 2014
    copyright            : (C) 2014 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSDATUMTRANSFORMSTORE_H
#define QGSDATUMTRANSFORMSTORE_H

#include "qgscoordinatereferencesystem.h"

class QgsCoordinateTransform;
class QgsMapLayer;

class QDomElement;


/**
 * @brief The QgsDatumTransformStore class keeps track of datum transformations
 * as chosen by the user.
 *
 * @note added in 2.4
 */
class CORE_EXPORT QgsDatumTransformStore
{
  public:

    struct Entry
    {
      QString srcAuthId;
      QString destAuthId;
      int srcDatumTransform; //-1 if unknown or not specified
      int destDatumTransform;
    };

    explicit QgsDatumTransformStore( const QgsCoordinateReferenceSystem& destCrs );

    void clear();

    void setDestinationCrs( const QgsCoordinateReferenceSystem& destCrs );

    void addEntry( const QString& layerId, const QString& srcAuthId, const QString& destAuthId, int srcDatumTransform, int destDatumTransform );

    bool hasEntryForLayer( const QgsMapLayer* layer ) const;

    /** Will return transform from layer's CRS to current destination CRS.
     *  Will emit datumTransformInfoRequested signal if the layer has no entry.
     *  Returns an instance from QgsCoordinateTransformCache
     */
    const QgsCoordinateTransform* transformation( QgsMapLayer* layer ) const;

    /** Returns the entry in the store
        @param list entries will be appended to the list
        @note not available in python bindings*/
    void entries( QList< QPair< QString, Entry> >& list ) const;

    void readXML( const QDomNode& parentNode );

    void writeXML( QDomNode& parentNode, QDomDocument& theDoc ) const;



  protected:
    QgsCoordinateReferenceSystem mDestCRS;

    //! key = layer ID
    QHash< QString, Entry > mEntries;
};

#endif // QGSDATUMTRANSFORMSTORE_H
