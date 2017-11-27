/***************************************************************************
    qgsoptionsdialogbase.h - base vertical tabs option dialog

    ---------------------
    begin                : March 24, 2013
    copyright            : (C) 2013 by Larry Shaffer
    email                : larrys at dakcarto dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSOPTIONSDIALOGBASE_H
#define QGSOPTIONSDIALOGBASE_H

#include "qgsguiutils.h"
#include "qgssettings.h"
#include "qgis_gui.h"

#include <functional>

#include <QDialog>
#include <QPointer>
#include <QStyledItemDelegate>


class QDialogButtonBox;
class QListWidget;
class QModelIndex;
class QPalette;
class QPainter;
class QStackedWidget;
class QStyleOptionViewItem;
class QSplitter;

class QgsFilterLineEdit;

/**
 * \ingroup gui
 * \class QgsSearchHighlightOptionWidget
 * Container for a widget to be used to search text in the option dialog
 * If the widget type is handled, it is valid.
 * It can perform a text search in the widget and highlight it in case of success.
 * This uses stylesheets.
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsSearchHighlightOptionWidget : public QObject
{
    Q_OBJECT
  public:

    /**
     * Constructor
     * \param widget the widget used to search text into
     */
    explicit QgsSearchHighlightOptionWidget( QWidget *widget = 0 );

    /**
     * Returns if it valid: if the widget type is handled and if the widget is not still available
     */
    bool isValid() {return mValid;}

    /**
     * search for a text pattern and highlight the widget if the text is found
     * \returns true if the text pattern is found
     */
    bool searchHighlight( const QString &searchText );

    /**
     *  reset the style to the original state
     */
    void reset();

    /**
     * return the widget
     */
    QWidget *widget() {return mWidget;}

  private slots:
    void widgetDestroyed();

  private:
    QWidget *mWidget = nullptr;
    QString mStyleSheet;
    bool mValid = true;
    bool mChangedStyle = false;
    std::function < QString() > mText;
};


/**
 * \ingroup gui
 * \class QgsOptionsDialogBase
 * A base dialog for options and properties dialogs that offers vertical tabs.
 * It handles saving/restoring of geometry, splitter and current tab states,
 * switching vertical tabs between icon/text to icon-only modes (splitter collapsed to left),
 * and connecting QDialogButtonBox's accepted/rejected signals to dialog's accept/reject slots
 *
 * To use:
 * 1) Start with copy of qgsoptionsdialog_template.ui and build options/properties dialog.
 * 2) In source file for dialog, inherit this class instead of QDialog, then in constructor:
 *    ...
 *    setupUi( this ); // set up .ui file objects
 *    initOptionsBase( false ); // set up this class to use .ui objects, optionally restoring base ui
 *    ...
 *    restoreOptionsBaseUi(); // restore the base ui with initOptionsBase or use this later on
 */

class GUI_EXPORT QgsOptionsDialogBase : public QDialog
{
    Q_OBJECT

  public:

    /**
     * Constructor
     * \param settingsKey QgsSettings subgroup key for saving/restore ui states, e.g. "ProjectProperties".
     * \param parent parent object (owner)
     * \param fl widget flags
     * \param settings custom QgsSettings pointer
     */
    QgsOptionsDialogBase( const QString &settingsKey, QWidget *parent SIP_TRANSFERTHIS = nullptr, Qt::WindowFlags fl = 0, QgsSettings *settings = nullptr );
    ~QgsOptionsDialogBase();

    /**
     * Set up the base ui connections for vertical tabs.
     * \param restoreUi Whether to restore the base ui at this time.
     * \param title the window title
     */
    void initOptionsBase( bool restoreUi = true, const QString &title = QString() );

    // set custom QgsSettings pointer if dialog used outside QGIS (in plugin)
    void setSettings( QgsSettings *settings );

    /**
     * Restore the base ui.
     * Sometimes useful to do at end of subclass's constructor.
     * \param title the window title (it does not need to be defined if previously given to initOptionsBase();
     */
    void restoreOptionsBaseUi( const QString &title = QString() );

    /**
     * Determine if the options list is in icon only mode
     */
    bool iconOnly() {return mIconOnly;}

  public slots:

    /**
     * searchText searches for a text in all the pages of the stacked widget and highlight the results
     * \param text the text to search
     * \since QGIS 3.0
     */
    void searchText( const QString &text );

  protected slots:
    //! Update tabs on the splitter move
    virtual void updateOptionsListVerticalTabs();
    //! Select relevant tab on current page change
    virtual void optionsStackedWidget_CurrentChanged( int index );
    //! Remove tab and unregister widgets on page remove
    virtual void optionsStackedWidget_WidgetRemoved( int index );

    void warnAboutMissingObjects();

  protected:
    void showEvent( QShowEvent *e ) override;
    void paintEvent( QPaintEvent *e ) override;

    virtual void updateWindowTitle();

    /**
     * register widgets in the dialog to search for text in it
     * it is automatically called if a line edit has "mSearchLineEdit" as object name.
     * \since QGIS 3.0
     */
    void registerTextSearchWidgets();

    QList< QPair< QgsSearchHighlightOptionWidget *, int > > mRegisteredSearchWidgets;

    QString mOptsKey;
    bool mInit;
    QListWidget *mOptListWidget = nullptr;
    QStackedWidget *mOptStackedWidget = nullptr;
    QSplitter *mOptSplitter = nullptr;
    QDialogButtonBox *mOptButtonBox = nullptr;
    QgsFilterLineEdit *mSearchLineEdit = nullptr;
    QString mDialogTitle;
    bool mIconOnly;
    // pointer to app or custom, external QgsSettings
    // QPointer in case custom settings obj gets deleted while dialog is open
    QPointer<QgsSettings> mSettings;
    bool mDelSettings;
};

#endif // QGSOPTIONSDIALOGBASE_H
