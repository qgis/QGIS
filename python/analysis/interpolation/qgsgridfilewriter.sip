class QgsGridFileWriter
{
%TypeHeaderCode
#include <qgsgridfilewriter.h>
%End

  public:
    QgsGridFileWriter( QgsInterpolator* i, const QString& outputPath, const QgsRectangle& extent, int nCols, int nRows, double cellSizeX, double cellSizeY );

    /** Writes the grid file.
     @param showProgressDialog shows a dialog with the possibility to cancel
    @return 0 in case of success*/

    int writeFile( bool showProgressDialog = false );
};
