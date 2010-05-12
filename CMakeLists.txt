project(okularplugin)

FIND_PACKAGE(KDE4 REQUIRED)

INCLUDE_DIRECTORIES( ${KDE4_INCLUDES} . qtbrowserplugin )

set(okularplugin_SRCS
   partwin.cpp
   qtbrowserplugin/qtbrowserplugin.cpp
   qtbrowserplugin/qtbrowserplugin_x11.cpp
 )

kde4_add_library(okularplugin SHARED ${okularplugin_SRCS})

target_link_libraries(okularplugin ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS})

########### install files ###############
install(TARGETS okularplugin DESTINATION ${KDE4_LIB_DIR} )
install( FILES okularpluginui.rc
    DESTINATION  ${DATA_INSTALL_DIR}/okularplugin )
