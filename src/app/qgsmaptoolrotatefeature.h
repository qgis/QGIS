/***************************************************************************
    qgsmaptoolrotatefeature.h  -  map tool for rotating features by mouse drag
    ---------------------
    begin                : January 2013
    copyright            : (C) 2013 by Vinayan Parameswaran
    email                : vinayan123 at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLROTATEFEATURE_H
#define QGSMAPTOOLROTATEFEATURE_H

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

class APP_EXPORT QgsAngleMagnetWidget : public QWidget
{
    Q_OBJECT

  public:
    explicit QgsAngleMagnetWidget( const QString &label = QString(), QWidget *parent = nullptr );

    void setAngle( double angle );
    double angle() const;
    void setMagnet( int magnet );
    int magnet() const;

    QgsDoubleSpinBox *editor() const { return mAngleSpinBox; }

  signals:
    void angleChanged( double angle );
    void angleEditingFinished( double angle );
    void angleEditingCanceled();


  public slots:

  protected:
    bool eventFilter( QObject *obj, QEvent *ev ) override;

  private slots:
    void angleSpinBoxValueChanged( double angle );

  private:
    QHBoxLayout *mLayout = nullptr;
    QgsDoubleSpinBox *mAngleSpinBox = nullptr;
    QgsSpinBox *mMagnetSpinBox = nullptr;
};


//! Map tool to rotate features
class APP_EXPORT QgsMapToolRotateFeature : public QgsMapToolAdvancedDigitizing
{
    Q_OBJECT
  public:
    QgsMapToolRotateFeature( QgsMapCanvas *canvas );
    ~QgsMapToolRotateFeature() override;

    void cadCanvasMoveEvent( QgsMapMouseEvent *e ) override;

    void cadCanvasReleaseEvent( QgsMapMouseEvent *e ) override;

    //! called when map tool is being deactivated
    void deactivate() override;

    void activate() override;

    //! catch escape when active to cancel selection
    void keyReleaseEvent( QKeyEvent *e ) override;

  private slots:
    void updateRubberband( double rotation );

    void applyRotation( double rotation );
    void cancel();

  private:
    QgsGeometry rotateGeometry( QgsGeometry geom, QgsPointXY point, double angle );
    QgsPointXY rotatePoint( QgsPointXY point, double angle );
    void deleteRubberband();
    void createRotationWidget();
    void deleteRotationWidget();

    //! Start point of the rotation in map coordinates
    QgsPointXY mStartPointMapCoords;
    QgsPointXY mInitialPos;

    //! Rubberband that shows the feature being rotated
    QgsRubberBand *mRubberBand = nullptr;

    //! Snapping indicators
    std::unique_ptr<QgsSnapIndicator> mSnapIndicator;

    //! Id of rotated feature
    QgsFeatureIds mRotatedFeatures;
    double mRotation = 0;
    double mRotationOffset = 0;

    QPoint mStPoint;
    std::unique_ptr<QgsVertexMarker> mAnchorPoint = nullptr;
    bool mAutoSetAnchorPoint = false;

    bool mRotationActive = false;

    //! Shows current angle value and allows numerical editing
    QgsAngleMagnetWidget *mRotationWidget = nullptr;

    // MultiGeometry of the features being rotated
    QgsGeometry mGeom;
};

#endif
