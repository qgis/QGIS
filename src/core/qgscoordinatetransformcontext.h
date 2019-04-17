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
#include "qgis_sip.h"
#include "qgsdatumtransform.h"

#include <QMetaType>
#include <QExplicitlySharedDataPointer>
class QgsCoordinateReferenceSystem;
class QgsReadWriteContext;
class QgsCoordinateTransformContextPrivate;
class QDomElement;

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in testqgsfeature.cpp.
 * See details in QEP #17
 ****************************************************************************/

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
 * \note QgsCoordinateTransformContext objects are thread safe for read and write.
 *
 * \note QgsCoordinateTransformContext objects are implicitly shared.
 *
 * \see QgsDatumTransform
 * \see QgsCoordinateTransform
 *
 * \since QGIS 3.0
*/

class CORE_EXPORT QgsCoordinateTransformContext
{
  public:

    /**
     * Constructor for QgsCoordinateTransformContext.
     */
    QgsCoordinateTransformContext();

    ~QgsCoordinateTransformContext() ;

    /**
     * Copy constructor
     */
    QgsCoordinateTransformContext( const QgsCoordinateTransformContext &rhs );

    /**
     * Assignment operator
     */
    QgsCoordinateTransformContext &operator=( const QgsCoordinateTransformContext &rhs ) SIP_SKIP;

    bool operator==( const QgsCoordinateTransformContext &rhs ) const ;

    /**
     * Clears all stored transform information from the context.
     */
    void clear();


#if 0
//singlesourcedest

    /**
     * Returns the stored mapping for source CRS to associated datum transform to use.
     * The map keys will be QgsCoordinateReferenceSystems::authid()s.
     *
     * A datum transform of -1 indicates that no datum transform is required for the
     * source CRS.
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
     * A datum \a transform of -1 indicates that no datum transform is required for the
     * source CRS.
     *
     * Returns TRUE if the new transform was added successfully.
     *
     * \warning Transforms set using this method may be overridden by specific source/destination
     * transforms set by addSourceDestinationDatumTransform().
     *
     * \see sourceDatumTransforms()
     * \see addDestinationDatumTransform()
     * \see removeSourceDatumTransform()
     */
    bool addSourceDatumTransform( const QgsCoordinateReferenceSystem &crs, int transform );

    /**
     * Removes the source datum transform for the specified \a crs.
     * \see addSourceDatumTransform()
     * \see removeDestinationDatumTransform()
     */
    void removeSourceDatumTransform( const QgsCoordinateReferenceSystem &crs );

    /**
     * Returns the stored mapping for destination CRS to associated datum transform to use.
     * The map keys will be QgsCoordinateReferenceSystems::authid()s.
     *
     * A datum transform of -1 indicates that no datum transform is required for the
     * destination CRS.
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
     * A datum \a transform of -1 indicates that no datum transform is required for the
     * destination CRS.
     *
     * Returns TRUE if the new transform was added successfully.
     *
     * \warning Transforms set using this method may be overridden by specific source/destination
     * transforms set by addSourceDestinationDatumTransform().
     *
     * \see destinationDatumTransforms()
     * \see addSourceDatumTransform()
     * \see removeDestinationDatumTransform()
     */
    bool addDestinationDatumTransform( const QgsCoordinateReferenceSystem &crs, int transform );

    /**
     * Removes the destination datum transform for the specified \a crs.
     * \see addDestinationDatumTransform()
     * \see removeSourceDatumTransform()
     */
    void removeDestinationDatumTransform( const QgsCoordinateReferenceSystem &crs );

#endif

    /**
     * Returns the stored mapping for source to destination CRS pairs to associated datum transforms to use.
     * The map keys will be QgsCoordinateReferenceSystems::authid()s.
     *
     * If either the source transform ID or destination transform ID is -1, then no datum transform is
     * required for transformations for that source or destination.
     *
     * \warning This method should not be used to calculate the corresponding datum transforms
     * to use for a coordinate transform. Instead, always use calculateDatumTransforms()
     * to determine this.
     *
     * \see addSourceDestinationDatumTransform()
     */
    QMap< QPair< QString, QString>, QgsDatumTransform::TransformPair > sourceDestinationDatumTransforms() const;

    /**
     * Adds a new \a sourceTransform and \a destinationTransform to use when projecting coordinates
     * from the specified \a sourceCrs to the specified \a destinationCrs.
     *
     * If either \a sourceTransformId or \a destinationTransformId is -1, then no datum transform is
     * required for transformations for that source or destination.
     *
     * Returns TRUE if the new transform pair was added successfully.
     *
     * \note Transforms set using this method will override any specific source or destination
     * transforms set by addSourceDatumTransform() or addDestinationDatumTransform().
     *
     * \see sourceDestinationDatumTransforms()
     * \see removeSourceDestinationDatumTransform()
     */
    bool addSourceDestinationDatumTransform( const QgsCoordinateReferenceSystem &sourceCrs,
        const QgsCoordinateReferenceSystem &destinationCrs,
        int sourceTransformId,
        int destinationTransformId );

    /**
     * Removes the source to destination datum transform pair for the specified \a sourceCrs and
     * \a destinationCrs.
     * \see addSourceDestinationDatumTransform()
     */
    void removeSourceDestinationDatumTransform( const QgsCoordinateReferenceSystem &sourceCrs,
        const QgsCoordinateReferenceSystem &destinationCrs );

    /**
     * Returns TRUE if the context has a valid datum transform to use
     * when transforming from the specified \a source CRS to \a destination CRS.
     * \note source and destination are reversible.
     */
    bool hasTransform( const QgsCoordinateReferenceSystem &source,
                       const QgsCoordinateReferenceSystem &destination ) const;

    /**
     * Returns the pair of source and destination datum transforms to use
     * for a transform from the specified \a source CRS to \a destination CRS.
     *
     * Returns an ID of -1 if a datum transform should not be used for the source or
     * destination.
     *
     * \note source and destination are reversible.
     */
    QgsDatumTransform::TransformPair calculateDatumTransforms( const QgsCoordinateReferenceSystem &source,
        const QgsCoordinateReferenceSystem &destination ) const;

    /**
     * Reads the context's state from a DOM \a element.
     *
     * Returns FALSE if transforms stored in the XML are not available. In this case \a missingTransforms will be
     * filled with missing datum transform strings.
     *
     * \see writeXml()
     */
    bool readXml( const QDomElement &element, const QgsReadWriteContext &context, QStringList &missingTransforms SIP_OUT );

    /**
     * Writes the context's state to a DOM \a element.
     * \see readXml()
     */
    void writeXml( QDomElement &element, const QgsReadWriteContext &context ) const;


    /**
     * Reads the context's state from application settings.
     * \see readSettings()
     */
    void readSettings();

    /**
     * Write the context's state to application settings.
     * \see writeSettings()
     */
    void writeSettings();


  private:

    QExplicitlySharedDataPointer<QgsCoordinateTransformContextPrivate> d;

};

Q_DECLARE_METATYPE( QgsCoordinateTransformContext )

#endif // QGSCOORDINATETRANSFORMCONTEXT_H




