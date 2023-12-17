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
class QgsSettingsEntryBool;
class QgsSettingsEntryString;

class APP_EXPORT QgsMeasureDialog : public QDialog, private Ui::QgsMeasureBase
{
    Q_OBJECT

  public:

    static const QgsSettingsEntryBool *settingClipboardHeader;
    static const QgsSettingsEntryString *settingClipboardSeparator;
    static const QgsSettingsEntryBool *settingClipboardAlwaysUseDecimalPoint;

    //! Constructor
    QgsMeasureDialog( QgsMeasureTool *tool, Qt::WindowFlags f = Qt::WindowFlags() );

    //! Save position
    void saveWindowLocation();

    //! Restore last window position/size
    void restorePosition();

    //! Add new point
    void addPoint();

    //! Mouse move
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

    //! Copy measurements to the clipboard
    void copyMeasurements();

    void crsChanged();

    void projChanged();

  private:

    //! \since QGIS 3.32 columns
    enum Columns
    {
      X = 0,
      Y,
      Distance,
    };

    //! formats distance to most appropriate units
    QString formatDistance( double distance, bool convertUnits = true ) const;

    //! formats area to most appropriate units
    QString formatArea( double area, bool convertUnits = true ) const;

    //! update units-related members passed on selected area/distance unit type
    void updateUnitsMembers();

    //! shows/hides table, shows correct units
    void updateUi();

    /**
     * Resets the units combo box to display either distance or area units
     * \param isArea set to TRUE to populate with areal units, or FALSE to show distance units
     */
    void repopulateComboBoxUnits( bool isArea );

    double convertLength( double length, Qgis::DistanceUnit toUnit ) const;

    double convertArea( double area, Qgis::AreaUnit toUnit ) const;

    double mTotal = 0.0;

    //! indicates whether we're measuring distances or areas
    bool mMeasureArea = false;

    //! Indicates whether the user chose "Map units" instead of directly selecting a unit
    bool mUseMapUnits = false;

    //! Indicates whether we need to convert units.
    bool mConvertToDisplayUnits = true;

    //! Number of decimal places we want.
    int mDecimalPlaces = 3;

    //! Number of decimal places we want for the coordinates.
    int mDecimalPlacesCoordinates = 3;

    //! Current unit for input values
    Qgis::DistanceUnit mCanvasUnits = Qgis::DistanceUnit::Unknown;

    //! Current unit for distance values
    Qgis::DistanceUnit mDistanceUnits  = Qgis::DistanceUnit::Unknown;

    //! Current map unit for distance values
    Qgis::DistanceUnit mMapDistanceUnits  = Qgis::DistanceUnit::Unknown;

    //! Current unit for area values
    Qgis::AreaUnit mAreaUnits  = Qgis::AreaUnit::Unknown;

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
