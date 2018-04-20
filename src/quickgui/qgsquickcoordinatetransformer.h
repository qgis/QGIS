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

    //! projected (destination) position
    Q_PROPERTY( QgsPoint projectedPosition READ projectedPosition NOTIFY projectedPositionChanged )

    //! Source position
    Q_PROPERTY( QgsPoint sourcePosition READ sourcePosition WRITE setSourcePosition NOTIFY sourcePositionChanged )

    //! Destination CRS
    Q_PROPERTY( QgsCoordinateReferenceSystem destinationCrs READ destinationCrs WRITE setDestinationCrs NOTIFY destinationCrsChanged )

    //! Source CRS, default 4326
    Q_PROPERTY( QgsCoordinateReferenceSystem sourceCrs READ sourceCrs WRITE setSourceCrs NOTIFY sourceCrsChanged )

    //! Map settings, for getting transformation context
    Q_PROPERTY( QgsQuickMapSettings *mapSettings MEMBER mMapSettings NOTIFY mapSettingsChanged )

  public:
    //! create new coordinate transformer
    explicit QgsQuickCoordinateTransformer( QObject *parent = 0 );

    //! Return projected position (in destination CRS)
    QgsPoint projectedPosition() const;

    //! Return source position (in source CRS)
    QgsPoint sourcePosition() const;

    //! Set source position (in source CRS)
    void setSourcePosition( QgsPoint sourcePosition );

    //! Return destination CRS
    QgsCoordinateReferenceSystem destinationCrs() const;

    //! Set destination CRS
    void setDestinationCrs( const QgsCoordinateReferenceSystem &destinationCrs );

    //! Return source CRS
    QgsCoordinateReferenceSystem sourceCrs() const;

    //! Set source CRS
    void setSourceCrs( const QgsCoordinateReferenceSystem &sourceCrs );

  signals:
    //! projected position changed
    void projectedPositionChanged();

    //! source position changed
    void sourcePositionChanged();

    //! destination CRS changed
    void destinationCrsChanged();

    //! source CRS changed
    void sourceCrsChanged();

    //! map settings changed
    void mapSettingsChanged();

  private:
    void updatePosition();

    QgsPoint mProjectedPosition;
    QgsPoint mSourcePosition;
    QgsCoordinateTransform mCoordinateTransform;
    QgsQuickMapSettings *mMapSettings = nullptr;
};

#endif // QGSQUICKCOORDINATETRANSFORMER_H
