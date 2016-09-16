class QgsMapCanvasSnappingUtils : QgsSnappingUtils
{
%TypeHeaderCode
#include <qgsmapcanvassnappingutils.h>
%End
  public:
    QgsMapCanvasSnappingUtils( QgsMapCanvas* canvas, QObject* parent = 0 );
    virtual ~QgsMapCanvasSnappingUtils();

  protected:
    virtual void prepareIndexStarting( int count );
    virtual void prepareIndexProgress( int index );
};
