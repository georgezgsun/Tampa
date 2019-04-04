#ifndef STATE_H
#define STATE_H

class state
{
public:
    static state& get() {
        static state instance;
        return instance;
    }

	state();
	~state();
	void initialization( void );
	
	void setState( int );
	int getState( void );

	void setPlaybackState( int );
	int getPlaybackState( void );

 protected:
	int m_state;
	int m_state_playback;
};
#endif
