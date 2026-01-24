/***************************************************************************
  qgsrastergpushaders.cpp
  -----------------------
  Date                 : January 2025
  Copyright            : (C) 2025 by Wietze Suijker
  Email                : wietze at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrastergpushaders.h"
#include <QOpenGLShader>

QString QgsRasterGPUShaders::vertexShaderSource()
{
  return R"SHADER(
    #version 330 core

    layout(location = 0) in vec2 aPosition;
    layout(location = 1) in vec2 aTexCoord;

    out vec2 vTexCoord;

    uniform mat4 uMVPMatrix;

    void main() {
      gl_Position = uMVPMatrix * vec4(aPosition, 0.0, 1.0);
      vTexCoord = aTexCoord;
    }
  )SHADER";
}

QString QgsRasterGPUShaders::commonFunctions()
{
  return R"SHADER(
    // Linear interpolation between color stops
    vec4 applyColorRamp(float value, float colorStops[64], vec4 colors[32], int numStops) {
      if (numStops < 2) {
        return vec4(value, value, value, 1.0);
      }

      // Clamp to range
      if (value <= colorStops[0]) {
        return colors[0];
      }
      if (value >= colorStops[numStops - 1]) {
        return colors[numStops - 1];
      }

      // Find stop interval
      for (int i = 0; i < numStops - 1; i++) {
        float v0 = colorStops[i];
        float v1 = colorStops[i + 1];

        if (value >= v0 && value <= v1) {
          float t = (value - v0) / (v1 - v0);
          return mix(colors[i], colors[i + 1], t);
        }
      }

      return vec4(0.0, 0.0, 0.0, 1.0);
    }

    // Normalize value to 0-1 range
    float normalizeValue(float value, float minVal, float maxVal) {
      return (value - minVal) / (maxVal - minVal);
    }
  )SHADER";
}

QString QgsRasterGPUShaders::fragmentShaderSource( ShaderType type )
{
  QString commonFuncs = commonFunctions();

  switch ( type )
  {
    case ShaderType::Byte:
    {
      return commonFuncs + R"SHADER(
        #version 330 core

        in vec2 vTexCoord;
        out vec4 fragColor;

        uniform sampler2D uTileTexture;
        uniform float uColorStops[64];
        uniform vec4 uColors[32];
        uniform int uNumStops;
        uniform float uMinValue;
        uniform float uMaxValue;
        uniform float uNoDataValue;
        uniform bool uUseNoData;
        uniform float uOpacity;

        void main() {
          // Sample byte texture (R8)
          float value = texture(uTileTexture, vTexCoord).r * 255.0;

          // Check for no-data
          if (uUseNoData && abs(value - uNoDataValue) < 0.5) {
            discard;
          }

          // Normalize to 0-1
          float normalized = normalizeValue(value, uMinValue, uMaxValue);

          // Apply color ramp
          vec4 color = applyColorRamp(normalized, uColorStops, uColors, uNumStops);

          fragColor = vec4(color.rgb, color.a * uOpacity);
        }
      )SHADER";
    }

    case ShaderType::UInt16:
    {
      return commonFuncs + R"SHADER(
        #version 330 core

        in vec2 vTexCoord;
        out vec4 fragColor;

        uniform sampler2D uTileTexture;
        uniform float uColorStops[64];
        uniform vec4 uColors[32];
        uniform int uNumStops;
        uniform float uMinValue;
        uniform float uMaxValue;
        uniform float uNoDataValue;
        uniform bool uUseNoData;
        uniform float uOpacity;

        void main() {
          // Sample uint16 texture (R16)
          float value = texture(uTileTexture, vTexCoord).r * 65535.0;

          // Check for no-data
          if (uUseNoData && abs(value - uNoDataValue) < 0.5) {
            discard;
          }

          // Normalize to 0-1
          float normalized = normalizeValue(value, uMinValue, uMaxValue);

          // Apply color ramp
          vec4 color = applyColorRamp(normalized, uColorStops, uColors, uNumStops);

          fragColor = vec4(color.rgb, color.a * uOpacity);
        }
      )SHADER";
    }

    case ShaderType::Float32:
    {
      return commonFuncs + R"SHADER(
        #version 330 core

        in vec2 vTexCoord;
        out vec4 fragColor;

        uniform sampler2D uTileTexture;
        uniform float uColorStops[64];
        uniform vec4 uColors[32];
        uniform int uNumStops;
        uniform float uMinValue;
        uniform float uMaxValue;
        uniform float uNoDataValue;
        uniform bool uUseNoData;
        uniform float uOpacity;

        void main() {
          // Sample float32 texture (R32F)
          float value = texture(uTileTexture, vTexCoord).r;

          // Check for no-data
          if (uUseNoData && abs(value - uNoDataValue) < 1e-6) {
            discard;
          }

          // Normalize to 0-1
          float normalized = normalizeValue(value, uMinValue, uMaxValue);

          // Apply color ramp
          vec4 color = applyColorRamp(normalized, uColorStops, uColors, uNumStops);

          fragColor = vec4(color.rgb, color.a * uOpacity);
        }
      )SHADER";
    }

    case ShaderType::RGBA8:
    {
      return R"SHADER(
        #version 330 core

        in vec2 vTexCoord;
        out vec4 fragColor;

        uniform sampler2D uTileTexture;
        uniform float uOpacity;

        void main() {
          // Sample RGBA8 texture directly
          vec4 color = texture(uTileTexture, vTexCoord);
          fragColor = vec4(color.rgb, color.a * uOpacity);
        }
      )SHADER";
    }

    case ShaderType::BytePaletted:
    {
      return R"SHADER(
        #version 330 core

        in vec2 vTexCoord;
        out vec4 fragColor;

        uniform sampler2D uTileTexture;
        uniform sampler1D uPaletteTexture;
        uniform float uOpacity;
        uniform int uPaletteSize;

        void main() {
          // Sample byte value (index into palette)
          float index = texture(uTileTexture, vTexCoord).r * 255.0;

          // Normalize index to 0-1 for palette lookup
          float t = index / float(uPaletteSize - 1);

          // Sample color from 1D palette texture
          vec4 color = texture(uPaletteTexture, t);

          fragColor = vec4(color.rgb, color.a * uOpacity);
        }
      )SHADER";
    }
  }

  return QString();
}

QOpenGLShaderProgram *QgsRasterGPUShaders::createShaderProgram( const ShaderConfig &config )
{
  QOpenGLShaderProgram *program = new QOpenGLShaderProgram();

  // Add vertex shader
  if ( !program->addShaderFromSourceCode( QOpenGLShader::Vertex, vertexShaderSource() ) )
  {
    delete program;
    return nullptr;
  }

  // Add fragment shader
  if ( !program->addShaderFromSourceCode( QOpenGLShader::Fragment, fragmentShaderSource( config.type ) ) )
  {
    delete program;
    return nullptr;
  }

  // Link program
  if ( !program->link() )
  {
    delete program;
    return nullptr;
  }

  return program;
}

void QgsRasterGPUShaders::updateShaderUniforms( QOpenGLShaderProgram *program, const ShaderConfig &config )
{
  if ( !program || !program->isLinked() )
    return;

  program->bind();

  // Set common uniforms
  program->setUniformValue( "uOpacity", config.opacity );

  // Type-specific uniforms
  if ( config.type != ShaderType::RGBA8 && config.type != ShaderType::BytePaletted )
  {
    program->setUniformValue( "uMinValue", config.minValue );
    program->setUniformValue( "uMaxValue", config.maxValue );
    program->setUniformValue( "uNoDataValue", config.noDataValue );
    program->setUniformValue( "uUseNoData", config.useNoData );

    // Upload color ramp
    if ( !config.colorRamp.isEmpty() )
    {
      uploadColorRamp( program, config.colorRamp );
    }
  }

  program->release();
}

void QgsRasterGPUShaders::uploadColorRamp( QOpenGLShaderProgram *program, const QVector<ColorStop> &colorRamp )
{
  if ( !program || colorRamp.isEmpty() )
    return;

  const int numStops = std::min( static_cast<int>( colorRamp.size() ), 32 );

  // Prepare arrays
  float stops[64];  // Max 32 stops * 2 (for alignment)
  float colors[128]; // Max 32 stops * 4 components (RGBA)

  for ( int i = 0; i < numStops; ++i )
  {
    stops[i] = colorRamp[i].value;

    const QColor &c = colorRamp[i].color;
    colors[i * 4 + 0] = c.redF();
    colors[i * 4 + 1] = c.greenF();
    colors[i * 4 + 2] = c.blueF();
    colors[i * 4 + 3] = c.alphaF();
  }

  // Upload to shader
  program->setUniformValue( "uNumStops", numStops );
  program->setUniformValueArray( "uColorStops", stops, numStops, 1 );
  program->setUniformValueArray( "uColors", colors, numStops, 4 );
}
