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
// qscintilla includes
#include <Qsci/qsciapis.h>
#include "qgis_sip.h"
#include "qgis_gui.h"


SIP_IF_MODULE( HAVE_QSCI_SIP )


class QWidget;

/**
 * \ingroup gui
 * A text editor based on QScintilla2.
 * \note may not be available in Python bindings, depending on platform support
 * \since QGIS 2.6
 */
class GUI_EXPORT QgsCodeEditor : public QsciScintilla
{
    Q_OBJECT

  public:

    /**
     * Construct a new code editor.
     *
     * \param parent The parent QWidget
     * \param title The title to show in the code editor dialog
     * \param folding FALSE: Enable folding for code editor
     * \param margin FALSE: Enable margin for code editor
     * \since QGIS 2.6
     */
    QgsCodeEditor( QWidget *parent SIP_TRANSFERTHIS = nullptr, const QString &title = QString(), bool folding = false, bool margin = false );

    /**
     * Set the widget title
     * \param title widget title
     */
    void setTitle( const QString &title );

    /**
     * Set margin visible state
     *  \param margin Set margin in the editor
     */
    void setMarginVisible( bool margin );
    bool marginVisible() { return mMargin; }

    /**
     * Set folding visible state
     *  \param folding Set folding in the editor
     */
    void setFoldingVisible( bool folding );
    bool foldingVisible() { return mFolding; }

    /**
     * Insert text at cursor position, or replace any selected text if user has
     * made a selection.
     * \param text The text to be inserted
     */
    void insertText( const QString &text );

    /**
     * Color roles.
     *
     * \since QGIS 3.16
     */
    enum class ColorRole
    {
      Default, //!< Default text color
      Keyword, //!< Keyword color
      Class, //!< Class color
      Method, //!< Method color
      Decoration, //!< Decoration color
      Number, //!< Number color
      Comment, //!< Comment color
      CommentLine, //!< Line comment color
      CommentBlock, //!< Comment block color
      Background, //!< Background color
      Cursor, //!< Cursor color
      CaretLine, //!< Caret line color
      SingleQuote, //!< Single quote color
      DoubleQuote, //!< Double quote color
      TripleSingleQuote, //!< Triple single quote color
      TripleDoubleQuote, //!< Triple double quote color
      Operator, //!< Operator color
      QuotedOperator, //!< Quoted operator color
      Identifier, //!< Identifier color
      QuotedIdentifier, //!< Quoted identifier color
      Tag, //!< Tag color
      UnknownTag, //!< Unknown tag
      MarginBackground, //!< Margin background color
      MarginForeground, //!< Margin foreground color
      SelectionBackground, //!< Selection background color
      SelectionForeground, //!< Selection foreground color
      MatchedBraceBackground, //!< Matched brace background color
      MatchedBraceForeground, //!< Matched brace foreground color
      Edge, //!< Edge color
      Fold, //!< Fold color
      Error, //!< Error color
    };

    /**
     * Returns the default color for the specified \a role.
     *
     * The optional \a theme argument can be used to specify a color \a theme. A blank
     * \a theme indicates the default color scheme.
     *
     * Possible \a theme values are:
     *
     * - (empty string) follow application default colors
     * - solarized
     * - solarized_dark
     *
     * \since QGIS 3.16
     */
    static QColor defaultColor( ColorRole role, const QString &theme = QString() );

    /**
     * Returns the color to use in the editor for the specified \a role.
     *
     * This color will be the default theme color for the role, unless the user has manually
     * selected a custom color scheme for the editor.
     *
     * \see setColor()
     * \since QGIS 3.16
     */
    static QColor color( ColorRole role );

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
    static void setColor( ColorRole role, const QColor &color );

    /**
     * Returns the monospaced font to use for code editors.
     *
     * \since QGIS 3.16
     */
    static QFont getMonospaceFont();

  protected:

    bool isFixedPitch( const QFont &font );

    void focusOutEvent( QFocusEvent *event ) override;
    void keyPressEvent( QKeyEvent *event ) override;

  private:

    void setSciWidget();

    QString mWidgetTitle;
    bool mFolding;
    bool mMargin;

    static QMap< ColorRole, QString > sColorRoleToSettingsKey;
};

// clazy:excludeall=qstring-allocations

#endif
