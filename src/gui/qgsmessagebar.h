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
class QMenu;
class QProgressBar;
class QToolButton;
class QLabel;
class QAction;
class QTimer;

/** \ingroup gui
 * A bar for displaying non-blocking messages to the user.
 * \note added in 1.9
 */
class GUI_EXPORT QgsMessageBar: public QFrame
{
    Q_OBJECT

  public:
    enum MessageLevel
    {
      INFO = 0,
      WARNING = 1,
      CRITICAL = 2
    };

    QgsMessageBar( QWidget *parent = 0 );
    ~QgsMessageBar();

    /*! display a widget on the bar after hiding the currently visible one
     *  and putting it in a stack
     * @param widget widget to add
     * @param level is QgsMessageBar::INFO, WARNING or CRITICAL
     * @param duration timeout duration of message in seconds, 0 value indicates no timeout
     */
    void pushWidget( QWidget *widget, MessageLevel level = INFO, int duration = 0 );

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

    //! convenience method for pushing a non-widget-based message to the bar
    void pushMessage( const QString &text, MessageLevel level = INFO, int duration = 0 ) { pushMessage( QString::null, text, level, duration ); }
    //! convenience method for pushing a non-widget-based message with title to the bar
    void pushMessage( const QString &title, const QString &text, MessageLevel level = INFO, int duration = 0 );

  signals:
    //! emitted when a message widget is added to the bar
    void widgetAdded( QWidget *widget );

    //! emitted when a message widget was removed from the bar
    void widgetRemoved( QWidget *widget );

  public slots:
    /*! remove the currently displayed widget from the bar and
     *  display the next in the stack if any or hide the bar
     *  @return true if the widget was removed, false otherwise
     */
    bool popWidget();

    /*! remove all items from the bar's widget list
     *  @return true if all items were removed, false otherwise
     */
    bool clearWidgets();

  protected:
    void mousePressEvent( QMouseEvent * e );

  private:
    class QgsMessageBarItem
    {
      public:
        QgsMessageBarItem( QWidget *widget, const QString &styleSheet, int duration = 0 ):
            mWidget( widget ), mStyleSheet( styleSheet ), mDuration( duration ) {}
        ~QgsMessageBarItem() {}

        QWidget* widget() const { return mWidget; }
        QString styleSheet() const { return mStyleSheet; }
        int duration() const { return mDuration; }

      private:
        QWidget *mWidget;
        QString mStyleSheet;
        int mDuration; // 0 value indicates no timeout duration
    };

    void pushWidget( QWidget *widget, const QString &styleSheet, int duration = 0 );

    void popItem( QgsMessageBarItem *item );
    void pushItem( QgsMessageBarItem *item );

    QgsMessageBarItem *mCurrentItem;
    QList<QgsMessageBarItem *> mList;
    QMenu *mCloseMenu;
    QToolButton *mCloseBtn;
    QGridLayout *mLayout;
    QLabel *mItemCount;
    QAction *mActionCloseAll;
    QTimer *mCountdownTimer;
    QProgressBar *mCountProgress;
    QString mCountStyleSheet;

  private slots:
    //! updates count of items in widget list
    void updateItemCount();

    //! updates the countdown for widgets that have a timeout duration
    void updateCountdown();
    void resetCountdown();
};

#endif
