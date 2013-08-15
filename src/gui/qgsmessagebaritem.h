/***************************************************************************
                          qgsmessagebaritem.h  -  description
                             -------------------
    begin                : August 2013
    copyright            : (C) 2013 by Denis Rouzaud
    email                : denis.rouzaud@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef qgsmessagebaritem_H
#define qgsmessagebaritem_H

#include <qgsmessagebaritem.h>
#include <qgsmessagebar.h>

#include <QWidget>
#include <QIcon>
#include <QTextEdit>
#include <QHBoxLayout>


class QgsMessageBarItem : public QWidget
{
    Q_OBJECT
  public:
    //! make out a widget containing a message to be displayed on the bar
    QgsMessageBarItem( const QString &text, QgsMessageBar::MessageLevel level = QgsMessageBar::INFO, int duration = 0, QWidget *parent = 0 );

    //! make out a widget containing title and message to be displayed on the bar
    QgsMessageBarItem( const QString &title, const QString &text, QgsMessageBar::MessageLevel level = QgsMessageBar::INFO, int duration = 0, QWidget *parent = 0 );

    //! make out a widget containing title, message and widget to be displayed on the bar
    QgsMessageBarItem( const QString &title, const QString &text, QWidget *widget, QgsMessageBar::MessageLevel level = QgsMessageBar::INFO, int duration = 0, QWidget *parent = 0 );

    //! make out a widget containing a widget to be displayed on the bar
    QgsMessageBarItem( QWidget *widget, QgsMessageBar::MessageLevel level = QgsMessageBar::INFO, int duration = 0, QWidget *parent = 0 );

    ~QgsMessageBarItem();

    QgsMessageBarItem *setText( QString text );

    QgsMessageBarItem *setTitle( QString title );

    QgsMessageBarItem *setLevel( QgsMessageBar::MessageLevel level );

    QgsMessageBarItem *setWidget( QWidget *widget );

    QgsMessageBarItem *setIcon( const QIcon &icon );

    QgsMessageBarItem *setDuration( int duration );

    //! returns the duration in second of the message
    int duration() const { return mDuration; }

    //! get the uuid of this message
    QString id() { return mUuid; }

    //! returns the level
    QgsMessageBar::MessageLevel level() { return mLevel; }

    //! returns the styleSheet
    QString getStyleSheet() { return mStyleSheet; }

  signals:
    //! emitted when the message level has changed
    void styleChanged( QString styleSheet );


  private:
    void writeContent();

    QString mUuid;
    QString mTitle;
    QString mText;
    QgsMessageBar::MessageLevel mLevel;
    int mDuration;
    QWidget *mWidget;
    QIcon mUserIcon;
    QHBoxLayout *mLayout;
    QLabel *mLblIcon;
    QString mStyleSheet;
    QTextEdit *mTextEdit;
};

#endif // qgsmessagebaritem_H
