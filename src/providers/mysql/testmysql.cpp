/***************************************************************************
     testmysql.cpp
     --------------------------------------
    Date                 : Sun Sep 16 12:18:04 AKDT 2007
    Copyright            : (C) 2007 by Gary E. Sherman
    Email                : sherman at mrcc dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <iostream>

extern "C"{
#include <mysql.h>
}
int main(int argc, char **argv)
{
  std::cout << "MySQL connection test" << std::endl; 
  // init mysql 
  MYSQL db;
  MYSQL *res = mysql_init(&db);
  MYSQL *con = mysql_real_connect(&db, "localhost", "gsherman", 0, "test", 0, NULL, 0);
  if(con)
  {
    std::cout << "Connection successful" << std::endl; 
  }
  else
  {
    std::cout << "Connection failed" << mysql_error(&db) << std::endl; 
  }
  return 0;

}
