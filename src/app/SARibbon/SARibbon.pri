#-------------------------------------------------
#
# 此文件给SARibbon静态编译使用
# 适用于SARibbon.h/SARibbon.cpp这两个文件
# 如果使用动态库，不要使用此文件
#
#
# 使用方法见例子：src\example\SimpleExample\
#
#-------------------------------------------------


######################################
# Config  |  配置
######################################
#SA_RIBBON_CONFIG 用于定义一些编译选项：
# SA_RIBBON_CONFIG+=use_frameless
#     此选项将使用frameless第三方库，这个选项在SARibbonBar.pri中会自动判断，如果，达到frameless的使用要求将会自动定义
#     frameless第三方库必须C++17且只有几个版本的qt可用，目前支持（qt5.14,qt5.15,qt6.4以上）
#     除了上诉版本SA_RIBBON_CONFIG中不会加入use_frameless
#     frameless库能实现Ubuntu下和mac下的显示，同时多屏幕的支持也较好
# 使用frameless库，需要定义QWindowKit的安装目录，默认在SARIBBON_BIN_DIR
# SA_RIBBON_QWindowKit_Install_DIR = $$SARIBBON_BIN_DIR
######################################
# 集成模式默认不使用frameless，如果使用，需要自己引入qwk依赖
SA_RIBBON_CONFIG -= use_frameless


# 这里判断SA_RIBBON_CONFIG是否包含use_frameless，如果包含将引入frameless库，并定义SARIBBON_USE_3RDPARTY_FRAMELESSHELPER为1
contains( SA_RIBBON_CONFIG, use_frameless ) {
    !contains(CONFIG,C++17){
        CONFIG += c++17
    }
    # 定义SARIBBON_USE_3RDPARTY_FRAMELESSHELPER为1
    DEFINES += SARIBBON_USE_3RDPARTY_FRAMELESSHELPER=1
}else{
    DEFINES += SARIBBON_USE_3RDPARTY_FRAMELESSHELPER=0
}


SOURCES += \
    $$PWD/SARibbon.cpp
    
HEADERS  += \
    $$PWD/SARibbon.h
    
    
INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD
