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

class QgsDistributeFeatureUserWidget;

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

    enum DistributeMode
    {
      FeatureCount,          //!< Distribute a fixed number of features with an undefined spacing
      FeatureSpacing,        //!< Distribute a undefined number of features with a fixed spacing
      FeatureCountAndSpacing //!< Distribute a fixed number of features with a fixed spacing
    };
    Q_ENUM( DistributeMode )

    //! The number of features to distribute
    int featureCount() const;
    void setFeatureCount( int featureCount );

    //! The spacing between features to distribute
    double featureSpacing() const;
    void setFeatureSpacing( double featureSpacing );

    //! The distribution mode
    DistributeMode mode() const { return mMode; };
    void setMode( DistributeMode mode );

    //! Settings in digitizing entry
    static const QgsSettingsEntryEnumFlag<DistributeMode> *settingsMode;
    static const QgsSettingsEntryInteger *settingsFeatureCount;
    static const QgsSettingsEntryDouble *settingsFeatureSpacing;

  private:
    //! The mode, feature count, and feature spacing
    int mFeatureCount;
    double mFeatureSpacing;
    DistributeMode mMode;

    //! Start point of the move in map coordinates
    QgsPointXY mStartPointMapCoords;
    QgsPointXY mEndPointMapCoords;

    //! The current feature ID, geometry, and layer
    QgsFeatureId mFeatureId;
    QgsGeometry mFeatureGeom;
    QgsVectorLayer *mFeatureLayer;

    //! The user widget
    std::unique_ptr<QgsDistributeFeatureUserWidget> mUserInputWidget;

    //! The rubberband that shows the feature being distributed
    std::unique_ptr<QgsRubberBand> mRubberBand;
    void deleteRubberbands();
    void updateRubberband();

    //! The coordinates of the first feature on the distribution
    QgsPointXY firstFeatureMapPoint() const;
};

//! User widget for the distribute map tool
class APP_EXPORT QgsDistributeFeatureUserWidget : public QWidget, private Ui::QgsDistributeFeatureUserInputBase
{
    Q_OBJECT

  public:
    explicit QgsDistributeFeatureUserWidget( QWidget *parent = nullptr );

    //! The number of features to distribute
    int featureCount() const;
    void setFeatureCount( int featureCount );

    //! The spacing between features to distribute
    double featureSpacing() const;
    void setFeatureSpacing( double featureSpacing );

    //! The distribution mode
    QgsMapToolDistributeFeature::DistributeMode mode() const;
    void setMode( QgsMapToolDistributeFeature::DistributeMode mode );

  signals:
    void modeChanged( QgsMapToolDistributeFeature::DistributeMode mode );
    void featureCountChanged( int featureCount );
    void featureSpacingChanged( double featureSpacing );

  private:
    void updateUi();
};

#endif
