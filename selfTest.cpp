#include "selfTest.h"
#include "ui_selfTest.h"

selfTest::selfTest(QWidget *parent) :
   baseMenu(parent),
   ui(new Ui::selfTest)
{
   ui->setupUi(this);
}

selfTest::~selfTest()
{
   delete ui;
}

void selfTest::on_pb_back_clicked()
{
   close();
}
