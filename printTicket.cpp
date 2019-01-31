#include "printTicket.h"
#include "utils.h"
#include <QFileInfo>
#include <QMessageBox>
#include <QPrinter>
#include <QPainter>
#include <QObject>
#include "debug.h"
#include "file_mgr.h"

printTicket::printTicket()
{
   DEBUG() << "printTicket Constructor";
}

printTicket::~printTicket()
{
   DEBUG() << "printTicket Destructor";
}

void printTicket::print( QString fileName )
{
  QString json1;
  QString ticketFile;
  
  // print the ticket
  
  // Check for printer
  QProcess process;
  QString cmd(" /bin/sh -c \"/usr/sbin/lsusb | /bin/fgrep 03f0 \"");
  QString stdout;
  
#if defined(LIDARCAM) || defined(HH1)
  process.start( cmd );
  process.waitForFinished(-1); // will wait forever until finished
  stdout = process.readAllStandardOutput();
  
  DEBUG() << stdout;
  if( stdout.size() == 0 ) {
    // msgbox
    QString quest_str = QString("HP Printer not attached or powered on!");
    QMessageBox msgBox;
    msgBox.setText(QObject::tr(quest_str.toStdString().c_str()));
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.setIcon(QMessageBox::Warning);
    QPalette p;
    p.setColor(QPalette::Window, Qt::red);
    msgBox.setPalette(p);
    msgBox.exec();
    //	    u.getExitButton()->clicked();   // Clicked 'Exit' button
    //	    DEBUG() << 	u.getExitButton();   // Clicked 'Exit' button
    return;
  }
#endif
  
  QString fullName = QString(DEFAULT_VEDIO_PATH);
  fullName.append( "/");
  fullName.append( Utils::get().session()->user()->loginName);
  fullName.append( "/");
  fullName.append( fileName );
  
  DEBUG() << fileName;
  DEBUG() << fullName;
  
  // make sure this is a jpg file.
  QFileInfo fi( fullName);
  QString ext = fi.suffix();
  
  QString jpgSuffix("jpg");
  
  //	  DEBUG() << ext << "jpgSuffix " << jpgSuffix;
  
  if ( ext != jpgSuffix) {
    DEBUG() << "File printed is not the jpg file";
    // msgbox
    QString quest_str = QString("Chosen file does not end with jpg");
    QMessageBox msgBox;
    msgBox.setText(QObject::tr(quest_str.toStdString().c_str()));
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.setIcon(QMessageBox::Warning);
    QPalette p;
    p.setColor(QPalette::Window, Qt::red);
    msgBox.setPalette(p);
    msgBox.exec();
    //	    u.getExitButton()->clicked();   // Clicked 'Exit' button
    //	    DEBUG() << 	u.getExitButton();   // Clicked 'Exit' button
    return;
  }
  
  // get the json filename
  int pos = fullName.lastIndexOf(QChar('_'));
  json1 = fullName.left(pos);
  json1 += ".json";

  ticketFile = fullName.left(pos);
  ticketFile += ".png";
  
  DEBUG() << json1;
  DEBUG() << ticketFile;
  
//  QPrinter *printer = new QPrinter();
  QPrinter printer;
  QImage ticketImage(printer.width(), printer.height(), QImage::Format_ARGB32);
  QPainter ticket; 
  
  ticket.begin( &ticketImage );
  
  
  QString LogofullName = QString("/usr/local/stalker/bin/");
  LogofullName.append( "aci-logo.png" );
  
  DEBUG() << LogofullName;
  //	  DEBUG() << "Printer Name " << printer->printerName();
  //	  DEBUG() << "Printer State " << printer->printerState();
  //	  DEBUG() << printer->width() << printer->height();
  
  QImage img0( LogofullName );
  
  ticket.drawImage(QRect(50,60,300,75),img0);
  
  // dump the json information
  cmd = QString(" /bin/sh -c \"/bin/grep ");
  cmd.append("time-local ");
  cmd.append(json1);
  cmd.append( "\"");
  
  process.start( cmd );
  process.waitForFinished(-1); // will wait forever until finished
  stdout = process.readAllStandardOutput();
  
  // Format Data and Time
  QStringList list = stdout.split('\"');
  QString dateTime = QString(list.at(3).toLocal8Bit().constData());
  
  QStringList list1 = dateTime.split(" ");
  
  QFont font( "DejaVu Sans");
  font.setPointSize(12);
  QFontMetrics fm(font);
  ticket.setFont(font);
  
#define OFFSET 225	  
  
  QString Date = QString ("DATE: ");
  QString Time = QString( "TIME: ");
  
  
  int lineHeigth = fm.height();
  int line =  60 + 75 + 25;
  ticket.drawText(QRect(50,line,600,lineHeigth), Date); 
  ticket.drawText(QRect(OFFSET,line,600,lineHeigth), list1.at(0).toLocal8Bit().constData()); 
  line = line + lineHeigth;
  ticket.drawText(QRect(50,line,600,lineHeigth), Time ); 
  ticket.drawText(QRect(OFFSET,line,600,lineHeigth), list1.at(1).toLocal8Bit().constData() ); 
  line = line + lineHeigth;
  
  cmd = QString(" /bin/sh -c \"/bin/grep ");
  cmd.append("serial-number ");
  cmd.append(json1);
  cmd.append( "\"");
  
  process.start( cmd );
  process.waitForFinished(-1); // will wait forever until finished
  stdout = process.readAllStandardOutput();
  list = stdout.split('\"');
  
  QString serialNumber = QString ("SERIAL NUMBER: ");
  ticket.drawText(QRect(50,line,600,lineHeigth), serialNumber ); 
  ticket.drawText(QRect(OFFSET,line,600,lineHeigth), list.at(3).toLocal8Bit().constData() ); 
  line = line + lineHeigth;
  
  cmd = QString(" /bin/sh -c \"/bin/grep ");
  cmd.append("stalker-id ");
  cmd.append(json1);
  cmd.append( "\"");
  
  process.start( cmd );
  process.waitForFinished(-1); // will wait forever until finished
  stdout = process.readAllStandardOutput();
  list = stdout.split('\"');
  
  QString recordNumber = QString ("RECORD NUMBER:       ");
  ticket.drawText(QRect(50,line,600,lineHeigth), recordNumber ); 
  ticket.drawText(QRect(OFFSET,line,600,lineHeigth), list.at(3).toLocal8Bit().constData() ); 
  line = line + lineHeigth;
  
  cmd = QString(" /bin/sh -c \"/bin/grep ");
  cmd.append("stalker-speed-units ");
  cmd.append(json1);
  cmd.append( "\"");
  
  process.start( cmd );
  process.waitForFinished(-1); // will wait forever until finished
  stdout = process.readAllStandardOutput();
  
  list = stdout.split('\"');
  
  QString units = list.at(3).toLocal8Bit().constData();
  
  cmd = QString(" /bin/sh -c \"/bin/grep ");
  cmd.append("location ");
  cmd.append(json1);
  cmd.append( "\"");
  
  process.start( cmd );
  process.waitForFinished(-1); // will wait forever until finished
  stdout = process.readAllStandardOutput();
  
  //	  DEBUG() << stdout;
  list = stdout.split('\"');
  QString location = QString ("LOCATION:                    ");
  ticket.drawText(QRect(50,line,600,lineHeigth), location );
  ticket.drawText(QRect(OFFSET,line,600,lineHeigth), list.at(3).toLocal8Bit().constData() );
  line = line + lineHeigth;
  
  cmd = QString(" /bin/sh -c \"/bin/grep ");
  cmd.append("stalker-latitude ");
  cmd.append(json1);
  cmd.append( "\"");
  
  process.start( cmd );
  process.waitForFinished(-1); // will wait forever until finished
  stdout = process.readAllStandardOutput();
  
  //	  DEBUG() << stdout;
  list = stdout.split('\"');
  QString latitude = QString ("LATITUDE: ");
  ticket.drawText(QRect(50,line,600,lineHeigth), latitude );
  ticket.drawText(QRect(OFFSET,line,600,lineHeigth), list.at(3).toLocal8Bit().constData() );
  line = line + lineHeigth;
  
  cmd = QString(" /bin/sh -c \"/bin/grep ");
  cmd.append("stalker-longitude ");
  cmd.append(json1);
  cmd.append( "\"");
  
  process.start( cmd );
  process.waitForFinished(-1); // will wait forever until finished
  stdout = process.readAllStandardOutput();
  
  //	  DEBUG() << stdout;
  list = stdout.split('\"');
  QString longitude = QString ("LONGITUDE: ");
  ticket.drawText(QRect(50,line,600,lineHeigth), longitude );
  ticket.drawText(QRect(OFFSET,line,600,lineHeigth),  list.at(3).toLocal8Bit().constData() );
  line = line + lineHeigth;
  
  cmd = QString(" /bin/sh -c \"/bin/grep ");
  cmd.append("officer ");
  cmd.append(json1);
  cmd.append( "\"");
  
  process.start( cmd );
  process.waitForFinished(-1); // will wait forever until finished
  stdout = process.readAllStandardOutput();
  
  //	  DEBUG() << stdout;
  list = stdout.split('\"');
  QString officer = QString ("OFFICER: ");
  ticket.drawText(QRect(50,line,600,lineHeigth), officer );
  ticket.drawText(QRect(OFFSET,line,600,lineHeigth), list.at(3).toLocal8Bit().constData() );
  line = line + lineHeigth;
  
  cmd = QString(" /bin/sh -c \"/bin/grep ");
  cmd.append("stalker-range ");
  cmd.append(json1);
  cmd.append( "\"");
  
  process.start( cmd );
  process.waitForFinished(-1); // will wait forever until finished
  stdout = process.readAllStandardOutput();
  //	  DEBUG() << stdout;
  list = stdout.split('\"');

  QString range = QString ("RANGE: ");
  QString rangeString( list.at(3).toLocal8Bit().constData());
  rangeString.append(" ");
  cmd = QString(" /bin/sh -c \"/bin/grep ");
  cmd.append("stalker-range-units ");
  cmd.append(json1);
  cmd.append( "\"");
  
  process.start( cmd );
  process.waitForFinished(-1); // will wait forever until finished
  stdout = process.readAllStandardOutput();
  //	  DEBUG() << stdout;
  list = stdout.split('\"');
  QString rangeUnits( list.at(3).toLocal8Bit().constData());

  rangeString.append( rangeUnits );
  
  ticket.drawText(QRect(50,line,600,lineHeigth), range );
  ticket.drawText(QRect(OFFSET,line,600,lineHeigth), rangeString );
  line = line + lineHeigth;
  
  cmd = QString(" /bin/sh -c \"/bin/grep ");
  cmd.append("stalker-speed-limit ");
  cmd.append(json1);
  cmd.append( "\"");
  
  process.start( cmd );
  process.waitForFinished(-1); // will wait forever until finished
  stdout = process.readAllStandardOutput();
  
  //	  DEBUG() << stdout;
  list = stdout.split('\"');
  QString Limit = QString ("SPEED LIMIT: ");
  QString speedLimit( list.at(3).toLocal8Bit().constData());
  speedLimit.append( " ");
  speedLimit.append( units );
  ticket.drawText(QRect(50,line,600,lineHeigth), Limit );
  ticket.drawText(QRect(OFFSET,line,600,lineHeigth), speedLimit );
  line = line + lineHeigth;
  
  cmd = QString(" /bin/sh -c \"/bin/grep ");
  cmd.append("stalker-speed-actual ");
  cmd.append(json1);
  cmd.append( "\"");
  
  process.start( cmd );
  process.waitForFinished(-1); // will wait forever until finished
  stdout = process.readAllStandardOutput();
  //	  DEBUG() << stdout;
  list = stdout.split('\"');
  QString speed = QString ("RECORDED SPEED: ");
  QString Actualspeed( list.at(3).toLocal8Bit().constData());
  Actualspeed.append( " " );
  Actualspeed.append( units );
  ticket.drawText(QRect(50,line,600,lineHeigth), speed );
  ticket.drawText(QRect(OFFSET,line,600,lineHeigth), Actualspeed );
  line = line + lineHeigth;
  
  // add jpg to the ticket
  QImage img1( fullName );
  //	  DEBUG() << img1;
  int width = (int)(printer.width() - 100);
  ticket.drawImage(QRect(50, line + 20, (int)width, (int)((width * 3.0) / 4.0)), img1);
  
  QString signature("SIGNATURE: ___________________________________________");
  
  line = 400 + (int)((width * 3.0) / 4.0) + 45;
  
  ticket.drawText(QRect(50,line,600,lineHeigth), signature ); 
  
  ticket.end();  
  
  ticketImage.save( ticketFile);
  
  QProcess printProcess;
  QString syncCmd = QString("/bin/sync");
  QString printCmd = QString("/usr/bin/lp -s -d HP -o fit-to-page ");
  printCmd.append( ticketFile );
  
  DEBUG() << syncCmd;
  DEBUG() << printCmd;
#if defined(LIDARCAM) || defined(HH1)
  printProcess.start( syncCmd );
  printProcess.waitForFinished(-1);
  //  stdout = process.readAllStandardOutput();
  //  QString stderr = process.readAllStandardError();
  //  DEBUG() << "STDOUT " << stdout;
  //  DEBUG() << "STDERR " << stderr;
  printProcess.start( printCmd );
  printProcess.waitForFinished(-1);
  //  stdout = process.readAllStandardOutput();
  //  stderr = process.readAllStandardError();
  //  DEBUG() << "STDOUT " << stdout;
  //  DEBUG() << "STDERR " << stderr;
#endif
  
  // Need to remove the printed file
  
  return;
}
