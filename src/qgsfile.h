/***************************************************************************
             qgsfile.h  - Basic file class, QFile add-in.
                           -------------------
  begin                : 2005-10-08
  copyright            : (C) 2005 Mateusz Łoskot
  email                : mateusz at loskot dot net
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/ 
/* $Id$ */
#ifndef QGSFILE_H
#define QGSFILE_H

#include <qstring.h>
#include <qfile.h>

/**
 * \class QgsFile
 * \brief Provides common interface to file operations.
 * Intentionally, this class tries to provide very similar interface
 * as QFile class from QT4. Just for compatibility in future.
 *
 * Future Note: QFile inherits from QObject, but QgsFile does not.
 */
class QgsFile : public QFile
{
public:	// Public functions

  //! Default constructor
  QgsFile();

  //! Constructs a QFile with given file name.
  QgsFile( const QString& name );

  //! Destructor
  virtual ~QgsFile();

  /**
   * Copies the file currently specified by fileName() to newName.
   *  Returns true if successful; otherwise returns false.
   * The file is closed before it is copied.
   * @param newName new file path
   * @return true if copy operation succeeded, false otherwise
   * @todo Check if dest dir exists
   */
  bool copy( const QString& newName );

  /**
   * Renames the file currently specified by fileName() to newName.
   * The file is closed before it is renamed.
   * @param newName new file name
   * @return true if successful; otherwise returns false
   * 
   * @todo Implementation
   */
  bool rename ( const QString & newName );

public:	// Static public functions

  /**
   * This is an overloaded member function, provided for convenience.
   * It behaves essentially like the above function.
   * Copies the file fileName to newName. Returns true if successful; otherwise returns false.
   * @param fileName source file to copy
   * @param newName new path to copy destination
   * @return true if copy operation succeeded, false otherwise
   */
  static bool copy ( const QString& fileName, const QString& newName );

private:  // Private functions

  /**
   * Disable copy ctor
   * IMPLEMENTATION OMITTED 
   */
  QgsFile(const QgsFile &); // 

  /**
   * Disable assignmend operator
   * IMPLEMENTATION OMITTED 
   */
  QgsFile& operator=(const QgsFile&);

}; // class QgsFile

#endif // #ifndef QGSFILE_H
