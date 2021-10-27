/***************************************************************************
                         qgsmaptoolcogo.h
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

#ifndef QGSMAPTOOLCOGO_H
#define QGSMAPTOOLCOGO_H

#include <QWidget>
#include <QDialog>
#include "qgis_app.h"
#include "geometry/qgscircle.h"
#include "qgsgeometryrubberband.h"
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

  signals:

  private slots:
    void onAccepted();
    void propertiesChanged( CircleNumber circleNum );
    void updateCircle( CircleNumber circleNum );

  private:
    QgsCircle mCircle1;
    QgsCircle mCircle2;
    QgsGeometryRubberBand *mRubberCircle1;
    QgsGeometryRubberBand *mRubberCircle2;

    QgsVectorLayer *mLayer;
};

#endif // QGSMAPTOOLCOGO_H
