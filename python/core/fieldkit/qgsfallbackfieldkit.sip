class QgsFallbackFieldKit : QgsFieldKit
{
%TypeHeaderCode
#include "qgsfallbackfieldkit.h"
%End
  public:
    QgsFallbackFieldKit();
    QString id() const;
};
