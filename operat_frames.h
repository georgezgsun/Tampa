#ifndef OPERAT_FRAMES_H
#define OPERAT_FRAMES_H

#include <QStackedWidget>
#include <QGraphicsScene>
#include <QGraphicsLineItem>

namespace Ui {
class OperatFrames;
}

//signal flag value, also frame page index
#define OPR_SCREEN          0
#define OPR_MENU_SCREEN     1
#define OPR_SETUP_SCREEN    2
#define REOPEN_HOME_SCREEN  100

//label index
#define OPR_SDLABEL1        1
#define OPR_SDLABEL2        2
#define OPR_SDLABEL3        3
#define OPR_SDLABEL4        4

class OperatFrames : public QStackedWidget
{
    Q_OBJECT

public:
    explicit OperatFrames(QWidget *parent = 0);
    ~OperatFrames();

    void setSpeedDistance(int index, QString& speed, QString& distance);

protected:
    bool eventFilter(QObject *, QEvent *);

signals:
    void sig_oprScreenReq(int flag);
    void sig_startRecord();
    void sig_stopRecord();
   // void sig_homeScreenReq(int);

private slots:
    void on_pb_exit_clicked();
    void on_pb_record_clicked();

    void on_pb_selfTest_clicked();

    void on_pb_spare1_clicked();

private:
    Ui::OperatFrames *ui;

    int m_recording;
    int mLoop1;
};

#endif // OPERAT_FRAMES_H
