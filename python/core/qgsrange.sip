class QgsDoubleRange
{
%TypeHeaderCode
#include <qgsrange.h>
%End

  public:

    QgsDoubleRange( double lower, double upper, bool includeLower = true, bool includeUpper = true );
    double lower() const;
    double upper() const;
    bool includeLower() const;
    bool includeUpper() const;
    bool isEmpty() const;
    bool contains( const QgsDoubleRange &other ) const;
    bool contains( double element ) const;
    bool overlaps( const QgsDoubleRange &other ) const;
};

class QgsIntRange
{
%TypeHeaderCode
#include <qgsrange.h>
%End

  public:

    QgsIntRange( int lower, int upper, bool includeLower = true, bool includeUpper = true );
    int lower() const;
    int upper() const;
    bool includeLower() const;
    bool includeUpper() const;
    bool isEmpty() const;
    bool contains( const QgsIntRange &other ) const;
    bool contains( int element ) const;
    bool overlaps( const QgsIntRange &other ) const;
};

class QgsDateRange
{
%TypeHeaderCode
#include <qgsrange.h>
%End

  public:

    QgsDateRange( QDate lower, QDate upper, bool includeLower = true, bool includeUpper = true );
    QDate lower() const;
    QDate upper() const;
    bool includeLower() const;
    bool includeUpper() const;
    bool isEmpty() const;
    bool contains( const QgsDateRange &other ) const;
    bool contains( QDate element ) const;
    bool overlaps( const QgsDateRange &other ) const;
};

