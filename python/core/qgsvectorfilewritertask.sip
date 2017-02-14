class QgsVectorFileWriterTask : QgsTask
{
%TypeHeaderCode
#include <qgsvectorfilewritertask.h>
%End

  public:

    QgsVectorFileWriterTask( QgsVectorLayer* layer,
                             const QString& fileName,
                             const QgsVectorFileWriter::SaveVectorOptions& options );

    virtual void cancel();

  signals:

    void writeComplete( const QString& newFilename );
    void errorOccurred( int error, const QString& errorMessage );

  protected:

    virtual bool run();
    virtual void finished( bool result );
};

