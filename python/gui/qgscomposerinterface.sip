class QgsComposerInterface: QObject
{
%TypeHeaderCode
#include <qgscomposerinterface.h>
%End
  public:
    QgsComposerInterface( QObject *parent /TransferThis/ = 0 );

    virtual ~QgsComposerInterface();
    virtual QgsComposerView *view() = 0;
    virtual QgsComposition *composition() = 0;
    virtual void close() = 0;

};

