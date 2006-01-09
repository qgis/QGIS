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
#ifndef QGSMEASURE_H
#define QGSMEASURE_H

#include "ui_qgsmeasurebase.h"
#include <QWidget>
#include "qgspoint.h"
#include <vector>

class QgsDistanceArea;
class QgsMapCanvas;
class QgsRubberBand;

class QCloseEvent;


class QgsMeasure:public QWidget, private Ui::QgsMeasureBase
{
  Q_OBJECT;
  public:

  //! Constructor
  QgsMeasure(bool measureArea, QgsMapCanvas *, QWidget *parent = 0, const char * name = 0, Qt::WFlags f = Qt::WStyle_Customize | Qt::WStyle_DialogBorder | Qt::WStyle_Title | Qt::WType_Dialog | Qt::WStyle_Tool );

  ~QgsMeasure();

  //! Save position
  void saveWindowLocation(void);

  //! Restore last window position/size and show the window
  void restorePosition(void);

  //! Add new point
  void addPoint(QgsPoint &point);
  
  //! Mose move
  void mouseMove(QgsPoint &point);
  
  //! Mouse press
  void mousePress(QgsPoint &point);
  
  //! returns whether measuring distance or area
  bool measureArea() { return mMeasureArea; }
  //! sets whether we're measuring area (and restarts)
  void setMeasureArea(bool measureArea);

public slots:
  //! Close
  void close ( void);

  //! Reset and start new
  void restart ( void);

  //! Close event
  void closeEvent(QCloseEvent *e);

  //! Redraw lines to match current state of canvas
  void mapCanvasChanged(); 

  //! Show the help for the dialog 
  void on_btnHelp_clicked();
  
private:
  
  //! formats distance to most appropriate units
  QString formatDistance(double distance);
  
  //! formats area to most appropriate units
  QString formatArea(double area);
  
  //! shows/hides table, shows correct units
  void updateUi();
  
  QgsMapCanvas *mMapCanvas;
  
  //! distance/area calculator
  QgsDistanceArea* mCalc;
  
  std::vector<QgsPoint> mPoints;

  double mTotal;

  //! Rubberband widget tracking the lines being drawn
  QgsRubberBand *mRubberBand;

  //! Help context id
  static const int context_id = 940759457;
  
  //! indicates whether we're measuring distances or areas
  bool mMeasureArea;
};

#endif
