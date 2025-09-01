#!/bin/bash
###########################################################################
#    collect_offline_rpms.sh - QGIS离线RPM包收集脚本
#    ---------------------
#    Date                 : September 2025
#    Copyright            : (C) 2025 QGIS Project
###########################################################################

set -e

# 配置变量
BUILD_DIR="result"  # 默认构建输出目录
OUTPUT_DIR="qgis-offline-$(date +%Y%m%d)"

echo "=== 创建QGIS离线RPM包 ==="

# 检查构建输出目录
if [ ! -d "$BUILD_DIR" ]; then
    # 尝试查找其他可能的构建位置
    for possible_dir in "../rpm/result" "$HOME/rpmbuild" "rpm/result" "$PWD/result"; do
        if [ -d "$possible_dir" ]; then
            BUILD_DIR="$possible_dir"
            break
        fi
    done
fi

# 自动检测架构目录
ARCH_DIR=""
if [ -d "$BUILD_DIR" ]; then
    # 查找fedora-*-x86_64目录
    for arch_candidate in "$BUILD_DIR"/fedora-*-x86_64; do
        if [ -d "$arch_candidate" ]; then
            ARCH_DIR="$arch_candidate"
            ARCH=$(basename "$arch_candidate")
            echo "检测到架构: $ARCH"
            break
        fi
    done
fi

# 创建输出目录
mkdir -p "$OUTPUT_DIR"
cd "$OUTPUT_DIR"

echo "1. 查找QGIS RPM包..."
# 查找QGIS RPM包的多种可能位置
QGIS_RPMS=""

# 优先使用检测到的架构目录
if [ -n "$ARCH_DIR" ]; then
    found_rpms=$(find "$ARCH_DIR" -name "qgis-*.rpm" -type f 2>/dev/null | head -5)
    if [ -n "$found_rpms" ]; then
        QGIS_RPMS="$found_rpms"
        echo "在 $ARCH_DIR 找到QGIS RPM包"
    fi
fi

# 如果未找到，尝试其他位置
if [ -z "$QGIS_RPMS" ]; then
    for search_path in \
        "$BUILD_DIR"/*-x86_64 \
        "$BUILD_DIR/RPMS/x86_64" \
        "../rpm/result"/*-x86_64 \
        "rpm/result"/*-x86_64 \
        "$HOME/rpmbuild/RPMS/x86_64" \
        "."; do
        
        if [ -d "$search_path" ]; then
            found_rpms=$(find "$search_path" -name "qgis-*.rpm" -type f 2>/dev/null | head -5)
            if [ -n "$found_rpms" ]; then
                QGIS_RPMS="$found_rpms"
                echo "在 $search_path 找到QGIS RPM包"
                break
            fi
        fi
    done
fi

if [ -z "$QGIS_RPMS" ]; then
    echo "错误: 未找到QGIS RPM包"
    echo "请确保已经构建了QGIS RPM包，或指定正确的构建目录"
    exit 1
fi

echo "找到的RPM包:"
echo "$QGIS_RPMS"

# 复制QGIS RPM包
echo "2. 复制QGIS RPM包..."
for rpm in $QGIS_RPMS; do
    cp "$rpm" .
done

# 获取主包名
MAIN_RPM=$(ls qgis-[0-9]*.rpm | head -1)
if [ -z "$MAIN_RPM" ]; then
    MAIN_RPM=$(ls qgis*.rpm | head -1)
fi
echo "主包: $MAIN_RPM"

echo "3. 下载运行时依赖..."

# 使用dnf或yum下载依赖
if command -v dnf &> /dev/null; then
    echo "使用dnf下载依赖..."
    
    # 尝试下载解析的依赖
    dnf download --resolve --alldeps --skip-broken "$MAIN_RPM" 2>/dev/null || true
    
    # 手动下载核心依赖
    echo "下载核心依赖包..."
    CORE_DEPS=(
        "gdal-libs" "geos" "proj" "sqlite" "spatialite"
        "python3" "python3-libs" "python3-numpy"
        "qt5-qtbase" "qt5-qtsvg" "qt5-qtwebkit"
        "postgresql-libs" "netcdf" "hdf5"
        "protobuf" "expat" "libzip"
    )
    
    for dep in "${CORE_DEPS[@]}"; do
        dnf download "$dep" 2>/dev/null || echo "跳过: $dep"
    done
    
elif command -v yum &> /dev/null; then
    echo "使用yum下载依赖..."
    yumdownloader --resolve "$MAIN_RPM" 2>/dev/null || true
    
    # 手动下载核心依赖
    for dep in gdal-libs geos proj sqlite spatialite python3 qt5-qtbase; do
        yumdownloader "$dep" 2>/dev/null || echo "跳过: $dep"
    done
else
    echo "警告: 未找到dnf或yum命令，仅复制QGIS包"
fi

echo "4. 创建RPM仓库..."
if command -v createrepo_c &> /dev/null; then
    createrepo_c .
elif command -v createrepo &> /dev/null; then
    createrepo .
else
    echo "警告: 未找到createrepo命令，请手动安装createrepo_c"
fi

echo "5. 创建安装和卸载脚本..."
# 这些脚本将在下一步单独创建

# 统计信息
RPM_COUNT=$(ls *.rpm 2>/dev/null | wc -l)
TOTAL_SIZE=$(du -sh . | cut -f1)

echo "=== 完成统计 ==="
echo "RPM包数量: $RPM_COUNT"
echo "总大小: $TOTAL_SIZE"
echo "输出目录: $(pwd)"

cd ..

echo "=== 创建传输包 ==="
TAR_NAME="${OUTPUT_DIR}.tar.gz"
tar -czf "$TAR_NAME" "$OUTPUT_DIR"
TAR_SIZE=$(du -sh "$TAR_NAME" | cut -f1)

echo "压缩包: $TAR_NAME"
echo "压缩包大小: $TAR_SIZE"
echo ""
echo "离线安装包创建完成!"
echo "接下来运行: cp install-qgis.sh uninstall-qgis.sh $OUTPUT_DIR/"