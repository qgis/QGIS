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

#include "qgisgui.h"
#include <functional>

#include <QDialog>
#include <QPointer>
#include <QSettings>
#include <QStyledItemDelegate>
#include "qgis_gui.h"


class QDialogButtonBox;
class QListWidget;
class QModelIndex;
class QPalette;
class QPainter;
class QStackedWidget;
class QStyleOptionViewItem;
class QSplitter;

class QgsFilterLineEdit;


class GUI_EXPORT QgsSearchHighlightOptionWidget
{
  public:
    explicit QgsSearchHighlightOptionWidget( QWidget* widget = 0 );
    ~QgsSearchHighlightOptionWidget();

    bool isValid();

    bool searchHighlight( QString searchText );

    void reset();

    QString widgetName();

  private:
    QWidget* mWidget;
    QPalette mOriginalPalette;
    QPalette::ColorRole mColorRole;
    QColor mColor;
    std::function < QString( QWidget* ) > mText;
};


/** \ingroup gui
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

    /** Constructor
     * @param settingsKey QSettings subgroup key for saving/restore ui states, e.g. "ProjectProperties".
     * @param parent parent object (owner)
     * @param fl widget flags
     * @param settings custom QSettings pointer
     */
    QgsOptionsDialogBase( const QString& settingsKey, QWidget* parent = nullptr, Qt::WindowFlags fl = 0, QSettings* settings = nullptr );
    ~QgsOptionsDialogBase();

    /** Set up the base ui connections for vertical tabs.
     * @param restoreUi Whether to restore the base ui at this time.
     * @param title the window title
     */
    void initOptionsBase( bool restoreUi = true, const QString& title = QString() );

    // set custom QSettings pointer if dialog used outside QGIS (in plugin)
    void setSettings( QSettings* settings );

    /** Restore the base ui.
     * Sometimes useful to do at end of subclass's constructor.
     * @param title the window title (it does not need to be defined if previously given to initOptionsBase();
     */
    void restoreOptionsBaseUi( const QString& title = QString() );

    /** Determine if the options list is in icon only mode
     */
    bool iconOnly() {return mIconOnly;}

  public slots:

    /**
     * searchText searches for a text in all the pages of the stacked widget and highlight the results
     * @param text the text to search
     */
    void searchText( QString text );

  protected slots:
    void updateOptionsListVerticalTabs();
    void optionsStackedWidget_CurrentChanged( int indx );
    void optionsStackedWidget_WidgetRemoved( int indx );
    void warnAboutMissingObjects();

  protected:
    void showEvent( QShowEvent* e ) override;
    void paintEvent( QPaintEvent* e ) override;

    virtual void updateWindowTitle();

    void registerTextSearch();

    QList< QPair< QgsSearchHighlightOptionWidget, int > > mRegisteredSearchWidgets;

    QString mOptsKey;
    bool mInit;
    QListWidget* mOptListWidget;
    QStackedWidget* mOptStackedWidget;
    QSplitter* mOptSplitter;
    QDialogButtonBox* mOptButtonBox;
    QgsFilterLineEdit* mSearchLineEdit;
    QString mDialogTitle;
    bool mIconOnly;
    // pointer to app or custom, external QSettings
    // QPointer in case custom settings obj gets deleted while dialog is open
    QPointer<QSettings> mSettings;
    bool mDelSettings;
};

#endif // QGSOPTIONSDIALOGBASE_H
