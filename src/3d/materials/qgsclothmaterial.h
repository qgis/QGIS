/***************************************************************************
  qgsclothmaterial.h
  --------------------------------------
  Date                 : April 2026
  Copyright            : (C) 2026 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCLOTHMATERIAL_H
#define QGSCLOTHMATERIAL_H

#include "qgis_3d.h"
#include "qgspbrmaterial.h"

#include <QObject>

#define SIP_NO_FILE

namespace Qt3DRender
{

  class QFilterKey;
  class QEffect;
  class QAbstractTexture;
  class QTechnique;
  class QParameter;
  class QShaderProgram;
  class QShaderProgramBuilder;
  class QRenderPass;

} // namespace Qt3DRender

///@cond PRIVATE

/**
 * \ingroup qgis_3d
 * \brief A PBR cloth material.
 * \since QGIS 4.2
 */
class _3D_EXPORT QgsClothMaterial : public QgsPBRMaterial
{
    Q_OBJECT
  public:
    /**
     * Constructor for QgsClothMaterial, with the specified \a parent node.
     */
    explicit QgsClothMaterial( Qt3DCore::QNode *parent = nullptr );
    ~QgsClothMaterial() override;

  public slots:

    void setSheenColor( const QColor &sheenColor );

  protected:
    QStringList fragmentShaderDefines() const final;

  private:
    void init();

    Qt3DRender::QParameter *mSheenParameter = nullptr;

    friend class TestQgsGltf3DUtils;
};

///@endcond PRIVATE

#endif // QGSCLOTHMATERIAL_H
