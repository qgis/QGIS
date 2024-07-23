/*
 *   libpal - Automated Placement of Labels Library
 *
 *   Copyright (C) 2008 Maxence Laurent, MIS-TIC, HEIG-VD
 *                      University of Applied Sciences, Western Switzerland
 *                      http://www.hes-so.ch
 *
 *   Contact:
 *      maxence.laurent <at> heig-vd <dot> ch
 *    or
 *      eric.taillard <at> heig-vd <dot> ch
 *
 * This file is part of libpal.
 *
 * libpal is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libpal is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libpal.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef PALSTAT_H
#define PALSTAT_H

#define SIP_NO_FILE


#include <QStringList>

namespace pal
{

  /**
   * \ingroup core
   * \brief Summary statistics of labeling problem.
   * \class pal::PalStat
   * \note not available in Python bindings
   */

  class PalStat
  {

      friend class Pal;
      friend class Problem;

    public:

      ~PalStat();

      PalStat( const PalStat &other ) = delete;
      PalStat &operator=( const PalStat &other ) = delete;

      /**
       * \brief the number of object in problem
       */
      int getNbObjects() const;

      /**
       * \brief the number of objects which are labelled
       */
      int getNbLabelledObjects() const;

      /**
       *  \brief how many layersare labelled ?
       */
      int getNbLayers() const;

      /**
       * Returns the name of the labelled layer \a layerId.
       */
      QString getLayerName( int layerId );

      /**
       * Returns the number of object in layer \a layerId.
       */
      int getLayerNbObjects( int layerId ) const;

      /**
       * Returns the number of object in layer \a layerId which are labelled.
       */
      int getLayerNbLabelledObjects( int layerId ) const;

    private:
      int nbObjects;
      int nbLabelledObjects;

      int nbLayers;

      QStringList layersName;
      int *layersNbObjects; // [nbLayers]
      int *layersNbLabelledObjects; // [nbLayers]

      PalStat();

  };

} // end namespace pal

#endif
