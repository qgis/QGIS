/***************************************************************************
                         qgsalgorithmorientedfoliogrid.h
                         -------------------------------
    begin                : January 2026
    copyright            : (C) 2026 by Loïc Bartoletti
    email                : loic dot bartoletti at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSALGORITHMORIENTEDFOLIOGRID_H
#define QGSALGORITHMORIENTEDFOLIOGRID_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgslayoutitempage.h"
#include "qgslayoutsize.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Native oriented folio grid creation algorithm.
 *
 * Generates a grid of rectangular folios aligned with the Oriented Minimum
 * Bounding Box (OMBB) of input polygons. Useful for creating print layouts
 * covering an area of interest.
 */
class QgsOrientedFolioGridAlgorithm : public QgsProcessingAlgorithm
{
  public:
    QgsOrientedFolioGridAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QString shortDescription() const override;
    QgsOrientedFolioGridAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    QgsGeometry mDissolvedGeometry;
    QgsCoordinateReferenceSystem mCrs;
    QStringList mPageSizeNames;
    int mPageSizeIndex = 0;
    QgsLayoutItemPage::Orientation mOrientation = QgsLayoutItemPage::Landscape;
    double mCustomWidth = 297.0;
    double mCustomHeight = 210.0;
    int mScale = 200;
    double mHOverlay = 0.0;
    double mVOverlay = 0.0;
    bool mAvoidEmpty = true;

    /**
     * Returns the page size based on current settings.
     * Applies orientation (portrait/landscape) to the size.
     */
    QgsLayoutSize getPageSize() const;
};

///@endcond PRIVATE

#endif // QGSALGORITHMORIENTEDFOLIOGRID_H
