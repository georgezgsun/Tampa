#include "mode_sel.h"
#include "ui_mode_sel.h"
#include "state.h"
#include "utils.h"
#include "lidarMsg.h"
#include "Lidar_Buff.h"
#include "ColdFireCommands.h"

//#define LIDAR_DEBUG

modeSel::modeSel(QWidget *parent) :
    baseMenu(parent),
    ui(new Ui::modeSel)
{
    ui->setupUi(this);

    this->initLists();
    this->buildHashTables();
#ifdef LIDARCAM
    // Steven Cao, 11/29/2017
    Utils& u = Utils::get();
//    u.getLidarMemory();
    struct Lidar_Buff *ptr = u.lidarDataBuf();
    mpLidar = &(ptr->lidarStruct);
    memcpy((void *)&mLidar, (const void *)mpLidar, sizeof(LIDAR));
#endif
    this->setInittoggleValues();

	state& v = state::get();
    v.setState(STATE_MODE_SEL_MENU);
    m_listIndex = m_prevListIndex = 0;
    m_command = m_cmdList.at(m_listIndex);

    connect(ui->cb_zone, SIGNAL(stateChanged(int)), this, SLOT(setCmd()));
    connect(ui->cb_autoObs, SIGNAL(stateChanged(int)), this, SLOT(setCmd()));
    connect(ui->cb_ftc, SIGNAL(stateChanged(int)), this, SLOT(setCmd()));
}

modeSel::~modeSel()
{
#ifdef LIDARCAM
    // Steven Cao, 11/29/2017
    Utils& u = Utils::get();
    int tmp1;

    // Write to shared memory if modified
    if (mLidar.SPEED_RANGE_DISPLAY != mpLidar->SPEED_RANGE_DISPLAY)
    {
        tmp1 = (int)mLidar.SPEED_RANGE_DISPLAY;
        printf("Update Mode Range, %X (%X)\n", tmp1, (int)mpLidar->SPEED_RANGE_DISPLAY);
        u.setLidarMemory(SPEED_RANGE_DISPLAY_ID, &tmp1, NULL);
    }
#endif
    delete ui;
}

void modeSel::initLists()
{
    m_list << ui->cb_range
           << ui->cb_autoObs
           << ui->cb_zone
           << ui->cb_incWx
           << ui->cb_singleShot
           << ui->cb_ftc
           << ui->cb_logChase
           << ui->cb_logStats;

    m_cmdList << CMD_MODE_RANGE
              << CMD_MODE_AUTO
              << CMD_MODE_ZONE
              << CMD_MODE_INCL
              << CMD_MODE_SS
              << CMD_MODE_FTC
              << CMD_MODE_LOG_CHASE
              << CMD_MODE_LOG_STATS;

    this->connectWidgetSigs();
}

void modeSel::buildHashTables()
{
}

void modeSel::setInittoggleValues()
{
#ifdef LIDAR_DEBUG
    // this line is debug only, need to remove later. It enables all checkboxes
    mLidar.MODES_1.MODES_1_VALUE = 0xF7;
    mLidar.SPECIAL_MODES.bits.FORCE_R_DISPLAY = 0;
#endif
    Utils& u = Utils::get();
    int operMode = u.getMode();

    printf("Inhibits: %X; Special: %X; Modes_1: %X, Zone: %d; Rang Display: %d\n", (unsigned int)mLidar.INHIBIT_MODES.INHIBIT_MODES_VALUE,
           (unsigned int)mLidar.SPECIAL_MODES.SPECIAL_MODES_VALUE, (unsigned int)mLidar.MODES_1.MODES_1_VALUE,
           mLidar.MODES_1.bits.ZONE_bit, (int)mLidar.SPEED_RANGE_DISPLAY);

//    mLidar.SPEED_RANGE_DISPLAY = 0;
    // Check Range/Speed configuration
    if (mLidar.SPECIAL_MODES.bits.FORCE_R_DISPLAY)
    {   // Force Range only
        if (mLidar.SPEED_RANGE_DISPLAY <= 2)
            mLidar.SPEED_RANGE_DISPLAY = 2; // Continuous
        else
            mLidar.SPEED_RANGE_DISPLAY = 5; // Single mode
        ui->cb_range->setChecked(true);
        ui->cb_range->setEnabled(false);
    }
    else
    {   // Can be one of these following two
        if (mLidar.SPEED_RANGE_DISPLAY == 2 || mLidar.SPEED_RANGE_DISPLAY == 5)
            ui->cb_range->setChecked(true); // Range only
        else
            ui->cb_range->setChecked(false); // Range and Speed
    }

    // Zone mode allowed?
    if (!mLidar.MODES_1.bits.ZONE_bit)
        ui->cb_zone->setEnabled(false);     // No
    else
    {   // Yes
        if (operMode == CMD_MODE_ZONE)
            ui->cb_zone->setChecked(true);
    }

    // Auto OBS allowed?
    if (!mLidar.MODES_1.bits.AUTO_OBSTRUCTION_bit)
        ui->cb_autoObs->setEnabled(false);  // No
    else
    {   // Yes
        if (operMode == CMD_MODE_AUTO)
            ui->cb_autoObs->setChecked(true);
    }

    // Inclement weather mode allowed?
    if (mLidar.INHIBIT_MODES.bits.INCLEMENT_WEATHER)
        ui->cb_incWx->setEnabled(false);    // No
    else
    {   // Yes
        if (operMode == CMD_MODE_INCL)
            ui->cb_incWx->setChecked(true);
    }

    // Check Single Shot mode
    if (mLidar.INHIBIT_MODES.bits.SINGLE_SHOT)
    {   // Inhibit Single Shot
        ui->cb_singleShot->setEnabled(false);
        // Continuous only
        if (mLidar.SPEED_RANGE_DISPLAY == 3)
            mLidar.SPEED_RANGE_DISPLAY = 0; // Speed and Range
        else if (mLidar.SPEED_RANGE_DISPLAY == 5)
            mLidar.SPEED_RANGE_DISPLAY = 2; // Range only
    }
    else
    {   // Can be either
        if (mLidar.SPEED_RANGE_DISPLAY < 3)
            ui->cb_singleShot->setChecked(false); // Continuous
        else
            ui->cb_singleShot->setChecked(true);   // Single Shot
    }

    // FTC mode allowed?
    if (!mLidar.MODES_1.bits.FTC_bit)
        ui->cb_ftc->setEnabled(false);      // No

    // Data logging allowed?
    if (!mLidar.MODES_1.bits.DATA_LOGGING_bit)
    {
        ui->cb_logChase->setEnabled(false);     // No
        ui->cb_logStats->setEnabled(false);     // No
    }

    // Ticket 21465: temporary to do this, Steven Cao, 8/31/2018
    ui->cb_logChase->setEnabled(false);     // No
    ui->cb_logStats->setEnabled(false);     // No
}

void modeSel::toggleValue(int cmd, int idx, int /*f*/)
{
    switch (cmd) {
//    case CMD_MIN:
//    case CMD_MAX:
//        return;
    default:
        baseMenu::toggleValue(cmd, idx);
    }
}

void modeSel::enterOperMode(int mode1, bool status)
{
  UNUSED( mode1 );
  UNUSED( status );

#ifdef LIDARCAM
   Utils& u = Utils::get();
   // Write to shared memory if modified
   if (mLidar.SPEED_RANGE_DISPLAY != mpLidar->SPEED_RANGE_DISPLAY)
   {
      int tmp1 = (int)mLidar.SPEED_RANGE_DISPLAY;
      printf("Update Mode Range, %X (%X)\n", tmp1, (int)mpLidar->SPEED_RANGE_DISPLAY);
      u.setLidarMemory(SPEED_RANGE_DISPLAY_ID, &tmp1, NULL);
   }
   if (mode1 == CMD_MODE_ZONE)
   {   // Zone mode
      if (status == true)
         u.sendMbPacket((unsigned char)CMD_ZONE_DIST1, 0, NULL, NULL);  // Enter Zone mode
      else
         u.sendMbPacket((unsigned char)CMD_ZONE_EXIT, 0, NULL, NULL);   // Exit Zone mode
   }
   else if (mode1 == CMD_MODE_AUTO)
   {
      if (status == true)
         u.sendMbPacket((unsigned char)CMD_OBS_LEARN, 0, NULL, NULL);   // Enter Auto OBS mode
      else
         u.sendMbPacket((unsigned char)CMD_OBS_EXIT, 0, NULL, NULL);    // Exit Auto OBS mode
   }
   else if (mode1 == CMD_MODE_FTC)
   {
      if (status == true)
         u.sendMbPacket((unsigned char)CMD_FTC_LANE_DIST, 0, NULL, NULL);  // Enter FTC mode
      else
         u.sendMbPacket((unsigned char)CMD_FTC_EXIT, 0, NULL, NULL);    // Exit FTC mode
   }
#endif
}

void modeSel::on_cb_range_stateChanged(int arg1)
{
    if (arg1 == Qt::Checked)
    {   // Range only
        if (!mLidar.SPEED_RANGE_DISPLAY)
            mLidar.SPEED_RANGE_DISPLAY = 2; // Continuous
        else if (mLidar.SPEED_RANGE_DISPLAY == 3)
            mLidar.SPEED_RANGE_DISPLAY = 5; // Single shot
    }
    else
    {   // Speed and Range
        if (mLidar.SPEED_RANGE_DISPLAY == 2)
            mLidar.SPEED_RANGE_DISPLAY = 0; // Continuous
        else if (mLidar.SPEED_RANGE_DISPLAY == 5)
            mLidar.SPEED_RANGE_DISPLAY = 3; // Single shot
    }
}

void modeSel::on_cb_autoObs_stateChanged(int arg1)
{
    if (arg1 == Qt::Checked)
    {   // Enter Auto OBS mode
        m_command = CMD_MODE_AUTO;
        if (mLidar.MODES_1.bits.ZONE_bit)
            ui->cb_zone->setEnabled(false);      // disable
        if (!mLidar.INHIBIT_MODES.bits.INCLEMENT_WEATHER)
            ui->cb_incWx->setEnabled(false);  // disable
        if (!mLidar.SPECIAL_MODES.bits.FORCE_R_DISPLAY)
        {
            ui->cb_range->setChecked(false);
            ui->cb_range->setEnabled(false);  // disable
            if (mLidar.SPEED_RANGE_DISPLAY == 2)
                mLidar.SPEED_RANGE_DISPLAY = 0;
            else if (mLidar.SPEED_RANGE_DISPLAY == 5)
                mLidar.SPEED_RANGE_DISPLAY = 3;
        }
        enterOperMode(CMD_MODE_AUTO, true);
    }
    else
    {   // Exit Auto OBS mode
       enterOperMode(CMD_MODE_AUTO, false);
        m_command = CMD_NONE;
        if (mLidar.MODES_1.bits.ZONE_bit)
            ui->cb_zone->setEnabled(true);       // enable
        if (!mLidar.INHIBIT_MODES.bits.INCLEMENT_WEATHER)
            ui->cb_incWx->setEnabled(true);  // enable
        if (!mLidar.SPECIAL_MODES.bits.FORCE_R_DISPLAY)
            ui->cb_range->setEnabled(true);  // enable
    }
    Utils::get().setMode(m_command);
}

void modeSel::on_cb_zone_stateChanged(int arg1)
{
    Utils& u = Utils::get();
    if (arg1 == Qt::Checked)
    {   // Enter Zone mode
        m_command = CMD_MODE_ZONE;
        if (!mLidar.INHIBIT_MODES.bits.INCLEMENT_WEATHER)
            ui->cb_incWx->setEnabled(false);    // disable
        if (mLidar.MODES_1.bits.AUTO_OBSTRUCTION_bit)
            ui->cb_autoObs->setEnabled(false);  // disable
        if (!mLidar.SPECIAL_MODES.bits.FORCE_R_DISPLAY)
        {
            ui->cb_range->setChecked(false);
            ui->cb_range->setEnabled(false);  // disable
            if (mLidar.SPEED_RANGE_DISPLAY == 2)
                mLidar.SPEED_RANGE_DISPLAY = 0;
            else if (mLidar.SPEED_RANGE_DISPLAY == 5)
                mLidar.SPEED_RANGE_DISPLAY = 3;
        }
        enterOperMode(CMD_MODE_ZONE, true);
    }
    else
    {   // Exit Zone mode
        enterOperMode(CMD_MODE_ZONE, false);
        m_command = CMD_NONE;
        if (!mLidar.INHIBIT_MODES.bits.INCLEMENT_WEATHER)
            ui->cb_incWx->setEnabled(true);     // enable
        if (mLidar.MODES_1.bits.AUTO_OBSTRUCTION_bit)
            ui->cb_autoObs->setEnabled(true);   // enable
        if (!mLidar.SPECIAL_MODES.bits.FORCE_R_DISPLAY)
            ui->cb_range->setEnabled(true);  // enable
    }
    u.setMode(m_command);
}

void modeSel::on_cb_incWx_stateChanged(int arg1)
{
    if (arg1 == Qt::Checked)
    {   // Enter Inclement weather mode
        m_command = CMD_MODE_INCL;
        if (mLidar.MODES_1.bits.ZONE_bit)
            ui->cb_zone->setEnabled(false);      // disable
        if (mLidar.MODES_1.bits.AUTO_OBSTRUCTION_bit)
            ui->cb_autoObs->setEnabled(false);  // disable
        if (!mLidar.SPECIAL_MODES.bits.FORCE_R_DISPLAY)
        {
            ui->cb_range->setChecked(false);
            ui->cb_range->setEnabled(false);  // disable
            if (mLidar.SPEED_RANGE_DISPLAY == 2)
                mLidar.SPEED_RANGE_DISPLAY = 0;
            else if (mLidar.SPEED_RANGE_DISPLAY == 5)
                mLidar.SPEED_RANGE_DISPLAY = 3;
        }
#ifdef LIDARCAM
        Utils& u = Utils::get();
        u.sendMbPacket((unsigned char)CMD_INC_MODE_ON, 0, NULL, NULL);   // Enter INC mode
#endif
    }
    else
    {
        m_command = CMD_NONE;
        if (mLidar.MODES_1.bits.ZONE_bit)
            ui->cb_zone->setEnabled(true);       // enable
        if (mLidar.MODES_1.bits.AUTO_OBSTRUCTION_bit)
            ui->cb_autoObs->setEnabled(true);  // enable
        if (!mLidar.SPECIAL_MODES.bits.FORCE_R_DISPLAY)
            ui->cb_range->setEnabled(true);  // enable
#ifdef LIDARCAM
        Utils& u = Utils::get();
        u.sendMbPacket((unsigned char)CMD_INC_MODE_OFF, 0, NULL, NULL);   // Exit INC mode
#endif
    }
}

void modeSel::on_cb_singleShot_stateChanged(int arg1)
{
    if (arg1 == Qt::Checked)
    {   // Single Shot
        if (mLidar.SPEED_RANGE_DISPLAY == 2)
            mLidar.SPEED_RANGE_DISPLAY = 5; // Range only
        else if (mLidar.SPEED_RANGE_DISPLAY == 1)
            mLidar.SPEED_RANGE_DISPLAY = 4; // Speed only
        else if (!mLidar.SPEED_RANGE_DISPLAY)
        {   // Speed and Range
            mLidar.SPEED_RANGE_DISPLAY = 3;
            if (ui->cb_range->isChecked() == true)
                ui->cb_range->setChecked(false);
        }
    }
    else
    {   // Continuous
        if (mLidar.SPEED_RANGE_DISPLAY == 5)
            mLidar.SPEED_RANGE_DISPLAY = 2; // Range only
        else if (mLidar.SPEED_RANGE_DISPLAY == 4)
            mLidar.SPEED_RANGE_DISPLAY = 1; // Speed only
        else if (mLidar.SPEED_RANGE_DISPLAY == 3)
        {   // Speed and Range
            mLidar.SPEED_RANGE_DISPLAY = 0;
            if (ui->cb_range->isChecked() == true)
                ui->cb_range->setChecked(false);
        }
    }
}

void modeSel::on_cb_ftc_stateChanged(int arg1)
{
   if (arg1 == Qt::Checked)
   {   // Enter FTC mode
       m_command = CMD_MODE_FTC;
       if (mLidar.MODES_1.bits.ZONE_bit)
           ui->cb_zone->setEnabled(false);      // disable
       if (mLidar.MODES_1.bits.AUTO_OBSTRUCTION_bit)
           ui->cb_autoObs->setEnabled(false);   // disable
       if (!mLidar.INHIBIT_MODES.bits.INCLEMENT_WEATHER)
           ui->cb_incWx->setEnabled(false);     // disable
       if (!mLidar.SPECIAL_MODES.bits.FORCE_R_DISPLAY)
       {
           ui->cb_range->setChecked(false);
           ui->cb_range->setEnabled(false);     // disable
           if (mLidar.SPEED_RANGE_DISPLAY == 2)
               mLidar.SPEED_RANGE_DISPLAY = 0;
           else if (mLidar.SPEED_RANGE_DISPLAY == 5)
               mLidar.SPEED_RANGE_DISPLAY = 3;
       }
       enterOperMode(CMD_MODE_FTC, true);
   }
   else
   {   // Exit FTC mode
      enterOperMode(CMD_MODE_FTC, false);
       m_command = CMD_NONE;
       if (mLidar.MODES_1.bits.ZONE_bit)
           ui->cb_zone->setEnabled(true);       // enable
       if (mLidar.MODES_1.bits.AUTO_OBSTRUCTION_bit)
           ui->cb_autoObs->setEnabled(true);    // enable
       if (!mLidar.INHIBIT_MODES.bits.INCLEMENT_WEATHER)
           ui->cb_incWx->setEnabled(true);      // enable
       if (!mLidar.SPECIAL_MODES.bits.FORCE_R_DISPLAY)
           ui->cb_range->setEnabled(true);      // enable
   }
   Utils::get().setMode(m_command);
}

void modeSel::on_cb_logChase_stateChanged(int arg1)
{
    if (arg1 == Qt::Checked)
        ui->cb_logStats->setEnabled(false); // disable
    else
        ui->cb_logStats->setEnabled(true);  // enable
}

void modeSel::on_cb_logStats_stateChanged(int arg1)
{
    if (arg1 == Qt::Checked)
        ui->cb_logChase->setEnabled(false); // disable
    else
        ui->cb_logChase->setEnabled(true);  // enable
}
