#ifndef QGSMAPCANVASINTERFACE_H
#define QGSMAPCANVASINTERFACE_H

#include "qgis_core.h"
#include "qgis_sip.h"

class QgsMapSettings;

/**
 * The QgsMapCanvasInterface class provides a simple interface to access
 * core components of a map canvas.
 * \since QGIS 3.6
 */
class CORE_EXPORT QgsMapCanvasInterface
{
  public:
    //! Constructor of QgsMapCanvasInterface
    explicit QgsMapCanvasInterface() = default;
    virtual ~QgsMapCanvasInterface() = default;

    //! Returns the map settings
    virtual const QgsMapSettings &mapSettings() const = 0 SIP_KEEPREFERENCE;
};

#endif // QGSMAPCANVASINTERFACE_H
