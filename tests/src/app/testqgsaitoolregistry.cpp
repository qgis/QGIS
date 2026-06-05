/***************************************************************************
  testqgsaitoolregistry.cpp
  --------------------------
  begin                : April 2026
  copyright            : (C) 2026
***************************************************************************/

#include "ai/tools/qgsaitoolregistry.h"
#include "qgstest.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QSignalSpy>
#include <memory>

namespace
{
  class FakeEchoTool : public QgsAiTool
  {
    public:
      explicit FakeEchoTool( const QString &name, bool requiresApproval = false )
        : mName( name ), mRequiresApproval( requiresApproval ) {}
      FakeEchoTool( const QString &name, bool requiresApproval, bool available )
        : mName( name ), mRequiresApproval( requiresApproval ), mAvailable( available ) {}

      QString name() const override { return mName; }
      QString description() const override { return QStringLiteral( "Echoes the 'text' argument." ); }

      QJsonObject schema() const override
      {
        QJsonObject properties;
        QJsonObject textProp;
        textProp.insert( QStringLiteral( "type" ), QStringLiteral( "string" ) );
        properties.insert( QStringLiteral( "text" ), textProp );

        QJsonObject schemaObject;
        schemaObject.insert( QStringLiteral( "type" ), QStringLiteral( "object" ) );
        schemaObject.insert( QStringLiteral( "properties" ), properties );

        QJsonArray required;
        required.append( QStringLiteral( "text" ) );
        schemaObject.insert( QStringLiteral( "required" ), required );
        return schemaObject;
      }

      QgsAiToolResult execute( const QJsonObject &args ) override
      {
        if ( !args.contains( QStringLiteral( "text" ) ) )
          return QgsAiToolResult::error( QStringLiteral( "missing 'text'" ) );
        return QgsAiToolResult::ok( args.value( QStringLiteral( "text" ) ) );
      }

      bool requiresApproval() const override { return mRequiresApproval; }
      bool isAvailable() const override { return mAvailable; }
      QString availabilityReason() const override { return QStringLiteral( "tool unavailable" ); }

    private:
      QString mName;
      bool mRequiresApproval;
      bool mAvailable = true;
  };
}

class TestQgsAiToolRegistry : public QObject
{
    Q_OBJECT

  private slots:
    void registerAndLookup();
    void rejectsDuplicateNames();
    void rejectsEmptyNameAndNull();
    void schemasJsonContainsAllTools();
    void schemasJsonFilter();
    void unavailableToolsAreHiddenAndNotExecuted();
    void executeRoundTrip();
    void clearEmptiesRegistry();
};

void TestQgsAiToolRegistry::registerAndLookup()
{
  QgsAiToolRegistry registry;
  QSignalSpy spy( &registry, &QgsAiToolRegistry::toolRegistered );

  QVERIFY( registry.registerTool( std::make_unique<FakeEchoTool>( QStringLiteral( "echo" ) ) ) );
  QCOMPARE( registry.count(), 1 );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( spy.takeFirst().at( 0 ).toString(), QStringLiteral( "echo" ) );

  QgsAiTool *tool = registry.find( QStringLiteral( "echo" ) );
  QVERIFY( tool );
  QCOMPARE( tool->name(), QStringLiteral( "echo" ) );
  QVERIFY( !registry.find( QStringLiteral( "missing" ) ) );
  QCOMPARE( registry.toolNames(), QStringList() << QStringLiteral( "echo" ) );
}

void TestQgsAiToolRegistry::rejectsDuplicateNames()
{
  QgsAiToolRegistry registry;
  QVERIFY( registry.registerTool( std::make_unique<FakeEchoTool>( QStringLiteral( "echo" ) ) ) );
  QVERIFY( !registry.registerTool( std::make_unique<FakeEchoTool>( QStringLiteral( "echo" ) ) ) );
  QCOMPARE( registry.count(), 1 );
}

void TestQgsAiToolRegistry::rejectsEmptyNameAndNull()
{
  QgsAiToolRegistry registry;
  QVERIFY( !registry.registerTool( nullptr ) );
  QVERIFY( !registry.registerTool( std::make_unique<FakeEchoTool>( QString() ) ) );
  QCOMPARE( registry.count(), 0 );
}

void TestQgsAiToolRegistry::schemasJsonContainsAllTools()
{
  QgsAiToolRegistry registry;
  registry.registerTool( std::make_unique<FakeEchoTool>( QStringLiteral( "alpha" ) ) );
  registry.registerTool( std::make_unique<FakeEchoTool>( QStringLiteral( "beta" ) ) );

  const QJsonArray schemas = registry.schemasJson();
  QCOMPARE( schemas.size(), 2 );

  QSet<QString> names;
  for ( const QJsonValue &value : schemas )
  {
    const QJsonObject obj = value.toObject();
    QVERIFY( obj.contains( QStringLiteral( "name" ) ) );
    QVERIFY( obj.contains( QStringLiteral( "description" ) ) );
    QVERIFY( obj.contains( QStringLiteral( "input_schema" ) ) );
    names.insert( obj.value( QStringLiteral( "name" ) ).toString() );
  }
  QVERIFY( names.contains( QStringLiteral( "alpha" ) ) );
  QVERIFY( names.contains( QStringLiteral( "beta" ) ) );
}

void TestQgsAiToolRegistry::schemasJsonFilter()
{
  QgsAiToolRegistry registry;
  registry.registerTool( std::make_unique<FakeEchoTool>( QStringLiteral( "read_file" ) ) );
  registry.registerTool( std::make_unique<FakeEchoTool>( QStringLiteral( "apply_patch" ), true ) );

  const QJsonArray filtered = registry.schemasJson( QStringList() << QStringLiteral( "read_file" ) );
  QCOMPARE( filtered.size(), 1 );
  QCOMPARE( filtered.first().toObject().value( QStringLiteral( "name" ) ).toString(),
            QStringLiteral( "read_file" ) );
}

void TestQgsAiToolRegistry::unavailableToolsAreHiddenAndNotExecuted()
{
  QgsAiToolRegistry registry;
  registry.registerTool( std::make_unique<FakeEchoTool>( QStringLiteral( "run_python" ), true, false ) );

  QVERIFY( registry.availableToolNames().isEmpty() );
  QVERIFY( registry.schemasJson().isEmpty() );

  QJsonObject args;
  args.insert( QStringLiteral( "text" ), QStringLiteral( "hello" ) );
  const QgsAiToolResult result = registry.execute( QStringLiteral( "run_python" ), args );
  QVERIFY( !result.success );
  QVERIFY( result.errorMessage.contains( QStringLiteral( "unavailable" ) ) );
}

void TestQgsAiToolRegistry::executeRoundTrip()
{
  QgsAiToolRegistry registry;
  registry.registerTool( std::make_unique<FakeEchoTool>( QStringLiteral( "echo" ) ) );

  QgsAiTool *tool = registry.find( QStringLiteral( "echo" ) );
  QVERIFY( tool );

  QJsonObject args;
  args.insert( QStringLiteral( "text" ), QStringLiteral( "hello" ) );
  const QgsAiToolResult result = tool->execute( args );
  QVERIFY( result.success );
  QCOMPARE( result.output.toString(), QStringLiteral( "hello" ) );

  const QgsAiToolResult missing = tool->execute( QJsonObject() );
  QVERIFY( !missing.success );
  QVERIFY( !missing.errorMessage.isEmpty() );
}

void TestQgsAiToolRegistry::clearEmptiesRegistry()
{
  QgsAiToolRegistry registry;
  registry.registerTool( std::make_unique<FakeEchoTool>( QStringLiteral( "echo" ) ) );
  QCOMPARE( registry.count(), 1 );
  registry.clear();
  QCOMPARE( registry.count(), 0 );
  QVERIFY( !registry.find( QStringLiteral( "echo" ) ) );
}

QGSTEST_MAIN( TestQgsAiToolRegistry )
#include "testqgsaitoolregistry.moc"
