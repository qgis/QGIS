/***************************************************************************
  qgspaintenginehack.cpp - Hack paint engine flags
  ------------------------------------------------
         begin                : July 2012
         copyright            : (C) Juergen E. Fischer
         email                : jef at norbit dot de

 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include <QPaintEngine>

#include "qgis_core.h"

/**
 * \ingroup core
 * Hack to workaround Qt #5114 by disabling PatternTransform
 */
class CORE_EXPORT QgsPaintEngineHack : public QPaintEngine
{
  public:
    void fixFlags();
    static void fixEngineFlags( QPaintEngine *engine );
};
