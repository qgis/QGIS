/***************************************************************************
                         qgscoordinatetransformcontext.h
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

#ifndef QGSCOORDINATETRANSFORMCONTEXT_H
#define QGSCOORDINATETRANSFORMCONTEXT_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgscoordinatereferencesystem.h"

/**
 * \class QgsCoordinateTransformContext
 * \ingroup core
 * Contains information about the context in which a coordinate transform is executed.
 *
 * The context stores various information regarding which coordinate transforms should
 * be used when transforming points from a source to destination coordinate reference
 * system.
 *
 * The highest priority transforms are those set using addSourceDestinationDatumTransform()
 * and which the transform has a matching source to destination CRS pair.
 *
 * Failing this, if the source CRS has a matching transform specified by
 * addSourceDatumTransform() then this datum transform will be used. The same logic
 * applies for destination CRS transforms set using addDestinationDatumTransform().
 *
 * \since QGIS 3.0
*/

class CORE_EXPORT QgsCoordinateTransformContext
{
  public:

    /**
     * Constructor for QgsCoordinateTransformContext.
     */
    QgsCoordinateTransformContext() = default;

    /**
     * Clears all stored transform information from the context.
     */
    void clear();

    /**
     * Returns the stored mapping for source CRS to associated datum transform to use.
     * The map keys will be QgsCoordinateReferenceSystems::authid()s.
     *
     * \warning This method should not be used to calculate the corresponding datum transforms
     * to use for a coordinate transform. Instead, always use calculateDatumTransforms()
     * to determine this.
     *
     * \see addSourceDatumTransform()
     * \see destinationDatumTransforms()
     */
    QMap<QString, int> sourceDatumTransforms() const;

    /**
     * Adds a new \a transform to use when projecting coordinates from the specified source
     * \a crs.
     *
     * If \a transform is -1, then any existing source transform for the \a crs will be removed.
     *
     * Returns true if the new transform was added successfully.
     *
     * \warning Transforms set using this method may be overridden by specific source/destination
     * transforms set by addSourceDestinationDatumTransform().
     *
     * \see sourceDatumTransforms()
     * \see addDestinationDatumTransform()
     */
    bool addSourceDatumTransform( const QgsCoordinateReferenceSystem &crs, int transform );

    /**
     * Returns the stored mapping for destination CRS to associated datum transform to use.
     * The map keys will be QgsCoordinateReferenceSystems::authid()s.
     *
     * \warning This method should not be used to calculate the corresponding datum transforms
     * to use for a coordinate transform. Instead, always use calculateDatumTransforms()
     * to determine this.
     *
     * \see addDestinationDatumTransform()
     * \see sourceDatumTransforms()
     */
    QMap< QString, int > destinationDatumTransforms() const;

    /**
     * Adds a new \a transform to use when projecting coordinates to the specified destination
     * \a crs.
     *
     * If \a transform is -1, then any existing destination transform for the \a crs will be removed.
     *
     * Returns true if the new transform was added successfully.
     *
     * \warning Transforms set using this method may be overridden by specific source/destination
     * transforms set by addSourceDestinationDatumTransform().
     *
     * \see destinationDatumTransforms()
     * \see addSourceDatumTransform()
     */
    bool addDestinationDatumTransform( const QgsCoordinateReferenceSystem &crs, int transform );

    /**
     * Returns the stored mapping for source to destination CRS pairs to associated datum transforms to use.
     * The map keys will be QgsCoordinateReferenceSystems::authid()s.
     *
     * \warning This method should not be used to calculate the corresponding datum transforms
     * to use for a coordinate transform. Instead, always use calculateDatumTransforms()
     * to determine this.
     *
     * \see addSourceDestinationDatumTransform()
     */
    QMap< QPair< QString, QString>, QPair< int, int > > sourceDestinationDatumTransforms() const;

    /**
     * Adds a new \a sourceTransform and \a destinationTransform to use when projecting coordinates
     * from the the specified \a sourceCrs to the specified \a destinationCrs.
     *
     * If either \a sourceTransform or \a destinationTransform is -1, then any existing source to destination
     * transform for the crs pair will be removed.
     *
     * Returns true if the new transform pair was added successfully.
     *
     * \note Transforms set using this method will override any specific source or destination
     * transforms set by addSourceDatumTransform() or addDestinationDatumTransform().
     *
     * \see sourceDestinationDatumTransforms()
     */
    bool addSourceDestinationDatumTransform( const QgsCoordinateReferenceSystem &sourceCrs,
        const QgsCoordinateReferenceSystem &destinationCrs,
        int sourceTransform,
        int destinationTransform );

    /**
     * Returns the pair of source and destination datum transforms to use
     * for a transform from the specified \a source CRS to \a destination CRS.
     *
     * Returns -1 if a datum transform should not be used for the source or
     * destination.
     */
    QPair< int, int > calculateDatumTransforms( const QgsCoordinateReferenceSystem &source,
        const QgsCoordinateReferenceSystem &destination ) const;

  private:

    /**
     * Mapping for datum transforms to use for source/destination CRS pairs.
     * Matching records from this map will take precedence over other transform maps.
     */
    QMap< QPair< QString, QString >, QPair< int, int > > mSourceDestDatumTransforms;

    //! Mapping for datum transforms to use for source CRS
    QMap< QString, int > mSourceDatumTransforms;

    //! Mapping for datum transforms to use for destination CRS
    QMap< QString, int > mDestDatumTransforms;

};

#endif // QGSCOORDINATETRANSFORMCONTEXT_H




