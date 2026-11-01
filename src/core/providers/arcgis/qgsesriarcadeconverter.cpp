/***************************************************************************
    qgsesriarcadeconverter.h
    --------------------
    begin                : October 2026
    copyright            : (C) 2026 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsesriarcadeconverter.h"

#include "qgsesriarcadescript.h"
#include "qgsexception.h"
#include "qgsexpression.h"

#include <QString>

using namespace Qt::StringLiterals;


class ArcadeToExpressionVisitor : public QgsEsriArcadeRecursiveVisitor
{
  public:
    QgsEsriArcadeConverter::Result mResult = QgsEsriArcadeConverter::Result::Success;

    static const QString NEXT_BLOCK_MARKER;

    QString mCurrentPart;
    bool mCurrentPartIsLiteral = false;
    bool mCurrentPartIsSimple = false;

    void visit( const QgsEsriArcadeNode &node )
    {
      if ( auto block = dynamic_cast< const QgsEsriArcadeBlockStatement * >( &node ) )
      {
        visit( *block );
      }
      else if ( auto varDeclare = dynamic_cast< const QgsEsriArcadeVariableDeclarationStatement * >( &node ) )
      {
        visit( *varDeclare );
      }
      else if ( auto varAssign = dynamic_cast< const QgsEsriArcadeAssignmentStatement * >( &node ) )
      {
        visit( *varAssign );
      }
      else if ( auto returnStatement = dynamic_cast< const QgsEsriArcadeReturnStatement * >( &node ) )
      {
        visit( *returnStatement );
      }
      else if ( auto functionDef = dynamic_cast< const QgsEsriArcadeFunctionDeclarationStatement * >( &node ) )
      {
        visit( *functionDef );
      }
      else if ( auto expression = dynamic_cast< const QgsEsriArcadeExpressionStatement * >( &node ) )
      {
        visit( *expression );
      }
      else if ( auto literal = dynamic_cast< const QgsEsriArcadeLiteralExpression * >( &node ) )
      {
        visit( *literal );
      }
      else if ( auto variableReference = dynamic_cast< const QgsEsriArcadeVariableReferenceExpression * >( &node ) )
      {
        visit( *variableReference );
      }
      else if ( auto profileVariableReference = dynamic_cast< const QgsEsriArcadeProfileVariableReferenceExpression * >( &node ) )
      {
        visit( *profileVariableReference );
      }
      else if ( auto memberAccess = dynamic_cast< const QgsEsriArcadeMemberAccessExpression * >( &node ) )
      {
        visit( *memberAccess );
      }
      else if ( auto binaryOperator = dynamic_cast< const QgsEsriArcadeBinaryOperatorExpression * >( &node ) )
      {
        visit( *binaryOperator );
      }
      else if ( auto unaryOperator = dynamic_cast< const QgsEsriArcadeUnaryOperatorExpression * >( &node ) )
      {
        visit( *unaryOperator );
      }
      else if ( auto functionCall = dynamic_cast< const QgsEsriArcadeFunctionCallExpression * >( &node ) )
      {
        visit( *functionCall );
      }
      else if ( auto index = dynamic_cast< const QgsEsriArcadeIndexExpression * >( &node ) )
      {
        visit( *index );
      }
      else if ( auto arrayLiteral = dynamic_cast< const QgsEsriArcadeArrayLiteralExpression * >( &node ) )
      {
        visit( *arrayLiteral );
      }
      else if ( auto dictionaryLiteral = dynamic_cast< const QgsEsriArcadeDictionaryLiteralExpression * >( &node ) )
      {
        visit( *dictionaryLiteral );
      }
      else if ( auto ifStatement = dynamic_cast< const QgsEsriArcadeIfStatement * >( &node ) )
      {
        visit( *ifStatement );
      }
      else if ( auto forStatement = dynamic_cast< const QgsEsriArcadeForStatement * >( &node ) )
      {
        visit( *forStatement );
      }
      else if ( auto forInStatement = dynamic_cast< const QgsEsriArcadeForInStatement * >( &node ) )
      {
        visit( *forInStatement );
      }
      else if ( auto whileStatement = dynamic_cast< const QgsEsriArcadeWhileStatement * >( &node ) )
      {
        visit( *whileStatement );
      }
      else if ( auto breakStatement = dynamic_cast< const QgsEsriArcadeBreakStatement * >( &node ) )
      {
        visit( *breakStatement );
      }
      else if ( auto continueStatement = dynamic_cast< const QgsEsriArcadeContinueStatement * >( &node ) )
      {
        visit( *continueStatement );
      }
    }

    void visit( const QgsEsriArcadeBlockStatement &node ) override
    {
      if ( node.statements().size() > 1 )
      {
        mResult = QgsEsriArcadeConverter::Result::InvalidScriptMultiStatements;
        throw QgsNotSupportedException( "" );
      }

      QgsEsriArcadeStatement *firstStatement = node.statements().at( 0 );
      visit( *firstStatement );
    }

    void visit( const QgsEsriArcadeVariableDeclarationStatement & ) override
    {
      mResult = QgsEsriArcadeConverter::Result::InvalidScriptIncompatible;
      throw QgsNotSupportedException( QObject::tr( "Scripts containing variable declarations cannot be converted to QGIS expressions." ) );
    }

    void visit( const QgsEsriArcadeAssignmentStatement & ) override
    {
      mResult = QgsEsriArcadeConverter::Result::InvalidScriptIncompatible;
      throw QgsNotSupportedException( QObject::tr( "Scripts containing variable assignments cannot be converted to QGIS expressions." ) );
    }

    void visit( const QgsEsriArcadeFunctionDeclarationStatement & ) override
    {
      mResult = QgsEsriArcadeConverter::Result::InvalidScriptIncompatible;
      throw QgsNotSupportedException( QObject::tr( "Scripts containing function definitions cannot be converted to QGIS expressions." ) );
    }

    void visit( const QgsEsriArcadeReturnStatement &node ) override { visit( *node.expression() ); }

    void visit( const QgsEsriArcadeExpressionStatement &node ) override { visit( *node.expression() ); }

    void visit( const QgsEsriArcadeLiteralExpression &node ) override
    {
      const QVariant value = node.value();
      if ( value.toString() == "\n" )
      {
        mCurrentPart = u"'\\n'"_s;
        mCurrentPartIsLiteral = false;
      }
      else if ( value.type() == QVariant::String )
      {
        mCurrentPart = QgsExpression::quotedString( value.toString() );
        mCurrentPartIsLiteral = true;
      }
      else if ( QgsVariantUtils::isNull( value ) )
      {
        mCurrentPart = u"NULL"_s;
        mCurrentPartIsLiteral = false;
      }
      else
      {
        mCurrentPart = value.toString();
        mCurrentPartIsLiteral = true;
      }
      mCurrentPartIsSimple = true;
    }

    void visit( const QgsEsriArcadeVariableReferenceExpression &node ) override
    {
      mCurrentPart = u"@%1"_s.arg( node.name() );
      mCurrentPartIsLiteral = false;
      mCurrentPartIsSimple = true;
    }

    void visit( const QgsEsriArcadeProfileVariableReferenceExpression &node ) override
    {
      const QString variableName = node.name();
      if ( variableName.compare( "$feature"_L1, Qt::CaseInsensitive ) == 0 )
      {
        mCurrentPart = u"@feature"_s;
        mCurrentPartIsLiteral = false;
        mCurrentPartIsSimple = true;
      }
      else if ( variableName.compare( "$view"_L1, Qt::CaseInsensitive ) == 0 )
      {
        // this will get caught in QgsEsriArcadeMemberAccessExpression visitor and translated to @map_ variables
        mCurrentPart = variableName;
        mCurrentPartIsLiteral = false;
        mCurrentPartIsSimple = true;
      }
      else
      {
        mResult = QgsEsriArcadeConverter::Result::UnknownError;
        throw QgsNotSupportedException( QObject::tr( "Converting the %1 profile variable is not supported" ).arg( variableName ) );
      }
    }

    void visit( const QgsEsriArcadeMemberAccessExpression &node ) override
    {
      visit( *node.containerExpression() );
      if ( mCurrentPart.compare( "@feature"_L1, Qt::CaseInsensitive ) == 0 )
      {
        mCurrentPart = QgsExpression::quotedColumnRef( node.memberName() );
        mCurrentPartIsLiteral = false;
        mCurrentPartIsSimple = true;
      }
      else if ( mCurrentPart.compare( "$view"_L1, Qt::CaseInsensitive ) == 0 )
      {
        static const QMap< QString, QString > sVIEW_VARS {
          { u"scale"_s, u"@map_scale"_s },
        };
        auto it = sVIEW_VARS.constFind( node.memberName().toLower() );
        if ( it != sVIEW_VARS.constEnd() )
        {
          mCurrentPart = it.value();
          mCurrentPartIsLiteral = false;
          mCurrentPartIsSimple = true;
        }
        else
        {
          mResult = QgsEsriArcadeConverter::Result::UnknownError;
          throw QgsNotSupportedException( QObject::tr( "Converting the %1 view member is not supported" ).arg( node.memberName() ) );
        }
      }
      else
      {
        mResult = QgsEsriArcadeConverter::Result::UnknownError;
        throw QgsNotSupportedException( QObject::tr( "Converting the member access operator in this expression is not supported" ) );
      }
    }

    void visit( const QgsEsriArcadeBinaryOperatorExpression &node ) override
    {
      bool isChainableOp = false;
      switch ( node.binaryOperator() )
      {
        case QgsEsriArcadeBinaryOperatorExpression::BinaryOperator::Plus:
        case QgsEsriArcadeBinaryOperatorExpression::BinaryOperator::Minus:
        case QgsEsriArcadeBinaryOperatorExpression::BinaryOperator::Multiply:
          isChainableOp = true;
          break;

        case QgsEsriArcadeBinaryOperatorExpression::BinaryOperator::Divide:
        case QgsEsriArcadeBinaryOperatorExpression::BinaryOperator::Modulo:
        case QgsEsriArcadeBinaryOperatorExpression::BinaryOperator::Equal:
        case QgsEsriArcadeBinaryOperatorExpression::BinaryOperator::NotEqual:
        case QgsEsriArcadeBinaryOperatorExpression::BinaryOperator::LessThan:
        case QgsEsriArcadeBinaryOperatorExpression::BinaryOperator::GreaterThan:
        case QgsEsriArcadeBinaryOperatorExpression::BinaryOperator::LessThanOrEqual:
        case QgsEsriArcadeBinaryOperatorExpression::BinaryOperator::GreaterThanOrEqual:
        case QgsEsriArcadeBinaryOperatorExpression::BinaryOperator::LogicalAnd:
        case QgsEsriArcadeBinaryOperatorExpression::BinaryOperator::LogicalOr:
        case QgsEsriArcadeBinaryOperatorExpression::BinaryOperator::BitwiseAnd:
        case QgsEsriArcadeBinaryOperatorExpression::BinaryOperator::BitwiseOr:
        case QgsEsriArcadeBinaryOperatorExpression::BinaryOperator::BitwiseXor:
        case QgsEsriArcadeBinaryOperatorExpression::BinaryOperator::BitwiseLeftShift:
        case QgsEsriArcadeBinaryOperatorExpression::BinaryOperator::BitwiseSignedRightShift:
        case QgsEsriArcadeBinaryOperatorExpression::BinaryOperator::BitwiseUnsignedRightShift:
          break;
      }

      bool leftChainable = false;
      bool rightChainable = false;
      if ( isChainableOp )
      {
        if ( dynamic_cast< QgsEsriArcadeBinaryOperatorExpression * >( node.leftOperand() )
             && dynamic_cast< QgsEsriArcadeBinaryOperatorExpression * >( node.leftOperand() )->binaryOperator() == node.binaryOperator() )
        {
          leftChainable = true;
        }
        if ( dynamic_cast< QgsEsriArcadeBinaryOperatorExpression * >( node.rightOperand() )
             && dynamic_cast< QgsEsriArcadeBinaryOperatorExpression * >( node.rightOperand() )->binaryOperator() == node.binaryOperator() )
        {
          rightChainable = true;
        }
      }

      visit( *node.leftOperand() );
      QString newOpLeftPart = mCurrentPart;
      const bool leftIsSimple = mCurrentPartIsSimple;

      visit( *node.rightOperand() );
      QString newOpRightChunk = mCurrentPart;
      const bool rightIsSimple = mCurrentPartIsSimple;

      static const QMap< QgsEsriArcadeBinaryOperatorExpression::BinaryOperator, QString > sBINARY_OPERATOR_MAP {
        { QgsEsriArcadeBinaryOperatorExpression::BinaryOperator::Plus, "+" },
        { QgsEsriArcadeBinaryOperatorExpression::BinaryOperator::Minus, "-" },
        { QgsEsriArcadeBinaryOperatorExpression::BinaryOperator::Multiply, "*" },
        { QgsEsriArcadeBinaryOperatorExpression::BinaryOperator::Divide, "/" },
        { QgsEsriArcadeBinaryOperatorExpression::BinaryOperator::Modulo, "%" },
        { QgsEsriArcadeBinaryOperatorExpression::BinaryOperator::Equal, "=" },
        { QgsEsriArcadeBinaryOperatorExpression::BinaryOperator::NotEqual, "<>" },
        { QgsEsriArcadeBinaryOperatorExpression::BinaryOperator::LessThan, "<" },
        { QgsEsriArcadeBinaryOperatorExpression::BinaryOperator::GreaterThan, ">" },
        { QgsEsriArcadeBinaryOperatorExpression::BinaryOperator::LessThanOrEqual, "<=" },
        { QgsEsriArcadeBinaryOperatorExpression::BinaryOperator::GreaterThanOrEqual, ">=" },
        { QgsEsriArcadeBinaryOperatorExpression::BinaryOperator::LogicalAnd, "AND" },
        { QgsEsriArcadeBinaryOperatorExpression::BinaryOperator::LogicalOr, "OR" },
      };

      mCurrentPartIsSimple = false;
      auto it = sBINARY_OPERATOR_MAP.constFind( node.binaryOperator() );
      if ( it != sBINARY_OPERATOR_MAP.constEnd() )
      {
        if ( !leftIsSimple && !leftChainable )
        {
          newOpLeftPart = u"(%1)"_s.arg( newOpLeftPart );
        }
        if ( !rightIsSimple && !rightChainable )
        {
          newOpRightChunk = u"(%1)"_s.arg( newOpRightChunk );
        }
        mCurrentPart = u"%1 %2 %3"_s.arg( newOpLeftPart, it.value(), newOpRightChunk );
        mCurrentPartIsLiteral = false;
        mCurrentPartIsSimple = false;
      }
      else
      {
        mResult = QgsEsriArcadeConverter::Result::InvalidScriptIncompatible;
        throw QgsNotSupportedException( QObject::tr( "Converting the %1 operator is not supported" ).arg( node.text() ) );
      }
    }

    void visit( const QgsEsriArcadeUnaryOperatorExpression &node ) override
    {
      visit( *node.operand() );

      static const QMap< QgsEsriArcadeUnaryOperatorExpression::UnaryOperator, QString > sUNARY_OPERATOR_MAP {
        { QgsEsriArcadeUnaryOperatorExpression::UnaryOperator::Not, u"NOT "_s },
        { QgsEsriArcadeUnaryOperatorExpression::UnaryOperator::Minus, u"-"_s },
        { QgsEsriArcadeUnaryOperatorExpression::UnaryOperator::Plus, u"+"_s },
      };

      auto it = sUNARY_OPERATOR_MAP.constFind( node.unaryOperator() );
      if ( it != sUNARY_OPERATOR_MAP.constEnd() )
      {
        QString part = mCurrentPart;
        if ( !mCurrentPartIsSimple )
        {
          part = u"(%1)"_s.arg( part );
        }
        mCurrentPart = u"%1%2"_s.arg( it.value(), part );
        mCurrentPartIsLiteral = false;
        mCurrentPartIsSimple = false;
      }
      else
      {
        mResult = QgsEsriArcadeConverter::Result::InvalidScriptIncompatible;
        throw QgsNotSupportedException( QObject::tr( "Converting the %1 operator is not supported" ).arg( node.text() ) );
      }
    }

    void visit( const QgsEsriArcadeFunctionCallExpression &node ) override
    {
      const QString functionName = node.functionName().toLower();
      QStringList convertedArguments;
      QList< bool > convertedArgumentsAreSimple;
      QList< bool > convertedArgumentsAreLiteral;

      for ( QgsEsriArcadeExpression *arg : node.arguments() )
      {
        visit( *arg );
        convertedArguments.append( mCurrentPart );
        convertedArgumentsAreSimple.append( mCurrentPartIsSimple );
        convertedArgumentsAreLiteral.append( mCurrentPartIsLiteral );
      }

      if ( functionName.compare( "date"_L1, Qt::CaseInsensitive ) == 0 )
      {
        mResult = QgsEsriArcadeConverter::Result::InvalidScriptIncompatible;
        throw QgsNotSupportedException( QObject::tr( "The Arcade Date function cannot be automatically converted to a QGIS expression." ) );
      }
      else if ( functionName.compare( "length"_L1, Qt::CaseInsensitive ) == 0 )
      {
        mResult = QgsEsriArcadeConverter::Result::InvalidScriptIncompatible;
        throw QgsNotSupportedException( QObject::tr( "The Arcade Length function cannot be automatically converted to a QGIS expression." ) );
      }
      else if ( functionName.compare( "domainname"_L1, Qt::CaseInsensitive ) == 0 )
      {
        if ( convertedArguments.size() == 2 && convertedArguments.at( 0 ) == "@feature"_L1 )
        {
          if ( convertedArgumentsAreLiteral[1] )
          {
            mCurrentPart = u"represent_value(%1)"_s.arg( QgsExpression::quotedColumnRef( convertedArguments[1].mid( 1, convertedArguments[1].length() - 2 ) ) );
            mCurrentPartIsLiteral = false;
            mCurrentPartIsSimple = true;
            return;
          }
        }

        mResult = QgsEsriArcadeConverter::Result::InvalidScriptIncompatible;
        throw QgsNotSupportedException( QObject::tr( "Complex forms of the Arcade DomainName function cannot be automatically converted to QGIS expressions." ) );
      }
      else if ( functionName.compare( "when"_L1, Qt::CaseInsensitive ) == 0 )
      {
        QString res = u"CASE WHEN %1 THEN %2"_s.arg( convertedArguments.value( 0 ), convertedArguments.value( 1 ) );
        if ( convertedArguments.size() > 2 )
        {
          int i = 2;
          while ( true )
          {
            if ( i >= convertedArguments.size() - 1 )
              break;
            res += u" WHEN %1 THEN %2"_s.arg( convertedArguments.value( i ), convertedArguments.value( i + 1 ) );
            i += 2;
          }
        }

        if ( convertedArguments.size() % 2 == 1 )
        {
          res += u" ELSE %1"_s.arg( convertedArguments.value( convertedArguments.size() - 1 ) );
        }

        res += " END"_L1;

        mCurrentPart = res;
        mCurrentPartIsLiteral = false;
        mCurrentPartIsSimple = false;
        return;
      }
      else if ( functionName.compare( "decode"_L1, Qt::CaseInsensitive ) == 0 )
      {
        QStringList mapGetParts;
        int i = 1;
        while ( true )
        {
          if ( i >= convertedArguments.size() - 1 )
            break;

          // condition
          mapGetParts.append( convertedArguments.at( i ) );
          // value
          mapGetParts.append( convertedArguments.at( i + 1 ) );
          i += 2;
        }

        const QString mapGetPart = u"map_get(map(%1), %2)"_s.arg( mapGetParts.join( ", " ), convertedArguments.at( 0 ) );

        QString res = mapGetPart;
        if ( convertedArguments.size() % 2 == 0 )
        {
          res = u"coalesce(%1, %2)"_s.arg( mapGetPart, convertedArguments.at( convertedArguments.size() - 1 ) );
        }
        mCurrentPart = res;
        mCurrentPartIsLiteral = false;
        mCurrentPartIsSimple = false;
        return;
      }
      else if ( functionName.compare( "isempty"_L1, Qt::CaseInsensitive ) == 0 )
      {
        if ( convertedArguments.size() == 1 )
        {
          if ( convertedArgumentsAreSimple[0] )
          {
            mCurrentPart = u"%1 IS NULL"_s.arg( convertedArguments[0] );
          }
          else
          {
            mCurrentPart = u"(%1) IS NULL"_s.arg( convertedArguments[0] );
          }
          mCurrentPartIsLiteral = false;
          mCurrentPartIsSimple = false;
          return;
        }

        mResult = QgsEsriArcadeConverter::Result::InvalidScriptIncompatible;
        throw QgsNotSupportedException( QObject::tr( "This form of the Arcade IsEmpty function cannot be automatically converted to QGIS expressions." ) );
      }
      else if ( functionName.compare( "mid"_L1, Qt::CaseInsensitive ) == 0 )
      {
        QString start = convertedArguments.value( 1 );
        bool intOk = false;
        int intStart = start.toInt( &intOk );
        if ( convertedArgumentsAreLiteral.value( 1 ) && intOk )
        {
          start = QString::number( intStart + 1 );
        }
        else
        {
          start = u"%1 + 1"_s.arg( start );
        }

        if ( convertedArguments.size() == 2 )
        {
          mCurrentPart = u"substr(%1, %2)"_s.arg( convertedArguments.value( 0 ), start );
        }
        else if ( convertedArguments.size() == 3 )
        {
          mCurrentPart = u"substr(%1, %2, %3)"_s.arg( convertedArguments.value( 0 ), start, convertedArguments.value( 2 ) );
        }
        else
        {
          mResult = QgsEsriArcadeConverter::Result::InvalidScriptIncompatible;
          throw QgsNotSupportedException( QObject::tr( "This form of the 'mid' function cannot be converted to a QGIS expression." ) );
        }
        mCurrentPartIsLiteral = false;
        mCurrentPartIsSimple = true;
        return;
      }
      else if ( functionName.compare( "number"_L1, Qt::CaseInsensitive ) == 0 )
      {
        if ( convertedArguments.size() == 1 )
        {
          mCurrentPart = u"to_real(%1)"_s.arg( convertedArguments.at( 0 ) );
          mCurrentPartIsLiteral = false;
          mCurrentPartIsSimple = true;
          return;
        }
        else
        {
          mResult = QgsEsriArcadeConverter::Result::InvalidScriptIncompatible;
          throw QgsNotSupportedException( QObject::tr( "This form of the 'number' function cannot be converted to a QGIS expression." ) );
        }
      }
      else if ( functionName.compare( "text"_L1, Qt::CaseInsensitive ) == 0 )
      {
        if ( convertedArguments.size() == 1 )
        {
          mCurrentPart = u"to_string(%1)"_s.arg( convertedArguments.at( 0 ) );
          mCurrentPartIsLiteral = false;
          mCurrentPartIsSimple = true;
          return;
        }
        else
        {
          mResult = QgsEsriArcadeConverter::Result::InvalidScriptIncompatible;
          throw QgsNotSupportedException( QObject::tr( "This form of the 'text' function cannot be converted to a QGIS expression." ) );
        }
      }
      else if ( functionName.compare( "split"_L1, Qt::CaseInsensitive ) == 0 )
      {
        if ( convertedArguments.size() == 2 )
        {
          mCurrentPart = u"string_to_array(%1, %2)"_s.arg( convertedArguments.at( 0 ), convertedArguments.at( 1 ) );
          mCurrentPartIsLiteral = false;
          mCurrentPartIsSimple = true;
          return;
        }
        else
        {
          mResult = QgsEsriArcadeConverter::Result::InvalidScriptIncompatible;
          throw QgsNotSupportedException( QObject::tr( "This form of the 'split' function cannot be converted to a QGIS expression." ) );
        }
      }
      else if ( functionName.compare( "concatenate"_L1, Qt::CaseInsensitive ) == 0 )
      {
        if ( convertedArguments.size() == 1 )
        {
          mCurrentPart = u"array_to_string(%1)"_s.arg( convertedArguments.at( 0 ) );
          mCurrentPartIsLiteral = false;
          mCurrentPartIsSimple = true;
          return;
        }
        else if ( convertedArguments.size() == 2 )
        {
          mCurrentPart = u"array_to_string(%1, %2)"_s.arg( convertedArguments.at( 0 ), convertedArguments.at( 1 ) );
          mCurrentPartIsLiteral = false;
          mCurrentPartIsSimple = true;
          return;
        }
        else
        {
          mResult = QgsEsriArcadeConverter::Result::InvalidScriptIncompatible;
          throw QgsNotSupportedException( QObject::tr( "This form of the 'concatenate' function cannot be converted to a QGIS expression." ) );
        }
      }
      else if ( functionName.compare( "find"_L1, Qt::CaseInsensitive ) == 0 )
      {
        if ( convertedArguments.size() == 2 )
        {
          mCurrentPart = u"strpos(%1, %2)"_s.arg( convertedArguments.at( 0 ), convertedArguments.at( 1 ) );
          mCurrentPartIsLiteral = false;
          mCurrentPartIsSimple = true;
          return;
        }
        else if ( convertedArguments.size() == 3 )
        {
          bool arg2IsInt = false;
          const int arg2Int = convertedArguments.at( 2 ).toInt( &arg2IsInt );
          if ( arg2IsInt && arg2Int == 0 )
          {
            mCurrentPart = u"strpos(%1, %2)"_s.arg( convertedArguments.at( 0 ), convertedArguments.at( 1 ) );
            mCurrentPartIsSimple = true;
          }
          else
          {
            mCurrentPart = u"strpos(substr(%1, %2), %3) + %4"_s.arg( convertedArguments.at( 0 ), convertedArguments.at( 2 ), convertedArguments.at( 1 ), convertedArguments.at( 2 ) );
            mCurrentPartIsSimple = false;
          }

          mCurrentPartIsLiteral = false;
          return;
        }
        else
        {
          mResult = QgsEsriArcadeConverter::Result::InvalidScriptIncompatible;
          throw QgsNotSupportedException( QObject::tr( "This form of the 'find' function cannot be converted to a QGIS expression." ) );
        }
      }
      else if ( functionName.compare( "replace"_L1, Qt::CaseInsensitive ) == 0 )
      {
        if ( convertedArguments.size() == 3 )
        {
          mCurrentPart = u"replace(%1, %2, %3)"_s.arg( convertedArguments.at( 0 ), convertedArguments.at( 1 ), convertedArguments.at( 2 ) );
          mCurrentPartIsLiteral = false;
          mCurrentPartIsSimple = true;
          return;
        }
        else
        {
          mResult = QgsEsriArcadeConverter::Result::InvalidScriptIncompatible;
          throw QgsNotSupportedException( QObject::tr( "This form of the 'replace' function cannot be converted to a QGIS expression." ) );
        }
      }

      // these functions are a direct match between Esri and QGIS behavior
      static const QMap< QString, QString > sARCADE_FUNCTIONS {
        { "upper", "upper" },
        { "lower", "lower" },
        { "proper", "title" },
        { "defaultvalue", "coalesce" },
        { "round", "round" },
        { "iif", "if" },
        { "count", "length" },
        { "right", "right" },
        { "left", "left" },
        { "trim", "trim" },
        { "includes", "array_contains" },
        { "max", "max" },
        { "min", "min" },
        { "abs", "abs" },
        { "acos", "acos" },
        { "asin", "asin" },
        { "atan", "atan" },
        { "atan2", "atan2" },
        { "cos", "cos" },
        { "exp", "exp" },
        { "random", "randf" },
        { "sin", "sin" },
        { "sqrt", "sqrt" },
        { "tan", "tan" },
      };

      auto it = sARCADE_FUNCTIONS.constFind( functionName );
      if ( it != sARCADE_FUNCTIONS.constEnd() )
      {
        if ( !convertedArguments.empty() )
        {
          mCurrentPart = u"%1(%2)"_s.arg( it.value(), convertedArguments.join( ", " ) );
          mCurrentPartIsLiteral = false;
        }
        else
        {
          mCurrentPart = u"%1()"_s.arg( it.value() );
          mCurrentPartIsLiteral = false;
        }
        mCurrentPartIsSimple = true;
      }
      else
      {
        mResult = QgsEsriArcadeConverter::Result::InvalidScriptIncompatible;
        throw QgsNotSupportedException( QObject::tr( "The %1 function cannot be converted to a QGIS expression." ).arg( functionName ) );
      }
    }

    void visit( const QgsEsriArcadeIndexExpression &node ) override
    {
      visit( *node.containerExpression() );
      const QString containerPart = mCurrentPart;

      visit( *node.indexExpression() );
      if ( containerPart.compare( "@feature"_L1, Qt::CaseInsensitive ) == 0 )
      {
        if ( mCurrentPartIsLiteral )
        {
          static const QMap< QString, QString > sSPECIAL_FEATURE_ATTRIBUTES {
            { "shape.stlength()", "length(@geometry)" },
            { "shape.starea()", "area(@geometry)" },
          };

          const QString candidate = mCurrentPart.toLower().mid( 1, mCurrentPart.length() - 2 );
          auto it = sSPECIAL_FEATURE_ATTRIBUTES.constFind( candidate );
          if ( it != sSPECIAL_FEATURE_ATTRIBUTES.constEnd() )
          {
            mCurrentPart = it.value();
          }
          else
          {
            if ( mCurrentPart.startsWith( "'" ) )
            {
              mCurrentPart = mCurrentPart.mid( 1, mCurrentPart.length() - 2 );
            }
            mCurrentPart = QgsExpression::quotedColumnRef( mCurrentPart );
          }

          mCurrentPartIsLiteral = false;
          mCurrentPartIsSimple = true;
          return;
        }
        else
        {
          mResult = QgsEsriArcadeConverter::Result::InvalidScriptIncompatible;
          throw QgsNotSupportedException( QObject::tr( "This script cannot be converted to a QGIS expression." ) );
        }
      }
      else if ( mCurrentPartIsLiteral )
      {
        bool ok = false;
        const int intValue = mCurrentPart.toInt( &ok );
        if ( ok )
        {
          mCurrentPart = u"%1[%2]"_s.arg( mCurrentPart ).arg( intValue );
          mCurrentPartIsLiteral = false;
          mCurrentPartIsSimple = true;
          return;
        }
        else
        {
          mResult = QgsEsriArcadeConverter::Result::InvalidScriptIncompatible;
          throw QgsNotSupportedException( QObject::tr( "This script cannot be converted to a QGIS expression." ) );
        }
      }
      else
      {
        mResult = QgsEsriArcadeConverter::Result::InvalidScriptIncompatible;
        throw QgsNotSupportedException( QObject::tr( "This script cannot be converted to a QGIS expression." ) );
      }
    }

    void visit( const QgsEsriArcadeArrayLiteralExpression &node ) override
    {
      QStringList parts;
      for ( QgsEsriArcadeExpression *element : node.elements() )
      {
        visit( *element );
        parts.append( mCurrentPart );
      }

      mCurrentPart = u"array(%1)"_s.arg( parts.join( ", "_L1 ) );
      mCurrentPartIsLiteral = false;
      mCurrentPartIsSimple = true;
    }

    void visit( const QgsEsriArcadeDictionaryLiteralExpression & ) override
    {
      mResult = QgsEsriArcadeConverter::Result::InvalidScriptIncompatible;
      throw QgsNotSupportedException( QObject::tr( "Converting dictionary literals is supported" ) );
    }

    void visit( const QgsEsriArcadeIfStatement &node ) override
    {
      visit( *node.condition() );
      const QString condition = mCurrentPart;

      visit( *node.trueBlock() );
      const QString whenTrue = mCurrentPart;

      QString whenFalse;
      if ( QgsEsriArcadeBlockStatement *falseBlock = node.falseBlock() )
      {
        visit( *falseBlock );
        whenFalse = mCurrentPart;
        mCurrentPart = u"if(%1, %2, %3)"_s.arg( condition, whenTrue, whenFalse );
      }
      else
      {
        mCurrentPart = u"if(%1, %2, %3)"_s.arg( condition, whenTrue, NEXT_BLOCK_MARKER );
      }

      mCurrentPartIsLiteral = false;
      mCurrentPartIsSimple = true;
    }

    void visit( const QgsEsriArcadeForStatement & ) override
    {
      mResult = QgsEsriArcadeConverter::Result::InvalidScriptIncompatible;
      throw QgsNotSupportedException( QObject::tr( "Scripts containing for loops cannot be converted to QGIS expressions." ) );
    }

    void visit( const QgsEsriArcadeForInStatement & ) override
    {
      mResult = QgsEsriArcadeConverter::Result::InvalidScriptIncompatible;
      throw QgsNotSupportedException( QObject::tr( "Scripts containing for loops cannot be converted to QGIS expressions." ) );
    }

    void visit( const QgsEsriArcadeWhileStatement & ) override
    {
      mResult = QgsEsriArcadeConverter::Result::InvalidScriptIncompatible;
      throw QgsNotSupportedException( QObject::tr( "Scripts containing while loops cannot be converted to QGIS expressions." ) );
    }

    void visit( const QgsEsriArcadeBreakStatement & ) override
    {
      mResult = QgsEsriArcadeConverter::Result::InvalidScriptIncompatible;
      throw QgsNotSupportedException( QObject::tr( "Scripts containing break statements cannot be converted to QGIS expressions." ) );
    }

    void visit( const QgsEsriArcadeContinueStatement & ) override
    {
      mResult = QgsEsriArcadeConverter::Result::InvalidScriptIncompatible;
      throw QgsNotSupportedException( QObject::tr( "Scripts containing continue statements cannot be converted to QGIS expressions." ) );
    }
};

const QString ArcadeToExpressionVisitor::NEXT_BLOCK_MARKER = u"!!!!!NEXT_BLOCK!!!!!"_s;


//
// QgsEsriArcadeConverter
//

QgsEsriArcadeConverter::QgsEsriArcadeConverter( const QString &script )
  : mScript( script )
{
  QgsEsriArcadeScript parsedScript = QgsEsriArcadeScript( script );
  if ( parsedScript.hasParserError() )
  {
    mErrorString = QObject::tr( "The Arcade script could not be parsed." );
    mResult = Result::ParserError;
    return;
  }

  ArcadeToExpressionVisitor visitor;
  try
  {
    parsedScript.acceptVisitor( visitor );
    mExpression = visitor.mCurrentPart;
  }
  catch ( QgsNotSupportedException &ex )
  {
    mErrorString = ex.what();
    mResult = visitor.mResult;
  }
}
