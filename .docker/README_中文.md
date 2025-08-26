# QGIS Docker 镜像

QGIS项目提供了几个官方Docker镜像，可用于测试目的。

这些Docker镜像目前用于运行QGIS项目本身的持续集成测试，以及运行几个第三方Python插件的持续集成测试。

这些镜像每天自动构建并推送到Docker Hub的QGIS账户：https://hub.docker.com/r/qgis/

## 可用镜像

### 依赖镜像

`qgis/qgis3-build-deps`

这是一个简单的基础镜像，包含构建QGIS所需的所有依赖项，被其他镜像使用。

此镜像可能有多个版本：镜像名称中的后缀表示它们基于的Ubuntu版本。

### 主QGIS镜像

`qgis/qgis`

这是包含QGIS构建版本的主镜像。

此镜像的Docker标签为每个点发布版本分配（前缀为`final-`），为活跃开发分支分配（前缀为`release-`），而`latest`标签指向当前master分支的构建版本。

#### 特性

Docker文件从当前目录构建QGIS，并设置适合在QGIS内运行测试的测试环境。

您可以使用此Docker镜像测试QGIS和/或在QGIS内运行单元测试。`xvfb`（虚拟X服务器）作为容器内的服务可用并运行，允许在CI流水线（如Travis或Circle-CI）中进行完全自动化的无头测试。

#### 构建

您可以从QGIS源代码树的主目录构建镜像：

```bash
$ docker build -t qgis/qgis:latest \
    --build-arg DOCKER_TAG=latest \
    -f .docker/qgis.dockerfile \
    .
```

`DOCKER_TAG`参数可用于指定依赖镜像的标签。

#### 运行QGIS

您也可以使用此镜像在桌面上运行QGIS。

要运行QGIS容器，假设您想使用当前显示器来使用QGIS，镜像标记为`qgis/qgis:latest`，您可以使用如下脚本：

```bash
# 允许来自任何主机的连接
$ xhost +
$ docker run --rm -it --name qgis \
    -v /tmp/.X11-unix:/tmp/.X11-unix  \
    -e DISPLAY=unix$DISPLAY \
    qgis/qgis:latest qgis
```

此代码片段将在容器内启动QGIS并在屏幕上显示应用程序。

#### 在QGIS内运行单元测试

假设您有包含要在QGIS中执行的测试的本地目录：

```
/my_tests/travis_tests/
├── __init__.py
└── test_TravisTest.py
```

要在容器内运行测试，您必须将包含测试的目录（例如您的本地目录`/my_tests`）挂载到容器可访问的卷中，参见下面示例中的`-v /my_tests/:/tests_directory`：

```bash
$ docker run -d --name qgis -v /tmp/.X11-unix:/tmp/.X11-unix \
    -v /my_tests/:/tests_directory \
    -e DISPLAY=:99 \
    qgis/qgis:latest
```

以下是`test_TravisTest.py`的摘录：

```python
# -*- coding: utf-8 -*-
import sys
from qgis.testing import unittest

class TestTest(unittest.TestCase):

    def test_passes(self):
        self.assertTrue(True)

def run_all():
    """如果没有指定其他内容，运行器调用的默认函数"""
    suite = unittest.TestSuite()
    suite.addTests(unittest.makeSuite(TestTest, 'test'))
    unittest.TextTestRunner(verbosity=3, stream=sys.stdout).run(suite)
```

完成后，您可以通过指定要运行的测试来调用测试运行器，例如：

```bash
$ docker exec -it qgis sh -c "cd /tests_directory && qgis_testrunner.sh travis_tests.test_TravisTest.run_fail"
```

测试可以使用点号表示法指定，类似于Python导入表示法。默认情况下将执行名为`run_all`的函数，但您可以在点号语法的最后一项中传递另一个函数名：

```bash
# 调用test_TravisTest模块内的默认函数"run_all"
qgis_testrunner.sh travis_tests.test_TravisTest
# 调用test_TravisTest模块内的函数"run_fail"
qgis_testrunner.sh travis_tests.test_TravisTest.run_fail
```

请注意，为了使测试脚本对Python可访问，调用命令必须确保测试在Python路径中。常见模式包括：
- 切换到包含测试的目录（如上面的示例）
- 将包含测试的目录添加到`PYTHONPATH`

##### 为Python插件运行测试

上述所有注意事项也适用于此情况，但是为了模拟在QGIS内安装插件，您需要额外执行一个步骤：在实际运行测试之前在docker容器中调用`qgis_setup.sh <YourPluginName>`（参见关于在Travis上运行的段落以获取完整示例）。

`qgis_setup.sh`脚本准备QGIS以无头模式运行并模拟插件安装过程：

- 创建QGIS配置文件夹
- 通过从配置文件夹到插件文件夹创建符号链接来"安装"插件
- 安装`startup.py`猴子补丁以防止阻塞对话框
- 启用插件

请注意，根据插件仓库内部目录结构，您可能需要调整（删除并创建）由`qgis_setup.sh`创建的符号链接。这在特定情况下是必需的，即如果仓库中的真实插件代码包含在主目录中而不是与插件内部名称同名的子目录中（`metadata.txt`中的名称）。

##### 测试运行器选项

环境变量`QGIS_EXTRA_OPTIONS`默认为空字符串，可以包含测试运行器传递给QGIS的额外参数。

##### 在Travis上运行

以下是运行小型QGIS插件（名为*QuickWKT*）单元测试的简单示例，假设测试位于QuickWKT插件主目录下的`tests/test_Plugin.py`：

```yml
services:
    - docker
install:
    - docker run -d --name qgis-testing-environment -v ${TRAVIS_BUILD_DIR}:/tests_directory -e DISPLAY=:99 qgis/qgis:latest
    - sleep 10 # 这是允许xvfb启动所必需的
    # 设置qgis并启用插件
    - docker exec -it qgis-testing-environment sh -c "qgis_setup.sh QuickWKT"
    # 额外步骤（例如make或paver setup）在这里
    # 修复由qgis_setup.sh创建的符号链接
    - docker exec  -it qgis-testing-environment sh -c "rm -f  /root/.local/share/QGIS/QGIS3/profiles/default/python/plugins/QuickWKT"
    - docker exec -it qgis-testing-environment sh -c "ln -s /tests_directory/ /root/.local/share/QGIS/QGIS3/profiles/default/python/plugins/QuickWKT"

script:
    - docker exec -it qgis-testing-environment sh -c "cd /tests_directory && qgis_testrunner.sh tests.test_Plugin"
```

请注意，这里可以避免在调用`qgis_testrunner.sh`之前使用`cd /tests_directory && `，因为QGIS会自动将插件主目录添加到Python路径。

##### 在Circle-CI上运行

以下是运行小型QGIS插件（名为*QuickWKT*）单元测试的示例，假设测试位于QuickWKT插件主目录下的`tests/test_Plugin.py`：

```yml
version: 2
jobs:
  build:
    docker:
      - image: qgis/qgis:latest
        environment:
          DISPLAY: ":99"
    working_directory: /tests_directory
    steps:
      - checkout
      - run:
          name: Setup plugin
          command: |
            qgis_setup.sh QuickWKT
      - run:
          name: Fix installation path created by qgis_setup.sh
          command: |
            rm -f  /root/.local/share/QGIS/QGIS3/profiles/default/python/plugins/QuickWKT
            ln -s /tests_directory/ /root/.local/share/QGIS/QGIS3/profiles/default/python/plugins/QuickWKT
      - run:
          name: run tests
          command: |
            sh -c "/usr/bin/Xvfb :99 -screen 0 1024x768x24 -ac +extension GLX +render -noreset -nolisten tcp &"
            qgis_testrunner.sh tests.test_Plugin
```

##### 实现说明

此镜像中测试运行器的主要目标是在真实的QGIS实例（而不是模拟实例）内执行单元测试。

QGIS测试应该能够从Travis/Circle-CI CI作业运行。

实现包括：

- 运行docker，将单元测试文件夹作为卷挂载到`/tests_directory`（如果单元测试属于插件且运行测试需要插件，则为QGIS插件文件夹）
- 在docker中执行`qgis_setup.sh MyPluginName`脚本，设置QGIS以避免阻塞模态对话框，并在需要时将插件安装到QGIS中
  - 为QGIS创建配置和python插件文件夹
  - 在QGIS配置文件中启用插件
  - 安装`startup.py`脚本以禁用python异常模态对话框
- 通过运行`qgis_testrunner.sh MyPluginName.tests.tests_MyTestModule.run_my_tests_function`执行测试
- 测试输出由`test_runner.sh`脚本捕获，并搜索`FAILED`（在标准单元测试输出中）和其他表示失败或成功条件的字符串。如果识别到失败条件，脚本以`1`退出，否则以`0`退出。

`qgis_testrunner.sh`接受测试模块的点号表示法路径，可以以必须在模块内调用以运行测试的函数结尾。最后部分（`.run_my_tests_function`）可以省略，默认为`run_all`。

## 使用场景和最佳实践

### 1. QGIS核心开发
- **本地开发**: 使用Docker环境进行本地编译和测试
- **CI/CD集成**: 自动化构建和测试流程
- **跨平台验证**: 确保代码在标准化环境中正常工作

### 2. 插件开发
- **插件测试**: 自动化插件单元测试
- **兼容性验证**: 测试插件与不同QGIS版本的兼容性
- **发布前验证**: 确保插件质量

### 3. 教育和培训
- **标准化环境**: 为学习者提供一致的QGIS环境
- **实验环境**: 安全的实验和学习平台
- **批量部署**: 大规模部署统一的QGIS环境

## 技术优势

### 1. 环境一致性
- **标准化依赖**: 所有依赖项版本固定
- **跨平台兼容**: Linux/Windows/macOS一致表现
- **可重现构建**: 确保构建结果可重现

### 2. 自动化能力
- **持续集成**: 完整的CI/CD支持
- **自动化测试**: 无人干预的测试执行
- **质量保证**: 自动化代码质量检查

### 3. 扩展性
- **模块化设计**: 依赖镜像和应用镜像分离
- **版本管理**: 支持多版本并行
- **自定义扩展**: 易于扩展和定制

这个Docker基础设施为QGIS项目提供了强大的开发、测试和部署能力，是现代软件工程实践的优秀范例。