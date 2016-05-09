/** \ingroup gui
 * \class QgsFocusWatcher
 * A event filter for watching for focus events on a parent object. Usually QObjects must
 * subclass and override methods like focusOutEvent to handle focus events. Using this class
 * as an event filter avoids the need to subclass objects and the focus events can be directly
 * caught using the emitted signals.
 * \note added in 2.16
 */

class QgsFocusWatcher: QObject
{
%TypeHeaderCode
#include <qgsfocuswatcher.h>
%End

public:

    /** Constructor for QgsFocusWatcher.
     * @param parent parent widget to catch focus events for. This class will automatically be
     * installed as an event filter for parent.
     */
    explicit QgsFocusWatcher( QObject* parent /TransferThis/ );

    virtual bool eventFilter( QObject* obj, QEvent* event );

  signals:

    /** Emitted when parent object's focus changes.
     * @param focused true if object gained focus, false if object lost focus
     */
    void focusChanged( bool focused );

    //! Emitted when parent object gains focus.
    void focusIn();

    //! Emitted when parent object loses focus.
    void focusOut();
};
