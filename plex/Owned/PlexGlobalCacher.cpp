

#include "Owned/PlexGlobalCacher.h"
#include "FileSystem/PlexDirectory.h"
#include "Client/PlexServerDataLoader.h"
#include "PlexApplication.h"
#include "utils/Stopwatch.h"
#include "dialogs/GUIDialogKaiToast.h"
#include "guilib/GUITextBox.h"
#include "guilib/GUIEditControl.h"
#include "dialogs/GUIDialogYesNo.h"
#include "utils/log.h"
#include "filesystem/File.h"
#include "TextureCache.h"

using namespace XFILE;

#define GLOBAL_CACHING_DESC "Precaching Metadata"

CPlexGlobalCacher* CPlexGlobalCacher::m_globalCacher = NULL;

CPlexGlobalCacher::CPlexGlobalCacher() : CPlexThumbCacher() , CThread("Plex Global Cacher")
{
    m_continue = true;
}

CPlexGlobalCacher::~CPlexGlobalCacher()
{
}


CPlexGlobalCacher* CPlexGlobalCacher::getGlobalCacher()
{
    if (!m_globalCacher)   // Only allow one instance of class to be generated.
        m_globalCacher = new CPlexGlobalCacher();
    return m_globalCacher;
}


void CPlexGlobalCacher::Start()
{
	CLog::Log(LOGNOTICE,"Global Cache : Creating cacher thread");
	CThread::Create(true);
	CLog::Log(LOGNOTICE,"Global Cache : Cacher thread created");
}


void controlGlobalCache()
{
    CPlexGlobalCacher* cacher = CPlexGlobalCacher::getGlobalCacher();


    //virtual void StopThread(bool bWait = true); 
   // bool IsRunning() const;        
   
    if ( ! cacher->IsRunning() )
    {
        bool ok = CGUIDialogYesNo::ShowAndGetInput("Start global caching?",
        "This will cache all metadata and thumbnails automatically.",
        "Larger libraries will take longer to precache.",
        "Reboot for best performance once caching is complete.",
        "No!", "Yes");

        if ( ok )
        {
            cacher->Start();
            m_continue = true;
        }
    }
    else if ( cacher ->IsRunning() )
    {
        bool ok = CGUIDialogYesNo::ShowAndGetInput("Stop global caching?",
        "A reboot is recommended after stopping ",
        "the global caching process.",
        "Stopping may not occur right away.",
        "No!", "Yes");

        if ( ok )
        {
            CLog::Log(LOGNOTICE,"Global Cache : Cacher thread stopped by user");
            cacher -> StopThread(false);
            m_continue = false;
        }
    }
}



void CPlexGlobalCacher::Process()
{
	CPlexDirectory dir;
	CFileItemList list;
	CStopWatch timer;
	CStopWatch looptimer;
	CStdString Message;
	int Lastcount = 0;
	

			
	// get all the sections names
	CFileItemListPtr pAllSections = g_plexApplication.dataLoader->GetAllSections();

	timer.StartZero();
	CLog::Log(LOGNOTICE,"Global Cache : found %d Sections",pAllSections->Size());
	for (int i = 0; i < pAllSections->Size() && m_continue; i ++)
	{
		looptimer.StartZero();
		CFileItemPtr Section = pAllSections->Get(i);

		// Pop the notification
		Message.Format("Retrieving content from '%s'...", Section->GetLabel());
		CGUIDialogKaiToast::QueueNotification(CGUIDialogKaiToast::Info, Message, GLOBAL_CACHING_DESC, 5000, false,500);

		// gets all the data from one section
		CURL url(Section->GetPath());
		PlexUtils::AppendPathToURL(url, "all");

		// store them into the list
		dir.GetDirectory(url, list);
		
		CLog::Log(LOGNOTICE,"Global Cache : Processed +%d items in '%s' (total %d), took %f (total %f)",list.Size() - Lastcount,Section->GetLabel().c_str(),list.Size(), looptimer.GetElapsedSeconds(),timer.GetElapsedSeconds());
		Lastcount = list.Size();
	}

    // Here we have the file list, just process the items
    for (int i = 0; i < list.Size() && m_continue; i++)
    {
      CFileItemPtr item = list.Get(i);
      if (item->IsPlexMediaServer())
      {
          if (item->HasArt("thumb") &&
              !CTextureCache::Get().HasCachedImage(item->GetArt("thumb")))
            CTextureCache::Get().CacheImage(item->GetArt("thumb"));

          if (item->HasArt("fanart") &&
              !CTextureCache::Get().HasCachedImage(item->GetArt("fanart")))
            CTextureCache::Get().CacheImage(item->GetArt("fanart"));


          if (item->HasArt("grandParentThumb") &&
              !CTextureCache::Get().HasCachedImage(item->GetArt("grandParentThumb")))
            CTextureCache::Get().CacheImage(item->GetArt("grandParentThumb"));


          if (item->HasArt("bigPoster") &&
              !CTextureCache::Get().HasCachedImage(item->GetArt("bigPoster")))
            CTextureCache::Get().CacheImage(item->GetArt("bigPoster"));

      }

      CStdString Message;
      Message.Format("Processing %0.0f%%..." ,float(i * 100.0f) / (float)list.Size());
      CGUIDialogKaiToast::QueueNotification(CGUIDialogKaiToast::Info, Message, GLOBAL_CACHING_DESC, 8000, false,10);

    }

    CGUIDialogKaiToast::QueueNotification(CGUIDialogKaiToast::Info, "Processing Complete.", GLOBAL_CACHING_DESC, 5000, false,500);
}


void CPlexGlobalCacher::OnExit()
{

    CGUIDialogKaiToast::QueueNotification(CGUIDialogKaiToast::Info, "Precaching stopped.", GLOBAL_CACHING_DESC, 5000, false,500);

}
