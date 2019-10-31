/***************************************************************************
     qgsexpression_p.h
     ---------------
    Date                 : October 2019
    Copyright            : (C) 2019 by Matthias Kuhn
    Email                : matthias@opengis.ch
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

/// @cond PRIVATE

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QGIS API.  It exists purely as an
// implementation detail. This header file may change from version to
// version without notice, or even be removed.
//

#define SIP_NO_FILE

#include "qgsexpression.h"

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
Q_GLOBAL_STATIC( HelpTextHash, sFunctionHelpTexts )
#endif // QGSEXPRESSIONPRIVATE_H
