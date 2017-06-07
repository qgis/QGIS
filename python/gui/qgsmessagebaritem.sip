/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsmessagebaritem.h                                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsMessageBarItem : QWidget
{

%TypeHeaderCode
#include "qgsmessagebaritem.h"
%End
  public:
    QgsMessageBarItem( const QString &text, QgsMessageBar::MessageLevel level = QgsMessageBar::INFO, int duration = 0, QWidget *parent /TransferThis/ = 0 );
%Docstring
make out a widget containing a message to be displayed on the bar
%End

    QgsMessageBarItem( const QString &title, const QString &text, QgsMessageBar::MessageLevel level = QgsMessageBar::INFO, int duration = 0, QWidget *parent /TransferThis/ = 0 );
%Docstring
make out a widget containing title and message to be displayed on the bar
%End

    QgsMessageBarItem( const QString &title, const QString &text, QWidget *widget, QgsMessageBar::MessageLevel level = QgsMessageBar::INFO, int duration = 0, QWidget *parent /TransferThis/ = 0 );
%Docstring
make out a widget containing title, message and widget to be displayed on the bar
%End

    QgsMessageBarItem( QWidget *widget, QgsMessageBar::MessageLevel level = QgsMessageBar::INFO, int duration = 0, QWidget *parent /TransferThis/ = 0 );
%Docstring
make out a widget containing a widget to be displayed on the bar
%End

    QgsMessageBarItem *setText( const QString &text );
%Docstring
 :rtype: QgsMessageBarItem
%End

    QgsMessageBarItem *setTitle( const QString &title );
%Docstring
 :rtype: QgsMessageBarItem
%End

    QgsMessageBarItem *setLevel( QgsMessageBar::MessageLevel level );
%Docstring
 :rtype: QgsMessageBarItem
%End

    QgsMessageBarItem *setWidget( QWidget *widget );
%Docstring
 :rtype: QgsMessageBarItem
%End

    QgsMessageBarItem *setIcon( const QIcon &icon );
%Docstring
 :rtype: QgsMessageBarItem
%End

    QgsMessageBarItem *setDuration( int duration );
%Docstring
 :rtype: QgsMessageBarItem
%End

    int duration() const;
%Docstring
returns the duration in second of the message
 :rtype: int
%End

    QgsMessageBar::MessageLevel level();
%Docstring
returns the level
 :rtype: QgsMessageBar.MessageLevel
%End

    QString getStyleSheet();
%Docstring
returns the styleSheet
 :rtype: str
%End

  signals:
    void styleChanged( const QString &styleSheet );
%Docstring
emitted when the message level has changed
%End


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsmessagebaritem.h                                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
