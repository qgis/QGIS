#include "qgsrasterfilewriter.h"
#include "qgsproviderregistry.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterinterface.h"
#include "qgsrasteriterator.h"
#include "qgsrasterlayer.h"
#include <QCoreApplication>
#include <QProgressDialog>
#include <QTextStream>
#include "gdal.h"

QgsRasterFileWriter::QgsRasterFileWriter( const QString& outputUrl ): mOutputUrl( outputUrl ), mOutputProviderKey( "gdal" ), mOutputFormat( "GTiff" ), mTiledMode( false ),
    /*mMaxTileWidth( 500 ), mMaxTileHeight( 500 ), */ mProgressDialog( 0 )
{

}

QgsRasterFileWriter::QgsRasterFileWriter()
{

}

QgsRasterFileWriter::~QgsRasterFileWriter()
{

}

QgsRasterFileWriter::WriterError QgsRasterFileWriter::writeRaster( QgsRasterIterator* iter, int nCols, QgsRectangle outputExtent, QProgressDialog* p )
{
  if ( !iter )
  {
    return SourceProviderError;
  }

  const QgsRasterInterface* iface = iter->input();
  if ( !iface )
  {
    return SourceProviderError;
  }

#if 0
  if ( outputExtent.isEmpty() )
  {
    outputExtent = sourceProvider->extent();
  }
#endif //0

  mProgressDialog = p;

  QgsRasterInterface::DataType debug = iface->dataType( 1 );

  if ( iface->dataType( 1 ) == QgsRasterInterface::ARGB32 )
  {
    WriterError e = writeARGBRaster( iter, nCols, outputExtent );
    mProgressDialog = 0;
    return e;
  }
  else
  {
    //
    mProgressDialog = 0;
    return NoError;
  }
}

QgsRasterFileWriter::WriterError QgsRasterFileWriter::writeRasterSingleTile( QgsRasterIterator* iter, int nCols )
{
#if 0
  if ( !sourceProvider || ! sourceProvider->isValid() )
  {
    return SourceProviderError;
  }

  QgsRasterDataProvider* destProvider = QgsRasterLayer::loadProvider( mOutputProviderKey, mOutputUrl );
  if ( !destProvider )
  {
    return DestProviderError;
  }

  QgsRectangle sourceProviderRect = sourceProvider->extent();
  double pixelSize = sourceProviderRect.width() / nCols;
  int nRows = ( double )nCols / sourceProviderRect.width() * sourceProviderRect.height() + 0.5;
  double geoTransform[6];
  geoTransform[0] = sourceProviderRect.xMinimum();
  geoTransform[1] = pixelSize;
  geoTransform[2] = 0.0;
  geoTransform[3] = sourceProviderRect.yMaximum();
  geoTransform[4] = 0.0;
  geoTransform[5] = -pixelSize;

  //debug
  bool hasARGBType = false;
  QgsRasterInterface::DataType outputDataType = ( QgsRasterInterface::DataType )sourceProvider->dataType( 1 );
  int nOutputBands = sourceProvider->bandCount();
  int nInputBands = sourceProvider->bandCount();
  if ( outputDataType == QgsRasterInterface::::ARGBDataType )
  {
    hasARGBType = true; //needs to be converted to four band 8bit
    outputDataType = QgsRasterInterface::Byte;
    nOutputBands = 4;
  }

  if ( !destProvider->create( mOutputFormat, nOutputBands, outputDataType, nCols, nRows, geoTransform,
                              sourceProvider->crs() ) )
  {
    delete destProvider;
    return CreateDatasourceError;
  }

  if ( hasARGBType && nInputBands == 1 )
  {
    //For ARGB data, always use 1 input band and four int8 output bands
    int nPixels = nCols * nRows;
    int dataSize = destProvider->dataTypeSize( 1 ) * nPixels;
    void* data = VSIMalloc( dataSize );
    sourceProvider->readBlock( 1, sourceProviderRect, nCols, nRows, data );

    //data for output bands
    void* redData = VSIMalloc( nPixels );
    void* greenData = VSIMalloc( nPixels );
    void* blueData = VSIMalloc( nPixels );
    void* alphaData = VSIMalloc( nPixels );

    int red = 0;
    int green = 0;
    int blue = 0;
    int alpha = 255;
    uint* p = ( uint* ) data;
    for ( int i = 0; i < nPixels; ++i )
    {
      QRgb c( *p++ );
      red = qRed( c ); green = qGreen( c ); blue = qBlue( c ); alpha = qAlpha( c );
      memcpy(( char* )redData + i, &red, 1 );
      memcpy(( char* )greenData + i, &green, 1 );
      memcpy(( char* )blueData + i, &blue, 1 );
      memcpy(( char* )alphaData + i, &alpha, 1 );
    }
    destProvider->write( redData, 1, nCols, nRows, 0, 0 );
    destProvider->write( greenData, 2, nCols, nRows, 0, 0 );
    destProvider->write( blueData, 3, nCols, nRows, 0, 0 );
    destProvider->write( alphaData, 4, nCols, nRows, 0, 0 );
  }
  else
  {
    //read/write data for each band
    for ( int i = 0; i < sourceProvider->bandCount(); ++i )
    {
      void* data = VSIMalloc( destProvider->dataTypeSize( i + 1 ) * nCols * nRows );
      sourceProvider->readBlock( i + 1, sourceProviderRect, nCols, nRows, data );
      bool writeSuccess = destProvider->write( data, i + 1, nCols, nRows, 0, 0 );
      CPLFree( data );
      if ( !writeSuccess )
      {
        delete destProvider;
        return WriteError;
      }
    }
  }

  delete destProvider;
#endif //0
  return NoError;
}

QgsRasterFileWriter::WriterError QgsRasterFileWriter::writeARGBRaster( QgsRasterIterator* iter, int nCols, const QgsRectangle& outputExtent )
{
#if 0
  if ( !iter )
  {
    return SourceProviderError;
  }

  const QgsRasterInterface* iface = iter->input();
  if ( !iface ||  iface->dataType( 1 ) != QgsRasterInterface::ARGB32 )
  {
    return SourceProviderError;
  }

  //create directory for output files
  QDir destDir( mOutputUrl );
  if ( mTiledMode )
  {
    destDir.mkdir( mOutputUrl );
  }

  //Get output map units per pixel
  double outputMapUnitsPerPixel = outputExtent.width() / nCols;

  QgsRasterIterator iter( 1, sourceProvider );
  iter.setMaximumTileWidth( mMaxTileWidth );
  iter.setMaximumTileHeight( mMaxTileHeight );

  void* data = VSIMalloc( iface->typeSize( QgsRasterInterface::ARGB32 ) * mMaxTileWidth * mMaxTileHeight );
  void* redData = VSIMalloc( mMaxTileWidth * mMaxTileHeight );
  void* greenData = VSIMalloc( mMaxTileWidth * mMaxTileHeight );
  void* blueData = VSIMalloc( mMaxTileWidth * mMaxTileHeight );
  void* alphaData = VSIMalloc( mMaxTileWidth * mMaxTileHeight );
  QgsRectangle mapRect;
  int iterLeft, iterTop, iterCols, iterRows;
  double progress = 0;
  int fileIndex = 0;

  QgsRasterDataProvider* destProvider = 0;

  //create destProvider for whole dataset here
  double pixelSize = outputExtent.width() / nCols;
  int nRows = ( double )nCols / outputExtent.width() * outputExtent.height() + 0.5;
  double geoTransform[6];
  geoTransform[0] = outputExtent.xMinimum();
  geoTransform[1] = pixelSize;
  geoTransform[2] = 0.0;
  geoTransform[3] = outputExtent.yMaximum();
  geoTransform[4] = 0.0;
  geoTransform[5] = -pixelSize;

  //where to get CRS from?
  QgsCoordinateReferenceSystem fakeCRS;
  fakeCRS.createFromEpsg( 21781 );

  if ( mTiledMode )
  {
    createVRT( nCols, nRows, fakeCRS, geoTransform );
  }
  else
  {
    destProvider = QgsRasterLayer::loadProvider( mOutputProviderKey, mOutputUrl );
    if ( !destProvider )
    {
      return DestProviderError;
    }

    if ( !destProvider->create( mOutputFormat, 4, QgsRasterInterface::Byte, nCols, nRows, geoTransform,
                                fakeCRS ) )
    {
      delete destProvider;
      return CreateDatasourceError;
    }
  }

  QgsRasterViewPort viewPort;
  viewPort.drawableAreaXDim = nCols;
  viewPort.drawableAreaYDim = nRows;

  QgsMapUnitsPerPixel mup;

  //iter->select( outputExtent, outputMapUnitsPerPixel );
  iter->startRasterRead( 1, &viewPort, &mup );

  //initialize progress dialog
  int nTiles = iter.nTilesX() * iter.nTilesY();
  if ( mProgressDialog )
  {
    mProgressDialog->setWindowTitle( QObject::tr( "Save raster" ) );
    mProgressDialog->setMaximum( nTiles );
    mProgressDialog->show();
  }

  while ( iter->nextPart( data, mapRect, iterLeft, iterTop, iterCols, iterRows, progress ) )
  {
    if ( iterCols <= 5 || iterRows <= 5 ) //some wms servers don't like small values
    {
      continue;
    }

    if ( mapRect.width() < 0.000000001 || mapRect.height() < 0.000000001 )
    {
      continue;
    }

    if ( mProgressDialog )
    {
      mProgressDialog->setValue( fileIndex + 1 );
      mProgressDialog->setLabelText( QObject::tr( "Downloaded raster tile %1 from %2" ).arg( fileIndex + 1 ).arg( nTiles ) );
      qApp->processEvents( QEventLoop::AllEvents, 2000 );
      if ( mProgressDialog->wasCanceled() )
      {
        break;
      }
    }

    //fill into red/green/blue/alpha channels
    uint* p = ( uint* ) data;
    int nPixels = iterCols * iterRows;
    int red = 0;
    int green = 0;
    int blue = 0;
    int alpha = 255;
    for ( int i = 0; i < nPixels; ++i )
    {
      QRgb c( *p++ );
      red = qRed( c ); green = qGreen( c ); blue = qBlue( c ); alpha = qAlpha( c );
      memcpy(( char* )redData + i, &red, 1 );
      memcpy(( char* )greenData + i, &green, 1 );
      memcpy(( char* )blueData + i, &blue, 1 );
      memcpy(( char* )alphaData + i, &alpha, 1 );
    }

    //create output file
    if ( mTiledMode )
    {
      QString outputFile = mOutputUrl + "/" + QString::number( fileIndex );
      delete destProvider;
      destProvider = QgsRasterLayer::loadProvider( mOutputProviderKey, outputFile );
      if ( !destProvider )
      {
        return DestProviderError;
      }

      //geotransform
      double geoTransform[6];
      geoTransform[0] = mapRect.xMinimum();
      geoTransform[1] = outputMapUnitsPerPixel;
      geoTransform[2] = 0.0;
      geoTransform[3] = mapRect.yMaximum();
      geoTransform[4] = 0.0;
      geoTransform[5] = -outputMapUnitsPerPixel;
      if ( !destProvider->create( mOutputFormat, 4, QgsRasterInterface::Byte, iterCols, iterRows, geoTransform,
                                  fakeCRS ) )
      {
        delete destProvider;
        return CreateDatasourceError;
      }

      //write data to output file
      destProvider->write( redData, 1, iterCols, iterRows, 0, 0 );
      destProvider->write( greenData, 2, iterCols, iterRows, 0, 0 );
      destProvider->write( blueData, 3, iterCols, iterRows, 0, 0 );
      destProvider->write( alphaData, 4, iterCols, iterRows, 0, 0 );

      addToVRT( QString::number( fileIndex ), 1, iterCols, iterRows, iterLeft, iterTop );
      addToVRT( QString::number( fileIndex ), 2, iterCols, iterRows, iterLeft, iterTop );
      addToVRT( QString::number( fileIndex ), 3, iterCols, iterRows, iterLeft, iterTop );
      addToVRT( QString::number( fileIndex ), 4, iterCols, iterRows, iterLeft, iterTop );
    }
    else
    {
      destProvider->write( redData, 1, iterCols, iterRows, iterLeft, iterTop );
      destProvider->write( greenData, 2, iterCols, iterRows, iterLeft, iterTop );
      destProvider->write( blueData, 3, iterCols, iterRows, iterLeft, iterTop );
      destProvider->write( alphaData, 4, iterCols, iterRows, iterLeft, iterTop );
    }

    ++fileIndex;
  }
  delete destProvider;
  CPLFree( data ); CPLFree( redData ); CPLFree( greenData ); CPLFree( blueData ); CPLFree( alphaData );

  if ( mProgressDialog )
  {
    mProgressDialog->setValue( nTiles );
  }

  if ( mTiledMode )
  {
    QFileInfo outputInfo( mOutputUrl );
    QString vrtFilePath( mOutputUrl + "/" + outputInfo.baseName() + ".vrt" );
    writeVRT( vrtFilePath );
    buildPyramides( vrtFilePath );
  }
#endif //0
  return NoError;
}

void QgsRasterFileWriter::addToVRT( const QString& filename, int band, int xSize, int ySize, int xOffset, int yOffset )
{
  QDomElement bandElem;

  switch ( band )
  {
    case 1:
      bandElem = mVRTRedBand;
      break;
    case 2:
      bandElem = mVRTGreenBand;
      break;
    case 3:
      bandElem = mVRTBlueBand;
      break;
    case 4:
      bandElem = mVRTAlphaBand;
      break;
    default:
      return;
  }

  QDomElement simpleSourceElem = mVRTDocument.createElement( "SimpleSource" );

  //SourceFilename
  QDomElement sourceFilenameElem = mVRTDocument.createElement( "SourceFilename" );
  sourceFilenameElem.setAttribute( "relativeToVRT", "1" );
  QDomText sourceFilenameText = mVRTDocument.createTextNode( filename );
  sourceFilenameElem.appendChild( sourceFilenameText );
  simpleSourceElem.appendChild( sourceFilenameElem );

  //SourceBand
  QDomElement sourceBandElem = mVRTDocument.createElement( "SourceBand" );
  QDomText sourceBandText = mVRTDocument.createTextNode( QString::number( band ) );
  sourceBandElem.appendChild( sourceBandText );
  simpleSourceElem.appendChild( sourceBandElem );

  //SourceProperties
  QDomElement sourcePropertiesElem = mVRTDocument.createElement( "SourceProperties" );
  sourcePropertiesElem.setAttribute( "RasterXSize", xSize );
  sourcePropertiesElem.setAttribute( "RasterYSize", ySize );
  sourcePropertiesElem.setAttribute( "BlockXSize", xSize );
  sourcePropertiesElem.setAttribute( "BlockYSize", ySize );
  sourcePropertiesElem.setAttribute( "DataType", "Byte" );
  simpleSourceElem.appendChild( sourcePropertiesElem );

  //SrcRect
  QDomElement srcRectElem = mVRTDocument.createElement( "SrcRect" );
  srcRectElem.setAttribute( "xOff", "0" );
  srcRectElem.setAttribute( "yOff", "0" );
  srcRectElem.setAttribute( "xSize", xSize );
  srcRectElem.setAttribute( "ySize", ySize );
  simpleSourceElem.appendChild( srcRectElem );

  //DstRect
  QDomElement dstRectElem = mVRTDocument.createElement( "DstRect" );
  dstRectElem.setAttribute( "xOff", xOffset );
  dstRectElem.setAttribute( "yOff", yOffset );
  dstRectElem.setAttribute( "xSize", xSize );
  dstRectElem.setAttribute( "ySize", ySize );
  simpleSourceElem.appendChild( dstRectElem );

  bandElem.appendChild( simpleSourceElem );
}

void QgsRasterFileWriter::buildPyramides( const QString& filename )
{
  GDALDatasetH dataSet;
  GDALAllRegister();
  dataSet = GDALOpen( filename.toLocal8Bit().data(), GA_Update );
  if ( !dataSet )
  {
    return;
  }

  //2,4,8,16,32,64
  int overviewList[6];
  overviewList[0] = 2;
  overviewList[1] = 4;
  overviewList[2] = 8;
  overviewList[3] = 16;
  overviewList[4] = 32;
  overviewList[5] = 64;

  /*if ( mProgressDialog )
  {
    mProgressDialog->setLabelText( QObject::tr( "Building Pyramides..." ) );
    mProgressDialog->setValue( 0 );
    mProgressDialog->setWindowModality( Qt::WindowModal );
    mProgressDialog->show();
  }*/
  GDALBuildOverviews( dataSet, "AVERAGE", 6, overviewList, 0, 0, /*pyramidesProgress*/ 0, /*mProgressDialog*/ 0 );
}

int QgsRasterFileWriter::pyramidesProgress( double dfComplete, const char *pszMessage, void* pData )
{
  Q_UNUSED( pszMessage );
  GDALTermProgress( dfComplete, 0, 0 );
  QProgressDialog* p = static_cast<QProgressDialog*>( pData );
  if ( pData && p->wasCanceled() )
  {
    return 0;
  }

  if ( pData )
  {
    p->setRange( 0, 100 );
    p->setValue( dfComplete * 100 );
  }
  return 1;
}

void QgsRasterFileWriter::createVRT( int xSize, int ySize, const QgsCoordinateReferenceSystem& crs, double* geoTransform )
{
  mVRTDocument.clear();
  QDomElement VRTDatasetElem = mVRTDocument.createElement( "VRTDataset" );

  //xsize / ysize
  VRTDatasetElem.setAttribute( "rasterXSize", xSize );
  VRTDatasetElem.setAttribute( "rasterYSize", ySize );
  mVRTDocument.appendChild( VRTDatasetElem );

  //CRS
  QDomElement SRSElem = mVRTDocument.createElement( "SRS" );
  QDomText crsText = mVRTDocument.createTextNode( crs.toWkt() );
  SRSElem.appendChild( crsText );
  VRTDatasetElem.appendChild( SRSElem );

  //geotransform
  if ( geoTransform )
  {
    QDomElement geoTransformElem = mVRTDocument.createElement( "GeoTransform" );
    QString geoTransformString = QString::number( geoTransform[0] ) + ", " + QString::number( geoTransform[1] ) + ", " + QString::number( geoTransform[2] ) +
                                 ", "  + QString::number( geoTransform[3] ) + ", " + QString::number( geoTransform[4] ) + ", " + QString::number( geoTransform[5] );
    QDomText geoTransformText = mVRTDocument.createTextNode( geoTransformString );
    geoTransformElem.appendChild( geoTransformText );
    VRTDatasetElem.appendChild( geoTransformElem );
  }

  //VRT rasterbands
  mVRTRedBand = mVRTDocument.createElement( "VRTRasterBand" );
  mVRTRedBand.setAttribute( "dataType", "Byte" );
  mVRTRedBand.setAttribute( "band", "1" );
  QDomElement colorInterpRedElement = mVRTDocument.createElement( "ColorInterp" );
  QDomText redInterprText = mVRTDocument.createTextNode( "Red" );
  colorInterpRedElement.appendChild( redInterprText );
  mVRTRedBand.appendChild( colorInterpRedElement );

  mVRTGreenBand = mVRTDocument.createElement( "VRTRasterBand" );
  mVRTGreenBand.setAttribute( "dataType", "Byte" );
  mVRTGreenBand.setAttribute( "band", "2" );
  QDomElement colorInterpGreenElement = mVRTDocument.createElement( "ColorInterp" );
  QDomText greenInterprText = mVRTDocument.createTextNode( "Green" );
  colorInterpGreenElement.appendChild( greenInterprText );
  mVRTGreenBand.appendChild( colorInterpGreenElement );

  mVRTBlueBand = mVRTDocument.createElement( "VRTRasterBand" );
  mVRTBlueBand.setAttribute( "dataType", "Byte" );
  mVRTBlueBand.setAttribute( "band", "3" );
  QDomElement colorInterpBlueElement = mVRTDocument.createElement( "ColorInterp" );
  QDomText blueInterprText = mVRTDocument.createTextNode( "Blue" );
  colorInterpBlueElement.appendChild( blueInterprText );
  mVRTBlueBand.appendChild( colorInterpBlueElement );

  mVRTAlphaBand = mVRTDocument.createElement( "VRTRasterBand" );
  mVRTAlphaBand.setAttribute( "dataType", "Byte" );
  mVRTAlphaBand.setAttribute( "band", "4" );
  QDomElement colorInterpAlphaElement = mVRTDocument.createElement( "ColorInterp" );
  QDomText alphaInterprText = mVRTDocument.createTextNode( "Alpha" );
  colorInterpAlphaElement.appendChild( alphaInterprText );
  mVRTAlphaBand.appendChild( colorInterpAlphaElement );

  VRTDatasetElem.appendChild( mVRTRedBand );
  VRTDatasetElem.appendChild( mVRTGreenBand );
  VRTDatasetElem.appendChild( mVRTBlueBand );
  VRTDatasetElem.appendChild( mVRTAlphaBand );
}

bool QgsRasterFileWriter::writeVRT( const QString& file )
{
  QFile outputFile( file );
  if ( ! outputFile.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
  {
    return false;
  }

  QTextStream outStream( &outputFile );
  mVRTDocument.save( outStream, 2 );
  return true;
}




