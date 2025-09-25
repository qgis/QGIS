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

#include <fstream>
#include <cstring>

// For checking if a file exists - hobu
#include <sys/stat.h>

#ifdef WIN32
#define stat _stat64
#endif

#include <spatialindex/SpatialIndex.h>
#include "DiskStorageManager.h"
#include <spatialindex/tools/Tools.h>

using namespace SpatialIndex;
using namespace SpatialIndex::StorageManager;

bool CheckFilesExists(Tools::PropertySet& ps)
{
	bool bExists = false;

	std::string filename("");
	std::string idx("idx");
	std::string dat("dat");

	Tools::Variant idx_name;
	Tools::Variant dat_name;
	Tools::Variant fn;

	idx_name = ps.getProperty("FileNameIdx");
	dat_name = ps.getProperty("FileNameDat");
	fn = ps.getProperty("FileName");

	if (idx_name.m_varType != Tools::VT_EMPTY) dat = std::string(idx_name.m_val.pcVal);
	if (dat_name.m_varType != Tools::VT_EMPTY) idx = std::string(dat_name.m_val.pcVal);
	if (fn.m_varType != Tools::VT_EMPTY) filename = std::string(fn.m_val.pcVal);

	struct stat stats;

	std::ostringstream os;
	int ret;
	os << filename <<"."<<dat;
	std::string data_name = os.str();
	ret = stat(data_name.c_str(), &stats);

	if (ret == 0) bExists = true;

	os.str("");
	os << filename <<"."<<idx;
	std::string index_name = os.str();
	ret = stat(index_name.c_str(), &stats);

	if ((ret == 0) && (bExists == true)) bExists = true;

	return bExists;
}
SpatialIndex::IStorageManager* SpatialIndex::StorageManager::returnDiskStorageManager(Tools::PropertySet& ps)
{
	IStorageManager* sm = new DiskStorageManager(ps);
	return sm;
}

SpatialIndex::IStorageManager* SpatialIndex::StorageManager::createNewDiskStorageManager(std::string& baseName, uint32_t pageSize)
{
	Tools::Variant var;
	Tools::PropertySet ps;

	var.m_varType = Tools::VT_BOOL;
	var.m_val.blVal = true;
	ps.setProperty("Overwrite", var);
		// overwrite the file if it exists.

	var.m_varType = Tools::VT_PCHAR;
	var.m_val.pcVal = const_cast<char*>(baseName.c_str());
	ps.setProperty("FileName", var);
		// .idx and .dat extensions will be added.

	var.m_varType = Tools::VT_ULONG;
	var.m_val.ulVal = pageSize;
	ps.setProperty("PageSize", var);
		// specify the page size. Since the index may also contain user defined data
		// there is no way to know how big a single node may become. The storage manager
		// will use multiple pages per node if needed. Off course this will slow down performance.

	return returnDiskStorageManager(ps);
}

SpatialIndex::IStorageManager* SpatialIndex::StorageManager::loadDiskStorageManager(std::string& baseName)
{
	Tools::Variant var;
	Tools::PropertySet ps;

	var.m_varType = Tools::VT_PCHAR;
	var.m_val.pcVal = const_cast<char*>(baseName.c_str());
	ps.setProperty("FileName", var);
		// .idx and .dat extensions will be added.

	return returnDiskStorageManager(ps);
}

DiskStorageManager::DiskStorageManager(Tools::PropertySet& ps) : m_pageSize(0), m_nextPage(-1), m_buffer(nullptr)
{
	Tools::Variant var;

	// Open/Create flag.
	bool bOverwrite = false;
	bool bFileExists = false;
	std::streamoff length = 0;

	var = ps.getProperty("Overwrite");

	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (var.m_varType != Tools::VT_BOOL)
			throw Tools::IllegalArgumentException("SpatialIndex::DiskStorageManager: Property Overwrite must be Tools::VT_BOOL");
		bOverwrite = var.m_val.blVal;
	}

	// storage filename.
	var = ps.getProperty("FileName");

	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (!(var.m_varType == Tools::VT_PCHAR ||
            var.m_varType == Tools::VT_PWCHAR))
			throw Tools::IllegalArgumentException("SpatialIndex::DiskStorageManager: Property FileName must be Tools::VT_PCHAR or Tools::VT_PWCHAR");

		std::string idx("idx");
		std::string dat("dat");

		Tools::Variant idx_name = ps.getProperty("FileNameIdx");
		if (idx_name.m_varType != Tools::VT_EMPTY) idx = std::string(idx_name.m_val.pcVal);

		Tools::Variant dat_name = ps.getProperty("FileNameDat");
		if (dat_name.m_varType != Tools::VT_EMPTY) dat = std::string(dat_name.m_val.pcVal);

		std::string sIndexFile = std::string(var.m_val.pcVal) + "." + idx;
		std::string sDataFile = std::string(var.m_val.pcVal) + "." + dat;

		// check if file exists.
		bFileExists = CheckFilesExists(ps);

		// check if file can be read/written.
		if (bFileExists == true && bOverwrite == false)
		{
            std::ios_base::openmode mode = std::ios::in | std::ios::out | std::ios::binary;
			m_indexFile.open(sIndexFile.c_str(), mode);
			m_dataFile.open(sDataFile.c_str(), mode);

			if (m_indexFile.fail() || m_dataFile.fail())
				throw Tools::IllegalArgumentException("SpatialIndex::DiskStorageManager: Index/Data file cannot be read/written.");
		}
		else
		{
            std::ios_base::openmode mode = std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc;
			m_indexFile.open(sIndexFile.c_str(), mode);
			m_dataFile.open(sDataFile.c_str(), mode);

			if (m_indexFile.fail() || m_dataFile.fail())
				throw Tools::IllegalArgumentException("SpatialIndex::DiskStorageManager: Index/Data file cannot be created.");

		}
	}
	else
	{
		throw Tools::IllegalArgumentException("SpatialIndex::DiskStorageManager: Property FileName was not specified.");
	}

	// get current length of file
	m_indexFile.seekg (0, m_indexFile.end);
	length = m_indexFile.tellg();
	m_indexFile.seekg (0, m_indexFile.beg);

	// find page size.
	if ((bOverwrite == true) || (length == 0) || (bFileExists == false))
	{
		var = ps.getProperty("PageSize");

		if (var.m_varType != Tools::VT_EMPTY)
		{
			if (var.m_varType != Tools::VT_ULONG)
				throw Tools::IllegalArgumentException("SpatialIndex::DiskStorageManager: Property PageSize must be Tools::VT_ULONG");
			m_pageSize = var.m_val.ulVal;
			m_nextPage = 0;
		}
		else
		{
			throw Tools::IllegalArgumentException("SpatialIndex::DiskStorageManager: A new storage manager is created and property PageSize was not specified.");
		}
	}
	else
	{
		m_indexFile.read(reinterpret_cast<char*>(&m_pageSize), sizeof(uint32_t));
		if (m_indexFile.fail())
			throw Tools::IllegalStateException("SpatialIndex::DiskStorageManager: Failed reading pageSize.");

		m_indexFile.read(reinterpret_cast<char*>(&m_nextPage), sizeof(id_type));
		if (m_indexFile.fail())
			throw Tools::IllegalStateException("SpatialIndex::DiskStorageManager: Failed reading nextPage.");
	}

	// create buffer.
	m_buffer = new uint8_t[m_pageSize];
	memset(m_buffer, 0, m_pageSize);

	if ((bOverwrite == false) && (length > 0))
	{
		uint32_t count;
		id_type page, id;

		// load empty pages in memory.
		m_indexFile.read(reinterpret_cast<char*>(&count), sizeof(uint32_t));
		if (m_indexFile.fail())
			throw Tools::IllegalStateException("SpatialIndex::DiskStorageManager: Corrupted storage manager index file.");

		for (uint32_t cCount = 0; cCount < count; ++cCount)
		{
			m_indexFile.read(reinterpret_cast<char*>(&page), sizeof(id_type));
			if (m_indexFile.fail())
				throw Tools::IllegalStateException("SpatialIndex::DiskStorageManager: Corrupted storage manager index file.");
			m_emptyPages.insert(page);
		}

		// load index table in memory.
		m_indexFile.read(reinterpret_cast<char*>(&count), sizeof(uint32_t));
		if (m_indexFile.fail())
			throw Tools::IllegalStateException("SpatialIndex::DiskStorageManager: Corrupted storage manager index file.");

		for (uint32_t cCount = 0; cCount < count; ++cCount)
		{
			Entry* e = new Entry();

			m_indexFile.read(reinterpret_cast<char*>(&id), sizeof(id_type));
			if (m_indexFile.fail())
				throw Tools::IllegalStateException("SpatialIndex::DiskStorageManager: Corrupted storage manager index file.");

			m_indexFile.read(reinterpret_cast<char*>(&(e->m_length)), sizeof(uint32_t));
			if (m_indexFile.fail())
				throw Tools::IllegalStateException("SpatialIndex::DiskStorageManager: Corrupted storage manager index file.");

			uint32_t count2;
			m_indexFile.read(reinterpret_cast<char*>(&count2), sizeof(uint32_t));
			if (m_indexFile.fail())
				throw Tools::IllegalStateException("SpatialIndex::DiskStorageManager: Corrupted storage manager index file.");

			for (uint32_t cCount2 = 0; cCount2 < count2; ++cCount2)
			{
				m_indexFile.read(reinterpret_cast<char*>(&page), sizeof(id_type));
				if (m_indexFile.fail())
					throw Tools::IllegalStateException("SpatialIndex::DiskStorageManager: Corrupted storage manager index file.");
				e->m_pages.push_back(page);
			}
			m_pageIndex.insert(std::pair<id_type, Entry* >(id, e));
		}
	}
}

DiskStorageManager::~DiskStorageManager()
{
	flush();
	m_indexFile.close();
	m_dataFile.close();
	if (m_buffer != nullptr) delete[] m_buffer;

	for (auto& v: m_pageIndex)
		delete v.second;
}

void DiskStorageManager::flush()
{
	m_indexFile.seekp(0, std::ios_base::beg);
	if (m_indexFile.fail())
		throw Tools::IllegalStateException("SpatialIndex::DiskStorageManager: Corrupted storage manager index file.");

	m_indexFile.write(reinterpret_cast<const char*>(&m_pageSize), sizeof(uint32_t));
	if (m_indexFile.fail())
		throw Tools::IllegalStateException("SpatialIndex::DiskStorageManager: Corrupted storage manager index file.");

	m_indexFile.write(reinterpret_cast<const char*>(&m_nextPage), sizeof(id_type));
	if (m_indexFile.fail())
		throw Tools::IllegalStateException("SpatialIndex::DiskStorageManager: Corrupted storage manager index file.");

	uint32_t count = static_cast<uint32_t>(m_emptyPages.size());
	m_indexFile.write(reinterpret_cast<const char*>(&count), sizeof(uint32_t));
	if (m_indexFile.fail())
			throw Tools::IllegalStateException("SpatialIndex::DiskStorageManager: Corrupted storage manager index file.");

	for (std::set<id_type>::const_iterator it = m_emptyPages.begin(); it != m_emptyPages.end(); ++it)
	{
		m_indexFile.write(reinterpret_cast<const char*>(&(*it)), sizeof(id_type));
		if (m_indexFile.fail())
			throw Tools::IllegalStateException("SpatialIndex::DiskStorageManager: Corrupted storage manager index file.");
	}

	count = static_cast<uint32_t>(m_pageIndex.size());
	m_indexFile.write(reinterpret_cast<const char*>(&count), sizeof(uint32_t));
	if (m_indexFile.fail())
		throw Tools::IllegalStateException("SpatialIndex::DiskStorageManager: Corrupted storage manager index file.");

	for (std::map<id_type, Entry*>::iterator it = m_pageIndex.begin(); it != m_pageIndex.end(); ++it)
	{
		m_indexFile.write(reinterpret_cast<const char*>(&((*it).first)), sizeof(id_type));
		if (m_indexFile.fail())
			throw Tools::IllegalStateException("SpatialIndex::DiskStorageManager: Corrupted storage manager index file.");

		m_indexFile.write(reinterpret_cast<const char*>(&((*it).second->m_length)), sizeof(uint32_t));
		if (m_indexFile.fail())
			throw Tools::IllegalStateException("SpatialIndex::DiskStorageManager: Corrupted storage manager index file.");

		count = static_cast<uint32_t>((*it).second->m_pages.size());
		m_indexFile.write(reinterpret_cast<const char*>(&count), sizeof(uint32_t));
		if (m_indexFile.fail())
			throw Tools::IllegalStateException("SpatialIndex::DiskStorageManager: Corrupted storage manager index file.");

		for (uint32_t cIndex = 0; cIndex < count; ++cIndex)
		{
			m_indexFile.write(reinterpret_cast<const char*>(&((*it).second->m_pages[cIndex])), sizeof(id_type));
			if (m_indexFile.fail())
				throw Tools::IllegalStateException("SpatialIndex::DiskStorageManager: Corrupted storage manager index file.");
		}
	}

	m_indexFile.flush();
	m_dataFile.flush();
}

void DiskStorageManager::loadByteArray(const id_type page, uint32_t& len, uint8_t** data)
{
	std::map<id_type, Entry*>::iterator it = m_pageIndex.find(page);

	if (it == m_pageIndex.end())
		throw InvalidPageException(page);

	std::vector<id_type>& pages = (*it).second->m_pages;
	uint32_t cNext = 0;
	uint32_t cTotal = static_cast<uint32_t>(pages.size());

	len = (*it).second->m_length;
	*data = new uint8_t[len];

	uint8_t* ptr = *data;
	uint32_t cLen;
	uint32_t cRem = len;

	do
	{
		m_dataFile.seekg(pages[cNext] * m_pageSize, std::ios_base::beg);
		if (m_dataFile.fail())
			throw Tools::IllegalStateException("SpatialIndex::DiskStorageManager: Corrupted data file.");

		m_dataFile.read(reinterpret_cast<char*>(m_buffer), m_pageSize);
		if (m_dataFile.fail())
			throw Tools::IllegalStateException("SpatialIndex::DiskStorageManager: Corrupted data file.");

		cLen = (cRem > m_pageSize) ? m_pageSize : cRem;
		memcpy(ptr, m_buffer, cLen);

		ptr += cLen;
		cRem -= cLen;
		++cNext;
	}
	while (cNext < cTotal);
}

void DiskStorageManager::storeByteArray(id_type& page, const uint32_t len, const uint8_t* const data)
{
	if (page == NewPage)
	{
		Entry* e = new Entry();
		e->m_length = len;

		const uint8_t* ptr = data;
		id_type cPage;
		uint32_t cRem = len;
		uint32_t cLen;

		while (cRem > 0)
		{
			if (! m_emptyPages.empty())
			{
				cPage = *m_emptyPages.begin();
				m_emptyPages.erase(m_emptyPages.begin());
			}
			else
			{
				cPage = m_nextPage;
				++m_nextPage;
			}

			cLen = (cRem > m_pageSize) ? m_pageSize : cRem;
			memcpy(m_buffer, ptr, cLen);

			m_dataFile.seekp(cPage * m_pageSize, std::ios_base::beg);
			if (m_dataFile.fail())
				throw Tools::IllegalStateException("SpatialIndex::DiskStorageManager: Corrupted data file.");

			m_dataFile.write(reinterpret_cast<const char*>(m_buffer), m_pageSize);
			if (m_dataFile.fail())
				throw Tools::IllegalStateException("SpatialIndex::DiskStorageManager: Corrupted data file.");

			ptr += cLen;
			cRem -= cLen;
			e->m_pages.push_back(cPage);
		}

		page = e->m_pages[0];
		m_pageIndex.insert(std::pair<id_type, Entry*>(page, e));
	}
	else
	{
		// find the entry.
		std::map<id_type, Entry*>::iterator it = m_pageIndex.find(page);

		// check if it exists.
		if (it == m_pageIndex.end())
			throw InvalidPageException(page);

		Entry* oldEntry = (*it).second;

		m_pageIndex.erase(it);

		Entry* e = new Entry();
		e->m_length = len;

		const uint8_t* ptr = data;
		id_type cPage;
		uint32_t cRem = len;
		uint32_t cLen, cNext = 0;

		while (cRem > 0)
		{
			if (cNext < oldEntry->m_pages.size())
			{
				cPage = oldEntry->m_pages[cNext];
				++cNext;
			}
			else if (! m_emptyPages.empty())
			{
				cPage = *m_emptyPages.begin();
				m_emptyPages.erase(m_emptyPages.begin());
			}
			else
			{
				cPage = m_nextPage;
				++m_nextPage;
			}

			cLen = (cRem > m_pageSize) ? m_pageSize : cRem;
			memcpy(m_buffer, ptr, cLen);

			m_dataFile.seekp(cPage * m_pageSize, std::ios_base::beg);
			if (m_dataFile.fail())
				throw Tools::IllegalStateException("SpatialIndex::DiskStorageManager: Corrupted data file.");

			m_dataFile.write(reinterpret_cast<const char*>(m_buffer), m_pageSize);
			if (m_dataFile.fail())
				throw Tools::IllegalStateException("SpatialIndex::DiskStorageManager: Corrupted data file.");

			ptr += cLen;
			cRem -= cLen;
			e->m_pages.push_back(cPage);
		}

		while (cNext < oldEntry->m_pages.size())
		{
			m_emptyPages.insert(oldEntry->m_pages[cNext]);
			++cNext;
		}

		m_pageIndex.insert(std::pair<id_type, Entry*>(page, e));
		delete oldEntry;
	}
}

void DiskStorageManager::deleteByteArray(const id_type page)
{
	std::map<id_type, Entry*>::iterator it = m_pageIndex.find(page);

	if (it == m_pageIndex.end())
		throw InvalidPageException(page);

	for (uint32_t cIndex = 0; cIndex < (*it).second->m_pages.size(); ++cIndex)
	{
		m_emptyPages.insert((*it).second->m_pages[cIndex]);
	}

	delete (*it).second;
	m_pageIndex.erase(it);
}
