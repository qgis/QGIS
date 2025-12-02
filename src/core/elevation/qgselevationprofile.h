/***************************************************************************
    qgselevationprofile.h
    ------------------
    Date                 : July 2025
    Copyright            : (C) 2025 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSELEVATIONPROFILE_H
#define QGSELEVATIONPROFILE_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsmaplayerref.h"

#include <QObject>
#include <QPointer>

class QgsProject;
class QgsReadWriteContext;
class QDomDocument;
class QDomElement;
class QgsMapLayer;
class QgsCurve;
class QgsLineSymbol;
class QgsLayerTree;

/**
 * \ingroup core
 * \class QgsElevationProfile
 *
 * \brief Represents an elevation profile attached to a project.
 *
 * \since QGIS 4.0
 */
class CORE_EXPORT QgsElevationProfile : public QObject
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsElevationProfile.
     */
    explicit QgsElevationProfile( QgsProject *project );
    ~QgsElevationProfile() override;

    /**
     * Returns the profile's unique name.
     *
     * \see setName()
     * \see nameChanged()
     */
    QString name() const { return mName; }

    /**
     * Returns the profiles's state encapsulated in a DOM element.
     * \see readXml()
     */
    QDomElement writeXml( QDomDocument &document, const QgsReadWriteContext &context ) const;

    /**
     * Sets the profiles's state from a DOM element.
     *
     * \a element is the DOM node corresponding to the profile.
     *
     * \see resolveReferences()
     * \see writeXml()
     */
    bool readXml( const QDomElement &element, const QDomDocument &document, const QgsReadWriteContext &context );

    /**
     * After reading settings from XML, resolves references to any layers in a \a project that have been read as layer IDs.
     *
     * \see readXml()
     */
    void resolveReferences( const QgsProject *project );

    /**
     * Returns the icon to use for the elevation profile.
     */
    QIcon icon() const;

    /**
     * Returns the layer tree used by the profile.
     */
    QgsLayerTree *layerTree();

    /**
     * Returns the crs associated with the profile's map coordinates.
     *
     * \see setCrs()
     */
    QgsCoordinateReferenceSystem crs() const;

    /**
     * Sets the profile \a curve.
     *
     * The CRS associated with \a curve is set via setCrs().
     *
     * Ownership is transferred to the profile.
     *
     * \see profileCurve()
     */
    void setProfileCurve( QgsCurve *curve SIP_TRANSFER );

    /**
     * Returns the profile curve.
     *
     * The CRS associated with the curve is retrieved via crs().
     *
     * \see setProfileCurve()
     */
    QgsCurve *profileCurve() const;

    /**
     * Returns the tolerance of the profile (in crs() units).
     *
     * This value determines how far from the profileCurve() is appropriate for inclusion of results. For instance,
     * when a profile is generated for a point vector layer this tolerance distance will dictate how far from the
     * actual profile curve a point can reside within to be included in the results.
     *
     * \see setTolerance()
     */
    double tolerance() const;

    /**
     * Returns TRUE if the distance and elevation scales are locked to each other.
     *
     * \see setLockAxisScales()
     */
    bool lockAxisScales() const;

    /**
     * Returns the distance unit used by the profile.
     *
     * \see setDistanceUnit()
     */
    Qgis::DistanceUnit distanceUnit() const;

    /**
     * Returns the symbol used to draw the subsections.
     *
     * \see setSubsectionsSymbol()
     */
    QgsLineSymbol *subsectionsSymbol();

    /**
     * Sets the \a symbol used to draw the subsections.
     *
     * If \a symbol is NULLPTR, the subsections are not drawn.
     * Ownership of \a symbol is transferred.
     *
     * \see subsectionsSymbol()
     */
    void setSubsectionsSymbol( QgsLineSymbol *symbol SIP_TRANSFER );

  public slots:

    /**
     * Sets the profile's unique \a name.
     *
     * \see name()
     * \see nameChanged()
     */
    void setName( const QString &name );

    /**
     * Sets the \a crs associated with the profile's map coordinates.
     *
     * \see crs()
     */
    void setCrs( const QgsCoordinateReferenceSystem &crs );

    /**
     * Sets the profile tolerance (in crs() units).
     *
     * This value determines how far from the profileCurve() is appropriate for inclusion of results. For instance,
     * when a profile is generated for a point vector layer this tolerance distance will dictate how far from the
     * actual profile curve a point can reside within to be included in the results.
     *
     * \see tolerance()
     */
    void setTolerance( double tolerance );

    /**
     * Sets whether the distance and elevation scales are locked to each other.
     *
     * \see lockAxisScales()
     */
    void setLockAxisScales( bool lock );

    /**
     * Sets the distance \a unit used by the profile.
     *
     * \see distanceUnit()
     */
    void setDistanceUnit( Qgis::DistanceUnit unit );

  signals:

    /**
     * Emitted when the profile is renamed.
     *
     * \see name()
     * \see setName()
     */
    void nameChanged( const QString &newName );

  private slots:

    void dirtyProject();

  private:

    void setupLayerTreeConnections();

    QPointer< QgsProject > mProject;
    QString mName;
    QgsCoordinateReferenceSystem mCrs;
    bool mLockAxisScales = false;
    Qgis::DistanceUnit mDistanceUnit = Qgis::DistanceUnit::Unknown;
    std::unique_ptr<QgsLayerTree> mLayerTree;
    std::unique_ptr<QgsCurve> mProfileCurve;
    double mTolerance = 0;
    std::unique_ptr<QgsLineSymbol> mSubsectionsSymbol;

};

#endif // QGSELEVATIONPROFILE_H
