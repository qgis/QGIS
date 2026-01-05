/***************************************************************************
 qgsmaptoolfeaturearray.cpp  -  map tool for copying a feature in an array of features by mouse drag
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

#ifndef QGSMAPTOOLFEATUREARRAY_H
#define QGSMAPTOOLFEATUREARRAY_H

#include "ui_qgsfeaturearrayuserinputwidget.h"

#include "qgis_app.h"
#include "qgsgeometry.h"
#include "qgsmaptooladvanceddigitizing.h"
#include "qgspointlocator.h"
#include "qgspointxy.h"
#include "qgsrubberband.h"
#include "qgssettingsentryenumflag.h"
#include "qgssettingsentryimpl.h"
#include "qgssnappingconfig.h"
#include "qobjectuniqueptr.h"

class QgsFeatureArrayUserWidget;

//! Map tool for copying and distributing features by mouse drag
class APP_EXPORT QgsMapToolFeatureArray : public QgsMapToolAdvancedDigitizing
{
    Q_OBJECT
  public:
    QgsMapToolFeatureArray( QgsMapCanvas *canvas );

    void cadCanvasMoveEvent( QgsMapMouseEvent *e ) override;
    void cadCanvasReleaseEvent( QgsMapMouseEvent *e ) override;
    void keyPressEvent( QKeyEvent *e ) override;

    void deactivate() override;
    void activate() override;

    enum class ArrayMode
    {
      FeatureCount,          //!< Fixed number of features with an undefined spacing
      FeatureSpacing,        //!< Undefined number of features with a fixed spacing
      FeatureCountAndSpacing //!< Fixed number of features with a fixed spacing
    };
    Q_ENUM( ArrayMode )

    //! The number of features in the array
    int featureCount() const;
    void setFeatureCount( int featureCount );

    //! The spacing between features in the array
    double featureSpacing() const;
    void setFeatureSpacing( double featureSpacing );

    //! The distribution mode
    ArrayMode mode() const { return mMode; };
    void setMode( ArrayMode mode );

    //! Settings in digitizing entry
    static const QgsSettingsEntryEnumFlag<ArrayMode> *settingsMode;
    static const QgsSettingsEntryInteger *settingsFeatureCount;
    static const QgsSettingsEntryDouble *settingsFeatureSpacing;

  private:
    //! The mode, feature count, and feature spacing
    int mFeatureCount = settingsFeatureCount->value();
    double mFeatureSpacing = settingsFeatureSpacing->value();
    ArrayMode mMode = settingsMode->value();

    //! Start point of the move in map coordinates
    QgsPointXY mStartPointMapCoords;
    QgsPointXY mEndPointMapCoords;

    //! The current feature ID, geometry, and layer
    QgsFeatureList mFeatureList;
    QgsVectorLayer *mFeatureLayer = nullptr;

    //! The user widget
    QObjectUniquePtr<QgsFeatureArrayUserWidget> mUserInputWidget;

    //! The rubberband that shows the array of features
    std::unique_ptr<QgsRubberBand> mRubberBand;
    void deleteRubberbands();
    void updateRubberband();

    //! The coordinates of the first feature on the distribution
    QgsPointXY firstFeatureMapPoint() const;
};

//! User widget for the feature array map tool
class APP_EXPORT QgsFeatureArrayUserWidget : public QWidget, private Ui::QgsFeatureArrayUserInputBase
{
    Q_OBJECT

  public:
    explicit QgsFeatureArrayUserWidget( QWidget *parent = nullptr );

    //! The number of features in the array
    int featureCount() const;
    void setFeatureCount( int featureCount );

    //! The spacing between features in the array
    double featureSpacing() const;
    void setFeatureSpacing( double featureSpacing );

    //! The distribution mode
    QgsMapToolFeatureArray::ArrayMode mode() const;
    void setMode( QgsMapToolFeatureArray::ArrayMode mode );

  signals:
    void modeChanged( QgsMapToolFeatureArray::ArrayMode mode );
    void featureCountChanged( int featureCount );
    void featureSpacingChanged( double featureSpacing );

  private:
    void updateUi();
};

#endif
