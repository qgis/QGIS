/***************************************************************************
 *   Copyright (C) 2003 by Tim Sutton                                      *
 *   tim@linfiniti.com                                                     *
 *                                                                         *
 *   This is a plugin generated from the QGIS plugin template              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef PLUGINGUI_H
#define PLUGINGUI_H

#ifdef WIN32
#include "pluginguibase.h"
#else
#include "pluginguibase.uic.h"
#endif

#include <qgisiface.h>
#include <qgsvectorlayer.h>


/**
@author Tim Sutton
*/
class PluginGui : public PluginGuiBase
{
Q_OBJECT
public:
  PluginGui();
  PluginGui( QgisIface* iFace, QWidget* parent = 0, const char* name = 0, 
	     bool modal = FALSE, WFlags fl = 0 );
  ~PluginGui();
  
public slots:
  
  void pbnOK_clicked();
  void pbnCancel_clicked();
  void cmbSelectLayer_clicked();
  void pbnSelectHTML_clicked();
  void pbnSelectTemplate_clicked();
  
private:
  
  /** Polygons in shapefiles are built from one or several "rings", which are
      closed simple curves. If the points in the ring are given in clockwise
      order the ring defines the outer boundary of a polygon, if they are given
      in counterclockwise order they define an inner boundary, a hole. This
      function checks the winding of a ring (clockwise or counterclockwise)
      and returns @c true if it is an inner boundary and @c false if it is an
      outer boundary.
      @param points A pointer to the start of the ring. The ring should be
                    given as an array of doubles with alternating x and y 
		    values.
      @param nPoints The number of points in the ring (including the end point,
                     which should be equal to the first point).
  */
  bool polygonIsHole(const double* points, int nPoints);
  
  /** This function writes a cluster index. */
  void writeClusterIndex(const std::vector<QString>& urls,
			 const std::vector<QString>& alts,
			 QTextStream& stream, int clusterID);
  
  /** This is used for internal sorting algorithms. */
  static bool cluster_comp(std::pair<QgsFeature*, int> i, 
		    std::pair<QgsFeature*, int> j) {
    return i.second < j.second;
  }
  
signals:
  void drawRasterLayer(QString);
  void drawVectorLayer(QString,QString,QString);
   
private:
  QgisIface* qgisIFace;
  QgsVectorLayer* layer;
};

#endif
