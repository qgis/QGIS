/***************************************************************************
                          qgsmessagebar.h  -  description
                             -------------------
    begin                : June 2012
    copyright            : (C) 2012 by Giuseppe Sucameli
    email                : sucameli at faunalia dot it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSMESSAGEBAR_H
#define QGSMESSAGEBAR_H

#include <qgisgui.h>

#include <QString>
#include <QFrame>
#include <QIcon>
#include <QColor>
#include <QList>

class QWidget;
class QGridLayout;
class QToolButton;

/** \ingroup gui
 * A bar for displaying non-blocking messages to the user.
 * \note added in 1.8
 */
class QgsMessageBar: public QFrame
{
    Q_OBJECT
  public:
    QgsMessageBar( QWidget *parent = 0 );
    ~QgsMessageBar();

    void pushWidget( QWidget *widget, int level = 0 );
    void pushWidget( QWidget *widget, const QString &styleSheet );

    bool popWidget( QWidget *widget );

    static QWidget* createMessage( const QString &text, QWidget *parent = 0 ) { return createMessage( QString::null, text, QIcon(), parent ); }
    static QWidget* createMessage( const QString &text, const QIcon &icon, QWidget *parent = 0 ) { return createMessage( QString::null, text, icon, parent ); }
    static QWidget* createMessage( const QString &title, const QString &text, QWidget *parent = 0 ) { return createMessage( title, text, QIcon(), parent ); }
    static QWidget* createMessage( const QString &title, const QString &text, const QIcon &icon, QWidget *parent = 0 );

  signals:
    void widgetRemoved( QWidget *widget );

  public slots:
    bool popWidget();

  private:
    class QgsMessageBarItem
    {
      public:
        QgsMessageBarItem( QWidget *widget, const QString &styleSheet ):
            mWidget( widget ), mStyleSheet( styleSheet ) {}
        ~QgsMessageBarItem() {}

        QWidget* widget() const { return mWidget; }
        QString styleSheet() const { return mStyleSheet; }

      private:
        QWidget *mWidget;
        QString mStyleSheet;
    };

    void popItem( QgsMessageBarItem *item );
    void pushItem( QgsMessageBarItem *item );

    QgsMessageBarItem *mCurrentItem;
    QList<QgsMessageBarItem *> mList;
    QToolButton *mCloseBtn;
    QGridLayout *mLayout;
};

#endif
