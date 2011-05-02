/*!
 * \class QgsContextHelp
 * \brief Provides a context based help browser for a dialog.
 *
 * The help text is stored in SQLite and accessed by a context identifier
 * unique to each dialog. This is a singleton class which invokes the help
 * viewer using QProcess and ensures that only one viewer is open.
 * The viewer will be terminated if open when the main application quits.
 *
 * If the compile-time flag QGSCONTEXTHELP_REUSE is defined, the help viewer
 * will be reused if it is still open. If this flag is not set, the viewer
 * process will be terminated if open and restarted; this makes it the top
 * window for window managers such as Linux/GNOME which will make a window
 * active but not bring it to the top if raised programatically.
 */
class QgsContextHelp : QObject
{
%TypeHeaderCode
#include <qgscontexthelp.h>
%End

public:
  static void run(QString context);

private:
  //! Constructor
  QgsContextHelp(QString context);

  //! Destructor
  ~QgsContextHelp();
};
