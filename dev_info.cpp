#include "dev_info.h"
#include "ui_dev_info.h"
#include "state.h"
#include "utils.h"
#include "debug.h"

devInfo::devInfo(QWidget *parent) :
    baseMenu(parent),
    ui(new Ui::devInfo)
{
    ui->setupUi(this);

    QString s = this->styleSheet() + QStringLiteral("QWidget {font: BOLD 10pt;}");
    this->setStyleSheet(s);

    setInittoggleValues();

	state& v = state::get();
    v.setState(STATE_DEV_INFO_MENU);

	m_command = CMD_NONE;

}

devInfo::~devInfo()
{
    delete ui;
}

void devInfo::setInittoggleValues()
{
   Utils &u = Utils::get();
   Administration admin = u.getAdmin();;

   std::string str;
   str.resize(8);
   QString lidarVersion;
#ifdef LIDARCAM
   memcpy((char *)str.data(), (char *)&u.lidarDataBuf()->lidarStruct.SERIAL_NUMBER[0], 8);
   lidarVersion.append(QString::number(u.lidarDataBuf()->lidarStruct.FW_VERSION[0]));
   lidarVersion.append(".");
   lidarVersion.append(QString::number(u.lidarDataBuf()->lidarStruct.FW_VERSION[1]));
   lidarVersion.append(".");
   lidarVersion.append(QString::number(u.lidarDataBuf()->lidarStruct.FW_VERSION[2]));
   lidarVersion.append(".");
   lidarVersion.append(QString::number(u.lidarDataBuf()->lidarStruct.FW_VERSION[3]));
#endif

#ifdef HH1
    ui->label_6->setText("Radar Version:");
    ui->label_7->setText("App Version:");
    SysConfig & cfg = u.getConfiguration();
    ui->lb_serialNumber->setText( cfg.serialNumber );
#else
    QString serialNumber = QString::fromStdString(str);
    ui->lb_serialNumber->setText( serialNumber );
    ui->lb_versionLidar->setText( lidarVersion);
#endif
    ui->lb_versionVideo->setText( APP_VER );
    QProcess process;
    process.start(" /bin/sh -c \"/bin/cat /proc/version | /usr/bin/awk {'print $3'} | /usr/bin/awk -F_ {'print $1'} \"");
    process.waitForFinished(-1); // will wait forever until finished
    QString stdout = process.readAllStandardOutput();
//    QString stderr = process.readAllStandardError();

    ui->lb_versionLinux->setText( stdout.trimmed() );
    struct Evidence evid;
    u.getEvidenceNum(evid);
    ui->lb_countVideo->setText(QString::number(evid.evidNum));
    ui->lb_countImage->setText(QString::number(evid.imageNum));
    ui->lb_countTicket->setText(QString::number(evid.ticketNum));
    if (admin.dateFormat)
    {   // dd-MM-yyyy format
       QStringList pieces = admin.expiration.split('-');
       QString date1 = "";
       if (pieces.length() >= 2)
       {
          date1 = pieces.value(2);
          QString month1 = pieces.value(1);
          QString year1 = pieces.value(0);
          date1 += "-" + month1 + "-" + year1;
       }
       ui->lb_expiration->setText(date1);
    }
    else
    { // yyyy-MM-dd format
       ui->lb_expiration->setText(admin.expiration);
    }
    ui->lb_authority->setText(admin.authority);
    ui->lb_certification->setText(admin.certification);

}

