/***************************************************************************
                              qgstininterpolator.h
                              --------------------
  begin                : March 10, 2008
  copyright            : (C) 2008 by Marco Hugentobler
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSTININTERPOLATOR_H
#define QGSTININTERPOLATOR_H

#include "qgsinterpolator.h"
#include <QString>

class Triangulation;
class TriangleInterpolator;
class QgsFeature;

/**Interpolation in a triangular irregular network*/
class ANALYSIS_EXPORT QgsTINInterpolator: public QgsInterpolator
{
  public:
    //describes the type of interpolation
    enum TIN_INTERPOLATION
    {
      Linear,
      CloughTocher
    };
    QgsTINInterpolator( const QList<LayerData>& inputData, TIN_INTERPOLATION interpolation = Linear, bool showProgressDialog = false );
    ~QgsTINInterpolator();

    /**Calculates interpolation value for map coordinates x, y
       @param x x-coordinate (in map units)
       @param y y-coordinate (in map units)
       @param result out: interpolation result
       @return 0 in case of success*/
    int interpolatePoint( double x, double y, double& result );

    void setExportTriangulationToFile( bool e ) {mExportTriangulationToFile = e;}
    void setTriangulationFilePath( const QString& filepath ) {mTriangulationFilePath = filepath;}

  private:
    Triangulation* mTriangulation;
    TriangleInterpolator* mTriangleInterpolator;
    bool mIsInitialized;
    bool mShowProgressDialog;
    /**If true: export triangulation to shapefile after initialisation*/
    bool mExportTriangulationToFile;
    /**File path to export the triangulation*/
    QString mTriangulationFilePath;
    /**Type of interpolation*/
    TIN_INTERPOLATION mInterpolation;

    /**Create dual edge triangulation*/
    void initialize();
    /**Inserts the vertices of a geometry into the triangulation
      @param g the geometry
      @param zCoord true if the z coordinate is the interpolation attribute
      @param attr interpolation attribute index (if zCoord is false)
      @param type point/structure line, break line
      @return 0 in case of success, -1 if the feature could not be inserted because of numerical problems*/
    int insertData( QgsFeature* f, bool zCoord, int attr, InputType type );
};

#endif
