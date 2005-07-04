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

#include  <openmodeller/AlgMetadata.hh>

#include <openmodeller/Occurrences.hh>
#include <openmodeller/MapFormat.hh>

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

  int occurrencesSet() { return _occurrencesSet; }
  int environmentSet() { return _environmentSet; }
  int projectionSet()  { return _projectionSet; }
  int algorithmSet()   { return _algorithmSet; }

  OccurrencesPtr getOccurrences();

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

  int _occurrencesSet;
  int _environmentSet;
  int _projectionSet;
  int _algorithmSet;

  OccurrencesPtr _occurrences;

  bool _nonNativeProjection;
  std::vector<std::string> _projectionCategoricalMap;
  std::vector<std::string> _projectionMap;
  std::string _inputMask;
  std::string _outputMask;
  std::string _inputModelFile;
  std::string _outputModelFile;
  std::string _projectionFile;

  MapFormat _outputFormat;

};


#endif

