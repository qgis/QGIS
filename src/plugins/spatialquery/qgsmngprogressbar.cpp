/***************************************************************************
                          qgsmngprogressbar.cpp
                             -------------------
    begin                : Dec 29, 2009
    copyright            : (C) 2009 by Diego Moreira And Luiz Motta
    email                : moreira.geo at gmail.com And motta.luiz at gmail.com

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/*  $Id$ */

#include "qgsmngprogressbar.h"

MngProgressBar::MngProgressBar( QProgressBar *pb )
{
  mPb = pb;
  mPb->reset();
} // MngProgressBar::MngProgressBar(QProgressBar *pb)

void MngProgressBar::init( int minimum, int maximum )
{
  mPb->reset();
  mPb->setRange( minimum, maximum );

} // void MngProgressBar::init(int minimum, int maximum)

void MngProgressBar::setFormat( QString format )
{
// special caracters:
// %p - is replaced by the percentage completed.
// %v - is replaced by the current value.
// %m - is replaced by the total number of steps.
  mPb->setFormat( format );

} // void MngProgressBar::setFormat(QString format)

void MngProgressBar::step( int step )
{
  mPb->setValue( step );
  mPb->repaint();

} // void MngProgressBar::step()
