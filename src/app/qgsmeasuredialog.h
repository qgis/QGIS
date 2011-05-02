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

class QgsMeasureDialog : public QDialog, private Ui::QgsMeasureBase
{
    Q_OBJECT

  public:

    //! Constructor
    QgsMeasureDialog( QgsMeasureTool* tool, Qt::WFlags f = 0 );

    //! Save position
    void saveWindowLocation( void );

    //! Restore last window position/size
    void restorePosition( void );

    //! Add new point
    void addPoint( QgsPoint &point );

    //! Mose move
    void mouseMove( QgsPoint &point );

    //! Mouse press
    void mousePress( QgsPoint &point );

  public slots:
    //! Reject
    void on_buttonBox_rejected( void );

    //! Reset and start new
    void restart();

    //! Close event
    void closeEvent( QCloseEvent *e );

    //! Show the help for the dialog
    void on_buttonBox_helpRequested() { QgsContextHelp::run( metaObject()->className() ); }
  private slots:
    //! on change state projection enable
    void changeProjectionEnabledState();

  private:

    //! formats distance to most appropriate units
    QString formatDistance( double distance, int decimalPlaces );

    //! formats area to most appropriate units
    QString formatArea( double area, int decimalPlaces );

    //! shows/hides table, shows correct units
    void updateUi();

    //! Converts the measurement, depending on settings in options and current transformation
    void convertMeasurement( double &measure, QGis::UnitType &u, bool isArea );

    //! Configures distance area objects with ellipsoid / output crs
    void configureDistanceArea( QgsDistanceArea& da );

    double mTotal;

    //! indicates whether we're measuring distances or areas
    bool mMeasureArea;

    //! pointer to measure tool which owns this dialog
    QgsMeasureTool* mTool;
};

#endif
