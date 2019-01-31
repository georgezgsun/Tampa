#include <QApplication>
#include <QCommandLineParser>
#include <QMessageBox>
#include <QSplashScreen>

#include "top_view.h"
#include "global.h"
#include "utils.h"
#include "state.h"
#include "hardButtons.h"
#include "back_ground.h"
#include <unistd.h>
#include "debug.h"
#include "hh1MetaData.h"

bool backLightOn = true;
bool coldFireSleep = false;
qint64 currentSeconds = 0;
int activity = 0;
bool inactive = false;
qint64 inactiveStart = 0;
bool triggerPulled = false;
bool m_recording = false;
QString currentPlaybackFileName;
QElapsedTimer sysTimer;

//CMLParseResult procOptions(QCommandLineParser &p, int *flags);
CMLParseResult procOptions(int *flags);

int main(int argc, char *argv[])
{
    int optFlags = 0;
    QApplication a(argc, argv);

    QCoreApplication::setApplicationName(APP_NAME);
    QCoreApplication::setApplicationVersion(APP_VER);

    QString s = NEAR_BLACK_TEXT_SS;
    a.setStyleSheet(s);

    if (procOptions(&optFlags) != CMLOk)
        return 0;

    qDebug("option = %#x\n", optFlags);
	
    sysTimer.start();
    //	DEBUG() << sysTimer.elapsed();
    Utils& u = Utils::get();
    u.initialization(optFlags);
    //	DEBUG() << sysTimer.elapsed();
    u.setVideoTransparency(false);
    
    // set up the video again
    u.setVideoTransparency(false);
    
   // Setup background thread
   backGround& bg = backGround::get();
   bg.start();

   HH1MetaData& md = HH1MetaData::get();
   md.init();
   
#ifdef HH1
   QPixmap pixmap(":/logo/Splashpng"); //Insert your splash page image here
#else
   QPixmap pixmap(":/logo/Splash"); //Insert your splash page image here
#endif
   //	QPixmap pixmap(":/logo/icon/hh1Graphics/SplashScreen-0a0a0a0.jpg"); //Insert your splash page image here
   if(pixmap.isNull())
   {
     QMessageBox::warning(0, "Error", "Failed to load Splash Screen image!");
   }else{
     QSplashScreen splash(pixmap, Qt::WindowStaysOnTopHint);
     splash.setEnabled(false); //Prevent user from closing the splash
     QFont splashFont;
     splashFont.setFamily("Arial");
     splashFont.setBold(true);
     splashFont.setPixelSize(24);
     splashFont.setStretch(125);
     splash.setFont(splashFont);
     
     QString str =  APP_NAME  APP_VER ;
     DEBUG() << str;
     splash.show();
     splash.showMessage(str, Qt::AlignCenter | Qt::AlignTop, Qt::white );
     a.processEvents(); //Make sure splash screen gets drawn ASAP
   }	

   u.openUserDB();
   u.creatVKB();
#ifdef LIDARCAM
   if (u.connectLidar() == 0) {
     DEBUG() << "lidar linked";
   } else {
     DEBUG() << "connect lidar failed";
   }
#endif
   
   if (u.connectGPSMEM() == 0) {
     DEBUG() << "GPS MEM Share Good";
   } else {
     DEBUG() << "GPS MEM SHARE failed";
   }
   if (u.connectTILTMEM() == 0) {
     DEBUG() << "tilt Mem Share good";
   } else {
     DEBUG() << "tilt Mem Share  failed";
   }
   if (u.connectMAGMEM() == 0) {
     DEBUG() << "mag Mem Share good";
   } else {
     DEBUG() << "mag Mem Share  failed";
   }
#ifdef HH1
   // Need the mag memory also
   if (u.connectRADARMEM() == 0) {
     DEBUG() << "Radar Mem Share good";
   } else {
     DEBUG() << "Radar Mem Share  failed";
   }
#endif
   
#if defined(LIDARCAM) || defined(HH1)
   hardButtons& t = hardButtons::get();
   t.initialization();
#endif
   
   state& v = state::get();
   v.initialization();
   
   // set up the video again
   u.setVideoTransparency(false);
   
   v.setState(STATE_START);
   //    DEBUG() << sysTimer.elapsed();
   int ret = 0;
   
#ifdef HH1
   // Init Radar communication
   bg.initRadarComm();
#endif
   
   while ( true )
   {
     topView w;
     //        DEBUG() << sysTimer.elapsed();
     w.show();
     //        DEBUG() << sysTimer.elapsed();
     ret = a.exec();
     //        DEBUG() << sysTimer.elapsed();
     //	  qDebug() << "ret from a.exec is " << ret  << "m_state " << v.getState();
     if ( ret < 1 )
     {
       DEBUG() << "ret from a.exec lidarCam terminating is " << ret  << "m_state " << v.getState();
       break;
     }
   }
   
   bg.exit();
   bg.wait();
   
   return ret ;
}

CMLParseResult
procOptions(int * )
{
    //return CMLOk;
    //QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(APP_NAME);
    QCoreApplication::setApplicationVersion(APP_VER);

    QCommandLineParser parser;
    parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);
    parser.setApplicationDescription("A radar camera recording system");
    const QCommandLineOption helpOption = parser.addHelpOption();
    const QCommandLineOption versionOption = parser.addVersionOption();

    //add options
#if 0
    QCommandLineOption projHH1Option("hh1", QCoreApplication::translate("main", "Execution for project HH1"));
    parser.addOption(projHH1Option);

    QCommandLineOption highResolutionOption("r", QCoreApplication::translate("main", "Display in high resolution"));
    parser.addOption(highResolutionOption);
#endif

    // Process the actual command line arguments given by the user
    if (!parser.parse(QCoreApplication::arguments())) {
        QMessageBox::warning(0, QGuiApplication::applicationDisplayName(),
                                 "<html><head/><body><h2>" + parser.errorText() + "</h2><pre>"
                                 + parser.helpText() + "</pre></body></html>");
        return CMLError;
    }

    if (parser.isSet(versionOption)) {
        QMessageBox::information(0, QGuiApplication::applicationDisplayName(),
                                     QGuiApplication::applicationDisplayName() + ' '
                                     + QCoreApplication::applicationVersion());
        return CMLVersionRequested;
    }

    if (parser.isSet(helpOption)) {
        QMessageBox::information(0, QGuiApplication::applicationDisplayName(),
                                "<html><head/><body><pre>"
                                + parser.helpText() + "</pre></body></html>");
        return CMLHelpRequested;
    }

#if 0
    if (parser.isSet(projHH1Option))
        *flags |= PROJ_HH1;
    if (parser.isSet(highResolutionOption))
        *flags |= RESOLUTION_HDMI;
#endif

    return CMLOk;
}
