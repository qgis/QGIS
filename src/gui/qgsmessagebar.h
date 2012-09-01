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
 * \note added in 1.9
 */
class GUI_EXPORT QgsMessageBar: public QFrame
{
    Q_OBJECT

  public:
    QgsMessageBar( QWidget *parent = 0 );
    ~QgsMessageBar();

    /*! display a widget on the bar after hiding the currently visible one
     *  and putting it in a stack
     * @param widget widget to add
     * @param level is 0 for information, 1 for warning, 2 for critical
     */
    void pushWidget( QWidget *widget, int level = 0 );

    /*! remove the passed widget from the bar (if previously added),
     *  then display the next one in the stack if any or hide the bar
     *  @param widget widget to remove
     *  @return true if the widget was removed, false otherwise
     */
    bool popWidget( QWidget *widget );

    //! make out a widget containing a message to be displayed on the bar
    static QWidget* createMessage( const QString &text, QWidget *parent = 0 ) { return createMessage( QString::null, text, QIcon(), parent ); }
    //! make out a widget containing icon and message to be displayed on the bar
    static QWidget* createMessage( const QString &text, const QIcon &icon, QWidget *parent = 0 ) { return createMessage( QString::null, text, icon, parent ); }
    //! make out a widget containing title and message to be displayed on the bar
    static QWidget* createMessage( const QString &title, const QString &text, QWidget *parent = 0 ) { return createMessage( title, text, QIcon(), parent ); }
    //! make out a widget containing icon, title and message to be displayed on the bar
    static QWidget* createMessage( const QString &title, const QString &text, const QIcon &icon, QWidget *parent = 0 );

  signals:
    //! emitted when a widget was removed from the bar
    void widgetRemoved( QWidget *widget );

  public slots:
    /*! remove the currently displayed widget from the bar and
     *  display the next in the stack if any or hide the bar
     *  @return true if the widget was removed, false otherwise
     */
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

    //! display a widget on the bar
    void pushWidget( QWidget *widget, const QString &styleSheet );

    void popItem( QgsMessageBarItem *item );
    void pushItem( QgsMessageBarItem *item );

    QgsMessageBarItem *mCurrentItem;
    QList<QgsMessageBarItem *> mList;
    QToolButton *mCloseBtn;
    QGridLayout *mLayout;
};

#endif
