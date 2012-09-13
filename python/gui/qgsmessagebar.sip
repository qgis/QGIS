class QgsMessageBar: QFrame
{
%TypeHeaderCode
#include <qgsmessagebar.h>
%End

  public:
    QgsMessageBar( QWidget *parent = 0 );
    ~QgsMessageBar();

    /*! display a widget on the bar after hiding the currently visible one
     *  and putting it in a stack
     * @param level is 0 for information, 1 for warning, 2 for critical
     */
    void pushWidget( QWidget *widget, int level = 0 );

    /*! remove the passed widget from the bar (if previously added),
     *  then display the next one in the stack if any or hide the bar
     *  @return true if the widget was removed, false otherwise
     */
    bool popWidget( QWidget *widget );

    //! make out a widget containing a message to be displayed on the bar
    static QWidget* createMessage( const QString &text, QWidget *parent = 0 ) /Factory/;

    //! make out a widget containing icon and message to be displayed on the bar
    static QWidget* createMessage( const QString &text, const QIcon &icon, QWidget *parent = 0 ) /Factory/;

    //! make out a widget containing title and message to be displayed on the bar
    static QWidget* createMessage( const QString &title, const QString &text, QWidget *parent = 0 )  /Factory/;

    //! make out a widget containing icon, title and message to be displayed on the bar
    static QWidget* createMessage( const QString &title, const QString &text, const QIcon &icon, QWidget *parent = 0 ) /Factory/;

  signals:
    //! emitted when a widget was removed from the bar
    void widgetRemoved( QWidget *widget );

  public slots:
    /*! remove the currently displayed widget from the bar and
     *  display the next in the stack if any or hide the bar
     *  @return true if the widget was removed, false otherwise
     */
    bool popWidget();
};
