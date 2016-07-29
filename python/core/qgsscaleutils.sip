class QgsScaleUtils
{
%TypeHeaderCode
#include <qgsscaleutils.h>
%End
  public:
    /** Save scales to the given file
     * @param fileName the name of the output file
     * @param scales the list of scales to save
     * @param errorMessage it will contain the error message if something
     * went wrong
     * @return true on success and false if failed
     */
    static bool saveScaleList( const QString &fileName, const QStringList &scales, QString &errorMessage );

    /** Load scales from the given file
     * @param fileName the name of the file to process
     * @param scales it will contain loaded scales
     * @param errorMessage it will contain the error message if something
     * went wrong
     * @return true on success and false if failed
     */
    static bool loadScaleList( const QString &fileName, QStringList &scales, QString &errorMessage );
};
