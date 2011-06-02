/*
** File: evisconfiguration.h
** Author: Peter J. Ersts ( ersts at amnh.org )
** Creation Date: 2007-12-11
**
** Copyright ( c ) 2007, American Museum of Natural History. All rights reserved.
**
** This library/program is free software; you can redistribute it
** and/or modify it under the terms of the GNU Library General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or ( at your option ) any later version.
**
** This library/program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.
**
** This work was made possible through a grant by the the John D. and
** Catherine T. MacArthur Foundation. Additionally, this program was prepared by
** the American Museum of Natural History under award No. NA05SEC46391002
** from the National Oceanic and Atmospheric Administration, U.S. Department
** of Commerce.  The statements, findings, conclusions, and recommendations
** are those of the author( s ) and do not necessarily reflect the views of the
** National Oceanic and Atmospheric Administration or the Department of Commerce.
**
**/
#ifndef eVisConfiguration_H
#define eVisConfiguration_H

#include <QString>

/**
* \class eVisConfiguration
* \brief This class is not much more than a structure for holding parameters
* The eVisConfiguration is simply a class to hold a variety of configuration options
* for the event browser. The event browser is actually responsible for saving
* the options. Give that, the individual function are not commented as they should be obvious
*/
class eVisConfiguration
{

  public:
    eVisConfiguration( );

    QString basePath( );
    QString compassBearingField( );
    double compassOffset( );
    QString compassOffsetField( );
    QString eventImagePathField( );

    bool isApplyPathRulesToDocsSet( );
    bool isAttributeCompassOffsetSet( );
    bool isDisplayCompassBearingSet( );
    bool isEventImagePathRelative( );
    bool isManualCompassOffsetSet( );
    bool isUseOnlyFilenameSet( );

    void setApplyPathRulesToDocs( bool );
    void setAttributeCompassOffset( bool );
    void setBasePath( QString );
    void setCompassBearingField( QString );
    void setCompassOffset( double );
    void setCompassOffsetField( QString );
    void setDisplayCompassBearing( bool );
    void setEventImagePathField( QString );
    void setEventImagePathRelative( bool );
    void setManualCompassOffset( bool );
    void setUseOnlyFilename( bool );


  private:
    bool mApplyPathRulesToDocs;
    bool mAttributeCompassOffset;
    bool mDisplayCompassBearing;
    bool mEventImagePathRelative;
    bool mManualCompassOffset;
    bool mUseOnlyFilename;

    QString mBasePath;
    QString mCompassOffsetField;
    QString mCompassBearingField;
    QString mEventImagePathField;


    double mCompassOffset;
};
#endif
