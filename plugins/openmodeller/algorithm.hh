/**
 * Declaration of Algorithm class.
 * 
 * @file
 * @author Mauro E S Muñoz (mauro@cria.org.br)
 * @date   2003-05-26
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


#ifndef _ALGORITHMHH_
#define _ALGORITHMHH_


#include <sampler.hh>


/****************************************************************/
/*************************** Algorithm **************************/

/** 
 * Base class for all distribution modeling algorithms. Provide 
 * methods to all derived classes so that they can do data sampling
 * and access the environmental layers to build the model.
 */
class Algorithm
{
public:

  Algorithm( Sampler *samp );
  virtual ~Algorithm();

  virtual char *name()  { return "Undefined"; }

  /** The algorithm should return != 0 if it needs normalization
   *  of environmental variables (non categorical ones). */
  virtual int needNormalization( Scalar *min, Scalar *max )
  { return 0; }

  /** Initiate a new training. If 'ncicle' != 0, then the 
   *  new training will have 'ncicle' cicles. */
  virtual int initialize( int ncicle ) { return 1; }

  /** One step further on the training. Return 0 if something 
   *  wrong happened. */
  virtual int iterate() = 0;

  /** Return != 0 if algorithm finished. */
  virtual int done() = 0;

  /** Read algorithm state. */
  virtual int load( int fd )  { return 0; }
  /** Store algorithm state. */
  virtual int save( int fd )  { return 0; }

  /** Return value of model on coordinates (x1,...,xN).
   *  Note that N is known as 'Sampler'.
   *  The value will be better visualized if within the range [-1,+1]. */
  virtual Scalar getValue( Scalar *x ) = 0;

  /** Return value of the convergence function. */
  virtual int getConvergence( Scalar *val )  { return 0; }


protected:

  Sampler *getSampler()  { return f_samp; }

  int *getCategories()   { return f_categ; }

  /** Return != 0 if variable "i" in the domain is categorical.
   *  Obs: i = 0 is the probability of occurrrence, therefore it is 
   *  never categorical. */
  int isCategorical( int i )  { return f_categ[i]; }

  /** Dimension of problem domain: number of independent 
   *  variables (climatic + soil) added to the number of
   *  dependent variables (occurrence prediction). */
  int dimDomain()  { return f_samp->dim(); }


private:

  Sampler *f_samp;

  int *f_categ; ///< f_categ[i] != 0 if map "i" is categorical.
};


#endif
