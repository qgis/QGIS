/**
 * Declaration of ControlInterface class.
 * 
 * @file
 * @author Mauro E S Muñoz <mauro@cria.org.br>
 * @date 2003-09-25
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

#ifndef _CONTROL_INTERFACEHH_
#define _CONTROL_INTERFACEHH_

#include <env_io/header.hh>

class Algorithm;
class Environment;
class Sampler;
class Occurrences;
class RasterFile;
class Map;

/**
 * Defines and implements all commands to interface with the model
 * generator.
 * 
 */
class ControlInterface
{
public:

  ControlInterface();

  /**
   * @param cs     Common coordinate system.
   * @param ncateg Number of categorical layers (they all need to be in the initial portion of "layers");
   * @param nlayer Total number of environmental layers;
   * @param layer  Names of the environmental layers;
   */
  ControlInterface( char *cs, int ncateg, int nlayer, char **layers,
	       char *mask=0 );

  ~ControlInterface();


  // Define environmental layers and other basic settings.
  void setEnvironment( char *cs, int ncateg, int nlayer,
		       char **layers, char *mask=0 );

  // Define output map.
  void setOutputMap( char *file, Header &hdr, Scalar mult );

  // Define algorithm that will be used to generate the distribution map.
  void setAlgorithm( char *alg, char *param=0 );

  /**
   * Define occurrence points to be used.
   * 
   * @param file File name with localities (coordinates) from
   *             the occurrences;
   * @param cs   Coordinate system of all localities in the file.
   * @param oc   Name of the species of interest. If == 0, then
   *             use the first species found in file.
   */
  void setOccurrences( char *file, char *cs, char *oc=0 );

  int run();

  char *error()  { return f_error; }


private:

  /** Reallocate *dst and copy content from *src.*/
  void stringCopy( char **dst, char *src );

  /** Check if all necessary parameters have been defined.
   *  If not, an error message is returned.*/
  char *basicCheck();

  /** Read occurences of species 'name', provided in the coordinate
   *  system 'cs', from the file 'file'.*/
  Occurrences *readOccurrences( char *file, char *cs, char *name=0 );

  /** Return the object that implements the algorithm to be used.*/
  Algorithm *algorithmFactory( Sampler *samp, char *name,
			       char *params );

  /** Build the model based on 'samp' and on algorithm.*/
  int createModel( Algorithm *alg, Sampler *samp, int max_cicles);

  /** Generate output map defined by 'f_file' and 'f_hdr'.*/
  int createMap( Environment *env, Algorithm *alg );


  char  *f_cs;     ///< Common Coordinate System.
  int    f_ncateg;
  int    f_nlayers;
  char **f_layers;
  char  *f_mask;

  char   *f_file;  ///< Output map.
  Header  f_hdr;
  Scalar f_mult;  ///< Output multiplier.

  char *f_alg;    ///< Algorithm's name and parameters.
  char *f_param;
  int   f_ncicle; ///< Max algorithm cicles.

  char *f_oc_file;
  char *f_oc_cs;   ///< Occurrences Coordinate System.
  char *f_oc_name;

  char *f_error;
};


#endif
