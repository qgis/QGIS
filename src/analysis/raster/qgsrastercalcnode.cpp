#include "qgsrastercalcnode.h"

QgsRasterCalcNode::QgsRasterCalcNode(): mLeft( 0 ), mRight( 0 ), mRasterMatrix( 0 ), mNumber( 0 )
{
}

QgsRasterCalcNode::QgsRasterCalcNode( double number ): mType( tNumber ), mLeft( 0 ), mRight( 0 ), mRasterMatrix( 0 ), mNumber( number )
{
}

QgsRasterCalcNode::QgsRasterCalcNode( Operator op, QgsRasterCalcNode* left, QgsRasterCalcNode* right ): mType( tOperator ), mLeft( left ), mRight( right ), mRasterMatrix( 0 ), mNumber( 0 ), mOperator( op )
{
}

QgsRasterCalcNode::QgsRasterCalcNode( const QString& rasterName ): mType( tRasterRef ), mLeft( 0 ), mRight( 0 ), mRasterMatrix( 0 ), mNumber( 0 ), mRasterName( rasterName )
{
}

QgsRasterCalcNode::~QgsRasterCalcNode()
{
  if( mLeft )
  {
    delete mLeft;
  }
  if( mRight )
  {
    delete mRight;
  }
}

bool QgsRasterCalcNode::calculate( QMap<QString, QgsRasterMatrix*>& rasterData, QgsRasterMatrix& result ) const
{
  //if type is raster ref: return a copy of the corresponding matrix

  //if type is operator, call the proper matrix operations
  if( mType == tRasterRef )
  {
    QMap<QString, QgsRasterMatrix*>::iterator it = rasterData.find( mRasterName );
    if( it == rasterData.end() )
    {
      return false;
    }

    int nEntries = ( *it )->nColumns() * ( *it )->nRows();
    float* data = new float[nEntries];
    memcpy( data, ( *it )->data(), nEntries * sizeof( float ) );
    result.setData(( *it )->nColumns(), ( *it )->nRows(), data );
    return true;
  }
  else if( mType == tOperator )
  {
    QgsRasterMatrix leftMatrix, rightMatrix;
    QgsRasterMatrix resultMatrix;
    if( !mLeft || !mLeft->calculate( rasterData, leftMatrix ) )
    {
      return false;
    }
    if( mRight && !mRight->calculate( rasterData, rightMatrix ) )
    {
      return false;
    }

    switch( mOperator )
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
      default:
        return false;
    }
    int newNColumns = leftMatrix.nColumns();
    int newNRows = leftMatrix.nRows();
    result.setData( newNColumns, newNRows, leftMatrix.takeData() );
    return true;
  }
  else if( mType == tNumber )
  {
    float* data = new float[1];
    data[0] = mNumber;
    result.setData( 1, 1, data );
    return true;
  }
  return false;
}


