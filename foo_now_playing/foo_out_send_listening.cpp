//author:medcl support:http://log.medcl.net  keep this.:)
#include "stdafx.h"
#include "../SDK/foobar2000.h"
#include "../SDK/component.h"
#include  "../curl/curl.h"
#include "resource.h"
#pragma comment(lib, "libcurl.lib")

DECLARE_COMPONENT_VERSION("foo_now_listening", "0.2", "Medcl's Now Listening Plugin For SinaMiniblo & Foobar 2000.<author:http://log.medcl.net mail:m@medcl.net> ");
VALIDATE_COMPONENT_FILENAME("foo_now_listening.dll");



//configuration begin

//config guid
static const GUID guid_cfg_username = { 0xbd5c777, 0x735c, 0x440d, { 0x8c, 0x71, 0x49, 0xb6, 0xac, 0xff, 0xce, 0xb8 } };
static const GUID guid_cfg_password = { 0x752f1186, 0x9f61, 0x4f91, { 0xb3, 0xee, 0x2f, 0x25, 0xb1, 0x24, 0x83, 0x5d } };

//default value
static  char default_cfg_username[]="yourname";
static  char default_cfg_password[]="password";

//config item
static  cfg_string cfg_username(guid_cfg_username,default_cfg_username);
static cfg_string cfg_password(guid_cfg_password,default_cfg_password);

class CMyPreferences : public CDialogImpl<CMyPreferences>, public preferences_page_instance {
public:
	//Constructor - invoked by preferences_page_impl helpers - don't do Create() in here, preferences_page_impl does this for us
	CMyPreferences(preferences_page_callback::ptr callback) : m_callback(callback) {}

	//Note that we don't bother doing anything regarding destruction of our class.
	//The host ensures that our dialog is destroyed first, then the last reference to our preferences_page_instance object is released, causing our object to be deleted.

	//dialog resource ID
	enum {IDD = IDD_MYPREFERENCES};
	// preferences_page_instance methods (not all of them - get_wnd() is supplied by preferences_page_impl helpers)
	t_uint32 get_state();
	void apply();
	void reset();

	//WTL message map
	BEGIN_MSG_MAP(CMyPreferences)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_HANDLER_EX(IDC_NAME, EN_CHANGE, OnEditChange)
		COMMAND_HANDLER_EX(IDC_PWD, EN_CHANGE, OnEditChange)
	END_MSG_MAP()
private:
	BOOL OnInitDialog(CWindow, LPARAM);
	void OnEditChange(UINT, int, CWindow);
	bool HasChanged();
	void OnChanged();

	const preferences_page_callback::ptr m_callback;
};

BOOL CMyPreferences::OnInitDialog(CWindow wnd, LPARAM)     
{	
	
	pfc::string_formatter msg; 
	msg << cfg_username;
		::uSetDlgItemText(*this, IDC_NAME, msg);

			pfc::string_formatter msg1; 
	msg1 << cfg_password;
		::uSetDlgItemText(*this, IDC_PWD, msg1);

	return FALSE;
}

void CMyPreferences::OnEditChange(UINT, int, CWindow) {
	// not much to do here
	OnChanged();
}

t_uint32 CMyPreferences::get_state() {
	t_uint32 state = preferences_state::resettable;
	if (HasChanged()) state |= preferences_state::changed;
	return state;
}

void CMyPreferences::reset() {
//	SetDlgItemText(IDC_NAME, default_cfg_username);
//	SetDlgItemText(IDC_PWD, default_cfg_password);
	OnChanged();
}

void CMyPreferences::apply() {
	uGetDlgItemText(*this,IDC_NAME,cfg_username);
	uGetDlgItemText(*this,IDC_PWD,cfg_password);
	
	OnChanged(); //our dialog content has not changed but the flags have - our currently shown values now match the settings so the apply button can be disabled
}

bool CMyPreferences::HasChanged() {
	//returns whether our dialog content is different from the current configuration (whether the apply button should be enabled or not)
	//simply to return true,may be you can check the value replace
	return true;
}
void CMyPreferences::OnChanged() {
	//tell the host that our state has changed to enable/disable the apply button appropriately.
	m_callback->on_state_changed();
}

class preferences_page_myimpl : public preferences_page_impl<CMyPreferences> {
	// preferences_page_impl<> helper deals with instantiation of our dialog; inherits from preferences_page_v3.
public:
	const char * get_name() {return "Medcl's Now Playing";}
	GUID get_guid() {
		// This is our GUID. Replace with your own when reusing the code.
		static const GUID guid = { 0x7702c93e, 0x24dc, 0x48ed, { 0x8d, 0xb1, 0x3f, 0x27, 0xb3, 0x8c, 0x7c, 0xc9 } };
		return guid;
	}
	GUID get_parent_guid() {return guid_tools;}
};

static preferences_page_factory_t<preferences_page_myimpl> g_preferences_page_myimpl_factory;

//configuration end

 //插件初始化及退出
class initquit_handler : public initquit {   
	virtual void on_init() {
		/*popup_message::g_complain("Hello world!");*/
		
	//全局初始化函数,只能被调用一次
	curl_global_init(CURL_GLOBAL_WIN32);//CURL_GLOBAL_SSL
	}
    //插件退出
	virtual void on_quit() {	
	curl_global_cleanup();
	}
};
static initquit_factory_t<initquit_handler> foo_initquit;

//配置页

//播放相关
class playback_handler: public play_callback_static {
    void on_playback_starting(play_control::t_track_command p_command,bool p_paused) {
	
	}
    void on_playback_stop(play_control::t_stop_reason p_reason) {}
    void on_playback_seek(double p_time) {}
    void on_playback_time(double p_time) {}
    void on_playback_pause(bool p_state) {}
    void on_playback_edited(metadb_handle_ptr p_track) {}
    void on_playback_dynamic_info(const file_info & p_info) {}
    void on_playback_dynamic_info_track(const file_info & p_info) {}
    void on_volume_change(float p_new_val) {}

    unsigned get_flags() { return flag_on_playback_new_track; }

static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
  popup_message::g_complain((char *)(ptr));
  console::printf("*** %s  %s  \n", stderr,(char *)(ptr));  
  return -1;
}
    void on_playback_new_track(metadb_handle_ptr p_track) {
	 
CURL *curl;
pfc::string8 authstr=cfg_username;
authstr+=":";
authstr+=cfg_password;

const char* URL="http://api.t.sina.com.cn/statuses/update.xml";		
pfc::string8 req_data="source=483394858&lat=0&long=0&status=%e6%88%91%e6%ad%a3%e5%9c%a8%e5%90%ac ";
		file_info_impl track_info;
		if (p_track->get_info(track_info))
		{
			const char* track_artist = track_info.meta_get("artist", 0);
			const char* track_title = track_info.meta_get("title", 0);
		//	const char* track_album = track_info.meta_get("album", 0);
			//if (track_album == NULL)
			//	track_album = "";
			if (track_artist == NULL)
			{
			track_artist="N/A ";
			}
			if (track_title != NULL)
			{
		/*	req_data+="";
			req_data+=track_album;
			req_data+=" ";*/
			req_data+=track_artist;
			req_data+="--";
			req_data+=track_title;

    curl = curl_easy_init();

    if(curl) {
		
		//设置url
       curl_easy_setopt(curl, CURLOPT_URL,URL);
	   
	   //设置basic验证信息
	   const char* authdata=authstr;
	   curl_easy_setopt(curl,CURLOPT_USERPWD,authdata);	   

	   //设置post字段信息
	   const char*  data=req_data;
	   curl_easy_setopt(curl,CURLOPT_POSTFIELDS,data);

	   //设置回调函数
	   curl_easy_setopt(curl,CURLOPT_READFUNCTION,read_callback);
	   
	   	//	popup_message::g_complain(authstr);
		//	popup_message::g_complain(req_data);
		//	popup_message::g_complain(URL);

	   //提交

       CURLcode res =
		   curl_easy_perform(curl);

	    if (res != CURLE_OK)
		  {
			  //popup_message::g_complain("fail!");
             console::printf( "failed to perform the request '%s' [%s]\n", URL,curl_easy_strerror(res));        
		}else{
			//popup_message::g_complain("success!");
              console::printf( "success to perform the request '%s' [%s]\n", URL,curl_easy_strerror(res));     
		}

       /* always cleanup */

       curl_easy_cleanup(curl);

			}		

		}

		return ;
    }
    }
};
static play_callback_static_factory_t<playback_handler> foo_playback;

