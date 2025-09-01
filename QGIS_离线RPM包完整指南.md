# QGIS离线RPM包完整构建安装指南

## 概述

本指南详细介绍如何在有网络的环境中构建QGIS RPM包，并创建包含所有依赖的离线安装包，最后在离线环境中完成安装。

## 环境要求

### 构建环境（需要网络连接）
- **操作系统**: Fedora 35+, RHEL 8+, CentOS Stream 8+, Rocky Linux 8+
- **最低配置**: 4GB RAM, 20GB磁盘空间
- **推荐配置**: 8GB RAM, 50GB磁盘空间
- **网络**: 稳定的互联网连接

### 目标离线环境
- **操作系统**: 与构建环境相同或兼容的Linux发行版
- **架构**: x86_64（默认）
- **磁盘空间**: 至少5GB用于安装包

---

## 第一阶段：构建环境准备

### 1.1 安装构建工具

```bash
# Fedora系统
sudo dnf groupinstall "Development Tools" "RPM Development Tools"
sudo dnf install rpm-build rpmdevtools mock git createrepo_c

# RHEL/CentOS/Rocky系统  
sudo yum groupinstall "Development Tools"
sudo yum install rpm-build rpmdevtools mock git createrepo
# 如果没有createrepo，可以安装createrepo_c
sudo dnf install epel-release
sudo dnf install createrepo_c
```

### 1.2 配置构建环境

```bash
# 设置RPM构建目录
rpmdev-setuptree

# 将当前用户添加到mock组
sudo usermod -a -G mock $USER

# 重新登录或刷新组权限
newgrp mock

# 验证mock权限
mock --version
```

### 1.3 获取QGIS源码

```bash
# 克隆QGIS源码仓库
git clone https://github.com/qgis/QGIS.git
cd QGIS

# 切换到稳定版本（可选）
git checkout lts-3_34  # 或其他稳定版本

# 确认版本信息
grep "SET(CPACK_PACKAGE_VERSION" CMakeLists.txt
```

---

## 第二阶段：构建QGIS RPM包

### 2.1 准备构建配置

```bash
# 进入rpm目录
cd rpm/

# 检查默认配置
cat default.cfg

# 创建本地配置（可选）
cat > local.cfg << 'EOF'
# 构建输出目录
OUTDIR="$HOME/qgis-build"

# 目标架构（可多选）
ARCHS=("fedora-39-x86_64")  # 根据实际系统调整

# 是否签名包（离线环境通常不需要）
NOSIGN=true
EOF
```

### 2.2 检查构建依赖

```bash
# 安装QGIS构建依赖
sudo dnf builddep ../CMakeLists.txt

# 手动安装可能缺失的依赖
sudo dnf install \
    qt5-qtbase-devel qt5-qttools-devel qt5-qtsvg-devel \
    qt5-qtwebkit-devel qt5-qtmultimedia-devel qt5-qtpositioning-devel \
    qt5-qtsensors-devel qt5-qtserialport-devel qt5-qt3d-devel \
    gdal-devel geos-devel proj-devel sqlite-devel \
    spatialite-devel spatialindex-devel \
    python3-devel python3-qscintilla-qt5-devel \
    python3-pyqt5-devel python3-sip-devel \
    qca-qt5-devel qscintilla-qt5-devel \
    qtkeychain-qt5-devel qwt-qt5-devel \
    netcdf-devel hdf5-devel postgresql-devel \
    fcgi-devel expat-devel libzip-devel \
    protobuf-devel draco-devel \
    flex bison cmake ninja-build
```

### 2.3 执行构建

```bash
# 生成spec文件（检查配置）
./buildrpms.sh -c

# 检查生成的spec文件
less qgis.spec

# 执行完整构建
./buildrpms.sh

# 或分步构建（便于调试）
./buildrpms.sh -s    # 先构建SRPM
./buildrpms.sh -b    # 再构建二进制RPM
```

### 2.4 验证构建结果

```bash
# 检查构建输出目录
ls -la $HOME/qgis-build/

# 查看生成的RPM包
find $HOME/qgis-build -name "*.rpm" -type f

# 检查包信息
rpm -qip $HOME/qgis-build/fedora-39-x86_64/qgis-*.rpm
```

---

## 第三阶段：创建离线安装包

### 3.1 收集RPM包和依赖

创建依赖收集脚本：

```bash
cat > collect_offline_rpms.sh << 'EOF'
#!/bin/bash

set -e

# 配置变量
BUILD_DIR="$HOME/qgis-build"
OUTPUT_DIR="qgis-offline-$(date +%Y%m%d)"
ARCH="fedora-39-x86_64"  # 根据实际情况调整

echo "=== 创建QGIS离线RPM包 ==="

# 创建输出目录
mkdir -p "$OUTPUT_DIR"
cd "$OUTPUT_DIR"

# 1. 复制构建好的QGIS RPM包
echo "1. 复制QGIS RPM包..."
cp $BUILD_DIR/$ARCH/qgis-*.rpm .

# 获取主包名
MAIN_RPM=$(ls qgis-[0-9]*.rpm | head -1)
echo "主包: $MAIN_RPM"

# 2. 下载运行时依赖
echo "2. 下载运行时依赖..."

# 方法A: 使用dnf download（推荐）
if command -v dnf &> /dev/null; then
    echo "使用dnf下载依赖..."
    
    # 下载直接依赖
    dnf download --resolve --alldeps --skip-broken $MAIN_RPM 2>/dev/null || true
    
    # 下载Python相关依赖
    dnf download python3-qgis python3-pyqt5 python3-sip python3-gdal 2>/dev/null || true
    
    # 下载核心库依赖
    dnf download gdal-libs geos proj-libs sqlite spatialite 2>/dev/null || true
    
    # 下载Qt依赖
    dnf download qt5-qtbase qt5-qtsvg qt5-qtwebkit qt5-qtmultimedia 2>/dev/null || true

# 方法B: 使用yum（备选）
elif command -v yum &> /dev/null; then
    echo "使用yum下载依赖..."
    yumdownloader --resolve $MAIN_RPM 2>/dev/null || true
fi

# 3. 手动添加常见依赖（确保完整性）
echo "3. 下载核心依赖包..."
CORE_DEPS=(
    "gdal-libs" "geos" "proj" "sqlite" "spatialite"
    "python3" "python3-libs" "python3-numpy"
    "qt5-qtbase" "qt5-qtsvg" "qt5-qtwebkit"
    "postgresql-libs" "netcdf" "hdf5"
    "protobuf" "expat" "libzip"
)

for dep in "${CORE_DEPS[@]}"; do
    if command -v dnf &> /dev/null; then
        dnf download $dep 2>/dev/null || echo "跳过: $dep"
    elif command -v yum &> /dev/null; then
        yumdownloader $dep 2>/dev/null || echo "跳过: $dep"
    fi
done

# 4. 创建仓库元数据
echo "4. 创建RPM仓库..."
if command -v createrepo_c &> /dev/null; then
    createrepo_c .
elif command -v createrepo &> /dev/null; then
    createrepo .
else
    echo "警告: 未找到createrepo，请手动安装"
fi

# 5. 创建安装脚本
echo "5. 创建安装脚本..."
cat > install-qgis.sh << 'INSTALL_EOF'
#!/bin/bash

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_NAME="qgis-offline"

echo "=== QGIS离线安装脚本 ==="

# 检查权限
if [ "$EUID" -ne 0 ]; then
    echo "请使用sudo运行此脚本"
    exit 1
fi

# 创建本地仓库配置
echo "配置本地RPM仓库..."
cat > /etc/yum.repos.d/$REPO_NAME.repo << EOL
[$REPO_NAME]
name=QGIS Offline Repository
baseurl=file://$SCRIPT_DIR
enabled=1
gpgcheck=0
priority=1
EOL

echo "仓库配置已创建: /etc/yum.repos.d/$REPO_NAME.repo"

# 清理并更新缓存
if command -v dnf &> /dev/null; then
    echo "清理DNF缓存..."
    dnf clean all
    dnf makecache --repo=$REPO_NAME
    
    echo "安装QGIS及其依赖..."
    dnf install qgis --repo=$REPO_NAME -y --skip-broken
    
elif command -v yum &> /dev/null; then
    echo "清理YUM缓存..."
    yum clean all
    yum makecache
    
    echo "安装QGIS及其依赖..."
    yum install qgis --disablerepo="*" --enablerepo=$REPO_NAME -y
fi

echo "=== 安装完成 ==="
echo "运行 'qgis' 命令启动QGIS"
echo "或从应用程序菜单中找到QGIS"

# 验证安装
if command -v qgis &> /dev/null; then
    echo "QGIS已成功安装"
    qgis --version 2>/dev/null || echo "QGIS版本信息获取失败，但程序已安装"
else
    echo "警告: QGIS命令未找到，可能安装不完整"
fi
INSTALL_EOF

chmod +x install-qgis.sh

# 6. 创建卸载脚本
cat > uninstall-qgis.sh << 'UNINSTALL_EOF'
#!/bin/bash

REPO_NAME="qgis-offline"

echo "=== QGIS卸载脚本 ==="

if [ "$EUID" -ne 0 ]; then
    echo "请使用sudo运行此脚本"
    exit 1
fi

# 卸载QGIS
if command -v dnf &> /dev/null; then
    dnf remove qgis qgis-* -y 2>/dev/null || true
elif command -v yum &> /dev/null; then
    yum remove qgis qgis-* -y 2>/dev/null || true
fi

# 删除仓库配置
if [ -f "/etc/yum.repos.d/$REPO_NAME.repo" ]; then
    rm -f "/etc/yum.repos.d/$REPO_NAME.repo"
    echo "已删除仓库配置"
fi

# 清理缓存
if command -v dnf &> /dev/null; then
    dnf clean all
elif command -v yum &> /dev/null; then
    yum clean all
fi

echo "QGIS已卸载"
UNINSTALL_EOF

chmod +x uninstall-qgis.sh

# 7. 创建说明文档
cat > README.md << 'README_EOF'
# QGIS离线安装包

## 包含内容
- QGIS主程序RPM包
- 所有运行时依赖包
- 自动安装/卸载脚本
- 本地RPM仓库元数据

## 安装方法

1. 解压安装包到目标目录
2. 运行安装脚本：
   ```bash
   sudo ./install-qgis.sh
   ```

## 卸载方法
```bash
sudo ./uninstall-qgis.sh
```

## 手动安装（备选）
```bash
# 配置本地仓库
sudo cp qgis-offline.repo /etc/yum.repos.d/

# 安装
sudo dnf install qgis --repo=qgis-offline
```

## 系统要求
- RHEL/CentOS/Fedora 8+
- 至少2GB可用磁盘空间
- X11图形环境（桌面版）

## 包信息
- 构建日期: $(date)
- 目标架构: x86_64
- 包数量: $(ls *.rpm 2>/dev/null | wc -l)
- 总大小: $(du -sh . 2>/dev/null | cut -f1)
README_EOF

# 8. 统计信息
echo "=== 构建统计 ==="
RPM_COUNT=$(ls *.rpm 2>/dev/null | wc -l)
TOTAL_SIZE=$(du -sh . | cut -f1)

echo "RPM包数量: $RPM_COUNT"
echo "总大小: $TOTAL_SIZE"
echo "输出目录: $(pwd)"

cd ..

echo ""
echo "=== 创建传输包 ==="
TAR_NAME="${OUTPUT_DIR}.tar.gz"
tar -czf "$TAR_NAME" "$OUTPUT_DIR"
TAR_SIZE=$(du -sh "$TAR_NAME" | cut -f1)

echo "压缩包: $TAR_NAME"
echo "压缩包大小: $TAR_SIZE"
echo ""
echo "=== 使用说明 ==="
echo "1. 传输到目标机器:"
echo "   scp $TAR_NAME user@target-server:/tmp/"
echo ""
echo "2. 在目标机器上:"
echo "   cd /tmp"
echo "   tar -xzf $TAR_NAME"
echo "   cd $OUTPUT_DIR"
echo "   sudo ./install-qgis.sh"
echo ""
echo "离线安装包创建完成!"
EOF

chmod +x collect_offline_rpms.sh
```

### 3.2 执行依赖收集

```bash
# 运行依赖收集脚本
./collect_offline_rpms.sh

# 检查输出结果
ls -la qgis-offline-*/
```

### 3.3 验证离线包完整性

```bash
# 进入创建的离线包目录
cd qgis-offline-*/

# 检查RPM包完整性
rpm -qp *.rpm 2>/dev/null | head -20

# 查看依赖关系
rpm -qpR qgis-*.rpm | head -10

# 验证仓库元数据
ls -la repodata/
```

---

## 第四阶段：传输到离线环境

### 4.1 打包和传输

```bash
# 1. 创建压缩包（如果脚本未自动创建）
tar -czf qgis-offline-$(date +%Y%m%d).tar.gz qgis-offline-*/

# 2. 传输方式选择

# 方法A: SCP传输
scp qgis-offline-*.tar.gz user@offline-server:/tmp/

# 方法B: USB拷贝
cp qgis-offline-*.tar.gz /media/usb/

# 方法C: 网络共享
# 放置在共享目录供离线机器下载
```

### 4.2 离线环境解压

```bash
# 在离线机器上
cd /tmp  # 或其他目录

# 解压安装包
tar -xzf qgis-offline-*.tar.gz

# 进入目录
cd qgis-offline-*/

# 查看内容
ls -la
cat README.md
```

---

## 第五阶段：离线环境安装

### 5.1 使用自动安装脚本

```bash
# 运行自动安装脚本（推荐）
sudo ./install-qgis.sh
```

安装脚本会自动：
- 配置本地RPM仓库
- 安装QGIS和依赖包
- 验证安装结果

### 5.2 手动安装（高级用户）

如果自动脚本失败，可以手动安装：

```bash
# 1. 手动配置仓库
sudo tee /etc/yum.repos.d/qgis-offline.repo > /dev/null << 'EOF'
[qgis-offline]
name=QGIS Offline Repository
baseurl=file:///tmp/qgis-offline-20250828
enabled=1
gpgcheck=0
priority=1
EOF

# 2. 清理缓存
sudo dnf clean all
sudo dnf makecache --repo=qgis-offline

# 3. 安装QGIS
sudo dnf install qgis --repo=qgis-offline -y

# 4. 或强制安装所有包（最后手段）
sudo rpm -ivh *.rpm --force --nodeps
```

### 5.3 验证安装

```bash
# 检查安装状态
rpm -qa | grep qgis

# 测试命令行
qgis --version

# 测试图形界面（需要X11环境）
qgis &

# 检查Python绑定
python3 -c "from qgis.core import *; print('QGIS Python绑定正常')"
```

---

## 故障排除

### 构建阶段问题

#### 1. 构建依赖缺失
```bash
# 查看具体缺失的依赖
./buildrpms.sh 2>&1 | grep -i "not found\|missing"

# 手动安装缺失依赖
sudo dnf install [缺失的包名]
```

#### 2. Mock权限问题
```bash
# 重新配置mock用户组
sudo usermod -a -G mock $USER
sudo newgrp mock

# 测试mock权限
mock --version
```

#### 3. 磁盘空间不足
```bash
# 清理构建缓存
rm -rf $HOME/qgis-build/*/build
mock --clean

# 检查磁盘空间
df -h
```

### 依赖收集问题

#### 1. 网络连接问题
```bash
# 测试网络连接
ping -c 3 download.fedoraproject.org

# 更新仓库缓存
sudo dnf makecache --refresh
```

#### 2. 依赖解析失败
```bash
# 手动下载核心依赖
sudo dnf download gdal-libs geos proj sqlite

# 检查依赖关系
rpm -qpR qgis-*.rpm | sort | uniq
```

### 安装阶段问题

#### 1. 仓库配置错误
```bash
# 检查仓库路径
cat /etc/yum.repos.d/qgis-offline.repo

# 修正baseurl路径
sudo sed -i 's|baseurl=.*|baseurl=file:///正确的路径|' /etc/yum.repos.d/qgis-offline.repo
```

#### 2. RPM包损坏
```bash
# 检查RPM包完整性
rpm -qp --checksig *.rpm

# 重新下载损坏的包
# （需要在有网络的机器上重新收集）
```

#### 3. 依赖冲突
```bash
# 查看冲突详情
sudo dnf install qgis --repo=qgis-offline --verbose

# 强制安装（不推荐，仅紧急情况）
sudo rpm -ivh qgis-*.rpm --force --replacefiles
```

---

## 高级配置

### 多架构支持

如果需要支持32位系统：

```bash
# 修改构建配置
cat > rpm/local.cfg << 'EOF'
ARCHS=("fedora-39-x86_64" "fedora-39-i386")
EOF

# 分别为每个架构创建离线包
```

### 自定义构建选项

```bash
# 修改spec文件，添加自定义选项
cp rpm/qgis.spec.template rpm/qgis.spec.custom

# 编辑自定义选项
vim rpm/qgis.spec.custom

# 使用自定义spec文件构建
mock --buildsrpm --spec qgis.spec.custom --sources ./sources
```

### 企业环境部署

```bash
# 创建企业内部仓库
# 1. 搭建内部HTTP服务器
sudo dnf install httpd
sudo systemctl start httpd

# 2. 复制RPM包到web目录
sudo cp -r qgis-offline-*/* /var/www/html/qgis/

# 3. 创建仓库元数据
sudo createrepo_c /var/www/html/qgis/

# 4. 客户端配置
cat > /etc/yum.repos.d/qgis-internal.repo << 'EOF'
[qgis-internal]
name=Internal QGIS Repository
baseurl=http://internal-server/qgis/
enabled=1
gpgcheck=0
EOF
```

---

## 最佳实践

### 1. 版本管理
- 为每个QGIS版本创建独立的离线包
- 保留构建日志和配置文件
- 使用语义化版本命名

### 2. 测试验证  
- 在隔离环境中测试离线安装
- 验证所有核心功能正常工作
- 测试Python插件加载

### 3. 文档维护
- 记录每次构建的配置变更
- 维护已知问题和解决方案清单
- 定期更新依赖包列表

### 4. 安全考虑
- 验证RPM包的数字签名
- 在受信任环境中进行构建
- 定期更新安全补丁

---

## 总结

本指南提供了完整的QGIS离线RPM包构建和部署流程：

1. **构建阶段**: 设置环境、编译源码、生成RPM包
2. **打包阶段**: 收集依赖、创建离线仓库、生成安装脚本
3. **传输阶段**: 打包压缩、传输到目标环境
4. **安装阶段**: 配置仓库、安装软件、验证功能

通过遵循本指南，你可以在完全离线的环境中成功部署QGIS，满足企业内网、科研环境等特殊场景的需求。

---

**文档版本**: 1.0  
**最后更新**: 2025-08-28  
**适用版本**: QGIS 3.x, Fedora/RHEL/CentOS 8+