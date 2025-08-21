<img src="images/README-md/main_logo.png" width="300">

[![🧪 QGIS测试](https://github.com/qgis/QGIS/actions/workflows/run-tests.yml/badge.svg)](https://github.com/qgis/QGIS/actions/workflows/run-tests.yml?query=branch%3Amaster+event%3Apush)
[![Docker状态](https://img.shields.io/docker/automated/qgis/qgis.svg)](https://hub.docker.com/r/qgis/qgis/tags)
[![构建状态](https://dev.azure.com/qgis/QGIS/_apis/build/status/qgis.QGIS?branchName=master)](https://dev.azure.com/qgis/QGIS/_build/latest?definitionId=1&branchName=master)
[![OpenSSF评分卡](https://api.securityscorecards.dev/projects/github.com/qgis/QGIS/badge)](https://securityscorecards.dev/viewer/?uri=github.com/qgis/QGIS)
[![OpenSSF最佳实践](https://www.bestpractices.dev/projects/1581/badge)](https://www.bestpractices.dev/projects/1581)
[![🪟 MingW64 Windows 64位构建](https://github.com/qgis/QGIS/actions/workflows/mingw64.yml/badge.svg)](https://github.com/qgis/QGIS/actions/workflows/mingw64.yml?query=branch%3Amaster+event%3Apush)
[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.5869837.svg)](https://doi.org/10.5281/zenodo.5869837)

QGIS是一个功能全面、用户友好的免费开源(FOSS)地理信息系统(GIS)，可在Unix平台、Windows和MacOS上运行。

<!-- 目录由 https://freelance-tech-writer.github.io/table-of-contents-generator/index.html 生成 -->

- [功能特性](#功能特性)
  - [1. 灵活强大的空间数据管理](#1-灵活强大的空间数据管理)
  - [2. 精美的地图制图](#2-精美的地图制图)
  - [3. 先进稳健的地理空间分析](#3-先进稳健的地理空间分析)
  - [4. 强大的自定义和扩展能力](#4-强大的自定义和扩展能力)
  - [5. QGIS服务器](#5-qgis服务器)
- [技术内幕](#技术内幕)
  - [版本和发布周期](#版本和发布周期)
  - [自由开源](#自由开源)
- [安装和使用QGIS](#安装和使用qgis)
  - [文档资料](#文档资料)
  - [帮助和支持渠道](#帮助和支持渠道)
- [参与社区](#参与社区)

## 功能特性

### 1. 灵活强大的空间数据管理

- 支持栅格、矢量、网格和点云数据的多种行业标准格式
    - *栅格格式包括*：GeoPackage、GeoTIFF、GRASS、ArcInfo二进制和ASCII网格、ERDAS Imagine SDTS、WMS、WCS、PostgreSQL/PostGIS以及[其他GDAL支持的格式](https://gdal.org/drivers/raster/index.html)。
    - *矢量格式包括*：GeoPackage、ESRI shapefiles、GRASS、SpatiaLite、PostgreSQL/PostGIS、MSSQL、Oracle、WFS、矢量瓦片以及[其他OGR支持的格式](https://www.gdal.org/ogr_formats.html)。
    - *网格格式包括*：NetCDF、GRIB、2DM以及[其他MDAL支持的格式](https://github.com/lutraconsulting/MDAL#supported-formats)。
    - *点云格式*：LAS/LAZ和EPT数据集。
- 数据抽象框架，通过统一的数据模型和浏览器界面访问本地文件、空间数据库（PostGIS、SpatiaLite、SQL Server、Oracle、SAP HANA）和网络服务（WMS、WCS、WFS、ArcGIS REST），并作为用户创建项目中的灵活图层
- 通过可视化和数值化数字化编辑进行空间数据创建，以及栅格和矢量数据的地理配准
- 坐标参考系统(CRS)之间的动态重投影
- Nominatim (OpenStreetMap) 地理编码器访问
- 时间支持

*示例：时间动画*

![示例：时间动画](images/README-md/icebergs.gif "时间动画")

*示例：3D地图视图*

![示例：3D地图视图](https://docs.qgis.org/latest/en/_images/3dmapview.png "3D地图视图")

### 2. 精美的地图制图
- 2D和3D中的多种渲染选项
- 对符号化、标注、图例和其他图形元素的精细控制，制作精美的地图
- 尊重许多空间数据源中嵌入的样式（如KML和TAB文件、Mapbox-GL样式矢量瓦片）
- 特别是，几乎完全复制（并显著扩展）ESRI专有软件中可用的符号化选项
- 使用数据定义覆盖、混合模式和绘制效果的高级样式
- 500多个内置色带（cpt-city、ColorBrewer等）
- 通过保存的布局创建和更新具有指定比例尺、范围、样式和装饰的地图
- 使用QGIS Atlas和QGIS Reports自动生成多个地图（和报告）
- 显示和导出具有灵活符号化的高程剖面图
- 灵活输出直接到打印机，或作为图像（栅格）、PDF或SVG进行进一步自定义
- 使用几何生成器的动态渲染增强（例如，从现有要素创建和样式化新几何）
- 无障碍地图制作的预览模式（如单色、色盲）

*[示例：哥伦比亚波哥大星空夜晚风格地图，作者Andrés Felipe Lancheros Sánchez](https://flic.kr/p/2jFfGJP)*

![哥伦比亚波哥大星空夜晚风格地图](https://live.staticflickr.com/65535/50327326323_3da28f0d86_b.jpg "哥伦比亚波哥大星空夜晚风格地图")

更多使用QGIS创建的地图，请访问[QGIS地图展示Flickr群组](https://www.flickr.com/groups/2244553@N22/pool/with/50355460063/)。

![QGIS地图展示](images/README-md/qgis_map_showcase.png "QGIS地图展示")

### 3. 先进稳健的地理空间分析
- 强大的处理框架，包含200多个原生处理算法
- 通过GDAL、SAGA、GRASS、OrfeoToolbox等提供商以及自定义模型和处理脚本访问1000多个处理算法
- 地理空间数据库引擎（过滤器、连接、关系、表单等），尽可能接近数据源和格式无关
- 地理空间查询和地理处理结果的即时可视化
- 模型设计器和批处理

*示例：出行等时线*

![示例：出行等时线](images/README-md/network_analysis_2.png "出行等时线")

*示例：模型设计器*

![示例：模型设计器](https://docs.qgis.org/latest/en/_images/models_model.png "模型设计器")

### 4. 强大的自定义和扩展能力

- 完全可自定义的用户体验，包括适合高级用户和初学者的用户界面和应用程序设置
- 丰富的[表达式引擎](https://docs.qgis.org/testing/en/docs/user_manual/working_with_vector/expression.html)，在可视化和处理中提供最大的灵活性
- 广泛多样的[插件生态系统](https://plugins.qgis.org/)，包括数据连接器、数字化辅助、高级分析和图表工具、野外数据采集、ESRI样式文件转换等
- 用于创建、存储和管理样式的样式管理器
- [QGIS样式中心](https://plugins.qgis.org/styles/)便于样式共享
- Python和C++ API用于独立（无头）应用程序以及应用程序内全面脚本（PyQGIS）

*示例：样式管理器*

![示例：样式管理器](https://docs.qgis.org/latest/en/_images/stylemanager.png "样式管理器")

*示例：插件*

![示例：插件](images/README-md/plugins_1.png "插件")

<!-- 暂时删除这个，因为提供的示例是Python2而不是3
示例：Python控制台

![示例：Python控制台](https://docs.qgis.org/latest/en/_images/python_console_editor.png "Python控制台")
-->

### 5. QGIS服务器

无头地图服务器——在Linux、macOS、Windows或Docker容器中运行——与QGIS共享相同的代码库。

- 行业标准协议（WMS、WFS、WFS3/OGC API for Features和WCS）允许与任何软件栈即插即用
- 适用于任何Web服务器（Apache、nginx等）或独立运行
- 支持所有精美的QGIS制图，具有最佳的打印支持
- 完全可通过Python脚本自定义

*示例：QGIS服务器WMS响应*

![示例：QGIS服务器对WMS请求的响应](https://docs.qgis.org/latest/en/_images/server_selection_parameter.png "QGIS服务器对WMS请求的响应")

*示例：QGIS服务器WFS响应*

![示例：QGIS服务器对WFS要素请求的响应](https://docs.qgis.org/latest/en/_images/server_wfs3_feature.png "QGIS服务器对WFS要素请求的响应")

## 技术内幕

QGIS自2002年以来使用[Qt工具包](https://qt.io)和C++开发，具有令人愉悦、易于使用的多语言支持图形用户界面。它由活跃的开发团队维护，并得到由GIS专业人员和爱好者以及地理空间数据发布者和终端用户组成的充满活力的社区支持。

### 版本和发布周期

QGIS开发和发布遵循[基于时间的时间表/路线图](https://www.qgis.org/en/site/getinvolved/development/roadmap.html)。用户可以安装QGIS的三个主要分支。它们是**长期支持版本(LTR)**分支、**最新发布版本(LR)**分支和**开发版本(Nightly)**分支。

每月都会有一个**点发布**，为LTR和LR提供错误修复。

### 自由开源

QGIS基于GNU通用公共许可证(GPL)第2版或任何后续版本发布。
在此许可证下开发QGIS意味着您可以（如果您想要）检查和修改源代码，并保证您——我们快乐的用户——将始终可以访问免费的GIS程序，该程序可以自由修改。

QGIS是开源地理空间基金会([OSGeo](https://www.osgeo.org/))的一部分，提供一系列互补的开源GIS软件项目。

## 安装和使用QGIS

QGIS的预编译二进制文件可在[QGIS.org下载页面](https://www.qgis.org/en/site/forusers/download.html)获得。
请仔细按照安装说明操作。

[构建指南](INSTALL.md)可用于开始从源代码构建QGIS。

有关QGIS服务器的安装，请参阅其[入门文档](https://docs.qgis.org/testing/en/docs/server_manual/getting_started.html)。

### 文档资料

有一系列[文档](https://qgis.org/resources/hub/#documentation)可用。包括：

- [培训手册](https://docs.qgis.org/latest/en/docs/training_manual/index.html)
- [QGIS用户指南](https://docs.qgis.org/latest/en/docs/user_manual/index.html)
- [QGIS服务器指南](https://docs.qgis.org/latest/en/docs/server_manual/index.html)
- [可视化变更日志](https://qgis.org/project/visual-changelogs/)
- [文档编写指南](https://docs.qgis.org/latest/en/docs/documentation_guidelines/index.html)
- [QGIS Python (PyQGIS) 手册](https://docs.qgis.org/latest/en/docs/pyqgis_developer_cookbook/index.html)
- [QGIS Python (PyQGIS) API](https://qgis.org/pyqgis/)
- [QGIS C++ API](https://qgis.org/api/)
- [开发者指南](https://docs.qgis.org/latest/en/docs/developers_guide/index.html)

### 帮助和支持渠道

有几个渠道可以找到QGIS的帮助和支持：

- 使用[QGIS社区网站](https://qgis.org)
- 加入[qgis-users邮件列表](https://lists.osgeo.org/mailman/listinfo/qgis-user)
- 与其他用户实时聊天。*请耐心等待对您问题的回复，因为频道上的许多人在做其他事情，可能需要一段时间才能注意到您的问题。以下路径都会带您到同一个聊天室：*
    - 使用IRC客户端并加入irc.libera.chat上的[#qgis](https://web.libera.chat/?channels=#qgis)频道。
    - 使用Matrix客户端并加入[#qgis:osgeo.org](https://matrix.to/#/#qgis:osgeo.org)房间。
- 在[GIS stackexchange](https://gis.stackexchange.com/)或[r/QGIS reddit](https://www.reddit.com/r/QGIS/)，这些不是由QGIS团队维护的，但QGIS和更广泛的GIS社区在那里提供了很多建议
- [其他支持渠道](https://qgis.org/resources/support/)

## 参与社区

[本项目的贡献指南](CONTRIBUTING.md)