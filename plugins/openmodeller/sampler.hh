/**
 * Declaration of Sampler and Samples classes.
 * 
 * @file
 * @author Mauro E S Muñoz (mauro@cria.org.br)
 * @date   2003-05-27
 * $Id$
 * 
 * LICENSE INFORMATION 
 * 
 * Copyright(c) 2003 by CRIA -
 * Centro de Referencia em Informacao Ambiental
 *
 * http://www.cria.org.br
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details:
 * 
 * http://www.gnu.org/copyleft/gpl.html
 */

#ifndef _SAMPLERHH_
#define _SAMPLERHH_

#include <defs.hh>


/****************************************************************/
/**************************** Samples ***************************/

/** 
 * Facilitate dealing with samples.
 */
class Samples
{
public:

  /** If 'points' is given, do not free the memory pointed to! */
  Samples( int length, int dimension, Scalar *points=0 );
  ~Samples();

  /** Redimension. */
  int redim( int length, int dimension, Scalar *points=0 );

  Scalar *operator[](int i)        {return pnt + i*dim;}
  Scalar  operator()(int i, int j) {return *(pnt + i*dim + j);}

  int dim;  ///< Spatial dimension of each point.
  int len;  ///< Number of points.

  Scalar *pnt;


private:

  int f_free;
};




/****************************************************************/
/*************************** Sampler ****************************/

class Occurrences;
class Environment;

/** 
 * Base class to create samplers of environmental variables and
 * occurrence data. Each derived class can implement different 
 * behaviors, such as proportional sampling according to the 
 * distribution of occurrence data, disproportionate sampling
 * regarding presence and absence data, etc.
 */
class Sampler
{
public:

  Sampler( Occurrences *oc, Environment *env );

  ~Sampler();


  /** Dimension of sampled space (number of dependent and independend variables). */
  int dim();

  /** Number of occurrences (presence). */
  int numOccurrences();

  /** Return only presence data. */
  virtual int getOccurrences( int numPoints, Scalar *points );
  /** Return only presence data. */
  int getOccurrences( int numPoints, Samples *data );

  /** By default, sample uniformly presence and absence data. */
  virtual int getSamples( int numPoints, Scalar *points );
  /** By default, sample uniformly presence and absence data. */
  int getSamples( int numPoints, Samples *data );

  /** Set types[i] = 1 if variable associated to "i" is categorical (eg:
      soil), otherwise set types[i] = 0. i = 0, ..., dim()-1 */
  int varTypes( int *types );


protected:

  Occurrences *f_oc;
  Environment *f_env;
};


#endif
