/***************************************************************************
    qgsuserinputtoolbar.h
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



#ifndef QGSUSERINPUTTOOLBAR_H
#define QGSUSERINPUTTOOLBAR_H

#include <QToolBar>
#include <QMap>

class GUI_EXPORT QgsUserInputToolBar : public QToolBar
{
    Q_OBJECT
  public:
    QgsUserInputToolBar( QWidget* parent = 0 );
    ~QgsUserInputToolBar();

    void addUserInputWidget( QWidget* widget );

  protected:
    void paintEvent( QPaintEvent *event );

  private slots:
    void widgetDestroyed( QObject* obj );

  private:
    // list of widget with their corresponding separator
    QMap<QWidget*, QAction*> mWidgetList;
};

#endif // QGSUSERINPUTTOOLBAR_H
