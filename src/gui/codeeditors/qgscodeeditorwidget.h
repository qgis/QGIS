/***************************************************************************
    qgscodeeditorwidget.h
     --------------------------------------
    Date                 : May 2024
    Copyright            : (C) 2024 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCODEEDITORWIDGET_H
#define QGSCODEEDITORWIDGET_H

#include "qgis_gui.h"
#include "qgis_sip.h"
#include "qgspanelwidget.h"

#include <QDateTime>

class QgsCodeEditor;
class QgsFilterLineEdit;
class QToolButton;
class QCheckBox;
class QgsMessageBar;
class QgsScrollBarHighlightController;

SIP_IF_MODULE( HAVE_QSCI_SIP )

/**
 * \ingroup gui
 * \brief A widget which wraps a QgsCodeEditor in additional functionality.
 *
 * This widget wraps an existing QgsCodeEditor object in a widget which provides
 * additional standard functionality, such as search/replace tools. The caller
 * must create an unparented QgsCodeEditor object (or a subclass of QgsCodeEditor)
 * first, and then construct a QgsCodeEditorWidget passing this object to the
 * constructor.
 *
 * \note may not be available in Python bindings, depending on platform support
 *
 * \since QGIS 3.38
 */
class GUI_EXPORT QgsCodeEditorWidget : public QgsPanelWidget
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsCodeEditorWidget, wrapping the specified \a editor widget.
     *
     * Ownership of \a editor will be transferred to this widget.
     *
     * If an explicit \a messageBar is specified then it will be used to provide
     * feedback, otherwise an integrated message bar will be used.
     */
    QgsCodeEditorWidget( QgsCodeEditor *editor SIP_TRANSFER, QgsMessageBar *messageBar = nullptr, QWidget *parent SIP_TRANSFERTHIS = nullptr );
    ~QgsCodeEditorWidget() override;

    void resizeEvent( QResizeEvent *event ) override;
    void showEvent( QShowEvent *event ) override;
    bool eventFilter( QObject *obj, QEvent *event ) override;

    /**
     * Returns the wrapped code editor.
     */
    QgsCodeEditor *editor() { return mEditor; }

    /**
     * Returns TRUE if the search bar is visible.
     */
    bool isSearchBarVisible() const;

    /**
     * Returns the message bar associated with the widget, to use for user feedback.
     */
    QgsMessageBar *messageBar();

    /**
     * Returns the scrollbar highlight controller, which can be used to add highlights
     * in the code editor scrollbar.
     */
    QgsScrollBarHighlightController *scrollbarHighlightController();

    /**
     * Adds a \a warning message and indicator to the specified a \a lineNumber.
     *
     * This method calls QgsCodeEditor::addWarning(), but also automatically adds
     * highlights to the widget scrollbars locating the warning location.
     *
     * \see clearWarnings()
     */
    void addWarning( int lineNumber, const QString &warning );

    /**
     * Clears all warning messages from the editor.
     *
     * This method calls QgsCodeEditor::clearWarnings(), but also removes
     * highlights from the widget scrollbars at the warning locations.
     *
     * \see addWarning()
     */
    void clearWarnings();

    /**
     * Returns the widget's associated file path.
     *
     * \see setFilePath()
     * \see filePathChanged()
     */
    QString filePath() const { return mFilePath; }

    /**
     * Saves the code editor content into the file \a path.
     * \returns FALSE if the file path has not previously been set, or if writing the file fails.
     * \note When the path is empty, the content will be saved to the current file path if not empty.
     * \since QGIS 3.38.2
     */
    bool save( const QString &path = QString() );

  public slots:

    /**
     * Shows the search bar.
     *
     * \see hideSearchBar()
     * \see setSearchBarVisible()
     */
    void showSearchBar();

    /**
     * Hides the search bar.
     *
     * \see showSearchBar()
     * \see setSearchBarVisible()
     */
    void hideSearchBar();

    /**
     * Sets whether the search bar is \a visible.
     *
     * \see showSearchBar()
     * \see hideSearchBar()
     * \see setReplaceBarVisible()
     */
    void setSearchBarVisible( bool visible );

    /**
     * Sets whether the replace bar is \a visible.
     *
     * \see setSearchBarVisible()
     */
    void setReplaceBarVisible( bool visible );

    /**
     * Triggers a find operation, using the default behavior.
     *
     * This will automatically open the search bar and start a find operation using
     * the default behavior, e.g. searching for any selected text in the code editor.
     */
    void triggerFind();

    /**
     * Loads the file at the specified \a path into the widget, replacing the code editor's
     * content with that from the file.
     *
     * This automatically sets the widget's filePath()
     *
     * Returns TRUE if the file was loaded successfully.
     */
    bool loadFile( const QString &path );

    /**
     * Sets the widget's associated file \a path.
     *
     * \see loadFile()
     * \see filePathChanged()
     * \see filePath()
     */
    void setFilePath( const QString &path );

    /**
     * Attempts to opens the script from the editor in an external text editor.
     *
     * This requires that the widget has an associated filePath() set.
     *
     * Optionally a target \a line and \a column number can be specified to open the editor
     * at the corresponding location. (Not all external editors support this.) Line/column
     * numbers of -1 indicate that the current cursor position should be used. A \a line
     * number of 0 corresponds to the first line, and a column number of 0 corresponds to
     * the first column.
     *
     * \returns TRUE if the file was opened successfully.
     */
    bool openInExternalEditor( int line = -1, int column = -1 );

    /**
     * Shares the contents of the code editor on GitHub Gist.
     *
     * Requires that the user has configured an API token with appropriate permission in advance.
     *
     * \returns FALSE if the user has not configured a GitHub personal access token.
     */
    bool shareOnGist( bool isPublic );

  signals:

    /**
     * Emitted when the visibility of the search bar is changed.
     */
    void searchBarToggled( bool visible );

    /**
     * Emitted when the widget's associated file path is changed.
     *
     * \see setFilePath()
     * \see filePath()
     */
    void filePathChanged( const QString &path );

    /**
     * Emitted when the widget loads in text from the associated file to bring in
     * changes made externally to the file.
     */
    void loadedExternalChanges();

  private slots:

    bool findNext();
    void findPrevious();
    void textSearchChanged( const QString &text );
    void updateSearch();
    void replace();
    void replaceSelection();
    void replaceAll();

  private:
    void clearSearchHighlights();
    void addSearchHighlights();
    int searchFlags() const;
    bool findText( bool forward, bool findFirst );
    void updateHighlightController();
    void searchMatchCountChanged( int matchCount );

    enum HighlightCategory
    {
      SearchMatch = 0,
      Warning = 1
    };

    QgsCodeEditor *mEditor = nullptr;
    QWidget *mFindWidget = nullptr;
    QgsFilterLineEdit *mLineEditFind = nullptr;
    QgsFilterLineEdit *mLineEditReplace = nullptr;
    QToolButton *mFindPrevButton = nullptr;
    QToolButton *mFindNextButton = nullptr;
    QToolButton *mCaseSensitiveButton = nullptr;
    QToolButton *mWholeWordButton = nullptr;
    QToolButton *mRegexButton = nullptr;
    QToolButton *mWrapAroundButton = nullptr;
    QToolButton *mShowReplaceBarButton = nullptr;
    QToolButton *mReplaceButton = nullptr;
    QToolButton *mReplaceAllButton = nullptr;
    int mBlockSearching = 0;
    QgsMessageBar *mMessageBar = nullptr;
    std::unique_ptr<QgsScrollBarHighlightController> mHighlightController;
    QString mFilePath;
    QDateTime mLastModified;
};

#endif // QGSCODEEDITORWIDGET_H
