/***************************************************************************
 *   Copyright (C) 2005 by Tim Sutton   *
 *   tim@linfiniti.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef OMGGDAL_H
#define OMGGDAL_H

#include <qobject.h>
class QString;

int  progressCallback( double dfComplete, const char *pszMessage,
                      void * pProgressArg );
class OmgGdal : public QObject
{
Q_OBJECT;

public:
  //declare eums first!
  enum FileType {GeoTiff,ArcInfoAscii};

  OmgGdal();
  ~OmgGdal();

  const QString getWorldFile(const QString theFileName) ;
  const QString getAsciiHeader(const QString theFileName) ;
   /**
  * A Qt style wrapper for gdal to convert from one file format to another.
  * @NOTE the output file name will be automatically determined by using the input file
  * name, and writing out a similarly named file with an appropriate extension.
  * @see FileType enum for the currently supported output file types
  * @param const QString theInputFile the name of the input file
  * @param const QString theOutputPath the name of the output folder
  * @param const FileType theFileType the type of the output file
  * @return const QString the name of the output file
  */
  const QString convert(const QString theInputFile, const QString theOutputPath, const FileType theOutputFileType) ;

   /**
  * A Qt style wrapper for gdal to resize a file.
  * @see FileType enum for the currently supported input file types
  * @param const QString theInputFile the name of the input file
  * @param const QString theOutputPath the name of the output folder
  * @param const int theWidth the width of output image
  * @param const int theHeight the height of output image
  * @return const QString the name of the output file
  */
//  const QString gdalResize(const QString theFileName, const QString theOutputPath, const int theWidth, const int theHeight){};

  /** Get a list of supported GDAL datatype file filters.
  *   @NOTE THis method was copied over from QGIS QgsRasterLayer
  *   @param A reference to a string. This will be populated with the results.
  *   @return void
  */
  void buildSupportedRasterFileFilter(QString & theFileFiltersString);
  
  /** Create contour lines from probability surface
  * Only the first raster band will be contoured.Contours are fixed at 10 unit increments.
  * @NOTE This method is based on the GDAL apps/gdal_contour.cpp sources
  * @param const QString theInputFile Gdal dataset to be contoured
  * @param const QString the name of the generated shapefile
  */
  const QString contour(const QString theInputFile);
  /** Call the emitter a progress signal.
  * @NOTE We need this level of indirection to support gdal style call backs.
  * @see updateProgress
  */
  void showProgress (int theProgress,int theMaximum);
signals:
  void error (QString theError);
  /** Emit a progress signal.
  * @NOTE should only ever be called using the showProgress method so we can properly 
  * support gdal style callbacks
  * @see showProgress
  */
  void updateProgress (int theProgress,int theMaximum);

private:

  const QString gdal2Tiff(const QString theFileName, const QString theOutputPath);
  const QString gdal2Ascii(const QString theFileName, const QString theOutputPath);

};
#endif //OMGGDAL_H
