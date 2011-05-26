/***************************************************************************
                          qgsmngprogressbar.h
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
#ifndef QGSMNGPROGRESSBAR_H
#define QGSMNGPROGRESSBAR_H

#include "qprogressbar.h"

/**
* \class MngProgressBar
* \brief This Class manager the progress bar
*/
class MngProgressBar
{
  public:
    /**
    * \brief Constructor for a MngProgressBar.
    * \param pb Pointer to the MngProgressBar object.
    */
    MngProgressBar( QProgressBar *pb );
    /**
    * \brief Destructor
    */
    ~MngProgressBar() { mPb->reset(); };

    /**
    * \brief Sets the progress bar's minimum and maximum values to minimum and maximum respectively
    * \param minimum minimun value.
    * \param maximum maximum value.
    */
    void init( int minimum, int maximum );

    /**
    * \brief Sets the format the current text.
    * \param format This property holds the string used to generate the current text.
    */
    void setFormat( QString format );

    /**
    * \brief Sets current value progress bar's
    * \param step current value
    */
    void step( int step );

  private:
    QProgressBar * mPb;

};

#endif // QGSMNGPROGRESSBAR_H
