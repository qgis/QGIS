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


class QWidget;

/** \ingroup gui
 * A text editor based on QScintilla2.
 * \note added in 2.6
 */
class GUI_EXPORT QgsCodeEditor : public QsciScintilla
{
    Q_OBJECT

  public:
    /**
     * Construct a new code editor.
     *
     * @param parent The parent QWidget
     * @param title The title to show in the code editor dialog
     * @param folding False: Enable margin for code editor
     * @param margin False: Enable folding for code editor
     * @note added in 2.6
     */
    QgsCodeEditor( QWidget *parent = 0, QString title = "", bool folding = false, bool margin = false );
    ~QgsCodeEditor();

    /** Set the widget title
     * @param title widget title
    */
    void setTitle( const QString title );

    /** Set margin visible state
     *  @param margin Set margin in the editor
     */
    void setMarginVisible( bool margin );
    bool marginVisible() { return mMargin; }

    /** Set folding visible state
     *  @param folding Set folding in the editor
     */
    void setFoldingVisible( bool folding );
    bool foldingVisible() { return mFolding; }

    /** Insert text at cursor position, or replace any selected text if user has
     * made a selection.
     * @param theText The text to be inserted
     */
    void insertText( const QString theText );

  protected:

    bool isFixedPitch( const QFont& font );

    QFont getMonospaceFont();

  private:

    void setSciWidget();

    QString mWidgetTitle;
    bool mFolding;
    bool mMargin;
};

#endif
