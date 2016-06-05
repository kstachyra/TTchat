/*
 * bazowa klasa monitora, definiująca zmienne warunkowe i semafory
 * stworzona na potrzeby przedmiotu SOI
 * potrzebny monitor powinien po niej dziedziczyć
 */

#ifndef MONITOR_H
#define MONITOR_H

#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

class MonitorSemaphore
{
public:

	MonitorSemaphore( int value )
	{
		if( sem_init( & sem, 0, value ) != 0 )
			throw "sem_init: failed";
	}
	~MonitorSemaphore()
	{
		sem_destroy( & sem );
	}

	void p()
	{
		if( sem_wait( & sem ) != 0 )
			throw "sem_wait: failed";
	}

	void v()
	{
		if( sem_post( & sem ) != 0 )
			throw "sem_post: failed";
	}


private:

	sem_t sem;
};

class Condition
{
	friend class Monitor;

public:
	Condition() : w( 0 )
	{
		waitingCount = 0;
	}

	void wait()
	{
		w.p();
	}

	bool signal()
	{
		if( waitingCount )
		{
			-- waitingCount;
			w.v();
			return true;
		}//if
		else
			return false;
	}

private:
	MonitorSemaphore w;
	int waitingCount; //liczba oczekujacych watkow
};


class Monitor
{
public:
	Monitor() : s( 1 ) {}

	void enter()
	{
		s.p();
	}

	void leave()
	{
		s.v();
	}

	void wait( Condition & cond )
	{
		++ cond.waitingCount;
		leave();
		cond.wait();
	}

	void signal( Condition & cond )
	{
		if( cond.signal() )
			enter();
	}


private:
	MonitorSemaphore s;
};

#endif //MONITOR_H