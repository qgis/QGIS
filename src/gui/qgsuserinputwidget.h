/***************************************************************************
    qgsuserinputwidget.h
     --------------------------------------
    Date                 : 04.2015
    Copyright            : (C) 2015 Denis Rouzaud
    Email                : denis.rouzaud@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSUSERINPUTWIDGET_H
#define QGSUSERINPUTWIDGET_H

#include "qgis.h"
#include "qgis_gui.h"
#include "qgsfloatingwidget.h"

#include <QMap>
#include <QBoxLayout>

class QBoxLayout;
class QFrame;


/**
 * \ingroup gui
 * \brief The QgsUserInputWidget class is a floating widget that shall be used to display widgets for user inputs.
 * It can be used by map tools, plugins, etc.
 * Several widgets can be displayed at once, they will be separated by a separator.
 * Widgets will be either layout horizontally or vertically.
 * The widget is automatically hidden if it contains no widget.
 * \since QGIS 2.10
 */
class GUI_EXPORT QgsUserInputWidget : public QgsFloatingWidget
{
    Q_OBJECT
  public:

    //! Constructor for QgsUserInputWidget
    QgsUserInputWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Add a widget to be displayed in the dock.
     * \param widget widget to add. Ownership is not transferred.
     */
    void addUserInputWidget( QWidget *widget );

  protected:
    // will not display the dock if it contains no widget
    void paintEvent( QPaintEvent *event ) override;

  private slots:
    void widgetDestroyed( QObject *obj );

  private:
    //! change layout direction
    void setLayoutDirection( QBoxLayout::Direction direction );

    // list of widget with their corresponding line separator
    QMap<QWidget *, QFrame *> mWidgetList;

    bool mLayoutHorizontal = true;
    QBoxLayout *mLayout = nullptr;
};

#endif // QGSUSERINPUTWIDGET_H
