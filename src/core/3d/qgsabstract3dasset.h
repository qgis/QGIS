/***************************************************************************
  qgsabstract3dasset.h
  --------------------------------------
  Date                 : April 2026
  Copyright            : (C) 2026 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSABSTRACT3DASSET_H
#define QGSABSTRACT3DASSET_H

#include "qgis_core.h"
#include "qgis_sip.h"

class QDomElement;
class QgsReadWriteContext;

using namespace Qt::StringLiterals;

/**
 * \ingroup core
 * \brief Abstract base class for 3D assets.
 *
 * \warning This is not considered stable API, and may change in future QGIS releases. It is
 * exposed to the Python bindings as a tech preview only.
 */
class CORE_EXPORT QgsAbstract3DAsset SIP_ABSTRACT
{
    //SIP_TYPEHEADER_INCLUDE( "qgsabstractmaterialsettings.h" );
    //SIP_TYPEHEADER_INCLUDE( "qgsgoochmaterialsettings.h" );
    //SIP_TYPEHEADER_INCLUDE( "qgsmetalroughmaterialsettings.h" );
    //SIP_TYPEHEADER_INCLUDE( "qgsmetalroughtexturedmaterialsettings.h" );
    //SIP_TYPEHEADER_INCLUDE( "qgsnullmaterialsettings.h" );
    //SIP_TYPEHEADER_INCLUDE( "qgsphongmaterialsettings.h" );
    //SIP_TYPEHEADER_INCLUDE( "qgsphongtexturedmaterialsettings.h" );
    //SIP_TYPEHEADER_INCLUDE( "qgssimplelinematerialsettings.h" );

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( dynamic_cast< QgsGoochMaterialSettings * >( sipCpp ) )
    {
      sipType = sipType_QgsGoochMaterialSettings;
    }
    else if ( dynamic_cast< QgsPhongMaterialSettings * >( sipCpp ) )
    {
      sipType = sipType_QgsPhongMaterialSettings;
    }
    else if ( dynamic_cast< QgsPhongTexturedMaterialSettings * >( sipCpp ) )
    {
      sipType = sipType_QgsPhongTexturedMaterialSettings;
    }
    else if ( dynamic_cast< QgsSimpleLineMaterialSettings * >( sipCpp ) )
    {
      sipType = sipType_QgsSimpleLineMaterialSettings;
    }
    else if ( dynamic_cast< QgsNullMaterialSettings * >( sipCpp ) )
    {
      sipType = sipType_QgsNullMaterialSettings;
    }
    else if ( dynamic_cast< QgsMetalRoughMaterialSettings * >( sipCpp ) )
    {
      sipType = sipType_QgsMetalRoughMaterialSettings;
    }
    else if ( dynamic_cast< QgsMetalRoughTexturedMaterialSettings * >( sipCpp ) )
    {
      sipType = sipType_QgsMetalRoughTexturedMaterialSettings;
    }
    else if ( dynamic_cast< QgsAbstractMaterialSettings * >( sipCpp ) )
    {
      // must be last material settings type here!
      sipType = sipType_QgsAbstractMaterialSettings;
    }
    else
    {
      sipType = 0;
    }
  SIP_END
#endif

  public:
    virtual ~QgsAbstract3DAsset() = default;

    /**
     * Returns the type of 3D asset.
     */
    virtual Qgis::Asset3DType assetType() const = 0;

    /**
     * Clones the asset.
     *
     * Caller takes ownership of the returned object.
     */
    virtual QgsAbstract3DAsset *clone() const = 0 SIP_FACTORY;

    //! Reads asset properties from a DOM \a element
    virtual void readXml( const QDomElement &element, const QgsReadWriteContext &context );

    //! Writes asset properties to a DOM \a element
    virtual void writeXml( QDomElement &element, const QgsReadWriteContext &context ) const;

    /**
     * Returns TRUE if this asset exactly matches an \a other asset.
     */
    virtual bool equals( const QgsAbstract3DAsset *other ) const = 0;
};


#endif // QGSABSTRACT3DASSET_H
