#include "loc_setup.h"
#include "Message_Queue_Struct.h"
#include "utils.h"
#include <QDebug>
#include <QPainter>
#include <QPaintEvent>
#include <QGraphicsView>
#include <QGraphicsScene>
#include "lidarMsg.h"
#include "Lidar_Buff.h"
#include "ui_dist_measure.h"
#include "ColdFireCommands.h"
#include "hardButtons.h"
#include "dist_measure.h"
#include "vkiline.h"

distMeasure::distMeasure(QWidget *parent) :
    baseMenu(parent),
    ui(new Ui::distMeasure)
{
    ui->setupUi(this);
}

distMeasure::~distMeasure()
{
    delete ui;
}

void distMeasure::init(int menuType)
{    
#ifdef LIDARCAM
    // Steven Cao, 12/5/2017
    Utils& u = Utils::get();
    mLidarLinked = u.getConnected();

    // Map Hard Button
    hardButtons& hd = hardButtons::get();
    hd.setHardButtonMap( 0, NULL);
    hd.setHardButtonMap( 1, ui->pb_enter);
    // Restart AV server
    char data1[APPRO_DATA_LEN];
    data1[0] = 1; // 1 -> start
    u.SndApproMsg(APPRO_AVONOFF, data1, NULL);  // 1 -> start
#else   // PC
    mLidarLinked = 1;   // Always connected
    mRange = 950.5;
#endif
    mMeasure = 0;
    mDistFlag = true;   // Distance 1
    mFtcShoot = false;
    mFtcTrigger = false;
    mFtcDataTrue = false;

    mMenuType = menuType;

    QFont font;
    if (menuType == CMD_MODE_FTC)
       font.setPointSize(14);
    else
       font.setPointSize(12);
    ui->lb_top->setFont(font);
    ui->lb_zoomVal_2->setFont(font);
    font.setPointSize(24);
    ui->lb_range->setFont(font);
    font.setPointSize(14);
    ui->lb_ftc1->setFont(font);
    ui->lb_ftc2->setFont(font);
    ui->lb_ftc3->setFont(font);
    ui->pb_exit->setEnabled(false);    // Has to measure now
    ui->wt_liveView->setStyleSheet(QStringLiteral("background-color: rgb(0, 0, 0);"));
    if (menuType == CMD_MODE_ZONE)
    {
        QString str1 = QString("<font color=red>%1").arg("CAPTURE DISTANCE 1");
        ui->lb_top->setText(str1);
        ui->pb_value->setEnabled(false);
#ifdef LIDARCAM
        hd.setHardButtonMap( 2, (QPushButton *)ui->le_keypad);
        hd.setHardButtonMap( 3, NULL);
        hd.setKeyBoardFlag(true);   // above hardbutton 2 is Keyboard type
#endif

        m_distHolder.setPlaceholderText("Enter Distance");
        connect(ui->le_keypad, SIGNAL(linePressed(vkILine*)), this, SLOT(lb_keypad_clicked(vkILine*)));
    }
    else if (menuType == CMD_MODE_AUTO)
    {
        ui->pb_enter->setText("ACCEPT");
        ui->le_keypad->setEnabled(false);
        ui->lb_zoomVal_2->setText("");
#ifdef LIDARCAM
        hd.setHardButtonMap( 2, NULL);
        hd.setHardButtonMap( 3, ui->pb_value);
#endif
    }
    else if (menuType == CMD_MODE_FTC)
    {
        ui->pb_enter->setText("ACCEPT");
        ui->le_keypad->setEnabled(false);
        ui->lb_zoomVal_2->setText("");
        ui->pb_value->setEnabled(false);
        QString str1 = QString("<font color=red>%1").arg("Shoot Center of Lane");
        ui->lb_top->setText(str1);
#ifdef LIDARCAM
        hd.setHardButtonMap( 2, NULL);
        hd.setHardButtonMap( 3, NULL);
#endif
    }
    // Laser point box
    ui->laserRec->setStyleSheet("border:1px solid #ff0000;");
    drawLaserSquare();   // Draw laser point
}

void distMeasure::openVKB(vkILine *l)
{
    toggleVKB(l);
    focusLine(l);
    m_vkb->setNumKeyboard();
    connect(m_vkb, SIGNAL(cancelKeyBoard()), this, SLOT(hideVKB()));    // New Keyboard
}

void distMeasure::closeVKB()
{
    Q_ASSERT(m_vkb);
    disconnect(m_vkb, SIGNAL(cancelKeyBoard()), this, SLOT(hideVKB())); // New Keyboard
    if (m_vkb->isVisible())
    {
        m_vkb->hide(true);
    }
}
void distMeasure::toggleVKB(vkILine *l)
{
    Q_ASSERT(m_vkb);

    if (m_vkb)
    {
        if (m_vkb->isVisible())
        {
            setFocus();
            m_vkb->hide(true);
        }
        else
        {
            m_vkb->show(l, focusWidget());
            m_vkb->move(x(), y());
        }
    }
    else
    {
        qDebug() << "virtual keyboard is not created";
    }
}

void distMeasure::focusLine(vkILine *l)
{
   Q_ASSERT(m_vkb);

   if (m_vkb->isVisible() == false)
      return;

   if (m_vkb->currentTextBox() == l)
      return;
}

void distMeasure::timerHit()
{
   float newRange;
#ifdef LIDARCAM
   if (mLidarLinked)
   {
      Utils& u = Utils::get();
      newRange = u.lidarRange();
   }
#else
   static int loop1 = 0;
   newRange = mRange;
   if (mFtcShoot == false)
      newRange += 5;
   else
   {
      if (++loop1 == 2)
         triggerPulled = true;
      else if (loop1 == 4)
         triggerPulled = false;
      else if (loop1 == 6)
         triggerPulled = true;
      else if (loop1 == 8)
         triggerPulled = false;
      else if (loop1 >= 20)
         loop1 = 0;
   }
#endif

   //display the radar range
   // Note: Speed/Range will only be updated when the trigger is pulled.
   // Otherwise, it would keep the previous data
   if (mLidarLinked)
   {  // Link is alive
      if ( mRange != newRange && mFtcShoot == false)
      {  // Yes, new data, not in special FTC mode
         mRange = newRange;
         if ((mRange > 2.0) || (mRange < -2.0) )
         {
//            QString d1 = QString("<font color=green><font size=13>%1</font>FT</font>").arg(mRange,0,'f',0,'0');
            QString d1 = QString("<font color=green>%1</font>FT</font>").arg(mRange);
            ui->lb_range->setText(d1);
         }
         else
         {
            ui->lb_range->setText("");
         }
      }

      // The followings are for special FTC handling
      // Only execute the followign code after Lane distance was measured in FTC mode
      if (mFtcShoot == true)
         ftcMeasure();
   }
}

void distMeasure::hideVKB()
{
    char buf1[16];
    QString distance;
    distance = m_distHolder.text();
#ifdef LIDARCAM
    Utils& u = Utils::get();
#endif
    if (mDistFlag == true)
    {   // Distance 1
        mDist1 = distance.toFloat();
#ifdef LIDARCAM
        u.sendMbPacket((unsigned char)CMD_ZONE_DIST2, 4, (unsigned char *)&mDist1, NULL);
#endif
        mDistFlag = false;  // Switch to distance 2
        sprintf(buf1, "%4.2f", mDist1);
        QString str1 = QString("<font color=red>%1").arg("CAPTURE DISTANCE 2");
        ui->lb_top->setText(str1);
        QString str2 = QString("<font color=red>%1").arg("PRESS ENTER TO ACCEPT OR<br>KEYPAD TO ENTER DISTANCE MANUALLY");
        ui->lb_zoomVal_2->setText(str2);
        m_baseTimer->start();
    }
    else
    {   // Distance 2
        mDist2 = distance.toFloat();
#ifdef LIDARCAM
        struct Message_Queue_Buff rcvMsg;
        unsigned int data1 = 0;
        u.sendMbPacket((unsigned char)CMD_ZONE_CALC, 4, (unsigned char *)&mDist2, &rcvMsg);
//        hexDump("Distance Measure:", (void *)&rcvMsg, sizeof(struct Message_Queue_Buff));
        data1 = rcvMsg.Msg_Info.data[5 + 5];
        data1 <<= 8;
        data1 |= rcvMsg.Msg_Info.data[5 + 4];
        data1 <<= 8;
        data1 |= rcvMsg.Msg_Info.data[5 + 3];
        data1 <<= 8;
        data1 |= rcvMsg.Msg_Info.data[5 + 2];
        mMeasure = *(float *)&data1;
        hardButtons& hd = hardButtons::get();
        hd.setHardButtonMap( 0, ui->pb_exit);   // Connect 'exit' hardbutton
        hd.setHardButtonMap( 1, NULL);          // Disconnect 'enter' hardbutton
        hd.setHardButtonMap( 2, NULL);
        hd.setHardButtonMap( 3, ui->pb_value);
#else
        mMeasure = mDist2 - mDist1;
#endif
        mDistFlag = true;
        sprintf(buf1, "%4.2f", mDist2);
        ui->pb_enter->setEnabled(false);    // No more measure
        ui->le_keypad->setEnabled(false);   // No more manual input
        ui->pb_exit->setEnabled(true);      // can exit now
        ui->pb_value->setEnabled(true);     // Enable Redo

        QFont font;
        font.setPointSize(18);
        ui->lb_range->setFont(font);
        sprintf(buf1, "%4.2f", mMeasure);
        QString d1 = QString("<font color=red>%1 <font color=green>%2 %3").arg("Zone Size: ").arg(buf1).arg(" FT");
        ui->lb_range->setText(d1);
        ui->lb_top->setText("");
        ui->lb_zoomVal_2->setText("");
        m_baseTimer->stop();
    }
    closeVKB();
}

void distMeasure::lb_keypad_clicked(vkILine* vkl)
{
    Utils& u = Utils::get();
    u.sendMbPacket( (unsigned char) CMD_KEYBOARD_BEEP, 0, NULL, NULL );
    if (!m_vkb)
    {
        m_vkb = u.vkb();
    }
    else
    {
        if (m_vkb->isVisible())
            return;
    }
    openVKB(vkl);
    m_distHolder.setText("");
    m_vkb->show(&m_distHolder, this);
    m_vkb->setActiveForms(&m_distHolder, this);

    m_baseTimer->stop();
}

void distMeasure::on_pb_enter_clicked()
{
    char buf1[16];

    Utils& u = Utils::get();
    u.sendMbPacket( (unsigned char) CMD_KEYBOARD_BEEP, 0, NULL, NULL );
    if (mMenuType == CMD_MODE_AUTO || mMenuType == CMD_MODE_FTC)
    {   // OBS mode
        mMeasure = mRange;
#ifdef LIDARCAM
        if (mMenuType == CMD_MODE_AUTO)
           u.sendMbPacket((unsigned char)CMD_OBS_SET, 4, (unsigned char *)&mMeasure, NULL);
        else
           u.sendMbPacket((unsigned char)CMD_FTC_START, 4, (unsigned char *)&mMeasure, NULL);
        hardButtons& hd = hardButtons::get();
        hd.setHardButtonMap( 0, ui->pb_exit);   // Connect 'exit' hardbutton
        hd.setHardButtonMap( 1, NULL);          // Disconnect all other hardbuttons
        hd.setHardButtonMap( 2, NULL);
        hd.setHardButtonMap( 3, NULL);
#endif
        ui->pb_value->setEnabled(false);
        ui->pb_enter->setEnabled(false);    // No more measure
        ui->pb_exit->setEnabled(true);    // can exit now

        QFont font;
        font.setPointSize(18);
        ui->lb_range->setFont(font);
        sprintf(buf1, "%4.2f", mMeasure);
        QString d1 = QString("<font color=red>%1 <font color=green>%2 %3").arg("Measure: ").arg(buf1).arg(" FT");
        ui->lb_range->setText(d1);
        ui->lb_zoomVal_2->setText("");
        if (mMenuType == CMD_MODE_AUTO)
        {
           m_baseTimer->stop();
           ui->lb_top->setText("");
        }
        else
        {
           mFtcShoot = true;
           mState = 0;
           m_baseTimer->stop();
           QFont font;
           font.setPointSize(18);
           ui->lb_top->setFont(font);
           QString str1 = QString("<font color=red>%1").arg("Following Too Close");
           ui->lb_top->setText(str1);
           font.setPointSize(14);
           ui->lb_range->setFont(font);
           str1 = QString("<font color=red>%1").arg("Shoot Vehicle 1");
           ui->lb_range->setText(str1);
           m_baseTimer->start(500);  // .5 ms
        }
        return;
    }

    // Zone mode
    if (mDistFlag == true)
    {   // Distance 1
        mDist1 = mRange;
#ifdef LIDARCAM
        u.sendMbPacket((unsigned char)CMD_ZONE_DIST2, 4, (unsigned char *)&mDist1, NULL);
#endif
        mDistFlag = false;  // Switch to distance 2
//        sprintf(buf1, "%4.2f", mDist1);
//        ui->pb_value->setText((QString)buf1);
        QString str1 = QString("<font color=red>%1").arg("CAPTURE DISTANCE 2");
        ui->lb_top->setText(str1);
        QString str2 = QString("<font color=red>%1").arg("PRESS ENTER TO ACCEPT");
        ui->lb_zoomVal_2->setText(str2);
    }
    else
    {   // Distance 2
        mDist2 = mRange;
#ifdef LIDARCAM
        struct Message_Queue_Buff rcvMsg;
        unsigned int data1 = 0;
        u.sendMbPacket((unsigned char)CMD_ZONE_CALC, 4, (unsigned char *)&mDist2, &rcvMsg);
//        hexDump("Distance Measure:", (void *)&rcvMsg, sizeof(struct Message_Queue_Buff));
        data1 = rcvMsg.Msg_Info.data[5 + 5];
        data1 <<= 8;
        data1 |= rcvMsg.Msg_Info.data[5 + 4];
        data1 <<= 8;
        data1 |= rcvMsg.Msg_Info.data[5 + 3];
        data1 <<= 8;
        data1 |= rcvMsg.Msg_Info.data[5 + 2];
        mMeasure = *(float *)&data1;

        hardButtons& hd = hardButtons::get();
        hd.setHardButtonMap( 0, ui->pb_exit);   // Connect 'exit' hardbutton
        hd.setHardButtonMap( 1, NULL);          // Disconnect 'enter' hardbutton
        hd.setHardButtonMap( 3, ui->pb_value);  // Connect 'redo' hardbutton
#else
        mMeasure = mDist2 - mDist1;
#endif
        mDistFlag = true;
        ui->pb_enter->setEnabled(false);    // No more measure
        ui->le_keypad->setEnabled(false);   // No more manual input
        ui->pb_exit->setEnabled(true);    // can exit now
        ui->pb_value->setEnabled(true);     // Enable Redo

        QFont font;
        font.setPointSize(18);
        ui->lb_range->setFont(font);
        sprintf(buf1, "%4.2f", mMeasure);
        QString d1 = QString("<font color=red>%1 <font color=green>%2 %3").arg("Zone Size: ").arg(buf1).arg(" FT");
        ui->lb_range->setText(d1);
        ui->lb_top->setText("");
        ui->lb_zoomVal_2->setText("");
        m_baseTimer->stop();
    }
}

void distMeasure::on_pb_exit_clicked()
{
    Utils& u = Utils::get();
    hardButtons& hd = hardButtons::get();
    hd.setHardButtonMap( 0, NULL);   // Disconnect 'exit' hardbutton
    hd.setKeyBoardFlag(false);
    u.sendMbPacket( (unsigned char) CMD_KEYBOARD_BEEP, 0, NULL, NULL );
    delete m_baseTimer;

    // Suspend AV server
    char data1[APPRO_DATA_LEN];
    data1[0] = 0; // 0 -> suspend
    u.SndApproMsg(APPRO_AVONOFF, data1, NULL);

    close();
}

void distMeasure::on_pb_value_clicked()
{   // Redo
#ifdef LIDARCAM
    Utils& u = Utils::get();
    hardButtons& hd = hardButtons::get();
    if (mMenuType == CMD_MODE_AUTO)
    {
        u.sendMbPacket((unsigned char)CMD_OBS_LEARN, 0, NULL, NULL);   // Enter Auto OBS mode
        u.sendMbPacket( (unsigned char) CMD_KEYBOARD_BEEP, 0, NULL, NULL );
    }
    else if (mMenuType == CMD_MODE_ZONE)
    {   // Zone mode
        u.sendMbPacket((unsigned char)CMD_ZONE_EXIT, 0, NULL, NULL);    // Exit Zone mode
        hd.setHardButtonMap( 0, NULL);   // Disconnect 'exit' hardbutton
        hd.setHardButtonMap( 1, ui->pb_enter);  // Reconnect Enter
        hd.setHardButtonMap( 2, (QPushButton *)ui->le_keypad);  // Reconnect manual input
        hd.setHardButtonMap( 3, NULL);
        u.sendMbPacket( (unsigned char) CMD_KEYBOARD_BEEP, 0, NULL, NULL );
        ui->pb_exit->setEnabled(false);    // Has to measure now
        ui->pb_enter->setEnabled(true);
        ui->le_keypad->setEnabled(true);
        ui->pb_value->setEnabled(false);    // Has to measure now
        u.sendMbPacket((unsigned char)CMD_ZONE_DIST1, 0, NULL, NULL);   // Enter Zone mode
        mDistFlag = true;   // Distance 1
        QString str1 = QString("<font color=red>%1").arg("CAPTURE DISTANCE 1");
        ui->lb_top->setText(str1);
        ui->lb_range->setText("");
        QString str2 = QString("<font color=red>%1").arg("PRESS ENTER TO ACCEPT OR<br>KEYPAD TO ENTER DISTANCE MANUALLY");
        ui->lb_zoomVal_2->setText(str2);
    }
#else
    mRange = 950.5;
    if (mMenuType == CMD_MODE_ZONE)
    {   // Zone mode
        ui->pb_exit->setEnabled(false);    // Has to measure now
        ui->pb_enter->setEnabled(true);
        ui->le_keypad->setEnabled(true);
        ui->pb_value->setEnabled(false);    // Has to measure now
        mDistFlag = true;   // Distance 1
        QString str1 = QString("<font color=red>%1").arg("CAPTURE DISTANCE 1");
        ui->lb_top->setText(str1);
        ui->lb_range->setText("");
        QString str2 = QString("<font color=red>%1").arg("PRESS ENTER TO ACCEPT OR<br>KEYPAD TO ENTER DISTANCE MANUALLY");
        ui->lb_zoomVal_2->setText(str2);
    }
#endif
    m_baseTimer->start();
}

void distMeasure::drawLaserSquare(void)
{
    int rWidth, startX, startY, retv = -1;
    QWidget *pActiveW = QApplication::activeWindow();

    FILE *fp1 = fopen("/usr/local/stalker/laser.cfg", "rb");
    if (fp1 != NULL)
    {
        char str1[32];
        fgets(str1, sizeof(str1), fp1);
        if (sscanf(str1, "%d %d %d", &startX, &startY, &rWidth) == 3)
        {
            int maxWidth = pActiveW->width();
            int maxHeight = pActiveW->height();
            maxWidth -= rWidth;
            maxHeight -= rWidth;
            if (startX >= 0 && startX <= maxWidth &&
                startY >= 0 && startY <= maxHeight && rWidth <= 60)
            {
//                printf("New Laser Box: X->%d, Y->%d, W->%d\n", startX, startY, rWidth);
                retv = 0;
            }
        }
    }

    if (retv)
    {   // Use default
        rWidth = 15;  //10 pixel
        startX = pActiveW->width() / 2;
        startY = pActiveW->height() / 2;
    }

    ui->laserRec->resize(rWidth, rWidth);
    ui->laserRec->move(startX, startY);
    ui->laserRec->raise();
}

void distMeasure::ftcMeasure(void)
{
   float newSpeed;
   unsigned short len1;
   char buf1[64];
   Utils& u = Utils::get();
#ifdef LIDARCAM
   bool ret;
   newSpeed = u.lidarSpeed();
#else
   if (mState == 1)
      newSpeed = 47.7;
   else if (mState == 3)
      newSpeed = 55.3;
   else if (mState == 5)
      triggerPulled = true;

#endif
/*   static int oldState;
   if (oldState != mState)
   {
      char buf1[4];
      sprintf(buf1, "%d", mState);
      QString str2 = QString("<font color=green>%1").arg(buf1);
      ui->lb_zoomVal_2->setText(str2);
      oldState = mState;
   }*/
   switch (mState)
   {
      case 0:  // Update screen for 1st vehicle
         {
            QString str1 = QString("<font color=red>%1").arg("Shoot Vehicle 1");
            ui->lb_range->setAlignment(Qt::AlignCenter);
            ui->lb_range->setText(str1);
            ui->lb_ftc1->setText("");
            ui->lb_ftc2->setText("");
            ui->lb_ftc3->setText("");
         }
         mState = 1;
         break;
      case 1:  // 1st vehicle
         if (newSpeed >= 3 || newSpeed <= -3)
         {
            QString str1 = QString("<font color=green>%1 <font color=red> MPH").arg(newSpeed);
            ui->lb_ftc1->setAlignment(Qt::AlignHCenter);
            ui->lb_ftc1->setText(str1);
         }
#ifdef LIDARCAM
         if (u.getAlertPacket(FTC_AUTO1_ALERT_ID, buf1, &len1, 3) == true)
            mState = 2; // Got the 1st vehicle information
#else
         u.getAlertPacket(FTC_AUTO1_ALERT_ID, buf1, &len1, 30);
         mState = 2;
#endif
         break;
      case 2:  // Update screen for 2nd vehicle
         {
            QString str1 = QString("<font color=red>%1").arg("Shoot Vehicle 2");
            ui->lb_range->setText(str1);
            ui->lb_ftc1->setText("");  // Clear old display
            mState = 3;
         }
         break;
      case 3:  // 2nd vehicle
         if (newSpeed >= 3 || newSpeed <= -3)
         {
            QString str1 = QString("<font color=green>%1 <font color=red> MPH").arg(newSpeed);
            ui->lb_ftc1->setAlignment(Qt::AlignHCenter);
            ui->lb_ftc1->setText(str1);
         }
#ifdef LIDARCAM
         if (u.getAlertPacket(FTC_AUTO2_ALERT_ID, buf1, &len1, 3) == true)
            mState = 4;
#else
         u.getAlertPacket(FTC_AUTO2_ALERT_ID, buf1, &len1, 30);
         mState = 4;
#endif
         break;
      case 4:  // Need to read the data from Coldfire
         {
            bool status = false;
            float sec1, dist1, speed1, speed2;
      #ifdef LIDARCAM
            char bufErr[32];
            ret = u.getAlertPacket(FTC_REPORT_ALERT_ID, buf1, &len1, 3);
            if ( ret == true )
            {
               if (len1 == 1)
               {  // FTC error
                  switch (buf1[0])
                  {
                     case 2:  // FTC_OPP_DIR_ERROR
                        sprintf(bufErr, "ERROR: Opposite Vehicle");
                        break;
                     case 3:  // FTC_LOW_DIST_ERROR
                        sprintf(bufErr, "FTC_LOW_DIST_ERROR Error");
                        break;
                     case 4:  // FTC_LONG_DELAY_ERROR
                        sprintf(bufErr, "ERROR: Delay > 3 secs");
                        break;
                     case 5:  // FTC_BAD_PAIR_ERROR
                        sprintf(bufErr, "FTC_BAD_PAIR_ERROR Error");
                        break;
                     default: // Unknown Error
                        sprintf(bufErr, "ERROR: Unknown");
                  }
                  QString str1 = QString("<font color=red>%1").arg((QString)bufErr);
                  ui->lb_range->setText(str1);
                  ui->lb_ftc1->setText("Please Retry or Exit");
                  fprintf(stderr, "%s\n", bufErr);
               }
               else if (len1 == 25)
               {  // Good
                  float f1[6], *pf;
                  pf = f1;
                  memcpy((void *)f1, (void *)&(buf1[1]), 24);
                  sec1 = f1[0];
                  dist1 = f1[1];
                  speed1 = f1[3];
                  speed2 = f1[5];
                  status = true;
               }
               else
               {
                  sprintf(bufErr, "ERROR: Unknown");
                  QString str1 = QString("<font color=red>%1").arg((QString)bufErr);
                  ui->lb_range->setText(str1);
                  fprintf(stderr, "%s\n", bufErr);
               }
            }
            else
            {
               sprintf(bufErr, "ERROR: RX Error");
               QString str1 = QString("<font color=red>%1").arg((QString)bufErr);
               ui->lb_range->setText(str1);
               fprintf(stderr, "%s\n", bufErr);
            }
      #else
            sec1 = 1.5, dist1 = 105.8, speed1 = 47.7, speed2 = 55.3;
            status = true;
      #endif
            if (status == true)
            {
               ui->laserRec->lower();  // Push laser point to the back
               sprintf(buf1, "%2.1f SEC", sec1);
               QString str1 = QString("<font color=red>%1 <font color=green>%2").arg("Time Sep ---------- ").arg(buf1);
               ui->lb_range->setAlignment(Qt::AlignLeft);
               ui->lb_range->setText(str1);
               sprintf(buf1, "%.1f FT", dist1);
               str1 = QString("<font color=red>%1 <font color=green>%2").arg("Sep Dist &nbsp;---------- ").arg(buf1);
               ui->lb_ftc1->setAlignment(Qt::AlignLeft);
               ui->lb_ftc1->setText(str1);
               sprintf(buf1, "%.1f MPH", speed1);
               str1 = QString("<font color=red>%1 <font color=green>%2").arg("Vehicle 1 ---------- ").arg(buf1);
               ui->lb_ftc2->setText(str1);
               sprintf(buf1, "%.1f MPH", speed2);
               str1 = QString("<font color=red>%1 <font color=green>%2").arg("Vehicle 2 ---------- ").arg(buf1);
               ui->lb_ftc3->setText(str1);
            }
         }
         mState = 5;
         break;
      case 5:
         if (triggerPulled == true)
         {
            ui->laserRec->raise();  // Pull laser point to top
            QString str1 = QString("<font color=red>%1").arg("Shoot Vehicle 1");
            ui->lb_range->setAlignment(Qt::AlignCenter);
            ui->lb_range->setText(str1);
            ui->lb_ftc1->setText("");
            ui->lb_ftc2->setText("");
            ui->lb_ftc3->setText("");
            mState = 1;
         }
         break;
      default:
         mState = 0;
   }
}
