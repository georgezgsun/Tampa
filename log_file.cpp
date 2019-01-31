#include <QMessageBox>
#include "log_file.h"
#include "ui_log_file.h"
#include "state.h"
#include "ColdFireCommands.h"
#include "utils.h"

logFile::logFile(QWidget *parent) :
    baseMenu(parent),
    ui(new Ui::logFile)
{
    ui->setupUi(this);

    this->initLists();

    state& v = state::get();
    v.setState(STATE_SUB_MENU2);
    m_command = CMD_LOG_FILE;

    QFile file("/usr/local/stalker/stalkerLog.txt");
    if (file.exists() == true)
    {
       file.open(QIODevice::ReadOnly);
       QTextStream stream(&file);
       QString content = stream.readAll();
       file.close();
       ui->te_log->setFontPointSize(10);
       ui->te_log->setPlainText(content);
       if (content.isEmpty())
          ui->pb_clearLog->setEnabled(false);
    }
    else
       ui->pb_clearLog->setEnabled(false);
}

logFile::~logFile()
{
    delete ui;
}

void logFile::on_pb_clearLog_clicked()
{
#ifdef LIDARCAM
    Utils::get().sendMbPacket( (unsigned char) CMD_KEYBOARD_BEEP, 0, NULL, NULL );
#endif
   QMessageBox msgBox;
   msgBox.setText("Are you sure you want to clear the log?");
   msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
   msgBox.setDefaultButton(QMessageBox::No);
   msgBox.setIcon(QMessageBox::Question);
   QPalette p;
   p.setColor(QPalette::Window, Qt::red);
   msgBox.setPalette(p);

   if(msgBox.exec() == QMessageBox::Yes)
   {
      // Clear the log here in the code
      system("rm -f /usr/local/stalker/stalkerLog.txt; touch /usr/local/stalker/stalkerLog.txt");
      ui->te_log->clear();
      ui->pb_clearLog->setEnabled(false);
//      Utils::get().getExitButton()->clicked();   // Clicked 'Exit' button
   }
}
