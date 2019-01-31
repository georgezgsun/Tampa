#ifndef TOP_VIEW_H
#define TOP_VIEW_H

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QGraphicsProxyWidget>
#include <QMainWindow>
#include <QTimer>
#include <QString>
#include <QLineEdit>
#include <QStack>
#include "global.h"
#include "base_menu.h"
#include "main_menu.h"
#include "widgetKeyBoard.h"
#include "vkiline.h"
#include "user_db.h"
#include "menu_view.h"
#include "operat_frames.h"
#include "back_ground.h"
#include "utils.h"

#ifdef HH1
#include "TiltSensor/TiltSensor.h"
#include "widgets/icon_frame/icon_frame.h"
#endif

#include <QDebug>

namespace Ui {
class topView;
}

#define FULL_WIDTH      480
#define FULL_HEIGHT     272
#define CAM_VIEW_WIDTH  380
#define CAM_SCENE_WIDTH (CAM_VIEW_WIDTH - 2)
#define QUICK_LOGIN_SIM 1       //quick login button simulation

#define TEXT_CAPTURE_FILE "/mnt/mmc/ipnc/capture_data.txt"
//#define CAPTURE_TEXT

class topView : public QMainWindow
{
    Q_OBJECT

public:
    explicit topView();
    ~topView();

    void drawZoomSquare (int zoomRatio);
    void updateRecordView (bool endRecord);
//#ifdef HH1
    //Radar related data
    RadarMemory *mpRadarData;
    float theta_hs_ref;
    float theta_rs_ref;
    float theta_vs_ref;
    float theta_vs;
    float theta_rs;
    float theta_hs;
    TiltSensor *pSensor;
    AccMagData accBuf;
    AccMagData magBuf;
    RadarData_t Radar;
    config_type RadarConfig;
    void emitHandleTargets();
    // Radar screen related data
    QPen  mP1;
    QPen  mP2;
    QPen  mP3;
    QPen  mP4;
//#endif
	
protected:
    void closeEvent(QCloseEvent *);
    bool capture_file_is_open;

signals:
    void clearLogin();
    void powerDown();
    void handleTargets();
    void sig_topViewScreenReq(int);

private slots:
    void powerDownSystem();
    void initProcesses();
    void TopViewTimerHit();
    void setUserName(QString);
    void setPassWord(QString);
    void exeLogin();
    void clearLoginInfo();
    void openMenuScreen();
    int openLoginScreen();
    int openStartScreen();
    void zoomin();
   void focusMode();
   void enableRecord();
#ifdef QUICK_LOGIN_SIM
    void setQuickCode();
#endif
    void openHomeScreen(int);
    void switchScreen(int);  //switch betweem operating and operating menu screen
    void openOperateScreen(void);
    void processTargets(void);

public slots:
    void exeRecord();
    void displayJPG();
    void toggleVKB(vkILine *l);
    void focusLine(vkILine *l);
    void closeVKB();
    void reopenTopScreen();

private:
    Ui::topView *ui;

    //Radar related data
    INT32 err;
    QTimer *m_TopViewTimer;

    int m_loginSuccess;
    int m_command;
    int m_prevCmd;  //command processed
    QString m_loginName;
    QString m_passWord;

    vkILine *m_le_userName;
    vkILine *m_le_passwd;
    iconFrame *m_iconFrame;

    QRect *m_fullRect;
    QRect *m_camRect;
    QRect *m_sceneRect;
    QGraphicsView *m_view;
    QGraphicsScene *m_topScene;
    QGraphicsScene *m_startScene;
    QGraphicsScene *m_loginScene;
    QGraphicsScene *m_homeScene;
    QGraphicsScene *m_currScene;    //???
    QGraphicsScene *m_delScene;     // scene to be deleted later to avoid app crash on hh1

    QGraphicsProxyWidget *m_currProxyWidget;
    QGraphicsProxyWidget *m_oprFramesProxy;

    //QStack <QGraphicsProxyWidget *> m_menuStack;

    //saved widget/scene numbers
    QPoint m_point;
    int m_height;
    int m_width;

    widgetKeyBoard *m_vkb;
	//    userDB *m_userDB;

    //save grapic items for m_topScene
    QRect mRect;
    QGraphicsRectItem *m_targetRect1;
    QGraphicsRectItem *m_targetRect2;
    QGraphicsRectItem *m_targetRect3;
    QGraphicsRectItem *m_targetRect4;
    QGraphicsRectItem *m_zoomSquare;
    QGraphicsEllipseItem *m_recordMark;     //used in recording
    QGraphicsTextItem *m_speed;       //used in recording
    QGraphicsTextItem *m_distance;    //used in recording

    //for icon frame
    QGraphicsProxyWidget *m_iconFrameProxy;

    int initVariables();
    void TopViewInitTimer();

    // radar data
    float m_range;
    float m_newRange;
    float mSpeed;
    float mNewSpeed;
    bool  mRangeOnly;  // Is it Range only?
    void displaySpeed( float speed=0.0);
    void displayRange ( float distance=0.0);

#ifdef QUICK_LOGIN_SIM
    QString m_quickCode;
    QSlider *m_qkcSlider;
    QFrame *m_qkcPbFrame;
    QPushButton *m_qkcButton[4];
    int m_qkcTimeLimit;
    void exeQuickLogin();
#endif

    // Recording
    //    int m_recording;
    int mRecordingSecs;
    bool mAutoRecording;
    int mPhotoNum;

    // Camera setting
    struct CameraSetting mCamSetting;

    int initLoginScene();
    int initTopScene();
    void connectSignals();

    int hideTopPanel();
    int showTopPanel();
    int showFullView();
    int showTopView();
    int hideTopView();

    void InitTimerRadar();
    void create_vkb();
    int openUserDB();

    //open Main Menu's sub-menu
    void openTopScreen();
	void findClickButton( QString name);
	void processHorizontalInput();

	void monitorSpeed();
	void setRecordingText();
	void queryCamSetting();
	void initZoom();
	void displayZoom( int );
#ifdef HH1
    QGraphicsLineItem *m_rollLine;
    QGraphicsLineItem *m_vertLine;
    QGraphicsScene *m_oprScene;
	OperatFrames *m_oprFrames;
//    pthread_t radarThreadId;
    CCoordTransforms Transforms;
    void showTheTarget(int showNum, int targetNum);
    int timeStamp;
    int violationTimeStamp;
#endif
};

#endif // TOP_VIEW_H
