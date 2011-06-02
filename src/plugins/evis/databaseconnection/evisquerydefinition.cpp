/*
** File: evisquerydefinition.cpp
** Author: Peter J. Ersts ( ersts at amnh.org )
** Creation Date: 2007-04-12
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
#include "evisquerydefinition.h"

/**
* Constructor
*/
eVisQueryDefinition::eVisQueryDefinition( )
{
  mDatabaseType = "";
  mDatabaseHost = "";
  mDatabasePort = -1;
  mDatabaseName = "";
  mDatabaseUsername = "";
  mDatabasePassword = "";
  mSqlStatement = "";
  mAutoConnect = false;
}

