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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifndef INTERNAL_EXCEPTION_H
#define INTERNAL_EXCEPTION_H

#include <exception>

namespace pal
{

  /** \brief Various Exceptions
   */
  class InternalException
  {
    public:

      /** \brief Thrown when something is added in a Full set
      */
      class Full : public std::exception
      {
          const char* what() const throw()
          {
            return "This set is full...";
          }
      };

      /** \brief Thrown when trying to access an empty dada set
      */
      class Empty : public std::exception
      {
          const char* what() const throw()
          {
            return "This set is empty...";
          }
      };

      /** \brief Thrown when a geometry type is not like expected
      */
      class WrongGeometry : public std::exception
      {
          const char* what() const throw()
          {
            return "GeometryTypeId is not expected...";
          }
      };

      /** \brief Thrown when a geometry type is not like expected
      */
      class UnknownGeometry : public std::exception
      {
          const char* what() const throw()
          {
            return "Geometry Type is unknow";
          }
      };


      /** \brief Throw an exception when it's impossible to compute labelPOsition
      */
      class NoLabelPosition : public std::exception
      {
          const char * what() const throw()
          {
            return "No way to compute positions";
          }
      };
  };

} // end namespace

#endif
