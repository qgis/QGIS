/**
 * Declaration of FileParser class.
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

#ifndef _FILE_PARSERHH_
#define _FILE_PARSERHH_

#include <list.hh>

/**
 * Read key/value pairs from a configuration file.
 * 
 */
class FileParser
{
  typedef struct
  {
    char *key;
    char *val;
  } Item;
  typedef List<Item *> ItemList;


public:

  FileParser( char *file );

  ~FileParser();

  int load( char *file );

  /**
  * Get the value of a key (case insensitive).
  */
  char *get( char *key );

  /**
  * Return the number of times that a certain key appears in the file.
  */
  int count( char *key );

  /**
  * Fill 'values' with all values associated with 'key' and 
  * return the number of values found.
  */
  int getAll( char *key, char **values );

  int length()   { return f_lst.Length(); }
  
  void head()    { f_lst.Head(); }
  void next()    { f_lst.Next(); }

  char *key()    { return f_lst.Get() ? f_lst.Get()->key : 0; }
  char *value()  { return f_lst.Get() ? f_lst.Get()->val : 0; }


private:

  void clear();

  // Aloca '*dst' e copia a string apontado por 'src' para '*dst'.
  // Elimina os espaços, tabulações e mudança de linhas do início
  // e do final de 'src'.
  void transfer( char **dst, char *src );


  ItemList f_lst;
};


#endif
