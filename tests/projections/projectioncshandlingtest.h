#include <iostream>
#include <fstream>
#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestCaller.h>
//
//qt includes
#include <qstring.h>
//gdal and ogr includes
#include <ogr_api.h>
#include <ogr_spatialref.h>
#include <cpl_error.h>
class ProjectionCsHandlingTest : public CppUnit::TestCase { 
  public: 
    ProjectionCsHandlingTest(){}
    
    ProjectionCsHandlingTest( std::string name ) : CppUnit::TestCase( name ) { }

    static CppUnit::Test *suite()
    {
      CppUnit::TestSuite *suiteOfTests = new CppUnit::TestSuite( "ProjectionCsHandlingTest" );
      suiteOfTests->addTest( new CppUnit::TestCaller<ProjectionCsHandlingTest>( 
            "testProjImportWkt", 
            &ProjectionCsHandlingTest::testProjImportWkt ) );
      suiteOfTests->addTest( new CppUnit::TestCaller<ProjectionCsHandlingTest>( 
            "testProjExportToProj4", 
            &ProjectionCsHandlingTest::testProjExportToProj4 ) );
      suiteOfTests->addTest( new CppUnit::TestCaller<ProjectionCsHandlingTest>( 
            "testProjNad27ExportToProj4", 
            &ProjectionCsHandlingTest::testProjNad27ExportToProj4 ) );
      suiteOfTests->addTest( new CppUnit::TestCaller<ProjectionCsHandlingTest>( 
            "testProjNad83ExportToProj4", 
            &ProjectionCsHandlingTest::testProjNad83ExportToProj4 ) );
      suiteOfTests->addTest( new CppUnit::TestCaller<ProjectionCsHandlingTest>( 
            "testFetchWktAttributes",
            &ProjectionCsHandlingTest::testFetchWktAttributes ) );
      suiteOfTests->addTest( new CppUnit::TestCaller<ProjectionCsHandlingTest>( 
            "testProjEpsgExportToProj4",
            &ProjectionCsHandlingTest::testProjEpsgExportToProj4 ) );
      suiteOfTests->addTest( new CppUnit::TestCaller<ProjectionCsHandlingTest>( 
            "testAkAlbersExportToProj4NoMorph",
            &ProjectionCsHandlingTest::testAkAlbersExportToProj4NoMorph ) );
      suiteOfTests->addTest( new CppUnit::TestCaller<ProjectionCsHandlingTest>( 
            "testAkAlbersExportToProj4Morph",
            &ProjectionCsHandlingTest::testAkAlbersExportToProj4Morph ) );
      suiteOfTests->addTest( new CppUnit::TestCaller<ProjectionCsHandlingTest>( 
            "testWktFromFile",
            &ProjectionCsHandlingTest::testWktFromFile ) );
      return suiteOfTests;
    }  
    // 
    // Setup the common test members, etc
    //
    void setUp()
    {
      // wkt for creating a spatial refernence system
      wkt =   "GEOGCS[\"WGS 84\", "
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
      wktDest = "GEOGCS[\"GCS_North_American_1927\",DATUM[\"D_North_American_1927\",SPHEROID[\"Clarke_1866\",6378206.4,294.9786982]],PRIMEM[\"Greenwich\",0],UNIT[\"Degree\",0.017453292519943295]]";
      wktDestNad83 = "GEOGCS[\"GCS_North_American_1983\",DATUM[\"D_North_American_1983\",SPHEROID[\"GRS_1980\",6378137,298.257222101]],PRIMEM[\"Greenwich\",0],UNIT[\"Degree\",0.0174532925199433]]";
wktEpsg = "GEOGCS[\"NAD27\",DATUM[\"North_American_Datum_1927\",SPHEROID[\"Clarke 1866\",6378206.4,294.978698213901]],PRIMEM[\"Greenwich\",0],UNIT[\"degree\",0.0174532925199433]]";

wktAkAlbers = "PROJCS[\"Alaska_Albers_Equal_Area_Conic\",GEOGCS[\"GCS_North_American_1927\",DATUM[\"D_North_American_1927\",SPHEROID[\"Clarke_1866\",6378206.4,294.9786982]],PRIMEM[\"Greenwich\",0.0],UNIT[\"Degree\",0.0174532925199433]],PROJECTION[\"Albers\"],PARAMETER[\"False_Easting\",0.0],PARAMETER[\"False_Northing\",0.0],PARAMETER[\"Central_Meridian\",-154.0],PARAMETER[\"Standard_Parallel_1\",55.0],PARAMETER[\"Standard_Parallel_2\",65.0],PARAMETER[\"Latitude_Of_Origin\",50.0],UNIT[\"Meter\",1.0]]";
    }
    
    //
    // Test creation of a OGRSpatialReference object from wkt
    //
    void testProjImportWkt() {
      // create a spatial reference system object
      std::cout << "\n\nCreating and OGRSpatialReference object from WKT" << std::endl; 
      OGRSpatialReference myInputSpatialRefSys;
      char *pWkt = (char*)wkt.ascii();
      CPPUNIT_ASSERT(myInputSpatialRefSys.importFromWkt(&pWkt)== OGRERR_NONE);
    }
    //
    // Test fetch of proj4 parameters from an OGRSpatialReference object
    // Failure occurs if datum field is not found in the proj4 parameter string
    //
    void testProjExportToProj4()
    {
      std::cout << "\n\nGetting proj4 parameters from OGRSpatialReference object" << std::endl; 
      // set up the spatial ref
      OGRSpatialReference myInputSpatialRefSys;
      char *pWkt = (char*)wkt.ascii();
      CPPUNIT_ASSERT(myInputSpatialRefSys.importFromWkt(&pWkt)== OGRERR_NONE);
      // get the proj4 for the projection
      char *proj4src;
      CPPUNIT_ASSERT(myInputSpatialRefSys.exportToProj4(&proj4src) == OGRERR_NONE);
      std::cout << "\tPROJ4: " << proj4src << std::endl;  
      CPPUNIT_ASSERT(QString(proj4src).find("datum") > -1);

    }
    //
    // Test fetch of proj4 parameters from GEOGCS NAD27 OGRSpatialReference object
    // Failure occurs if datum field is not found in the proj4 parameter string
    //
    void testProjNad27ExportToProj4()
    {
      std::cout << "\n\nGetting NAD27 proj4 parameters from OGRSpatialReference object" << std::endl; 
      // set up the spatial ref
      OGRSpatialReference myInputSpatialRefSys;
      char *pWkt = (char*)wktDest.ascii();
      CPPUNIT_ASSERT(myInputSpatialRefSys.importFromWkt(&pWkt)== OGRERR_NONE);
      // get the proj4 for the projection
      char *proj4src;
      CPPUNIT_ASSERT(myInputSpatialRefSys.exportToProj4(&proj4src) == OGRERR_NONE);
      std::cout << "\tPROJ4: " << proj4src << std::endl;  
      CPPUNIT_ASSERT(QString(proj4src).find("datum") > -1);

    }
    //
    // Test fetch of proj4 parameters from GEOGCS NAD83 OGRSpatialReference object
    // Failure occurs if datum field is not found in the proj4 parameter string
    //
    void testProjNad83ExportToProj4()
    {
      std::cout << "\n\nGetting NAD83 proj4 parameters used in states.shp" << std::endl; 
      // set up the spatial ref
      OGRSpatialReference myInputSpatialRefSys;
      char *pWkt = (char*)wktDestNad83.ascii();
      CPPUNIT_ASSERT(myInputSpatialRefSys.importFromWkt(&pWkt)== OGRERR_NONE);
      // morph it from esri
      std::cout << "\tMorphing from ESRI to standard form before getting proj4 parameters" << std::endl; 
      CPPUNIT_ASSERT(myInputSpatialRefSys.morphFromESRI() == OGRERR_NONE);
      // get the proj4 for the projection
      char *proj4src;
      CPPUNIT_ASSERT(myInputSpatialRefSys.exportToProj4(&proj4src) == OGRERR_NONE);
      std::cout << "\tPROJ4: " << proj4src << std::endl;  
      CPPUNIT_ASSERT(QString(proj4src).find("datum") > -1);
    }
    //
    // Test fetch of proj4 parameters from GEOGCS NAD83 OGRSpatialReference object
    // in EPSG format
    // Failure occurs if datum field is not found in the proj4 parameter string
    //
    void testProjEpsgExportToProj4()
    {
      std::cout << "\n\nGetting NAD83 proj4 parameters from an EPSG format WKT" << std::endl; 
      // set up the spatial ref
      OGRSpatialReference myInputSpatialRefSys;
      char *pWkt = (char*)wktEpsg.ascii();
      CPPUNIT_ASSERT(myInputSpatialRefSys.importFromWkt(&pWkt)== OGRERR_NONE);
      // get the proj4 for the projection
      char *proj4src;
      CPPUNIT_ASSERT(myInputSpatialRefSys.exportToProj4(&proj4src) == OGRERR_NONE);
      std::cout << "\tPROJ4: " << proj4src << std::endl;  
      CPPUNIT_ASSERT(QString(proj4src).find("datum") > -1);
    }
    //
    // Test fetch of proj4 parameters from Alaska Albers  OGRSpatialReference object
    // without morph to ESRI form
    // Failure occurs if datum field is not found in the proj4 parameter string
    //
    void testAkAlbersExportToProj4NoMorph()
    {
      std::cout << "\n\nGetting Alaska Albers proj4 parameters from kodiak.prj" << std::endl; 
      // set up the spatial ref
      OGRSpatialReference myInputSpatialRefSys;
      char *pWkt = (char*)wktAkAlbers.ascii();
      CPPUNIT_ASSERT(myInputSpatialRefSys.importFromWkt(&pWkt)== OGRERR_NONE);
      std::cout << "\tGetting proj4 paramters without morph to ESRI form" << std::endl; 
      // get the proj4 for the unmorphed projection
      char *proj4src;
      CPPUNIT_ASSERT(myInputSpatialRefSys.exportToProj4(&proj4src) == OGRERR_NONE);
      std::cout << "\tPROJ4: " << proj4src << std::endl;  
      CPPUNIT_ASSERT(QString(proj4src).find("datum") > -1);
    }
    //
    // Test fetch of proj4 parameters from Alaska Albers  OGRSpatialReference object
    // with morph to ESRI form
    // Failure occurs if datum field is not found in the proj4 parameter string
    //
    void testAkAlbersExportToProj4Morph()
    {
      std::cout << "\n\nGetting Alaska Albers proj4 parameters from kodiak.prj" << std::endl; 
      // set up the spatial ref
      OGRSpatialReference myInputSpatialRefSys;
      char *pWkt = (char*)wktAkAlbers.ascii();
      CPPUNIT_ASSERT(myInputSpatialRefSys.importFromWkt(&pWkt)== OGRERR_NONE);
      std::cout << "\tGetting proj4 paramters with morph to ESRI form" << std::endl; 
      CPPUNIT_ASSERT(myInputSpatialRefSys.morphFromESRI() == OGRERR_NONE);
      // get the proj4 for the unmorphed projection
      char *proj4src;
      CPPUNIT_ASSERT(myInputSpatialRefSys.exportToProj4(&proj4src) == OGRERR_NONE);
      std::cout << "\tPROJ4: " << proj4src << std::endl;  
      CPPUNIT_ASSERT(QString(proj4src).find("datum") > -1);
    }
    //
    // Test fetching of wkt properties from a spatial ref object
    //
    void testFetchWktAttributes()
    {
      std::cout << "\n\nFetching states.prj WKT attributes using OGRSpatialReference::GetAttrValue" << std::endl; 
      // set up the spatial ref - use the nad83 from states.prj
      OGRSpatialReference myInputSpatialRefSys;
      char *pWkt = (char*)wktDestNad83.ascii();
      CPPUNIT_ASSERT(myInputSpatialRefSys.importFromWkt(&pWkt)== OGRERR_NONE);
      // test access to the datum
      const char *datum = myInputSpatialRefSys.GetAttrValue("DATUM", 0);
      CPPUNIT_ASSERT(datum != 0);
      std::cout << "\tDatum: " << datum << std::endl; 
    }
    //
    // Test the WKT contained in wkt.txt in the current directory to see if
    // the datum can be determined
    //
    void testWktFromFile()
    {
     std::ifstream wktIn("./wkt.txt");
    char *buf = new char[16384];
    wktIn.getline(buf, 16384);
    wktIn.close();
      std::cout << "\n\nGetting proj4 parameters from wkt.txt" << std::endl; 
      // set up the spatial ref
      OGRSpatialReference myInputSpatialRefSys;
      CPPUNIT_ASSERT(myInputSpatialRefSys.importFromWkt(&buf)== OGRERR_NONE);
//      std::cout << "\tGetting proj4 paramters with morph to ESRI form" << std::endl; 
//      CPPUNIT_ASSERT(myInputSpatialRefSys.morphFromESRI() == OGRERR_NONE);
      // get the proj4 for the unmorphed projection
      char *proj4src;
      CPPUNIT_ASSERT(myInputSpatialRefSys.exportToProj4(&proj4src) == OGRERR_NONE);
      std::cout << "\tPROJ4: " << proj4src << std::endl;  
      CPPUNIT_ASSERT(QString(proj4src).find("datum") > -1);
      
    }

    
  private:
    // WKT for default projection hardcoded in QgsCoordinateTransform class
    QString wkt;
    // WKT for an ESRI style GEOGCS in NAD27
    QString wktDest;
    // WKT for an ESRI style GEOGCS in NAD83 (from states.shp shapefile)
    QString wktDestNad83;
    // WKT for an EPSG style GEOGCS
    QString wktEpsg;
    // WKT for an ESRI style PROJCS as read from a shapefile
    QString wktAkAlbers;
};

