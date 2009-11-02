/***************************************************************************
                              qgsgridfilewriter.h
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

#ifndef QGSGRIDFILEWRITER_H
#define QGSGRIDFILEWRITER_H

#include "qgsrectangle.h"
#include <QString>
#include <QTextStream>

class QgsInterpolator;

/**A class that does interpolation to a grid and writes the results to an ascii grid*/
//todo: extend such that writing to other file types is possible
class ANALYSIS_EXPORT QgsGridFileWriter
{
  public:
    QgsGridFileWriter( QgsInterpolator* i, QString outputPath, QgsRectangle extent, int nCols, int nRows, double cellSizeX, double cellSizeY );
    ~QgsGridFileWriter();

    /**Writes the grid file.
     @param showProgressDialog shows a dialog with the possibility to cancel
    @return 0 in case of success*/

    int writeFile( bool showProgressDialog = false );

  private:

    QgsGridFileWriter(); //forbidden
    int writeHeader( QTextStream& outStream );

    QgsInterpolator* mInterpolator;
    QString mOutputFilePath;
    QgsRectangle mInterpolationExtent;
    int mNumColumns;
    int mNumRows;

    double mCellSizeX;
    double mCellSizeY;
};

#endif
