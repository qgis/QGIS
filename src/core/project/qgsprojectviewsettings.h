/***************************************************************************
    qgsprojectviewsettings.h
    ---------------------------
    begin                : October 2019
    copyright            : (C) 2019 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSPROJECTVIEWSETTINGS_H
#define QGSPROJECTVIEWSETTINGS_H

#include "qgis_core.h"
#include "qgsreferencedgeometry.h"
#include <QObject>
#include <QVector>

class QDomElement;
class QgsReadWriteContext;
class QDomDocument;
class QgsProject;

/**
 * \brief Contains settings and properties relating to how a QgsProject should be displayed inside
 * map canvas, e.g. map scales and default view extent for the project.
 *
 * \ingroup core
 * \since QGIS 3.10.1
 */
class CORE_EXPORT QgsProjectViewSettings : public QObject
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsProjectViewSettings for the specified \a project.
     *
     * Ownership is transferred to the \a project.
     */
    QgsProjectViewSettings( QgsProject *project = nullptr );

    /**
     * Resets the settings to a default state.
     */
    void reset();

    /**
     * Returns the default view extent, which should be used as the initial map extent
     * when this project is opened.
     *
     * \warning When a project is opened in the QGIS desktop application and saved, individual
     * map canvases will store their own previous view extent as custom project properties. Reloading
     * this saved version of the project will trigger the canvases to restore their individual last view
     * extents. Accordingly, in the QGIS desktop application, this setting only forms a default, initial
     * view used when the project is opened for the very first time.
     *
     * \see setDefaultViewExtent()
     */
    QgsReferencedRectangle defaultViewExtent() const;

    /**
     * Sets the default view \a extent, which should be used as the initial map extent
     * when this project is opened.
     *
     * \warning When a project is opened in the QGIS desktop application and saved, individual
     * map canvases will store their own previous view extent as custom project properties. Reloading
     * this saved version of the project will trigger the canvases to restore their individual last view
     * extents. Accordingly, in the QGIS desktop application, this setting only forms a default, initial
     * view used when the project is opened for the very first time.
     *
     * \see defaultViewExtent()
     */
    void setDefaultViewExtent( const QgsReferencedRectangle &extent );

    /**
     * Returns the project's preset full extent.
     *
     * This extent represents the maximal limits of the project. The full extent defaults to a null rectangle,
     * which indicates that the maximal extent should be calculated based on the layers in the project.
     *
     * \see setPresetFullExtent()
     * \see fullExtent()
     *
     * \since QGIS 3.18
     */
    QgsReferencedRectangle presetFullExtent() const;

    /**
     * Sets the project's preset full \a extent.
     *
     * This extent represents the maximal limits of the project. Setting the full \a extent to a null rectangle
     * indicates that the maximal extent should be calculated based on the layers in the project.
     *
     * \see setPresetFullExtent()
     * \see fullExtent()
     *
     * \since QGIS 3.18
     */
    void setPresetFullExtent( const QgsReferencedRectangle &extent );

    /**
     * Returns the full extent of the project, which represents the maximal limits of the project.
     *
     * The returned extent will be in the project's CRS.
     *
     * If the presetFullExtent() is non null, this will be returned as the full extent.
     * Otherwise the full extent will be calculated based on the extent of all layers
     * in the project.
     *
     * \see presetFullExtent()
     * \see setPresetFullExtent()
     */
    QgsReferencedRectangle fullExtent() const;

    /**
     * Sets the list of custom project map \a scales.
     *
     * The \a scales list consists of a list of scale denominator values, e.g.
     * 1000 for a 1:1000 scale.
     *
     * \see mapScales()
     * \see mapScalesChanged()
     */
    void setMapScales( const QVector<double> &scales );

    /**
     * Returns the list of custom project map scales.
     *
     * The scales list consists of a list of scale denominator values, e.g.
     * 1000 for a 1:1000 scale.
     *
     * \see setMapScales()
     * \see mapScalesChanged()
     */
    QVector<double> mapScales() const;

    /**
     * Sets whether project mapScales() are \a enabled.
     *
     * When project map scales are enabled these scales will replace the default QGIS map scales list
     * while working with this project.
     *
     * \see useProjectScales()
     * \see setMapScales()
     */
    void setUseProjectScales( bool enabled );

    /**
     * Returns TRUE if project mapScales() are enabled.
     *
     * When project map scales are enabled these scales will replace the default QGIS map scales list
     * while working with this project.
     *
     * \see setUseProjectScales()
     * \see mapScales()
     */
    bool useProjectScales() const;

    /**
     * Returns the default map rotation (in clockwise degrees) for maps in the project.
     *
     * \warning When a project is opened in the QGIS desktop application and saved, individual
     * map canvases will store their own previous map rotations as custom project properties. Reloading
     * this saved version of the project will trigger the canvases to restore their individual rotations.
     * Accordingly, in the QGIS desktop application, this setting only forms a default, initial
     * view used when the project is opened for the very first time (or when new map canvases are opened in
     * that project.)
     *
     * \see setDefaultRotation()
     * \since QGIS 3.28
     */
    double defaultRotation() const;

    /**
     * Set the default \a rotation of maps in the project, in clockwise degrees.
     *
     * \warning When a project is opened in the QGIS desktop application and saved, individual
     * map canvases will store their own previous map rotations as custom project properties. Reloading
     * this saved version of the project will trigger the canvases to restore their individual rotations.
     * Accordingly, in the QGIS desktop application, this setting only forms a default, initial
     * view used when the project is opened for the very first time (or when new map canvases are opened in
     * that project.)
     *
     * \see defaultRotation()
     * \since QGIS 3.28
     */
    void setDefaultRotation( double rotation );

    /**
     * Reads the settings's state from a DOM element.
     * \see writeXml()
     */
    bool readXml( const QDomElement &element, const QgsReadWriteContext &context );

    /**
     * Returns a DOM element representing the settings.
     * \see readXml()
     */
    QDomElement writeXml( QDomDocument &doc, const QgsReadWriteContext &context ) const;

  signals:

    /**
     * Emitted when the list of custom project map scales changes.
     *
     * \see mapScales()
     * \see setMapScales()
     */
    void mapScalesChanged();

    /**
     * Emitted whenever the presetFullExtent() is changed.
     *
     * \since QGIS 3.18
     */
    void presetFullExtentChanged();

  private:

    QgsProject *mProject = nullptr;
    QVector<double> mMapScales;
    bool mUseProjectScales = false;
    QgsReferencedRectangle mDefaultViewExtent;
    QgsReferencedRectangle mPresetFullExtent;
    double mDefaultRotation = 0;
};

#endif // QGSPROJECTVIEWSETTINGS_H
