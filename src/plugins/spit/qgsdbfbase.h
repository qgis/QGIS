/***************************************************************************
    qgsbdfbase.h - Dbase IV Header
     --------------------------------------
    Date                 : 25-Dec-2003
    Copyright            : (C) 2003 by Gary E.Sherman
    email                : sherman at mrcc.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

// Dbase header structure
#ifndef _MSC_VER
#include <stdint.h>
#else
typedef __int32 int32_t;
typedef __int16 int16_t;
#endif

struct DbaseHeader
{
  unsigned char valid_dbf;
  char year;
  char month;
  char day;
  int32_t num_recs;
  int16_t size_hdr;
  int16_t size_rec;
  char reserved[3];
  char lan[13];
  char reserved2[4];
};
// Field descriptor array - defines a field and its attributes (type,
// length, etc.
struct FieldDescriptorArray
{
  char field_name[11];
  char field_type;
  int32_t field_addr;  /* used only in memory */
  unsigned char field_length;
  unsigned char field_decimal;
  char reserved[2];
  char work_area;
  char lan[2];
  char set_fields;
  char reserved2[8];
};
// Typedefs
typedef struct FieldDescriptorArray Fda;
typedef struct DbaseHeader DbH;

// Field Array class
class DbaseFieldArray
{
  public:
    DbaseFieldArray( int numberOfFields );
    void addField( char *name, char type, unsigned char length,
                   unsigned char decimal );
    int getNumFields();
    Fda  *getField( int index );
  private:
    struct FieldDescriptorArray *fda;
    unsigned int fieldCount;
    int numFields;

};

// Dbase file class (incomplete implementation)
class DbaseFile
{
  public:
    DbaseFile( char *fileName, int numRecords, int recordSize, DbaseFieldArray &fda );
    void writeHeader();
    void writeFieldDescriptors();
    void writeRecord( const char *data );
    void closeFile();
    struct DbaseHeader header;
  private:
    char * fileName;
    int numRecords;
    int recordSize;
    DbaseFieldArray fieldArray;
    long pos;
    bool firstRecord;
};
