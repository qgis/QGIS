/***************************************************************************
                          qgselevationprofiletoolmeasure.h
                          ---------------
    begin                : April 2022
    copyright            : (C) 2022 by Nyall Dawson
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
#ifndef QGSELEVATIONPROFILETOOLMEASURE_H
#define QGSELEVATIONPROFILETOOLMEASURE_H

#include "qgsplottool.h"
#include "qgsprofilepoint.h"
#include "qgscoordinatereferencesystem.h"

#include <QPointer>
#include <QDialog>

class QgsElevationProfileCanvas;
class QGraphicsLineItem;
class QLabel;

class QgsProfileMeasureResultsDialog : public QDialog
{
    Q_OBJECT

  public:

    QgsProfileMeasureResultsDialog();

    void setCrs( const QgsCoordinateReferenceSystem &crs );
    bool eventFilter( QObject *object, QEvent *event ) override;

  signals:

    void closed();

  public slots:

    void setMeasures( double total, double distance, double elevation );
    void clear();

  private:

    QLabel *mTotalLabel = nullptr;
    QLabel *mDistanceLabel = nullptr;
    QLabel *mElevationLabel = nullptr;

    QgsCoordinateReferenceSystem mCrs;

};

class QgsElevationProfileToolMeasure : public QgsPlotTool
{
    Q_OBJECT

  public:
    QgsElevationProfileToolMeasure( QgsElevationProfileCanvas *canvas );

    ~QgsElevationProfileToolMeasure() override;

    void plotMoveEvent( QgsPlotMouseEvent *event ) override;
    void plotPressEvent( QgsPlotMouseEvent *event ) override;
    void plotReleaseEvent( QgsPlotMouseEvent *event ) override;

  signals:

    void measureChanged( double totalDistance, double deltaCurveDistance, double deltaElevation );
    void cleared();

  private slots:

    void plotAreaChanged();

  private:
    void updateRubberBand();

    QgsElevationProfileCanvas *mElevationCanvas = nullptr;

    QgsProfileMeasureResultsDialog *mDialog = nullptr;

    QGraphicsLineItem *mRubberBand = nullptr;

    QgsProfilePoint mStartPoint;
    QgsProfilePoint mEndPoint;
    bool mMeasureInProgress = false;

};

#endif // QGSELEVATIONPROFILETOOLMEASURE_H
