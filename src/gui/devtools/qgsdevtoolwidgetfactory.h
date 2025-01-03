/***************************************************************************
    qgsdevtoolwidgetfactory.h
     ------------------------
    Date                 : March 2020
    Copyright            : (C) 2020 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSDEVTOOLWIDGETFACTORY_H
#define QGSDEVTOOLWIDGETFACTORY_H

#include "qgis_gui.h"
#include "qgis_sip.h"
#include <QString>
#include <QIcon>

class QgsDevToolWidget;
class QWidget;

/**
 * \ingroup gui
 * \class QgsDevToolWidgetFactory
 * \brief Factory class for creating custom developer/debugging tool pages
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsDevToolWidgetFactory
{
  public:
    /**
     * Constructor for a QgsDevToolWidgetFactory with the specified \a title and \a icon.
     */
    QgsDevToolWidgetFactory( const QString &title = QString(), const QIcon &icon = QIcon() );

    virtual ~QgsDevToolWidgetFactory() = default;

    /**
     * Returns the icon that will be shown in the tool in the panel.
     * \see setIcon()
     */
    virtual QIcon icon() const { return mIcon; }

    /**
     * Sets the \a icon for the factory object, which will be shown for the tool in the panel.
     * \see icon()
     */
    void setIcon( const QIcon &icon ) { mIcon = icon; }

    /**
     * Returns the (translated) title of the tool.
     * \see setTitle()
     */
    virtual QString title() const { return mTitle; }

    /**
     * Set the translated \a title for the tool.
     */
    void setTitle( const QString &title ) { mTitle = title; }

    /**
     * Factory function to create the widget on demand as needed by the dock.
     *
     * The \a parent argument gives the correct parent for the newly created widget.
     */
    virtual QgsDevToolWidget *createWidget( QWidget *parent = nullptr ) const = 0 SIP_FACTORY;

  private:
    QIcon mIcon;
    QString mTitle;
};

#endif // QGSDEVTOOLWIDGETFACTORY_H
