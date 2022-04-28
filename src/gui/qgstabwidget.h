/***************************************************************************
  qgstabwidget.h - QgsTabWidget

 ---------------------
 begin                : 8.9.2016
 copyright            : (C) 2016 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSTABWIDGET_H
#define QGSTABWIDGET_H

#include <QTabWidget>
#include "qgstabbarproxystyle.h"
#include "qgsattributeeditorelement.h"

#include "qgis_gui.h"

/**
 * \ingroup gui
 * \brief The QgsTabWidget class is the same as the QTabWidget but with additional methods to
 * temporarily hide/show tabs.
 *
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsTabWidget : public QTabWidget
{
    Q_OBJECT

  public:

    /**
     * Create a new QgsTabWidget with the optionally provided parent.
     *
     * \since QGIS 3.0
     */
    QgsTabWidget( QWidget *parent = nullptr );

    /**
     * Hides the tab with the given widget
     *
     * \since QGIS 3.0
     */
    void hideTab( QWidget *tab );

    /**
     * Shows the tab with the given widget
     *
     * \since QGIS 3.0
     */
    void showTab( QWidget *tab );

    /**
     * Control the visibility for the tab with the given widget.
     *
     * \since QGIS 3.0
     */
    void setTabVisible( QWidget *tab, bool visible );

    /**
     * Returns the index of the tab with the given widget.
     * This index is not the same as the one provided to insertTab and removeTab
     * since these methods are not aware of hidden tabs.
     *
     * \since QGIS 3.0
     */
    int realTabIndex( QWidget *widget );

    /**
     * Is called internally whenever a new tab has been inserted.
     *
     * Is used to keep track of currently available and visible tabs.
     *
     * \since QGIS 3.0
     */
    void tabInserted( int index ) override;

    /**
     * Is called internally whenever a tab has been removed.
     *
     * Is used to keep track of currently available and visible tabs.
     *
     * \since QGIS 3.0
     */
    void tabRemoved( int index ) override;

    /**
     * Sets the optional custom \a labelStyle for the tab identified by \a tabIndex.
     * \since QGIS 3.26
     */
    void setTabStyle( int tabIndex, const QgsAttributeEditorElement::LabelStyle &labelStyle );

  private:
    void synchronizeIndexes();

    struct TabInformation
    {
      TabInformation( QWidget *wdg, const QString &lbl )
        : widget( wdg )
        , label( lbl )
      {}

      //! Constructor for TabInformation
      TabInformation() = default;

      bool operator ==( const TabInformation &other ) const;
      bool operator !=( const TabInformation &other ) const;

      int sourceIndex = -1;
      QWidget *widget = nullptr;
      QString label;
      bool visible = true;
    };

    TabInformation tabInfo( QWidget *widget );

    QList<TabInformation> mTabs;
    bool mSetTabVisibleFlag = false;
    QgsTabBarProxyStyle *mTabBarStyle = nullptr;
};

#endif // QGSTABWIDGET_H
