/**
 * Class used to configure an OpenModeller object reading parameters from a standard request file.
 * It also has methods to encapsulate model creation and projection.
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

#include <om_occurrences.hh>
#include <map_format.hh>

#include <string>
#include <vector>

class OpenModeller;
class FileParser;
class AlgParameter;

/**************************************************************/
/************************ Request File ************************/

class RequestFile
{
public:

  RequestFile();
  ~RequestFile();

  /** Loads parameters from the request file.
   * 
   * @param om OpenModeller object to be configured.
   * @param request_file File from which the parameters will be read.
   * 
   * @return Number of parameters missing in the request file.
   */
  int configure( OpenModeller *om, char *request_file );

  int occurrencesSet() { return _occurrences_set; }
  int environmentSet() { return _environment_set; }
  int projectionSet()  { return _projection_set; }
  int algorithmSet()   { return _algorithm_set; }

  OccurrencesPtr getOccurrences() { return _occurrences; }

  void makeModel( OpenModeller *om );
  void makeProjection( OpenModeller *om );

private:

  int setOccurrences( OpenModeller *om, FileParser &fp );
  int setEnvironment( OpenModeller *om, FileParser &fp );
  int setProjection ( OpenModeller *om, FileParser &fp );
  int setAlgorithm  ( OpenModeller *om, FileParser &fp );

  OccurrencesPtr readOccurrences( std::string file, std::string name,
                                std::string coord_system );

  int readParameters( AlgParameter *result, AlgMetadata const *metadata,
		      std::vector<std::string> str_param );

  /** Search for 'name' in the 'nvet' elements of the vector 'vet'.
   * If the string 'name' is in the beginning of some string vet[i]
   * then returns a pointer to the next character of vet[i],
   * otherwise returns 0.
   *
   * @param name Name to be searched.
   * @param vet Vector of names.
   * 
   * @return Pointer to the next character of matching vector element.
   */
  std::string extractParameter( std::string const name, 
				std::vector<std::string> vet );

  int _occurrences_set;
  int _environment_set;
  int _projection_set;
  int _algorithm_set;

  OccurrencesPtr _occurrences;

  bool _nonNativeProjection;
  std::vector<std::string> _cat;
  std::vector<std::string> _map;
  std::string _mask;
  std::string _model_file;
  std::string _projection_file;

  MapFormat _outputFormat;

};


#endif

