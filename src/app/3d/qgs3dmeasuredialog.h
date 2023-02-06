/***************************************************************************
  qgs3dmeasuredialog.h
  --------------------------------------
  Date                 : Jun 2019
  Copyright            : (C) 2019 by Ismail Sunni
  Email                : imajimatika at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGS3DMEASUREDIALOG_H
#define QGS3DMEASUREDIALOG_H

#include <QCloseEvent>

#include "ui_qgsmeasurebase.h"
#include "qgs3dmaptoolmeasureline.h"
#include "qgsunittypes.h"


class Qgs3DMeasureDialog : public QDialog, private Ui::QgsMeasureBase
{
    Q_OBJECT

  public:
    // Constructor
    Qgs3DMeasureDialog( Qgs3DMapToolMeasureLine *tool, Qt::WindowFlags f = Qt::WindowFlags() );

    //! Save position
    void saveWindowLocation();

    //! Restore last window position/size
    void restorePosition();

    //! Add new point
    void addPoint();

    //! Get last distance in map distance unit
    double lastDistance();

    //! Get last Z value distance in map distance unit
    double lastVerticalDistance();

    //! Get last horizontal value distance in map distance unit
    double lastHorizontalDistance();

    //! Populating unit combo box
    void repopulateComboBoxUnits();

    //! Remove last point
    void removeLastPoint();

    // Clear the content of the table
    void resetTable();

  public slots:
    void reject() override;

    void restart();

    //! Close event
    void closeEvent( QCloseEvent *e ) override;

    //! When any external settings change
    void updateSettings();

  private slots:
    void unitsChanged( int index );

  private:
    Qgs3DMapToolMeasureLine *mTool;

    //! Total length in map distance unit
    double mTotal = 0.0;

    //! Total horizontal length in map distance unit
    double mHorizontalTotal = 0.0;

    //! Number of decimal places we want.
    int mDecimalPlaces = 3;

    //! Distance unit of the map
    QgsUnitTypes::DistanceUnit mMapDistanceUnit  = QgsUnitTypes::DistanceUnknownUnit;

    //! Distance unit of the displayed value
    QgsUnitTypes::DistanceUnit mDisplayedDistanceUnit  = QgsUnitTypes::DistanceUnknownUnit;

    //! Convert from mMapDistanceUnit to mDisplayedDistanceUnit
    double convertLength( double length, QgsUnitTypes::DistanceUnit toUnit ) const;

    //! formats distance to most appropriate units
    QString formatDistance( double distance ) const;

    //! Show the help page of the 3D measurement tool
    void showHelp();

    //! Open configuration tab
    void openConfigTab();

    //! Setup the header of the table
    void setupTableHeader();

    //! Add measurement (3d-distance, vertical distance, horizontal distance) to the table
    void addMeasurement( double distance, double verticalDistance, double horizontalDistance );

    //! Update total value
    void updateTotal();

    //! Update table based on current setting
    void updateTable();
};

#endif // QGS3DMEASUREDIALOG_H
