/**
 * Se an OpenModeller object reading parameters from a request file.
 * 
 * @file
 * @author Mauro E S Muñoz (mauro@cria.org.br)
 * @date   2004-06-22
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

#ifndef _REQUEST_FILEHH_
#define _REQUEST_FILEHH_

#include  <om_algorithm_metadata.hh>

class OpenModeller;
class FileParser;
class Occurrences;
class AlgParameter;


/**************************************************************/
/************************ Request File ************************/

class RequestFile
{
public:

  RequestFile()  {}

  /** Loads parameters from the request file.
   * 
   * @param om OpenModeller object to be configured.
   * @param request_file File from which the parameters will be read.
   * 
   * @return Number of parameters missing in the request file.
   */
  int configure( OpenModeller *om, char *request_file );

  int occurrencesSetted()  { return _occurrences_setted; }
  int environmentSetted()  { return _environment_setted; }
  int outputMapSetted()    { return _outputmap_setted; }
  int algorithmSetted()    { return _algorithm_setted; }


private:

  int setOccurrences( OpenModeller *om, FileParser &fp );
  int setEnvironment( OpenModeller *om, FileParser &fp );
  int setOutputMap  ( OpenModeller *om, FileParser &fp );
  int setAlgorithm  ( OpenModeller *om, FileParser &fp );

  Occurrences *readOccurrences( char *file, char *name,
                                char *coord_system );

  int readParameters( AlgParameter *result, AlgMetadata *metadata,
                      int str_nparam, char **str_param );

  char *extractParameter( char *name, int nvet, char **vet );


  int _occurrences_setted;
  int _environment_setted;
  int _outputmap_setted;
  int _algorithm_setted;
};


#endif

