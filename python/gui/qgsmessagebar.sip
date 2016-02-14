class QgsMessageBar: QFrame
{
%TypeHeaderCode
#include <qgsmessagebar.h>
%End

  public:
    enum MessageLevel
    {
      INFO,
      WARNING,
      CRITICAL,
      SUCCESS
    };

    QgsMessageBar( QWidget *parent /TransferThis/ = 0 );
    ~QgsMessageBar();

    /**
     * Display a message item on the bar after hiding the currently visible one
     * and putting it in a stack.
     * The message bar will take ownership of the item.
     *
     * @param item Item to display
     */
    void pushItem( QgsMessageBarItem *item /Transfer/);

    /** Display a widget as a message on the bar after hiding the currently visible one
     *  and putting it in a stack.
     * @param widget message widget to display
     * @param level is QgsMessageBar::INFO, WARNING, CRITICAL or SUCCESS
     * @param duration timeout duration of message in seconds, 0 value indicates no timeout
     */
    QgsMessageBarItem *pushWidget( QWidget *widget /Transfer/, MessageLevel level = INFO, int duration = 0 );

    /** Remove the passed widget from the bar (if previously added),
     *  then display the next one in the stack if any or hide the bar
     *  @param item item to remove
     *  @return true if the widget was removed, false otherwise
     */
    bool popWidget( QgsMessageBarItem *item );

    //! make out a widget containing a message to be displayed on the bar
    static QgsMessageBarItem* createMessage( const QString &text, QWidget *parent = 0 ) /Factory/;
    //! make out a widget containing title and message to be displayed on the bar
    static QgsMessageBarItem* createMessage( const QString &title, const QString &text, QWidget *parent = 0 )  /Factory/;
    //! make out a widget containing title and message to be displayed on the bar
    static QgsMessageBarItem* createMessage( QWidget *widget, QWidget *parent = 0 ) /Factory/;

    //! convenience method for pushing a message to the bar
    void pushMessage( const QString &text, MessageLevel level = INFO, int duration = 0 );
    //! convenience method for pushing a message with title to the bar
    void pushMessage( const QString &title, const QString &text, MessageLevel level = INFO, int duration = 0 );

    QgsMessageBarItem *currentItem();

  signals:
    //! emitted when a message widget is added to the bar
    void widgetAdded( QgsMessageBarItem *item );

    //! emitted when a message widget was removed from the bar
    void widgetRemoved( QgsMessageBarItem *item );

  public slots:
    /** Remove the currently displayed widget from the bar and
     *  display the next in the stack if any or hide the bar.
     *  @return true if the widget was removed, false otherwise
     */
    bool popWidget();

    /** Remove all items from the bar's widget list
     *  @return true if all items were removed, false otherwise
     */
    bool clearWidgets();

    /**
     * Pushes a success message with default timeout to the message bar
     * @param title title string for message
     * @param message The message to be displayed
     * @note added in 2.8
     */
    void pushSuccess( const QString& title, const QString& message );

    /**
     * Pushes a information message with default timeout to the message bar
     * @param title title string for message
     * @param message The message to be displayed
     * @note added in 2.8
     */
    void pushInfo( const QString& title, const QString& message );

    /**
     * Pushes a warning with default timeout to the message bar
     * @param title title string for message
     * @param message The message to be displayed
     * @note added in 2.8
     */
    void pushWarning( const QString& title, const QString& message );

    /**
     * Pushes a critical warning with default timeout to the message bar
     * @param title title string for message
     * @param message The message to be displayed
     * @note added in 2.8
     */
    void pushCritical( const QString& title, const QString& message );

  protected:
    void mousePressEvent( QMouseEvent * e );
};
