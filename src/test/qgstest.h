/***************************************************************************
  qgstest - %{Cpp:License:ClassName}

 ---------------------
 begin                : 5.12.2016
 copyright            : (C) 2016 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSTEST_H
#define QGSTEST_H

#include <QtTest/QTest>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDesktopServices>

#include "qgsapplication.h"

#include "qgsabstractgeometry.h"
#include "qgscurve.h"
#include "qgscircularstring.h"
#include "qgscompoundcurve.h"
#include "qgslinestring.h"
#include "qgsgeometrycollection.h"
#include "qgsmulticurve.h"
#include "qgsmultilinestring.h"
#include "qgsmultipoint.h"
#include "qgsmultisurface.h"
#include "qgsmultipolygon.h"
#include "qgspoint.h"
#include "qgssurface.h"
#include "qgscurvepolygon.h"
#include "qgspolygon.h"
#include "qgstriangle.h"
#include "qgsrectangle.h"
#include "qgsregularpolygon.h"
#include "qgsrange.h"
#include "qgsinterval.h"
#include "qgsrenderchecker.h"
#include "qgsmultirenderchecker.h"
#include "qgsunittypes.h"
#include "qgis_test.h"

#define QGSTEST_MAIN( TestObject )             \
  QT_BEGIN_NAMESPACE                           \
  QT_END_NAMESPACE                             \
  int main( int argc, char *argv[] )           \
  {                                            \
    QgsApplication app( argc, argv, false );   \
    app.init();                                \
    app.setAttribute( Qt::AA_Use96Dpi, true ); \
    QTEST_DISABLE_KEYPAD_NAVIGATION            \
    TestObject tc;                             \
    QTEST_SET_MAIN_SOURCE_PATH                 \
    return QTest::qExec( &tc, argc, argv );    \
  }


#define QGSCOMPARENEAR( value, expected, epsilon )                                                                                                                                                                     \
  {                                                                                                                                                                                                                    \
    bool _xxxresult = qgsDoubleNear( value, expected, epsilon );                                                                                                                                                       \
    if ( !_xxxresult )                                                                                                                                                                                                 \
    {                                                                                                                                                                                                                  \
      qDebug( "Expecting %.10f got %.10f (diff %.10f > %.10f)", static_cast<double>( expected ), static_cast<double>( value ), std::fabs( static_cast<double>( expected ) - value ), static_cast<double>( epsilon ) ); \
    }                                                                                                                                                                                                                  \
    QVERIFY( qgsDoubleNear( value, expected, epsilon ) );                                                                                                                                                              \
  }                                                                                                                                                                                                                    \
  ( void ) ( 0 )

#define QGSCOMPARENOTNEAR( value, not_expected, epsilon )                                                                                                                                                                             \
  {                                                                                                                                                                                                                                   \
    bool _xxxresult = qgsDoubleNear( value, not_expected, epsilon );                                                                                                                                                                  \
    if ( _xxxresult )                                                                                                                                                                                                                 \
    {                                                                                                                                                                                                                                 \
      qDebug( "Expecting %f to be differerent from %f (diff %f > %f)", static_cast<double>( value ), static_cast<double>( not_expected ), std::fabs( static_cast<double>( not_expected ) - value ), static_cast<double>( epsilon ) ); \
    }                                                                                                                                                                                                                                 \
    QVERIFY( !qgsDoubleNear( value, not_expected, epsilon ) );                                                                                                                                                                        \
  }                                                                                                                                                                                                                                   \
  ( void ) ( 0 )

#define QGSVERIFYLESSTHAN( value, expected )                                                                  \
  {                                                                                                           \
    bool _xxxresult = ( value ) < ( expected );                                                               \
    if ( !_xxxresult )                                                                                        \
    {                                                                                                         \
      qDebug( "Expecting < %.10f got %.10f", static_cast<double>( expected ), static_cast<double>( value ) ); \
    }                                                                                                         \
    QVERIFY( ( value ) < ( expected ) );                                                                      \
  }                                                                                                           \
  ( void ) ( 0 )

#define QGSCOMPARENEARPOINT( point1, point2, epsilon ) \
  {                                                    \
    QGSCOMPARENEAR( point1.x(), point2.x(), epsilon ); \
    QGSCOMPARENEAR( point1.y(), point2.y(), epsilon ); \
  }                                                    \
  ( void ) ( 0 )

#define QGSCOMPARENEARRECTANGLE( rectangle1, rectangle2, epsilon )           \
  {                                                                          \
    QGSCOMPARENEAR( rectangle1.xMinimum(), rectangle2.xMinimum(), epsilon ); \
    QGSCOMPARENEAR( rectangle1.xMaximum(), rectangle2.xMaximum(), epsilon ); \
    QGSCOMPARENEAR( rectangle1.yMinimum(), rectangle2.yMinimum(), epsilon ); \
    QGSCOMPARENEAR( rectangle1.yMaximum(), rectangle2.yMaximum(), epsilon ); \
  }                                                                          \
  ( void ) ( 0 )

#define QGSCOMPARENEARVECTOR3D( v1, v2, epsilon ) \
  {                                               \
    QGSCOMPARENEAR( v1.x(), v2.x(), epsilon );    \
    QGSCOMPARENEAR( v1.y(), v2.y(), epsilon );    \
    QGSCOMPARENEAR( v1.z(), v2.z(), epsilon );    \
  }                                               \
  ( void ) ( 0 )

//sometimes GML attributes are in a different order - but that's ok
#define QGSCOMPAREGML( result, expected )                                                                               \
  {                                                                                                                     \
    QCOMPARE( result.replace( QLatin1String( "ts=\" \" cs=\",\"" ), QLatin1String( "cs=\",\" ts=\" \"" ) ), expected ); \
  }                                                                                                                     \
  ( void ) ( 0 )

// Start your PostgreSQL-backend connection requiring test with this macro
#define QGSTEST_NEED_PGTEST_DB()         \
  if ( getenv( "QGIS_PGTEST_DB_SKIP" ) ) \
    QSKIP( "Test disabled due to QGIS_PGTEST_DB_SKIP env variable being set" );

// args are:
// const QString &name, const QString &referenceImage, const QgsMapSettings &mapSettings, int allowedMismatch = 0, int colorTolerance = 0
#define QGSRENDERMAPSETTINGSCHECK( ... ) renderMapSettingsCheck( __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__ )
#define QGSVERIFYRENDERMAPSETTINGSCHECK( ... ) QVERIFY( renderMapSettingsCheck( __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__ ) )

// args are either:
// const QString &name, const QString &referenceName, const QString &actualStr
#define QGSCOMPARELONGSTR( ... ) QVERIFY( checkLongStr( __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__ ) )

// args are either:
// const QString &name, const QString &referenceImage, const QImage &image, const QString &controlName = QString(), int allowedMismatch = 20, const QSize &sizeTolerance = QSize( 0, 0 ), const int colorTolerance = 0
// const QString &name, const QString &referenceImage, const QString &renderedFileName, const QString &controlName = QString(), int allowedMismatch = 20, const QSize &sizeTolerance = QSize( 0, 0 ), const int colorTolerance = 0
#define QGSIMAGECHECK( ... ) imageCheck( __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__ )
#define QGSVERIFYIMAGECHECK( ... ) QVERIFY( imageCheck( __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__ ) )

// args are:
// const QString &name, QgsLayout *layout, int page = 0, int allowedMismatch = 0, const QSize size = QSize(), int colorTolerance = 0
#define QGSLAYOUTCHECK( ... ) layoutCheck( __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__ )
#define QGSVERIFYLAYOUTCHECK( ... ) QVERIFY( layoutCheck( __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__ ) )

/**
 * Base class for tests.
 *
 * \since QGIS 3.28
 */
class TEST_EXPORT QgsTest : public QObject
{
    Q_OBJECT

  public:
    //! Returns TRUE if test is running on a CI infrastructure
    static bool isCIRun()
    {
      return qgetenv( "QGIS_CONTINUOUS_INTEGRATION_RUN" ) == QStringLiteral( "true" );
    }

    static bool runFlakyTests()
    {
      return qgetenv( "RUN_FLAKY_TESTS" ) == QStringLiteral( "true" );
    }

    QgsTest( const QString &name, const QString &controlPathPrefix = QString() )
      : mName( name )
      , mControlPathPrefix( controlPathPrefix )
      , mTestDataDir( QStringLiteral( TEST_DATA_DIR ) + '/' ) //defined in CmakeLists.txt
    {}

    ~QgsTest() override
    {
      if ( !mReport.isEmpty() )
        writeLocalHtmlReport( mReport );
      if ( !mMarkdownReport.isEmpty() )
        writeMarkdownReport( mMarkdownReport );
    }

    /**
     * Returns the full path to the test data with the given file path.
     */
    QString testDataPath( const QString &filePath ) const
    {
      return mTestDataDir.filePath( filePath.startsWith( '/' ) ? filePath.mid( 1 ) : filePath );
    }

    /**
     * Copies the test data with the given file path to a
     * temporary directory and returns the full path to the copy.
     */
    QString copyTestData( const QString &filePath )
    {
      const QString srcPath = testDataPath( filePath );
      const QFileInfo srcFileInfo( srcPath );

      // lazy create temporary dir
      if ( !mTemporaryDir )
        mTemporaryDir = std::make_unique<QTemporaryDir>();

      // we put all copies into a subdirectory of the temporary dir, so that we isolate clean copies
      // of the same source file used by different test functions
      mTemporaryCopyCount++;
      const QString temporarySubdirectory = QStringLiteral( "test_%1" ).arg( mTemporaryCopyCount );
      QDir().mkdir( mTemporaryDir->filePath( temporarySubdirectory ) );

      const QString copiedDataPath = mTemporaryDir->filePath( temporarySubdirectory + '/' + srcFileInfo.fileName() );

      QFile::copy( srcPath, copiedDataPath );
      return copiedDataPath;
    }

    /**
     * Recursively copies a whole directory.
     */
    void copyDirectory( const QString &source, const QString &destination )
    {
      QDir sourceDir( source );
      if ( !sourceDir.exists() )
        return;

      QDir destDir( destination );
      if ( !destDir.exists() )
      {
        destDir.mkdir( destination );
      }

      const QStringList files = sourceDir.entryList( QDir::Files );
      for ( const QString &file : files )
      {
        const QString srcFileName = sourceDir.filePath( file );
        const QString destFileName = destDir.filePath( file );
        QFile::copy( srcFileName, destFileName );
      }
      const QStringList dirs = sourceDir.entryList( QDir::AllDirs | QDir::NoDotAndDotDot );
      for ( const QString &dir : dirs )
      {
        const QString srcDirName = sourceDir.filePath( dir );
        const QString destDirName = destDir.filePath( dir );
        copyDirectory( srcDirName, destDirName );
      }
    }

    /**
     * Copies a complete directory from the test data with the given directory path to a
     * temporary directory and returns the full path to the copy.
     */
    QString copyTestDataDirectory( const QString &dirPath )
    {
      const QString srcPath = testDataPath( dirPath );
      const QFileInfo srcFileInfo( srcPath );

      // lazy create temporary dir
      if ( !mTemporaryDir )
        mTemporaryDir = std::make_unique<QTemporaryDir>();

      // we put all copies into a subdirectory of the temporary dir, so that we isolate clean copies
      // of the same source file used by different test functions
      mTemporaryCopyCount++;
      const QString temporarySubdirectory = QStringLiteral( "test_%1" ).arg( mTemporaryCopyCount );
      QDir().mkdir( mTemporaryDir->filePath( temporarySubdirectory ) );

      const QString copiedDataPath = mTemporaryDir->filePath( temporarySubdirectory + '/' + srcFileInfo.fileName() );

      copyDirectory( srcPath, copiedDataPath );
      return copiedDataPath;
    }

  protected:
    QString mName;
    QString mReport;
    QString mControlPathPrefix;
    std::unique_ptr<QTemporaryDir> mTemporaryDir;
    int mTemporaryCopyCount = 0;

    const QDir mTestDataDir;

    /**
     * For internal use only -- use QGSRENDERMAPSETTINGSCHECK or QGSVERIFYRENDERMAPSETTINGSCHECK macros instead.
     */
    bool renderMapSettingsCheck( const char *file, const char *function, int line, const QString &name, const QString &referenceImage, const QgsMapSettings &mapSettings, int allowedMismatch = 0, int colorTolerance = 0 )
    {
      //use the QgsRenderChecker test utility class to
      //ensure the rendered output matches our control image
      QgsMultiRenderChecker checker;
      checker.setFileFunctionLine( file, function, line );
      checker.setControlPathPrefix( mControlPathPrefix );
      checker.setControlName( "expected_" + referenceImage );
      checker.setMapSettings( mapSettings );
      checker.setColorTolerance( colorTolerance );
      const bool result = checker.runTest( name, allowedMismatch );
      if ( !result )
      {
        appendToReport( name, checker.report(), checker.markdownReport() );
      }
      return result;
    }

    /**
     * For internal use only -- use QGSCOMPARELONGSTR macro instead.
     */
    bool checkLongStr( const char *file, const char *, int line, const QString &name, const QString &referenceName, const QByteArray &actualStr )
    {
      QString header = QString( "checkLongStr (%1, %2):" ).arg( name, referenceName );
      QString subPath = "control_files/" + mControlPathPrefix + "/expected_" + name + "/" + "expected_" + referenceName;
      QString expectedPath = testDataPath( subPath );
      QFile expectedFile( expectedPath );
      if ( !expectedFile.open( QFile::ReadOnly | QIODevice::Text ) )
      {
        qWarning() << header.toStdString().c_str() << "Unable to open expected data file" << expectedPath;
        return false;
      }
      QByteArray expectedStr = expectedFile.readAll();

      if ( actualStr.size() != expectedStr.size() )
      {
        qWarning() << header.toStdString().c_str() << "Array have not the same length (actual vs expected):" << actualStr.size() << "vs" << expectedStr.size() << ".";
      }

      const int strSize = std::max( actualStr.size(), expectedStr.size() );
      constexpr int step = 100;
      for ( int i = 0; i < strSize || i < strSize + step; i += step )
      {
        QByteArray act = actualStr.mid( i, step );
        QByteArray exp = expectedStr.mid( i, step );

        if ( act != exp )
        {
          QString actualPath = QDir::tempPath() + "/actual_" + name + "_" + referenceName;
          QFile actualFile( actualPath );
          if ( actualFile.open( QFile::WriteOnly | QIODevice::Text ) )
          {
            actualFile.write( actualStr );
          }
          else
          {
            qWarning() << header.toStdString().c_str() << "Unable to write actual data to file" << actualPath << ".";
          }

          qWarning() << header.toStdString().c_str() << "Hex version of the parts of array that differ starting from char" << i << "."
                     << "\n   Actual hex:  " << act.toHex() << "\n   Expected hex:" << exp.toHex();
          QString msg = QString( "%1 Comparison failed in starting from char %2." ).arg( header ).arg( QString::number( i ) );

          // create copies of data as QTest::compare_helper will delete them
          char *actualCopy = new char[act.size() + 1];
          memcpy( actualCopy, act.data(), act.size() );
          actualCopy[act.size()] = 0;
          char *expectedCopy = new char[exp.size() + 1];
          memcpy( expectedCopy, exp.data(), exp.size() );
          expectedCopy[exp.size()] = 0;

          return QTest::compare_helper( act == exp, msg.toStdString().c_str(), // NOLINT(clang-analyzer-cplusplus.NewDeleteLeaks)
                                        actualCopy, expectedCopy, actualPath.toStdString().c_str(), subPath.toStdString().c_str(), file, line );
        }
      }
      return true;
    }

    /**
     * For internal use only -- use QGSIMAGECHECK or QGSVERIFYIMAGECHECK macros instead.
     */
    bool imageCheck( const char *file, const char *function, int line, const QString &name, const QString &referenceImage, const QImage &image, const QString &controlName = QString(), int allowedMismatch = 20, const QSize &sizeTolerance = QSize( 0, 0 ), const int colorTolerance = 0 )
    {
      if ( image.isNull() )
        return false;
      const QString renderedFileName = QDir::tempPath() + '/' + name + ".png";
      if ( !image.save( renderedFileName ) )
        return false;

      return imageCheck( file, function, line, name, referenceImage, renderedFileName, controlName, allowedMismatch, sizeTolerance, colorTolerance );
    }

    /**
     * For internal use only -- use QGSIMAGECHECK or QGSVERIFYIMAGECHECK macros instead.
     */
    bool imageCheck( const char *file, const char *function, int line, const QString &name, const QString &referenceImage, const QString &renderedFileName, const QString &controlName = QString(), int allowedMismatch = 20, const QSize &sizeTolerance = QSize( 0, 0 ), const int colorTolerance = 0 )
    {
      QgsMultiRenderChecker checker;
      checker.setControlPathPrefix( mControlPathPrefix );
      checker.setFileFunctionLine( file, function, line );
      checker.setControlName( controlName.isEmpty() ? "expected_" + referenceImage : controlName );
      checker.setRenderedImage( renderedFileName );
      checker.setColorTolerance( colorTolerance );
      checker.setSizeTolerance( sizeTolerance.width(), sizeTolerance.height() );

      const bool result = checker.runTest( name, allowedMismatch );
      if ( !result )
      {
        appendToReport( name, checker.report(), checker.markdownReport() );
      }
      return result;
    }

    /**
     * For internal use only -- use QGSLAYOUTCHECK or QGSVERIFYLAYOUTCHECK macros instead.
     */
    bool layoutCheck( const char *file, const char *function, int line, const QString &name, QgsLayout *layout, int page = 0, int allowedMismatch = 0, const QSize size = QSize(), int colorTolerance = 0 )
    {
      QgsLayoutChecker checker( name, layout );
      checker.setFileFunctionLine( file, function, line );
      checker.setControlPathPrefix( mControlPathPrefix );
      if ( size.isValid() )
        checker.setSize( size );
      if ( colorTolerance > 0 )
        checker.setColorTolerance( colorTolerance );

      QString report;
      const bool result = checker.testLayout( report, page, allowedMismatch );
      if ( !result )
      {
        appendToReport( name, report, checker.markdownReport() );
      }
      return result;
    }

    /**
     * Appends some \a html and \a markdown to the test report.
     *
     * This should be used only for appending useful information when a test fails.
     */
    void appendToReport( const QString &testName, const QString &html, const QString &markdown = QString() )
    {
      QString testIdentifier;
      if ( QTest::currentDataTag() )
        testIdentifier = QStringLiteral( "%1 (%2: %3)" ).arg( testName, QTest::currentTestFunction(), QTest::currentDataTag() );
      else
        testIdentifier = QStringLiteral( "%1 (%2)" ).arg( testName, QTest::currentTestFunction() );

      if ( !html.isEmpty() )
      {
        mReport += QStringLiteral( "<h2>%1</h2>\n" ).arg( testIdentifier );
        mReport += html;
      }

      const QString markdownContent = markdown.isEmpty() ? html : markdown;
      if ( !markdownContent.isEmpty() )
      {
        mMarkdownReport += QStringLiteral( "## %1\n\n" ).arg( testIdentifier );
        mMarkdownReport += markdownContent + QStringLiteral( "\n\n" );
      }
    }

  private:
    QString mMarkdownReport;

    /**
     * Writes out a HTML report to a temporary file for visual comparison
     * of test results on a local build.
     */
    void writeLocalHtmlReport( const QString &report )
    {
      const QDir reportDir = QgsRenderChecker::testReportDir();
      if ( !reportDir.exists() )
        QDir().mkpath( reportDir.path() );

      const QString reportFile = reportDir.filePath( "index.html" );
      QFile file( reportFile );

      QFile::OpenMode mode = QIODevice::WriteOnly;
      bool fileIsEmpty = true;
      if ( qgetenv( "QGIS_CONTINUOUS_INTEGRATION_RUN" ) == QStringLiteral( "true" )
           || qgetenv( "QGIS_APPEND_TO_TEST_REPORT" ) == QStringLiteral( "true" ) )
      {
        mode |= QIODevice::Append;
        if ( file.open( QIODevice::ReadOnly ) )
        {
          fileIsEmpty = file.readAll().isEmpty();
        }
      }
      else
      {
        mode |= QIODevice::Truncate;
      }

      if ( file.open( mode ) )
      {
        QTextStream stream( &file );
        if ( fileIsEmpty )
        {
          // append standard header
          QFile reportHeader( QStringLiteral( TEST_DATA_DIR ) + "/../test_report_header.html" );
          if ( reportHeader.open( QIODevice::ReadOnly ) )
          {
            stream << reportHeader.readAll();
          }

          // embed render checker script so that we can run the HTML report from anywhere
          stream << QStringLiteral( "<script>" );
          QFile renderCheckerScript( QStringLiteral( TEST_DATA_DIR ) + "/../renderchecker.js" );
          if ( renderCheckerScript.open( QIODevice::ReadOnly ) )
          {
            stream << renderCheckerScript.readAll();
          }
          stream << QStringLiteral( "</script>" );
        }

        stream << QStringLiteral( "<h1>%1</h1>\n" ).arg( mName );
        stream << report;
        file.close();

        if ( !isCIRun() )
          QDesktopServices::openUrl( QStringLiteral( "file:///%1" ).arg( reportFile ) );
      }
    }

    /**
     * Writes out a markdown report to a temporary file for use on CI runs.
     */
    void writeMarkdownReport( const QString &report )
    {
      const QDir reportDir = QgsRenderChecker::testReportDir();
      if ( !reportDir.exists() )
        QDir().mkpath( reportDir.path() );

      const QString reportFile = reportDir.filePath( "summary.md" );
      QFile file( reportFile );

      QFile::OpenMode mode = QIODevice::WriteOnly;
      if ( qgetenv( "QGIS_CONTINUOUS_INTEGRATION_RUN" ) == QStringLiteral( "true" )
           || qgetenv( "QGIS_APPEND_TO_TEST_REPORT" ) == QStringLiteral( "true" ) )
        mode |= QIODevice::Append;
      else
        mode |= QIODevice::Truncate;

      if ( file.open( mode ) )
      {
        QTextStream stream( &file );
        stream << report;
        file.close();
      }
    }
};

/**
 * For QCOMPARE pretty printing
 */
char *toString( const QgsAbstractGeometry &geom )
{
  return QTest::toString( geom.asWkt() );
}

char *toString( const QgsCurve &geom )
{
  return QTest::toString( geom.asWkt() );
}

char *toString( const QgsCircularString &geom )
{
  return QTest::toString( geom.asWkt() );
}

char *toString( const QgsCompoundCurve &geom )
{
  return QTest::toString( geom.asWkt() );
}

char *toString( const QgsLineString &geom )
{
  return QTest::toString( geom.asWkt() );
}

char *toString( const QgsGeometryCollection &geom )
{
  return QTest::toString( geom.asWkt() );
}

char *toString( const QgsMultiCurve &geom )
{
  return QTest::toString( geom.asWkt() );
}

char *toString( const QgsMultiLineString &geom )
{
  return QTest::toString( geom.asWkt() );
}

char *toString( const QgsMultiPoint &geom )
{
  return QTest::toString( geom.asWkt() );
}

char *toString( const QgsMultiSurface &geom )
{
  return QTest::toString( geom.asWkt() );
}

char *toString( const QgsMultiPolygon &geom )
{
  return QTest::toString( geom.asWkt() );
}

char *toString( const QgsPoint &geom )
{
  return QTest::toString( geom.asWkt() );
}

char *toString( const QgsPointXY &geom )
{
  return QTest::toString( geom.asWkt() );
}

char *toString( const QgsSurface &geom )
{
  return QTest::toString( geom.asWkt() );
}

char *toString( const QgsCurvePolygon &geom )
{
  return QTest::toString( geom.asWkt() );
}

char *toString( const QgsPolygon &geom )
{
  return QTest::toString( geom.asWkt() );
}

char *toString( const QgsRegularPolygon &geom )
{
  return QTest::toString( geom.toString() );
}

char *toString( const QgsTriangle &geom )
{
  return QTest::toString( geom.asWkt() );
}

char *toString( const QgsRectangle &geom )
{
  return QTest::toString( geom.toString() );
}

char *toString( const QgsEllipse &geom )
{
  return QTest::toString( geom.toString() );
}

char *toString( const QgsCircle &geom )
{
  return QTest::toString( geom.toString() );
}

char *toString( const QgsDateTimeRange &range )
{
  return QTest::toString( QStringLiteral( "<QgsDateTimeRange: %1%2, %3%4>" ).arg( range.includeBeginning() ? QStringLiteral( "[" ) : QStringLiteral( "(" ), range.begin().toString( Qt::ISODateWithMs ), range.end().toString( Qt::ISODateWithMs ), range.includeEnd() ? QStringLiteral( "]" ) : QStringLiteral( ")" ) ) );
}

char *toString( const QgsInterval &interval )
{
  return QTest::toString( QStringLiteral( "<QgsInterval: %1 %2>" ).arg( interval.originalDuration() ).arg( QgsUnitTypes::toString( interval.originalUnit() ) ) );
}


#endif // QGSTEST_H
