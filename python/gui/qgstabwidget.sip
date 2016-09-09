/***************************************************************************
  qgstabwidget.sip - QgsTabWidget

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

/** \ingroup gui
 * The QgsTabWidget class is the same as the QTabWidget but with additional methods to
 * temporarily hide/show tabs.
 *
 * @note Added in QGIS 3.0
 */
class QgsTabWidget : QTabWidget
{
%TypeHeaderCode
#include <qgstabwidget.h>
%End

  public:
    /**
     * Create a new QgsTabWidget with the optionally provided parent.
     *
     * @note Added in QGIS 3.0
     */
    QgsTabWidget( QWidget *parent = nullptr );

    /**
     * Hides the tab with the given widget
     *
     * @note Added in QGIS 3.0
     */
    void hideTab( QWidget* tab );

    /**
     * Shows the tab with the given widget
     *
     * @note Added in QGIS 3.0
     */
    void showTab( QWidget* tab );

    /**
     * Control the visibility for the tab with the given widget.
     *
     * @note Added in QGIS 3.0
     */
    void setTabVisible( QWidget* tab, bool visible );

    /**
     * Returns the index of the tab with the given widget.
     * This index is not the same as the one provided to insertTab and removeTab
     * since these methods are not aware of hidden tabs.
     *
     * @note Added in QGIS 3.0
     */
    int realTabIndex( QWidget* widget );

    virtual void tabInserted( int index );
    virtual void tabRemoved( int index );
};
