#!/bin/bash
###########################################################################
#    create_offline_rpm.sh
#    ---------------------
#    Date                 : September 2025
#    Copyright            : (C) 2025 QGIS Project
#    Email                : qgis-developer@lists.osgeo.org
###########################################################################
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
###########################################################################

set -e

function print_info() {
    echo -e "\e[0;32m$1\033[0m"
}

function print_error() {
    echo -e "\033[0;31m$1\033[0m"
}

function print_help() {
    echo '
创建QGIS离线RPM安装包

用法:
  -o DIR      输出目录 (默认: qgis-offline-rpm)
  -r RPMS     QGIS RPM包路径 (默认: ~/rpmbuild/RPMS/x86_64/)
  -d          仅下载依赖，不创建仓库
  -h          显示帮助

示例:
  ./create_offline_rpm.sh                    # 使用默认设置
  ./create_offline_rpm.sh -o /tmp/qgis-pkg  # 指定输出目录
  ./create_offline_rpm.sh -d                # 仅下载依赖
'
}

# 默认配置
OUTPUT_DIR="qgis-offline-rpm"
RPM_SOURCE="$HOME/rpmbuild/RPMS/x86_64"
DOWNLOAD_ONLY=0

while getopts "o:r:dh" opt; do
    case ${opt} in
        o)
            OUTPUT_DIR="$OPTARG"
            ;;
        r)
            RPM_SOURCE="$OPTARG"
            ;;
        d)
            DOWNLOAD_ONLY=1
            ;;
        h)
            print_help
            exit 0
            ;;
        \?)
            print_error "无效选项: -$OPTARG"
            print_help
            exit 1
            ;;
    esac
done

print_info "创建QGIS离线RPM安装包..."

# 检查是否存在构建的RPM包
if [ ! -d "$RPM_SOURCE" ]; then
    print_error "RPM源目录不存在: $RPM_SOURCE"
    print_error "请先构建QGIS RPM包"
    exit 1
fi

QGIS_RPMS=$(find "$RPM_SOURCE" -name "qgis-*.rpm" -type f)
if [ -z "$QGIS_RPMS" ]; then
    print_error "在 $RPM_SOURCE 中未找到QGIS RPM包"
    exit 1
fi

print_info "找到QGIS RPM包:"
echo "$QGIS_RPMS"

# 创建输出目录
mkdir -p "$OUTPUT_DIR"

# 复制QGIS RPM包
print_info "复制QGIS RPM包到 $OUTPUT_DIR..."
cp $QGIS_RPMS "$OUTPUT_DIR/"

cd "$OUTPUT_DIR"

# 获取QGIS包名
QGIS_PKG=$(basename $(echo $QGIS_RPMS | head -n1))

print_info "下载依赖包..."

# 方法1: 使用dnf download
if command -v dnf &> /dev/null; then
    print_info "使用dnf下载依赖..."
    dnf download --resolve --alldeps --skip-broken "$QGIS_PKG" 2>/dev/null || {
        print_info "dnf download失败，尝试使用repoquery..."
        
        # 方法2: 使用repoquery获取依赖列表
        DEPS=$(dnf repoquery --requires --resolve "$QGIS_PKG" 2>/dev/null | grep -v "^$QGIS_PKG" | sort -u)
        if [ -n "$DEPS" ]; then
            echo "$DEPS" | xargs dnf download --skip-broken 2>/dev/null || print_error "部分依赖下载失败"
        fi
    }
elif command -v yum &> /dev/null; then
    print_info "使用yum下载依赖..."
    yumdownloader --resolve "$QGIS_PKG" 2>/dev/null || {
        print_error "yum下载失败，请手动安装yum-utils包"
    }
else
    print_error "未找到dnf或yum命令"
    exit 1
fi

if [ "$DOWNLOAD_ONLY" -eq 1 ]; then
    print_info "仅下载模式，完成。"
    exit 0
fi

# 创建本地RPM仓库
print_info "创建本地RPM仓库..."
if command -v createrepo_c &> /dev/null; then
    createrepo_c .
elif command -v createrepo &> /dev/null; then
    createrepo .
else
    print_error "未找到createrepo命令，安装createrepo_c或createrepo"
    print_info "运行: sudo dnf install createrepo_c"
    exit 1
fi

# 创建安装脚本
print_info "创建安装脚本..."
cat > install-qgis-offline.sh << 'EOF'
#!/bin/bash
# QGIS离线安装脚本

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_NAME="qgis-offline"

echo "安装QGIS离线包..."

# 创建本地仓库配置
sudo tee /etc/yum.repos.d/${REPO_NAME}.repo > /dev/null << EOL
[${REPO_NAME}]
name=QGIS Offline Repository
baseurl=file://${SCRIPT_DIR}
enabled=1
gpgcheck=0
priority=1
EOL

echo "本地仓库已配置: /etc/yum.repos.d/${REPO_NAME}.repo"

# 清理缓存
if command -v dnf &> /dev/null; then
    sudo dnf clean all
    sudo dnf makecache --repo=${REPO_NAME}
    
    echo "安装QGIS..."
    sudo dnf install qgis --repo=${REPO_NAME} -y
elif command -v yum &> /dev/null; then
    sudo yum clean all
    sudo yum makecache
    
    echo "安装QGIS..."
    sudo yum install qgis --disablerepo="*" --enablerepo=${REPO_NAME} -y
fi

echo "QGIS安装完成！"
echo "可以运行 'qgis' 命令启动程序"
EOF

chmod +x install-qgis-offline.sh

# 创建卸载脚本
cat > uninstall-qgis-offline.sh << 'EOF'
#!/bin/bash
# QGIS离线卸载脚本

REPO_NAME="qgis-offline"

echo "卸载QGIS..."

if command -v dnf &> /dev/null; then
    sudo dnf remove qgis -y
elif command -v yum &> /dev/null; then
    sudo yum remove qgis -y
fi

# 删除本地仓库配置
if [ -f "/etc/yum.repos.d/${REPO_NAME}.repo" ]; then
    sudo rm -f "/etc/yum.repos.d/${REPO_NAME}.repo"
    echo "已删除仓库配置文件"
fi

echo "QGIS卸载完成！"
EOF

chmod +x uninstall-qgis-offline.sh

# 统计信息
cd ..
TOTAL_SIZE=$(du -sh "$OUTPUT_DIR" | cut -f1)
RPM_COUNT=$(find "$OUTPUT_DIR" -name "*.rpm" | wc -l)

print_info "离线安装包创建完成！"
print_info "输出目录: $OUTPUT_DIR"
print_info "包含RPM数量: $RPM_COUNT"
print_info "总大小: $TOTAL_SIZE"
print_info ""
print_info "传输到离线环境后，运行以下命令安装:"
print_info "  cd $OUTPUT_DIR"
print_info "  sudo ./install-qgis-offline.sh"
print_info ""
print_info "卸载命令:"
print_info "  sudo ./uninstall-qgis-offline.sh"

# 可选：创建tar包便于传输
read -p "是否创建tar包便于传输? (y/N): " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    TAR_NAME="${OUTPUT_DIR}-$(date +%Y%m%d).tar.gz"
    print_info "创建压缩包: $TAR_NAME"
    tar -czf "$TAR_NAME" "$OUTPUT_DIR"
    TAR_SIZE=$(du -sh "$TAR_NAME" | cut -f1)
    print_info "压缩包大小: $TAR_SIZE"
    print_info "可以使用以下命令传输:"
    print_info "  scp $TAR_NAME user@offline-server:/tmp/"
fi