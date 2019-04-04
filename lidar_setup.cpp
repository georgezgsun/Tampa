#include "lidar_setup.h"
#include "ui_lidar_setup.h"
#include "state.h"
#include "utils.h"
#include "lidarMsg.h"
#include "Lidar_Buff.h"
#include "debug.h"

lidarSetup::lidarSetup(QWidget *parent) :
    baseMenu(parent),
    ui(new Ui::lidarSetup)
{
    ui->setupUi(this);

    initLists();
    buildHashTables();
#ifdef LIDARCAM
    // Steven Cao, 11/29/2017
    Utils& u = Utils::get();
    struct Lidar_Buff *ptr = u.lidarDataBuf();
    mpLidar = &(ptr->lidarStruct);
    memcpy((void *)&mLidar, (const void *)mpLidar, sizeof(LIDAR));
#endif
    setInittoggleValues();
    m_listIndex = m_prevListIndex = 0;
    m_command = m_cmdList.at(m_listIndex);
	state& v = state::get();
    v.setState(STATE_LIDAR_SETUP_MENU);
}

lidarSetup::~lidarSetup()
{
    Utils& u = Utils::get();
#ifdef LIDARCAM
    // Steven Cao, 11/29/2017
    int tmp1;

    // Write to shared memory if modified
    if (mLidar.DISPLAY_RESOLUTION.bits.SPEED != mpLidar->DISPLAY_RESOLUTION.bits.SPEED ||
        mLidar.DISPLAY_RESOLUTION.bits.RANGE != mpLidar->DISPLAY_RESOLUTION.bits.RANGE)
    {
        tmp1 = (int)mLidar.DISPLAY_RESOLUTION.DISPLAY_RESOLUTION_VALUE & 0x0FF;
//        printf("Update Speed/Range Tenth, %X (%X)\n", tmp1, (int)mpLidar->DISPLAY_RESOLUTION.DISPLAY_RESOLUTION_VALUE);
        u.setLidarMemory(SPEED_RANGE_TENTHS_ID, &tmp1, NULL);
    }
    if (mLidar.ANTI_JAMMING != mpLidar->ANTI_JAMMING)
    {
        tmp1 = (int)mLidar.ANTI_JAMMING & 0x0FF;
        u.setLidarMemory(ANTI_JAMMING_ID, &tmp1, NULL);
    }
    // Trigger mode
    if (mLidar.TRIGGER_MODE != mpLidar->TRIGGER_MODE)
    {
        tmp1 = (int)mLidar.TRIGGER_MODE & 0x0FF;
        u.setLidarMemory(TRIGGER_MODE_ID, &tmp1, NULL);
    }
    // UNITS
    if (mLidar.DISPLAY_UNITS != mpLidar->DISPLAY_UNITS)
    {
        tmp1 = (int)mLidar.DISPLAY_UNITS & 0x0FF;
        u.setLidarMemory(UNITS_ID, &tmp1, NULL);
    }
    // TILT
    if (mOldTiltIndex != m_tiltIndex)
    {
        switch (m_tiltIndex)
        {
            case 1:
                mLidar.TILT_ANGLE_THRESHOLD = 20;
                break;
            case 2:
                mLidar.TILT_ANGLE_THRESHOLD = 30;
                break;
            case 3:
                mLidar.TILT_ANGLE_THRESHOLD = 40;
                break;
            case 4:
                mLidar.TILT_ANGLE_THRESHOLD = 50;
                break;
            case 5:
                mLidar.TILT_ANGLE_THRESHOLD = 60;
                break;
            default:
                mLidar.TILT_ANGLE_THRESHOLD = 90;   // Off
        }

        tmp1 = (int)mLidar.TILT_ANGLE_THRESHOLD & 0x0FF;
        u.setLidarMemory(TILT_ANGLE_ID, &tmp1, NULL);
    }
#endif

    bool flag1 = false; // Assumed no modification
#ifdef HH1
    if (mConf.speedTenths != mOldSpeedTenth
	|| mConf.rangeTenths != mOldRangeTenth
    || mConf.autoTrigger != mOldAutoTrigger
    || mConf.audioAlert != mOldAudioAlert)
        flag1 = true;

    if (mConf.targetSort != m_targetSortIndex)
    {
        mConf.targetSort = m_targetSortIndex;
        flag1 = true;
    }
    if (mConf.units != m_unitsIndex)
    {
        mConf.units = m_unitsIndex;
        flag1 = true;
    }
    if (mConf.direction != m_tiltIndex)
    {
        mConf.direction = m_tiltIndex;
        flag1 = true;
    }
#endif
    if (mOldBacklightIndex != m_backlightIndex)
    {// Backlight off timer changed
        if (m_backlightIndex == 1)
            mConf.backlightOff = 5;
        else if (m_backlightIndex == 2)
            mConf.backlightOff = 10;
        else if (m_backlightIndex == 3)
            mConf.backlightOff = 15;
        else
            mConf.backlightOff = 0;
        flag1 = true;
    }
    if (mOldPowerIndex != m_powerIndex)
    {   // Power off timer changed
        if (m_powerIndex == 1)
            mConf.powerOff = 10;
        else if (m_powerIndex == 2)
            mConf.powerOff = 15;
        else if (m_powerIndex == 3)
            mConf.powerOff = 20;
        else
            mConf.powerOff = 0;
        flag1 = true;
    }

    if (flag1 == true)
        u.setConfiguration(mConf);

    delete ui;
}

void lidarSetup::initLists()
{
    m_list /*<< ui->cb_speedTenths
           << ui->cb_rangeTenths
           << ui->cb_antiJamming
           << ui->cb_autoTrigger*/
#ifdef HH1
           << ui->pb_targets
#endif
           << ui->pb_units
           << ui->pb_tilt
           << ui->pb_backlight
           << ui->pb_power;

    m_cmdList /*<< CMD_SPEED_TENTH
              << CMD_RANGE_TENTH
              << CMD_ANTI_JAM
              << CMD_AUTO_TRIGGER*/
#ifdef HH1
              << CMD_SORT_TARGETS
#endif
              << CMD_UNIT
              << CMD_TILT
              << CMD_BACKLIGHT_OFF
              << CMD_POWER_OFF;
    this->connectWidgetSigs();
}

void lidarSetup::buildHashTables()
{
    m_unitsList << "MPH" << "km/h" << "KNOTS";
#ifdef HH1
    m_tiltList << "Approaching" << "Receding" << "Both";
#else
    m_tiltList << "OFF" << "20 Degrees" << "30 Degrees" << "40 Degrees" << "50 Degrees" << "60 Degrees" ;
#endif
    m_backlightList << "ALWAYS ON" << "5 Minutes" << "10 Minutes" << "15 Minutes" ;
    m_powerList << "ALWAYS ON" << "10 Minutes" << "15 Minutes" << "20 Minutes" ;
//    m_unitsIndex = m_tiltIndex = m_backlightIndex = m_powerIndex = 0;
    m_hashValueList[CMD_UNIT] = &m_unitsList;
    m_hashValueList[CMD_TILT] = &m_tiltList;
    m_hashValueList[CMD_BACKLIGHT_OFF] = &m_backlightList;
    m_hashValueList[CMD_POWER_OFF] = &m_powerList;

    m_hashValueIndex[CMD_UNIT] = (int *)&m_unitsIndex;
    m_hashValueIndex[CMD_TILT] = (int *)&m_tiltIndex;
    m_hashValueIndex[CMD_BACKLIGHT_OFF] = (int *)&m_backlightIndex;
    m_hashValueIndex[CMD_POWER_OFF] = (int *)&m_powerIndex;

#ifdef HH1
    m_targetSortList << "SPEED" << "DISTANCE";
#endif
}

void lidarSetup::setInittoggleValues()
{
    Utils& u = Utils::get();
    mConf = u.getConfiguration();

#ifdef HH1
    //ui->cb_antiJamming->setVisible(false);

    mOldSpeedTenth = mConf.speedTenths;
    mOldRangeTenth = mConf.rangeTenths;
    mOldAutoTrigger = mConf.autoTrigger;
    mOldAudioAlert = mConf.audioAlert;
    if (mOldSpeedTenth)
        ui->cb_speedTenths->setCheckState(Qt::Checked);   // turn on
    else
        ui->cb_speedTenths->setCheckState(Qt::Unchecked); // Turn off
    if (mOldRangeTenth)
        ui->cb_rangeTenths->setCheckState(Qt::Checked);
    else
        ui->cb_rangeTenths->setCheckState(Qt::Unchecked); // Turn off
    if (mOldAutoTrigger)
        ui->cb_autoTrigger->setCheckState(Qt::Checked);
    else
        ui->cb_autoTrigger->setCheckState(Qt::Unchecked);
    if (mOldAudioAlert)
        ui->cb_antiJamming->setCheckState(Qt::Checked);
    else
        ui->cb_antiJamming->setCheckState(Qt::Unchecked);

    // Check Sort Target
    if (mConf.targetSort >= (unsigned int)m_targetSortList.size())
    {
        mConf.targetSort = m_targetSortIndex = 0;
    }
    else
        m_targetSortIndex = mConf.targetSort;

    // Check UNITS
    if (mConf.units >= (unsigned int)m_unitsList.size())
    {
        mConf.units = 0;
        m_unitsIndex = 0;
    }
    else
        m_unitsIndex = mConf.units;
    mOldUnitIndex = m_unitsIndex;

    // Check Direction
    if (mConf.direction >= (unsigned int)m_tiltList.size())
    {
        mConf.direction = 0;
        m_tiltIndex = 0;
    }
    else
        m_tiltIndex = mConf.direction;
    mOldTiltIndex = m_tiltIndex;
    ui->label_2->setText("DIRECTION");
#else
    ui->pb_targets->setVisible(false);
    ui->label_targets->setVisible(false);
    if (mLidar.DISPLAY_RESOLUTION.bits.SPEED)
        ui->cb_speedTenths->setCheckState(Qt::Checked);   // turn on
    else
        ui->cb_speedTenths->setCheckState(Qt::Unchecked); // Turn off
    if (mLidar.DISPLAY_RESOLUTION.bits.RANGE)
        ui->cb_rangeTenths->setCheckState(Qt::Checked);
    else
        ui->cb_rangeTenths->setCheckState(Qt::Unchecked); // Turn off

    printf("Speed/Range: %X, Inhibit: %X, Units: %d, TILT: %d, Backlight: %d\n", (int)mLidar.DISPLAY_RESOLUTION.DISPLAY_RESOLUTION_VALUE,
           (int)mLidar.INHIBIT_MODES.INHIBIT_MODES_VALUE, (int)mLidar.DISPLAY_UNITS, (int)mLidar.TILT_ANGLE_THRESHOLD,
           (int)mLidar.BACKLIGHT_ON);

    if (mLidar.ANTI_JAMMING)
        ui->cb_antiJamming->setCheckState(Qt::Checked);
    else
        ui->cb_antiJamming->setCheckState(Qt::Unchecked);

    if (mLidar.TRIGGER_MODE)
        ui->cb_autoTrigger->setCheckState(Qt::Checked);
    else
        ui->cb_autoTrigger->setCheckState(Qt::Unchecked);

    // Check UNITS
    if (mLidar.DISPLAY_UNITS >= m_unitsList.size())
    {
        mLidar.DISPLAY_UNITS = 0;
        m_unitsIndex = 0;
    }
    else
        m_unitsIndex = (int)mLidar.DISPLAY_UNITS & 0x0FF;

    // Check TILT
    switch (mLidar.TILT_ANGLE_THRESHOLD)
    {
        case 20:
            m_tiltIndex = 1;
            break;
        case 30:
            m_tiltIndex = 2;
            break;
        case 40:
            m_tiltIndex = 3;
            break;
        case 50:
            m_tiltIndex = 4;
            break;
        case 60:
            m_tiltIndex = 5;
            break;
        default:
            m_tiltIndex = 0;
    }
    mOldTiltIndex = m_tiltIndex;
#endif

    // Check Backlight Off
    switch (mConf.backlightOff)
    {
        case 5:
            m_backlightIndex = 1;
            break;
        case 10:
            m_backlightIndex = 2;
            break;
        case 15:
            m_backlightIndex = 3;
            break;
        default:
            m_backlightIndex = 0;
    }
    mOldBacklightIndex = m_backlightIndex;

    // Check Power Off
    switch (mConf.powerOff)
    {
        case 10:
            m_powerIndex = 1;
            break;
        case 15:
            m_powerIndex = 2;
            break;
        case 20:
            m_powerIndex = 3;
            break;
        default:
            m_powerIndex = 0;
    }
    mOldPowerIndex = m_powerIndex;

#ifdef HH1
    ui->pb_targets->setText(m_targetSortList.at(m_targetSortIndex));
#endif
    ui->pb_units->setText(m_unitsList.at(m_unitsIndex));
    ui->pb_tilt->setText(m_tiltList.at(m_tiltIndex));
    ui->pb_backlight->setText(m_backlightList.at(m_backlightIndex));
    ui->pb_power->setText(m_powerList.at(m_powerIndex));

#ifndef HH1
    // See if we allow the user to change these features
	//	DEBUG() << mLidar.FEATURE_SET_3.bits.SPEED_TENTHS << mLidar.FEATURE_SET_3.bits.RANGE_TENTHS << mLidar.MODES_1.bits.ANTI_JAM_OFF;
    if (mLidar.FEATURE_SET_3.bits.SPEED_TENTHS)
        ui->cb_speedTenths->setEnabled(false);  // No
    if (mLidar.FEATURE_SET_3.bits.RANGE_TENTHS)
        ui->cb_rangeTenths->setEnabled(false);  // No
    if (mLidar.MODES_1.bits.ANTI_JAM_OFF)
        ui->cb_antiJamming->setEnabled(false);  // No
#endif
}

void lidarSetup::toggleValue(int cmd, int idx, int /*f*/)
{
    switch (cmd)
    {
        case CMD_FOCUS_MANUAL:
            return;
        default:
            baseMenu::toggleValue(cmd, idx, 1);
            break;
    }
}

void lidarSetup::on_pb_units_clicked()
{
  if (++m_unitsIndex >= (unsigned int)m_unitsList.size())
    {
        m_unitsIndex = 0;
    }
    mLidar.DISPLAY_UNITS = m_unitsIndex;
    ui->pb_units->setText(m_unitsList.at(m_unitsIndex));
}

void lidarSetup::on_pb_tilt_clicked()
{
  if (++m_tiltIndex >= (unsigned int)m_tiltList.size())
    {
        m_tiltIndex = 0;
    }
    ui->pb_tilt->setText(m_tiltList.at(m_tiltIndex));

}

void lidarSetup::on_pb_backlight_clicked()
{
  if (++m_backlightIndex >= (unsigned int)m_backlightList.size())
    {
        m_backlightIndex = 0;
    }
    ui->pb_backlight->setText(m_backlightList.at(m_backlightIndex));
}

void lidarSetup::on_pb_power_clicked()
{
  if (++m_powerIndex >= (unsigned int)m_powerList.size())
    {
        m_powerIndex = 0;
    }
    ui->pb_power->setText(m_powerList.at(m_powerIndex));
}

void lidarSetup::on_cb_speedTenths_stateChanged(int arg1)
{
    // Steven Cao, 11/29/2017
#ifdef HH1
    if (arg1 == Qt::Checked)
       mConf.speedTenths = 1;
    else
       mConf.speedTenths = 0;
#else
    if (arg1 == Qt::Checked)
       mLidar.DISPLAY_RESOLUTION.bits.SPEED = 1;
    else
       mLidar.DISPLAY_RESOLUTION.bits.SPEED = 0;
#endif
}

void lidarSetup::on_cb_rangeTenths_stateChanged(int arg1)
{
    // Steven Cao, 11/29/2017
#ifdef HH1
    if (arg1 == Qt::Checked)
       mConf.rangeTenths = 1;
    else
       mConf.rangeTenths = 0;
#else
    if (arg1 == Qt::Checked)
       mLidar.DISPLAY_RESOLUTION.bits.RANGE = 1;
    else
       mLidar.DISPLAY_RESOLUTION.bits.RANGE = 0;
#endif
}

void lidarSetup::on_cb_antiJamming_stateChanged(int arg1)
{
#ifdef HH1
    //In hh1 anti_jamming box is used for audio alert.
    if (arg1 == Qt::Checked)
       mConf.audioAlert = 1;
    else
       mConf.audioAlert = 0;
#else
    // Steven Cao, 11/29/2017
    if (arg1 == Qt::Checked)
       mLidar.ANTI_JAMMING = 1;
    else
       mLidar.ANTI_JAMMING = 0;
#endif
}

void lidarSetup::on_cb_autoTrigger_stateChanged(int arg1)
{
#ifdef HH1
    if (arg1 == Qt::Checked)
       mConf.autoTrigger = 1;
    else
       mConf.autoTrigger = 0;
#else
   if (arg1 == Qt::Checked)
   {  // Auto trigger mode
       mLidar.TRIGGER_MODE = 1;
   }
   else
   {
       mLidar.TRIGGER_MODE = 0;
   }
#endif
}

void lidarSetup::on_pb_targets_clicked()
{
#ifdef HH1
  if (++m_targetSortIndex >= (unsigned int)m_targetSortList.size())
    {
        m_targetSortIndex = 0;
    }
    ui->pb_targets->setText(m_targetSortList.at(m_targetSortIndex));
#endif
}
