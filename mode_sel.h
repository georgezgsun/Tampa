#ifndef MODE_SEL_H
#define MODE_SEL_H

#include <QWidget>
#include "base_menu.h"
#include "Lidar.h"

namespace Ui {
class modeSel;
}

class modeSel : public baseMenu
{
    Q_OBJECT

public:
    explicit modeSel(QWidget *parent = 0);
    ~modeSel();

    void toggleValue(int, int, int f=0);

private slots:
    void on_pb_lensFocal_clicked();

private:
    Ui::modeSel *ui;

    void initLists();
    void buildHashTables();
    void setInittoggleValues();

     SysConfig mConf;
     QStringList m_lensFocal;
     QStringList m_lensFocalList;

     int m_lensFocalIndex;

     float mOldRadarH;
     float mOldDistFrR;
     float mOldTargetH;
     unsigned int mOldLensFocal;

};

#endif // MODE_SEL_H
