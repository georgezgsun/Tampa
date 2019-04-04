#include "state.h"
#include <stdio.h>
#include <QDebug>

state::state()
{
}

state::~state()
{
}

void state::initialization( void )
{
  printf("%s(%d) INIT  statePtr %p m_state %d\n\r", __FILE__, __LINE__, &m_state, m_state );
  m_state = 0;
  m_state_playback = 0;
}

void state::setState ( int i )
{
  //qDebug() << "SETSTATE m_state " << m_state <<  "newState " << i;
  m_state = i;
}
int state::getState ( void )
{
  //qDebug() << "GETSTATE m_state " << m_state;
  return m_state;
}

void state::setPlaybackState ( int i )
{
  m_state_playback = i;
}

int state::getPlaybackState ( void )
{
  return m_state_playback;
}
