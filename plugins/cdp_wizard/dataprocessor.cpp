/***************************************************************************
                          dataprocessor.cpp  -  description
                             -------------------
    begin                : Wed Jan 8 2003
    copyright            : (C) 2003 by Tim Sutton
    email                : t.sutton@reading.ac.uk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "dataprocessor.h"
#include <iostream>
#include <math.h>
using namespace std;

const int QUARTER = 3;
const float FREEZING_POINT=0;

DataProcessor::DataProcessor()
{

   debugModeFlag=false;
  
}
DataProcessor::~DataProcessor()
{}


/** This method calculates the mean value over the quarter with the lowest values
*    (i.e. the three consecutive months with the minimum combined total). */
float DataProcessor::meanOverLowestQ ( float *theClimateArray, int theArrayLength)
{
    if (debugModeFlag) cout << "DataProcessor::meanOverLowestQ ( float *theClimateArray) called" << endl;
    if (debugModeFlag) cout << "==========================================================" << endl;

    float myLowestMeanFloat;
    float myCurrentMeanFloat;
    int myInt;
    int mySegmentInt;
    bool myFirstTimeFlag;

    if (debugModeFlag) cout << "Array length : " << theArrayLength << endl;
    myLowestMeanFloat = 0;
    myFirstTimeFlag = true;

    //Loop from first quarter - i.e. months 1, 2 and 3 to
    //last quarter - i.e. months 12, 1 and 2.
    for (myInt=0; myInt < theArrayLength; myInt++)
    {
        myCurrentMeanFloat = 0;
        if (debugModeFlag) cout << "Processing element " <<    myInt << endl;
        //Check to see whether you are near end of array and it is necessary to start reading
        //from the beginning again.
        if (myInt <= (theArrayLength - (QUARTER)))
        {
            for (mySegmentInt=myInt ; mySegmentInt<=(myInt + QUARTER -1);mySegmentInt++)
            {
                myCurrentMeanFloat = myCurrentMeanFloat + theClimateArray[mySegmentInt];
                if (debugModeFlag) cout << "Processing element " <<    myInt << ", value is " << theClimateArray[mySegmentInt] << endl;
            }
            myCurrentMeanFloat = myCurrentMeanFloat / QUARTER;
        }
        else
        {
            //Near end of array so read last month(s)
            for (mySegmentInt = myInt; mySegmentInt < theArrayLength ; mySegmentInt++)
            {
                myCurrentMeanFloat = myCurrentMeanFloat + theClimateArray[mySegmentInt];
                if (debugModeFlag) cout << "Wrap Processing element (from end) " <<    myInt << ", value is " << theClimateArray[mySegmentInt] << endl;
            }
            //Read remainding months from beginning of array
            for (mySegmentInt = 0; mySegmentInt<=(QUARTER - (theArrayLength - myInt + 1));mySegmentInt++)
            {
                myCurrentMeanFloat = myCurrentMeanFloat + theClimateArray[mySegmentInt];
                if (debugModeFlag) cout << "Wrap Processing element (from start)" <<    myInt << ", value is " << theClimateArray[mySegmentInt] << endl;
            }
            myCurrentMeanFloat = myCurrentMeanFloat / QUARTER;
        }

        //Check whether value is lowest so far. If this is the first quarter always keep value.
        if (myFirstTimeFlag)
        {
            myLowestMeanFloat = myCurrentMeanFloat;
            myFirstTimeFlag = false;
        }
        else
        {
            //Test to see whether current value is the lowest so far.
            if (myLowestMeanFloat > myCurrentMeanFloat)
            {
                myLowestMeanFloat = myCurrentMeanFloat;
            }
        }

    if (debugModeFlag) cout << "Iteration " << myInt << " - current lowest mean over quarter is : " << myLowestMeanFloat << endl;
    }
    if (debugModeFlag) cout << "Completed - lowest mean over quarter is : " << myLowestMeanFloat << endl;
    if (debugModeFlag) cout << "----------------------------------------------------------------------------------------------" << endl;
    //Return lowest value over the quarter
    return myLowestMeanFloat;

}
/** This method calculates the mean value over the quarter with the highest values
* (i.e. the three consecutive months with the maximum combined total). */
float DataProcessor::meanOverHighestQ ( float *theClimateArray,int theArrayLength)
{
    if (debugModeFlag) cout << "DataProcessor::meanOverHighestQ ( float *theClimateArray, int theArrayLength) called" << endl;
    if (debugModeFlag) cout << "==========================================================" << endl;

    float myHighestMeanFloat;
    float myCurrentMeanFloat;
    int myInt;
    int mySegmentInt;
    bool myFirstTimeFlag;

    if (debugModeFlag) cout << "Array length : " << theArrayLength << endl;
    myHighestMeanFloat = 0;
    myFirstTimeFlag = true;

    //Loop from first quarter - i.e. months 1, 2 and 3 to
    //last quarter - i.e. months 12, 1 and 2.
    for (myInt=0; myInt<theArrayLength; myInt++)
    {
        myCurrentMeanFloat = 0;
        if (debugModeFlag) cout << "Processing element " <<    myInt << endl;
        //Check to see whether you are near end of array and it is necessary to start reading
        //from the beginning again.
        if (myInt <= (theArrayLength - (QUARTER)))
        {
            for (mySegmentInt=myInt ; mySegmentInt<=(myInt + QUARTER -1);mySegmentInt++)
            {
                myCurrentMeanFloat = myCurrentMeanFloat + theClimateArray[mySegmentInt];
                if (debugModeFlag) cout << "Processing element " <<    myInt << ", value is " << theClimateArray[mySegmentInt] << endl;
            }
            myCurrentMeanFloat = myCurrentMeanFloat / QUARTER;
        }
        else
        {
            //Near end of array so read last month(s)
            for (mySegmentInt = myInt; mySegmentInt < theArrayLength ; mySegmentInt++)
            {
                myCurrentMeanFloat = myCurrentMeanFloat + theClimateArray[mySegmentInt];
                if (debugModeFlag) cout << "Wrap Processing element (from end) " <<    myInt << ", value is " << theClimateArray[mySegmentInt] << endl;
            }
            //Read remainding months from beginning of array
            for (mySegmentInt = 0; mySegmentInt<=(QUARTER - (theArrayLength - myInt + 1));mySegmentInt++)
            {
                myCurrentMeanFloat = myCurrentMeanFloat + theClimateArray[mySegmentInt];
                if (debugModeFlag) cout << "Wrap Processing element (from start)" <<    myInt << ", value is " << theClimateArray[mySegmentInt] << endl;
            }
            myCurrentMeanFloat = myCurrentMeanFloat / QUARTER;
        }

        //Check whether value is highest so far. If this is the first quarter always keep value.
        if (myFirstTimeFlag)
        {
            myHighestMeanFloat = myCurrentMeanFloat;
            myFirstTimeFlag = false;
        }
        else
        {
            //Test to see whether current value is the Highest so far.
            if (myHighestMeanFloat < myCurrentMeanFloat)
            {
                myHighestMeanFloat = myCurrentMeanFloat;
            }
        }

    if (debugModeFlag) cout << "Iteration " << myInt << " - current highest mean over quarter is : " << myHighestMeanFloat << endl;
    }
    if (debugModeFlag) cout << "Completed - highest mean over quarter is : " << myHighestMeanFloat << endl;
    if (debugModeFlag) cout << "----------------------------------------------------------------------------------------------" << endl;
    //Return Highest value over the quarter
    return myHighestMeanFloat;
}
/** This method returns the month that starts the quarter with the lowest
average values. For example, if 12 months were :

1   2   3   4   5   6   7   8   9   10  11  12
-----------------------------------------------
21 18 19 15 12  5   6   8   12  15  16  20

Then the return from this method would be 6 because
5,6 and 8 combined form the lowest quarter. */
int DataProcessor::firstMonthOfLowestQ (float *theClimateArray, int theArrayLength)
{
    if (debugModeFlag) cout << "DataProcessor::firstMonthOfLowestQ ( float *theClimateArray, int theArrayLength) called" << endl;
    if (debugModeFlag) cout << "==========================================================" << endl;

    float myLowestFloat=0;
    float myCurrentFloat=0;
    int myInt=0;
    int mySegmentInt=0;
    bool myFirstTimeFlag=true;
    int myLowestMonthInt=0;

  try
  {
    //Loop from first quarter - i.e. months 1, 2 and 3 to
    //last quarter - i.e. months 12, 1 and 2.
    for (myInt = 0; myInt < theArrayLength; myInt++)
    {
        myCurrentFloat = 0;
        //Check to see whether you are near end of array and it is necessary to start reading
        //from the beginning again.

        if (myInt <= (theArrayLength - (QUARTER)))
        {
            for (mySegmentInt=myInt ; mySegmentInt<=(myInt + QUARTER -1);mySegmentInt++)
            {
                myCurrentFloat += theClimateArray[mySegmentInt];
                if (debugModeFlag) cout << "Processing element " <<    myInt << ", value is " << theClimateArray[mySegmentInt] << endl;
            }
        }
        else
        {
            //Near end of array so read last month(s)
            for (mySegmentInt = myInt; mySegmentInt < theArrayLength; mySegmentInt++)
            {
                myCurrentFloat += theClimateArray[mySegmentInt];
                if (debugModeFlag) cout << "Wrap Processing element (from end) " <<    myInt << ", value is " << theClimateArray[mySegmentInt] << endl;
            }
            //Read remainding months from beginning of array
            //for (mySegmentInt = 0; mySegmentInt < (QUARTER - (theArrayLength - myInt) - 1); mySegmentInt++)
            for (mySegmentInt = 0; mySegmentInt <=(QUARTER - (theArrayLength - myInt + 1));mySegmentInt++)
            {
                myCurrentFloat += theClimateArray[mySegmentInt];
                if (debugModeFlag) cout << "Wrap Processing element (from start)" <<    myInt << ", value is " << theClimateArray[mySegmentInt] << endl;
            }
        }

        //Check whether value is highest so far.  If this is the first quarter always keep value.
        if (myFirstTimeFlag == true)
        {
            myLowestMonthInt = myInt;
            myLowestFloat = myCurrentFloat;
            myFirstTimeFlag = false;
        }
        else
        {
            //Test to see whether current value is the highest so far. If so store month number
            if (myCurrentFloat < myLowestFloat)
            {
                myLowestMonthInt = myInt;
                myLowestFloat = myCurrentFloat;
            }
        }
    }
  }
  catch (...)
  {
          if (debugModeFlag) cout << "A fatal error occured in the firstMonthOfLowestQ method " << endl;
          myLowestMonthInt = -9999;
  }


    //Return lowest value over the quarter (add 1 to get it into base 1 instead of base0)
    return myLowestMonthInt+1;


}
/** This method returns the month that starts the quarter with the highest
average values. For example, if 12 months were :

1   2   3   4   5   6   7   8   9   10  11  12
-----------------------------------------------
21  18  19  15  12  5   6   8   12  15  16  20

Then the return from this method would be 12 because
12,1 and 2 combined form the highest quarter. */
int DataProcessor::firstMonthOfHighestQ (float *theClimateArray, int theArrayLength)
{
    if (debugModeFlag) cout << "DataProcessor::firstMonthOfHighestQ ( float *theClimateArray, int theArrayLength) called" << endl;
    if (debugModeFlag) cout << "==========================================================" << endl;

    float myHighestFloat=0;
    float myCurrentFloat=0;
    int myInt=0;
    int mySegmentInt=0;
    bool myFirstTimeFlag=true;
    int myHighestMonthInt=0;

  try
  {
    //Loop from first quarter - i.e. months 1, 2 and 3 to
    //last quarter - i.e. months 12, 1 and 2.
    for (myInt = 0; myInt < theArrayLength; myInt++)
    {
        myCurrentFloat = 0;
        //Check to see whether you are near end of array and it is necessary to start reading
        //from the beginning again.

        if (myInt <= (theArrayLength - (QUARTER)))
        {
            for (mySegmentInt=myInt ; mySegmentInt<=(myInt + QUARTER -1);mySegmentInt++)
            {
                myCurrentFloat += theClimateArray[mySegmentInt];
                if (debugModeFlag) cout << "Processing element " <<    myInt << ", value is " << theClimateArray[mySegmentInt] << endl;
            }
        }
        else
        {
            //Near end of array so read last month(s)
            for (mySegmentInt = myInt; mySegmentInt < theArrayLength; mySegmentInt++)
            {
                myCurrentFloat += theClimateArray[mySegmentInt];
                if (debugModeFlag) cout << "Wrap Processing element (from end) " <<    myInt << ", value is " << theClimateArray[mySegmentInt] << endl;
            }
            //Read remainding months from beginning of array
            //for (mySegmentInt = 0; mySegmentInt < (QUARTER - (theArrayLength - myInt) - 1); mySegmentInt++)
            for (mySegmentInt = 0; mySegmentInt <=(QUARTER - (theArrayLength - myInt + 1));mySegmentInt++)
            {
                myCurrentFloat += theClimateArray[mySegmentInt];
                if (debugModeFlag) cout << "Wrap Processing element (from start)" <<    myInt << ", value is " << theClimateArray[mySegmentInt] << endl;
            }
        }

        //Check whether value is highest so far.  If this is the first quarter always keep value.
        if (myFirstTimeFlag == true)
        {
            myHighestMonthInt = myInt;
            myHighestFloat = myCurrentFloat;
            myFirstTimeFlag = false;
        }
        else
        {
            //Test to see whether current value is the highest so far. If so store month number
            if (myCurrentFloat > myHighestFloat)
            {
                myHighestMonthInt = myInt;
                myHighestFloat = myCurrentFloat;
            }
        }
    }
  }
  catch (...)
  {
          if (debugModeFlag) cout << "A fatal error occured in the firstMonthOfHighestQ method " << endl;
          myHighestMonthInt = -9999;
  }


    //Return highest value over the quarter (add 1 to get it into base 1 instead of base0)
    return myHighestMonthInt+1;
}
/** This method will return the mean over three months in
theClimateArray, starting at theStartMonth. */
float DataProcessor::meanOverQuarter (float *theClimateArray, int theArrayLength, int theStartMonth)
{

    if (debugModeFlag) cout << "DataProcessor::meanOverQuarter (float *theClimateArray, int theArrayLength, int theStartMonth) called" << endl;
    if (debugModeFlag) cout << "==========================================================" << endl;


    int myInt=0;
    float myMeanFloat=0;
    int mySegmentInt=0;

    try
    {

      //Check to see whether months need to wrap
      if (theStartMonth + QUARTER < theArrayLength)
      {
        //No wrapping necessary
        for (myInt = theStartMonth-1; myInt < (theStartMonth-1 + (QUARTER)); myInt++)
        {
            myMeanFloat += theClimateArray[myInt];
            if (debugModeFlag) cout << "Added " << theClimateArray[myInt] << ", total is now : " << myMeanFloat << endl;
        }
      }
      else
      {
        //Wrapping necessary so read last month(s) as normal
        for (mySegmentInt=theStartMonth-1; mySegmentInt < theArrayLength; mySegmentInt++)
        {
            myMeanFloat += theClimateArray[mySegmentInt];
            if (debugModeFlag) cout << "Wrap Added from end " << theClimateArray[mySegmentInt] << ", total is now : " << myMeanFloat << endl;
        }
        //Read remainding months from beginning of array
        for (mySegmentInt = 0; mySegmentInt < (QUARTER - (theArrayLength - theStartMonth) - 1); mySegmentInt++)
        {
            myMeanFloat += theClimateArray[mySegmentInt];
            if (debugModeFlag) cout << "Wrap Added from start " << theClimateArray[mySegmentInt] << ", total is now : " << myMeanFloat << endl;
        }
      }
      myMeanFloat = (myMeanFloat / QUARTER);
    }
    catch (...)
    {
          if (debugModeFlag) cout << "A fatal error occured in the meanOverQuarter method " << endl;
          myMeanFloat = -9999;
    }
    return myMeanFloat;
}
/** Given an array, this method will return the value of the smallest
element in the array. */
float DataProcessor::lowestValue (float *theClimateArray, int theArrayLength)
{

    if (debugModeFlag) cout << "DataProcessor::lowestValue ( float *theClimateArray, int theArrayLength) called" << endl;
    if (debugModeFlag) cout << "==========================================================" << endl;


    float myLowestFloat=0;
    int myInt=0;
    bool myFirstTimeFlag=true;

    try
    {
      for (myInt=0; myInt < theArrayLength; myInt++)
      {
        //If this is the first run store value as lowest
        if (myFirstTimeFlag == true)
        {
            myLowestFloat = theClimateArray[myInt];
            if (debugModeFlag) cout << "Lowest value set to " << myLowestFloat << " on first iteration." << endl;
            myFirstTimeFlag = false;

        }
        //Test to see whether value is lowest so far
        else
        {
            if (myLowestFloat > theClimateArray[myInt])
            {
              myLowestFloat = theClimateArray[myInt];
            }
        }
      }
    }
    catch (...)
    {
          if (debugModeFlag) cout << "A fatal error occured in the lowestValue method " << endl;
          myLowestFloat = -9999;
    }
    //Return lowest value
    return myLowestFloat;
}
/** Given an array, this function will return the value of the largest element
in the array. */
float DataProcessor::highestValue (float *theClimateArray, int theArrayLength)
{

    if (debugModeFlag) cout << "DataProcessor::highestValue ( float *theClimateArray, int theArrayLength) called" << endl;
    if (debugModeFlag) cout << "==========================================================" << endl;


    float myHighestFloat=0;
    int myInt=0;
    bool myFirstTimeFlag=true;

    try
    {
      for (myInt=0; myInt < theArrayLength; myInt++)
      {
        //If this is the first run store value as lowest
        if (myFirstTimeFlag == true)
        {
            myHighestFloat = theClimateArray[myInt];
            if (debugModeFlag) cout << "Highest value set to " << myHighestFloat << " on first iteration." << endl;
            myFirstTimeFlag = false;

        }
        //Test to see whether value is lowest so far
        else
        {
            if (myHighestFloat < theClimateArray[myInt])
            {
              myHighestFloat = theClimateArray[myInt];
            }
        }
      }
    }
    catch (...)
    {
          if (debugModeFlag) cout << "A fatal error occured in the highest value method " << endl;
          myHighestFloat = -9999;
    }
    //Return highest value
    return myHighestFloat;

}

/** Given two arrays (e.g. min temp and max temp) range will determine
the smallest and largest values that occur and return the difference.
The function is indescriminate as to whether the values are in the same
month or not.
@see greatestMonthlyRange
*/
float DataProcessor::greatestTotalRange (float *theClimateArray1, int theArrayLength1, float *theClimateArray2, int theArrayLength2)
{

    if (debugModeFlag) cout << "DataProcessor::greatestTotalRange (float *theClimateArray1, int theArrayLength1, float *theClimateArray2, int theArrayLength2) called" << endl;
    if (debugModeFlag) cout << "==========================================================" << endl;

    //dont default to 0 as it may be lower (or higher) than any existing value in each array
    float myHighestFloat=theClimateArray1[0]; //default to a valid value in the set
    float myLowestFloat=theClimateArray1[0];  //default to a valid value in the set
    float myRangeFloat=-9999;
    int myInt=0;

    try
    {
        //process array1 first
        for (myInt = 0; myInt < theArrayLength1; myInt++)
        {
          if (myHighestFloat < theClimateArray1[myInt]) myHighestFloat = theClimateArray1[myInt];
          if (myLowestFloat > theClimateArray1[myInt]) myLowestFloat = theClimateArray1[myInt];
        }
        //now the second array
        for (myInt = 0; myInt < theArrayLength2; myInt++)
        {
          if (myHighestFloat < theClimateArray2[myInt]) myHighestFloat = theClimateArray2[myInt];
          if (myLowestFloat > theClimateArray2[myInt]) myLowestFloat = theClimateArray2[myInt];
        }
        myRangeFloat = myHighestFloat - myLowestFloat;
    }
    catch (...)
    {
          if (debugModeFlag) cout << "A fatal error occured in the greatestTotalRange method " << endl;
          myRangeFloat = -9999;

    }
    return myRangeFloat;
}
/** Given two arrays (e.g. min temp and max temp) range will determine
the smallest and largest values that occur and return the difference.
The value of the max-min difference for a given month that is the
greatest is returned.
@see greatestTotalRange */
float DataProcessor::greatestMonthlyRange (float *theClimateArray1, int theArrayLength1, float *theClimateArray2, int theArrayLength2)
{

  return 0;

}
/** This function will return the sum of theClimateArray divided by the
number of elements in theClimateArray. */
float DataProcessor::meanOverYear (float *theClimateArray,int theArrayLength)
{
    if (debugModeFlag) cout << "DataProcessor::meanOverYear ( float *theClimateArray, int theArrayLength) called" << endl;
    if (debugModeFlag) cout << "==============================================================" << endl;

    float myRunningTotFloat=0;
    float myMeanOverYearFloat;

    try
    {
        for (int myInt = 0; myInt <= theArrayLength-1; myInt++)
        {
            myRunningTotFloat += theClimateArray[myInt];
            if (debugModeFlag) cout << "Iteration " << myInt << ", running total is : " << myRunningTotFloat << endl;
        }
        myMeanOverYearFloat = myRunningTotFloat / theArrayLength;
    }
   catch (...)
   {
          if (debugModeFlag) cout << "A fatal error occured in the meanOverYear method " << endl;
          myMeanOverYearFloat = -9999;
    }
    if (debugModeFlag) cout << "Completed - mean over year is : " << myMeanOverYearFloat << endl;
    if (debugModeFlag) cout << "----------------------------------------------------------------------------------------------" << endl;
  return myMeanOverYearFloat;

}
/** This function will return the standard deviation of the climate array. */
float DataProcessor::stddevOverYear (float *theClimateArray, int theArrayLength)
{

    if (debugModeFlag) cout << "DataProcessor::stddevOverYear ( float *theClimateArray, int theArrayLength) called" << endl;
    if (debugModeFlag) cout << "==============================================================" << endl;
    int myInt=0;
    float myRunningTotFloat=0;
    float myMeanOverYearFloat=0;
    float mySumSqdDevFloat=0;
    float myStddevOverYearFloat=0;
    try
    {
        for (myInt = 0; myInt <= theArrayLength-1; myInt++)
        {
            myRunningTotFloat += theClimateArray[myInt];
            if (debugModeFlag) cout << "Iteration " << myInt << ", running total is : " << myRunningTotFloat << endl;
        }
        myMeanOverYearFloat = myRunningTotFloat / theArrayLength;

        //Calculate the sum of the squared deviations
        for (myInt = 0; myInt <  theArrayLength; myInt++)
        {
          mySumSqdDevFloat += static_cast<float>(pow((theClimateArray[myInt] - myMeanOverYearFloat),2));
        }

        //divide result by sample size - 1 and get square root to get stdev
        myStddevOverYearFloat = static_cast<float>(sqrt(mySumSqdDevFloat / (theArrayLength - 1)));
    }
   catch (...)
   {
          if (debugModeFlag) cout << "A fatal error occured in the stddevOverYear method " << endl;
          myStddevOverYearFloat = -9999;
    }
    if (debugModeFlag) cout << "Completed - stddev over year is : " << myMeanOverYearFloat << endl;
    if (debugModeFlag) cout << "----------------------------------------------------------------------------------------------" << endl;

  return myStddevOverYearFloat;
}
/** This function will return an integer between 1 and 12 corresponding to
the month with the lowest value. */
int DataProcessor::monthWithLowestValue(float *theClimateArray, int theArrayLength)
{
    int myLowestInt=0;     //assume first month is lowest until proven otherwise
    try
    {
       for (int myInt = 0; myInt < theArrayLength; myInt++)
       {
          //Test to see whether current value is the lowest so far.
          if (theClimateArray[myInt] < theClimateArray[myLowestInt]) myLowestInt = myInt ;
        }
      }
      catch (...)
      {
          if (debugModeFlag) cout << "A fatal error occured in the monthWithLowestValue method " << endl;
          myLowestInt = 0;
      }
    if (debugModeFlag) cout << "Completed - month with lowest value is : " << myLowestInt << endl;
    if (debugModeFlag) cout << "----------------------------------------------------------------------------------------------" << endl;
  return myLowestInt+1;


}
/** This function will return an integer between 1 and 12 corresponding to
the month with the highest value. */
int DataProcessor::monthWithHighestValue (float *theClimateArray, int theArrayLength)
{

    int myHighestInt=0;      //assume first month is highest until proven otherwise
    try
    {
       for (int myInt = 0; myInt < theArrayLength; myInt++)
       {
          //Test to see whether current value is the lowest so far.
          if (theClimateArray[myInt] > theClimateArray[myHighestInt]) myHighestInt = myInt ;
        }
      }
      catch (...)
      {
          if (debugModeFlag) cout << "A fatal error occured in the monthWithHighestValue method " << endl;
          myHighestInt = 0;
      }
    if (debugModeFlag) cout << "Completed - month with highest value is : " << myHighestInt << endl;
    if (debugModeFlag) cout << "----------------------------------------------------------------------------------------------" << endl;
  return myHighestInt+1;

}
/** This function will return the value of the element in theClimateArray
that corresponds to theMonth. */
float DataProcessor::valueGivenMonth (float *theClimateArray, int theMonth)
{
  float myValueFloat=0;
  try
  {
    if (theMonth < 1 || theMonth > 12) throw "Month out of bounds";
    myValueFloat = theClimateArray[theMonth-1];
  }
  catch (...)
  {
    if (debugModeFlag) cout << "A fatal error occured in the valueGivenMonth function" << endl;
    myValueFloat = -9999;
  }
  return myValueFloat;
}
/** This value will return the mean value (normally temperature or precipitation) of months in theClimateArray where the corresponding months in
theFrostArray have frost free days. The frost array should contain the number of frost days
per month, so a value of 0 in the frost array means there were no frost days that month. If there
are no frost free months, -9999 will be returned.*/
float DataProcessor::meanValueOverFrostFreeMonths (float *theFrostArray, int theFrostArrayLength, float *theClimateArray, int theClimateArrayLength)
{
    if (debugModeFlag) cout << "DataProcessor::meanValueOverFrostFreeMonths (float *theFrostArray, int theFrostArrayLength, float *theClimateArray, int theClimateArrayLength)" << endl;
    if (debugModeFlag) cout << "==============================================================" << endl;

    int myInt =0;
    float myRunningTotalFloat =0;
    float myMonthCountInt =0;
    float myMeanFloat=-9999;

    try
    {
      //Check that the frost and climate arrays are the same size
      if (theFrostArrayLength != theClimateArrayLength) throw ("Array sizes must be equal!");
      for (myInt = 0; myInt < theFrostArrayLength; myInt++)
      {
        //iterate through frost array looking for frost free months
        if (theFrostArray[myInt] == 0)
        {
            //If month is frost free add value to myRunningTotalDbl and add month to monthcounter
            myRunningTotalFloat += theClimateArray[myInt];
            myMonthCountInt += 1;
        }
      }

      //return average value
      if (myMonthCountInt > 0)
      {
         myMeanFloat = myRunningTotalFloat / myMonthCountInt;
      }
    }
    catch (...)
    {
      if (debugModeFlag) cout << "A fatal error occured in the meanValueOverFrostFreeMonths function" << endl;
      myMeanFloat = -9999;
    }

  return myMeanFloat;
}

/** This function will return an integer in the range 0-12 representing
the number of months in theClimateArray where the value for that
month is above zero. Typically used to calculate how many months
in the year there are where the average temp is above freezing. */
int DataProcessor::numberOfMonthsAboveZero (float *theClimateArray, int theArrayLength)
{

    int myInt=0;
    int myMonthCountInt=0;

    try
    {
      for (myInt=0; myInt < theArrayLength; myInt++)
      {
        if (theClimateArray[myInt] == -9999) return -9999;
        if (theClimateArray[myInt] > FREEZING_POINT) myMonthCountInt += 1;
      }
    }
    catch (...)
    {
      if (debugModeFlag) cout << "A fatal error occured in the numberOfMonthsAboveZero function" << endl;
      myMonthCountInt = -9999;
    }
    return myMonthCountInt;
}

/////////////////////////////////////////////////////
// Overloaded versions of above methods follow     //
/////////////////////////////////////////////////////


/**Overloaded version that takes a vector */
float DataProcessor::meanOverLowestQ(std::vector <float> theClimateVector)
{
    if (debugModeFlag) cout << "DataProcessor::meanOverLowestQ(std::vector <float> theClimateVector) called" << endl;
    if (debugModeFlag) cout << "==========================================================" << endl;
       //ideally this first part should be in a separate function but the
       //array created will lose scope when the function returns :-(
       
       int myArrayLengthInt = theClimateVector.size();
       float myClimateArray[myArrayLengthInt];
       //iterate through the climate vector, adding each element to the array
       for (int i=0;i < myArrayLengthInt; i++)
       {
             myClimateArray[i] = theClimateVector[i];
       }
       //now call the function we overloaded with the appropriate parameters
        float myResult = meanOverLowestQ( myClimateArray,myArrayLengthInt);
       return myResult;
}
/**Overloaded version that takes a vector */
float DataProcessor::meanOverHighestQ ( std::vector <float> theClimateVector)
{
    if (debugModeFlag) cout << "DataProcessor::meanOverHighestQ(std::vector <float> theClimateVector) called" << endl;
    if (debugModeFlag) cout << "==========================================================" << endl;
       //ideally this first part should be in a separate function but the
       //array created will lose scope when the function returns :-(
       int myArrayLengthInt;
       myArrayLengthInt = theClimateVector.size();
       float myClimateArray[myArrayLengthInt];
       //iterate through the climate vector, adding each element to the array
       for (int i=0;i < myArrayLengthInt; i++)
       {
             myClimateArray[i] = theClimateVector[i];
       }
       //now call the function we overloaded with the appropriate parameters
        float myResult = meanOverHighestQ( myClimateArray,myArrayLengthInt);
       return myResult;
}

/**Overloaded version that takes a vector */
int DataProcessor::firstMonthOfLowestQ (std::vector <float> theClimateVector)
{
    if (debugModeFlag) cout << "DataProcessor::firstMonthOfLowestQ(std::vector <float> theClimateVector) called" << endl;
    if (debugModeFlag) cout << "==========================================================" << endl;
       //ideally this first part should be in a separate function but the
       //array created will lose scope when the function returns :-(
       int myArrayLengthInt;
       myArrayLengthInt = theClimateVector.size();
       float myClimateArray[myArrayLengthInt];
       //iterate through the climate vector, adding each element to the array
       for (int i=0;i < myArrayLengthInt; i++)
       {
             myClimateArray[i] = theClimateVector[i];
       }
       //now call the function we overloaded with the appropriate parameters
       int myResult = firstMonthOfLowestQ( myClimateArray,myArrayLengthInt);
       return myResult;

}


/**Overloaded version that takes a vector */
int DataProcessor::firstMonthOfHighestQ (std::vector <float> theClimateVector)
{
    if (debugModeFlag) cout << "DataProcessor::firstMonthOfHighestQ(std::vector <float> theClimateVector) called" << endl;
    if (debugModeFlag) cout << "==========================================================" << endl;
       //ideally this first part should be in a separate function but the
       //array created will lose scope when the function returns :-(
       int myArrayLengthInt;
       myArrayLengthInt = theClimateVector.size();
       float myClimateArray[myArrayLengthInt];
       //iterate through the climate vector, adding each element to the array
       for (int i=0;i < myArrayLengthInt; i++)
       {
             myClimateArray[i] = theClimateVector[i];
       }
       //now call the function we overloaded with the appropriate parameters
       int myResult = firstMonthOfHighestQ( myClimateArray,myArrayLengthInt);
       return myResult;

}


/**Overloaded version that takes a vector */
float DataProcessor::meanOverQuarter (std::vector <float> theClimateVector, int theStartMonth)
{

    if (debugModeFlag) cout << "DataProcessor::meanOverQuarter(std::vector <float> theClimateVector) called" << endl;
    if (debugModeFlag) cout << "==========================================================" << endl;
       //ideally this first part should be in a separate function but the
       //array created will lose scope when the function returns :-(
       int myArrayLengthInt;
       myArrayLengthInt = theClimateVector.size();
       float myClimateArray[myArrayLengthInt];
       //iterate through the climate vector, adding each element to the array
       for (int i=0;i < myArrayLengthInt; i++)
       {
             myClimateArray[i] = theClimateVector[i];
       }
       //now call the function we overloaded with the appropriate parameters
        float myResult = meanOverQuarter( myClimateArray, myArrayLengthInt, myArrayLengthInt);
       return myResult;

}

/**Overloaded version that takes a vector */
float DataProcessor::lowestValue (std::vector <float> theClimateVector)
{

    if (debugModeFlag) cout << "DataProcessor::lowestValue(std::vector <float> theClimateVector) called" << endl;
    if (debugModeFlag) cout << "==========================================================" << endl;
       //ideally this first part should be in a separate function but the
       //array created will lose scope when the function returns :-(
       int myArrayLengthInt;
       myArrayLengthInt = theClimateVector.size();
       float myClimateArray[myArrayLengthInt];
       //iterate through the climate vector, adding each element to the array
       for (int i=0;i < myArrayLengthInt; i++)
       {
             myClimateArray[i] = theClimateVector[i];
       }
       //now call the function we overloaded with the appropriate parameters
        float myResult = meanOverHighestQ( myClimateArray, myArrayLengthInt);
       return myResult;


}


/**Overloaded version that takes a vector */
float DataProcessor::highestValue (std::vector <float> theClimateVector)
{
    if (debugModeFlag) cout << "DataProcessor::highestValue(std::vector <float> theClimateVector) called" << endl;
    if (debugModeFlag) cout << "==========================================================" << endl;
       //ideally this first part should be in a separate function but the
       //array created will lose scope when the function returns :-(
       int myArrayLengthInt;
       myArrayLengthInt = theClimateVector.size();
       float myClimateArray[myArrayLengthInt];
       //iterate through the climate vector, adding each element to the array
       for (int i=0;i < myArrayLengthInt; i++)
       {
             myClimateArray[i] = theClimateVector[i];
       }
       //now call the function we overloaded with the appropriate parameters
        float myResult = highestValue( myClimateArray, myArrayLengthInt );
       return myResult;

}

/**Overloaded version that takes a vector */
float DataProcessor::greatestTotalRange (std::vector <float> theClimateVector, std::vector <float> theClimateVector2)
{

    if (debugModeFlag) cout << "DataProcessor::greatestTotalRange (std::vector <float> theClimateVector, std::vector <float> theClimateVector2) called" << endl;
    if (debugModeFlag) cout << "==========================================================" << endl;
       //ideally this first part should be in a separate function but the
       //array created will lose scope when the function returns :-(
       int myArrayLengthInt, myArrayLengthInt2;
       myArrayLengthInt = theClimateVector.size();
       myArrayLengthInt2 = theClimateVector2.size();
       if (myArrayLengthInt !=  myArrayLengthInt2) throw ("DataProcessor::greatestTotalRange error - Incompatible array sizes");
       float myClimateArray[myArrayLengthInt];
       float myClimateArray2[myArrayLengthInt2];
       //iterate through the climate vector, adding each element to the array
       for (int i=0;i < myArrayLengthInt; i++)
       {
             myClimateArray[i] = theClimateVector[i];
       }
              for (int i=0;i < myArrayLengthInt; i++)
       {
             myClimateArray2[i] = theClimateVector2[i];
       }
       //now call the function we overloaded with the appropriate parameters
        float myResult = greatestTotalRange( myClimateArray,myArrayLengthInt, myClimateArray2,myArrayLengthInt2);
       return myResult;

}


float DataProcessor::greatestMonthlyRange (std::vector <float> theClimateVector, std::vector <float> theClimateVector2)
{

    if (debugModeFlag) cout << "DataProcessor::greatestMonthlyRange (std::vector <float> theClimateVector, std::vector <float> theClimateVector2) called" << endl;
    if (debugModeFlag) cout << "==========================================================" << endl;
       //ideally this first part should be in a separate function but the
       //array created will lose scope when the function returns :-(
       int myArrayLengthInt, myArrayLengthInt2;
       myArrayLengthInt = theClimateVector.size();
       myArrayLengthInt2 = theClimateVector2.size();
       if (myArrayLengthInt !=  myArrayLengthInt2) throw ("DataProcessor::greatestTotalRange error - Incompatible array sizes");
       float myClimateArray[myArrayLengthInt];
       float myClimateArray2[myArrayLengthInt2];
       //iterate through the climate vector, adding each element to the array
       for (int i=0;i < myArrayLengthInt; i++)
       {
             myClimateArray[i] = theClimateVector[i];
       }
              for (int i=0;i < myArrayLengthInt; i++)
       {
             myClimateArray2[i] = theClimateVector2[i];
       }
       //now call the function we overloaded with the appropriate parameters
        float myResult = greatestMonthlyRange( myClimateArray,myArrayLengthInt, myClimateArray2,myArrayLengthInt2);
       return myResult;
}



/**Overloaded version that takes a vector */
float DataProcessor::meanOverYear (std::vector <float> theClimateVector)
{

    if (debugModeFlag) cout << "DataProcessor::meanOverYear(std::vector <float> theClimateVector) called" << endl;
    if (debugModeFlag) cout << "==========================================================" << endl;
       //ideally this first part should be in a separate function but the
       //array created will lose scope when the function returns :-(
       int myArrayLengthInt;
       myArrayLengthInt = theClimateVector.size();
       float myClimateArray[myArrayLengthInt];
       //iterate through the climate vector, adding each element to the array
       for (int i=0;i < myArrayLengthInt; i++)
       {
             myClimateArray[i] = theClimateVector[i];
       }
       //now call the function we overloaded with the appropriate parameters
        float myResult = meanOverYear( myClimateArray,myArrayLengthInt);
       return myResult;

}


/**Overloaded version that takes a vector */
float DataProcessor::stddevOverYear (std::vector <float> theClimateVector)
{

    if (debugModeFlag) cout << "DataProcessor::stddevOverYear(std::vector <float> theClimateVector) called" << endl;
    if (debugModeFlag) cout << "==========================================================" << endl;
       //ideally this first part should be in a separate function but the
       //array created will lose scope when the function returns :-(
       int myArrayLengthInt;
       myArrayLengthInt = theClimateVector.size();
       float myClimateArray[myArrayLengthInt];
       //iterate through the climate vector, adding each element to the array
       for (int i=0;i < myArrayLengthInt; i++)
       {
             myClimateArray[i] = theClimateVector[i];
       }
       //now call the function we overloaded with the appropriate parameters
        float myResult = stddevOverYear( myClimateArray, myArrayLengthInt);
       return myResult;

}


/**Overloaded version that takes a vector */
int DataProcessor::monthWithLowestValue(std::vector <float> theClimateVector)
{

    if (debugModeFlag) cout << "DataProcessor::monthWithLowestValue(std::vector <float> theClimateVector) called" << endl;
    if (debugModeFlag) cout << "==========================================================" << endl;
       //ideally this first part should be in a separate function but the
       //array created will lose scope when the function returns :-(
       int myArrayLengthInt;
       myArrayLengthInt = theClimateVector.size();
       float myClimateArray[myArrayLengthInt];
       //iterate through the climate vector, adding each element to the array
       for (int i=0;i < myArrayLengthInt; i++)
       {
             myClimateArray[i] = theClimateVector[i];
       }
       //now call the function we overloaded with the appropriate parameters
       int myResult = monthWithLowestValue( myClimateArray,myArrayLengthInt);
       return myResult;

}


/**Overloaded version that takes a vector */
int DataProcessor::monthWithHighestValue (std::vector <float> theClimateVector)
{
    if (debugModeFlag) cout << "DataProcessor::monthWithHighestValue(std::vector <float> theClimateVector) called" << endl;
    if (debugModeFlag) cout << "==========================================================" << endl;
       //ideally this first part should be in a separate function but the
       //array created will lose scope when the function returns :-(
       int myArrayLengthInt;
       myArrayLengthInt = theClimateVector.size();
       float myClimateArray[myArrayLengthInt];
       //iterate through the climate vector, adding each element to the array
       for (int i=0;i < myArrayLengthInt; i++)
       {
             myClimateArray[i] = theClimateVector[i];
       }
       //now call the function we overloaded with the appropriate parameters
       int myResult = monthWithHighestValue( myClimateArray,myArrayLengthInt);
       return myResult;

}


/**Overloaded version that takes a vector */
float DataProcessor::valueGivenMonth (std::vector <float> theClimateVector, int theMonth)
{
    if (debugModeFlag) cout << "DataProcessor::valueGivenMonth (std::vector <float> theClimateVector, int theMonth) called" << endl;
    if (debugModeFlag) cout << "==========================================================" << endl;
       //ideally this first part should be in a separate function but the
       //array created will lose scope when the function returns :-(
       int myArrayLengthInt;
       myArrayLengthInt = theClimateVector.size();
       float myClimateArray[myArrayLengthInt];
       //iterate through the climate vector, adding each element to the array
       for (int i=0;i < myArrayLengthInt; i++)
       {
             myClimateArray[i] = theClimateVector[i];
       }
       //now call the function we overloaded with the appropriate parameters
        float myResult = valueGivenMonth( myClimateArray,theMonth);
       return myResult;

}

/**Overloaded version that takes a vector */
float DataProcessor::meanValueOverFrostFreeMonths (std::vector <float> theFrostVector, std::vector <float> theClimateVector)
{
    if (debugModeFlag) cout << "DataProcessor::meanValueOverFrostFreeMonths (std::vector <float> theFrostVector, std::vector <float> theClimateVector)" << endl;
    if (debugModeFlag) cout << "==========================================================" << endl;
       //ideally this first part should be in a separate function but the
       //array created will lose scope when the function returns :-(
       int myArrayLengthInt, myArrayLengthInt2;
       myArrayLengthInt = theFrostVector.size();
       myArrayLengthInt2 = theClimateVector.size();
       if (myArrayLengthInt !=  myArrayLengthInt2) throw ("DataProcessor::meanValueOverFrostFreeMonths error - Incompatible array sizes");
       float myClimateArray[myArrayLengthInt];
       float myClimateArray2[myArrayLengthInt2];
       //iterate through the climate vector, adding each element to the array
       for (int i=0;i < myArrayLengthInt; i++)
       {
             myClimateArray[i] = theFrostVector[i];
       }
              for (int i=0;i < myArrayLengthInt; i++)
       {
             myClimateArray2[i] = theClimateVector[i];
       }
       //now call the function we overloaded with the appropriate parameters
        float myResult = greatestMonthlyRange( myClimateArray,myArrayLengthInt, myClimateArray2,myArrayLengthInt2);
       return myResult;

}


/**Overloaded version that takes a vector */
int DataProcessor::numberOfMonthsAboveZero (std::vector <float> theClimateVector)
{
    if (debugModeFlag) cout << "DataProcessor::numberOfMonthsAboveZero(std::vector <float> theClimateVector) called" << endl;
    if (debugModeFlag) cout << "==========================================================" << endl;
       //ideally this first part should be in a separate function but the
       //array created will lose scope when the function returns :-(
       int myArrayLengthInt;
       myArrayLengthInt = theClimateVector.size();
       float myClimateArray[myArrayLengthInt];
       //iterate through the climate vector, adding each element to the array
       for (int i=0;i < myArrayLengthInt; i++)
       {
             myClimateArray[i] = theClimateVector[i];
       }
       //now call the function we overloaded with the appropriate parameters
       int myResult = numberOfMonthsAboveZero( myClimateArray,myArrayLengthInt);
       return myResult;

}
