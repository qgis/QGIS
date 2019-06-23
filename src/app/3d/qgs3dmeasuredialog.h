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
#include "qgs3dmapcanvas.h"
#include "qgsunittypes.h"
#include "qgsdistancearea.h"


class Qgs3DMeasureDialog : public QDialog, private Ui::QgsMeasureBase
{
    Q_OBJECT

  public:
    // Constructor
    Qgs3DMeasureDialog( Qgs3DMapToolMeasureLine *tool, Qt::WindowFlags f = nullptr );

    //! Save position
    void saveWindowLocation();

    //! Restore last window position/size
    void restorePosition();

    //! Add new point
    void addPoint();

    //! Get last distance
    double lastDistance();

    //! update UI
    void updateUi();

    //! Populating unit combo box
    void repopulateComboBoxUnits();

    //! Remove last point
    void removeLastPoint();

  public slots:
    void reject() override;

    void restart();

    //! Close event
    void closeEvent( QCloseEvent *e ) override;

    //! When any external settings change
    void updateSettings();

  private slots:
    void unitsChanged( int index );

    void crsChanged();

  private:
    Qgs3DMapToolMeasureLine *mTool;

    //! Total length
    double mTotal = 0.0;

    //! Number of decimal places we want.
    int mDecimalPlaces = 3;

    //! Indicates whether the user chose "Map units" instead of directly selecting a unit
    bool mUseMapUnits = true;

    //! Indicates whether we need to convert units.
    bool mConvertToDisplayUnits = true;

    //! Current unit for input values
    QgsUnitTypes::DistanceUnit mCanvasUnits = QgsUnitTypes::DistanceUnknownUnit;

    //! Current unit for distance values
    QgsUnitTypes::DistanceUnit mDistanceUnits  = QgsUnitTypes::DistanceUnknownUnit;

    //! Current map unit for distance values
    QgsUnitTypes::DistanceUnit mMapDistanceUnits  = QgsUnitTypes::DistanceUnknownUnit;

    //! Our measurement object
    QgsDistanceArea mDa;

    double convertLength( double length, QgsUnitTypes::DistanceUnit toUnit ) const;

    //! formats distance to most appropriate units
    QString formatDistance( double distance, bool convertUnits = true ) const;

    void showHelp();

    //! Open configuration tab
    void openConfigTab();
};

#endif // QGS3DMEASUREDIALOG_H
