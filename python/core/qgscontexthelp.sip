/** \ingroup core
 * Provides a context based help browser for a dialog.
 *
 * The help text is stored in SQLite and accessed by a context identifier
 * unique to each dialog. This is a singleton class which invokes the help
 * viewer using QProcess and ensures that only one viewer is open.
 * The viewer will be terminated if open when the main application quits.
 *
 */
class QgsContextHelp : QObject
{
%TypeHeaderCode
#include <qgscontexthelp.h>
%End
  public:
    static void run( QString context );

  private:
    //! Constructor
    QgsContextHelp();
    //! Destructor
    ~QgsContextHelp();
};
