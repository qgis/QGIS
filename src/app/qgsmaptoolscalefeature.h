/***************************************************************************
    qgsmaptoolscalefeature.h  -  map tool for scaling features by mouse drag
    ---------------------
    Date                 : December 2020
    Copyright            : (C) 2020 by roya0045
    Contact              : ping me on github
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLSCALEFEATURE_H
#define QGSMAPTOOLSCALEFEATURE_H

#include <QWidget>

#include "qgsmaptooladvanceddigitizing.h"
#include "qgsvertexmarker.h"
#include "qgis_app.h"
#include "qgsgeometry.h"
#include "qgsfeatureid.h"

class QgsDoubleSpinBox;
class QHBoxLayout;
class QgsSpinBox;
class QgsSnapIndicator;

class APP_EXPORT QgsScaleMagnetWidget : public QWidget
{
    Q_OBJECT

  public:
    explicit QgsScaleMagnetWidget( const QString &label = QString(), QWidget *parent = nullptr );

    void setScale( double scale );
    double scale() const;

    QgsDoubleSpinBox *editor() const { return mScaleSpinBox; }

  signals:
    void scaleChanged( double scale );
    void scaleEditingFinished( double scale );
    void scaleEditingCanceled();


  public slots:

  protected:
    bool eventFilter( QObject *obj, QEvent *ev ) override;

  private slots:
    void scaleSpinBoxValueChanged( double scale );

  private:
    QHBoxLayout *mLayout = nullptr;
    QgsDoubleSpinBox *mScaleSpinBox = nullptr;
};


//! Map tool to scale features
class APP_EXPORT QgsMapToolScaleFeature : public QgsMapToolAdvancedDigitizing
{
    Q_OBJECT
  public:
    QgsMapToolScaleFeature( QgsMapCanvas *canvas );
    ~QgsMapToolScaleFeature() override;

    void cadCanvasMoveEvent( QgsMapMouseEvent *e ) override;

    void cadCanvasReleaseEvent( QgsMapMouseEvent *e ) override;

    //! called when map tool is being deactivated
    void deactivate() override;

    void activate() override;

    //! catch escape when active to cancel selection
    void keyReleaseEvent( QKeyEvent *e ) override;

  private slots:
    void updateRubberband( double scale );

    void applyScaling( double scale );
    void cancel();

  private:
    QgsGeometry scaleGeometry( QgsGeometry geom, QgsPointXY point, double scale );
    QgsPointXY scalePoint( QgsPointXY point, double scale );
    void deleteRubberband();
    void createScalingWidget();
    void deleteScalingWidget();

    //! Start point of the scaling in map coordinates
    QgsPointXY mFeatureCenterMapCoords;
    //! Rubberband that shows the feature being scaled
    QgsRubberBand *mRubberBand = nullptr;

    //! Id of scaled feature
    QgsFeatureIds mScaledFeatures;
    QVector<QgsGeometry> mOriginalGeometries;

    //! Snapping indicators
    std::unique_ptr<QgsSnapIndicator> mSnapIndicator;

    double mScaling = 0;
    double mBaseDistance = 1;
    QgsRectangle mExtent;

    std::unique_ptr<QgsVertexMarker> mAnchorPoint = nullptr;
    bool mAutoSetAnchorPoint = false;

    bool mScalingActive = false;

    //! Shows current scale value and allows numerical editing
    QgsScaleMagnetWidget *mScalingWidget = nullptr;
};

#endif
