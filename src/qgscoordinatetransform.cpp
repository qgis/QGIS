#include "qgscoordinatetransform.h"

QgsCoordinateTransform::QgsCoordinateTransform( QString theSourceWKT, QString theDestWKT )
{
  mSourceWKT = theSourceWKT;
  mDestWKT = theDestWKT;
  initialise();
}

QgsCoordinateTransform::~QgsCoordinateTransform()
{
  delete mSourceToDestXForm;
}

void QgsCoordinateTransform::setSourceWKT(QString theWKT)
{
  mSourceWKT = theWKT;
  initialise();
}
void QgsCoordinateTransform::setDestWKT(QString theWKT)
{
  mDestWKT = theWKT;
  initialise();
}

void QgsCoordinateTransform::initialise()
{
 mInitialisedFlag=false; //guilty until proven innocent...
  //default to geo / wgs84 for now .... later we will make this user configurable
  QString myGeoWKT =    "GEOGCS[\"WGS 84\", "
    "  DATUM[\"WGS_1984\", "
    "    SPHEROID[\"WGS 84\",6378137,298.257223563, "
    "      AUTHORITY[\"EPSG\",7030]], "
    "    TOWGS84[0,0,0,0,0,0,0], "
    "    AUTHORITY[\"EPSG\",6326]], "
    "  PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",8901]], "
    "  UNIT[\"DMSH\",0.0174532925199433,AUTHORITY[\"EPSG\",9108]], "
    "  AXIS[\"Lat\",NORTH], "
    "  AXIS[\"Long\",EAST], "
    "  AUTHORITY[\"EPSG\",4326]]";
  //default input projection to geo wgs84  
  if (mSourceWKT.isEmpty())
  {
   mSourceWKT = myGeoWKT;
  }
  //but default output projection to be the same as input proj
  //whatever that may be...
  if (mDestWKT.isEmpty())
  {
    mDestWKT = mSourceWKT;
  }  
  
  if (mSourceWKT == mDestWKT)
  {
    mShortCircuit=true;
    return;
  }
  else
  {
    mShortCircuit=false;
  }
  
  OGRSpatialReference myInputSpatialRefSys, myOutputSpatialRefSys;
  //this is really ugly but we need to get a QString to a char**
  const char * mySourceCharArray =  mSourceWKT.ascii();
  char *mySourceCharArrayPointer = (char *)mySourceCharArray;
  const char * myDestCharArray =  mDestWKT.ascii();
  char *myDestCharArrayPointer = (char *)myDestCharArray;
  
  /* Here are the possible OGR error codes :
     typedef int OGRErr;

    #define OGRERR_NONE                0
    #define OGRERR_NOT_ENOUGH_DATA     1    --> not enough data to deserialize 
    #define OGRERR_NOT_ENOUGH_MEMORY   2
    #define OGRERR_UNSUPPORTED_GEOMETRY_TYPE 3
    #define OGRERR_UNSUPPORTED_OPERATION 4
    #define OGRERR_CORRUPT_DATA        5
    #define OGRERR_FAILURE             6
    #define OGRERR_UNSUPPORTED_SRS     7 */
    
  OGRErr myInputResult = myInputSpatialRefSys.importFromWkt( & mySourceCharArrayPointer );
  if (myInputResult != OGRERR_NONE)
  {
    std::cout << "The source projection for this layer could not be set " << std::endl;
    std::cout << "INPUT: " << mSourceWKT << std::endl;
    return;
  }

  OGRErr myOutputResult = myOutputSpatialRefSys.importFromWkt( & myDestCharArrayPointer );
  if (myOutputResult != OGRERR_NONE)
  {
    std::cout << "The dest projection for this layer could not be set " << std::endl;
    std::cout << "OUTPUT: " << mDestWKT << std::endl;
    return;
  }  
  
  mSourceToDestXForm = OGRCreateCoordinateTransformation( &myInputSpatialRefSys, &myOutputSpatialRefSys );

  if ( ! mSourceToDestXForm )
  {
    std::cout << "The OGR Coordinate transformation for this layer could not be set " << std::endl;
    std::cout << "INPUT: " << mSourceWKT << std::endl;
    std::cout << "OUTPUT: " << mDestWKT  << std::endl;
    return;
  }
  else
  {
    mInitialisedFlag = true;
  }
  // Deactivate GDAL error messages.
  //CPLSetErrorHandler( errorHandler );

  // Guess if the source o dest CS is in degrees.
  //Searchf for this phrase in each wkt:  "unit[\"degree\"" 
}
