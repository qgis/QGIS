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

/** \ingroup gui
 * \class QgsOptionsPageWidget
 * Base class for widgets for pages included in the options dialog.
 * \note added in QGIS 3.0
 */
class GUI_EXPORT QgsOptionsPageWidget : public QWidget
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsOptionsPageWidget.
     */
    QgsOptionsPageWidget( QWidget *parent = nullptr )
      : QWidget( parent )
    {}

  public slots:

    /**
     * Called to permanently apply the settings shown in the options page (e.g. save them to
     * QgsSettings objects). This is usually called when the options dialog is accepted.
     */
    virtual void apply() = 0;

};

/** \ingroup gui
 * \class QgsOptionsWidgetFactory
 * A factory class for creating custom options pages.
 * \note added in QGIS 3.0
 */
class GUI_EXPORT QgsOptionsWidgetFactory
{
  public:

    //! Constructor
    QgsOptionsWidgetFactory() {}

    //! Constructor
    QgsOptionsWidgetFactory( const QString &title, const QIcon &icon )
      : mTitle( title )
      , mIcon( icon )
    {}

    virtual ~QgsOptionsWidgetFactory() = default;

    /**
     * @brief The icon that will be shown in the UI for the panel.
     * @return A QIcon for the panel icon.
     * @see setIcon()
     */
    virtual QIcon icon() const { return mIcon; }

    /**
     * Set the \a icon to show in the interface for the factory object.
     * @see icon()
     */
    void setIcon( const QIcon &icon ) { mIcon = icon; }

    /**
     * The title of the panel.
     * @see setTitle()
     */
    virtual QString title() const { return mTitle; }

    /**
     * Set the \a title for the interface.
     * @see title()
     */
    void setTitle( const QString &title ) { mTitle = title; }

    /**
     * @brief Factory function to create the widget on demand as needed by the options dialog.
     * @param parent The parent of the widget.
     * @return A new widget to show as a page in the options dialog.
     */
    virtual QgsOptionsPageWidget *createWidget( QWidget *parent = nullptr ) const = 0;

  private:
    QString mTitle;
    QIcon mIcon;


};

#endif // QGSOPTIONSWIDGETFACTORY_H
