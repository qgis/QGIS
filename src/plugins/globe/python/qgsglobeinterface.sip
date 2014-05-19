/***************************************************************************
    qgsglobeinterface.sip
     --------------------------------------
    Date                 : 22.8.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias dot kuhn at gmx dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

class QgsGlobeInterface : QgsPluginInterface
{
%TypeHeaderCode
    #include "qgsglobeinterface.h"
%End

// Provide converter code
// Attention: qgis.globe has to be imported before this interface
// is requested the first time. In case this is not done,
// the converter code will not be executed and this will be treated as
// a (useless) QgsPluginInterface object instead.
%ConvertToSubClassCode
  if ( sipCpp->inherits( sipName_QgsGlobeInterface ) )
    sipType = sipType_QgsGlobeInterface;
  else
    sipType = NULL;
%End

  public:
    void syncExtent();

  private:
    // Instances will only be created from the globe plugin itself
    QgsGlobeInterface();
};
