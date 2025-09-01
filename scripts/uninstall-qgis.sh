#!/bin/bash
###########################################################################
#    uninstall-qgis.sh - QGIS卸载脚本
#    ---------------------
#    Date                 : September 2025
#    Copyright            : (C) 2025 QGIS Project  
###########################################################################

set -e

REPO_NAME="qgis-offline"

echo "=== QGIS卸载脚本 ==="

# 检查权限
if [ "$EUID" -ne 0 ]; then
    echo "错误: 请使用sudo运行此脚本"
    echo "使用方法: sudo ./uninstall-qgis.sh"
    exit 1
fi

# 检查QGIS是否已安装
if ! rpm -qa | grep -q qgis; then
    echo "QGIS未安装或已经卸载"
    exit 0
fi

echo "正在卸载QGIS..."

# 卸载QGIS包
if command -v dnf &> /dev/null; then
    dnf remove qgis qgis-* -y 2>/dev/null || true
    
elif command -v yum &> /dev/null; then
    yum remove qgis qgis-* -y 2>/dev/null || true
    
else
    echo "使用rpm直接卸载..."
    rpm -qa | grep qgis | xargs rpm -e --nodeps 2>/dev/null || true
fi

# 删除仓库配置
if [ -f "/etc/yum.repos.d/$REPO_NAME.repo" ]; then
    rm -f "/etc/yum.repos.d/$REPO_NAME.repo"
    echo "已删除仓库配置文件"
fi

# 清理缓存
if command -v dnf &> /dev/null; then
    dnf clean all
elif command -v yum &> /dev/null; then
    yum clean all
fi

# 验证卸载
if rpm -qa | grep -q qgis; then
    echo "⚠ 警告: 部分QGIS组件可能未完全卸载"
    echo "剩余组件:"
    rpm -qa | grep qgis
else
    echo "✓ QGIS已完全卸载"
fi

echo "卸载完成!"