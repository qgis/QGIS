/***************************************************************************
    qgspastetransformations.h - set up how source fields are transformed to
                                destination fields in copy/paste operations
                             -------------------
    begin                : 8 July 2005
    copyright            : (C) 2005 by Brendan Morley
    email                : morb at ozemail dot com dot au
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
#ifndef QGSPASTETRANSFORMATIONS_H
#define QGSPASTETRANSFORMATIONS_H
#ifdef WIN32
#include "qgspastetransformationsbase.h"
#else
#include "qgspastetransformationsbase.uic.h"
#endif
/*! 
 * \brief Dialog to allow the user to set up how source fields are transformed to destination fields in copy/paste operations
 */
class QgsPasteTransformations : public QgsPasteTransformationsBase
{
  Q_OBJECT
 public:
    //! Constructor
    QgsPasteTransformations();

    //! Destructor
    ~QgsPasteTransformations();

    //! Saves the state of the paste transformations
    void saveState();
};

#endif //  QGSPASTETRANSFORMATIONS_H
