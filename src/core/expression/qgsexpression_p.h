/***************************************************************************
 qgsexpression_p.h

 ---------------------
 begin                : 9.12.2015
 copyright            : (C) 2015 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSEXPRESSIONPRIVATE_H
#define QGSEXPRESSIONPRIVATE_H

#include <QString>
#include <memory>

#include "qgsexpression.h"
#include "qgsdistancearea.h"
#include "qgsunittypes.h"
#include "qgsexpressionnode.h"

///@cond

/**
 * This class exists only for implicit sharing of QgsExpression
 * and is not part of the public API.
 * It should be considered an implementation detail.
 */
class QgsExpressionPrivate
{
  public:
    QgsExpressionPrivate()
      : ref( 1 )
    {}

    QgsExpressionPrivate( const QgsExpressionPrivate &other )
      : ref( 1 )
      , mRootNode( other.mRootNode ? other.mRootNode->clone() : nullptr )
      , mParserErrorString( other.mParserErrorString )
      , mEvalErrorString( other.mEvalErrorString )
      , mParserErrors( other.mParserErrors )
      , mExp( other.mExp )
      , mCalc( other.mCalc )
      , mDistanceUnit( other.mDistanceUnit )
      , mAreaUnit( other.mAreaUnit )
    {}

    ~QgsExpressionPrivate()
    {
      delete mRootNode;
    }

    QAtomicInt ref;

    QgsExpressionNode *mRootNode = nullptr;

    QString mParserErrorString;
    QString mEvalErrorString;

    QList<QgsExpression::ParserError> mParserErrors;

    QString mExp;

    std::shared_ptr<QgsDistanceArea> mCalc;
    QgsUnitTypes::DistanceUnit mDistanceUnit = QgsUnitTypes::DistanceUnknownUnit;
    QgsUnitTypes::AreaUnit mAreaUnit = QgsUnitTypes::AreaUnknownUnit;

    //! Whether prepare() has been called before evaluate()
    bool mIsPrepared = false;
};


struct HelpArg
{
  HelpArg( const QString &arg, const QString &desc, bool descOnly = false, bool syntaxOnly = false,
           bool optional = false, const QString &defaultVal = QString() )
    : mArg( arg )
    , mDescription( desc )
    , mDescOnly( descOnly )
    , mSyntaxOnly( syntaxOnly )
    , mOptional( optional )
    , mDefaultVal( defaultVal )
  {}

  QString mArg;
  QString mDescription;
  bool mDescOnly;
  bool mSyntaxOnly;
  bool mOptional;
  QString mDefaultVal;
};

struct HelpExample
{
  HelpExample( const QString &expression, const QString &returns, const QString &note = QString() )
    : mExpression( expression )
    , mReturns( returns )
    , mNote( note )
  {}

  QString mExpression;
  QString mReturns;
  QString mNote;
};


struct HelpVariant
{
  HelpVariant( const QString &name, const QString &description,
               const QList<HelpArg> &arguments = QList<HelpArg>(),
               bool variableLenArguments = false,
               const QList<HelpExample> &examples = QList<HelpExample>(),
               const QString &notes = QString() )
    : mName( name )
    , mDescription( description )
    , mArguments( arguments )
    , mVariableLenArguments( variableLenArguments )
    , mExamples( examples )
    , mNotes( notes )
  {}

  QString mName;
  QString mDescription;
  QList<HelpArg> mArguments;
  bool mVariableLenArguments;
  QList<HelpExample> mExamples;
  QString mNotes;
};


struct Help
{
  //! Constructor for expression help
  Help() = default;

  Help( const QString &name, const QString &type, const QString &description, const QList<HelpVariant> &variants )
    : mName( name )
    , mType( type )
    , mDescription( description )
    , mVariants( variants )
  {}

  QString mName;
  QString mType;
  QString mDescription;
  QList<HelpVariant> mVariants;
};

typedef QHash<QString, Help> HelpTextHash;

HelpTextHash &functionHelpTexts();

///@endcond

#endif // QGSEXPRESSIONPRIVATE_H
