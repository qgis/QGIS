class QgsMapCanvasSnappingUtils : QgsSnappingUtils
{
%TypeHeaderCode
#include <qgsmapcanvassnappingutils.h>
%End
  public:
    QgsMapCanvasSnappingUtils( QgsMapCanvas* canvas, QObject* parent = 0 );

  protected:
    virtual void prepareIndexStarting( int count );
    virtual void prepareIndexProgress( int index );
};
