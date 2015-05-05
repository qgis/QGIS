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

class GUI_EXPORT QgsUserInputDockWidget : public QDockWidget
{
    Q_OBJECT
  public:
    QgsUserInputDockWidget( QWidget* parent = 0 );
    ~QgsUserInputDockWidget();

    void addUserInputWidget( QWidget* widget );

  protected:
    void paintEvent( QPaintEvent *event );

  private slots:
    void widgetDestroyed( QObject* obj );

    void areaChanged( Qt::DockWidgetArea area );

  private:
    bool isLayoutHorizontal();

    void createLayout();

    // list of widget with their corresponding line separator
    QMap<QWidget*, QFrame*> mWidgetList;

    Qt::DockWidgetArea mDockArea;
    QBoxLayout* mLayout;
};

#endif // QGSUSERINPUTDOCKWIDGET_H
