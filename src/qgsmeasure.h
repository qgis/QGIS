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

class QCloseEvent;
class QPainter;
class QPixmap;

class QgsMapCanvas;
class QgsDistanceArea;

#include <vector>
#include "qgspoint.h"

#ifdef WIN32
#include "qgsmeasurebase.h"
#else
#include "qgsmeasurebase.uic.h"
#endif


class QgsMeasure:public QgsMeasureBase
{
  Q_OBJECT;
  public:

  //! Constructor
  QgsMeasure(bool measureArea, QgsMapCanvas *, QWidget *parent = 0, const char * name = 0, WFlags f = Qt::WStyle_Customize | Qt::WStyle_DialogBorder | Qt::WStyle_Title | Qt::WStyle_Dialog | Qt::WStyle_Tool );

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

  //! Connected to canvas renderComplete
  void draw(QPainter *); 

  //! Show the help for the dialog 
  void showHelp();
  
private:
  
  //! formats distance to most appropriate units
  QString formatDistance(double distance);
  
  //! formats area to most appropriate units
  QString formatArea(double area);
  
  //! shows/hides table, shows correct units
  void updateUi();
  
  QgsMapCanvas *mMapCanvas;

  QPixmap *mPixmap;
  
  //! distance/area calculator
  QgsDistanceArea* mCalc;
  
  std::vector<QgsPoint> mPoints;

  double mTotal;

  //! Dynamic line from last point to current position was drawn
  bool mDynamic;

  //! Dynamic line
  QgsPoint mDynamicPoints[2];

  //! Draw current points with XOR
  void drawLine(bool erase = false); 
  
  //! Draw current dynamic line
  void drawDynamicLine(void); 

  //! Help context id
  static const int context_id = 940759457;
  
  //! indicates whether we're measuring distances or areas
  bool mMeasureArea;
};

#endif
