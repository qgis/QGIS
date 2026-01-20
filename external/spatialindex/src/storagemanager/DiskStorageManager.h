/******************************************************************************
 * Project:  libspatialindex - A C++ library for spatial indexing
 * Author:   Marios Hadjieleftheriou, mhadji@gmail.com
 ******************************************************************************
 * Copyright (c) 2002, Marios Hadjieleftheriou
 *
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
******************************************************************************/

#pragma once

namespace SpatialIndex
{
	namespace StorageManager
	{
		class DiskStorageManager : public SpatialIndex::IStorageManager
		{
		public:
			DiskStorageManager(Tools::PropertySet&);
			~DiskStorageManager() override;

			void flush() override;
			void loadByteArray(const id_type page, uint32_t& len, uint8_t** data) override;
			void storeByteArray(id_type& page, const uint32_t len, const uint8_t* const data) override;
			void deleteByteArray(const id_type page) override;

		private:
			class Entry
			{
			public:
				uint32_t m_length;
				std::vector<id_type> m_pages;
			};

		protected:
			std::fstream m_dataFile;
			std::fstream m_indexFile;
			uint32_t m_pageSize;
			id_type m_nextPage;
			std::set<id_type> m_emptyPages;
			std::map<id_type, Entry*> m_pageIndex;

			uint8_t* m_buffer;
		}; // DiskStorageManager
	}
}
