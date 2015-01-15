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

class QgsMessageBarItem;

/** \ingroup gui
 * A bar for displaying non-blocking messages to the user.
 */
class GUI_EXPORT QgsMessageBar: public QFrame
{
    Q_OBJECT

  public:
    enum MessageLevel
    {
      INFO = 0,
      WARNING = 1,
      CRITICAL = 2,
      SUCCESS = 3
    };

    QgsMessageBar( QWidget *parent = 0 );
    ~QgsMessageBar();

    /*! display a message item on the bar after hiding the currently visible one
     *  and putting it in a stack.
     * @param item item to display
     */
    void pushItem( QgsMessageBarItem *item );

    /*! display a widget as a message on the bar after hiding the currently visible one
     *  and putting it in a stack.
     * @param widget message widget to display
     * @param level is QgsMessageBar::INFO, WARNING, CRITICAL or SUCCESS
     * @param duration timeout duration of message in seconds, 0 value indicates no timeout
     */
    QgsMessageBarItem *pushWidget( QWidget *widget, MessageLevel level = INFO, int duration = 0 );

    /*! remove the passed widget from the bar (if previously added),
     *  then display the next one in the stack if any or hide the bar
     *  @param item item to remove
     *  @return true if the widget was removed, false otherwise
     */
    bool popWidget( QgsMessageBarItem *item );

    //! make out a widget containing a message to be displayed on the bar
    static QgsMessageBarItem* createMessage( const QString &text, QWidget *parent = 0 );
    //! make out a widget containing title and message to be displayed on the bar
    static QgsMessageBarItem* createMessage( const QString &title, const QString &text, QWidget *parent = 0 );
    //! make out a widget containing title and message to be displayed on the bar
    static QgsMessageBarItem* createMessage( QWidget *widget, QWidget *parent = 0 );

    //! convenience method for pushing a message to the bar
    void pushMessage( const QString &text, MessageLevel level = INFO, int duration = 0 ) { return pushMessage( QString::null, text, level, duration ); }
    //! convenience method for pushing a message with title to the bar
    void pushMessage( const QString &title, const QString &text, MessageLevel level = INFO, int duration = 0 );

  signals:
    //! emitted when a message widget is added to the bar
    void widgetAdded( QgsMessageBarItem *item );

    //! emitted when a message widget was removed from the bar
    void widgetRemoved( QgsMessageBarItem *item );

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

    /**
     * Pushes a warning with default timeout to the message bar
     * @param message The message to be displayed
     * @note added in 2.8
     */
    void pushSuccess( const QString& title, const QString& message );

    /**
     * Pushes a warning with default timeout to the message bar
     * @param message The message to be displayed
     * @note added in 2.8
     */
    void pushInfo( const QString& title, const QString& message );

    /**
     * Pushes a warning with default timeout to the message bar
     * @param message The message to be displayed
     * @note added in 2.8
     */
    void pushWarning( const QString& title, const QString& message );

    /**
     * Pushes a warning with default timeout to the message bar
     * @param message The message to be displayed
     * @note added in 2.8
     */
    void pushCritical( const QString& title, const QString& message );

  protected:
    void mousePressEvent( QMouseEvent * e ) override;

  private:
    void popItem( QgsMessageBarItem *item );
    void showItem( QgsMessageBarItem *item );
    QgsMessageBarItem *mCurrentItem;
    QList<QgsMessageBarItem *> mItems;
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
