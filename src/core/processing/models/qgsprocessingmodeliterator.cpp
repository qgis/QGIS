/***************************************************************************
                         qgsprocessingmodeliterator.cpp
                         ------------------------------------
    begin                : December 2017
    copyright            : (C) 2017 by Arnaud Morvan
    email                : arnaud dot morvan at camptocamp dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprocessingmodeliterator.h"
#include "qgsapplication.h"
#include "qgsprocessingregistry.h"
#include "qgsprocessingmodelalgorithm.h"

///@cond NOT_STABLE

QgsProcessingAlgorithm::Flags QgsProcessingModelIterator::flags() const
{
  return FlagIterator;
}

///@endcond
