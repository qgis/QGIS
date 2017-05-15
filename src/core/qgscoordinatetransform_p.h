/***************************************************************************
               qgscoordinatetransform_p.h
               --------------------------
    begin                : July 2016
    copyright            : (C) 2016 Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSCOORDINATETRANSFORMPRIVATE_H
#define QGSCOORDINATETRANSFORMPRIVATE_H

/// @cond PRIVATE

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QGIS API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//

#include <QSharedData>
#include "qgscoordinatereferencesystem.h"

typedef void *projPJ;

class QgsCoordinateTransformPrivate : public QSharedData
{

  public:

    explicit QgsCoordinateTransformPrivate();

    QgsCoordinateTransformPrivate( const QgsCoordinateReferenceSystem &source,
                                   const QgsCoordinateReferenceSystem &destination );

    QgsCoordinateTransformPrivate( const QgsCoordinateTransformPrivate &other );

    ~QgsCoordinateTransformPrivate();

    bool initialize();

    //! Flag to indicate whether the transform is valid (ie has a valid
    //! source and destination crs)
    bool mIsValid;

    /**
     * Flag to indicate that the source and destination coordinate systems are
     * equal and not transformation needs to be done
     */
    bool mShortCircuit;

    //! QgsCoordinateReferenceSystem of the source (layer) coordinate system
    QgsCoordinateReferenceSystem mSourceCRS;

    //! QgsCoordinateReferenceSystem of the destination (map canvas) coordinate system
    QgsCoordinateReferenceSystem mDestCRS;

    //! Proj4 data structure of the source projection (layer coordinate system)
    projPJ mSourceProjection;

    //! Proj4 data structure of the destination projection (map canvas coordinate system)
    projPJ mDestinationProjection;

    int mSourceDatumTransform;
    int mDestinationDatumTransform;

    static QString datumTransformString( int datumTransform );

  private:

    //! Removes +nadgrids and +towgs84 from proj4 string
    QString stripDatumTransform( const QString &proj4 ) const;

    //! In certain situations, null grid shifts have to be added to src / dst proj string
    void addNullGridShifts( QString &srcProjString, QString &destProjString ) const;

    void setFinder();
};

/// @endcond

#endif // QGSCOORDINATETRANSFORMPRIVATE_H
