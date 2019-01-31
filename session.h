#ifndef SESSION_H
#define SESSION_H

#include "db_types.h"

class Session
{
public:
    Session(struct Users *u);

    struct Users * user() { return &m_user; }
private:

    struct Users m_user;

};

#endif // SESSION_H
