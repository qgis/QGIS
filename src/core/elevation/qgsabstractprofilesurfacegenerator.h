/***************************************************************************
                         qgsabstractprofilesurfacegenerator.h
                         ---------------
    begin                : April 2022
    copyright            : (C) 2022 by Nyall Dawson
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
#ifndef QGSABSTRACTPROFILESURFACEGENERATOR_H
#define QGSABSTRACTPROFILESURFACEGENERATOR_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsabstractprofilegenerator.h"
#include "qgslinesymbol.h"
#include "qgsfillsymbol.h"

#include <memory>

class QgsProfileRequest;

#define SIP_NO_FILE

/**
 * \brief Abstract base class for storage of elevation profiles which represent a continuous surface (e.g. mesh layers and raster layers).
 *
 * \note Not available in Python bindings
 * \ingroup core
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsAbstractProfileSurfaceResults : public QgsAbstractProfileResults
{
  public:

    ~QgsAbstractProfileSurfaceResults() override;

    QString mId;
    QgsPointSequence mRawPoints;
    QMap< double, double > mDistanceToHeightMap;
    double minZ = std::numeric_limits< double >::max();
    double maxZ = std::numeric_limits< double >::lowest();

    Qgis::ProfileSurfaceSymbology symbology = Qgis::ProfileSurfaceSymbology::Line;
    std::unique_ptr< QgsLineSymbol > mLineSymbol;
    std::unique_ptr< QgsFillSymbol > mFillSymbol;
    double mElevationLimit = std::numeric_limits< double >::quiet_NaN();

    std::unique_ptr< QgsCurve > mProfileCurve;

    QMap< double, double > distanceToHeightMap() const override;
    QgsPointSequence sampledPoints() const override;
    QgsDoubleRange zRange() const override;
    QVector< QgsGeometry > asGeometries() const override;
    QVector<  QgsAbstractProfileResults::Feature > asFeatures( Qgis::ProfileExportType type, QgsFeedback *feedback = nullptr ) const override;
    QgsProfileSnapResult snapPoint( const QgsProfilePoint &point, const QgsProfileSnapContext &context ) override;
    QVector<QgsProfileIdentifyResults> identify( const QgsProfilePoint &point, const QgsProfileIdentifyContext &context ) override;
    void renderResults( QgsProfileRenderContext &context ) override;
    void copyPropertiesFromGenerator( const QgsAbstractProfileGenerator *generator ) override;
};

/**
 * \brief Abstract base class for objects which generate elevation profiles which represent a continuous surface (e.g. mesh layers and raster layers).
 *
 * \note Not available in Python bindings
 * \ingroup core
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsAbstractProfileSurfaceGenerator : public QgsAbstractProfileGenerator
{
  public:

    /**
     * Constructor for QgsAbstractProfileSurfaceGenerator.
     */
    QgsAbstractProfileSurfaceGenerator( const QgsProfileRequest &request );

    ~QgsAbstractProfileSurfaceGenerator() override;

    /**
     * Returns the symbology type for rendering the results.
     */
    Qgis::ProfileSurfaceSymbology symbology() const;

    /**
     * Returns the line symbol to be used for rendering the results.
     */
    QgsLineSymbol *lineSymbol() const;

    /**
     * Returns the fill symbol to be used for rendering the results.
     */
    QgsFillSymbol *fillSymbol() const;

    /**
     * Returns the elevation limit, which is used when symbology() is
     * Qgis::ProfileSurfaceSymbology::FillBelow or Qgis::ProfileSurfaceSymbology::FillAbove
     * to limit the fill to a specific elevation range.
     *
     * By default this is NaN, which indicates that there is no elevation limit.
     *
     * \see setElevationLimit()
     * \since QGIS 3.32
     */
    double elevationLimit() const;

    /**
     * Sets the elevation \a limit, which is used when symbology() is
     * Qgis::ProfileSurfaceSymbology::FillBelow or Qgis::ProfileSurfaceSymbology::FillAbove
     * to limit the fill to a specific elevation range.
     *
     * Set to NaN to indicate that there is no elevation limit.
     *
     * \see elevationLimit()
     * \since QGIS 3.32
     */
    void setElevationLimit( double limit );

  protected:

    Qgis::ProfileSurfaceSymbology mSymbology = Qgis::ProfileSurfaceSymbology::Line;
    std::unique_ptr< QgsLineSymbol > mLineSymbol;
    std::unique_ptr< QgsFillSymbol > mFillSymbol;
    double mElevationLimit = std::numeric_limits< double >::quiet_NaN();

    std::unique_ptr< QgsCurve > mProfileCurve;

    friend class QgsAbstractProfileSurfaceResults;

};


#endif // QGSABSTRACTPROFILESURFACEGENERATOR_H
