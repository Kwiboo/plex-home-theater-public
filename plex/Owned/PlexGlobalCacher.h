
#ifndef _PLEXGLOBALCACHER_H_
#define _PLEXGLOBALCACHER_H_

#include "VideoThumbLoader.h"
#include "threads/Thread.h"
#include "threads/Event.h"

class CPlexGlobalCacher : public CPlexThumbCacher, public CThread
{
public :
    static CPlexGlobalCacher* getGlobalCacher();
	~CPlexGlobalCacher();
	void Start();
	void Process();
	void OnExit();

private:
	CPlexGlobalCacher();
    static CPlexGlobalCacher* m_globalCacher;
    bool m_continue;

};

void controlGlobalCache();
#endif /* _PLEXGLOBALCACHER_H_*/ 
