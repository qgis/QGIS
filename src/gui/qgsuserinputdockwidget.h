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

    //! add a widget to be displayed in the dock
    void addUserInputWidget( QWidget* widget );

  protected:
    void paintEvent( QPaintEvent *event ) override;

  private slots:
    void widgetDestroyed( QObject* obj );

    //! when area change, update the layout according to the new dock location
    void areaChanged( Qt::DockWidgetArea area );
    void floatingChanged( bool floating );

  private:
    void createLayout();

    void updateLayoutDirection();

    // list of widget with their corresponding line separator
    QMap<QWidget*, QFrame*> mWidgetList;

    bool mLayoutHorizontal;
    QBoxLayout* mLayout;
};

#endif // QGSUSERINPUTDOCKWIDGET_H
