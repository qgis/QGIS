class QgsFallbackFieldFormatter : QgsFieldFormatter
{
%TypeHeaderCode
#include "qgsfallbackfieldformatter.h"
%End
  public:
    QgsFallbackFieldFormatter();
    QString id() const;
};
