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
 * On top of QgsMapSettings functionality, when QgsProject is attached,
 * it automatically loads its default settings from the project.
 *
 * \note QML Type: MapSettings
 *
 * \sa QgsMapCanvas
 *
 * \since QGIS 3.2
 */
class QUICK_EXPORT QgsQuickMapSettings : public QObject
{
    Q_OBJECT

    //! QGIS project
    Q_PROPERTY( QgsProject *project READ project WRITE setProject NOTIFY projectChanged )
    //! \see QgsMapSettings::extent()
    Q_PROPERTY( QgsRectangle extent READ extent WRITE setExtent NOTIFY extentChanged )
    //! \see QgsMapSettings::visibleExtent()
    Q_PROPERTY( QgsRectangle visibleExtent READ visibleExtent NOTIFY visibleExtentChanged )
    //! \see QgsMapSettings::mapUnitsPerPixel()
    Q_PROPERTY( double mapUnitsPerPixel READ mapUnitsPerPixel NOTIFY mapUnitsPerPixelChanged )
    //! \see QgsMapSettings::rotation()
    Q_PROPERTY( double rotation READ rotation WRITE setRotation NOTIFY rotationChanged )
    //! \see QgsMapSettings::outputSize()
    Q_PROPERTY( QSize outputSize READ outputSize WRITE setOutputSize NOTIFY outputSizeChanged )
    //! \see QgsMapSettings::outputDpi()
    Q_PROPERTY( double outputDpi READ outputDpi WRITE setOutputDpi NOTIFY outputDpiChanged )
    //! \see QgsMapSettings::destinationCrs()
    Q_PROPERTY( QgsCoordinateReferenceSystem destinationCrs READ destinationCrs WRITE setDestinationCrs NOTIFY destinationCrsChanged )
    //! \see QgsMapSettings::layers()
    Q_PROPERTY( QList<QgsMapLayer *> layers READ layers WRITE setLayers NOTIFY layersChanged )

  public:
    //! Create new map settings
    QgsQuickMapSettings( QObject *parent = 0 );
    ~QgsQuickMapSettings();

    //! Clone map settings
    QgsMapSettings mapSettings() const;

    //! \see QgsMapSettings::extent()
    QgsRectangle extent() const;

    //! \see QgsMapSettings::setExtent()
    void setExtent( const QgsRectangle &extent );

    /**
     * Attach \a project with the map settings.
     * When project is loaded, map settings are automatically populated from it's stored values
     */
    void setProject( QgsProject *project );
    //! Return associated QGIS project
    QgsProject *project() const;

    //! Move current map extent to have center point defined by \a center
    Q_INVOKABLE void setCenter( const QgsPoint &center );

    //! \see QgsMapSettings::mapUnitsPerPixel()
    double mapUnitsPerPixel() const;

    //! \see QgsMapSettings::visibleExtent()
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
     * \param p A coordinate in map coordinates
     *
     * \return A coordinate in pixel / screen space
     */
    Q_INVOKABLE QPointF coordinateToScreen( const QgsPoint &p ) const;


    /**
     * Convert a screen coordinate to a map coordinate
     *
     * \param p A coordinate in pixel / screen coordinates
     *
     * \return A coordinate in map coordinates
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

    //! Return rotation
    double rotation() const;

    //! Set rotation
    void setRotation( double rotation );

    /**
     * Return output size
     * \see QgsMapSettings::outputSize()
     */
    QSize outputSize() const;

    /**
     * Set output size and emit outputSizeChanged
     * \see QgsMapSettings::setOutputSize()
     */
    void setOutputSize( const QSize &outputSize );

    /**
     * Return output DPI
     * \see QgsMapSettings::outputDpi()
     */
    double outputDpi() const;

    /**
     * Set output DPI and emit outputDpiChanged
     * \see QgsMapSettings::setOutputDpi()
     */
    void setOutputDpi( double outputDpi );

    /**
     * Return destination CRS
     * \see QgsMapSettings::destinationCrs()
     */
    QgsCoordinateReferenceSystem destinationCrs() const;

    /**
     * Set destination CRS and emit destinationCrsChanged
     * \see QgsMapSettings::setDestinationCrs()
     */
    void setDestinationCrs( const QgsCoordinateReferenceSystem &destinationCrs );

    /**
     * Return layers
     * \see QgsMapSettings::layers()
     */
    QList<QgsMapLayer *> layers() const;

    /**
     * Set layers and emit layersChanged
     * \see QgsMapSettings::setLayers()
     */
    void setLayers( const QList<QgsMapLayer *> &layers );

  signals:
    //! project changed
    void projectChanged();

    //! extent changed
    void extentChanged();

    //! destination CRS changed
    void destinationCrsChanged();

    //! map units per pixel changed
    void mapUnitsPerPixelChanged();

    //! rotation changed
    void rotationChanged();

    //! visible extent changed
    void visibleExtentChanged();

    //! output size changed
    void outputSizeChanged();

    //! output DPI changed
    void outputDpiChanged();

    //! layers changed
    void layersChanged();

  private slots:

    /**
     * Read map canvas settings stored in a QGIS project file
     *
     * \param doc parsed DOM of a QgsProject
     */
    void onReadProject( const QDomDocument &doc );

  private:
    QgsProject *mProject = nullptr;
    QgsMapSettings mMapSettings;

};

#endif // QGSQUICKMAPSETTINGS_H
