#ifndef QGSPDFWRITER_H
#define QGSPDFWRITER_H

#include "qgswmsrequest.h"

namespace QgsWms
{

  /**
   * Output GetMap response in DXF format
   */
  void writeAsPdf( QgsServerInterface *serverIface, const QgsProject *project,
                   const QgsWmsRequest &request,
                   QgsServerResponse &response );

} // namespace QgsWms

#endif // QGSPDFWRITER_H
