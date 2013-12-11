
/**  Render checker for tests in python */

class QgsRenderChecker
{
%TypeHeaderCode
#include <qgsrenderchecker.h>
%End
  public:

    QgsRenderChecker();

    //! Destructor
    ~QgsRenderChecker();

    QString controlImagePath() const;

    QString report();

    float matchPercent();

    unsigned int mismatchCount();

    unsigned int matchTarget();

    int elapsedTime();

    void setControlName( const QString theName );

    void setControlPathPrefix( const QString theName );

    QString imageToHash( QString theImageFile );

    void setRenderedImage( QString theImageFileName );
    void setMapRenderer( QgsMapRenderer *  thepMapRenderer ) /Deprecated/;
    void setMapSettings( const QgsMapSettings& mapSettings );
    bool runTest( QString theTestName, unsigned int theMismatchCount = 0 );

    bool compareImages( QString theTestName, unsigned int theMismatchCount = 0, QString theRenderedImageFile = "" );

    bool isKnownAnomaly( QString theDiffImageFile );
};
