#!/bin/bash
###########################################################################
#    install-qgis.sh - QGIS离线安装脚本
#    ---------------------
#    Date                 : September 2025  
#    Copyright            : (C) 2025 QGIS Project
###########################################################################

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_NAME="qgis-offline"

echo "=== QGIS离线安装脚本 ==="

# 检查权限
if [ "$EUID" -ne 0 ]; then
    echo "错误: 请使用sudo运行此脚本"
    echo "使用方法: sudo ./install-qgis.sh"
    exit 1
fi

# 检查是否有RPM包
if [ ! -f "$SCRIPT_DIR"/*.rpm ]; then
    echo "错误: 当前目录下未找到RPM包"
    exit 1
fi

echo "安装目录: $SCRIPT_DIR"
echo "RPM包数量: $(ls "$SCRIPT_DIR"/*.rpm | wc -l)"

# 创建本地仓库配置
echo "配置本地RPM仓库..."
cat > /etc/yum.repos.d/$REPO_NAME.repo << EOF
[$REPO_NAME]
name=QGIS Offline Repository
baseurl=file://$SCRIPT_DIR
enabled=1
gpgcheck=0
priority=1
EOF

echo "仓库配置已创建: /etc/yum.repos.d/$REPO_NAME.repo"

# 清理并更新缓存
if command -v dnf &> /dev/null; then
    echo "使用DNF包管理器..."
    dnf clean all
    dnf makecache --repo=$REPO_NAME
    
    echo "安装QGIS及其依赖..."
    dnf install qgis --repo=$REPO_NAME -y --skip-broken
    
elif command -v yum &> /dev/null; then
    echo "使用YUM包管理器..."
    yum clean all
    yum makecache
    
    echo "安装QGIS及其依赖..."
    yum install qgis --disablerepo="*" --enablerepo=$REPO_NAME -y
    
else
    echo "警告: 未找到dnf或yum，尝试直接安装RPM包..."
    rpm -ivh "$SCRIPT_DIR"/qgis*.rpm --force --nodeps
fi

echo "=== 安装完成 ==="

# 验证安装
if command -v qgis &> /dev/null; then
    echo "✓ QGIS已成功安装"
    
    # 尝试获取版本信息
    if qgis --version &>/dev/null; then
        echo "✓ QGIS版本: $(qgis --version 2>/dev/null | head -1)"
    fi
    
    echo ""
    echo "启动方法:"
    echo "  命令行: qgis"
    echo "  图形界面: 从应用程序菜单中找到QGIS"
    
else
    echo "⚠ 警告: QGIS命令未找到，可能安装不完整"
    echo "请检查安装日志中的错误信息"
fi

echo ""
echo "卸载方法: sudo ./uninstall-qgis.sh"