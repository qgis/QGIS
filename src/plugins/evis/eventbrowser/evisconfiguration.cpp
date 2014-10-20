/*
** File: evisconfiguration.cpp
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
** This work was made possible through a grant by the John D. and
** Catherine T. MacArthur Foundation. Additionally, this program was prepared by
** the American Museum of Natural History under award No. NA05SEC46391002
** from the National Oceanic and Atmospheric Administration, U.S. Department
** of Commerce.  The statements, findings, conclusions, and recommendations
** are those of the author( s ) and do not necessarily reflect the views of the
** National Oceanic and Atmospheric Administration or the Department of Commerce.
**
**/
#include <QSettings>

#include "evisconfiguration.h"

eVisConfiguration::eVisConfiguration()
{
  QSettings myQSettings;

  setApplyPathRulesToDocs( myQSettings.value( "/eVis/applypathrulestodocs", false ).toBool() );

  setEventImagePathField( myQSettings.value( "/eVis/eventimagepathfield", "" ).toString() );
  setEventImagePathRelative( myQSettings.value( "/eVis/eventimagepathrelative", false ).toBool() );

  setDisplayCompassBearing( myQSettings.value( "/eVis/displaycompassbearing", false ).toBool() );
  setCompassBearingField( myQSettings.value( "/eVis/compassbearingfield", "" ).toString() );

  setManualCompassOffset( myQSettings.value( "/eVis/manualcompassoffset", false ).toBool() );
  setCompassOffset( myQSettings.value( "/eVis/compassoffset", "0.0" ).toDouble() );
  setAttributeCompassOffset( myQSettings.value( "/eVis/attributecompassoffset", false ).toBool() );
  setCompassOffsetField( myQSettings.value( "/eVis/compassoffsetfield", "" ).toString() );

  setBasePath( myQSettings.value( "/eVis/basepath", "" ).toString() );
  mUseOnlyFilename = myQSettings.value( "/eVis/useonlyfilename", false ).toBool();
}

QString eVisConfiguration::basePath()
{
  return mBasePath;
}

QString eVisConfiguration::compassBearingField()
{
  return mCompassBearingField;
}

double eVisConfiguration::compassOffset()
{
  return mCompassOffset;
}

QString eVisConfiguration::compassOffsetField()
{
  return mCompassOffsetField;
}

QString eVisConfiguration::eventImagePathField()
{
  return mEventImagePathField;
}

bool eVisConfiguration::isApplyPathRulesToDocsSet()
{
  return mApplyPathRulesToDocs;
}

bool eVisConfiguration::isAttributeCompassOffsetSet()
{
  return mAttributeCompassOffset;
}

bool eVisConfiguration::isDisplayCompassBearingSet()
{
  return mDisplayCompassBearing;
}

bool eVisConfiguration::isEventImagePathRelative()
{
  return mEventImagePathRelative;
}

bool eVisConfiguration::isManualCompassOffsetSet()
{
  return mManualCompassOffset;
}

bool eVisConfiguration::isUseOnlyFilenameSet()
{
  return mUseOnlyFilename;
}

void eVisConfiguration::setApplyPathRulesToDocs( bool theBool )
{
  mApplyPathRulesToDocs = theBool;
}

void eVisConfiguration::setAttributeCompassOffset( bool theBool )
{
  mAttributeCompassOffset = theBool;
}

void eVisConfiguration::setBasePath( QString thePath )
{
  QSettings myQSettings;
  mBasePath = thePath;
  if ( "" != mBasePath )
  {
    if ( mBasePath.contains( '/' ) )
    {
      if ( mBasePath[mBasePath.length()-1] != '/' )
      {
        mBasePath = mBasePath + "/";
      }
    }
    else
    {
      if ( mBasePath[mBasePath.length()-1] != '\\' )
      {
        mBasePath = mBasePath + "\\";
      }
    }
  }
}

void eVisConfiguration::setCompassBearingField( QString theField )
{
  mCompassBearingField = theField;
}

void eVisConfiguration::setCompassOffset( double theOffset )
{
  mCompassOffset = theOffset;
}

void eVisConfiguration::setCompassOffsetField( QString theField )
{
  mCompassOffsetField = theField;
}

void eVisConfiguration::setDisplayCompassBearing( bool theBool )
{
  mDisplayCompassBearing = theBool;
}

void eVisConfiguration::setEventImagePathField( QString theField )
{
  mEventImagePathField = theField;
}

void eVisConfiguration::setEventImagePathRelative( bool theBool )
{
  mEventImagePathRelative = theBool;
}

void eVisConfiguration::setManualCompassOffset( bool theBool )
{
  mManualCompassOffset = theBool;
}

void eVisConfiguration::setUseOnlyFilename( bool theBool )
{
  mUseOnlyFilename = theBool;
}
