/***************************************************************************
    qgscodeeditor.h - A base code editor for QGIS and plugins.  Provides
                      a base editor using QScintilla for editors
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

#ifndef QGSCODEEDITOR_H
#define QGSCODEEDITOR_H

#include <QString>
#include "qgscodeeditorcolorscheme.h"
#include "qgis.h"

// qscintilla includes
#include <Qsci/qsciapis.h>
#include "qgis_sip.h"
#include "qgis_gui.h"

#include <QMap>


SIP_IF_MODULE( HAVE_QSCI_SIP )

/**
 * \ingroup gui
 * \brief An interface for code interpreters.
 * \since QGIS 3.30
 */
class GUI_EXPORT QgsCodeInterpreter
{
  public:

    virtual ~QgsCodeInterpreter();

    /**
     * Executes a \a command in the interpreter.
     *
     * Returns an interpreter specific state value.
     */
    int exec( const QString &command );

    /**
     * Returns the current interpreter state.
     *
     * The actual interpretation of the returned values depend on
     * the interpreter subclass.
     */
    virtual int currentState() const { return mState; }

    /**
     * Returns the interactive prompt string to use for the
     * interpreter, given a \a state.
     */
    virtual QString promptForState( int state ) const = 0;

  protected:

    /**
     * Pure virtual method for executing commands in the interpreter.
     *
     * Subclasses must implement this method. It will be called internally
     * whenever the public exec() method is called.
     */
    virtual int execCommandImpl( const QString &command ) = 0;

  private:

    int mState = 0;

};



class QWidget;

/**
 * \ingroup gui
 * \brief A text editor based on QScintilla2.
 * \note may not be available in Python bindings, depending on platform support
 * \since QGIS 2.6
 */
class GUI_EXPORT QgsCodeEditor : public QsciScintilla
{
    Q_OBJECT

  public:

    /**
     * Code editor modes.
     *
     * \since QGIS 3.30
     */
    enum class Mode
    {
      ScriptEditor, //!< Standard mode, allows for display and edit of entire scripts
      OutputDisplay, //!< Read only mode for display of command outputs
      CommandInput, //!< Command input mode
    };
    Q_ENUM( Mode )

    /**
     * Margin roles.
     *
     * This enum contains the roles which the different numbered margins are used for.
     *
     * \since QGIS 3.16
     */
    enum class MarginRole SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsCodeEditor, MarginRole ) : int
      {
      LineNumbers = 0, //!< Line numbers
      ErrorIndicators = 1, //!< Error indicators
      FoldingControls = 2, //!< Folding controls
    };
    Q_ENUM( MarginRole )

    /**
     * \brief Flags controlling behavior of code editor
     *
     * \since QGIS 3.28
     */
    enum class Flag : int
    {
      CodeFolding = 1 << 0, //!< Indicates that code folding should be enabled for the editor
    };
    Q_ENUM( Flag )

    /**
     * \brief Flags controlling behavior of code editor
     *
     * \since QGIS 3.28
     */
    Q_DECLARE_FLAGS( Flags, Flag )
    Q_FLAG( Flags )

    /**
     * Construct a new code editor.
     *
     * \param parent The parent QWidget
     * \param title The title to show in the code editor dialog
     * \param folding FALSE: Enable folding for code editor (deprecated, use \a flags instead)
     * \param margin FALSE: Enable margin for code editor (deprecated)
     * \param flags flags controlling behavior of code editor (since QGIS 3.28)
     * \param mode code editor mode (since QGIS 3.30)
     * \since QGIS 2.6
     */
    QgsCodeEditor( QWidget * parent SIP_TRANSFERTHIS = nullptr, const QString & title = QString(), bool folding = false, bool margin = false, QgsCodeEditor::Flags flags = QgsCodeEditor::Flags(), QgsCodeEditor::Mode mode = QgsCodeEditor::Mode::ScriptEditor );

    /**
     * Set the widget title
     * \param title widget title
     */
    void setTitle( const QString & title );

    /**
     * Returns the associated scripting language.
     *
     * \since QGIS 3.30
     */
    virtual Qgis::ScriptLanguage language() const;

    /**
     * Returns a user-friendly, translated name of the specified script \a language.
     *
     * \since QGIS 3.30
     */
    static QString languageToString( Qgis::ScriptLanguage language );

    /**
     * Set margin visible state
     * \param margin Set margin in the editor
     * \deprecated Use base class methods for individual margins instead, or setLineNumbersVisible()
     */
    Q_DECL_DEPRECATED void setMarginVisible( bool margin ) SIP_DEPRECATED;

    /**
     * Returns whether margins are in a visible state
     * \deprecated Use base class methods for individual margins instead, or lineNumbersVisible()
     */
    Q_DECL_DEPRECATED bool marginVisible() SIP_DEPRECATED { return mMargin; }

    /**
     * Sets whether line numbers should be visible in the editor.
     *
     * Defaults to FALSE.
     *
     * \see lineNumbersVisible()
     * \since QGIS 3.16
     */
    void setLineNumbersVisible( bool visible );

    /**
     * Returns whether line numbers are visible in the editor.
     *
     * \see setLineNumbersVisible()
     * \since QGIS 3.16
     */
    bool lineNumbersVisible() const;

    /**
     * Set whether the folding controls are visible in the editor.
     * \see foldingVisible()
     */
    void setFoldingVisible( bool folding );

    /**
     * Returns TRUE if the folding controls are visible in the editor.
     * \see setFoldingVisible()
     */
    bool foldingVisible();

    /**
     * Insert text at cursor position, or replace any selected text if user has
     * made a selection.
     * \param text The text to be inserted
     */
    void insertText( const QString & text );

    /**
     * Returns the default color for the specified \a role.
     *
     * The optional \a theme argument can be used to specify a color \a theme. A blank
     * \a theme indicates the default color scheme.
     *
     * Available themes are stored in QgsCodeEditorColorSchemeRegistry, and can be retrieved
     * via QgsGui::codeEditorColorSchemeRegistry().
     *
     * \since QGIS 3.16
     */
    static QColor defaultColor( QgsCodeEditorColorScheme::ColorRole role, const QString & theme = QString() );

    /**
     * Returns the color to use in the editor for the specified \a role.
     *
     * This color will be the default theme color for the role, unless the user has manually
     * selected a custom color scheme for the editor.
     *
     * \see setColor()
     * \since QGIS 3.16
     */
    static QColor color( QgsCodeEditorColorScheme::ColorRole role );

    /**
     * Sets the \a color to use in the editor for the specified \a role.
     *
     * This color will be stored as the new default color for the role, to be used for all code editors.
     *
     * Set \a color to an invalid QColor in order to clear the stored color value and reset it to
     * the default color.
     *
     * \see color()
     * \since QGIS 3.16
     */
    static void setColor( QgsCodeEditorColorScheme::ColorRole role, const QColor & color );

    /**
     * Returns the monospaced font to use for code editors.
     *
     * \since QGIS 3.16
     */
    static QFont getMonospaceFont();

    /**
     * Sets a custom appearance for the widget, disconnecting it from using the standard appearance
     * taken from QSettings.
     *
     * \note Not available in Python bindings
     * \since QGIS 3.16
     */
    void setCustomAppearance( const QString & scheme = QString(), const QMap< QgsCodeEditorColorScheme::ColorRole, QColor > & customColors = QMap< QgsCodeEditorColorScheme::ColorRole, QColor >(), const QString & fontFamily = QString(), int fontSize = 0 ) SIP_SKIP;

    /**
     * Adds a \a warning message and indicator to the specified a \a lineNumber.
     *
     * \see clearWarnings()
     * \since QGIS 3.16
     */
    void addWarning( int lineNumber, const QString & warning );

    /**
     * Clears all warning messages from the editor.
     *
     * \see addWarning()
     * \since QGIS 3.16
     */
    void clearWarnings();

    /**
     * Returns the code editor mode.
     *
     * \since QGIS 3.30
     */
    QgsCodeEditor::Mode mode() const { return mMode; }

    /**
     * Returns TRUE if the cursor is on the last line of the document.
     *
     * \since QGIS 3.28
     */
    bool isCursorOnLastLine() const;

    /**
     * Sets the file path to use for recording and retrieving previously
     * executed commands.
     *
     * \note Applies to code editors in the QgsCodeEditor::Mode::CommandInput mode only.
     *
     * \since QGIS 3.30
     */
    void setHistoryFilePath( const QString &path );

    /**
     * Returns the list of commands previously executed in the editor.
     *
     * \note Applies to code editors in the QgsCodeEditor::Mode::CommandInput mode only.
     *
     * \since QGIS 3.30
     */
    QStringList history() const;

    /**
     * Returns the attached code interpreter, or NULLPTR if not set.
     *
     * \see setInterpreter()
     * \since QGIS 3.30
     */
    QgsCodeInterpreter *interpreter() const;

    /**
     * Sets an attached code interpreter for executing commands when the editor
     * is in the QgsCodeEditor::Mode::CommandInput mode.
     *
     * \see interpreter()
     * \since QGIS 3.30
     */
    void setInterpreter( QgsCodeInterpreter *newInterpreter );

  public slots:

    /**
     * Runs a command in the editor.
     *
     * An interpreter() must be set.
     *
     * \since QGIS 3.30
     */
    void runCommand( const QString &command );

    /**
     * Moves the cursor to the start of the document and scrolls to ensure
     * it is visible.
     *
     * \since QGIS 3.28
     */
    virtual void moveCursorToStart();

    /**
     * Moves the cursor to the end of the document and scrolls to ensure
     * it is visible.
     *
     * \since QGIS 3.28
     */
    virtual void moveCursorToEnd();

    /**
     * Shows the previous command from the session in the editor.
     *
     * \note Applies to code editors in the QgsCodeEditor::Mode::CommandInput mode only.
     *
     * \since QGIS 3.30
     */
    void showPreviousCommand();

    /**
     * Shows the next command from the session in the editor.
     *
     * \note Applies to code editors in the QgsCodeEditor::Mode::CommandInput mode only.
     *
     * \since QGIS 3.30
     */
    void showNextCommand();

    /**
     * Shows the command history dialog.

     * \note Applies to code editors in the QgsCodeEditor::Mode::CommandInput mode only.
     *
     * \since QGIS 3.30
     */
    void showHistory();

    /**
     * Removes the command at the specified \a index from the history of the code editor.
     *
     * \since QGIS 3.30
     */
    void removeHistoryCommand( int index );

    /**
     * Clears the history of commands run in the current session.
     *
     * \note Applies to code editors in the QgsCodeEditor::Mode::CommandInput mode only.
     *
     * \since QGIS 3.30
     */
    void clearSessionHistory();

    /**
     * Clears the entire persistent history of commands run in the editor.
     *
     * \note Applies to code editors in the QgsCodeEditor::Mode::CommandInput mode only.
     *
     * \since QGIS 3.30
     */
    void clearPersistentHistory();

    /**
     * Stores the commands executed in the editor to the persistent history file.
     *
     * \since QGIS 3.30
     */
    bool writeHistoryFile();

  signals:

    /**
     * Emitted when the history of commands run in the current session is cleared.
     *
     * \since QGIS 3.30
     */
    void sessionHistoryCleared();

    /**
     * Emitted when the persistent history of commands run in the editor is cleared.
     *
     * \since QGIS 3.30
     */
    void persistentHistoryCleared();

  protected:

    /**
     * Returns TRUE if a \a font is a fixed pitch font.
     */
    static bool isFixedPitch( const QFont &font );

    void focusOutEvent( QFocusEvent *event ) override;
    void keyPressEvent( QKeyEvent *event ) override;
    void contextMenuEvent( QContextMenuEvent *event ) override;

    /**
     * Called when the dialect specific code lexer needs to be initialized (or reinitialized).
     *
     * The default implementation does nothing.
     *
     * \since QGIS 3.16
     */
    virtual void initializeLexer();

    /**
     * Returns the color to use in the lexer for the specified \a role.
     *
     * \since QGIS 3.16
     */
    QColor lexerColor( QgsCodeEditorColorScheme::ColorRole role ) const;

    /**
     * Returns the font to use in the lexer.
     *
     * \since QGIS 3.16
     */
    QFont lexerFont() const;

    /**
     * Performs tasks which must be run after a lexer has been set for the widget.
     *
     * \since QGIS 3.16
     */
    void runPostLexerConfigurationTasks();

    /**
     * Updates the soft history by storing the current editor text in the history.
     *
     * \since QGIS 3.30
     */
    void updateSoftHistory();

    /**
     * Triggers an update of the interactive prompt part of the editor.
     *
     * \note Applies to code editors in the QgsCodeEditor::Mode::CommandInput mode only.
     *
     * \since QGIS 3.30
     */
    void updatePrompt();

    /**
     * Called when the context \a menu for the widget is about to be shown, after it
     * has been fully populated with the standard actions created by the base class.
     *
     * This method provides an opportunity for subclasses to add additional non-standard
     * actions to the context menu.
     *
     * \since QGIS 3.30
     */
    virtual void populateContextMenu( QMenu *menu );

  private:

    void setSciWidget();
    void updateFolding();
    bool readHistoryFile();
    void syncSoftHistory();
    void updateHistory( const QStringList &commands, bool skipSoftHistory = false );

    QString mWidgetTitle;
    bool mMargin = false;
    QgsCodeEditor::Flags mFlags;
    QgsCodeEditor::Mode mMode = QgsCodeEditor::Mode::ScriptEditor;

    bool mUseDefaultSettings = true;
    // used if above is false, inplace of values taken from QSettings:
    bool mOverrideColors = false;
    QString mColorScheme;
    QMap< QgsCodeEditorColorScheme::ColorRole, QColor > mCustomColors;
    QString mFontFamily;
    int mFontSize = 0;

    QVector< int > mWarningLines;

    // for use in command input mode
    QStringList mHistory;
    QStringList mSoftHistory;
    int mSoftHistoryIndex = 0;
    QString mHistoryFilePath;

    QgsCodeInterpreter *mInterpreter = nullptr;

    static QMap< QgsCodeEditorColorScheme::ColorRole, QString > sColorRoleToSettingsKey;

    static constexpr int MARKER_NUMBER = 6;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsCodeEditor::Flags )

// clazy:excludeall=qstring-allocations

#endif
