/***************************************************************************
                          qgsrastercalculator.cpp  -  description
                          -----------------------
    begin                : September 28th, 2010
    copyright            : (C) 2010 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgdalutils.h"
#include "qgsrastercalculator.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterinterface.h"
#include "qgsrasterlayer.h"
#include "qgsrastermatrix.h"
#include "qgsrasterprojector.h"
#include "qgsfeedback.h"
#include "qgsogrutils.h"
#include "qgsproject.h"

#include <QFile>

#include <cpl_string.h>
#include <gdalwarper.h>

#ifdef HAVE_OPENCL
#include "qgsopenclutils.h"
#include "qgsgdalutils.h"
#endif


//
// global callback function
//
int CPL_STDCALL GdalProgressCallback( double dfComplete,
                                      const char *pszMessage,
                                      void *pProgressArg )
{
  Q_UNUSED( pszMessage )

  static double sDfLastComplete = -1.0;

  QgsFeedback *feedback = static_cast<QgsFeedback *>( pProgressArg );

  if ( sDfLastComplete > dfComplete )
  {
    if ( sDfLastComplete >= 1.0 )
      sDfLastComplete = -1.0;
    else
      sDfLastComplete = dfComplete;
  }

  if ( std::floor( sDfLastComplete * 10 ) != std::floor( dfComplete * 10 ) )
  {
    if ( feedback )
      feedback->setProgress( dfComplete * 100 );
  }
  sDfLastComplete = dfComplete;

  if ( feedback && feedback->isCanceled() )
    return false;

  return true;
}

QgsRasterCalculator::QgsRasterCalculator( const QString &formulaString, const QString &outputFile, const QString &outputFormat, const QgsRectangle &outputExtent, int nOutputColumns, int nOutputRows, const QVector<QgsRasterCalculatorEntry> &rasterEntries, const QgsCoordinateTransformContext &transformContext )
  : mFormulaString( formulaString )
  , mOutputFile( outputFile )
  , mOutputFormat( outputFormat )
  , mOutputRectangle( outputExtent )
  , mNumOutputColumns( nOutputColumns )
  , mNumOutputRows( nOutputRows )
  , mRasterEntries( rasterEntries )
  , mTransformContext( transformContext )
{

}

QgsRasterCalculator::QgsRasterCalculator( const QString &formulaString, const QString &outputFile, const QString &outputFormat,
    const QgsRectangle &outputExtent, const QgsCoordinateReferenceSystem &outputCrs, int nOutputColumns, int nOutputRows,
    const QVector<QgsRasterCalculatorEntry> &rasterEntries, const QgsCoordinateTransformContext &transformContext )
  : mFormulaString( formulaString )
  , mOutputFile( outputFile )
  , mOutputFormat( outputFormat )
  , mOutputRectangle( outputExtent )
  , mOutputCrs( outputCrs )
  , mNumOutputColumns( nOutputColumns )
  , mNumOutputRows( nOutputRows )
  , mRasterEntries( rasterEntries )
  , mTransformContext( transformContext )
{

}

// Deprecated!
QgsRasterCalculator::QgsRasterCalculator( const QString &formulaString, const QString &outputFile, const QString &outputFormat,
    const QgsRectangle &outputExtent, int nOutputColumns, int nOutputRows, const QVector<QgsRasterCalculatorEntry> &rasterEntries )
  : mFormulaString( formulaString )
  , mOutputFile( outputFile )
  , mOutputFormat( outputFormat )
  , mOutputRectangle( outputExtent )
  , mNumOutputColumns( nOutputColumns )
  , mNumOutputRows( nOutputRows )
  , mRasterEntries( rasterEntries )
{
  //default to first layer's crs
  mOutputCrs = mRasterEntries.at( 0 ).raster->crs();
  mTransformContext = QgsProject::instance()->transformContext();
}


// Deprecated!
QgsRasterCalculator::QgsRasterCalculator( const QString &formulaString, const QString &outputFile, const QString &outputFormat,
    const QgsRectangle &outputExtent, const QgsCoordinateReferenceSystem &outputCrs, int nOutputColumns, int nOutputRows, const QVector<QgsRasterCalculatorEntry> &rasterEntries )
  : mFormulaString( formulaString )
  , mOutputFile( outputFile )
  , mOutputFormat( outputFormat )
  , mOutputRectangle( outputExtent )
  , mOutputCrs( outputCrs )
  , mNumOutputColumns( nOutputColumns )
  , mNumOutputRows( nOutputRows )
  , mRasterEntries( rasterEntries )
  , mTransformContext( QgsProject::instance()->transformContext() )
{
}

QgsRasterCalculator::Result QgsRasterCalculator::processCalculation( QgsFeedback *feedback )
{
  mLastError.clear();

  //prepare search string / tree
  std::unique_ptr< QgsRasterCalcNode > calcNode( QgsRasterCalcNode::parseRasterCalcString( mFormulaString, mLastError ) );
  if ( !calcNode )
  {
    //error
    return ParserError;
  }

  // Check input layers and bands
  for ( const auto &entry : std::as_const( mRasterEntries ) )
  {
    if ( !entry.raster ) // no raster layer in entry
    {
      mLastError = QObject::tr( "No raster layer for entry %1" ).arg( entry.ref );
      return InputLayerError;
    }
    if ( entry.bandNumber <= 0 || entry.bandNumber > entry.raster->bandCount() )
    {
      mLastError = QObject::tr( "Band number %1 is not valid for entry %2" ).arg( entry.bandNumber ).arg( entry.ref );
      return BandError;
    }
  }

  // Check if we need to read the raster as a whole (which is memory inefficient
  // and not interruptible by the user) by checking if any raster matrix nodes are
  // in the expression
  bool requiresMatrix = ! calcNode->findNodes( QgsRasterCalcNode::Type::tMatrix ).isEmpty();

#ifdef HAVE_OPENCL
  // Check for matrix nodes, GPU implementation does not support them
  if ( QgsOpenClUtils::enabled() && QgsOpenClUtils::available() && ! requiresMatrix )
  {
    return processCalculationGPU( std::move( calcNode ), feedback );
  }
#endif

  //open output dataset for writing
  GDALDriverH outputDriver = openOutputDriver();
  if ( !outputDriver )
  {
    mLastError = QObject::tr( "Could not obtain driver for %1" ).arg( mOutputFormat );
    return CreateOutputError;
  }

  gdal::dataset_unique_ptr outputDataset( openOutputFile( outputDriver ) );
  if ( !outputDataset )
  {
    mLastError = QObject::tr( "Could not create output %1" ).arg( mOutputFile );
    return CreateOutputError;
  }

  GDALSetProjection( outputDataset.get(), mOutputCrs.toWkt( QgsCoordinateReferenceSystem::WKT_PREFERRED_GDAL ).toLocal8Bit().data() );
  GDALRasterBandH outputRasterBand = GDALGetRasterBand( outputDataset.get(), 1 );

  float outputNodataValue = -FLT_MAX;
  GDALSetRasterNoDataValue( outputRasterBand, outputNodataValue );


  // Take the fast route (process one line at a time) if we can
  if ( ! requiresMatrix )
  {
    // Map of raster names -> blocks
    std::map<QString, std::unique_ptr<QgsRasterBlock>> inputBlocks;
    std::map<QString, QgsRasterCalculatorEntry> uniqueRasterEntries;
    const QList<const QgsRasterCalcNode *> rasterRefNodes = calcNode->findNodes( QgsRasterCalcNode::Type::tRasterRef );
    for ( const QgsRasterCalcNode *r : rasterRefNodes )
    {
      QString layerRef( r->toString().remove( 0, 1 ) );
      layerRef.chop( 1 );
      if ( ! inputBlocks.count( layerRef ) )
      {
        for ( const QgsRasterCalculatorEntry &ref : std::as_const( mRasterEntries ) )
        {
          if ( ref.ref == layerRef )
          {
            uniqueRasterEntries[layerRef] = ref;
            inputBlocks[layerRef ] = std::make_unique<QgsRasterBlock>();
          }
        }
      }
    }

    //read / write line by line
    QMap<QString, QgsRasterBlock * > _rasterData;
    // Cast to float
    std::vector<float> castedResult( static_cast<size_t>( mNumOutputColumns ), 0 );
    auto rowHeight = mOutputRectangle.height() / mNumOutputRows;
    for ( size_t row = 0; row < static_cast<size_t>( mNumOutputRows ); ++row )
    {
      if ( feedback )
      {
        feedback->setProgress( 100.0 * static_cast< double >( row ) / mNumOutputRows );
      }

      if ( feedback && feedback->isCanceled() )
      {
        break;
      }

      // Calculates the rect for a single row read
      QgsRectangle rect( mOutputRectangle );
      rect.setYMaximum( rect.yMaximum() - rowHeight * row );
      rect.setYMinimum( rect.yMaximum() - rowHeight );

      // Read rows into input blocks
      for ( auto &layerRef : inputBlocks )
      {
        QgsRasterCalculatorEntry ref = uniqueRasterEntries[layerRef.first];
        if ( ref.raster->crs() != mOutputCrs )
        {
          QgsRasterProjector proj;
          proj.setCrs( ref.raster->crs(), mOutputCrs, mTransformContext );
          proj.setInput( ref.raster->dataProvider() );
          proj.setPrecision( QgsRasterProjector::Exact );
          layerRef.second.reset( proj.block( ref.bandNumber, rect, mNumOutputColumns, 1 ) );
        }
        else
        {
          layerRef.second.reset( ref.raster->dataProvider()->block( ref.bandNumber, rect, mNumOutputColumns, 1 ) );
        }
      }

      // 1 row X mNumOutputColumns matrix
      QgsRasterMatrix resultMatrix( mNumOutputColumns, 1, nullptr, outputNodataValue );

      _rasterData.clear();
      for ( const auto &layerRef : inputBlocks )
      {
        _rasterData.insert( layerRef.first, layerRef.second.get() );
      }

      if ( calcNode->calculate( _rasterData, resultMatrix, 0 ) )
      {
        std::copy( resultMatrix.data(), resultMatrix.data() + mNumOutputColumns, castedResult.begin() );
        if ( GDALRasterIO( outputRasterBand, GF_Write, 0, row, mNumOutputColumns, 1, castedResult.data(), mNumOutputColumns, 1, GDT_Float32, 0, 0 ) != CE_None )
        {
          QgsDebugMsg( QStringLiteral( "RasterIO error!" ) );
        }
      }
      else
      {
        //delete the dataset without closing (because it is faster)
        gdal::fast_delete_and_close( outputDataset, outputDriver, mOutputFile );
        return CalculationError;
      }
    }

    if ( feedback )
    {
      feedback->setProgress( 100.0 );
    }
  }
  else  // Original code (memory inefficient route)
  {
    QMap< QString, QgsRasterBlock * > inputBlocks;
    QVector<QgsRasterCalculatorEntry>::const_iterator it = mRasterEntries.constBegin();
    for ( ; it != mRasterEntries.constEnd(); ++it )
    {

      std::unique_ptr< QgsRasterBlock > block;
      // if crs transform needed
      if ( it->raster->crs() != mOutputCrs )
      {
        QgsRasterProjector proj;
        proj.setCrs( it->raster->crs(), mOutputCrs, it->raster->transformContext() );
        proj.setInput( it->raster->dataProvider() );
        proj.setPrecision( QgsRasterProjector::Exact );

        QgsRasterBlockFeedback *rasterBlockFeedback = new QgsRasterBlockFeedback();
        QObject::connect( feedback, &QgsFeedback::canceled, rasterBlockFeedback, &QgsRasterBlockFeedback::cancel );
        block.reset( proj.block( it->bandNumber, mOutputRectangle, mNumOutputColumns, mNumOutputRows, rasterBlockFeedback ) );
        if ( rasterBlockFeedback->isCanceled() )
        {
          qDeleteAll( inputBlocks );
          return Canceled;
        }
      }
      else
      {
        block.reset( it->raster->dataProvider()->block( it->bandNumber, mOutputRectangle, mNumOutputColumns, mNumOutputRows ) );
      }
      if ( block->isEmpty() )
      {
        mLastError = QObject::tr( "Could not allocate required memory for %1" ).arg( it->ref );
        qDeleteAll( inputBlocks );
        return MemoryError;
      }
      inputBlocks.insert( it->ref, block.release() );
    }

    QgsRasterMatrix resultMatrix;
    resultMatrix.setNodataValue( outputNodataValue );

    //read / write line by line
    for ( int i = 0; i < mNumOutputRows; ++i )
    {
      if ( feedback )
      {
        feedback->setProgress( 100.0 * static_cast< double >( i ) / mNumOutputRows );
      }

      if ( feedback && feedback->isCanceled() )
      {
        break;
      }

      if ( calcNode->calculate( inputBlocks, resultMatrix, i ) )
      {
        bool resultIsNumber = resultMatrix.isNumber();
        float *calcData = new float[mNumOutputColumns];

        for ( int j = 0; j < mNumOutputColumns; ++j )
        {
          calcData[j] = ( float )( resultIsNumber ? resultMatrix.number() : resultMatrix.data()[j] );
        }

        //write scanline to the dataset
        if ( GDALRasterIO( outputRasterBand, GF_Write, 0, i, mNumOutputColumns, 1, calcData, mNumOutputColumns, 1, GDT_Float32, 0, 0 ) != CE_None )
        {
          QgsDebugMsg( QStringLiteral( "RasterIO error!" ) );
        }

        delete[] calcData;
      }
      else
      {
        qDeleteAll( inputBlocks );
        inputBlocks.clear();
        gdal::fast_delete_and_close( outputDataset, outputDriver, mOutputFile );
        return CalculationError;
      }

    }

    if ( feedback )
    {
      feedback->setProgress( 100.0 );
    }

    //close datasets and release memory
    calcNode.reset();
    qDeleteAll( inputBlocks );
    inputBlocks.clear();

  }

  if ( feedback && feedback->isCanceled() )
  {
    //delete the dataset without closing (because it is faster)
    gdal::fast_delete_and_close( outputDataset, outputDriver, mOutputFile );
    return Canceled;
  }

  GDALComputeRasterStatistics( outputRasterBand, true, nullptr, nullptr, nullptr, nullptr, GdalProgressCallback, feedback );

  return Success;
}

#ifdef HAVE_OPENCL
QgsRasterCalculator::Result QgsRasterCalculator::processCalculationGPU( std::unique_ptr< QgsRasterCalcNode > calcNode, QgsFeedback *feedback )
{

  QString cExpression( calcNode->toString( true ) );

  QList<const QgsRasterCalcNode *> nodeList( calcNode->findNodes( QgsRasterCalcNode::Type::tRasterRef ) );
  QSet<QString> capturedTexts;
  for ( const auto &r : std::as_const( nodeList ) )
  {
    QString s( r->toString().remove( 0, 1 ) );
    s.chop( 1 );
    capturedTexts.insert( s );
  }

  // Extract all references
  struct LayerRef
  {
    QString name;
    int band;
    QgsRasterLayer *layer = nullptr;
    QString varName;
    QString typeName;
    size_t index;
    size_t bufferSize;
    size_t dataSize;
  };

  // Collects all layers, band, name, varName and size information
  std::vector<LayerRef> inputRefs;
  size_t refCounter = 0;
  for ( const auto &r : capturedTexts )
  {
    if ( r.startsWith( '"' ) )
      continue;
    QStringList parts( r.split( '@' ) );
    if ( parts.count() != 2 )
      continue;
    bool ok = false;
    LayerRef entry;
    entry.name = r;
    entry.band = parts[1].toInt( &ok );
    for ( const auto &ref : std::as_const( mRasterEntries ) )
    {
      if ( ref.ref == entry.name )
        entry.layer = ref.raster;
    }
    if ( !( entry.layer && entry.layer->dataProvider() && ok ) )
      continue;
    entry.dataSize = entry.layer->dataProvider()->dataTypeSize( entry.band );
    switch ( entry.layer->dataProvider()->dataType( entry.band ) )
    {
      case Qgis::DataType::Byte:
        entry.typeName = QStringLiteral( "unsigned char" );
        break;
      case Qgis::DataType::UInt16:
        entry.typeName = QStringLiteral( "unsigned int" );
        break;
      case Qgis::DataType::Int16:
        entry.typeName = QStringLiteral( "short" );
        break;
      case Qgis::DataType::UInt32:
        entry.typeName = QStringLiteral( "unsigned int" );
        break;
      case Qgis::DataType::Int32:
        entry.typeName = QStringLiteral( "int" );
        break;
      case Qgis::DataType::Float32:
        entry.typeName = QStringLiteral( "float" );
        break;
      // FIXME: not sure all OpenCL implementations support double
      //        maybe safer to fall back to the CPU implementation
      //        after a compatibility check
      case Qgis::DataType::Float64:
        entry.typeName = QStringLiteral( "double" );
        break;
      default:
        return BandError;
    }
    entry.bufferSize = entry.dataSize * mNumOutputColumns;
    entry.index = refCounter;
    entry.varName = QStringLiteral( "input_raster_%1_band_%2" )
                    .arg( refCounter++ )
                    .arg( entry.band );
    inputRefs.push_back( entry );
  }

  // May throw an openCL exception
  try
  {
    // Prepare context and queue
    cl::Context ctx( QgsOpenClUtils::context() );
    cl::CommandQueue queue( QgsOpenClUtils::commandQueue() );

    // Create the C expression
    std::vector<cl::Buffer> inputBuffers;
    inputBuffers.reserve( inputRefs.size() );
    QStringList inputArgs;
    for ( const auto &ref : inputRefs )
    {
      cExpression.replace( QStringLiteral( "\"%1\"" ).arg( ref.name ), QStringLiteral( "%1[i]" ).arg( ref.varName ) );
      inputArgs.append( QStringLiteral( "__global %1 *%2" )
                        .arg( ref.typeName, ref.varName ) );
      inputBuffers.push_back( cl::Buffer( ctx, CL_MEM_READ_ONLY, ref.bufferSize, nullptr, nullptr ) );
    }

    //qDebug() << cExpression;

    // Create the program
    QString programTemplate( R"CL(
    // Inputs:
    ##INPUT_DESC##
    // Expression: ##EXPRESSION_ORIGINAL##
    __kernel void rasterCalculator( ##INPUT##
                              __global float *resultLine
                            )
    {
      // Get the index of the current element
      const int i = get_global_id(0);
      // Check for nodata in input
      if ( ##INPUT_NODATA_CHECK## )
        resultLine[i] = -FLT_MAX;
      // Expression
      else
        resultLine[i] = ##EXPRESSION##;
    }
    )CL" );

    QStringList inputDesc;
    QStringList inputNoDataCheck;
    for ( const auto &ref : inputRefs )
    {
      inputDesc.append( QStringLiteral( "  // %1 = %2" ).arg( ref.varName, ref.name ) );
      if ( ref.layer->dataProvider()->sourceHasNoDataValue( ref.band ) )
      {
        inputNoDataCheck.append( QStringLiteral( "(float) %1[i] == (float) %2" ).arg( ref.varName, QString::number( ref.layer->dataProvider()->sourceNoDataValue( ref.band ), 'g', 10 ) ) );
      }
    }

    programTemplate = programTemplate.replace( QLatin1String( "##INPUT_NODATA_CHECK##" ), inputNoDataCheck.isEmpty() ? QStringLiteral( "false" ) : inputNoDataCheck.join( QLatin1String( " || " ) ) );
    programTemplate = programTemplate.replace( QLatin1String( "##INPUT_DESC##" ), inputDesc.join( '\n' ) );
    programTemplate = programTemplate.replace( QLatin1String( "##INPUT##" ), !inputArgs.isEmpty() ? ( inputArgs.join( ',' ).append( ',' ) ) : QChar( ' ' ) );
    programTemplate = programTemplate.replace( QLatin1String( "##EXPRESSION##" ), cExpression );
    programTemplate = programTemplate.replace( QLatin1String( "##EXPRESSION_ORIGINAL##" ), calcNode->toString( ) );

    //qDebug() << programTemplate;

    // Create a program from the kernel source
    cl::Program program( QgsOpenClUtils::buildProgram( programTemplate, QgsOpenClUtils::ExceptionBehavior::Throw ) );

    // Create the buffers, output is float32 (4 bytes)
    // We assume size of float = 4 because that's the size used by OpenCL and IEEE 754
    Q_ASSERT( sizeof( float ) == 4 );
    std::size_t resultBufferSize( 4 * static_cast<size_t>( mNumOutputColumns ) );
    cl::Buffer resultLineBuffer( ctx, CL_MEM_WRITE_ONLY,
                                 resultBufferSize, nullptr, nullptr );

    auto kernel = cl::Kernel( program, "rasterCalculator" );

    for ( unsigned int i = 0; i < inputBuffers.size() ; i++ )
    {
      kernel.setArg( i, inputBuffers.at( i ) );
    }
    kernel.setArg( static_cast<unsigned int>( inputBuffers.size() ), resultLineBuffer );

    QgsOpenClUtils::CPLAllocator<float> resultLine( static_cast<size_t>( mNumOutputColumns ) );

    //open output dataset for writing
    GDALDriverH outputDriver = openOutputDriver();
    if ( !outputDriver )
    {
      mLastError = QObject::tr( "Could not obtain driver for %1" ).arg( mOutputFormat );
      return CreateOutputError;
    }

    gdal::dataset_unique_ptr outputDataset( openOutputFile( outputDriver ) );
    if ( !outputDataset )
    {
      mLastError = QObject::tr( "Could not create output %1" ).arg( mOutputFile );
      return CreateOutputError;
    }

    GDALSetProjection( outputDataset.get(), mOutputCrs.toWkt( QgsCoordinateReferenceSystem::WKT_PREFERRED_GDAL ).toLocal8Bit().data() );


    GDALRasterBandH outputRasterBand = GDALGetRasterBand( outputDataset.get(), 1 );
    if ( !outputRasterBand )
      return BandError;

    const float outputNodataValue = -FLT_MAX;
    GDALSetRasterNoDataValue( outputRasterBand, outputNodataValue );

    // Input block (buffer)
    std::unique_ptr<QgsRasterBlock> block;

    // Run kernel on all scanlines
    auto rowHeight = mOutputRectangle.height() / mNumOutputRows;
    for ( int line = 0; line < mNumOutputRows; line++ )
    {
      if ( feedback && feedback->isCanceled() )
      {
        break;
      }

      if ( feedback )
      {
        feedback->setProgress( 100.0 * static_cast< double >( line ) / mNumOutputRows );
      }

      // Read lines from rasters into the buffers
      for ( const auto &ref : inputRefs )
      {
        // Read one row
        QgsRectangle rect( mOutputRectangle );
        rect.setYMaximum( rect.yMaximum() - rowHeight * line );
        rect.setYMinimum( rect.yMaximum() - rowHeight );

        // TODO: check if this is too slow
        // if crs transform needed
        if ( ref.layer->crs() != mOutputCrs )
        {
          QgsRasterProjector proj;
          proj.setCrs( ref.layer->crs(), mOutputCrs, ref.layer->transformContext() );
          proj.setInput( ref.layer->dataProvider() );
          proj.setPrecision( QgsRasterProjector::Exact );
          block.reset( proj.block( ref.band, rect, mNumOutputColumns, 1 ) );
        }
        else
        {
          block.reset( ref.layer->dataProvider()->block( ref.band, rect, mNumOutputColumns, 1 ) );
        }

        //for ( int i = 0; i < mNumOutputColumns; i++ )
        //  qDebug() << "Input: " << line << i << ref.varName << " = " << block->value( 0, i );
        //qDebug() << "Writing buffer " << ref.index;

        Q_ASSERT( ref.bufferSize == static_cast<size_t>( block->data().size( ) ) );
        queue.enqueueWriteBuffer( inputBuffers[ref.index], CL_TRUE, 0,
                                  ref.bufferSize, block->bits() );

      }
      // Run the kernel
      queue.enqueueNDRangeKernel(
        kernel,
        0,
        cl::NDRange( mNumOutputColumns )
      );

      // Write the result
      queue.enqueueReadBuffer( resultLineBuffer, CL_TRUE, 0,
                               resultBufferSize, resultLine.get() );

      //for ( int i = 0; i < mNumOutputColumns; i++ )
      //  qDebug() << "Output: " << line << i << " = " << resultLine[i];

      if ( GDALRasterIO( outputRasterBand, GF_Write, 0, line, mNumOutputColumns, 1, resultLine.get(), mNumOutputColumns, 1, GDT_Float32, 0, 0 ) != CE_None )
      {
        return CreateOutputError;
      }
    }

    if ( feedback && feedback->isCanceled() )
    {
      //delete the dataset without closing (because it is faster)
      gdal::fast_delete_and_close( outputDataset, outputDriver, mOutputFile );
      return Canceled;
    }

    inputBuffers.clear();

    GDALComputeRasterStatistics( outputRasterBand, true, nullptr, nullptr, nullptr, nullptr, GdalProgressCallback, feedback );

  }
  catch ( cl::Error &e )
  {
    mLastError = e.what();
    return CreateOutputError;
  }

  return Success;
}
#endif

GDALDriverH QgsRasterCalculator::openOutputDriver()
{
  //open driver
  GDALDriverH outputDriver = GDALGetDriverByName( mOutputFormat.toLocal8Bit().data() );

  if ( !outputDriver )
  {
    return outputDriver; //return nullptr, driver does not exist
  }

  if ( !QgsGdalUtils::supportsRasterCreate( outputDriver ) )
  {
    return nullptr; //driver exist, but it does not support the create operation
  }

  return outputDriver;
}

gdal::dataset_unique_ptr QgsRasterCalculator::openOutputFile( GDALDriverH outputDriver )
{
  //open output file
  char **papszOptions = nullptr;
  gdal::dataset_unique_ptr outputDataset( GDALCreate( outputDriver, mOutputFile.toUtf8().constData(), mNumOutputColumns, mNumOutputRows, 1, GDT_Float32, papszOptions ) );
  if ( !outputDataset )
  {
    return nullptr;
  }

  //assign georef information
  double geotransform[6];
  outputGeoTransform( geotransform );
  GDALSetGeoTransform( outputDataset.get(), geotransform );

  return outputDataset;
}

void QgsRasterCalculator::outputGeoTransform( double *transform ) const
{
  transform[0] = mOutputRectangle.xMinimum();
  transform[1] = mOutputRectangle.width() / mNumOutputColumns;
  transform[2] = 0;
  transform[3] = mOutputRectangle.yMaximum();
  transform[4] = 0;
  transform[5] = -mOutputRectangle.height() / mNumOutputRows;
}

QString QgsRasterCalculator::lastError() const
{
  return mLastError;
}

QVector<QgsRasterCalculatorEntry> QgsRasterCalculatorEntry::rasterEntries()
{
  QVector<QgsRasterCalculatorEntry> availableEntries;
  const QMap<QString, QgsMapLayer *> &layers = QgsProject::instance()->mapLayers();

  auto uniqueRasterBandIdentifier = [ & ]( QgsRasterCalculatorEntry & entry ) -> bool
  {
    unsigned int i( 1 );
    entry.ref = QStringLiteral( "%1@%2" ).arg( entry.raster->name() ).arg( entry.bandNumber );
    while ( true )
    {
      bool unique( true );
      for ( const auto &ref : std::as_const( availableEntries ) )
      {
        // Safety belt
        if ( !( entry.raster && ref.raster ) )
          continue;
        // Check if is another band of the same raster
        if ( ref.raster->publicSource() == entry.raster->publicSource() )
        {
          if ( ref.bandNumber != entry.bandNumber )
          {
            continue;
          }
          else // a layer with the same data source was already added to the list
          {
            return false;
          }
        }
        // If same name but different source
        if ( ref.ref == entry.ref )
        {
          unique = false;
          entry.ref = QStringLiteral( "%1_%2@%3" ).arg( entry.raster->name() ).arg( i++ ).arg( entry.bandNumber );
        }
      }
      if ( unique )
        return true;
    }
  };

  QMap<QString, QgsMapLayer *>::const_iterator layerIt = layers.constBegin();
  for ( ; layerIt != layers.constEnd(); ++layerIt )
  {
    QgsRasterLayer *rlayer = qobject_cast<QgsRasterLayer *>( layerIt.value() );
    if ( rlayer && rlayer->dataProvider() && ( rlayer->dataProvider()->capabilities() & QgsRasterDataProvider::Size ) )
    {
      //get number of bands
      for ( int i = 0; i < rlayer->bandCount(); ++i )
      {
        QgsRasterCalculatorEntry entry;
        entry.raster = rlayer;
        entry.bandNumber = i + 1;
        if ( ! uniqueRasterBandIdentifier( entry ) )
          break;
        availableEntries.push_back( entry );
      }
    }
  }
  return availableEntries;
}
