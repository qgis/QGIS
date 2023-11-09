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

#ifndef QGSCODEEDITORSQL_H
#define QGSCODEEDITORSQL_H

#include "qgscodeeditor.h"
#include "qgis_sip.h"
#include "qgis_gui.h"
#include "qgsfeature.h"
#include <Qsci/qscilexersql.h>

SIP_IF_MODULE( HAVE_QSCI_SIP )

/**
 * \ingroup gui
 * \brief A SQL editor based on QScintilla2. Adds syntax highlighting and
 * code autocompletion.
 * \note may not be available in Python bindings, depending on platform support
 * \since QGIS 2.6
 */
class GUI_EXPORT QgsCodeEditorSQL : public QgsCodeEditor
{
    Q_OBJECT

  public:
    //! Constructor for QgsCodeEditorSQL
    QgsCodeEditorSQL( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    Qgis::ScriptLanguage language() const override;

    virtual ~QgsCodeEditorSQL();

    /**
     * Set field names to be added to the lexer API.
     *
     * \since QGIS 3.14
     */
    void setFields( const QgsFields &fields );

    /**
     * Set field names to \a fieldNames to be added to the lexer API.
     *
     * \since QGIS 3.18
     */
    void setFieldNames( const QStringList &fieldNames );

    /**
     * Returns field names from the lexer API.
     * \since QGIS 3.22
     */
    QStringList fieldNames() const;

    /**
     * Set extra keywords to \a extraKeywords.
     *
     * Extra keywords are usually added
     * from provider connections and represent function and other provider specific
     * keywords.
     *
     * \since QGIS 3.22
     */
    void setExtraKeywords( const QStringList &extraKeywords );

    /**
     * Returns the extra keywords.
     *
     * Extra keywords are usually added
     * from provider connections and represent function and other provider specific
     * keywords.
     *
     * \since QGIS 3.22
     */
    QStringList extraKeywords() const;

  protected:
    void initializeLexer() override;

  private:
    void updateApis();
    QsciAPIs *mApis = nullptr;
    QsciLexerSQL *mSqlLexer = nullptr;
    QSet<QString> mExtraKeywords;

    QSet<QString> mFieldNames;

    friend class TestQgsQueryResultWidget;
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
class QgsCaseInsensitiveLexerSQL: public QsciLexerSQL
{
    Q_OBJECT

  public:
    //! constructor
    explicit QgsCaseInsensitiveLexerSQL( QObject *parent = nullptr ) : QsciLexerSQL( parent ) {}

    bool caseSensitive() const override { return false; }
};
///@endcond
#endif

#endif
