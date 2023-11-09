/***************************************************************************
    qgscodeeditorsql.h - A SQL editor based on QScintilla
     --------------------------------------
    Date                 : 06-Oct-2013
    Copyright            : (C) 2013 by Salvatore Larosa
    Email                : lrssvtml (at) gmail (dot) com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCODEEDITOREXPRESSION_H
#define QGSCODEEDITOREXPRESSION_H

#include "qgis_sip.h"
#include "qgis_gui.h"
#include "qgscodeeditor.h"
#include "qgsexpressioncontext.h"

#include <Qsci/qscilexersql.h>

SIP_IF_MODULE( HAVE_QSCI_SIP )

/**
 * \ingroup gui
 *
 * \brief A QGIS expression editor based on QScintilla2. Adds syntax highlighting and
 * code autocompletion.
 *
 * \since QGIS 3.4
 */
class GUI_EXPORT QgsCodeEditorExpression : public QgsCodeEditor
{
    Q_OBJECT

  public:
    //! Constructor for QgsCodeEditorExpression
    QgsCodeEditorExpression( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    Qgis::ScriptLanguage language() const override;

    /**
     * Variables and functions from this expression context will be added to
     * the API.
     * Will also reload all globally registered functions.
     */
    void setExpressionContext( const QgsExpressionContext &context );

    /**
     * Field names will be added to the API.
     */
    void setFields( const QgsFields &fields );

  protected:

    void initializeLexer() override;

  private:
    void updateApis();
    QsciAPIs *mApis = nullptr;
    QsciLexerSQL *mSqlLexer = nullptr;

    QStringList mVariables;
    QStringList mContextFunctions;
    QStringList mFunctions;
    QStringList mFieldNames;
};

#ifndef SIP_RUN
///@cond PRIVATE

/**
 * Internal use.
 *
 * setAutoCompletionCaseSensitivity( false ) is not sufficient when installing
 * a lexer, since its caseSensitive() method is actually used, and defaults
 * to TRUE.
 * \note not available in Python bindings
 * \ingroup gui
*/
class QgsLexerExpression : public QsciLexerSQL
{
    Q_OBJECT

  public:
    //! Constructor
    explicit QgsLexerExpression( QObject *parent = nullptr );

    const char *language() const override;

    bool caseSensitive() const override;

    const char *wordCharacters() const override;
};

class QgsSciApisExpression : public QsciAPIs
{
    Q_OBJECT
  public:
    QgsSciApisExpression( QsciLexer *lexer );

    QStringList callTips( const QStringList &context, int commas, QsciScintilla::CallTipsStyle style, QList<int> &shifts ) override;
};
///@endcond
#endif

#endif
