class QgsPaintEngineHack : QPaintEngine
{
%TypeHeaderCode
#include <qgspaintenginehack.h>
%End

  public:
    void fixFlags();
    static void fixEngineFlags( QPaintEngine *engine );
};
