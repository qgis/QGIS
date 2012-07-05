class CORE_EXPORT QgsPaintEngineHack : public QPaintEngine
{
%TypeHeaderCode
#include <qgspaintenginehack.h>
%End
  public:
    static void fixEngineFlags( QPaintEngine *engine );
};
