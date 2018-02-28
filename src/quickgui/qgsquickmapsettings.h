/***************************************************************************
  qgsquickmapsettings.h
  --------------------------------------
  Date                 : 27.12.2014
  Copyright            : (C) 2014 by Matthias Kuhn
  Email                : matthias (at) opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSQUICKMAPSETTINGS_H
#define QGSQUICKMAPSETTINGS_H

#include <QObject>

#include "qgscoordinatetransformcontext.h"
#include "qgsmapsettings.h"
#include "qgsmapthemecollection.h"
#include "qgspoint.h"
#include "qgsrectangle.h"

#include "qgis_quick.h"

class QgsProject;

/**
 * \ingroup quick
 * The QgsQuickMapSettings class encapsulates QgsMapSettings class to offer
 * settings of configuration of map rendering via QML properties.
 *
 * \note QML Type: MapSettings
 *
 * \since QGIS 3.2
 */
class QUICK_EXPORT QgsQuickMapSettings : public QObject
{
    Q_OBJECT

    Q_PROPERTY( QgsProject *project READ project WRITE setProject NOTIFY projectChanged )
    Q_PROPERTY( QgsRectangle extent READ extent WRITE setExtent NOTIFY extentChanged )
    Q_PROPERTY( QgsRectangle visibleExtent READ visibleExtent NOTIFY visibleExtentChanged )
    Q_PROPERTY( double mapUnitsPerPixel READ mapUnitsPerPixel NOTIFY mapUnitsPerPixelChanged )
    Q_PROPERTY( double rotation READ rotation WRITE setRotation NOTIFY rotationChanged )
    Q_PROPERTY( QSize outputSize READ outputSize WRITE setOutputSize NOTIFY outputSizeChanged )
    Q_PROPERTY( double outputDpi READ outputDpi WRITE setOutputDpi NOTIFY outputDpiChanged )
    Q_PROPERTY( QgsCoordinateReferenceSystem destinationCrs READ destinationCrs WRITE setDestinationCrs NOTIFY destinationCrsChanged )
    Q_PROPERTY( QList<QgsMapLayer *> layers READ layers WRITE setLayers NOTIFY layersChanged )

  public:
    QgsQuickMapSettings( QObject *parent = 0 );
    ~QgsQuickMapSettings();

    QgsRectangle extent() const;
    void setExtent( const QgsRectangle &extent );

    void setProject( QgsProject *project );
    QgsProject *project() const;

    Q_INVOKABLE void setCenter( const QgsPoint &center );

    double mapUnitsPerPixel() const;

    QgsRectangle visibleExtent() const;

    /**
     * Returns the coordinate transform context, which stores various
     * information regarding which datum transforms should be used when transforming points
     * from a source to destination coordinate reference system.
     */
    Q_INVOKABLE QgsCoordinateTransformContext transformContext() const;

    /**
     * Convert a map coordinate to screen pixel coordinates
     *
     * @param p A coordinate in map coordinates
     *
     * @return A coordinate in pixel / screen space
     */
    Q_INVOKABLE QPointF coordinateToScreen( const QgsPoint &p ) const;


    /**
     * Convert a screen coordinate to a map coordinate
     *
     * @param p A coordinate in pixel / screen coordinates
     *
     * @return A coordinate in map coordinates
     */
    Q_INVOKABLE QgsPoint screenToCoordinate( const QPointF &p ) const;

    /**
     * Sets the coordinate transform \a context, which stores various
     * information regarding which datum transforms should be used when transforming points
     * from a source to destination coordinate reference system.
     *
     * \since QGIS 3.0
     * \see transformContext()
     */
    void setTransformContext( const QgsCoordinateTransformContext &context );

    double rotation() const;
    void setRotation( double rotation );

    QgsMapSettings mapSettings() const;

    QSize outputSize() const;
    void setOutputSize( const QSize &outputSize );

    double outputDpi() const;
    void setOutputDpi( double outputDpi );

    QgsCoordinateReferenceSystem destinationCrs() const;
    void setDestinationCrs( const QgsCoordinateReferenceSystem &destinationCrs );

    QList<QgsMapLayer *> layers() const;
    void setLayers( const QList<QgsMapLayer *> &layers );

  signals:
    void projectChanged();
    void extentChanged();
    void destinationCrsChanged();
    void mapUnitsPerPixelChanged();
    void rotationChanged();
    void visibleExtentChanged();
    void outputSizeChanged();
    void outputDpiChanged();
    void layersChanged();

  private slots:
    void onReadProject( const QDomDocument &doc );

  private:
    QgsProject *mProject;
    QgsMapSettings mMapSettings;

};

#endif // QGSQUICKMAPSETTINGS_H
