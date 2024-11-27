#define BASE_COLOR_MAP
#define ROUGHNESS_MAP
// Copyright (C) 2017 Klaralvdalens Datakonsult AB (KDAB).
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifdef BASE_COLOR_MAP
uniform sampler2D baseColorMap;
#else
uniform vec4 baseColor;
#endif

#if defined(BASE_COLOR_MAP) || defined(METALNESS_MAP) || defined(ROUGHNESS_MAP) || defined(AMBIENT_OCCLUSION_MAP) || defined(NORMAL_MAP)
in vec2 texCoord;
#endif

out vec4 fragColor;

void main()
{
#ifdef BASE_COLOR_MAP
    vec4 c = texture(baseColorMap, texCoord);
#else
    vec4 c = baseColor;
#endif

    fragColor = c;
}
