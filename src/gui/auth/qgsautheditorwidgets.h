/***************************************************************************
    qgsautheditorwidgets.h
    ---------------------
    begin                : April 26, 2015
    copyright            : (C) 2015 by Boundless Spatial, Inc. USA
    author               : Larry Shaffer
    email                : lshaffer at boundlessgeo dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSAUTHEDITORWIDGETS_H
#define QGSAUTHEDITORWIDGETS_H

#include <QWidget>
#include "ui_qgsautheditorwidgets.h"

class QTabWidget;

/** \ingroup gui
 * Widget for tabbed interface to separately available authentication editors
 */
class GUI_EXPORT QgsAuthEditorWidgets : public QWidget, private Ui::QgsAuthEditors
{
    Q_OBJECT

  public:
    /**
     * Construct a widget to contain various authentication editors
     * @param parent Parent widget
     */
    explicit QgsAuthEditorWidgets( QWidget *parent = 0 ) :
        QWidget( parent )
    {
      setupUi( this );
    }

    ~QgsAuthEditorWidgets() {}

    /** Get access to embedded tabbed widget */
    QTabWidget * tabbedWidget() { return tabWidget; }
};

#endif // QGSAUTHEDITORWIDGETS_H
