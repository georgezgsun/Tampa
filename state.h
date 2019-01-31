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

 protected:
	int m_state;

};
#endif
