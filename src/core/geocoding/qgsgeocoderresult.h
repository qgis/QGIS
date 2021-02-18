/***************************************************************************
  qgsgeocoderresult.h
  ---------------
  Date                 : August 2020
  Copyright            : (C) 2020 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGEOCODERRESULT_H
#define QGSGEOCODERRESULT_H

#include "qgis_core.h"

#include "qgscoordinatereferencesystem.h"
#include "qgsgeometry.h"


/**
 * \ingroup core
 * \brief Represents a matching result from a geocoder search.
 *
 * QgsGeocoderResult objects may represent valid matches, or an invalid
 * error result. This can be checked by testing isValid(). An invalid
 * result represents that an error was encountered while geocoding, in which case the
 * error message can be retrieved by calling error().
 *
 * Valid geocoding results will have a geometry() and crs(), along with a set
 * of optional additionalAttributes() which may contain information such as the
 * accuracy of the geocoding result.
 *
 * \since QGIS 3.18
*/
class CORE_EXPORT QgsGeocoderResult
{

  public:

    /**
     * Creates an invalid error result, with the specified \a errorMessage string.
     */
    static QgsGeocoderResult errorResult( const QString &errorMessage );

    /**
     * Constructor for a valid QgsGeocoderResult, with the specified \a geometry and \a crs.
     */
    QgsGeocoderResult( const QString &identifier, const QgsGeometry &geometry, const QgsCoordinateReferenceSystem &crs );

    /**
     * Returns TRUE if the result is a valid result.
     *
     * If the result is invalid, the error message can be retrieved by calling error().
     */
    bool isValid() const { return mIsValid; }

    /**
     * Returns the error string, if the result is invalid.
     */
    QString error() const { return mErrorString; }

    /**
     * Returns the identifier string for the result.
     */
    QString identifier() const { return mIdentifier; }

    /**
     * Returns the resultant geometry resulting from the geocoding operation.
     *
     * The coordinate reference system for the geometry can be retrieved
     * via crs().
     *
     * \see setGeometry()
     * \see crs()
     */
    QgsGeometry geometry() const { return mGeometry; }

    /**
     * Sets the resultant \a geometry resulting from the geocoding operation.
     *
     * The coordinate reference system for the geometry should also be set
     * via setCrs().
     *
     * \see geometry()
     * \see setCrs()
     */
    void setGeometry( const QgsGeometry &geometry ) { mGeometry = geometry; }

    /**
     * Returns the coordinate reference system for the calculated geometry().
     *
     * \see setCrs()
     * \see geometry()
     */
    QgsCoordinateReferenceSystem crs() const { return mCrs; }

    /**
     * Sets the coordinate reference system for the calculated geometry().
     *
     * \see crs()
     * \see geometry()
     */
    void setCrs( const QgsCoordinateReferenceSystem &crs ) { mCrs = crs; }

    /**
     * Returns the suggested viewport for the result, which reflects a recommended
     * map extent for displaying the result.
     *
     * This is an optional property, and will return a null rectangle if a recommended viewport
     * is not available (or not appropriate).
     *
     * The viewport CRS will match the CRS of geometry(), and can be retrieved via the crs() method.
     *
     * \see setViewport()
     */
    QgsRectangle viewport() const { return mViewport; }

    /**
     * Sets the suggested \a viewport for the result, which reflects a recommended
     * map extent for displaying the result.
     *
     * This is an optional property, and can be set to a null rectangle if a recommended viewport
     * is not available (or not appropriate).
     *
     * The viewport CRS must match the CRS of geometry()d.
     *
     * \see viewport()
     */
    void setViewport( const QgsRectangle &viewport ) { mViewport = viewport; }

    /**
     * Contains additional attributes generated during the geocode,
     * which may be added to features being geocoded.
     *
     * \see setAdditionalAttributes()
     */
    QVariantMap additionalAttributes() const { return mAdditionalAttributes; }

    /**
     * Setss additional attributes generated during the geocode,
     * which may be added to features being geocoded.
     *
     * \see additionalAttributes()
     */
    void setAdditionalAttributes( const QVariantMap &attributes ) { mAdditionalAttributes = attributes; }

    /**
     * Returns the optional group value for the result.
     *
     * The group can be used to categorize results into different groups, e.g. by appropriate administrative region.
     *
     * \see setGroup()
     */
    QString group() const;

    /**
     * Sets the optional \a group value for the result.
     *
     * The \a group can be used to categorize results into different groups, e.g. by appropriate administrative region.
     *
     * \see group()
     */
    void setGroup( const QString &group );

    /**
     * Returns the optional description for the result.
     *
     * \see setDescription()
     */
    QString description() const;

    /**
     * Sets an optional \a description for the result.
     *
     * \see description()
     */
    void setDescription( const QString &description );

  private:

    QgsGeocoderResult() = default;

    bool mIsValid = false;
    QString mErrorString;

    QString mIdentifier;
    QString mDescription;
    QString mGroup;
    QgsGeometry mGeometry;
    QgsCoordinateReferenceSystem mCrs;
    QgsRectangle mViewport;
    QVariantMap mAdditionalAttributes;

};

#endif // QGSGEOCODERRESULT_H
