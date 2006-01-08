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
  MYSQL *con = mysql_real_connect(&db, "localhost", "sherman", "javado", "teZZt", 0, NULL, 0);
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
