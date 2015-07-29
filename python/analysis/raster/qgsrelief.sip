class QgsRelief
{
%TypeHeaderCode
#include <qgsrelief.h>
%End

  public:
    struct ReliefColor
    {
      ReliefColor( const QColor& c, double min, double max );
      QColor color;
      double minElevation;
      double maxElevation;
    };

    QgsRelief( const QString& inputFile, const QString& outputFile, const QString& outputFormat );
    ~QgsRelief();

    /** Starts the calculation, reads from mInputFile and stores the result in mOutputFile
      @param p progress dialog that receives update and that is checked for abort. 0 if no progress bar is needed.
      @return 0 in case of success*/
    int processRaster( QProgressDialog* p );

    double zFactor() const;
    void setZFactor( double factor );

    void clearReliefColors();
    void addReliefColorClass( const QgsRelief::ReliefColor& color );
    const QList< QgsRelief::ReliefColor >& reliefColors() const;
    void setReliefColors( const QList< QgsRelief::ReliefColor >& c );

    /** Calculates class breaks according with the method of Buenzli (2011) using an iterative algorithm for segmented regression
      @return true in case of success*/
    QList< QgsRelief::ReliefColor > calculateOptimizedReliefClasses();

    /** Write frequency of elevation values to file for manual inspection*/
    bool exportFrequencyDistributionToCsv( const QString& file );
};
