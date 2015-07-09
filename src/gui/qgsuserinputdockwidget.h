/***************************************************************************
    qgsuserinputdockwidget.h
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



#ifndef QGSUSERINPUTDOCKWIDGET_H
#define QGSUSERINPUTDOCKWIDGET_H

#include <QDockWidget>
#include <QMap>

class QFrame;
class QBoxLayout;


/**
 * @brief The QgsUserInputDockWidget class is a dock widget that shall be used to display widgets for user inputs.
 * It can be used by map tools, plugins, etc.
 * Several widgets can be displayed at once, they will be separated by a separator. Widgets will be either layout horizontally or vertically.
 * The dock is automatically hidden if it contains no widget.
 */
class GUI_EXPORT QgsUserInputDockWidget : public QDockWidget
{
    Q_OBJECT
  public:
    QgsUserInputDockWidget( QWidget* parent = 0 );
    ~QgsUserInputDockWidget();

    //! add a widget to be displayed in the dock
    void addUserInputWidget( QWidget* widget );

  protected:
    //! will not display the dock if it contains no widget
    void paintEvent( QPaintEvent *event ) override;

  private slots:
    void widgetDestroyed( QObject* obj );

    //! when area change, update the layout according to the new dock location
    void areaChanged( Qt::DockWidgetArea area );
    void floatingChanged( bool floating );

  private:
    //! change layout according to dock location
    void updateLayoutDirection();

    // list of widget with their corresponding line separator
    QMap<QWidget*, QFrame*> mWidgetList;

    bool mLayoutHorizontal;
    QBoxLayout* mLayout;
};

#endif // QGSUSERINPUTDOCKWIDGET_H
