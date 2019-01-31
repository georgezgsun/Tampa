#include "password.h"
#include "ui_password.h"
#include "utils.h"
#include "ColdFireCommands.h"
#include "vkiline.h"
#include "hardButtons.h"

password::password(QWidget *parent) :
    baseMenu(parent),
    ui(new Ui::password)
{
    ui->setupUi(this);
    mPb = NULL;
}

password::~password()
{
    delete ui;
}

void password::init(int menuType)
{
    mMenuType = menuType;
    connect(ui->vk_userinput, SIGNAL(linePressed(vkILine*)), this, SLOT(lb_keypad_clicked(vkILine*)));
#ifdef LIDARCAM
    hardButtons& hd = hardButtons::get();
    hd.setHardButtonMap( 0, ui->pb_exit);
    hd.setHardButtonMap( 1, NULL);
    hd.setHardButtonMap( 2, NULL);
    hd.setHardButtonMap( 3, NULL);
#endif
}

void password::lb_keypad_clicked(vkILine* vkl)
{
    Utils& u = Utils::get();
#ifdef LIDARCAM
    u.sendMbPacket( (unsigned char) CMD_KEYBOARD_BEEP, 0, NULL, NULL );
#endif
    if (!m_vkb)
    {
        m_vkb = u.vkb();
    }
    else
    {
        if (m_vkb->isVisible())
            return;
    }
    openVKB(vkl);
    m_passHolder.setText("");
    m_vkb->show(&m_passHolder, this);
    m_vkb->setActiveForms(&m_passHolder, this);
}

void password::openVKB(vkILine *l)
{
    toggleVKB(l);
    focusLine(l);
//    m_vkb->setNumKeyboard();
    connect(m_vkb, SIGNAL(cancelKeyBoard()), this, SLOT(hideVKB()));    // New Keyboard
}

void password::closeVKB()
{
    Q_ASSERT(m_vkb);
    disconnect(m_vkb, SIGNAL(cancelKeyBoard()), this, SLOT(hideVKB())); // New Keyboard
    if (m_vkb->isVisible())
    {
        m_vkb->hide(true);
    }
}
void password::toggleVKB(vkILine *l)
{
    Q_ASSERT(m_vkb);

    if (m_vkb)
    {
        if (m_vkb->isVisible())
        {
            setFocus();
            m_vkb->hide(true);
        }
        else
        {
            m_vkb->show(l, focusWidget());
            m_vkb->move(x(), y());
        }
    }
    else
    {
        qDebug() << "virtual keyboard is not created";
    }
}

void password::focusLine(vkILine *l)
{
    Q_ASSERT(m_vkb);

    if (m_vkb->isVisible() == false)
        return;

    if (m_vkb->currentTextBox() == l)
        return;
}

void password::hideVKB()
 {
    QString distance;
    distance = m_passHolder.text();
    closeVKB();
    if ((mMenuType == CMD_SERVICE && !distance.compare("135799")) ||
        (mMenuType == CMD_FACTORY && !distance.compare("231586%")))
    {
        Utils& u = Utils::get();
        u.setPasswordStatus(true);
        ui->pb_exit->clicked();
    }
}

void password::on_pb_exit_clicked()
{
    Utils& u = Utils::get();
    close();

    Q_ASSERT(mPb);
    if (mPb != NULL && u.passwordEntered() == true)
        mPb->clicked(); // Password entered, automatically go to the next menu
    else
        u.sendMbPacket( (unsigned char) CMD_KEYBOARD_BEEP, 0, NULL, NULL );
}
