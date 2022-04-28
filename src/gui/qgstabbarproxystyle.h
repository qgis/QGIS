/***************************************************************************
  qgstabbarproxystyle.h - QgsTabBarProxyStyle
 ---------------------
 begin                : 25.3.2022
 copyright            : (C) 2022 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSTABBARPROXYSTYLE_H
#define QGSTABBARPROXYSTYLE_H

#include "qgsproxystyle.h"
#include "qgsattributeeditorelement.h"

#include <QProxyStyle>
#include <QFont>
#include <QColor>
#include <QMap>
#include <QTabBar>

#define SIP_NO_FILE

#include "qgis_gui.h"

///@cond private

/**
 * The QgsTabBarProxyStyle class provides a proxy style to set font for a tab bar.
 * This class is an implementation detail and it is not exposed to public API.
 */
class GUI_EXPORT QgsTabBarProxyStyle : public QgsProxyStyle
{
  public:

    QgsTabBarProxyStyle( QTabBar *tabBar );

    void drawControl( ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget ) const override;

    void addStyle( int tabIndex, const QgsAttributeEditorElement::LabelStyle &style );

    const QMap<int, QgsAttributeEditorElement::LabelStyle> &tabStyles() const;

  private:

    QMap<int, QgsAttributeEditorElement::LabelStyle> mTabStyles;

};



/**
 * The QgsTabBar class supports custom fonts for tab's titles and provides the tab bar for QgsTabWidget.
 * This class is an implementation detail and it is not exposed to public API.
 */
class QgsTabBar: public QTabBar
{
    Q_OBJECT

  public:

    /**
     * Create a new QgsTabBar with the optionally provided parent.
     */
    QgsTabBar( QWidget *parent );

    /**
     * Set the \a tabStyle, ownership is not transferred.
     */
    void setTabBarStyle( QgsTabBarProxyStyle *tabStyle );

  protected:

    QSize tabSizeHint( int index ) const;

  private:
    QgsTabBarProxyStyle *mTabBarStyle = nullptr;

};

/// @endcond private

#endif // QGSTABBARPROXYSTYLE_H
