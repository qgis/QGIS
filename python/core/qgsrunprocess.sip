
/** \ingroup core
 * A class that executes an external program/script.
 * It can optionally capture the standard output and error from the
 * process and displays them in a dialog box.
 */
class QgsRunProcess : QObject /NoDefaultCtors/
{
%TypeHeaderCode
#include <qgsrunprocess.h>
%End

  public:
    // This class deletes itself, so to ensure that it is only created
    // using new, the Named Consturctor Idiom is used, and one needs to
    // use the create() static function to get an instance of this class.

    // The action argument contains string with the command.
    // If capture is true, the standard output and error from the process
    // will be sent to QgsMessageOuptut - usually a dialog box.
    static QgsRunProcess* create( const QString& action, bool capture ) /Factory/;

  public slots:
    void stdoutAvailable();
    void stderrAvailable();
    void processError( QProcess::ProcessError );
    void processExit( int, QProcess::ExitStatus );
    void dialogGone();

  private:
    QgsRunProcess( const QString& action, bool capture );
    ~QgsRunProcess();
};
