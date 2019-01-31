#ifndef LOC_SETUP_H
#define LOC_SETUP_H

#include <QWidget>
#include <QDateTime>
#include "base_menu.h"

namespace Ui {
class locSetup;
}

class locSetup : public baseMenu
{
    Q_OBJECT

public:
    explicit locSetup(QWidget *parent = 0);
    ~locSetup();

    void toggleValue(int, int, int f=0);
    void refreshData();

public slots:
    void setDateTime(QDate &d, QTime &t);

private slots:
	void locSetupTimerHit();
    void on_pb_roadCondition_clicked();

protected slots:
    virtual void setCmd();

private:
    Ui::locSetup *ui;
    QDate m_date;
    QTime m_time;
    struct Location m_currLoc;  //current location in use
    int m_spped;
    QString m_mode;

    vkILine m_locationHolder;   //temporary hold the location input

    void initLists();

    void buildHashTables();
    void setInittoggleValues();

    //values for toggle fields, some use m_onOffList
    QStringList m_envList;
    QStringList m_spkrVolList;
    QStringList m_zoneList;
    QStringList m_backLghtList;

    int m_envIndex;
    int m_spkrVolIndex;
    int m_zoneIndex;
    int m_backLghtIndex;
	void doGPS();
	QTimer *m_timer;
	void locSetupInitTimer();
	

};

#endif // LOC_SETUP_H
