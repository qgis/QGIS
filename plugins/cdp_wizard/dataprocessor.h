/***************************************************************************
                          dataprocessor.h  -  description
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

#ifndef DATAPROCESSOR_H
#define DATAPROCESSOR_H

#include <vector>            

/**This class contains various functions that can be used to post process raw climate data
  * sets such as obtained from the IPCC. Please use only ANSI C++ here (no QT) so that this
  * this class will be portable to different platforms.
  *
  *Methods that take ArrayLengths as parameters should be passed as the number of elements,
  *not the array size. e.g. an array of 12 months is int[11] but should be passed as 12.
  *@author Tim Sutton
  */

class DataProcessor {
public: 
	DataProcessor();
	virtual ~DataProcessor();

  /** This method calculates the mean value over the quarter with the lowest values
  * (i.e. the three consecutive months with the minimum combined total). */
  virtual float meanOverLowestQ ( float *theClimateArray, int theArrayLength);
  /**Overloaded version that takes a vector */
  virtual float meanOverLowestQ(std::vector <float> theClimateVector);
  /** This method calculates the mean value over the quarter with the highest values
  * (i.e. the three consecutive months with the maximum combined total). */
  virtual float meanOverHighestQ ( float *theClimateArray, int theArrayLength);
  /**Overloaded version that takes a vector */
  virtual float meanOverHighestQ ( std::vector <float> theClimateVector);
  /** This method returns the month that starts the quarter with the highest
average values.*/
  virtual int firstMonthOfHighestQ (float *theClimateArray, int theArrayLength);
  /**Overloaded version that takes a vector */
  virtual int firstMonthOfHighestQ (std::vector <float> theClimateVector);
  /** This method returns the month that starts the quarter with the lowest
average values. */
  virtual int firstMonthOfLowestQ (float *theClimateArray, int theArrayLength);
  /**Overloaded version that takes a vector */
  virtual int firstMonthOfLowestQ (std::vector <float> theClimateVector);
  /** This method will return the mean over three months in
theClimateArray, starting at theCurrentBlock. */
  virtual float meanOverQuarter (float *theClimateArray, int theArrayLength, int theCurrentBlock);
  /**Overloaded version that takes a vector */
  virtual float meanOverQuarter (std::vector <float> theClimateVector, int theCurrentBlock);
  /** Given an array, this method will return the value of the smallest
element in the array. */
  virtual float lowestValue (float *theClimateArray, int theArrayLength);
  /**Overloaded version that takes a vector */
  virtual float lowestValue (std::vector <float> theClimateVector);
  /** Given an array, this function will return the value of the largest element
in the array. */
  virtual float highestValue (float *theClimateArray, int theArrayLength);
  /**Overloaded version that takes a vector */
  virtual float highestValue (std::vector <float> theClimateVector);
  /** Given two arrays (e.g. min temp and max temp) range will determine
  * the smallest and largest values that occur and return the difference.
  * The function is indescriminate as to whether the values are in the same
  * month or not.
  * @see greatestMonthlyRange
 */
  virtual float greatestTotalRange (float *theClimateArray1, int theArrayLength1, float *theClimateArray2, int theArrayLength2);
  /**Overloaded version that takes a vector */
  virtual float greatestTotalRange (std::vector <float> theClimateVector1, std::vector <float> theClimateVector2);
  /** Given two arrays (e.g. min temp and max temp) range will determine
  * the smallest and largest values that occur and return the difference.
  * The value of the max-min difference for a given month that is the
  * greatest is returned.
  * @see greatestTotalRange */
  virtual float greatestMonthlyRange (float *theClimateArray1, int theArrayLength1, float *theClimateArray2, int theArrayLength2);
  /**Overloaded version that takes a vector */
  virtual float greatestMonthlyRange (std::vector <float> theClimateVector, std::vector <float> theClimateVector2);
  /** This function will return the standard deviation of the climate array. */
  virtual float stddevOverYear (float *theClimateArray, int theArrayLength);
  /**Overloaded version that takes a vector */
  virtual float stddevOverYear (std::vector <float> theClimateVector);
  /** This function will return the sum of theClimateArray divided by the
  * number of elements in theClimateArray. */
  virtual float meanOverYear (float *theClimateArray, int theArrayLength);
  /**Overloaded version that takes a vector */
  virtual float meanOverYear (std::vector <float> theClimateVector);
  /** This function will return the value of the element in theClimateArray
  * that corresponds to theMonth. */
  virtual float valueGivenMonth (float *theClimateArray, int theMonth);
  /**Overloaded version that takes a vector */
  virtual float valueGivenMonth (std::vector <float> theClimateVector, int theMonth);
  /** This function will return an integer between 1 and 12 corresponding to
  * the month with the highest value. */
  virtual int monthWithHighestValue (float *theClimateArray, int theArrayLength);
  /**Overloaded version that takes a vector */
  virtual int monthWithHighestValue (std::vector <float> theClimateVector);
  /** This function will return an integer between 1 and 12 corresponding to
  * the month with the lowest value. */
  virtual int monthWithLowestValue(float *theClimateArray, int theArrayLength);
  /**Overloaded version that takes a vector */
  virtual int monthWithLowestValue(std::vector <float> theClimateVector);
  /** This function will return an integer in the range 0-12 representing
  * the number of months in theClimateArray where the value for that
  * month is above zero. Typically used to calculate how many months
  * in the year there are where the average temp is above freezing. */
  virtual int numberOfMonthsAboveZero (float *theClimateArray, int theArrayLength);
  /**Overloaded version that takes a vector */
  virtual int numberOfMonthsAboveZero (std::vector <float> theClimateVector);
  /** This value will return the mean value of months in theClimateArray where the corresponding months in theFrostArray have no frost free days. */
  virtual float meanValueOverFrostFreeMonths (float *theFrostArray, int theFrostArrayLength, float *theClimateArray, int theClimateArrayLength);
  /**Overloaded version that takes a vector */
  virtual float meanValueOverFrostFreeMonths (std::vector <float> theFrostVector, std::vector <float> theClimateVector);


 private:
  bool debugModeFlag;
  
};



#endif
