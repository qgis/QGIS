class QgsPaintEngineHack : QPaintEngine
{
%TypeHeaderCode
#include <qgspaintenginehack.h>
%End
  public:
    static void fixEngineFlags( QPaintEngine *engine );
};
