#include <iostream>
#include <fstream>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <qgsproject.h>

//qt includes
#include <qstring.h>

class ProjectTest : public CppUnit::TestFixture 
{ 
    CPPUNIT_TEST_SUITE( ProjectTest );

    CPPUNIT_TEST( testFileName );
    CPPUNIT_TEST( testTitle );
    CPPUNIT_TEST( testMapUnits );
    CPPUNIT_TEST( testDirtyFlag );

    CPPUNIT_TEST_SUITE_END();

  public: 

    /** 
        Setup the common test members, etc
    */
    void setUp()
    {
        file = "test.project";
        title = "test title";
    }
    

    void testFileName()
    {
        QgsProject::instance()->filename( file );

        CPPUNIT_ASSERT( file == QgsProject::instance()->filename() );
    }
    

    void testTitle()
    {
        QgsProject::instance()->title( title );

        CPPUNIT_ASSERT( title == QgsProject::instance()->title() );
    }

    void testMapUnits()
    {
        QgsProject::instance()->mapUnits( QgsScaleCalculator::METERS );
        CPPUNIT_ASSERT( QgsScaleCalculator::METERS == QgsProject::instance()->mapUnits() );

        QgsProject::instance()->mapUnits( QgsScaleCalculator::FEET );
        CPPUNIT_ASSERT( QgsScaleCalculator::FEET == QgsProject::instance()->mapUnits() );

        QgsProject::instance()->mapUnits( QgsScaleCalculator::DEGREES );
        CPPUNIT_ASSERT( QgsScaleCalculator::DEGREES == QgsProject::instance()->mapUnits() );
    }

    void testDirtyFlag()
    {
        QgsProject::instance()->dirty( true );
        CPPUNIT_ASSERT( QgsProject::instance()->dirty() );

        QgsProject::instance()->dirty( false );
        CPPUNIT_ASSERT( ! QgsProject::instance()->dirty() );
    }
    
private:

    /// file name for project file
    QString file;

    /// test project title
    QString title;

}; // class ProjectTest

