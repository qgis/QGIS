/***************************************************************************
                              qgscolortable.cpp
                             -------------------
    begin                : August 2004
    copyright            : (C) 2004 by Radim Blazek
    email                : blazek@itc.it
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <iostream>
#include <vector>
#include <algorithm>

#include "qgscolortable.h"

/* compare ramps */
bool compareRampSort ( const RAMP &a, const RAMP &b) 
{ 
    if ( a.min < b.min || a.max < b.max ) return true;

    return false; 
} 

bool compareRampSearch ( const RAMP &a, const RAMP &b)
{
    if( a.max < b.min ) return true;
    return false;
}


QgsColorTable::QgsColorTable ( int interp ) 
{
    #ifdef QGISDEBUG
    std::cerr << "QgsColorTable::QgsColorTable()" << std::endl;
    #endif

    mInterp = interp;
}

QgsColorTable::~QgsColorTable()
{
}

void QgsColorTable::add ( int index, unsigned char c1, unsigned char c2, unsigned char c3, unsigned char c4 )
{
    #ifdef QGISDEBUG
    std::cerr << "QgsColorTable::add() index = " << index << std::endl;
    #endif

    if ( mDiscrete.size() == 0 ) {
	mMin = index;
	mMax = index;
    } else { 
	if ( index < mMin ) mMin = index;
	if ( index > mMax ) mMax = index;
    }

    if ( mDiscrete.size() <= index ) {
       mDiscrete.resize ( index + 1 ); 
    }

    mDiscrete[index].c1 = c1;
    mDiscrete[index].c2 = c2;
    mDiscrete[index].c3 = c3;
    mDiscrete[index].c4 = c4;
}

void QgsColorTable::clear()
{
    #ifdef QGISDEBUG
    std::cerr << "QgsColorTable::clear() called " << std::endl;
    #endif
    mDiscrete.clear();
    mRamp.clear();
    mMax=0;
    mMin=0;
}


void QgsColorTable::add ( double min,  double max,
                  unsigned char min_c1, unsigned char min_c2, unsigned char min_c3, unsigned char min_c4,
		  unsigned char max_c1, unsigned char max_c2, unsigned char max_c3, unsigned char max_c4 )
{
    RAMP ramp;
    
    #ifdef QGISDEBUG
    std::cerr << "QgsColorTable::add() min = " << min << " max = " << max << std::endl;
    #endif

    if ( mRamp.size() == 0 ) {
	mMin = min;
	mMax = max;
    } else { 
	if ( min < mMin ) mMin = min;
	if ( max > mMax ) mMax = max;
    }

    ramp.min = min;
    ramp.max = max;
    ramp.min_c1 = min_c1;
    ramp.min_c2 = min_c2;
    ramp.min_c3 = min_c3;
    ramp.min_c4 = min_c4;
    ramp.max_c1 = max_c1;
    ramp.max_c2 = max_c2;
    ramp.max_c3 = max_c3;
    ramp.max_c4 = max_c4;

    mRamp.push_back ( ramp );
}

bool QgsColorTable::color ( double value, int *c1, int *c2, int *c3 )
{
    if ( mRamp.size() > 0 ) {
	std::vector<RAMP>::iterator it;

	RAMP ramp;
	ramp.min = ramp.max = value;
	
	it = std::lower_bound ( mRamp.begin(), mRamp.end(), ramp, compareRampSearch );

	if ( it != mRamp.end() ) { // Found
	    double k, d;
            d = it->max - it->min;
	    
	    if ( d <= 0 ) { 
		k = 0;
	    } else {
	        k = (value - it->min)/d;
	    }

	    *c1 = (int) (it->min_c1 + k * (it->max_c1 - it->min_c1));
	    *c2 = (int) (it->min_c2 + k * (it->max_c2 - it->min_c2));
	    *c3 = (int) (it->min_c3 + k * (it->max_c3 - it->min_c3));
	    
	    return true;
	}
    } else if ( mDiscrete.size() > 0 ) {
	int index = (int) value;
	if ( index < mDiscrete.size() ) {
	    *c1 = mDiscrete[index].c1;
	    *c2 = mDiscrete[index].c2;
	    *c3 = mDiscrete[index].c3;
	    return true;
	}
    }

    *c1 = 0; *c2 = 0; *c3 = 0;
    return false;
}

void QgsColorTable::sort ( void )
{
    std::sort ( mRamp.begin(), mRamp.end(), compareRampSort );
}

bool QgsColorTable::defined ( void )
{
    if ( mDiscrete.size() > 0 || mRamp.size() > 0 ) {
	return true;
    }
    return false;
}

int QgsColorTable::interpretation ( void )
{
    return mInterp;
}

double QgsColorTable::min ( void )
{
    return mMin;
}

double QgsColorTable::max ( void )
{
    return mMax;
}

void QgsColorTable::print ( void )
{
    std::cerr << "******** Color table ********" << std::endl;

    std::cerr << "Discrete table size = " << mDiscrete.size() << std::endl;
    for ( int i = 0; i < mDiscrete.size(); i++ ) {
        std::cerr << "  i = " << i << " c1 = " << (int) mDiscrete[i].c1 << " c2 = " << (int)mDiscrete[i].c2 
	          << " c3 = " << (int)mDiscrete[i].c3 << std::endl;
    }

    std::cerr << "Ramp table size = " << mRamp.size() << std::endl;
    for ( int i = 0; i < mRamp.size(); i++ ) {
        std::cerr << "  min = " << mRamp[i].min << " max = " << mRamp[i].max 
	          << " min_c1 = " << (int)mRamp[i].min_c1 << " min_c2 = " << (int)mRamp[i].min_c2 
	          << " min_c3 = " << (int)mRamp[i].min_c3 << " max_c1 = " << (int)mRamp[i].max_c1
		  << " max_c2 = " << (int)mRamp[i].max_c2 << " max_c3 = " << (int)mRamp[i].max_c3 
		  << std::endl;
    }
}

