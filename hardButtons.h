#ifndef HARDBUTTONS_H
#define HARDBUTTONS_H

#include <QWidget>
#include "base_menu.h"
#include "top_view.h"

class hardButtons : public QObject
{
    Q_OBJECT

    static void *static_entry(void *objPtr)
    {
        hardButtons *myObj = static_cast<hardButtons *>(objPtr);
        myObj->ReadThread();
        return NULL;
    }

    int mDevFd;
    pthread_t mReadThreadId;
    int mReadThreadFlag;
    bool mEnabled;
    QPushButton *map[4];
    int MSG_ID;
    bool mKeyBoard;
    unsigned char mInputKey;
    topView* tv;

    void ReadThread(void);
    void ProcessHardButtons( struct input_event & );
    void Send_Audio_Beep(void);
    void Set_Audio_Volume(int volume);

public:
    static hardButtons& get()
    {
        static hardButtons instance;
        return instance;
    }

    hardButtons(void);
    ~hardButtons(void);
    int initialization();
    void EnableHardButtons(bool flag1) {mEnabled = flag1;}
    void setHardButtonMap( int , QPushButton * );
    void settopView( topView* );
    void setKeyBoardFlag(bool flag1) { mKeyBoard = flag1;}
#ifdef LIDARCAM
    void Send_Display_Brightness( int );
#endif
};

#endif // HARDBUTTONS_H
