/***************************************************************************
     projecttest.h
     --------------------------------------
    Date                 : Sun Sep 16 12:20:49 AKDT 2007
    Copyright            : (C) 2007 by Gary E. Sherman
    Email                : sherman at mrcc dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/**

 @file projecttst.h

*/
#include <iostream>

using namespace std;

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <qgsproject.h>
#include <qgis.h>
#include <qgsexception.h>

#include <qstring.h>



/**
   tests for QgsProject

   @todo XXX add tests for:

   read()
   write()

   Although the last two may be difficult to test for since qgis won't be running.

*/
class ProjectTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE( ProjectTest );

    CPPUNIT_TEST( testFileName );
    CPPUNIT_TEST( testTitle );
    CPPUNIT_TEST( testMapUnits );
    CPPUNIT_TEST( testDirtyFlag );
    CPPUNIT_TEST( readNullEntries );
    CPPUNIT_TEST( testWriteEntries );
    CPPUNIT_TEST( testXML );
    CPPUNIT_TEST( testRemoveEntry );
    CPPUNIT_TEST( testClearProperties );
    CPPUNIT_TEST( testEntryList );
    CPPUNIT_TEST( testSubkeyList );

    CPPUNIT_TEST_SUITE_END();

  public:

    /**
        Setup the common test members, etc
    */
    void setUp()
    {
      mFile = "test.project";
      mTitle = "test title";
      mScope = "project_test";

      mNumValueKey = "/values/myNum";

      mDoubleValueKey = "/values/myDouble";

      mBoolValueKey = "/values/myBool";

      mStringValueKey = "/values/very/nested/myString";

      mStringListValueKey = "/values/myStrings/myStringlist";


      mNumValueConst = 42;

      mDoubleValueConst = 12345.6789;

      mBoolValueConst = true;

      mStringValueConst = "Test String";

      mStringListValueConst += "first";
      mStringListValueConst += "second";
      mStringListValueConst += "third";
    } // setUp



    void testFileName()
    {
      QgsProject::instance()->dirty( false );
      QgsProject::instance()->setFileName( mFile );

      CPPUNIT_ASSERT( mFile == QgsProject::instance()->fileName() );
      CPPUNIT_ASSERT( QgsProject::instance()->isDirty() );
    } // testFileName



    void testTitle()
    {
      QgsProject::instance()->dirty( false );
      QgsProject::instance()->title( mTitle );

      CPPUNIT_ASSERT( mTitle == QgsProject::instance()->title() );
      CPPUNIT_ASSERT( QgsProject::instance()->isDirty() );
    } // testTitle


    void testMapUnits()
    {
      QgsProject::instance()->dirty( false );
      QgsProject::instance()->mapUnits( QGis::Meters );
      CPPUNIT_ASSERT( QGis::Meters == QgsProject::instance()->mapUnits() );
      CPPUNIT_ASSERT( QgsProject::instance()->isDirty() );

      QgsProject::instance()->dirty( false );
      QgsProject::instance()->mapUnits( QGis::Feet );
      CPPUNIT_ASSERT( QGis::Feet == QgsProject::instance()->mapUnits() );
      CPPUNIT_ASSERT( QgsProject::instance()->isDirty() );

      QgsProject::instance()->dirty( false );
      QgsProject::instance()->mapUnits( QGis::Degrees );
      CPPUNIT_ASSERT( QGis::Degrees == QgsProject::instance()->mapUnits() );
      CPPUNIT_ASSERT( QgsProject::instance()->isDirty() );
    } // testMapUnits



    void testDirtyFlag()
    {
      QgsProject::instance()->dirty( true );
      CPPUNIT_ASSERT( QgsProject::instance()->isDirty() );

      QgsProject::instance()->dirty( false );
      CPPUNIT_ASSERT( ! QgsProject::instance()->isDirty() );
    } // testDirtyFlag


    /**
       Reading entries that are known not to exist should fail and use default
       values.
     */
    void readNullEntries()
    {
      bool status;

      bool b = QgsProject::instance()->readBoolEntry( mScope, mBoolValueKey, false, &status );
      CPPUNIT_ASSERT( false == b && ! status );

      int i = QgsProject::instance()->readNumEntry( mScope, mNumValueKey, 13, &status );
      CPPUNIT_ASSERT( 13 == i && ! status );

      double d = QgsProject::instance()->readDoubleEntry( mScope, mDoubleValueKey, 99.0, &status );
      CPPUNIT_ASSERT( 99.0 == d && ! status );

      QString s = QgsProject::instance()->readEntry( mScope, mStringValueKey, "FOO", &status );
      CPPUNIT_ASSERT( "FOO" == s && ! status );

      QStringList sl = QgsProject::instance()->readListEntry( mScope, mStringListValueKey, &status );
      CPPUNIT_ASSERT( sl.empty() && ! status );

    } // readNullEntries


    /** check that writing entries works */
    void testWriteEntries()
    {
      QgsProject::instance()->dirty( false );
      CPPUNIT_ASSERT( QgsProject::instance()->writeEntry( mScope, mBoolValueKey, mBoolValueConst ) );
      CPPUNIT_ASSERT( QgsProject::instance()->isDirty() );

      QgsProject::instance()->dirty( false );
      CPPUNIT_ASSERT( QgsProject::instance()->writeEntry( mScope, mNumValueKey, mNumValueConst ) );
      CPPUNIT_ASSERT( QgsProject::instance()->isDirty() );

      QgsProject::instance()->dirty( false );
      CPPUNIT_ASSERT( QgsProject::instance()->writeEntry( mScope, mDoubleValueKey, mDoubleValueConst ) );
      CPPUNIT_ASSERT( QgsProject::instance()->isDirty() );

      QgsProject::instance()->dirty( false );
      CPPUNIT_ASSERT( QgsProject::instance()->writeEntry( mScope, mStringValueKey, mStringValueConst ) );
      CPPUNIT_ASSERT( QgsProject::instance()->isDirty() );

      QgsProject::instance()->dirty( false );
      CPPUNIT_ASSERT( QgsProject::instance()->writeEntry( mScope, mStringListValueKey, mStringListValueConst ) );
      CPPUNIT_ASSERT( QgsProject::instance()->isDirty() );


      bool status;

      int i = QgsProject::instance()->readNumEntry( mScope, mNumValueKey, 13, &status );
      CPPUNIT_ASSERT( mNumValueConst == i && status );

      bool b = QgsProject::instance()->readBoolEntry( mScope, mBoolValueKey, false, &status );
      CPPUNIT_ASSERT( mBoolValueConst == b && status );

      double d = QgsProject::instance()->readDoubleEntry( mScope, mDoubleValueKey, 99.0, &status );
      CPPUNIT_ASSERT( mDoubleValueConst == d && status );

      QString s = QgsProject::instance()->readEntry( mScope, mStringValueKey, "FOO", &status );
      CPPUNIT_ASSERT( mStringValueConst == s && status );

      QStringList sl = QgsProject::instance()->readListEntry( mScope, mStringListValueKey, &status );
      CPPUNIT_ASSERT( mStringListValueConst == sl && status );

    } // testWriteEntries

    void testXML()
    {  // write out the state, clear the project, reload it, and see if we got
      // everything back
      CPPUNIT_ASSERT( QgsProject::instance()->write() );

      QgsProject::instance()->clearProperties();

      try
      {
        CPPUNIT_ASSERT( QgsProject::instance()->read() );
      }
      catch ( QgsException & e )
      {
        // since we're not running the full application, this exception is
        // expected, so we can safely ignore it

        qDebug( "%s:%d caught expected exception %s", __FILE__, __LINE__, e.what() );

      }

      bool status;

      bool b = QgsProject::instance()->readBoolEntry( mScope, mBoolValueKey, false, &status );
      CPPUNIT_ASSERT( mBoolValueConst == b && status );

      int i = QgsProject::instance()->readNumEntry( mScope, mNumValueKey, 13, &status );
      CPPUNIT_ASSERT( mNumValueConst == i && status );

      double d = QgsProject::instance()->readDoubleEntry( mScope, mDoubleValueKey, 99.0, &status );
      CPPUNIT_ASSERT( mDoubleValueConst == d && status );

      QString s = QgsProject::instance()->readEntry( mScope, mStringValueKey, "FOO", &status );
      CPPUNIT_ASSERT( mStringValueConst == s && status );

      QStringList sl = QgsProject::instance()->readListEntry( mScope, mStringListValueKey, &status );
      CPPUNIT_ASSERT( mStringListValueConst == sl && status );

//         qDebug( "%s:%d testXML after read" );

//         QgsProject::instance()->dumpProperties();
    }


    void testRemoveEntry()
    {
      // presume that testWriteEntries() already invoked so that properties are set

      CPPUNIT_ASSERT( QgsProject::instance()->removeEntry( mScope, mBoolValueKey ) );
      CPPUNIT_ASSERT( QgsProject::instance()->removeEntry( mScope, mNumValueKey ) );
      CPPUNIT_ASSERT( QgsProject::instance()->removeEntry( mScope, mDoubleValueKey ) );
      CPPUNIT_ASSERT( QgsProject::instance()->removeEntry( mScope, mStringValueKey ) );
      CPPUNIT_ASSERT( QgsProject::instance()->removeEntry( mScope, mStringListValueKey ) );

      // since we've removed everything, re-run this test to verify that
      readNullEntries();

    } // testRemoveEntry


    void testClearProperties()
    {   // rebuild the properties deleted in testRemoveEntry()
      testWriteEntries();

      // remove all in one fell swoop
      QgsProject::instance()->clearProperties();

      // since we've removed everything, re-run this test to verify that
      readNullEntries();
    } // testClearProperties


    /** test entryList()
     */
    void testEntryList()
    {
      // at first the entry list should be empty
      QStringList entries = QgsProject::instance()->entryList( mScope, "/foo" );

      CPPUNIT_ASSERT( entries.isEmpty() );

      QgsProject::instance()->writeEntry( mScope, "/foo/bar", "one" );
      QgsProject::instance()->writeEntry( mScope, "/foo/baz", "two" );
      QgsProject::instance()->writeEntry( mScope, "/foo/quux", "three" );

      QgsProject::instance()->writeEntry( mScope, "/foo/xmmy/blah", "four" );
      QgsProject::instance()->writeEntry( mScope, "/foo/xmmy/bogus", "five" );

      QgsProject::instance()->dumpProperties();

      // So entrylist for /foo should return "bar", "baz", and "quux" but
      // NOT "xmmy".  Nor should it contain any key values.

      entries = QgsProject::instance()->entryList( mScope, "/foo" );

      cerr << "entries: ";
      for ( QStringList::iterator i = entries.begin();
            i != entries.end();
            ++i )
      {
        cerr << *i << " ";
      }
      cerr << "\n";

      CPPUNIT_ASSERT( entries.find( "bar" ) != entries.end() );
      CPPUNIT_ASSERT( entries.find( "baz" ) != entries.end() );
      CPPUNIT_ASSERT( entries.find( "quux" ) != entries.end() );

      CPPUNIT_ASSERT( entries.find( "xmmy" ) == entries.end() );
      CPPUNIT_ASSERT( entries.find( "blah" ) == entries.end() );
      CPPUNIT_ASSERT( entries.find( "bogus" ) == entries.end() );

      CPPUNIT_ASSERT( entries.find( "one" ) == entries.end() );
      CPPUNIT_ASSERT( entries.find( "two" ) == entries.end() );
      CPPUNIT_ASSERT( entries.find( "three" ) == entries.end() );
      CPPUNIT_ASSERT( entries.find( "four" ) == entries.end() );
      CPPUNIT_ASSERT( entries.find( "five" ) == entries.end() );
    }


    void testSubkeyList()
    {
      // at first the entry list should be empty -- "bogus" known to be empty
      QStringList entries = QgsProject::instance()->subkeyList( mScope, "/bogus" );
      cerr << "subkeys: ";
      for ( QStringList::iterator i = entries.begin();
            i != entries.end();
            ++i )
      {
        cerr << *i << " ";
      }
      cerr << "\n";

      CPPUNIT_ASSERT( entries.isEmpty() );

      // So subkeylist for /foo should return only "xmmy".

      entries = QgsProject::instance()->subkeyList( mScope, "/foo" );

      cerr << "subkeys: ";
      for ( QStringList::iterator i = entries.begin();
            i != entries.end();
            ++i )
      {
        cerr << *i << " ";
      }
      cerr << "\n";

      CPPUNIT_ASSERT( entries.find( "bar" ) == entries.end() );
      CPPUNIT_ASSERT( entries.find( "baz" ) == entries.end() );
      CPPUNIT_ASSERT( entries.find( "quux" ) == entries.end() );

      CPPUNIT_ASSERT( entries.find( "xmmy" ) != entries.end() );
      CPPUNIT_ASSERT( entries.find( "blah" ) == entries.end() );
      CPPUNIT_ASSERT( entries.find( "bogus" ) == entries.end() );

      CPPUNIT_ASSERT( entries.find( "one" ) == entries.end() );
      CPPUNIT_ASSERT( entries.find( "two" ) == entries.end() );
      CPPUNIT_ASSERT( entries.find( "three" ) == entries.end() );
      CPPUNIT_ASSERT( entries.find( "four" ) == entries.end() );
      CPPUNIT_ASSERT( entries.find( "five" ) == entries.end() );
    }


  private:

    /// file name for project file
    QString mFile;

    /// test project title
    QString mTitle;

    /// test project scope
    QString mScope;

    /// num value key
    QString mNumValueKey;

    /// double value key
    QString mDoubleValueKey;

    /// bool value key
    QString mBoolValueKey;

    /// string value key
    QString mStringValueKey;

    /// string list value key
    QString mStringListValueKey;

    /// num value const
    int mNumValueConst;

    /// double value const
    double mDoubleValueConst;

    /// bool value const
    bool mBoolValueConst;

    /// string value const
    QString mStringValueConst;

    /// string list value const
    QStringList mStringListValueConst;

}; // class ProjectTest

