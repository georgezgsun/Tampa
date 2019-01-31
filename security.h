#ifndef SECURITY_OPTIONS_H
#define SECURITY_OPTIONS_H

#include <QWidget>
#include <QPushButton>
#include "global.h"
#include "base_menu.h"

namespace Ui {
class securityOptions;
}

class securityOptions : public baseMenu
{
    Q_OBJECT

public:
    explicit securityOptions(QWidget *parent = 0);
    ~securityOptions();

    void toggleValue(int cmd, int idx, int f=0);

private slots:

    void on_pb_dateFormat_clicked();
    void on_pb_language_clicked();

    void on_pb_counters_clicked();

private:
    Ui::securityOptions *ui;

    void initLists();
    void buildHashTables();
    void setInittoggleValues();

    //values for toggle fields, some use m_onOffList
    QStringList m_dateFormatList;
    QStringList m_languageList;

    Administration mAdmin;
    Administration mOldAdmin;
};

#endif // SECURITY_OPTIONS_H
