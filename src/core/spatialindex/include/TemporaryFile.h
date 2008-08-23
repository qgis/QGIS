// Tools Library
//
// Copyright (C) 2004  Navel Ltd.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  Email:
//    mhadji@gmail.com

#ifndef __tools_temporary_file_h
#define __tools_temporary_file_h

namespace Tools
{
  class TemporaryFile
  {
    public:
      TemporaryFile();
      virtual ~TemporaryFile();

      void storeNextObject( ISerializable* r );
      void storeNextObject( unsigned long len, const byte* const data );
      void loadNextObject( ISerializable* r );
      void loadNextObject( byte** data, unsigned long& len );

      void rewindForReading();
      void rewindForWriting();

    private:
      std::fstream m_file;
      std::vector<std::string> m_strFileName;
      unsigned long m_currentFile;
      unsigned long m_fileSize;
      bool m_bEOF;
  }; // TemporaryFile
}

#endif /* __tools_temporary_file_h */

