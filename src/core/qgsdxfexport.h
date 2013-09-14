/***************************************************************************
                         qgsdxfexport.h
                         --------------
    begin                : September 2013
    copyright            : (C) 2013 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSDXFEXPORT_H
#define QGSDXFEXPORT_H

#include "qgsgeometry.h"
#include <QList>

class QgsMapLayer;
class QgsPoint;
class QIODevice;
class QTextStream;

class QgsDxfExport
{
  public:
    enum SymbologyExport
    {
      NoSymbology = 0, //export only data
      FeatureSymbology, //Keeps the number of features and export symbology per feature
      SymbolLayerSymbology //Exports one feature per symbol layer (considering symbol levels)
    };

    QgsDxfExport();
    ~QgsDxfExport();

    void addLayers( QList< QgsMapLayer* >& layers ) { mLayers = layers; }
    int writeToFile( QIODevice* d, SymbologyExport s = SymbolLayerSymbology );  //maybe add progress dialog? //other parameters (e.g. scale, dpi)?

  private:

    QList< QgsMapLayer* > mLayers;

    void writeHeader( QTextStream& stream );
    void writeTables( QTextStream& stream );
    void writeEntities( QTextStream& stream );
    void writeEndFile( QTextStream& stream );

    void startSection( QTextStream& stream );
    void endSection( QTextStream& stream );

    void writePolyline( QTextStream& stream, const QgsPolyline& line, const QString& layer, bool closed = false );
    void writeVertex( QTextStream& stream, const QgsPoint& pt, const QString& layer );

    //collect styles
    //writeEntities

    //Option: export feature once / multiple export (considering symbol layers / symbol levels)
};

#endif // QGSDXFEXPORT_H
