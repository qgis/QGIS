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

#include "qgspoint.h"
#include "qgsdistancearea.h"
#include "qgscontexthelp.h"

class QCloseEvent;
class QgsMeasureTool;

class APP_EXPORT QgsMeasureDialog : public QDialog, private Ui::QgsMeasureBase
{
    Q_OBJECT

  public:

    //! Constructor
    QgsMeasureDialog( QgsMeasureTool* tool, Qt::WindowFlags f = nullptr );

    //! Save position
    void saveWindowLocation();

    //! Restore last window position/size
    void restorePosition();

    //! Add new point
    void addPoint( const QgsPoint &point );

    //! Mose move
    void mouseMove( const QgsPoint &point );

    //! Remove last point
    void removeLastPoint();

  public slots:
    virtual void reject() override;

    //! Reset and start new
    void restart();

    //! Close event
    void closeEvent( QCloseEvent *e ) override;

    //! Show the help for the dialog
    void on_buttonBox_helpRequested() { QgsContextHelp::run( metaObject()->className() ); }

    //! When any external settings change
    void updateSettings();

  private slots:
    void unitsChanged( int index );

    //! Open configuration tab
    void openConfigTab();

  private:

    //! formats distance to most appropriate units
    QString formatDistance( double distance, bool convertUnits = true ) const;

    //! formats area to most appropriate units
    QString formatArea( double area, bool convertUnits = true ) const;

    //! shows/hides table, shows correct units
    void updateUi();

    /** Resets the units combo box to display either distance or area units
     * @param isArea set to true to populate with areal units, or false to show distance units
     */
    void repopulateComboBoxUnits( bool isArea );

    double convertLength( double length, QGis::UnitType toUnit ) const;

    double convertArea( double area, QgsUnitTypes::AreaUnit toUnit ) const;

    double mTotal;

    //! indicates whether we're measuring distances or areas
    bool mMeasureArea;

    //! Number of decimal places we want.
    int mDecimalPlaces;

    //! Current unit for input values
    QGis::UnitType mCanvasUnits;

    //! Current unit for distance values
    QGis::UnitType mDistanceUnits;

    //! Current unit for area values
    QgsUnitTypes::AreaUnit mAreaUnits;

    //! Our measurement object
    QgsDistanceArea mDa;

    //! pointer to measure tool which owns this dialog
    QgsMeasureTool* mTool;

    QgsPoint mLastMousePoint;

    friend class TestQgsMeasureTool;
};

#endif
