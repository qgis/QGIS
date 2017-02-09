/** \ingroup gui
 * \class QgsDockWidget
 * QDockWidget subclass with more fine-grained control over how the widget is closed or opened.
 * \note added in 2.16
 */

class QgsDockWidget : QDockWidget
{
%TypeHeaderCode
#include <qgsdockwidget.h>
%End

  public:

    /** Constructor for QgsDockWidget.
     * @param parent parent widget
     * @param flags window flags
     */
    explicit QgsDockWidget( QWidget* parent /TransferThis/ = nullptr, Qt::WindowFlags flags = 0 );

    /** Constructor for QgsDockWidget.
     * @param title dock title
     * @param parent parent widget
     * @param flags window flags
     */
    explicit QgsDockWidget( const QString &title, QWidget* parent /TransferThis/ = nullptr, Qt::WindowFlags flags = 0 );

  public slots:

    /** Sets the dock widget as visible to a user, ie both shown and raised to the front.
     * @param visible set to true to show the dock to the user, or false to hide the dock.
     * When setting a dock as user visible, the dock will be opened (if it is not already
     * opened) and raised to the front.
     * When setting as hidden, the following logic is used:
     * - hiding a dock which is open but not raised (ie hidden by another tab) will have no
     * effect, and the dock will still be opened and hidden by the other tab
     * - hiding a dock which is open and raised (ie, user visible) will cause the dock to
     * be closed
     * - hiding a dock which is closed has no effect and raises no signals
     * @see isUserVisible()
     */
    void setUserVisible( bool visible );

    /** Returns true if the dock is both opened and raised to the front (ie not hidden by
     * any other tabs.
     * @see setUserVisible()
     */
    bool isUserVisible() const;

  protected:

    virtual void closeEvent( QCloseEvent * );
    virtual void showEvent( QShowEvent* event );

  signals:

    /** Emitted when dock widget is closed.
     * @see closedStateChanged()
     * @see opened()
     */
    void closed();

    /** Emitted when dock widget is closed (or opened).
     * @param wasClosed will be true if dock widget was closed, or false if dock widget was opened
     * @see closed()
     * @see openedStateChanged()
     */
    void closedStateChanged( bool wasClosed );

    /** Emitted when dock widget is opened.
     * @see openedStateChanged()
     * @see closed()
     */
    void opened();

    /** Emitted when dock widget is opened (or closed).
     * @param wasOpened will be true if dock widget was opened, or false if dock widget was closed
     * @see closedStateChanged()
     * @see opened()
     */
    void openedStateChanged( bool wasOpened );

};
