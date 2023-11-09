/***************************************************************************
                         qgscoordinatetransformcontext_p.h
                         -------------------------------
    begin                : November 2017
    copyright            : (C) 2017 by Nyall Dawson
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

#ifndef QGSCOORDINATETRANSFORMCONTEXT_PRIVATE_H
#define QGSCOORDINATETRANSFORMCONTEXT_PRIVATE_H

/// @cond PRIVATE

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QGIS API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in testqgsfeature.cpp.
 * See details in QEP #17
 ****************************************************************************/

#define SIP_NO_FILE

#include "qgscoordinatereferencesystem.h"
#include "qgsdatumtransform.h"

class QgsCoordinateTransformContextPrivate : public QSharedData
{

  public:

    QgsCoordinateTransformContextPrivate() = default;

    QgsCoordinateTransformContextPrivate( const QgsCoordinateTransformContextPrivate &other )
      : QSharedData( other )
      , mLock{}
    {
      other.mLock.lockForRead();
      mSourceDestDatumTransforms = other.mSourceDestDatumTransforms;
      other.mLock.unlock();
    }

    /**
     * Mapping for coordinate operation Proj string to use for source/destination CRS pairs.
     */
    class OperationDetails
    {
      public:
        QString operation;
        bool allowFallback = true;

        // TODO c++20 - replace with = default
        bool operator==( const OperationDetails &other ) const
        {
          return operation == other.operation && allowFallback == other.allowFallback;
        }
    };
    QMap< QPair< QgsCoordinateReferenceSystem, QgsCoordinateReferenceSystem >, OperationDetails > mSourceDestDatumTransforms;

    //! Mutex for making QgsCoordinateTransformContextPrivate thread safe
    mutable QReadWriteLock mLock{};

  private:
    QgsCoordinateTransformContextPrivate &operator= ( const QgsCoordinateTransformContextPrivate & ) = delete;
};


/// @endcond


#endif // QGSCOORDINATETRANSFORMCONTEXT_PRIVATE_H




