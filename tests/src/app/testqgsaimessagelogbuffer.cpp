/***************************************************************************
  testqgsaimessagelogbuffer.cpp
  --------------------------------
  begin                : June 2026
***************************************************************************/

#include "ai/qgsaimessagelogbuffer.h"
#include "ai/tools/qgsaimessagelogtool.h"
#include "qgsmessagelog.h"
#include "qgstest.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QString>

using namespace Qt::StringLiterals;

class TestQgsAiMessageLogBuffer : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();
    void cleanupTestCase();

    void capturesMessagesFromMessageLog();
    void defaultLevelFilterExcludesInfo();
    void tagFilterMatchesExactTag();
    void limitIsRespected();
    void ringBufferEvictsOldestEntries();
    void readMessageLogToolReturnsEntries();
};

void TestQgsAiMessageLogBuffer::initTestCase()
{
  QgsApplication::initQgis();
}

void TestQgsAiMessageLogBuffer::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsAiMessageLogBuffer::capturesMessagesFromMessageLog()
{
  QgsAiMessageLogBuffer buffer;
  const int before = buffer.entryCount();

  QgsMessageLog::logMessage( u"capture test"_s, u"AI/Test"_s, Qgis::MessageLevel::Warning, false );

  QCOMPARE( buffer.entryCount(), before + 1 );

  QgsAiMessageLogBuffer::Query query;
  query.levels = { Qgis::MessageLevel::Warning };
  query.tags = { u"AI/Test"_s };
  const QgsAiMessageLogBuffer::QueryResult result = buffer.query( query );
  QVERIFY( !result.entries.isEmpty() );
  QCOMPARE( result.entries.constFirst().message, u"capture test"_s );
}

void TestQgsAiMessageLogBuffer::defaultLevelFilterExcludesInfo()
{
  QgsAiMessageLogBuffer buffer;

  QgsMessageLog::logMessage( u"info message"_s, u"AI/Test"_s, Qgis::MessageLevel::Info, false );
  QgsMessageLog::logMessage( u"warning message"_s, u"AI/Test"_s, Qgis::MessageLevel::Warning, false );

  QgsAiMessageLogBuffer::Query query;
  query.levels = {
    Qgis::MessageLevel::Warning,
    Qgis::MessageLevel::Critical,
  };
  query.tags = { u"AI/Test"_s };
  query.search = u"message"_s;
  const QgsAiMessageLogBuffer::QueryResult result = buffer.query( query );

  QCOMPARE( result.entries.size(), 1 );
  QCOMPARE( result.entries.constFirst().message, u"warning message"_s );
}

void TestQgsAiMessageLogBuffer::tagFilterMatchesExactTag()
{
  QgsAiMessageLogBuffer buffer;

  QgsMessageLog::logMessage( u"processing failure"_s, u"Processing"_s, Qgis::MessageLevel::Critical, false );
  QgsMessageLog::logMessage( u"python failure"_s, u"AI/Python"_s, Qgis::MessageLevel::Critical, false );

  QgsAiMessageLogBuffer::Query query;
  query.levels = { Qgis::MessageLevel::Critical };
  query.tags = { u"AI/Python"_s };
  const QgsAiMessageLogBuffer::QueryResult result = buffer.query( query );

  QCOMPARE( result.entries.size(), 1 );
  QCOMPARE( result.entries.constFirst().tag, u"AI/Python"_s );
}

void TestQgsAiMessageLogBuffer::limitIsRespected()
{
  QgsAiMessageLogBuffer buffer;

  for ( int i = 0; i < 5; ++i )
    QgsMessageLog::logMessage( u"limit %1"_s.arg( i ), u"AI/Test"_s, Qgis::MessageLevel::Warning, false );

  QgsAiMessageLogBuffer::Query query;
  query.levels = { Qgis::MessageLevel::Warning };
  query.tags = { u"AI/Test"_s };
  query.search = u"limit"_s;
  query.limit = 2;
  const QgsAiMessageLogBuffer::QueryResult result = buffer.query( query );

  QCOMPARE( result.entries.size(), 2 );
  QVERIFY( result.truncated );
  QCOMPARE( result.entries.at( 0 ).message, u"limit 4"_s );
  QCOMPARE( result.entries.at( 1 ).message, u"limit 3"_s );
}

void TestQgsAiMessageLogBuffer::ringBufferEvictsOldestEntries()
{
  QgsAiMessageLogBuffer buffer( nullptr, 3 );

  QgsMessageLog::logMessage( u"oldest"_s, u"AI/Ring"_s, Qgis::MessageLevel::Warning, false );
  QgsMessageLog::logMessage( u"middle"_s, u"AI/Ring"_s, Qgis::MessageLevel::Warning, false );
  QgsMessageLog::logMessage( u"recent"_s, u"AI/Ring"_s, Qgis::MessageLevel::Warning, false );
  QgsMessageLog::logMessage( u"newest"_s, u"AI/Ring"_s, Qgis::MessageLevel::Warning, false );

  QCOMPARE( buffer.entryCount(), 3 );

  QgsAiMessageLogBuffer::Query query;
  query.levels = { Qgis::MessageLevel::Warning };
  query.tags = { u"AI/Ring"_s };
  query.limit = 10;
  const QgsAiMessageLogBuffer::QueryResult result = buffer.query( query );

  QCOMPARE( result.entries.size(), 3 );
  QCOMPARE( result.entries.at( 0 ).message, u"newest"_s );
  QCOMPARE( result.entries.at( 2 ).message, u"middle"_s );
}

void TestQgsAiMessageLogBuffer::readMessageLogToolReturnsEntries()
{
  QgsAiMessageLogBuffer buffer;
  QgsAiReadMessageLogTool tool( &buffer );

  QgsMessageLog::logMessage( u"tool visible warning"_s, u"AI/Tool"_s, Qgis::MessageLevel::Warning, false );

  QJsonObject args;
  QJsonArray levels;
  levels.append( u"warning"_s );
  args.insert( u"levels"_s, levels );
  QJsonArray tags;
  tags.append( u"AI/Tool"_s );
  args.insert( u"tags"_s, tags );
  args.insert( u"search"_s, u"visible"_s );

  const QgsAiToolResult result = tool.execute( args );
  QVERIFY( result.success );
  QVERIFY( result.output.isObject() );

  const QJsonObject output = result.output.toObject();
  const QJsonArray entries = output.value( u"entries"_s ).toArray();
  QVERIFY( !entries.isEmpty() );
  QCOMPARE( entries.constFirst().toObject().value( u"message"_s ).toString(), u"tool visible warning"_s );
  QCOMPARE( output.value( u"returned"_s ).toInt(), entries.size() );
  QVERIFY( output.contains( u"total_buffered"_s ) );
}

QGSTEST_MAIN( TestQgsAiMessageLogBuffer )
#include "testqgsaimessagelogbuffer.moc"
