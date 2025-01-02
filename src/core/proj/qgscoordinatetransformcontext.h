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
 * \brief Contains information about the context in which a coordinate transform is executed.
 *
 * The context stores various information regarding which coordinate operations should
 * be used when transforming points from a source to destination coordinate reference
 * system.
 *
 * \note QgsCoordinateTransformContext objects are thread safe for read and write.
 *
 * \note QgsCoordinateTransformContext objects are implicitly shared.
 *
 * \see QgsDatumTransform
 * \see QgsCoordinateTransform
 *
*/

class CORE_EXPORT QgsCoordinateTransformContext
{
  public:

    /**
     * Constructor for QgsCoordinateTransformContext.
     */
    QgsCoordinateTransformContext();

    ~QgsCoordinateTransformContext() ;

    QgsCoordinateTransformContext( const QgsCoordinateTransformContext &rhs );
    QgsCoordinateTransformContext &operator=( const QgsCoordinateTransformContext &rhs ) SIP_SKIP;

    bool operator==( const QgsCoordinateTransformContext &rhs ) const;
    bool operator!=( const QgsCoordinateTransformContext &rhs ) const;

    /**
     * Clears all stored transform information from the context.
     */
    void clear();

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
     *
     * \deprecated QGIS 3.40. Has no effect on builds based on Proj 6.0 or later, use coordinateOperations() instead.
     */
    Q_DECL_DEPRECATED QMap< QPair< QString, QString>, QgsDatumTransform::TransformPair > sourceDestinationDatumTransforms() const SIP_DEPRECATED;

    /**
     * Returns the stored mapping for source to destination CRS pairs to associated coordinate operation to use
     * (as a proj string). The map keys will be QgsCoordinateReferenceSystems::authid()s.
     *
     * \warning This method should not be used to calculate the corresponding coordinate operation
     * to use for a coordinate transform. Instead, always use calculateCoordinateOperation()
     * to determine this.
     *
     * \see addCoordinateOperation()
     *
     * \note Requires Proj 6.0 or later. Builds based on earlier Proj versions will always return an empty list,
     * and the deprecated sourceDestinationDatumTransforms() method must be used instead.
     *
     * \since QGIS 3.8
     */
    QMap< QPair< QString, QString>, QString > coordinateOperations() const;

    /**
     * Adds a new \a sourceTransform and \a destinationTransform to use when projecting coordinates
     * from the specified \a sourceCrs to the specified \a destinationCrs.
     *
     * If either \a sourceTransformId or \a destinationTransformId is -1, then no datum transform is
     * required for transformations for that source or destination.
     *
     * Returns TRUE if the new transform pair was added successfully.
     *
     * \see sourceDestinationDatumTransforms()
     * \see removeSourceDestinationDatumTransform()
     *
     * \deprecated QGIS 3.40. Has no effect on builds based on Proj 6.0 or later, use addCoordinateOperation() instead.
     */
    Q_DECL_DEPRECATED bool addSourceDestinationDatumTransform( const QgsCoordinateReferenceSystem &sourceCrs, const QgsCoordinateReferenceSystem &destinationCrs, int sourceTransformId, int destinationTransformId ) SIP_DEPRECATED;

    /**
     * Adds a new \a coordinateOperationProjString to use when projecting coordinates
     * from the specified \a sourceCrs to the specified \a destinationCrs.
     *
     * \a coordinateOperationProjString should be set to a valid Proj coordinate operation
     * string. If \a coordinateOperationProjString is empty, then the default Proj operation
     * will be used when transforming between the coordinate reference systems.
     *
     * If \a allowFallback is TRUE (since QGIS 3.12), then "ballpark" fallback transformations
     * will be used in the case that the specified coordinate operation fails (such as when
     * coordinates from outside a required grid shift file are transformed). See
     * QgsCoordinateTransform::fallbackOperationOccurred() for further details. Note that if an
     * existing \a sourceCrs and \a destinationCrs pair are added with a different \a allowFallback
     * value, that value will replace the existing one (i.e. each combination of \a sourceCrs and
     * \a destinationCrs must be unique).
     *
     * \warning coordinateOperationProjString MUST be a proj string which has been normalized for
     * visualization, and must be constructed so that coordinates are always input and output
     * with x/y coordinate ordering. (Proj strings output by utilities such as projinfo will NOT
     * automatically normalize the axis order!).
     *
     * Returns TRUE if the new coordinate operation was added successfully.
     *
     * \see coordinateOperations()
     * \see removeCoordinateOperation()
     *
     * \note Requires Proj 6.0 or later. Builds based on earlier Proj versions will ignore this setting,
     * and the deprecated addSourceDestinationDatumTransform() method must be used instead.
     *
     * \since QGIS 3.8
     */
    bool addCoordinateOperation( const QgsCoordinateReferenceSystem &sourceCrs, const QgsCoordinateReferenceSystem &destinationCrs, const QString &coordinateOperationProjString, bool allowFallback = true );

    /**
     * Removes the source to destination datum transform pair for the specified \a sourceCrs and
     * \a destinationCrs.
     * \see addSourceDestinationDatumTransform()
     *
     * \deprecated QGIS 3.40. Use removeCoordinateOperation() instead.
     */
    Q_DECL_DEPRECATED void removeSourceDestinationDatumTransform( const QgsCoordinateReferenceSystem &sourceCrs, const QgsCoordinateReferenceSystem &destinationCrs ) SIP_DEPRECATED ;

    /**
     * Removes the coordinate operation for the specified \a sourceCrs and \a destinationCrs.
     *
     * \since QGIS 3.8
     */
    void removeCoordinateOperation( const QgsCoordinateReferenceSystem &sourceCrs, const QgsCoordinateReferenceSystem &destinationCrs );

    /**
     * Returns TRUE if the context has a valid coordinate operation to use
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
     * \deprecated QGIS 3.40. Has no effect on builds based on Proj 6.0 or later. Use calculateCoordinateOperation() instead.
     */
    Q_DECL_DEPRECATED QgsDatumTransform::TransformPair calculateDatumTransforms( const QgsCoordinateReferenceSystem &source, const QgsCoordinateReferenceSystem &destination ) const SIP_DEPRECATED;

    /**
     * Returns the Proj coordinate operation string to use when transforming
     * from the specified \a source CRS to \a destination CRS.
     *
     * Returns an empty string if no specific coordinate operation is set for the source to
     * destination pair, in which case the default Proj coordinate operation should
     * be used.
     *
     * \note source and destination are reversible.
     *
     * \note Requires Proj 6.0 or later. Builds based on earlier Proj versions will always return
     * an empty string, and the deprecated calculateDatumTransforms() method should be used instead.
     *
     * \warning Always check the result of mustReverseCoordinateOperation() in order to determine if the
     * proj coordinate operation string returned by this method corresponds to the reverse operation, and
     * must be manually flipped when calculating coordinate transforms.
     *
     * \since QGIS 3.8
     */
    QString calculateCoordinateOperation( const QgsCoordinateReferenceSystem &source, const QgsCoordinateReferenceSystem &destination ) const;

    /**
     * Returns TRUE if approximate "ballpark" transforms may be used when transforming
     * between a \a source and \a destination CRS pair, in the case that the preferred
     * coordinate operation fails (such as when
     * coordinates from outside a required grid shift file are transformed). See
     * QgsCoordinateTransform::fallbackOperationOccurred() for further details.
     *
     * \since QGIS 3.12
     */
    bool allowFallbackTransform( const QgsCoordinateReferenceSystem &source, const QgsCoordinateReferenceSystem &destination ) const;

    /**
     * Returns TRUE if the coordinate operation returned by calculateCoordinateOperation() for the \a source to \a destination pair
     * must be inverted.
     *
     * \since QGIS 3.10.2
     */
    bool mustReverseCoordinateOperation( const QgsCoordinateReferenceSystem &source, const QgsCoordinateReferenceSystem &destination ) const;

    // TODO QGIS 4.0 - remove missingTransforms, not used for Proj >= 6.0 builds

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




