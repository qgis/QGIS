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

#ifndef PAL_EXCEPTION_H
#define PAL_EXCEPTION_H

#include <exception>


namespace pal
{

  /** \brief Various Exceptions
   * \ingroup core
   */
  class PalException
  {
    public:

      /** \brief Thrown when a feature is not yet implemented
       * \ingroup core
      */
      class NotImplemented : public std::exception
      {
          const char * what() const throw() override
          {
            return "Not yet implemented... sorry";
          }
      };

      /** \brief Try to access an unknown feature
       * \ingroup core
      */
      class UnknownFeature : public std::exception
      {
          const char * what() const throw() override
          {
            return "Feature not found";
          }
      };

      /** \brief Try to access an unknown layer
       * \ingroup core
      */
      class UnknownLayer : public std::exception
      {
          const char * what() const throw() override
          {
            return "Layer not found";
          }
      };

      /** \brief layer already exists
       * \ingroup core
      */
      class LayerExists : public std::exception
      {
          const char * what() const throw() override
          {
            return "Layers names must be unique";
          }
      };

      /** \brief features already exists
       * \ingroup core
      */
      class FeatureExists : public std::exception
      {
          const char * what() const throw() override
          {
            return "Features IDs must be unique within a layer";
          }
      };

      /** \brief thrown when a value is not in the valid scale range\
       * \ingroup core
       *
       *  It can be thrown by :
       *
       *    - pal::Layer::setFeatureLabelSize if either the height or the width of the label is < 0
       *
       *    - pal::Layer::setFeatureDistlabel is distlable < 0
       */
      class ValueNotInRange : public std::exception
      {
          const char * what() const throw() override
          {
            return "value not allowed";
          }
      };
  };

} // namespace

#endif
