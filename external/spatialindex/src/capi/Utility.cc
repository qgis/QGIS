/******************************************************************************
 * Project:  libsidx - A C API wrapper around libspatialindex
 * Purpose:  C++ object declarations to implement utilities.
 * Author:   Howard Butler, hobu.inc@gmail.com
 ******************************************************************************
 * Copyright (c) 2009, Howard Butler
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

#include <spatialindex/capi/sidx_impl.h>

Tools::PropertySet* GetDefaults()
{
	Tools::PropertySet* ps = new Tools::PropertySet;

	Tools::Variant var;

	// Rtree defaults

	var.m_varType = Tools::VT_DOUBLE;
	var.m_val.dblVal = 0.7;
	ps->setProperty("FillFactor", var);

	var.m_varType = Tools::VT_ULONG;
	var.m_val.ulVal = 100;
	ps->setProperty("IndexCapacity", var);

	var.m_varType = Tools::VT_ULONG;
	var.m_val.ulVal = 100;
	ps->setProperty("LeafCapacity", var);

	var.m_varType = Tools::VT_LONG;
	var.m_val.lVal = SpatialIndex::RTree::RV_RSTAR;
	ps->setProperty("TreeVariant", var);

	// var.m_varType = Tools::VT_LONGLONG;
	// var.m_val.llVal = 0;
	// ps->setProperty("IndexIdentifier", var);

	var.m_varType = Tools::VT_ULONG;
	var.m_val.ulVal = 32;
	ps->setProperty("NearMinimumOverlapFactor", var);

	var.m_varType = Tools::VT_DOUBLE;
	var.m_val.dblVal = 0.4;
	ps->setProperty("SplitDistributionFactor", var);

	var.m_varType = Tools::VT_DOUBLE;
	var.m_val.dblVal = 0.3;
	ps->setProperty("ReinsertFactor", var);

	var.m_varType = Tools::VT_ULONG;
	var.m_val.ulVal = 2;
	ps->setProperty("Dimension", var);

	var.m_varType = Tools::VT_BOOL;
	var.m_val.bVal = true;
	ps->setProperty("EnsureTightMBRs", var);

	var.m_varType = Tools::VT_ULONG;
	var.m_val.ulVal = 100;
	ps->setProperty("IndexPoolCapacity", var);

	var.m_varType = Tools::VT_ULONG;
	var.m_val.ulVal = 100;
	ps->setProperty("LeafPoolCapacity", var);

	var.m_varType = Tools::VT_ULONG;
	var.m_val.ulVal = 1000;
	ps->setProperty("RegionPoolCapacity", var);

	var.m_varType = Tools::VT_ULONG;
	var.m_val.ulVal = 500;
	ps->setProperty("PointPoolCapacity", var);

	// horizon for TPRTree
	var.m_varType = Tools::VT_DOUBLE;
	var.m_val.dblVal = 20.0;
	ps->setProperty("Horizon", var);

	// Buffering defaults
	var.m_varType = Tools::VT_ULONG;
	var.m_val.ulVal = 10;
	ps->setProperty("Capacity", var);

	var.m_varType = Tools::VT_BOOL;
	var.m_val.bVal = false;
	ps->setProperty("WriteThrough", var);

	// Disk Storage Manager defaults
	var.m_varType = Tools::VT_BOOL;
	var.m_val.bVal = true;
	ps->setProperty("Overwrite", var);

	var.m_varType = Tools::VT_PCHAR;
	var.m_val.pcVal = const_cast<char*>("");
	ps->setProperty("FileName", var);

	var.m_varType = Tools::VT_ULONG;
	var.m_val.ulVal = 4096;
	ps->setProperty("PageSize", var);

	var.m_varType = Tools::VT_LONGLONG;
	var.m_val.llVal = 0;
	ps->setProperty("ResultSetLimit", var);

	// Our custom properties related to whether
	// or not we are using a disk or memory storage manager

	var.m_varType = Tools::VT_ULONG;
	var.m_val.ulVal = RT_Disk;
	ps->setProperty("IndexStorageType", var);

	var.m_varType = Tools::VT_ULONG;
	var.m_val.ulVal = RT_RTree;
	ps->setProperty("IndexType", var);

	var.m_varType = Tools::VT_PCHAR;
	var.m_val.pcVal = const_cast<char*>("dat");
	ps->setProperty("FileNameDat", var);

	var.m_varType = Tools::VT_PCHAR;
	var.m_val.pcVal = const_cast<char*>("idx");
	ps->setProperty("FileNameIdx", var);

    // Custom storage manager properties
    var.m_varType = Tools::VT_ULONG;
	var.m_val.pcVal = 0;
	ps->setProperty("CustomStorageCallbacksSize", var);

    var.m_varType = Tools::VT_PVOID;
	var.m_val.pcVal = 0;
	ps->setProperty("CustomStorageCallbacks", var);

    return ps;
}

void Page_ResultSet_Ids(IdVisitor& visitor, int64_t** ids, int64_t nStart, int64_t nResultLimit, uint64_t* nResults)
{
  int64_t nResultCount;

  nResultCount = visitor.GetResultCount();

  if (nResultLimit == 0)
  {
    // no offset paging
    nResultLimit = nResultCount;
    nStart = 0;
  }
  else
  {
    if ((nResultCount - (nStart + nResultLimit)) < 0)
    {
      // not enough results to fill nResultCount
      nStart = (std::min)(nStart, nResultCount);
      nResultCount = nStart + (std::min)(nResultLimit, nResultCount - nStart);
    }
    else
    {
      nResultCount = (std::min)(nResultCount, nStart + nResultLimit);
    }
  }

  *ids = (int64_t*) malloc (nResultLimit * sizeof(int64_t));

  std::vector<uint64_t>& results = visitor.GetResults();

  for (int64_t i = nStart; i < nResultCount; ++i)
  {
    (*ids)[i - nStart] = results[i];
  }

  *nResults = nResultCount - nStart;
}

void Page_ResultSet_Obj(ObjVisitor& visitor, IndexItemH** items, int64_t nStart, int64_t nResultLimit, uint64_t* nResults)
{
	int64_t nResultCount;

	nResultCount = visitor.GetResultCount();

	if (nResultLimit == 0)
	{
		// no offset paging
		nResultLimit = nResultCount;
		nStart = 0;
	}
	else
	{
		if ((nResultCount - (nStart + nResultLimit)) < 0)
		{
			// not enough results to fill nResultCount
			nStart = (std::min)(nStart, nResultCount);
			nResultCount = nStart + (std::min)(nResultLimit, nResultCount - nStart);
		}
		else
		{
			nResultCount = (std::min)(nResultCount, nStart + nResultLimit);
		}
	}

	*items = (IndexItemH*) malloc (nResultLimit * sizeof(SpatialIndex::IData*));

	std::vector<SpatialIndex::IData*>& results = visitor.GetResults();

	// copy the Items into the newly allocated item array
	// we need to make sure to copy the actual Item instead
	// of just the pointers, as the visitor will nuke them
	// upon ~
	for (int64_t i = nStart; i < nResultCount; ++i)
	{
		SpatialIndex::IData* result =results[i];
		(*items)[i - nStart] =  (IndexItemH)dynamic_cast<SpatialIndex::IData*>(result->clone());
	}
	*nResults = nResultCount - nStart;
}
