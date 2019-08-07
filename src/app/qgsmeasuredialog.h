/***************************************************************************
                                qgsmeasure.h
                               ------------------
        begin                : March 2005
        copyright            : (C) 2005 by Radim Blazek
        email                : blazek@itc.it
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMEASUREDIALOG_H
#define QGSMEASUREDIALOG_H

#include "ui_qgsmeasurebase.h"

#include "qgspointxy.h"
#include "qgsdistancearea.h"
#include "qgshelp.h"
#include "qgis_app.h"

class QCloseEvent;
class QgsMeasureTool;
class QgsMapCanvas;

class APP_EXPORT QgsMeasureDialog : public QDialog, private Ui::QgsMeasureBase
{
    Q_OBJECT

  public:

    //! Constructor
    QgsMeasureDialog( QgsMeasureTool *tool, Qt::WindowFlags f = nullptr );

    //! Save position
    void saveWindowLocation();

    //! Restore last window position/size
    void restorePosition();

    //! Add new point
    void addPoint();

    //! Mose move
    void mouseMove( const QgsPointXY &point );

    //! Removes the last point
    void removeLastPoint();

  public slots:
    void reject() override;

    //! Reset and start new
    void restart();

    //! Close event
    void closeEvent( QCloseEvent *e ) override;

    //! When any external settings change
    void updateSettings();

  private slots:
    void unitsChanged( int index );

    //! Open configuration tab
    void openConfigTab();

    void crsChanged();

    void projChanged();

  private:

    //! formats distance to most appropriate units
    QString formatDistance( double distance, bool convertUnits = true ) const;

    //! formats area to most appropriate units
    QString formatArea( double area, bool convertUnits = true ) const;

    //! shows/hides table, shows correct units
    void updateUi();

    /**
     * Resets the units combo box to display either distance or area units
     * \param isArea set to true to populate with areal units, or false to show distance units
     */
    void repopulateComboBoxUnits( bool isArea );

    double convertLength( double length, QgsUnitTypes::DistanceUnit toUnit ) const;

    double convertArea( double area, QgsUnitTypes::AreaUnit toUnit ) const;

    double mTotal = 0.0;

    //! indicates whether we're measuring distances or areas
    bool mMeasureArea = false;

    //! Indicates whether the user chose "Map units" instead of directly selecting a unit
    bool mUseMapUnits = false;

    //! Indicates whether we need to convert units.
    bool mConvertToDisplayUnits = true;

    //! Number of decimal places we want.
    int mDecimalPlaces = 3;

    //! Current unit for input values
    QgsUnitTypes::DistanceUnit mCanvasUnits = QgsUnitTypes::DistanceUnknownUnit;

    //! Current unit for distance values
    QgsUnitTypes::DistanceUnit mDistanceUnits  = QgsUnitTypes::DistanceUnknownUnit;

    //! Current map unit for distance values
    QgsUnitTypes::DistanceUnit mMapDistanceUnits  = QgsUnitTypes::DistanceUnknownUnit;

    //! Current unit for area values
    QgsUnitTypes::AreaUnit mAreaUnits  = QgsUnitTypes::AreaUnknownUnit;

    //! Our measurement object
    QgsDistanceArea mDa;

    //! pointer to measure tool which owns this dialog
    QgsMeasureTool *mTool = nullptr;

    QgsMapCanvas *mCanvas = nullptr;

    QgsPointXY mLastMousePoint;

    void showHelp();

    friend class TestQgsMeasureTool;
};

#endif
