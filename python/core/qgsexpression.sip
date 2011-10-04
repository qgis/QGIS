
class QgsExpression
{
%TypeHeaderCode
#include "qgsexpression.h"
%End

public:
  QgsExpression( const QString& expr );
  ~QgsExpression();

  //! Returns true if an error occurred when parsing the input expression
  bool hasParserError() const;
  //! Returns parser error
  QString parserErrorString() const;

  //! Get the expression ready for evaluation - find out column indexes.
  bool prepare( const QgsFieldMap& fields );

  //! Get list of columns referenced by the expression
  QStringList referencedColumns();
  //! Returns true if the expression uses feature geometry for some computation
  bool needsGeometry();

  // evaluation

  //! Evaluate the feature and return the result
  //! @note prepare() should be called before calling this method
  QVariant evaluate( QgsFeature* f = NULL );

  //! Evaluate the feature and return the result
  //! @note this method does not expect that prepare() has been called on this instance
  QVariant evaluate( QgsFeature* f, const QgsFieldMap& fields );

  //! Returns true if an error occurred when evaluating last input
  bool hasEvalError() const;
  //! Returns evaluation error
  QString evalErrorString() const;
  //! Set evaluation error (used internally by evaluation functions)
  void setEvalErrorString( QString str );

  //! Set the number for $rownum special column
  void setCurrentRowNumber( int rowNumber );
  //! Return the number used for $rownum special column
  int currentRowNumber();

  //! Return the parsed expression as a string - useful for debugging
  QString dump() const;

  //! Return calculator used for distance and area calculations
  //! (used by internal functions)
  QgsDistanceArea* geomCalculator();

  //

  // tells whether the identifier is a name of existing function
  static bool isFunctionName( QString name );

  // return index of the function in BuiltinFunctions array
  static int functionIndex( QString name );

  //! return quoted column reference (in double quotes)
  static QString quotedColumnRef( QString name );
};
