#-------------------------------------------------
#
# Project created by QtCreator 2014-11-22T15:15:33
#
#-------------------------------------------------
QT += core gui multimedia sql  printsupport multimediawidgets

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
    security.cpp \
    user_access.cpp \
    factory.cpp \
    service.cpp \
    serv_opt.cpp \
    calib_data.cpp \
    upload_mgr.cpp \
    loc_save.cpp \
    camera_setup.cpp \
    dist_measure.cpp \
    play_back.cpp \
    db/tbl_Users.cpp \
    utils.cpp \
    hardButtons.cpp \
    state.cpp \
    ../kernel/lidarcamevm/ipnc_app/interface/src/dm355_gio_util.c \
    session.cpp \
    db/tbl_cameraSetting.cpp \
    db/tbl_Evidence.cpp \
    log_file.cpp \
    password.cpp \
    db/tbl_Configuration.cpp \
    db/tbl_Admin.cpp \
    db/tbl_Network.cpp \
    back_ground.cpp \
    illuminator.cpp \
    operat_frames.cpp \
    widgets/icon_frame/icon_frame.cpp \
    calibrateMag3110.cpp \
    printTicket.cpp \
    radarParams.cpp \
    RadarInterface/CoordTransforms.cpp \
    RadarInterface/Crc.cpp \
    RadarInterface/RadarData.cpp \
    RadarInterface/Serial.cpp \
    TiltSensor/smbus.cpp \
    TiltSensor/TiltSensor.cpp \
    selfTest.cpp \
    db/tbl_Sensor.cpp \
    tiltParams.cpp \
    hh1MetaData.cpp \
    focus.cpp

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
    security.h \
    user_access.h \
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
    utils.h \
    session.h \
    hardButtons.h \
    state.h \
    log_file.h \
    password.h \
    back_ground.h \
    illuminator.h \
    operat_frames.h \
    widgets/icon_frame/icon_frame.h \
    calibrateMag3110.h \
    printTicket.h \
    radarParams.h \
    RadarInterface/CoordTransforms.h \
    RadarInterface/Crc.h \
    RadarInterface/RadarData.h \
    RadarInterface/Serial.h \
    TiltSensor/smbus.h \
    TiltSensor/TiltSensor.h \
    selfTest.h \
    tiltRarams.h \
    tiltParams.h \
    hh1MetaData.h \
    focus.h

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
    security.ui \
    user_access.ui \
    factory.ui \
    service.ui \
    serv_opt.ui \
    calib_data.ui \
    upload_mgr.ui \
    loc_save.ui \
    camera_setup.ui \
    dist_measure.ui \
    play_back.ui \
    log_file.ui \
    password.ui \
    illuminator.ui \
    operat_frames.ui \
    widgets/icon_frame/icon_frame.ui \
    calibrateMag3110.ui \
    selfTest.ui \
    tiltParams.ui \
    radarParams.ui \
    focus.ui

INCLUDEPATH += ./keyboard ./widgets ./widgets/vki ./db ./radar $$PWD/../kernel/lidarcamevm/ipnc_app/interface/inc $$PWD/../common ./RadarInterface ./TiltSensor


TRANSLATIONS += keyboard/translations/virtualBoard_it.ts \
    keyboard/translations/virtualBoard_ru.ts \
    keyboard/translations/virtualBoard_fr.ts \
    keyboard/translations/virtualBoard_en.ts

RESOURCES += \
    resource.qrc

QMAKE_CXXFLAGS += -DHH1

if (!linux-g++*) {
    QMAKE_CXXFLAGS += -DLIDARCAM
}
