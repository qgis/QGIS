/***************************************************************************
                         qgsintersection2circles.h
                         ----------------------
    begin                : October 2021
    copyright            : (C) 2021 by Antoine Facchini
    email                : antoine dot facchini at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSINTERSECTION2CIRCLES_H
#define QGSINTERSECTION2CIRCLES_H

#include <QWidget>
#include <QDialog>

#include "qgis_app.h"
#include "qgscircle.h"
#include "qgsmapcanvas.h"
#include "qgsmaptoolemitpoint.h"
#include "qgsrubberband.h"
#include "qgsvectorlayer.h"

#include "ui_intersection2circles.h"

class APP_EXPORT QgsIntersection2CirclesDialog : public QDialog, private Ui::QgsIntersection2Circles
{
    Q_OBJECT
  public:
    QgsIntersection2CirclesDialog( QgsMapCanvas *mapCanva, QgsVectorLayer *vlayer, QWidget *parent = nullptr );

    enum CircleNumber
    {
      CircleNum1,
      CircleNum2,
    };

  public slots:
    void show();

  private slots:
    void toggleSelectCenter( CircleNumber circleNum );
    void propertiesChanged();
    void updateCenterPoint( CircleNumber circleNum, const QgsPointXY &point, Qt::MouseButton button );
    void updateCircle();
    void selectIntersection( QgsRubberBand *intersection, QCheckBox *button );

    void onAccepted();
    void reject();

  private:
    void hideDrawings();
    void clearInformations();
    void initCircleParameters( QgsRubberBand *&rubberCircle, QgsRubberBand *&rubberInter,
                               QCheckBox *btnIntersection, QgsDoubleSpinBox *x1,
                               QgsDoubleSpinBox *y1, QgsDoubleSpinBox *radius,
                               QToolButton *selectCenter, CircleNumber circleNum );

    QgsCircle mCircle1;
    QgsCircle mCircle2;
    QgsPoint mIntersection1;
    QgsPoint mIntersection2;

    QgsRubberBand *mRubberCircle1;
    QgsRubberBand *mRubberCircle2;
    QgsRubberBand *mRubberInter1;
    QgsRubberBand *mRubberInter2;

    QgsVectorLayer *mLayer;
    QgsMapCanvas *mMapCanvas;
    QgsMapToolEmitPoint *mMapToolPoint = nullptr;

    QColor mDefaultColor;
    QColor mSelectedColor;
};

#endif // QGSINTERSECTION2CIRCLES_H
