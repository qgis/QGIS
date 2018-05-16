/***************************************************************************
 qgsquickcoordinatetransformer.h
  --------------------------------------
  Date                 : 1.6.2017
  Copyright            : (C) 2017 by Matthias Kuhn
  Email                :  matthias (at) opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSQUICKCOORDINATETRANSFORMER_H
#define QGSQUICKCOORDINATETRANSFORMER_H

#include <QObject>

#include "qgspoint.h"

#include "qgis_quick.h"
#include "qgsquickmapsettings.h"

/**
 * \ingroup quick
 * Helper class for transform of coordinates (QgsPoint) to a different coordinate reference system.
 *
 * \note QML Type: CoordinateTransformer
 *
 * \since QGIS 3.2
 */
class QUICK_EXPORT QgsQuickCoordinateTransformer : public QObject
{
    Q_OBJECT

    //! Projected (destination) position (in destination CRS)
    Q_PROPERTY( QgsPoint projectedPosition READ projectedPosition NOTIFY projectedPositionChanged )

    //! Source position  (in source CRS)
    Q_PROPERTY( QgsPoint sourcePosition READ sourcePosition WRITE setSourcePosition NOTIFY sourcePositionChanged )

    //! Destination CRS
    Q_PROPERTY( QgsCoordinateReferenceSystem destinationCrs READ destinationCrs WRITE setDestinationCrs NOTIFY destinationCrsChanged )

    //! Source CRS, default 4326
    Q_PROPERTY( QgsCoordinateReferenceSystem sourceCrs READ sourceCrs WRITE setSourceCrs NOTIFY sourceCrsChanged )

    //! Map settings, for getting transformation context
    Q_PROPERTY( QgsQuickMapSettings *mapSettings MEMBER mMapSettings NOTIFY mapSettingsChanged )

  public:
    //! Creates new coordinate transformer
    explicit QgsQuickCoordinateTransformer( QObject *parent = 0 );

    //!\copydoc QgsQuickCoordinateTransformer::projectedPosition
    QgsPoint projectedPosition() const;

    //!\copydoc QgsQuickCoordinateTransformer::sourcePosition
    QgsPoint sourcePosition() const;

    //!\copydoc QgsQuickCoordinateTransformer::sourcePosition
    void setSourcePosition( QgsPoint sourcePosition );

    //!\copydoc QgsQuickCoordinateTransformer::destinationCrs
    QgsCoordinateReferenceSystem destinationCrs() const;

    //!\copydoc QgsQuickCoordinateTransformer::destinationCrs
    void setDestinationCrs( const QgsCoordinateReferenceSystem &destinationCrs );

    //!\copydoc QgsQuickCoordinateTransformer::sourceCrs
    QgsCoordinateReferenceSystem sourceCrs() const;

    //!\copydoc QgsQuickCoordinateTransformer::sourceCrs
    void setSourceCrs( const QgsCoordinateReferenceSystem &sourceCrs );

  signals:
    //! Projected position changed
    void projectedPositionChanged();

    //! Source position changed
    void sourcePositionChanged();

    //! Destination CRS changed
    void destinationCrsChanged();

    //! Source CRS changed
    void sourceCrsChanged();

    //! Map settings changed
    void mapSettingsChanged();

  private:
    void updatePosition();

    QgsPoint mProjectedPosition;
    QgsPoint mSourcePosition;
    QgsCoordinateTransform mCoordinateTransform;
    QgsQuickMapSettings *mMapSettings = nullptr;
};

#endif // QGSQUICKCOORDINATETRANSFORMER_H
