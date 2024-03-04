/***************************************************************************
    qgscodeeditorpython.h - A Python editor based on QScintilla
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

#ifndef QGSCODEEDITORPYTHON_H
#define QGSCODEEDITORPYTHON_H

#include "qgscodeeditor.h"
#include "qgis_sip.h"
#include "qgis_gui.h"
#include <Qsci/qscilexerpython.h>

class QgsSettingsEntryInteger;
class QgsSettingsEntryBool;

SIP_IF_MODULE( HAVE_QSCI_SIP )

#ifndef SIP_RUN
///@cond PRIVATE
class QgsQsciLexerPython : public QsciLexerPython
{
    Q_OBJECT
  public:

    QgsQsciLexerPython( QObject *parent = nullptr );

    const char *keywords( int set ) const override;

};
///@endcond
#endif

/**
 * \ingroup gui
 * \brief A Python editor based on QScintilla2. Adds syntax highlighting and
 * code autocompletion.
 * \note may not be available in Python bindings, depending on platform support
 */
class GUI_EXPORT QgsCodeEditorPython : public QgsCodeEditor
{
    Q_OBJECT

  public:

#ifndef SIP_RUN
///@cond PRIVATE
    static inline QgsSettingsTreeNode *sTreePythonCodeEditor = QgsCodeEditor::sTreeCodeEditor->createChildNode( QStringLiteral( "python" ) );
    static const QgsSettingsEntryString *settingCodeFormatter;
    static const QgsSettingsEntryInteger *settingMaxLineLength;
    static const QgsSettingsEntryBool *settingSortImports;
    static const QgsSettingsEntryInteger *settingAutopep8Level;
    static const QgsSettingsEntryBool *settingBlackNormalizeQuotes;
///@endcond PRIVATE
#endif

    /**
     * Construct a new Python editor.
     *
     * \param parent The parent QWidget
     * \param filenames The list of apis files to load for the Python lexer
     * \param mode code editor mode (since QGIS 3.30)
     * \param flags code editor flags (since QGIS 3.32)
     */
    QgsCodeEditorPython( QWidget *parent SIP_TRANSFERTHIS = nullptr, const QList<QString> &filenames = QList<QString>(),
                         QgsCodeEditor::Mode mode = QgsCodeEditor::Mode::ScriptEditor, QgsCodeEditor::Flags flags = QgsCodeEditor::Flag::CodeFolding );

    Qgis::ScriptLanguage language() const override;
    Qgis::ScriptLanguageCapabilities languageCapabilities() const override;

    /**
     * Load APIs from one or more files
     * \param filenames The list of apis files to load for the Python lexer
     */
    void loadAPIs( const QList<QString> &filenames );

    /**
     * Loads a \a script file.
     */
    bool loadScript( const QString &script );

    /**
     * Check whether the current cursor position is inside a string literal or a comment
     *
     * \since QGIS 3.30
     */
    bool isCursorInsideStringLiteralOrComment() const;

    /**
     * Returns the character before the cursor, or an empty string if cursor is set at start
     *
     * \since QGIS 3.30
     */
    QString characterBeforeCursor() const;

    /**
     * Returns the character after the cursor, or an empty string if the cursor is set at end
     *
     * \since QGIS 3.30
     */
    QString characterAfterCursor() const;

    /**
     * Updates the editor capabilities.
     *
     * \since QGIS 3.32
     */
    void updateCapabilities();

    bool checkSyntax() override;

  public slots:

    /**
     * Searches the selected text in the official PyQGIS online documentation.
     *
     * \since QGIS 3.16
     */
    void searchSelectedTextInPyQGISDocs();

    /**
     * Toggle comment for the selected text.
     *
     * \since QGIS 3.30
     */
    void toggleComment() override;

  protected:

    void initializeLexer() override;
    virtual void keyPressEvent( QKeyEvent *event ) override;
    QString reformatCodeString( const QString &string ) override;
    void populateContextMenu( QMenu *menu ) override;

  protected slots:

    /**
     * Triggers the autocompletion popup.
     *
     * \since QGIS 3.16
     */
    void autoComplete();

  private:

    QList<QString> mAPISFilesList;
    QString mPapFile;

    Qgis::ScriptLanguageCapabilities mCapabilities;

    static const QMap<QString, QString> sCompletionPairs;

    // Only used for selected text
    static const QStringList sCompletionSingleCharacters;

};

#endif
