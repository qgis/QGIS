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

#include "qgsmaptooledit.h"
#include "qgsvectorlayer.h"


class QgsDoubleSpinBox;
class QHBoxLayout;
class QgsSpinBox;
class QgsVertexMarker;

class APP_EXPORT QgsAngleMagnetWidget : public QWidget
{
    Q_OBJECT

  public:

    explicit QgsAngleMagnetWidget( const QString& label = QString(), QWidget *parent = nullptr );

    ~QgsAngleMagnetWidget();

    void setAngle( double angle );

    double angle();

    void setMagnet( int magnet );

  signals:
    void angleChanged( double angle );
    void angleEditingFinished( double angle );


  public slots:

  protected:
    bool eventFilter( QObject *obj, QEvent *ev ) override;

  private slots:
    void angleSpinBoxValueChanged( double angle );

  private:
    QHBoxLayout* mLayout;
    QgsDoubleSpinBox* mAngleSpinBox;
    QgsSpinBox* mMagnetSpinBox;
};


/** Map tool to rotate features */
class APP_EXPORT QgsMapToolRotateFeature: public QgsMapToolEdit
{
    Q_OBJECT
  public:
    QgsMapToolRotateFeature( QgsMapCanvas* canvas );
    virtual ~QgsMapToolRotateFeature();

    virtual void canvasMoveEvent( QgsMapMouseEvent* e ) override;

    virtual void canvasReleaseEvent( QgsMapMouseEvent* e ) override;

    //! called when map tool is being deactivated
    void deactivate() override;

    void activate() override;

  private slots:
    void updateRubberband( double rotation );

    void applyRotation( double rotation );

  private:

    QgsGeometry rotateGeometry( QgsGeometry geom, QgsPoint point, double angle );
    QgsPoint rotatePoint( QgsPoint point, double angle );
    void deleteRubberband();
    void createRotationWidget();
    void deleteRotationWidget();

    /** Start point of the move in map coordinates*/
    QgsPoint mStartPointMapCoords;
    QPointF mInitialPos;

    /** Rubberband that shows the feature being moved*/
    QgsRubberBand* mRubberBand;

    /** Id of moved feature*/
    QgsFeatureIds mRotatedFeatures;
    double mRotation;
    double mRotationOffset;

    QPoint mStPoint;
    QgsVertexMarker* mAnchorPoint;

    bool mRotationActive;

    /** Shows current angle value and allows numerical editing*/
    QgsAngleMagnetWidget* mRotationWidget;
};

#endif
