# The following has been generated automatically from src/core/expression/qgsexpression.h
QgsExpression.ParserError.Unknown = QgsExpression.ParserError.ParserErrorType.Unknown
QgsExpression.ParserError.FunctionUnknown = QgsExpression.ParserError.ParserErrorType.FunctionUnknown
QgsExpression.ParserError.FunctionWrongArgs = QgsExpression.ParserError.ParserErrorType.FunctionWrongArgs
QgsExpression.ParserError.FunctionInvalidParams = QgsExpression.ParserError.ParserErrorType.FunctionInvalidParams
QgsExpression.ParserError.FunctionNamedArgsError = QgsExpression.ParserError.ParserErrorType.FunctionNamedArgsError
QgsExpression.soBbox = QgsExpression.SpatialOperator.soBbox
QgsExpression.soIntersects = QgsExpression.SpatialOperator.soIntersects
QgsExpression.soContains = QgsExpression.SpatialOperator.soContains
QgsExpression.soCrosses = QgsExpression.SpatialOperator.soCrosses
QgsExpression.soEquals = QgsExpression.SpatialOperator.soEquals
QgsExpression.soDisjoint = QgsExpression.SpatialOperator.soDisjoint
QgsExpression.soOverlaps = QgsExpression.SpatialOperator.soOverlaps
QgsExpression.soTouches = QgsExpression.SpatialOperator.soTouches
QgsExpression.soWithin = QgsExpression.SpatialOperator.soWithin
try:
    QgsExpression.__attribute_docs__ = {'errorType': 'The type of parser error that was found.', 'errorMsg': 'The message for the error at this location.', 'firstLine': "The first line that contained the error in the parser.\nDepending on the error sometimes this doesn't mean anything.", 'firstColumn': "The first column that contained the error in the parser.\nDepending on the error sometimes this doesn't mean anything.", 'lastLine': 'The last line that contained the error in the parser.', 'lastColumn': 'The last column that contained the error in the parser.'}
except NameError:
    pass
