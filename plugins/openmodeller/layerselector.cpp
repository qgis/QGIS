
/***************************************************************************
 *   Copyright (C) 2003 by Tim Sutton                                      *
 *   tim@linfiniti.com                                                     *
 *                                                                         *
 *   Gyps - Species Distribution Modelling Toolkit                         *
 *   This toolkit provides data transformation and visualisation           *
 *   tools for use in species distribution modelling tools such as GARP,   *
 *   CSM, Bioclim etc.                                                     *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "layerselector.h"

//qt includes
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qlistview.h>

//for the travers directories and is valid gdal file functions
#include "openmodellergui.h"

//standard includes
#include <iostream>



LayerSelector::LayerSelector( QWidget* parent , const char* name , bool modal , WFlags fl  )
  : LayerSelectorBase( parent, name, modal, fl )
{
  QString myStartDirString = "/home/aps02ts/dev/cpp/sample_data/cres/";
  QString myFileNameString = "test file";
  QListViewItem listParent = new QListViewItem(listFileTree,myFileNameString);
}
