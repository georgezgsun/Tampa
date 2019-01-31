#include "session.h"

Session::Session(Users *u)
{
    if (u) {
        m_user.firstName = u->firstName;
        m_user.lastName = u->lastName;
        m_user.loginName = u->loginName;
        m_user.bagdeNumber = u->bagdeNumber;
        m_user.password = u->password;
        m_user.userLevel = u->userLevel;
    }
    else {
        qWarning("Session: invalid parameter!");
    }

}
