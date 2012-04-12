#ifndef QGSWFSUTILS_H
#define QGSWFSUTILS_H

class QgsExpression;
class QDomDocument;

class QgsWFSUtils
{
  public:

    //! Creates ogc filter xml document. Supports minimum standard filter according to the OGC filter specs (=,!=,<,>,<=,>=,AND,OR,NOT)
    //! @return true in case of success. False if string contains something that goes beyond the minimum standard filter
    static bool expressionToOGCFilter( QgsExpression& exp, QDomDocument& doc );
};

#endif // QGSWFSUTILS_H
