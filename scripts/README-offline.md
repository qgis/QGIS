# QGIS离线安装包

## 包含内容
- QGIS主程序RPM包及相关组件
- 运行时依赖包
- 自动安装/卸载脚本
- 本地RPM仓库元数据

## 系统要求
- RHEL/CentOS/Fedora 8+
- x86_64架构
- 至少2GB可用磁盘空间
- root权限（用于安装）

## 安装方法

### 自动安装（推荐）
```bash
sudo ./install-qgis.sh
```

### 手动安装（备选）
```bash
# 1. 配置本地仓库
sudo cp qgis-offline.repo /etc/yum.repos.d/

# 2. 安装
sudo dnf install qgis --repo=qgis-offline

# 或使用yum
sudo yum install qgis --disablerepo="*" --enablerepo=qgis-offline
```

## 卸载方法
```bash
sudo ./uninstall-qgis.sh
```

## 启动QGIS
```bash
# 命令行启动
qgis

# 或从应用程序菜单中找到QGIS
```

## 故障排除

### 权限问题
确保使用sudo运行安装脚本

### 依赖冲突
```bash
sudo dnf install qgis --repo=qgis-offline --skip-broken
```

### 仓库配置问题
检查 `/etc/yum.repos.d/qgis-offline.repo` 文件中的路径是否正确

## 技术信息
- 构建日期: $(date)
- 目标架构: x86_64
- 包管理器: DNF/YUM