/***************************************************************************
                          qgsdlgvectorlayerproperties.h
                   Unified property dialog for vector layers
                             -------------------
    begin                : 2004-01-28
    copyright            : (C) 2004 by Gary E.Sherman
    email                : sherman at mrcc.com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 /* $Id$ */
#include "qgsdlgvectorlayerpropertiesbase.h"
class QgsVectorLayer;

class QgsDlgVectorLayerProperties : public QgsDlgVectorLayerPropertiesBase{
  Q_OBJECT
  public:
  QgsDlgVectorLayerProperties(QgsVectorLayer *lyr =0,QWidget *parent=0, const char *name=0);
  ~QgsDlgVectorLayerProperties();
  protected:
  QgsVectorLayer *layer;
  
};
