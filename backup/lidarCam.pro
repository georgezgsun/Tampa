#-------------------------------------------------
#
# Project created by QtCreator 2014-11-22T15:15:33
#
#-------------------------------------------------

QT       += core gui multimedia sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = lidarCam
TEMPLATE = app


SOURCES += main.cpp\
        top_view.cpp \
    main_menu.cpp \
    base_menu.cpp \
    keyboard/QKeyPushButton.cpp \
    keyboard/widgetKeyBoard.cpp \
    widgets/vki/vkiline.cpp \
    db/user_db.cpp \
    db/tbl_Location.cpp \
    menu_view.cpp \
    loc_setup.cpp \
    lidar_setup.cpp \
    mode_sel.cpp \
    sys_opt.cpp \
    user_mgr.cpp \
    dev_info.cpp \
    file_mgr.cpp \
    edit_user.cpp \
    widgets/vki/vkiedit.cpp \
    video_setup.cpp \
    wifi_setup.cpp \
    bt_setup.cpp \
    enet_setup.cpp \
    admin.cpp \
    user_lvls.cpp \
    user_perm.cpp \
    factory.cpp \
    service.cpp \
    serv_opt.cpp \
    calib_data.cpp \
    upload_mgr.cpp \
    loc_save.cpp \
    camera_setup.cpp \
    dist_measure.cpp \
    play_back.cpp \
    widgets/icon_frame.cpp \
    db/tbl_Users.cpp \
    utils.cpp \
    hardButtons.cpp \
    state.cpp \
    ./interface_src/dm355_gio_util.c \
    session.cpp \
    db/tbl_cameraSetting.cpp

HEADERS  += \
    main_menu.h \
    top_view.h \
    global.h \
    base_menu.h \
    keyboard/QKeyPushButton.h \
    keyboard/widgetKeyBoard.h \
    widgets/vki/vkiline.h \
    db/user_db.h \
    menu_view.h \
    loc_setup.h \
    lidar_setup.h \
    mode_sel.h \
    sys_opt.h \
    user_mgr.h \
    dev_info.h \
    file_mgr.h \
    edit_user.h \
    widgets/vki/vkiedit.h \
    video_setup.h \
    wifi_setup.h \
    bt_setup.h \
    enet_setup.h \
    admin.h \
    user_lvls.h \
    user_perm.h \
    factory.h \
    service.h \
    serv_opt.h \
    calib_data.h \
    upload_mgr.h \
    loc_save.h \
    camera_setup.h \
    dist_measure.h \
    play_back.h \
    db/db_types.h \
    widgets/icon_frame.h \
    utils.h \
    session.h \
    hardButtons.h \
    state.h 

FORMS    += \
    main_menu.ui \
    menu_view.ui \
    top_view.ui \
    loc_setup.ui \
    lidar_setup.ui \
    mode_sel.ui \
    sys_opt.ui \
    user_mgr.ui \
    dev_info.ui \
    file_mgr.ui \
    edit_user.ui \
    video_setup.ui \
    wifi_setup.ui \
    bt_setup.ui \
    enet_setup.ui \
    admin.ui \
    user_lvls.ui \
    user_perm.ui \
    factory.ui \
    service.ui \
    serv_opt.ui \
    calib_data.ui \
    upload_mgr.ui \
    loc_save.ui \
    camera_setup.ui \
    dist_measure.ui \
    play_back.ui \
    widgets/icon_frame.ui

INCLUDEPATH += ./keyboard ./widgets ./widgets/vki ./db ./radar ./inc ./common


TRANSLATIONS += keyboard/translations/virtualBoard_it.ts \
    keyboard/translations/virtualBoard_ru.ts \
    keyboard/translations/virtualBoard_fr.ts \
    keyboard/translations/virtualBoard_en.ts

RESOURCES += \
    resource.qrc

if (!linux-g++*) {
    QMAKE_CXXFLAGS += -DLIDARCAM
}
