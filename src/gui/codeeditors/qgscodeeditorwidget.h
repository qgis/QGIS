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
    QgsCodeEditorWidget( QgsCodeEditor *editor SIP_TRANSFER,
                         QgsMessageBar *messageBar = nullptr,
                         QWidget *parent SIP_TRANSFERTHIS = nullptr );
    ~QgsCodeEditorWidget() override;

    void resizeEvent( QResizeEvent *event ) override;
    void showEvent( QShowEvent *event ) override;

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

  signals:

    /**
     * Emitted when the visibility of the search bar is changed.
     */
    void searchBarToggled( bool visible );

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
    std::unique_ptr< QgsScrollBarHighlightController > mHighlightController;
};

#endif // QGSCODEEDITORWIDGET_H
