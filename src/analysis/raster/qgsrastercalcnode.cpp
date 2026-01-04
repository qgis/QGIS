/***************************************************************************
    qgsrastercalcnode.cpp
    ---------------------
    begin                : October 2010
    copyright            : (C) 2010 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsrastercalcnode.h"

#include "qgsrasterblock.h"
#include "qgsrastermatrix.h"

QgsRasterCalcNode::QgsRasterCalcNode( double number )
  : mNumber( number )
{
}

QgsRasterCalcNode::QgsRasterCalcNode( QgsRasterMatrix *matrix )
  : mType( tMatrix )
  , mMatrix( matrix )
{
}

QgsRasterCalcNode::QgsRasterCalcNode( Operator op, QgsRasterCalcNode *left, QgsRasterCalcNode *right )
  : mType( tOperator )
  , mLeft( left )
  , mRight( right )
  , mOperator( op )
{
}

QgsRasterCalcNode::QgsRasterCalcNode( QString functionName, QVector<QgsRasterCalcNode *> functionArgs )
  : mType( tFunction )
  , mFunctionName( functionName )
  , mFunctionArgs( functionArgs )
{
}

QgsRasterCalcNode::QgsRasterCalcNode( const QString &rasterName )
  : mType( tRasterRef )
  , mRasterName( rasterName )
{
  if ( mRasterName.startsWith( '"' ) && mRasterName.endsWith( '"' ) )
    mRasterName = mRasterName.mid( 1, mRasterName.size() - 2 );
}

QgsRasterCalcNode::~QgsRasterCalcNode()
{
  delete mLeft;
  delete mRight;
  for ( int i = 0; i < mFunctionArgs.size(); ++i )
  {
    if ( mFunctionArgs.at( i ) )
      delete mFunctionArgs.at( i );
  }
}

bool QgsRasterCalcNode::calculate( QMap<QString, QgsRasterBlock *> &rasterData, QgsRasterMatrix &result, int row ) const
{
  //if type is raster ref: return a copy of the corresponding matrix

  //if type is operator, call the proper matrix operations
  if ( mType == tRasterRef )
  {
    const QMap<QString, QgsRasterBlock *>::iterator it = rasterData.find( mRasterName );
    if ( it == rasterData.end() )
    {
      QgsDebugError( u"Error: could not find raster data for \"%1\""_s.arg( mRasterName ) );
      return false;
    }

    const int nRows = ( row >= 0 ? 1 : ( *it )->height() );
    const int startRow = ( row >= 0 ? row : 0 );
    const int endRow = startRow + nRows;
    const int nCols = ( *it )->width();
    const int nEntries = nCols * nRows;
    double *data = new double[nEntries];

    //convert input raster values to double, also convert input no data to result no data

    int outRow = 0;
    bool isNoData = false;
    for ( int dataRow = startRow; dataRow < endRow; ++dataRow, ++outRow )
    {
      for ( int dataCol = 0; dataCol < nCols; ++dataCol )
      {
        const double value = ( *it )->valueAndNoData( dataRow, dataCol, isNoData );
        data[dataCol + nCols * outRow] = isNoData ? result.nodataValue() : value;
      }
    }
    result.setData( nCols, nRows, data, result.nodataValue() );
    return true;
  }
  else if ( mType == tOperator )
  {
    QgsRasterMatrix leftMatrix( result.nColumns(), result.nRows(), nullptr, result.nodataValue() );
    QgsRasterMatrix rightMatrix( result.nColumns(), result.nRows(), nullptr, result.nodataValue() );

    if ( !mLeft || !mLeft->calculate( rasterData, leftMatrix, row ) )
    {
      return false;
    }
    if ( mRight && !mRight->calculate( rasterData, rightMatrix, row ) )
    {
      return false;
    }

    switch ( mOperator )
    {
      case opPLUS:
        leftMatrix.add( rightMatrix );
        break;
      case opMINUS:
        leftMatrix.subtract( rightMatrix );
        break;
      case opMUL:
        leftMatrix.multiply( rightMatrix );
        break;
      case opDIV:
        leftMatrix.divide( rightMatrix );
        break;
      case opPOW:
        leftMatrix.power( rightMatrix );
        break;
      case opEQ:
        leftMatrix.equal( rightMatrix );
        break;
      case opNE:
        leftMatrix.notEqual( rightMatrix );
        break;
      case opGT:
        leftMatrix.greaterThan( rightMatrix );
        break;
      case opLT:
        leftMatrix.lesserThan( rightMatrix );
        break;
      case opGE:
        leftMatrix.greaterEqual( rightMatrix );
        break;
      case opLE:
        leftMatrix.lesserEqual( rightMatrix );
        break;
      case opAND:
        leftMatrix.logicalAnd( rightMatrix );
        break;
      case opOR:
        leftMatrix.logicalOr( rightMatrix );
        break;
      case opMIN:
        leftMatrix.min( rightMatrix );
        break;
      case opMAX:
        leftMatrix.max( rightMatrix );
        break;
      case opSQRT:
        leftMatrix.squareRoot();
        break;
      case opSIN:
        leftMatrix.sinus();
        break;
      case opCOS:
        leftMatrix.cosinus();
        break;
      case opTAN:
        leftMatrix.tangens();
        break;
      case opASIN:
        leftMatrix.asinus();
        break;
      case opACOS:
        leftMatrix.acosinus();
        break;
      case opATAN:
        leftMatrix.atangens();
        break;
      case opSIGN:
        leftMatrix.changeSign();
        break;
      case opLOG:
        leftMatrix.log();
        break;
      case opLOG10:
        leftMatrix.log10();
        break;
      case opABS:
        leftMatrix.absoluteValue();
        break;
      default:
        return false;
    }
    const int newNColumns = leftMatrix.nColumns();
    const int newNRows = leftMatrix.nRows();
    result.setData( newNColumns, newNRows, leftMatrix.takeData(), leftMatrix.nodataValue() );
    return true;
  }
  else if ( mType == tNumber )
  {
    const size_t nEntries = static_cast<size_t>( result.nColumns() * result.nRows() );
    double *data = new double[nEntries];
    std::fill( data, data + nEntries, mNumber );
    result.setData( result.nColumns(), 1, data, result.nodataValue() );

    return true;
  }
  else if ( mType == tMatrix )
  {
    const int nEntries = mMatrix->nColumns() * mMatrix->nRows();
    double *data = new double[nEntries];
    for ( int i = 0; i < nEntries; ++i )
    {
      data[i] = mMatrix->data()[i] == mMatrix->nodataValue() ? result.nodataValue() : mMatrix->data()[i];
    }
    result.setData( mMatrix->nColumns(), mMatrix->nRows(), data, result.nodataValue() );
    return true;
  }
  else if ( mType == tFunction )
  {
    std::vector<std::unique_ptr<QgsRasterMatrix>> matrixContainer;
    for ( int i = 0; i < mFunctionArgs.size(); ++i )
    {
      auto singleMatrix = std::make_unique<QgsRasterMatrix>( result.nColumns(), result.nRows(), nullptr, result.nodataValue() );
      if ( !mFunctionArgs.at( i ) || !mFunctionArgs.at( i )->calculate( rasterData, *singleMatrix, row ) )
      {
        return false;
      }
      matrixContainer.emplace_back( std::move( singleMatrix ) );
    }
    evaluateFunction( matrixContainer, result );
    return true;
  }
  return false;
}

QString QgsRasterCalcNode::toString( bool cStyle ) const
{
  QString result;
  QString left;
  QString right;
  if ( mLeft )
    left = mLeft->toString( cStyle );
  if ( mRight )
    right = mRight->toString( cStyle );

  switch ( mType )
  {
    case tOperator:
      switch ( mOperator )
      {
        case opPLUS:
          result = u"( %1 + %2 )"_s.arg( left ).arg( right );
          break;
        case opMINUS:
          result = u"( %1 - %2 )"_s.arg( left ).arg( right );
          break;
        case opSIGN:
          result = u"-%1"_s.arg( left );
          break;
        case opMUL:
          result = u"%1 * %2"_s.arg( left ).arg( right );
          break;
        case opDIV:
          result = u"%1 / %2"_s.arg( left ).arg( right );
          break;
        case opPOW:
          if ( cStyle )
            result = u"pow( %1, %2 )"_s.arg( left ).arg( right );
          else
            result = u"%1^%2"_s.arg( left ).arg( right );
          break;
        case opEQ:
          if ( cStyle )
            result = u"( float ) ( %1 == %2 )"_s.arg( left ).arg( right );
          else
            result = u"%1 = %2"_s.arg( left ).arg( right );
          break;
        case opNE:
          if ( cStyle )
            result = u"( float ) ( %1 != %2 )"_s.arg( left ).arg( right );
          else
            result = u"%1 != %2"_s.arg( left ).arg( right );
          break;
        case opGT:
          if ( cStyle )
            result = u"( float ) ( %1 > %2 )"_s.arg( left ).arg( right );
          else
            result = u"%1 > %2"_s.arg( left ).arg( right );
          break;
        case opLT:
          if ( cStyle )
            result = u"( float ) ( %1 < %2 )"_s.arg( left ).arg( right );
          else
            result = u"%1 < %2"_s.arg( left ).arg( right );
          break;
        case opGE:
          if ( cStyle )
            result = u"( float ) ( %1 >= %2 )"_s.arg( left ).arg( right );
          else
            result = u"%1 >= %2"_s.arg( left ).arg( right );
          break;
        case opLE:
          if ( cStyle )
            result = u"( float ) ( %1 <= %2 )"_s.arg( left ).arg( right );
          else
            result = u"%1 <= %2"_s.arg( left ).arg( right );
          break;
        case opAND:
          if ( cStyle )
            result = u"( float ) ( %1 && %2 )"_s.arg( left ).arg( right );
          else
            result = u"%1 AND %2"_s.arg( left ).arg( right );
          break;
        case opOR:
          if ( cStyle )
            result = u"( float ) ( %1 || %2 )"_s.arg( left ).arg( right );
          else
            result = u"%1 OR %2"_s.arg( left ).arg( right );
          break;
        case opSQRT:
          result = u"sqrt( %1 )"_s.arg( left );
          break;
        case opSIN:
          result = u"sin( %1 )"_s.arg( left );
          break;
        case opCOS:
          result = u"cos( %1 )"_s.arg( left );
          break;
        case opTAN:
          result = u"tan( %1 )"_s.arg( left );
          break;
        case opASIN:
          result = u"asin( %1 )"_s.arg( left );
          break;
        case opACOS:
          result = u"acos( %1 )"_s.arg( left );
          break;
        case opATAN:
          result = u"atan( %1 )"_s.arg( left );
          break;
        case opLOG:
          result = u"log( %1 )"_s.arg( left );
          break;
        case opLOG10:
          result = u"log10( %1 )"_s.arg( left );
          break;
        case opABS:
          if ( cStyle )
            result = u"fabs( %1 )"_s.arg( left );
          else
            // Call the floating point version
            result = u"abs( %1 )"_s.arg( left );
          break;
        case opMIN:
          if ( cStyle )
            result = u"min( ( float ) ( %1 ), ( float ) ( %2 ) )"_s.arg( left ).arg( right );
          else
            result = u"min( %1, %2 )"_s.arg( left ).arg( right );
          break;
        case opMAX:
          if ( cStyle )
            result = u"max( ( float ) ( %1 ), ( float ) ( %2 ) )"_s.arg( left ).arg( right );
          else
            result = u"max( %1, %2 )"_s.arg( left ).arg( right );
          break;
        case opNONE:
          break;
      }
      break;
    case tRasterRef:
      if ( cStyle )
        result = u"( float ) \"%1\""_s.arg( mRasterName );
      else
        result = u"\"%1\""_s.arg( mRasterName );
      break;
    case tNumber:
      result = QString::number( mNumber );
      if ( cStyle )
      {
        result = u"( float ) %1"_s.arg( result );
      }
      break;
    case tMatrix:
      break;
    case tFunction:
      if ( mFunctionName == "if" )
      {
        const QString argOne = mFunctionArgs.at( 0 )->toString( cStyle );
        const QString argTwo = mFunctionArgs.at( 1 )->toString( cStyle );
        const QString argThree = mFunctionArgs.at( 2 )->toString( cStyle );
        if ( cStyle )
          result = u" ( %1 ) ? ( %2 ) : ( %3 ) "_s.arg( argOne, argTwo, argThree );
        else
          result = u"if( %1 , %2 , %3 )"_s.arg( argOne, argTwo, argThree );
      }
      break;
  }
  return result;
}

QList<const QgsRasterCalcNode *> QgsRasterCalcNode::findNodes( const QgsRasterCalcNode::Type type ) const
{
  QList<const QgsRasterCalcNode *> nodeList;
  if ( mType == type )
    nodeList.push_back( this );
  if ( mLeft )
    nodeList.append( mLeft->findNodes( type ) );
  if ( mRight )
    nodeList.append( mRight->findNodes( type ) );

  for ( QgsRasterCalcNode *node : mFunctionArgs )
    nodeList.append( node->findNodes( type ) );

  return nodeList;
}

QgsRasterCalcNode *QgsRasterCalcNode::parseRasterCalcString( const QString &str, QString &parserErrorMsg )
{
  extern QgsRasterCalcNode *localParseRasterCalcString( const QString &str, QString &parserErrorMsg );
  return localParseRasterCalcString( str, parserErrorMsg );
}

QStringList QgsRasterCalcNode::referencedLayerNames() const
{
  QStringList referencedRasters;

  QStringList rasterRef = this->cleanRasterReferences();
  for ( const auto &i : rasterRef )
  {
    if ( referencedRasters.contains( i.mid( 0, i.lastIndexOf( "@" ) ) ) )
      continue;
    referencedRasters << i.mid( 0, i.lastIndexOf( "@" ) );
  }

  return referencedRasters;
}

QStringList QgsRasterCalcNode::cleanRasterReferences() const
{
  QStringList rasterReferences;
  const QList<const QgsRasterCalcNode *> rasterRefNodes = this->findNodes( QgsRasterCalcNode::Type::tRasterRef );

  for ( const QgsRasterCalcNode *r : rasterRefNodes )
  {
    QString layerRef( r->toString() );
    if ( layerRef.at( 0 ) == "\""_L1 && layerRef.at( layerRef.size() - 1 ) == "\""_L1 )
    {
      layerRef.remove( 0, 1 );
      layerRef.chop( 1 );
    }
    layerRef.remove( QChar( '\\' ), Qt::CaseInsensitive );
    rasterReferences << layerRef;
  }

  return rasterReferences;
}

QgsRasterMatrix QgsRasterCalcNode::evaluateFunction( const std::vector<std::unique_ptr<QgsRasterMatrix>> &matrixVector, QgsRasterMatrix &result ) const
{
  if ( mFunctionName == "if" )
  {
    //scalar condition
    if ( matrixVector.at( 0 )->isNumber() )
    {
      result = ( matrixVector.at( 0 )->data() ? *matrixVector.at( 1 ) : *matrixVector.at( 2 ) );
      return result;
    }
    int nCols = matrixVector.at( 0 )->nColumns();
    int nRows = matrixVector.at( 0 )->nRows();
    int nEntries = nCols * nRows;
    auto dataResult = std::make_unique<double[]>( nEntries );
    double *dataResultRawPtr = dataResult.get();

    double *condition = matrixVector.at( 0 )->data();
    double *firstOption = matrixVector.at( 1 )->data();
    double *secondOption = matrixVector.at( 2 )->data();

    bool isFirstOptionNumber = matrixVector.at( 1 )->isNumber();
    bool isSecondCOptionNumber = matrixVector.at( 2 )->isNumber();
    double noDataValueCondition = matrixVector.at( 0 )->nodataValue();

    for ( int i = 0; i < nEntries; ++i )
    {
      if ( condition[i] == noDataValueCondition )
      {
        dataResultRawPtr[i] = result.nodataValue();
        continue;
      }
      else if ( condition[i] != 0 )
      {
        dataResultRawPtr[i] = isFirstOptionNumber ? firstOption[0] : firstOption[i];
        continue;
      }
      dataResultRawPtr[i] = isSecondCOptionNumber ? secondOption[0] : secondOption[i];
    }

    result.setData( nCols, nRows, dataResult.release(), result.nodataValue() );
  }
  return result;
}
