#ifndef QGSABSTRACT3DSYMBOL_H
#define QGSABSTRACT3DSYMBOL_H

#include "qgis_3d.h"

class QDomElement;
class QString;

class QgsReadWriteContext;


/**
 * \inmodule qgis3d
 * Abstract base class for 3D symbols that are used by VectorLayer3DRenderer objects.
 *
 * 3D symbol objects define appearance of GIS data.
 *
 * \since QGIS 3.0
 */
class _3D_EXPORT QgsAbstract3DSymbol
{
  public:
    virtual ~QgsAbstract3DSymbol() {}

    //! Returns identifier of symbol type. Each 3D symbol implementation should return a different type.
    virtual QString type() const = 0;
    //! Returns a new instance of the symbol with the same settings
    virtual QgsAbstract3DSymbol *clone() const = 0;

    //! Writes symbol configuration to the given DOM element
    virtual void writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const = 0;
    //! Reads symbol configuration from the given DOM element
    virtual void readXml( const QDomElement &elem, const QgsReadWriteContext &context ) = 0;
};


#endif // QGSABSTRACT3DSYMBOL_H
