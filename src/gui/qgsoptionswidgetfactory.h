/***************************************************************************
    qgsoptionswidgetfactory.h
     -------------------------------
    Date                 : March 2017
    Copyright            : (C) 2017 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSOPTIONSWIDGETFACTORY_H
#define QGSOPTIONSWIDGETFACTORY_H

#include <QListWidgetItem>
#include "qgis_gui.h"
#include "qgis_sip.h"
#include "qgsoptionsdialoghighlightwidget.h"

/**
 * \ingroup gui
 * \class QgsOptionsPageWidget
 * \brief Base class for widgets for pages included in the options dialog.
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsOptionsPageWidget : public QWidget
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsOptionsPageWidget.
     */
    QgsOptionsPageWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr )
      : QWidget( parent )
    {}

    /**
     * Returns the optional help key for the options page. The default implementation
     * returns an empty string.
     *
     * If a non-empty string is returned by this method, it will be used as the help key
     * retrieved when the "help" button is clicked while this options page is active.
     *
     * If an empty string is returned by this method the default QGIS options
     * help will be retrieved.
     */
    virtual QString helpKey() const { return QString(); }

    /**
     * Returns the registered highlight widgets used to search and highlight text in
     * options dialogs.
     */
    QHash<QWidget *, QgsOptionsDialogHighlightWidget *> registeredHighlightWidgets() {return mHighlightWidgets;} SIP_SKIP

    /**
     * Validates the current state of the widget.
     *
     * Subclasses should return TRUE if the widget state is currently valid and acceptable to apply().
     *
     * The default implementation returns TRUE.
     *
     * \since QGIS 3.24
     */
    virtual bool isValid() { return true; }

  public slots:

    /**
     * Called to permanently apply the settings shown in the options page (e.g. save them to
     * QgsSettings objects). This is usually called when the options dialog is accepted.
     */
    virtual void apply() = 0;

  protected:

    /**
     * Register a highlight widget to be used to search and highlight text in
     * options dialogs. This can be used to provide a custom implementation of
     * QgsOptionsDialogHighlightWidget.
     */
    void registerHighlightWidget( QgsOptionsDialogHighlightWidget *highlightWidget )
    {
      mHighlightWidgets.insert( highlightWidget->widget(), highlightWidget );
    }

  private:
    QHash<QWidget *, QgsOptionsDialogHighlightWidget *> mHighlightWidgets;


};

/**
 * \ingroup gui
 * \class QgsOptionsWidgetFactory
 * \brief A factory class for creating custom options pages.
 * \since QGIS 3.0
 */
// NOTE - this is a QObject so we can detect its destruction and avoid
// QGIS crashing when a plugin crashes/exits without deregistering a factory
class GUI_EXPORT QgsOptionsWidgetFactory : public QObject
{
    Q_OBJECT

  public:

    //! Constructor
    QgsOptionsWidgetFactory() = default;

    //! Constructor
    QgsOptionsWidgetFactory( const QString &title, const QIcon &icon )
      : mTitle( title )
      , mIcon( icon )
    {}

    /**
     * \brief The icon that will be shown in the UI for the panel.
     * \returns A QIcon for the panel icon.
     * \see setIcon()
     */
    virtual QIcon icon() const { return mIcon; }

    /**
     * Set the \a icon to show in the interface for the factory object.
     * \see icon()
     */
    void setIcon( const QIcon &icon ) { mIcon = icon; }

    /**
     * The title of the panel.
     * \see setTitle()
     */
    virtual QString title() const { return mTitle; }

    /**
     * Set the \a title for the interface.
     * \see title()
     */
    void setTitle( const QString &title ) { mTitle = title; }

    /**
     * Returns a tab name hinting at where this page should be inserted into the
     * options properties tab list.
     *
     * If the returned string is non-empty, the options widget page will be inserted
     * before the existing page with matching object name.
     *
     * The default implementation returns an empty string, which causes the widget
     * to be placed at the end of the dialog page list.
     *
     * \since QGIS 3.18
     */
    virtual QString pagePositionHint() const { return QString(); }

    /**
     * Returns the path to place the widget page at, for options dialogs
     * which are structured using a tree view.
     *
     * A factory which returns "Code", "Javascript" would have its widget placed
     * in a group named "Javascript", contained in a parent group named "Code".
     *
     * \since QGIS 3.22
     */
    virtual QStringList path() const { return QStringList(); }

    /**
     * \brief Factory function to create the widget on demand as needed by the options dialog.
     * \param parent The parent of the widget.
     * \returns A new widget to show as a page in the options dialog.
     */
    virtual QgsOptionsPageWidget *createWidget( QWidget *parent = nullptr ) const = 0 SIP_FACTORY;

  private:
    QString mTitle;
    QIcon mIcon;


};

#endif // QGSOPTIONSWIDGETFACTORY_H
