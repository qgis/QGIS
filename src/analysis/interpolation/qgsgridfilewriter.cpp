/***************************************************************************
                              qgsgridfilewriter.cpp
                              ---------------------
  begin                : Marco 10, 2008
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

#include "qgsgridfilewriter.h"
#include "qgsinterpolator.h"
#include "qgsvectorlayer.h"
#include <QFile>
#include <QFileInfo>
#include <QProgressDialog>

QgsGridFileWriter::QgsGridFileWriter( QgsInterpolator* i, QString outputPath, QgsRectangle extent, int nCols, int nRows, double cellSizeX, double cellSizeY )
    : mInterpolator( i )
    , mOutputFilePath( outputPath )
    , mInterpolationExtent( extent )
    , mNumColumns( nCols )
    , mNumRows( nRows )
    , mCellSizeX( cellSizeX )
    , mCellSizeY( cellSizeY )
{

}

QgsGridFileWriter::QgsGridFileWriter()
    : mInterpolator( 0 )
    , mNumColumns( 0 )
    , mNumRows( 0 )
    , mCellSizeX( 0 )
    , mCellSizeY( 0 )
{

}

QgsGridFileWriter::~QgsGridFileWriter()
{

}

int QgsGridFileWriter::writeFile( bool showProgressDialog )
{
  QFile outputFile( mOutputFilePath );

  if ( !outputFile.open( QFile::WriteOnly ) )
  {
    return 1;
  }

  if ( !mInterpolator )
  {
    outputFile.remove();
    return 2;
  }

  QTextStream outStream( &outputFile );
  outStream.setRealNumberPrecision( 8 );
  writeHeader( outStream );

  double currentYValue = mInterpolationExtent.yMaximum() - mCellSizeY / 2.0; //calculate value in the center of the cell
  double currentXValue;
  double interpolatedValue;

  QProgressDialog* progressDialog = 0;
  if ( showProgressDialog )
  {
    progressDialog = new QProgressDialog( QObject::tr( "Interpolating..." ), QObject::tr( "Abort" ), 0, mNumRows, 0 );
    progressDialog->setWindowModality( Qt::WindowModal );
  }

  for ( int i = 0; i < mNumRows; ++i )
  {
    currentXValue = mInterpolationExtent.xMinimum() + mCellSizeX / 2.0; //calculate value in the center of the cell
    for ( int j = 0; j < mNumColumns; ++j )
    {
      if ( mInterpolator->interpolatePoint( currentXValue, currentYValue, interpolatedValue ) == 0 )
      {
        outStream << interpolatedValue << " ";
      }
      else
      {
        outStream << "-9999 ";
      }
      currentXValue += mCellSizeX;
    }
    outStream << endl;
    currentYValue -= mCellSizeY;

    if ( showProgressDialog )
    {
      if ( progressDialog->wasCanceled() )
      {
        outputFile.remove();
        return 3;
      }
      progressDialog->setValue( i );
    }
  }

  // create prj file
  QgsInterpolator::LayerData ld;
  ld = mInterpolator->layerData().first();
  QgsVectorLayer* vl = ld.vectorLayer;
  QString crs = vl->crs().toWkt();
  QFileInfo fi( mOutputFilePath );
  QString fileName = fi.absolutePath() + "/" + fi.completeBaseName() + ".prj";
  QFile prjFile( fileName );
  if ( !prjFile.open( QFile::WriteOnly ) )
  {
    return 1;
  }
  QTextStream prjStream( &prjFile );
  prjStream << crs;
  prjStream << endl;
  prjFile.close();

  delete progressDialog;
  return 0;
}

int QgsGridFileWriter::writeHeader( QTextStream& outStream )
{
  outStream << "NCOLS " << mNumColumns << endl;
  outStream << "NROWS " << mNumRows << endl;
  outStream << "XLLCORNER " << mInterpolationExtent.xMinimum() << endl;
  outStream << "YLLCORNER " <<  mInterpolationExtent.yMinimum() << endl;
  if ( mCellSizeX == mCellSizeY ) //standard way
  {
    outStream << "CELLSIZE " << mCellSizeX << endl;
  }
  else //this is supported by GDAL but probably not by other products
  {
    outStream << "DX " << mCellSizeX << endl;
    outStream << "DY " << mCellSizeY << endl;
  }
  outStream << "NODATA_VALUE -9999" << endl;

  return 0;
}
