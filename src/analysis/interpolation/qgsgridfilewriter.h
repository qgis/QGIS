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
#include "qgis_analysis.h"

class QgsInterpolator;
class QgsFeedback;

/**
 * \ingroup analysis
 * A class that does interpolation to a grid and writes the results to an ascii grid*/
//todo: extend such that writing to other file types is possible
class ANALYSIS_EXPORT QgsGridFileWriter
{
  public:

    /**
     * Constructor for QgsGridFileWriter, for the specified \a interpolator.
     *
     * The \a outputPath argument is used to set the output file path.
     *
     * The \a extent and \a nCols, \a nRows arguments dictate the extent and size of the output raster.
     */
    QgsGridFileWriter( QgsInterpolator *interpolator, const QString &outputPath, const QgsRectangle &extent, int nCols, int nRows );

    /**
     * Writes the grid file.
     *
     * An optional \a feedback object can be set for progress reports and cancellation support
     *
     * \returns 0 in case of success
    */
    int writeFile( QgsFeedback *feedback = nullptr );

  private:

    QgsGridFileWriter() = delete;

    int writeHeader( QTextStream &outStream );

    QgsInterpolator *mInterpolator = nullptr;
    QString mOutputFilePath;
    QgsRectangle mInterpolationExtent;
    int mNumColumns = 0;
    int mNumRows = 0;

    double mCellSizeX = 0;
    double mCellSizeY = 0;
};

#endif
