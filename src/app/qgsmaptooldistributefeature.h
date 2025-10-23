/***************************************************************************
 qgsmaptooldistributefeature.h  -  map tool for copying and distributing features by mouse drag
 ---------------------
 begin                : November 2025
 copyright            : (C) 2025 by Jacky Volpes
 email                : jacky dot volpes at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLDISTRIBUTEFEATURE_H
#define QGSMAPTOOLDISTRIBUTEFEATURE_H

#include "qgis_app.h"
#include "qgsfeatureid.h"
#include "qgsgeometry.h"
#include "qgsmaptooladvanceddigitizing.h"
#include "qgsrubberband.h"
#include "qgspointxy.h"
#include "qgssnappingconfig.h"
#include "qgspointlocator.h"

#include "qgssettingsentryimpl.h"
#include "qgssettingsentryenumflag.h"

#include "ui_qgsdistributefeatureuserinputwidget.h"

class APP_EXPORT QgsDistributeFeatureUserWidget : public QWidget, private Ui::QgsDistributeFeatureUserInputBase
{
    Q_OBJECT

  public:
    explicit QgsDistributeFeatureUserWidget( QWidget *parent = nullptr );

    ~QgsDistributeFeatureUserWidget() override;

    enum DistributeMode
    {
      FeatureCount,           //!< Distribute a fixed number of features with an undefined spacing
      FeatureSpacing,         //!< Distribute a undefined number of features with a fixed spacing
      FeatureNumberAndSpacing //!< Distribute a fixed number of features with a fixed spacing
    };
    Q_ENUM( DistributeMode )

    void setFeatureSpacing( double spacing );
    double featureSpacing() const;
    void setFeatureCount( int featureCount );
    int featureCount() const;
    void setMode( QgsDistributeFeatureUserWidget::DistributeMode mode );
    QgsDistributeFeatureUserWidget::DistributeMode mode() const;
    void updateUi();
};

//! Map tool for copying and distributing features by mouse drag
class APP_EXPORT QgsMapToolDistributeFeature : public QgsMapToolAdvancedDigitizing
{
    Q_OBJECT
  public:
    QgsMapToolDistributeFeature( QgsMapCanvas *canvas );

    void cadCanvasMoveEvent( QgsMapMouseEvent *e ) override;

    void cadCanvasReleaseEvent( QgsMapMouseEvent *e ) override;

    void keyPressEvent( QKeyEvent *e ) override;

    void deactivate() override;

    void activate() override;

    //! Settings entry digitizing: mode
    static const QgsSettingsEntryEnumFlag<QgsDistributeFeatureUserWidget::DistributeMode> *settingsMode;

    //! Settings entry digitizing: feature count
    static const QgsSettingsEntryInteger *settingsFeatureCount;

    //! Settings entry digitizing: feature spacing
    static const QgsSettingsEntryDouble *settingsFeatureSpacing;

  private:
    //! Start point of the move in map coordinates
    QgsPointXY mStartPointMapCoords;
    QgsPointXY mEndPointMapCoords;

    //! The current feature ID
    QgsFeatureId mFeatureId;

    //! The feature geometry
    QgsGeometry mFeatureGeom;

    //! The feature layer
    QgsVectorLayer *mFeatureLayer;

    //! The distribute feature widget
    std::unique_ptr<QgsDistributeFeatureUserWidget> mUserInputWidget;

    //! Rubberband that shows the feature being moved
    std::unique_ptr<QgsRubberBand> mFeaturesRubberBand;

    //! Snapping config that will be restored on deactivation
    QgsSnappingConfig mOriginalSnappingConfig;

    void deleteRubberbands();

    void updateRubberband();

    //! The number of features to distribute
    int featureCount() const;

    //! The spacing between features to distribute
    double featureSpacing() const;

    //! The coordinates of the first feature on the distribution
    QgsPointXY firstFeatureMapPoint() const;
};

#endif
