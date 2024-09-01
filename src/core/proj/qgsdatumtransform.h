/***************************************************************************
               qgsdatumtransform.h
               ------------------------
    begin                : Dec 2017
    copyright            : (C) 2017 Nyall Dawson
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
#ifndef QGSDATUMTRANSFORM_H
#define QGSDATUMTRANSFORM_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsrectangle.h"
#include <QString>
#include <QList>

class QgsCoordinateReferenceSystem;

#ifndef SIP_RUN
struct PJconsts;
typedef struct PJconsts PJ;
#endif

/**
 * \brief Contains methods and classes relating the datum transformations.
 * \ingroup core
 *
 * \see QgsCoordinateTransformContext
 * \see QgsCoordinateTransform
 *
 */
class CORE_EXPORT QgsDatumTransform
{

  public:

    /**
     * Contains datum transform information.
     * \deprecated QGIS 3.40. Not used for builds based on Proj >= 6.0.
     */
    struct TransformPair
    {

      /**
       * Constructor for a TransformPair with the specified \a sourceTransformId
       * and \a destinationTransformId transforms.
       */
      TransformPair( int sourceTransformId = -1, int destinationTransformId = -1 )
        : sourceTransformId( sourceTransformId )
        , destinationTransformId( destinationTransformId )
      {}

      /**
        * ID for the datum transform to use when projecting from the source CRS.
        * \see QgsCoordinateTransform::datumTransformCrsInfo()
       */
      int sourceTransformId = -1;

      /**
       * ID for the datum transform to use when projecting to the destination CRS.
       * \see QgsCoordinateTransform::datumTransformCrsInfo()
       */
      int destinationTransformId = -1;

      // TODO c++20 - replace with = default
      bool operator==( QgsDatumTransform::TransformPair other ) const
      {
        return other.sourceTransformId == sourceTransformId && other.destinationTransformId == destinationTransformId;
      }

      bool operator!=( QgsDatumTransform::TransformPair other ) const
      {
        return other.sourceTransformId != sourceTransformId || other.destinationTransformId != destinationTransformId;
      }

    };

    /**
     * Contains datum transform information.
     * \deprecated QGIS 3.40. Not used on builds based on Proj >= 6.0.
     */
    struct TransformInfo
    {
      //! Datum transform ID
      int datumTransformId = -1;

      //! EPSG code for the transform, or 0 if not found in EPSG database
      int epsgCode = 0;

      //! Source CRS auth ID
      QString sourceCrsAuthId;

      //! Destination CRS auth ID
      QString destinationCrsAuthId;

      //! Source CRS description
      QString sourceCrsDescription;

      //! Destination CRS description
      QString destinationCrsDescription;

      //! Transform remarks
      QString remarks;

      //! Scope of transform
      QString scope;

      //! True if transform is the preferred transform to use for the source/destination CRS combination
      bool preferred = false;

      //! True if transform is deprecated
      bool deprecated = false;

    };


    /**
     * Contains information about a projection transformation grid file.
     * \since QGIS 3.8
     */
    struct GridDetails
    {
      //! Short name of transform grid
      QString shortName;
      //! Full name of transform grid
      QString fullName;
      //! Name of package the grid is included within
      QString packageName;
      //! Url to download grid from
      QString url;
      //! TRUE if direct download of grid is possible
      bool directDownload = false;
      //! TRUE if grid is available under an open license
      bool openLicense = false;
      //! TRUE if grid is currently available for use
      bool isAvailable = false;
    };

    /**
     * Contains information about a single coordinate operation.
     *
     * \note Only used in builds based on on Proj >= 6.2
     * \since QGIS 3.10
     */
    struct SingleOperationDetails
    {
      //! Scope of operation, from EPSG registry database
      QString scope;

      //! Remarks for operation, from EPSG registry database
      QString remarks;

      //! Area of use, from EPSG registry database
      QString areaOfUse;

      //! Authority name, e.g. EPSG.
      QString authority;

      //! Authority code, e.g. "8447" (for EPSG:8447).
      QString code;
    };

    /**
     * Contains information about a coordinate transformation operation.
     *
     * \note Only used in builds based on on Proj >= 6.0
     * \since QGIS 3.8
     */
    struct TransformDetails
    {
      //! Proj representation of transform operation
      QString proj;
      //! Display name of transform operation
      QString name;
      //! Transformation accuracy (in meters)
      double accuracy = 0;

      /**
       * Authority name, e.g. EPSG.
       *
       * This is only available for single step coordinate operations. For multi-step operations, check
       * \a operationDetails instead.
       */
      QString authority;

      /**
       * Identification code, e.g. "8447" (For EPSG:8447).
       *
       * This is only available for single step coordinate operations. For multi-step operations, check
       * \a operationDetails instead.
       */
      QString code;

      /**
       * Scope of operation, from EPSG registry database.
       *
       * This is only available for single step coordinate operations. For multi-step operations, check
       * \a operationDetails instead.
       */
      QString scope;

      /**
      * Remarks for operation, from EPSG registry database.
      *
      * This is only available for single step coordinate operations. For multi-step operations, check
      * \a operationDetails instead.
      */
      QString remarks;

      /**
       * TRUE if operation is available.
       *
       * If FALSE, it likely means a transform grid is required which is not
       * available.
       */
      bool isAvailable = false;

      /**
       * Area of use string.
       *
       * This is only available for single step coordinate operations. For multi-step operations, check
       * \a operationDetails instead.
       *
       * \see bounds
       */
      QString areaOfUse;

      /**
       * Valid bounds for the coordinate operation.
       * \see areaOfUse
       */
      QgsRectangle bounds;

      /**
       * Contains a list of transform grids used by the operation.
       */
      QList< QgsDatumTransform::GridDetails > grids;

      /**
       * Contains information about the single operation steps used in the transform operation.
       *
       * \note Only used in builds based on on Proj >= 6.2
       * \since QGIS 3.10
       */
      QList< QgsDatumTransform::SingleOperationDetails > operationDetails;
    };

    /**
     * Returns a list of coordinate operations available for transforming
     * coordinates from the \a source to \a destination CRS.
     *
     * This list is sorted in order of preference, with the most preferable
     * operation listed first.
     *
     * Not all operations may be available for use. Check QgsDatumTransform::TransformDetails::isAvailable
     * first. Operations may require grid shift files which are not available on the local
     * install.
     *
     * If \a includeSuperseded is TRUE, superseded (but not deprecated) transforms will be included
     * in the results. This requires Proj >= 6.2.
     *
     * \note Requires Proj 6.0 or later. Builds based on earlier Proj versions will always return an empty list,
     * and the deprecated API from QgsDatumTransform must be used instead.
     *
     * \since QGIS 3.8
     */
    static QList< QgsDatumTransform::TransformDetails > operations( const QgsCoordinateReferenceSystem &source, const QgsCoordinateReferenceSystem &destination, bool includeSuperseded = false );

    /**
     * Returns a list of datum transformations which are available for the given \a source and \a destination CRS.
     * \see datumTransformToProj()
     * \see datumTransformInfo()
     *
     * \deprecated QGIS 3.40. Not used for builds based on Proj >= 6.0.
     */
    Q_DECL_DEPRECATED static QList< QgsDatumTransform::TransformPair > datumTransformations( const QgsCoordinateReferenceSystem &source, const QgsCoordinateReferenceSystem &destination ) SIP_DEPRECATED;

    /**
     * Returns a proj string representing the specified \a datumTransformId datum transform ID.
     * \see datumTransformations()
     * \see datumTransformInfo()
     * \see projStringToDatumTransformId()
     *
     * \deprecated QGIS 3.40. Not used for builds based on Proj >= 6.0.
     */
    Q_DECL_DEPRECATED static QString datumTransformToProj( int datumTransformId ) SIP_DEPRECATED;

    /**
     * Returns the datum transform ID corresponding to a specified proj \a string.
     * Returns -1 if matching datum ID was not found.
     * \see datumTransformToProj()
     *
     * \deprecated QGIS 3.40. Not used for builds based on Proj >= 6.0.
     */
    Q_DECL_DEPRECATED static int projStringToDatumTransformId( const QString &string ) SIP_DEPRECATED;

    /**
     * Returns detailed information about the specified \a datumTransformId.
     * If \a datumTransformId was not a valid transform ID, a TransformInfo with TransformInfo::datumTransformId of
     * -1 will be returned.
     * \see datumTransformations()
     * \see datumTransformToProj()
     *
     * \deprecated QGIS 3.40. Not used for builds based on Proj >= 6.0.
    */
    Q_DECL_DEPRECATED static QgsDatumTransform::TransformInfo datumTransformInfo( int datumTransformId ) SIP_DEPRECATED;

#ifndef SIP_RUN

    /**
     * Returns the transform details for a Proj coordinate operation \a op.
     *
     * \note Requires Proj 6.0 or later
     * \since QGIS 3.8
     */
    static QgsDatumTransform::TransformDetails transformDetailsFromPj( PJ *op );
#endif

  private:

    static void searchDatumTransform( const QString &sql, QList< int > &transforms );


};

#endif // QGSDATUMTRANSFORM_H
