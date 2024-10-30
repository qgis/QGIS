/***************************************************************************
                             qgslayoutitemelevationprofile.h
                             -------------------------------
    begin                : January 2023
    copyright            : (C) 2023 by Nyall Dawson
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
#ifndef QGSLAYOUTITEMELEVATIONPROFILE_H
#define QGSLAYOUTITEMELEVATIONPROFILE_H

#include "qgis_core.h"
#include "qgslayoutitem.h"
#include "qgsmaplayerref.h"

class QgsLayoutItemElevationProfilePlot;
class Qgs2DPlot;
class QgsProfileRequest;
class QgsProfilePlotRenderer;

/**
 * \ingroup core
 * \brief A layout item subclass for elevation profile plots.
 * \since QGIS 3.30
 */
class CORE_EXPORT QgsLayoutItemElevationProfile: public QgsLayoutItem
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsLayoutItemElevationProfile, with the specified parent \a layout.
     */
    QgsLayoutItemElevationProfile( QgsLayout *layout );

    ~QgsLayoutItemElevationProfile() override;

    /**
     * Returns a new elevation profile item for the specified \a layout.
     *
     * The caller takes responsibility for deleting the returned object.
     */
    static QgsLayoutItemElevationProfile *create( QgsLayout *layout ) SIP_FACTORY;

    int type() const override;
    QIcon icon() const override;
    void refreshDataDefinedProperty( QgsLayoutObject::DataDefinedProperty property = QgsLayoutObject::DataDefinedProperty::AllProperties ) override;
    QgsLayoutItem::Flags itemFlags() const override;
    bool requiresRasterization() const override;
    bool containsAdvancedEffects() const override;

    /**
     * Returns a reference to the elevation plot object, which can be used to
     * set plot appearance and properties.
     */
    Qgs2DPlot *plot();

    /**
     * Returns a reference to the elevation plot object, which can be used to
     * set plot appearance and properties.
     */
    const Qgs2DPlot *plot() const SIP_SKIP;

    /**
     * Returns the list of map layers participating in the elevation profile.
     *
     * \see setLayers()
     */
    QList< QgsMapLayer * > layers() const;

    /**
     * Sets the list of map \a layers participating in the elevation profile.
     *
     * \see layers()
     */
    void setLayers( const QList< QgsMapLayer * > &layers );

    /**
     * Sets the cross section profile \a curve, which represents the line along which the profile should be generated.
     *
     * Ownership of \a curve is transferred to the item.
     *
     * The coordinate reference system of the \a curve is set via setCrs().
     *
     * \see profileCurve()
     */
    void setProfileCurve( QgsCurve *curve SIP_TRANSFER );

    /**
     * Returns the cross section profile curve, which represents the line along which the profile should be generated.
     *
     * The coordinate reference system of the curve is retrieved via crs().
     *
     * \see setProfileCurve()
     */
    QgsCurve *profileCurve() const;

    /**
     * Sets the desired Coordinate Reference System (\a crs) for the profile.
     *
     * This also represents the CRS associated with the profileCurve().
     *
     * \see crs()
     */
    void setCrs( const QgsCoordinateReferenceSystem &crs );

    /**
     * Returns the desired Coordinate Reference System for the profile.
     *
     * This also represents the CRS associated with the profileCurve().
     *
     * \see setCrs()
     */
    QgsCoordinateReferenceSystem crs() const;

    /**
     * Sets the tolerance of the request (in crs() units).
     *
     * This value determines how far from the profileCurve() is appropriate for inclusion of results. For instance,
     * when a profile is generated for a point vector layer this tolerance distance will dictate how far from the
     * actual profile curve a point can reside within to be included in the results. Other sources may completely
     * ignore this tolerance if it is not appropriate for the particular source.
     *
     * \see tolerance()
     */
    void setTolerance( double tolerance );

    /**
     * Returns the tolerance of the request (in crs() units).
     *
     * This value determines how far from the profileCurve() is appropriate for inclusion of results. For instance,
     * when a profile is generated for a point vector layer this tolerance distance will dictate how far from the
     * actual profile curve a point can reside within to be included in the results. Other sources may completely
     * ignore this tolerance if it is not appropriate for the particular source.
     *
     * \see setTolerance()
     */
    double tolerance() const;

    /**
     * Returns whether the profile curve is set to follow the current atlas feature.

     * \see setAtlasDriven()
     */
    bool atlasDriven() const { return mAtlasDriven; }

    /**
     * Sets whether the profile curve will follow the current atlas feature.
     *
     * This requires an active layout atlas or report, using a line geometry type coverage layer.
     *
     * \see atlasDriven()
     */
    void setAtlasDriven( bool enabled );

    /**
     * Returns the profile request used to generate the elevation profile.
     */
    QgsProfileRequest profileRequest() const;

    void paint( QPainter *painter, const QStyleOptionGraphicsItem *itemStyle, QWidget *pWidget ) override;

    /**
     * Returns the units for the distance axis.
     *
     * \see setDistanceUnit()
     * \since QGIS 3.32
     */
    Qgis::DistanceUnit distanceUnit() const;

    /**
     * Sets the \a unit for the distance axis.
     *
     * \see distanceUnit()
     * \since QGIS 3.32
     */
    void setDistanceUnit( Qgis::DistanceUnit unit );

  public slots:

    void refresh() override;
    void invalidateCache() override;

  signals:

    /**
     * Emitted whenever the item's preview has been refreshed.
     *
     * \since QGIS 3.34
     */
    void previewRefreshed();

  protected:
    void draw( QgsLayoutItemRenderContext &context ) override;
    bool writePropertiesToElement( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const override;
    bool readPropertiesFromElement( const QDomElement &element, const QDomDocument &document, const QgsReadWriteContext &context ) override;

  private slots:

    void recreateCachedImageInBackground();
    void profileGenerationFinished();
  private:

    std::unique_ptr< QgsLayoutItemElevationProfilePlot > mPlot;

    QList< QgsMapLayerRef > mLayers;

    QgsCoordinateReferenceSystem mCrs;
    Qgis::DistanceUnit mDistanceUnit = Qgis::DistanceUnit::Unknown;
    std::unique_ptr< QgsCurve> mCurve;
    bool mAtlasDriven = false;

    double mTolerance = 0;

    // render job handling

    // see note in QgsLayoutItemMap about these!
    std::unique_ptr< QImage > mCacheFinalImage;
    std::unique_ptr< QImage > mCacheRenderingImage;
    bool mUpdatesEnabled = true;
    bool mCacheInvalidated = true;
    bool mDrawing = false;
    bool mDrawingPreview = false;
    QTimer *mBackgroundUpdateTimer = nullptr;
    double mPreviewScaleFactor = 0;
    std::unique_ptr< QPainter > mPainter;
    std::unique_ptr< QgsProfilePlotRenderer > mRenderJob;


};

#endif //QGSLAYOUTITEMELEVATIONPROFILE_H
