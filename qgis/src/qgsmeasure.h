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
  QgsMeasure(QgsMapCanvas *, QWidget *parent = 0, const char * name = 0, 
	     WFlags f = Qt::WStyle_Customize | Qt::WStyle_Dialog );

  ~QgsMeasure();

  //! Save position
  void saveWindowLocation(void);

  //! Restore last window position/size and show the window
  void restorePosition(void);

  //! Add new point
  void addPoint(QgsPoint &point);
  
  //! Mose move
  void mouseMove(QgsPoint &point);

public slots:
  //! Close
  void close ( void);

  //! Reset and start new
  void restart ( void);

  //! Close event
  void closeEvent(QCloseEvent *e);

  //! Connected to canvas renderComplete
  void draw(QPainter *); 
  
private:
  QgsMapCanvas *mMapCanvas;

  QPixmap *mPixmap;

  std::vector<QgsPoint> mPoints;

  double mTotal;

  //! Dynamic line from last point to current position was drawn
  bool mDynamic;

  //! Dynamic line
  QgsPoint mDynamicPoints[2];

  //! Draw current points with XOR
  void drawLine(void); 
  
  //! Draw current dynamic line
  void drawDynamicLine(void); 
};

#endif
