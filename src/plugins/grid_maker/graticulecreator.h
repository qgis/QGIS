/***************************************************************************
     graticulecreator.h
     --------------------------------------
    Date                 : Sun Sep 16 12:06:50 AKDT 2007
    Copyright            : (C) 2007 by Gary E. Sherman
    Email                : sherman at mrcc dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef _GRATICULECREATOR_H_
#define _GRATICULECREATOR_H_


//qt includes
#include <QString>
//#include <qfile.h>

//QGIS includes
#include <qgsvectorfilewriter.h> //logic for writing shpfiles
#include <qgscoordinatereferencesystem.h> //needed for creating a srs
#include <qgsfield.h> //defines fieldmap too

class GraticuleCreator
{
  public:
    GraticuleCreator( QString theOutputFileName );
    ~GraticuleCreator() ;
    void generatePointGraticule(
      double theXIntervalDouble,
      double theYIntervalDouble,
      double theXOriginDouble,
      double theYOriginDouble,
      double theXEndPointDouble,
      double theYEndPointDouble );
    void generatePolygonGraticule(
      double theXIntervalDouble,
      double theYIntervalDouble,
      double theXOriginDouble,
      double theYOriginDouble,
      double theXEndPointDouble,
      double theYEndPointDouble );
    void generatePoints( QString theInputFileName );

  private:
    QString mFileName;
    QString mEncoding;
    QgsVectorFileWriter::WriterError mError;
    QgsCoordinateReferenceSystem mCRS;
    QgsFieldMap mFields;
};
#endif
