/***************************************************************************
     test_template.cpp
     --------------------------------------
    Date                 : Sun Sep 16 12:22:23 AKDT 2007
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
#include <QtTest>
#include <QObject>
#include <QString>
#include <QObject>
#include <qgsapplication.h>
//header for class being tested
#include <qgsexpression.h>
#include <qgsfeature.h>
#include <qgsgeometry.h>

#if QT_VERSION < 0x40701
// See http://hub.qgis.org/issues/4284
Q_DECLARE_METATYPE( QVariant )
#endif

class TestQgsExpression: public QObject
{
    Q_OBJECT;
  private slots:

    void initTestCase()
    {
      //
      // Runs once before any tests are run
      //
      // init QGIS's paths - true means that all path will be inited from prefix
      QgsApplication::init();
      QgsApplication::initQgis();
      QgsApplication::showSettings();
    }

    void parsing_data()
    {
      QTest::addColumn<QString>( "string" );
      QTest::addColumn<bool>( "valid" );

      // invalid strings
      QTest::newRow( "empty string" ) << "" << false;
      QTest::newRow( "invalid character" ) << "@" << false;
      QTest::newRow( "invalid column reference" ) << "my col" << false;
      QTest::newRow( "invalid binary operator" ) << "1+" << false;
      QTest::newRow( "invalid function no params" ) << "cos" << false;
      QTest::newRow( "invalid function not known" ) << "coz(1)" << false;
      QTest::newRow( "invalid operator IN" ) << "x in y" << false;
      QTest::newRow( "empty node list" ) << "1 in ()" << false;
      QTest::newRow( "invalid sqrt params" ) << "sqrt(2,4)" << false;
      QTest::newRow( "special column as function" ) << "$id()" << false;
      QTest::newRow( "unknown special column" ) << "$idx" << false;

      // valid strings
      QTest::newRow( "null" ) << "NULL" << true;
      QTest::newRow( "int literal" ) << "1" << true;
      QTest::newRow( "float literal" ) << "1.23" << true;
      QTest::newRow( "string literal" ) << "'test'" << true;
      QTest::newRow( "column reference" ) << "my_col" << true;
      QTest::newRow( "quoted column" ) << "\"my col\"" << true;
      QTest::newRow( "unary minus" ) << "--3" << true;
      QTest::newRow( "function" ) << "cos(0)" << true;
      QTest::newRow( "function2" ) << "atan2(0,1)" << true;
      QTest::newRow( "operator IN" ) << "x in (a,b)" << true;
      QTest::newRow( "pow" ) << "2 ^ 8" << true;
      QTest::newRow( "$id" ) << "$id + 1" << true;

      QTest::newRow( "arithmetics" ) << "1+2*3" << true;
      QTest::newRow( "logic" ) << "be or not be" << true;

      QTest::newRow( "conditions +1" ) << "case when x then y end" << true;
      QTest::newRow( "conditions +2" ) << "case when x then y else z end" << true;
      QTest::newRow( "conditions +3" ) << "case when x then y when a then b end" << true;
      QTest::newRow( "conditions +4" ) << "case when x then y when a then b else z end" << true;

      QTest::newRow( "conditions -1" ) << "case end" << false;
      QTest::newRow( "conditions -2" ) << "when x then y" << false;
      QTest::newRow( "conditions -3" ) << "case" << false;
      QTest::newRow( "conditions -4" ) << "case when x y end" << false;
      QTest::newRow( "conditions -5" ) << "case y end" << false;
    }
    void parsing()
    {
      QFETCH( QString, string );
      QFETCH( bool, valid );

      QgsExpression exp( string );

      if ( exp.hasParserError() )
        qDebug() << "Parser error: " << exp.parserErrorString();
      else
        qDebug() << "Parsed string: " << exp.dump();

      QCOMPARE( !exp.hasParserError(), valid );
    }

    void parsing_with_locale()
    {
      // check that parsing of numbers works correctly even when using some other locale

      char* old_locale = setlocale( LC_NUMERIC, NULL );
      qDebug( "Old locale: %s", old_locale );
      setlocale( LC_NUMERIC, "de_DE.UTF8" );
      char* new_locale = setlocale( LC_NUMERIC, NULL );
      qDebug( "New locale: %s", new_locale );

      QgsExpression exp( "1.23 + 4.56" );
      QVERIFY( !exp.hasParserError() );

      setlocale( LC_NUMERIC, "" );

      QVariant v = exp.evaluate();
      QCOMPARE( v.toDouble(), 5.79 );
    }

    void evaluation_data()
    {
      QTest::addColumn<QString>( "string" );
      QTest::addColumn<bool>( "evalError" );
      QTest::addColumn<QVariant>( "result" );

      // literal evaluation
      QTest::newRow( "literal null" ) << "NULL" << false << QVariant();
      QTest::newRow( "literal int" ) << "123" << false << QVariant( 123 );
      QTest::newRow( "literal double" ) << "1.2" << false << QVariant( 1.2 );
      QTest::newRow( "literal text" ) << "'hello'" << false << QVariant( "hello" );

      // unary minus
      QTest::newRow( "unary minus double" ) << "-1.3" << false << QVariant( -1.3 );
      QTest::newRow( "unary minus int" ) << "-1" << false << QVariant( -1 );
      QTest::newRow( "unary minus text" ) << "-'hello'" << true << QVariant();
      QTest::newRow( "unary minus null" ) << "-null" << true << QVariant();

      // arithmetics
      QTest::newRow( "plus int" ) << "1+3" << false << QVariant( 4 );
      QTest::newRow( "plus double" ) << "1+1.3" << false << QVariant( 2.3 );
      QTest::newRow( "plus with null" ) << "null+3" << false << QVariant();
      QTest::newRow( "plus invalid" ) << "1+'foo'" << true << QVariant();
      QTest::newRow( "minus int" ) << "1-3" << false << QVariant( -2 );
      QTest::newRow( "mul int" ) << "8*7" << false << QVariant( 56 );
      QTest::newRow( "div int" ) << "20/6" << false << QVariant( 3 );
      QTest::newRow( "mod int" ) << "20%6" << false << QVariant( 2 );
      QTest::newRow( "minus double" ) << "5.2-3.1" << false << QVariant( 2.1 );
      QTest::newRow( "mul double" ) << "2.1*5" << false << QVariant( 10.5 );
      QTest::newRow( "div double" ) << "11.0/2" << false << QVariant( 5.5 );
      QTest::newRow( "mod double" ) << "6.1 % 2.5" << false << QVariant( 1.1 );
      QTest::newRow( "pow" ) << "2^8" << false << QVariant( 256. );
      QTest::newRow( "division by zero" ) << "1/0" << false << QVariant();
      QTest::newRow( "division by zero" ) << "1.0/0.0" << false << QVariant();

      // comparison
      QTest::newRow( "eq int" ) << "1+1 = 2" << false << QVariant( 1 );
      QTest::newRow( "eq double" ) << "3.2 = 2.2+1" << false << QVariant( 1 );
      QTest::newRow( "eq string" ) << "'a' = 'b'" << false << QVariant( 0 );
      QTest::newRow( "eq null" ) << "2 = null" << false << QVariant();
      QTest::newRow( "eq mixed" ) << "'a' = 1" << false << QVariant( 0 );
      QTest::newRow( "ne int 1" ) << "3 != 4" << false << QVariant( 1 );
      QTest::newRow( "ne int 2" ) << "3 != 3" << false << QVariant( 0 );
      QTest::newRow( "lt int 1" ) << "3 < 4" << false << QVariant( 1 );
      QTest::newRow( "lt int 2" ) << "3 < 3" << false << QVariant( 0 );
      QTest::newRow( "gt int 1" ) << "3 > 4" << false << QVariant( 0 );
      QTest::newRow( "gt int 2" ) << "3 > 3" << false << QVariant( 0 );
      QTest::newRow( "le int 1" ) << "3 <= 4" << false << QVariant( 1 );
      QTest::newRow( "le int 2" ) << "3 <= 3" << false << QVariant( 1 );
      QTest::newRow( "ge int 1" ) << "3 >= 4" << false << QVariant( 0 );
      QTest::newRow( "ge int 2" ) << "3 >= 3" << false << QVariant( 1 );
      QTest::newRow( "lt text 1" ) << "'bar' < 'foo'" << false << QVariant( 1 );
      QTest::newRow( "lt text 2" ) << "'foo' < 'bar'" << false << QVariant( 0 );

      // is, is not
      QTest::newRow( "is null,null" ) << "null is null" << false << QVariant( 1 );
      QTest::newRow( "is not null,null" ) << "null is not null" << false << QVariant( 0 );
      QTest::newRow( "is null" ) << "1 is null" << false << QVariant( 0 );
      QTest::newRow( "is not null" ) << "1 is not null" << false << QVariant( 1 );
      QTest::newRow( "is int" ) << "1 is 1" << false << QVariant( 1 );
      QTest::newRow( "is not int" ) << "1 is not 1" << false << QVariant( 0 );
      QTest::newRow( "is text" ) << "'x' is 'y'" << false << QVariant( 0 );
      QTest::newRow( "is not text" ) << "'x' is not 'y'" << false << QVariant( 1 );

      //  logical
      QTest::newRow( "T or F" ) << "1=1 or 2=3" << false << QVariant( 1 );
      QTest::newRow( "F or F" ) << "1=2 or 2=3" << false << QVariant( 0 );
      QTest::newRow( "T and F" ) << "1=1 and 2=3" << false << QVariant( 0 );
      QTest::newRow( "T and T" ) << "1=1 and 2=2" << false << QVariant( 1 );
      QTest::newRow( "not T" ) << "not 1=1" << false << QVariant( 0 );
      QTest::newRow( "not F" ) << "not 2=3" << false << QVariant( 1 );
      QTest::newRow( "null" ) << "null=1" << false << QVariant();
      QTest::newRow( "U or F" ) << "null=1 or 2=3" << false << QVariant();
      QTest::newRow( "U and F" ) << "null=1 and 2=3" << false << QVariant( 0 );
      QTest::newRow( "invalid and" ) << "'foo' and 2=3" << true << QVariant();
      QTest::newRow( "invalid or" ) << "'foo' or 2=3" << true << QVariant();
      QTest::newRow( "invalid not" ) << "not 'foo'" << true << QVariant();

      // in, not in
      QTest::newRow( "in 1" ) << "1 in (1,2,3)" << false << QVariant( 1 );
      QTest::newRow( "in 2" ) << "1 in (1,null,3)" << false << QVariant( 1 );
      QTest::newRow( "in 3" ) << "1 in (null,2,3)" << false << QVariant();
      QTest::newRow( "in 4" ) << "null in (1,2,3)" << false << QVariant();
      QTest::newRow( "not in 1" ) << "1 not in (1,2,3)" << false << QVariant( 0 );
      QTest::newRow( "not in 2" ) << "1 not in (1,null,3)" << false << QVariant( 0 );
      QTest::newRow( "not in 3" ) << "1 not in (null,2,3)" << false << QVariant();
      QTest::newRow( "not in 4" ) << "null not in (1,2,3)" << false << QVariant();

      // regexp, like
      QTest::newRow( "like 1" ) << "'hello' like '%ll_'" << false << QVariant( 1 );
      QTest::newRow( "like 2" ) << "'hello' like 'lo'" << false << QVariant( 0 );
      QTest::newRow( "like 3" ) << "'hello' like '%LO'" << false << QVariant( 0 );
      QTest::newRow( "ilike" ) << "'hello' ilike '%LO'" << false << QVariant( 1 );
      QTest::newRow( "regexp 1" ) << "'hello' ~ 'll'" << false << QVariant( 1 );
      QTest::newRow( "regexp 2" ) << "'hello' ~ '^ll'" << false << QVariant( 0 );
      QTest::newRow( "regexp 3" ) << "'hello' ~ 'llo$'" << false << QVariant( 1 );

      // concatenation
      QTest::newRow( "concat" ) << "'a' || 'b'" << false << QVariant( "ab" );
      QTest::newRow( "concat with int" ) << "'a' || 1" << false << QVariant( "a1" );
      QTest::newRow( "concat with int" ) << "2 || 'b'" << false << QVariant( "2b" );
      QTest::newRow( "concat with null" ) << "'a' || null" << false << QVariant();
      QTest::newRow( "concat numbers" ) << "1 || 2" << false << QVariant( "12" );

      // math functions
      QTest::newRow( "sqrt" ) << "sqrt(16)" << false << QVariant( 4. );
      QTest::newRow( "invalid sqrt value" ) << "sqrt('a')" << true << QVariant();
      QTest::newRow( "sin 0" ) << "sin(0)" << false << QVariant( 0. );
      QTest::newRow( "cos 0" ) << "cos(0)" << false << QVariant( 1. );
      QTest::newRow( "tan 0" ) << "tan(0)" << false << QVariant( 0. );
      QTest::newRow( "asin 0" ) << "asin(0)" << false << QVariant( 0. );
      QTest::newRow( "acos 1" ) << "acos(1)" << false << QVariant( 0. );
      QTest::newRow( "atan 0" ) << "atan(0)" << false << QVariant( 0. );
      QTest::newRow( "atan2(0,1)" ) << "atan2(0,1)" << false << QVariant( 0. );
      QTest::newRow( "atan2(1,0)" ) << "atan2(1,0)" << false << QVariant( M_PI / 2 );
      QTest::newRow( "exp(0)" ) << "exp(0)" << false << QVariant( 1. );
      QTest::newRow( "exp(1)" ) << "exp(1)" << false << QVariant( exp( 1. ) );
      QTest::newRow( "ln(0)" ) << "ln(0)" << false << QVariant();
      QTest::newRow( "log10(-1)" ) << "log10(-1)" << false << QVariant();
      QTest::newRow( "ln(1)" ) << "ln(1)" << false << QVariant( log( 1. ) );
      QTest::newRow( "log10(100)" ) << "log10(100)" << false << QVariant( 2. );
      QTest::newRow( "log(2,32)" ) << "log(2,32)" << false << QVariant( 5. );
      QTest::newRow( "log(10,1000)" ) << "log(10,1000)" << false << QVariant( 3. );
      QTest::newRow( "log(-2,32)" ) << "log(-2,32)" << false << QVariant( );
      QTest::newRow( "log(2,-32)" ) << "log(2,-32)" << false << QVariant( );
      QTest::newRow( "log(0.5,32)" ) << "log(0.5,32)" << false << QVariant( -5. );
      QTest::newRow( "round(1234.557,2) - round up" ) << "round(1234.557,2)" << false << QVariant( 1234.56 );
      QTest::newRow( "round(1234.554,2) - round down" ) << "round(1234.554,2)" << false << QVariant( 1234.55 );
      QTest::newRow( "round(1234.6) - round up to int" ) << "round(1234.6)" << false << QVariant( 1235 );
      QTest::newRow( "round(1234.6) - round down to int" ) << "round(1234.4)" << false << QVariant( 1234 );

      // cast functions
      QTest::newRow( "double to int" ) << "toint(3.2)" << false << QVariant( 3 );
      QTest::newRow( "text to int" ) << "toint('53')" << false << QVariant( 53 );
      QTest::newRow( "null to int" ) << "toint(null)" << false << QVariant();
      QTest::newRow( "int to double" ) << "toreal(3)" << false << QVariant( 3. );
      QTest::newRow( "text to double" ) << "toreal('53.1')" << false << QVariant( 53.1 );
      QTest::newRow( "null to double" ) << "toreal(null)" << false << QVariant();
      QTest::newRow( "int to text" ) << "tostring(6)" << false << QVariant( "6" );
      QTest::newRow( "double to text" ) << "tostring(1.23)" << false << QVariant( "1.23" );
      QTest::newRow( "null to text" ) << "tostring(null)" << false << QVariant();

      // string functions
      QTest::newRow( "lower" ) << "lower('HeLLo')" << false << QVariant( "hello" );
      QTest::newRow( "upper" ) << "upper('HeLLo')" << false << QVariant( "HELLO" );
      QTest::newRow( "length" ) << "length('HeLLo')" << false << QVariant( 5 );
      QTest::newRow( "replace" ) << "replace('HeLLo', 'LL', 'xx')" << false << QVariant( "Hexxo" );
      QTest::newRow( "regexp_replace" ) << "regexp_replace('HeLLo','[eL]+', '-')" << false << QVariant( "H-o" );
      QTest::newRow( "regexp_replace invalid" ) << "regexp_replace('HeLLo','[[[', '-')" << true << QVariant();
      QTest::newRow( "substr" ) << "substr('HeLLo', 3,2)" << false << QVariant( "LL" );
      QTest::newRow( "substr outside" ) << "substr('HeLLo', -5,2)" << false << QVariant( "" );
      QTest::newRow( "strpos" ) << "strpos('Hello World','World')" << false << QVariant( 6 );
      QTest::newRow( "strpos outside" ) << "strpos('Hello World','blah')" << false << QVariant( -1 );
      QTest::newRow( "left" ) << "left('Hello World',5)" << false << QVariant( "Hello" );
      QTest::newRow( "right" ) << "right('Hello World', 5)" << false << QVariant( "World" );
      QTest::newRow( "rpad" ) << "rpad('Hello', 10, 'x')" << false << QVariant( "xxxxxHello" );
      QTest::newRow( "rpad truncate" ) << "rpad('Hello', 4, 'x')" << false << QVariant( "Hell" );
      QTest::newRow( "lpad" ) << "lpad('Hello', 10, 'x')" << false << QVariant( "Helloxxxxx" );
      QTest::newRow( "lpad truncate" ) << "rpad('Hello', 4, 'x')" << false << QVariant( "Hell" );
      QTest::newRow( "title" ) << "title(' HeLlO   WORLD ')" << false << QVariant( " Hello   World " );
      QTest::newRow( "format" ) << "format('%1 %2 %3 %1', 'One', 'Two', 'Three')" << false << QVariant( "One Two Three One" );

      // implicit conversions
      QTest::newRow( "implicit int->text" ) << "length(123)" << false << QVariant( 3 );
      QTest::newRow( "implicit double->text" ) << "length(1.23)" << false << QVariant( 4 );
      QTest::newRow( "implicit int->bool" ) << "1 or 0" << false << QVariant( 1 );
      QTest::newRow( "implicit double->bool" ) << "0.1 or 0" << false << QVariant( 1 );
      QTest::newRow( "implicit text->int" ) << "'5'+2" << false << QVariant( 7 );
      QTest::newRow( "implicit text->double" ) << "'5.1'+2" << false << QVariant( 7.1 );
      QTest::newRow( "implicit text->bool" ) << "'0.1' or 0" << false << QVariant( 1 );

      // conditions (without base expression, i.e. CASE WHEN ... THEN ... END)
      QTest::newRow( "condition when" ) << "case when 2>1 then 'good' end" << false << QVariant( "good" );
      QTest::newRow( "condition else" ) << "case when 1=0 then 'bad' else 678 end" << false << QVariant( 678 );
      QTest::newRow( "condition null" ) << "case when length(123)=0 then 111 end" << false << QVariant();
      QTest::newRow( "condition 2 when" ) << "case when 2>3 then 23 when 3>2 then 32 else 0 end" << false << QVariant( 32 );
      QTest::newRow( "coalesce null" ) << "coalesce(NULL)" << false << QVariant( );
      QTest::newRow( "coalesce mid-null" ) << "coalesce(1, NULL, 3)" << false << QVariant( 1 );
      QTest::newRow( "coalesce exp" ) << "coalesce(NULL, 1+1)" << false << QVariant( 2 );

      // Datetime functions
      QTest::newRow( "to date" ) << "todate('2012-06-28')" << false << QVariant( QDate( 2012, 6, 28 ) );
      QTest::newRow( "to interval" ) << "tointerval('1 Year 1 Month 1 Week 1 Hour 1 Minute')" << false << QVariant::fromValue( QgsExpression::Interval( 34758060 ) );
      QTest::newRow( "day with date" ) << "day('2012-06-28')" << false << QVariant( 28 );
      QTest::newRow( "day with interval" ) << "day(tointerval('28 days'))" << false << QVariant( 28.0 );
      QTest::newRow( "month with date" ) << "month('2012-06-28')" << false << QVariant( 6 );
      QTest::newRow( "month with interval" ) << "month(tointerval('2 months'))" << false << QVariant( 2.0 );
      QTest::newRow( "year with date" ) << "year('2012-06-28')" << false << QVariant( 2012 );
      QTest::newRow( "year with interval" ) << "year(tointerval('2 years'))" << false << QVariant( 2.0 );
      QTest::newRow( "age" ) << "age('2012-06-30','2012-06-28')" << false << QVariant::fromValue( QgsExpression::Interval( 172800 ) );
      QTest::newRow( "negative age" ) << "age('2012-06-28','2012-06-30')" << false << QVariant::fromValue( QgsExpression::Interval( -172800 ) );

      // Color functions
      QTest::newRow( "ramp color" ) << "ramp_color('Spectral',0.3)" << false << QVariant( "#fdbe73" );
      QTest::newRow( "color rgb" ) << "color_rgb(255,127,0)" << false << QVariant( "#ff7f00" );
      QTest::newRow( "color rgba" ) << "color_rgba(255,127,0,200)" << false << QVariant( "255,127,0,200" );
      QTest::newRow( "color hsl" ) << "color_hsl(100,50,70)" << false << QVariant( "#a6d98c" );
      QTest::newRow( "color hsla" ) << "color_hsla(100,50,70,200)" << false << QVariant( "166,217,140,200" );
      QTest::newRow( "color hsv" ) << "color_hsv(40,100,100)" << false << QVariant( "#ffaa00" );
      QTest::newRow( "color hsva" ) << "color_hsva(40,100,100,200)" << false << QVariant( "255,170,0,200" );
    }

    void evaluation()
    {
      QFETCH( QString, string );
      QFETCH( bool, evalError );
      QFETCH( QVariant, result );

      QgsExpression exp( string );
      QCOMPARE( exp.hasParserError(), false );
      if ( exp.hasParserError() )
        qDebug() << exp.parserErrorString();

      QVariant res = exp.evaluate();
      if ( exp.hasEvalError() )
        qDebug() << exp.evalErrorString();
      if ( res.type() != result.type() )
      {
        qDebug() << "got " << res.typeName() << " instead of " << result.typeName();
      }
      //qDebug() << res.type() << " " << result.type();
      //qDebug() << "type " << res.typeName();
      QCOMPARE( exp.hasEvalError(), evalError );

      QCOMPARE( res.type(), result.type() );
      switch ( res.type() )
      {
        case QVariant::Invalid:
          break; // nothing more to check
        case QVariant::Int:
          QCOMPARE( res.toInt(), result.toInt() );
          break;
        case QVariant::Double:
          QCOMPARE( res.toDouble(), result.toDouble() );
          break;
        case QVariant::String:
          QCOMPARE( res.toString(), result.toString() );
          break;
        case QVariant::Date:
          QCOMPARE( res.toDate(), result.toDate() );
          break;
        case QVariant::DateTime:
          QCOMPARE( res.toDateTime(), result.toDateTime() );
          break;
        case QVariant::Time:
          QCOMPARE( res.toTime(), result.toTime() );
          break;
        case QVariant::UserType:
        {
          if ( res.userType() == qMetaTypeId<QgsExpression::Interval>() )
          {
            QgsExpression::Interval inter = res.value<QgsExpression::Interval>();
            QgsExpression::Interval gotinter = result.value<QgsExpression::Interval>();
            QCOMPARE( inter.seconds(), gotinter.seconds() );
          }
          else
          {
            QFAIL( "unexpected user type" );
          }
          break;
        }
        default:
          Q_ASSERT( false ); // should never happen
      }
    }

    void eval_columns()
    {
      QgsFields fields;
      fields.append( QgsField( "x1" ) );
      fields.append( QgsField( "x2" ) );
      fields.append( QgsField( "foo", QVariant::Int ) );

      QgsFeature f;
      f.initAttributes( 3 );
      f.setAttribute( 2, QVariant( 20 ) );

      // good exp
      QgsExpression exp( "foo + 1" );
      bool prepareRes = exp.prepare( fields );
      QCOMPARE( prepareRes, true );
      QCOMPARE( exp.hasEvalError(), false );
      QVariant res = exp.evaluate( &f );
      QCOMPARE( res.type(), QVariant::Int );
      QCOMPARE( res.toInt(), 21 );

      // bad exp
      QgsExpression exp2( "bar + 1" );
      bool prepareRes2 = exp2.prepare( fields );
      QCOMPARE( prepareRes2, false );
      QCOMPARE( exp2.hasEvalError(), true );
      QVariant res2 = exp2.evaluate( &f );
      QCOMPARE( res2.type(), QVariant::Invalid );
    }

    void eval_rownum()
    {
      QgsExpression exp( "$rownum + 1" );
      QVariant v1 = exp.evaluate();
      QCOMPARE( v1.toInt(), 1 );

      exp.setCurrentRowNumber( 100 );
      QVariant v2 = exp.evaluate();
      QCOMPARE( v2.toInt(), 101 );
    }

    void eval_scale()
    {
      QgsExpression exp( "$scale" );
      QVariant v1 = exp.evaluate();
      QCOMPARE( v1.toInt(), 0 );

      exp.setScale( 100.00 );
      QVariant v2 = exp.evaluate();
      QCOMPARE( v2.toDouble(), 100.00 );
    }

    void eval_feature_id()
    {
      QgsFeature f( 100 );
      QgsExpression exp( "$id * 2" );
      QVariant v = exp.evaluate( &f );
      QCOMPARE( v.toInt(), 200 );
    }

    void referenced_columns()
    {
      QSet<QString> expectedCols;
      expectedCols << "foo" << "bar" << "ppp" << "qqq" << "rrr";
      QgsExpression exp( "length(Bar || FOO) = 4 or foo + sqrt(bar) > 0 or case when ppp then qqq else rrr end" );
      QCOMPARE( exp.hasParserError(), false );
      QStringList refCols = exp.referencedColumns();
      // make sure we have lower case
      QSet<QString> refColsSet;
      foreach ( QString col, refCols )
        refColsSet.insert( col.toLower() );

      QCOMPARE( refColsSet, expectedCols );
    }

    void needs_geometry_data()
    {
      QTest::addColumn<QString>( "string" );
      QTest::addColumn<bool>( "needsGeom" );

      // literal evaluation
      QTest::newRow( "x > 0" ) << "x > 0" << false;
      QTest::newRow( "1 = 1" ) << "1 = 1" << false;
      QTest::newRow( "$x > 0" ) << "$x > 0" << true;
      QTest::newRow( "xat(0) > 0" ) << "xat(0) > 0" << true;
      QTest::newRow( "$x" ) << "$x" << true;
      QTest::newRow( "$area" ) << "$area" << true;
      QTest::newRow( "$length" ) << "$length" << true;
      QTest::newRow( "$perimeter" ) << "$perimeter" << true;
      QTest::newRow( "toint($perimeter)" ) << "toint($perimeter)" << true;
      QTest::newRow( "toint(123)" ) << "toint(123)" << false;
      QTest::newRow( "case 0" ) << "case when 1 then 0 end" << false;
      QTest::newRow( "case 1" ) << "case when $area > 0 then 1 end" << true;
      QTest::newRow( "case 2" ) << "case when 1 then $area end" << true;
      QTest::newRow( "case 3" ) << "case when 1 then 0 else $area end" << true;
    }

    void needs_geometry()
    {
      QFETCH( QString, string );
      QFETCH( bool, needsGeom );

      QgsExpression exp( string );
      if ( exp.hasParserError() )
        qDebug() << "parser error! " << exp.parserErrorString();
      QCOMPARE( exp.hasParserError(), false );
      QCOMPARE( exp.needsGeometry(), needsGeom );
    }

    void eval_geometry_data()
    {
      QTest::addColumn<QString>( "string" );
      QTest::addColumn<void*>( "geomptr" );
      QTest::addColumn<bool>( "evalError" );
      QTest::addColumn<double>( "result" );

      QgsPoint point( 123, 456 );
      QgsPolyline line;
      line << QgsPoint( 1, 1 ) << QgsPoint( 4, 2 ) << QgsPoint( 3, 1 );

      QTest::newRow( "geom x" ) << "$x" << ( void* ) QgsGeometry::fromPoint( point ) << false << 123.;
      QTest::newRow( "geom y" ) << "$y" << ( void* ) QgsGeometry::fromPoint( point ) << false << 456.;
      QTest::newRow( "geom xat" ) << "xat(-1)" << ( void* ) QgsGeometry::fromPolyline( line ) << false << 3.;
      QTest::newRow( "geom yat" ) << "yat(1)" << ( void* ) QgsGeometry::fromPolyline( line ) << false << 2.;
    }

    void eval_geometry()
    {
      QFETCH( QString, string );
      QFETCH( void*, geomptr );
      QFETCH( bool, evalError );
      QFETCH( double, result );

      QgsGeometry* geom = ( QgsGeometry* ) geomptr;

      QgsFeature f;
      f.setGeometry( geom );

      QgsExpression exp( string );
      QCOMPARE( exp.hasParserError(), false );
      QCOMPARE( exp.needsGeometry(), true );
      QVariant out = exp.evaluate( &f );
      QCOMPARE( exp.hasEvalError(), evalError );
      QCOMPARE( out.toDouble(), result );
    }

    void eval_geometry_calc()
    {
      QgsPolyline polyline, polygon_ring;
      polyline << QgsPoint( 0, 0 ) << QgsPoint( 10, 0 );
      polygon_ring << QgsPoint( 1, 1 ) << QgsPoint( 6, 1 ) << QgsPoint( 6, 6 ) << QgsPoint( 1, 6 ) << QgsPoint( 1, 1 );
      QgsPolygon polygon;
      polygon << polygon_ring;
      QgsFeature fPolygon, fPolyline;
      fPolyline.setGeometry( QgsGeometry::fromPolyline( polyline ) );
      fPolygon.setGeometry( QgsGeometry::fromPolygon( polygon ) );

      QgsExpression exp1( "$area" );
      QVariant vArea = exp1.evaluate( &fPolygon );
      QCOMPARE( vArea.toDouble(), 25. );

      QgsExpression exp2( "$length" );
      QVariant vLength = exp2.evaluate( &fPolyline );
      QCOMPARE( vLength.toDouble(), 10. );

      QgsExpression exp3( "$perimeter" );
      QVariant vPerimeter = exp3.evaluate( &fPolygon );
      QCOMPARE( vPerimeter.toDouble(), 20. );
    }

    void eval_geometry_constructor_data()
    {
      QTest::addColumn<QString>( "string" );
      QTest::addColumn<void*>( "geomptr" );
      QTest::addColumn<bool>( "evalError" );

      QgsPoint point( 123, 456 );
      QgsPolyline line;
      line << QgsPoint( 1, 1 ) << QgsPoint( 4, 2 ) << QgsPoint( 3, 1 );

      QgsPolyline polyline, polygon_ring;
      polyline << QgsPoint( 0, 0 ) << QgsPoint( 10, 0 );
      polygon_ring << QgsPoint( 1, 1 ) << QgsPoint( 6, 1 ) << QgsPoint( 6, 6 ) << QgsPoint( 1, 6 ) << QgsPoint( 1, 1 );
      QgsPolygon polygon;
      polygon << polygon_ring;

      QTest::newRow( "geomFromWKT Point" ) << "geomFromWKT('" + QgsGeometry::fromPoint( point )->exportToWkt() + "')" << ( void* ) QgsGeometry::fromPoint( point ) << false;
      QTest::newRow( "geomFromWKT Line" ) << "geomFromWKT('" + QgsGeometry::fromPolyline( line )->exportToWkt() + "')" << ( void* ) QgsGeometry::fromPolyline( line ) << false;
      QTest::newRow( "geomFromWKT Polyline" ) << "geomFromWKT('" + QgsGeometry::fromPolyline( polyline )->exportToWkt() + "')" << ( void* ) QgsGeometry::fromPolyline( polyline ) << false;
      QTest::newRow( "geomFromWKT Polygon" ) << "geomFromWKT('" + QgsGeometry::fromPolygon( polygon )->exportToWkt() + "')" << ( void* ) QgsGeometry::fromPolygon( polygon ) << false;

      // GML Point
      QTest::newRow( "GML Point (coordinates)" ) << "geomFromGML('<gml:Point><gml:coordinates>123,456</gml:coordinates></gml:Point>')" << ( void * ) QgsGeometry::fromPoint( point ) << false;
      // gml:pos if from GML3
      QTest::newRow( "GML Point (pos)" ) << "geomFromGML('<gml:Point srsName=\"foo\"><gml:pos srsDimension=\"2\">123 456</gml:pos></gml:Point>')" << ( void * ) QgsGeometry::fromPoint( point ) << false;

      // GML Box
      QgsRectangle rect( 135.2239, 34.4879, 135.8578, 34.8471 );
      QTest::newRow( "GML Box" ) << "geomFromGML('<gml:Box srsName=\"foo\"><gml:coordinates>135.2239,34.4879 135.8578,34.8471</gml:coordinates></gml:Box>')" << ( void * ) QgsGeometry::fromRect( rect ) << false;
      // Envelope is from GML3 ?
      QTest::newRow( "GML Envelope" ) << "geomFromGML('<gml:Envelope>"
      "<gml:lowerCorner>135.2239 34.4879</gml:lowerCorner>"
      "<gml:upperCorner>135.8578 34.8471</gml:upperCorner>"
      "</gml:Envelope>')" << ( void * ) QgsGeometry::fromRect( rect ) << false;
    }

    void eval_geometry_constructor()
    {
      QFETCH( QString, string );
      QFETCH( void*, geomptr );
      QFETCH( bool, evalError );

      QgsGeometry* geom = ( QgsGeometry* ) geomptr;

      QgsFeature f;
      f.setGeometry( geom );

      QgsExpression exp( string );
      QCOMPARE( exp.hasParserError(), false );
      QCOMPARE( exp.needsGeometry(), false );
      QVariant out = exp.evaluate( &f );
      QCOMPARE( exp.hasEvalError(), evalError );

      QCOMPARE( out.canConvert<QgsGeometry>(), true );
      QgsGeometry outGeom = out.value<QgsGeometry>();
      QCOMPARE( geom->equals( &outGeom ), true );
    }

    void eval_spatial_operator_data()
    {
      QTest::addColumn<QString>( "string" );
      QTest::addColumn<void*>( "geomptr" );
      QTest::addColumn<bool>( "evalError" );
      QTest::addColumn<QVariant>( "result" );

      QgsPoint point( 0, 0 );
      QgsPolyline line, polygon_ring;
      line << QgsPoint( 0, 0 ) << QgsPoint( 10, 10 );
      polygon_ring << QgsPoint( 0, 0 ) << QgsPoint( 10, 10 ) << QgsPoint( 10, 0 ) << QgsPoint( 0, 0 );
      QgsPolygon polygon;
      polygon << polygon_ring;

      QTest::newRow( "No Intersects" ) << "intersects( $geometry, geomFromWKT('LINESTRING ( 2 0, 0 2 )') )" << ( void* ) QgsGeometry::fromPoint( point ) << false << QVariant( 0 );
      QTest::newRow( "Intersects" ) << "intersects( $geometry, geomFromWKT('LINESTRING ( 0 0, 0 2 )') )" << ( void* ) QgsGeometry::fromPoint( point ) << false << QVariant( 1 );
      QTest::newRow( "No Disjoint" ) << "disjoint( $geometry, geomFromWKT('LINESTRING ( 0 0, 0 2 )') )" << ( void* ) QgsGeometry::fromPoint( point ) << false << QVariant( 0 );
      QTest::newRow( "Disjoint" ) << "disjoint( $geometry, geomFromWKT('LINESTRING ( 2 0, 0 2 )') )" << ( void* ) QgsGeometry::fromPoint( point ) << false << QVariant( 1 );

      // OGR test
      QTest::newRow( "OGR Intersects" ) << "intersects( $geometry, geomFromWKT('LINESTRING ( 10 0, 0 10 )') )" << ( void* ) QgsGeometry::fromPolyline( line ) << false << QVariant( 1 );
      QTest::newRow( "OGR no Intersects" ) << "intersects( $geometry, geomFromWKT('POLYGON((20 20, 20 30, 30 20, 20 20))') )" << ( void* ) QgsGeometry::fromPolyline( line ) << false << QVariant( 0 );
      QTest::newRow( "OGR no Disjoint" ) << "disjoint( $geometry, geomFromWKT('LINESTRING ( 10 0, 0 10 )') )" << ( void* ) QgsGeometry::fromPolyline( line ) << false << QVariant( 0 );
      QTest::newRow( "OGR Disjoint" ) << "disjoint( $geometry, geomFromWKT('POLYGON((20 20, 20 30, 30 20, 20 20))') )" << ( void* ) QgsGeometry::fromPolyline( line ) << false << QVariant( 1 );
      QTest::newRow( "OGR Touches" ) << "touches( $geometry, geomFromWKT('LINESTRING ( 0 0, 0 10 )') )" << ( void* ) QgsGeometry::fromPolyline( line ) << false << QVariant( 1 );
      QTest::newRow( "OGR no Touches" ) << "touches( $geometry, geomFromWKT('POLYGON((20 20, 20 30, 30 20, 20 20))') )" << ( void* ) QgsGeometry::fromPolyline( line ) << false << QVariant( 0 );
      QTest::newRow( "OGR Crosses" ) << "crosses( $geometry, geomFromWKT('LINESTRING ( 10 0, 0 10 )') )" << ( void* ) QgsGeometry::fromPolyline( line ) << false << QVariant( 1 );
      QTest::newRow( "OGR no Crosses" ) << "crosses( $geometry, geomFromWKT('LINESTRING ( 0 0, 0 10 )') )" << ( void* ) QgsGeometry::fromPolyline( line ) << false << QVariant( 0 );
      QTest::newRow( "OGR Within" ) << "within( $geometry, geomFromWKT('POLYGON((-90 -90, -90 90, 190 -90, -90 -90))') )" << ( void* ) QgsGeometry::fromPolygon( polygon ) << false << QVariant( 1 );
      QTest::newRow( "OGR no Within" ) << "within( geomFromWKT('POLYGON((-90 -90, -90 90, 190 -90, -90 -90))'), $geometry )" << ( void* ) QgsGeometry::fromPolygon( polygon ) << false << QVariant( 0 );
      QTest::newRow( "OGR Contians" ) << "contains( geomFromWKT('POLYGON((-90 -90, -90 90, 190 -90, -90 -90))'), $geometry )" << ( void* ) QgsGeometry::fromPolygon( polygon ) << false << QVariant( 1 );
      QTest::newRow( "OGR no Contains" ) << "contains( $geometry, geomFromWKT('POLYGON((-90 -90, -90 90, 190 -90, -90 -90))') )" << ( void* ) QgsGeometry::fromPolygon( polygon ) << false << QVariant( 0 );
      QTest::newRow( "OGR no Overlaps" ) << "overlaps( geomFromWKT('POLYGON((-90 -90, -90 90, 190 -90, -90 -90))'), $geometry )" << ( void* ) QgsGeometry::fromPolygon( polygon ) << false << QVariant( 0 );
      QTest::newRow( "OGR overlaps" ) << "overlaps( geomFromWKT('POLYGON((0 -5,10 5,10 -5,0 -5))'), $geometry )" << ( void* ) QgsGeometry::fromPolygon( polygon ) << false << QVariant( 1 );
    }

    void eval_spatial_operator()
    {
      QFETCH( QString, string );
      QFETCH( void*, geomptr );
      QFETCH( bool, evalError );
      QFETCH( QVariant, result );

      QgsGeometry* geom = ( QgsGeometry* ) geomptr;

      QgsFeature f;
      f.setGeometry( geom );

      QgsExpression exp( string );
      QCOMPARE( exp.hasParserError(), false );
      QCOMPARE( exp.needsGeometry(), true );
      QVariant out = exp.evaluate( &f );
      QCOMPARE( exp.hasEvalError(), evalError );
      QCOMPARE( out.toInt(), result.toInt() );
    }

    void eval_geometry_method_data()
    {
      QTest::addColumn<QString>( "string" );
      QTest::addColumn<void*>( "geomptr" );
      QTest::addColumn<bool>( "evalError" );
      QTest::addColumn<bool>( "needGeom" );
      QTest::addColumn<void*>( "resultptr" );

      QgsPolyline polygon_ring;
      polygon_ring << QgsPoint( 0, 0 ) << QgsPoint( 10, 10 ) << QgsPoint( 10, 0 ) << QgsPoint( 0, 0 );
      QgsPolygon polygon;
      polygon << polygon_ring;

      QgsGeometry* geom = QgsGeometry::fromPolygon( polygon );

      QTest::newRow( "buffer" ) << "buffer( $geometry, 1.0, 3)" << ( void* ) geom << false << true << ( void* ) geom->buffer( 1.0, 3 );
      geom = QgsGeometry::fromPolygon( polygon );
      QTest::newRow( "buffer" ) << "buffer( $geometry, 2.0)" << ( void* ) geom << false << true << ( void* ) geom->buffer( 2.0, 8 );

      QgsPoint point1( 10, 20 );
      QgsPoint point2( 30, 20 );
      QgsGeometry* pnt1 = QgsGeometry::fromPoint( point1 );
      QgsGeometry* pnt2 = QgsGeometry::fromPoint( point2 );
      QTest::newRow( "union" ) << "union( $geometry, geomFromWKT('" + pnt2->exportToWkt() + "') )" << ( void* ) pnt1 << false << true << ( void* ) pnt1->combine( pnt2 );

      geom = QgsGeometry::fromPolygon( polygon );
      QTest::newRow( "intersection" ) << "intersection( $geometry, geomFromWKT('POLYGON((0 0, 0 10, 10 0, 0 0))') )" << ( void* ) geom << false << true << ( void* ) QgsGeometry::fromWkt( "POLYGON ((0 0,5 5,10 0,0 0))" );
      geom = QgsGeometry::fromPolygon( polygon );
      QTest::newRow( "difference" ) << "difference( $geometry, geomFromWKT('POLYGON((0 0, 0 10, 10 0, 0 0))') )" << ( void* ) geom << false << true << ( void* ) QgsGeometry::fromWkt( "POLYGON ((5 5,10 10,10 0,5 5))" );
      geom = QgsGeometry::fromPolygon( polygon );
      QTest::newRow( "symDifference" ) << "symDifference( $geometry, geomFromWKT('POLYGON((0 0, 0 10, 10 0, 0 0))') )" << ( void* ) geom << false << true << ( void* ) QgsGeometry::fromWkt( "MULTIPOLYGON(((5 5,0 0,0 10,5 5)),((5 5,10 10,10 0,5 5)))" );

      geom = QgsGeometry::fromPolygon( polygon );
      QTest::newRow( "centroid polygon" ) << "centroid( $geometry )" << ( void* ) geom << false << true << ( void* ) geom->centroid();
      geom = QgsGeometry::fromPolygon( polygon );
      QTest::newRow( "centroid self intersecting polygon" ) << "centroid( geomFromWKT('POLYGON((0 0, 0 2, 2 -0.1, 2 2.1, 0 0))') )" << ( void* ) geom << false << false << ( void* ) QgsGeometry::fromWkt( "POINT (8.0 1.0)" );
      geom = QgsGeometry::fromPolygon( polygon );
      QTest::newRow( "centroid multi polygon" ) << "centroid( geomFromWKT('MULTIPOLYGON(((0 0,0 1,1 1,1 0,0 0)),((2 0,2 1,3 1,3 0,2 0)))') )" << ( void* ) geom << false << false << ( void* ) QgsGeometry::fromWkt( "POINT (1.5 0.5)" );
      geom = QgsGeometry::fromPolygon( polygon );
      QTest::newRow( "convexHull simple" ) << "convexHull( $geometry )" << ( void* ) geom << false << true << ( void* ) geom->convexHull();
      geom = QgsGeometry::fromPolygon( polygon );
      QTest::newRow( "convexHull multi" ) << "convexHull( geomFromWKT('GEOMETRYCOLLECTION(POINT(0 1), POINT(0 0), POINT(1 0), POINT(1 1))') )" << ( void* ) geom << false << false << ( void* ) QgsGeometry::fromWkt( "POLYGON ((0 0,0 1,1 1,1 0,0 0))" );
    }

    void eval_geometry_method()
    {
      QFETCH( QString, string );
      QFETCH( void*, geomptr );
      QFETCH( bool, evalError );
      QFETCH( bool, needGeom );
      QFETCH( void*, resultptr );

      QgsGeometry* geom = ( QgsGeometry* ) geomptr;
      QgsGeometry* result = ( QgsGeometry* ) resultptr;

      QgsFeature f;
      f.setGeometry( geom );

      QgsExpression exp( string );
      QCOMPARE( exp.hasParserError(), false );
      QCOMPARE( exp.needsGeometry(), needGeom );
      QVariant out = exp.evaluate( &f );
      QCOMPARE( exp.hasEvalError(), evalError );

      QCOMPARE( out.canConvert<QgsGeometry>(), true );
      QgsGeometry outGeom = out.value<QgsGeometry>();
      QCOMPARE( outGeom.exportToWkt(), result->exportToWkt() );
    }

    void eval_special_columns()
    {
      QTest::addColumn<QString>( "string" );
      QTest::addColumn<QVariant>( "result" );

      QgsExpression::setSpecialColumn( "$var1", QVariant(( int )42 ) );

      QgsExpression exp( "$var1 + 1" );
      QVariant v1 = exp.evaluate();
      QCOMPARE( v1.toInt(), 43 );

      QgsExpression::setSpecialColumn( "$var1", QVariant(( int )100 ) );
      QVariant v2 = exp.evaluate();
      QCOMPARE( v2.toInt(), 101 );

      QgsExpression exp2( "_specialcol_('$var1')+1" );
      QVariant v3 = exp2.evaluate();
      QCOMPARE( v3.toInt(), 101 );

      QgsExpression exp3( "_specialcol_('undefined')" );
      QVariant v4 = exp3.evaluate();
      QCOMPARE( v4, QVariant() );

      QgsExpression::unsetSpecialColumn( "$var1" );
    }

};

QTEST_MAIN( TestQgsExpression )

#include "moc_testqgsexpression.cxx"

