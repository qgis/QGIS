/***************************************************************************
                          qgsexception.cpp  -  description
                             -------------------
    begin                : Time-stamp: <2005-04-28 14:30:58 mcoletti>
    copyright            : (C) 2002 by Mark Coletti
    email                : mcoletti at gmail.com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgsexception.h"


const char * const ident_ = "$Id$";




const char * QgsProjectBadLayerException::msg_ = "Unable to open one or more project layers";
