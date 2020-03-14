/***************************************************************************
                         qgslocatorfilter.cpp
                         --------------------
    begin                : May 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QThread>

#include "qgslocatorfilter.h"
#include "qgsstringutils.h"
#include "qgsfeedback.h"
#include "qgsmessagelog.h"


#define FUZZY_SCORE_WORD_MATCH 5
#define FUZZY_SCORE_NEW_MATCH 3
#define FUZZY_SCORE_CONSECUTIVE_MATCH 4


QgsLocatorFilter::QgsLocatorFilter( QObject *parent )
  : QObject( parent )
{
}

QgsLocatorFilter::Flags QgsLocatorFilter::flags() const
{
  return nullptr;
}

void QgsLocatorFilter::triggerResultFromAction( const QgsLocatorResult &result, const int actionId )
{
  Q_UNUSED( result )
  Q_UNUSED( actionId )
}

bool QgsLocatorFilter::stringMatches( const QString &candidate, const QString &search )
{
  return !search.isEmpty() && candidate.contains( search, Qt::CaseInsensitive );
}

double QgsLocatorFilter::fuzzyScore( const QString &candidate, const QString &search )
{
    QString candidateNormalized = candidate.simplified().normalized( QString:: NormalizationForm_C ).toLower();
    QString searchNormalized = search.simplified().normalized( QString:: NormalizationForm_C ).toLower();

    int candidateLength = candidateNormalized.length();
    int searchLength = searchNormalized.length();
    int score = 0;

    // if the candidate and the search term are empty, no other option than 0 score
    if ( candidateLength == 0 || searchLength == 0 )
      return score;

    int candidateIdx = 0;
    int searchIdx = 0;
    int maxScore = 0;

    bool isPreviousIndexMatching = false;
    bool isWordOpen = true;

    // loop throught each candidate char and calculate the potential max score
    while ( candidateIdx < candidateLength )
    {
      QChar candidateChar = candidateNormalized[ candidateIdx++ ];

      // the first char is always the default score
      if ( candidateIdx == 1)
        maxScore += FUZZY_SCORE_NEW_MATCH;
      // every space character or end of string is a opportunity for a new word
      else if ( candidateChar.isSpace() || candidateIdx == candidateLength )
        maxScore += FUZZY_SCORE_WORD_MATCH;
      // potentially we can match every other character
      else
        maxScore += FUZZY_SCORE_CONSECUTIVE_MATCH;

      // we looped through all the characters
      if ( searchIdx >= searchLength )
        continue;

      QChar searchChar = searchNormalized[ searchIdx ];

      // match!
      if ( candidateChar == searchChar )
      {
        searchIdx++;

        // if we have just successfully finished a word, give higher score
        if ( candidateChar.isSpace() || searchIdx == searchLength )
        {
          if ( isWordOpen )
            score += FUZZY_SCORE_WORD_MATCH;
          else if ( isPreviousIndexMatching )
            score += FUZZY_SCORE_CONSECUTIVE_MATCH;
          else
            score += FUZZY_SCORE_NEW_MATCH;

          isWordOpen = true;
        }
        // if we have consecutive characters matching, give higher score
        else if ( isPreviousIndexMatching )
        {
            score += FUZZY_SCORE_CONSECUTIVE_MATCH;
        }
        // normal score for new independent character that matches
        else
        {
            score += FUZZY_SCORE_NEW_MATCH;
        }

        isPreviousIndexMatching = true;
      }
      // if the current character does NOT match, we are sure we cannot build a word for now
      else
      {
          isPreviousIndexMatching = false;
          isWordOpen = false;
      }
    }

    // we didn't loop through all the search chars, it means, that they are not present in the current candidate
    if ( searchIdx != searchLength )
      score = 0;

    return static_cast<float>(std::max( score, 0 )) / std::max(maxScore, 1);
}

bool QgsLocatorFilter::enabled() const
{
  return mEnabled;
}

void QgsLocatorFilter::setEnabled( bool enabled )
{
  mEnabled = enabled;
}

bool QgsLocatorFilter::hasConfigWidget() const
{
  return false;
}

void QgsLocatorFilter::openConfigWidget( QWidget *parent )
{
  Q_UNUSED( parent )
}

bool QgsLocatorFilter::useWithoutPrefix() const
{
  return mUseWithoutPrefix;
}

void QgsLocatorFilter::setUseWithoutPrefix( bool useWithoutPrefix )
{
  mUseWithoutPrefix = useWithoutPrefix;
}

QString QgsLocatorFilter::activePrefix() const
{
  // do not change this to isEmpty!
  // if any issue with an in-built locator filter
  // do not forget to add it in QgsLocator::CORE_FILTERS
  if ( mActivePrefifx.isNull() )
    return prefix();
  else
    return mActivePrefifx;
}

void QgsLocatorFilter::setActivePrefix( const QString &activePrefix )
{
  mActivePrefifx = activePrefix;
}

void QgsLocatorFilter::logMessage( const QString &message, Qgis::MessageLevel level )
{
  QgsMessageLog::logMessage( QString( "%1: %2" ).arg( name(), message ), QStringLiteral( "Locator bar" ), level );
}

