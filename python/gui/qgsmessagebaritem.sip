class QgsMessageBarItem: QWidget
{
%TypeHeaderCode
#include <qgsmessagebaritem.h>
#include <qgsmessagebar.h>
%End

  public:
    //! make out a widget containing a message to be displayed on the bar
    QgsMessageBarItem( const QString &text, QgsMessageBar::MessageLevel level = QgsMessageBar::INFO, int duration = 0, QWidget *parent /TransferThis/ = 0 );

    //! make out a widget containing title and message to be displayed on the bar
    QgsMessageBarItem( const QString &title, const QString &text, QgsMessageBar::MessageLevel level = QgsMessageBar::INFO, int duration = 0, QWidget *parent /TransferThis/ = 0 );

    //! make out a widget containing title, message and widget to be displayed on the bar
    QgsMessageBarItem( const QString &title, const QString &text, QWidget *widget, QgsMessageBar::MessageLevel level = QgsMessageBar::INFO, int duration = 0, QWidget *parent /TransferThis/ = 0 );

    //! make out a widget containing a widget to be displayed on the bar
    QgsMessageBarItem( QWidget *widget, QgsMessageBar::MessageLevel level = QgsMessageBar::INFO, int duration = 0, QWidget *parent /TransferThis/ = 0 );

    QgsMessageBarItem *setText( const QString& text );

    QgsMessageBarItem *setTitle( const QString& title );

    QgsMessageBarItem *setLevel( QgsMessageBar::MessageLevel level );

    QgsMessageBarItem *setWidget( QWidget *widget );

    QgsMessageBarItem *setIcon( const QIcon &icon );

    QgsMessageBarItem *setDuration( int duration );

    //! returns the duration in second of the message
    int duration() const;

    //! returns the level
    QgsMessageBar::MessageLevel level();

    //! returns the styleSheet
    QString getStyleSheet();

  signals:
    //! emitted when the message level has changed
    void styleChanged( const QString& styleSheet );
};

