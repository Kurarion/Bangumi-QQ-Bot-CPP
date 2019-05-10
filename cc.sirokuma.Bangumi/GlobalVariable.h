#include <string>
#include <map>
#include <mutex>
//#include "direct.h"
//#include "io.h"
#include "Bangumi.h"
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/xml_parser.hpp"
#include "boost/property_tree/json_parser.hpp"
#include "boost/property_tree/ini_parser.hpp"
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
//#include <locale>
//#include <codecvt>
#include <fstream>
#include <exception>

#ifndef BANGUMI_GLOBALVARIABLE_H
#define BANGUMI_GLOBALVARIABLE_H
using namespace boost::property_tree;

//默认目录位置等等


struct BangumiBotVaribel
{
private:
	ptree pt;
	void ReadPath() {
		try {
			//读取配置
			read_ini(ini_include_path + Bangumi_ini_name, pt);
			bangumi_client_id = pt.get<std::string>("INI.BGM_Client_Id");
			bangumi_client_secret = pt.get<std::string>("INI.BGM_Client_Secret");
			bangumi_api_url = pt.get<std::string>("INI.BGM_API_URL", "api.bgm.tv");
			max_subject_map = pt.get<unsigned int>("INI.Max_Subject_Map", 20);
			max_user_map = pt.get<unsigned int>("INI.Max_User_Map", 50);
			max_time_work_num = pt.get<unsigned int>("INI.Max_Time_Work_Num", 12);
			threadpool_size = pt.get<unsigned int>("INI.Max_Thread_Pool_Num", 0);
			sql_url = pt.get<std::string>("SQL.SQL_Url");
			sql_user_name = pt.get<std::string>("SQL.SQL_User_Name");
			sql_password = pt.get<std::string>("SQL.SQL_Password");
			sql_db = pt.get<std::string>("SQL.SQL_DB", "bangumi");
			max_list = pt.get<unsigned int>("BOT.Max_List", 31);
			owner_qq = pt.get<std::string>("BOT.Owner_QQ");
			redirect_url = pt.get<std::string>("INI.Redirect_URL");
			card_image_url = pt.get<std::string>("INI.Card_Image_URL");
			server_port_num = pt.get<unsigned int>("INI.Server_Port_Num", 3333);
			not_found_pic_path = pt.get<std::string>("INI.Not_Found_Pic_Path", "Cache\\404.png");
			//由于酪Q发送图片,其图片文件必须在data/image内部,因此注释以下语句
			//cache_path = pt.get<std::string>("BOT.Cache_Path", ini_include_path + Bangumi_cache_name);
			cache_path =  Bangumi_cache_name;
			sqlpool_size = pt.get<unsigned int>("SQL.Max_SQL_Pool_Num", 3);
			//向Coolq提示
			CQ_addLog(ac, CQLOG_INFOSUCCESS, "Bot-Ini", "配置初始化完成!");
			CQ_addLog(ac, CQLOG_INFOSUCCESS, "Bot-Ini", GetConf());
		}
		catch (ini_parser_error e) {
			//读取错误
			CQ_addLog(ac, CQLOG_ERROR, "Bot-Ini", "配置读取失败");
			CQ_addLog(ac, CQLOG_ERROR, "Bot-Ini", e.what());
		}

	}


public:

	std::string Bangumi_App_Dir;
	std::string Bangumi_Img_Dir;
	std::string Bangumi_directory_name = "Bangumi\\";
	std::string Bangumi_ini_name = "Bangumi.ini";
	std::string Bangumi_cache_name = "Cache\\";

	std::string ini_include_path;
	std::string bangumi_client_id;
	std::string bangumi_client_secret;
	std::string bangumi_api_url;
	unsigned max_subject_map;
	unsigned max_user_map;
	std::string sql_url;
	std::string sql_user_name;
	std::string sql_password;
	std::string sql_db;
	unsigned int max_list;
	std::string owner_qq;
	std::string cache_path;
	unsigned sqlpool_size;
	unsigned threadpool_size;
	unsigned curr_thread_size;
	std::mutex thread_size_mux;

	std::string redirect_url;
	std::string card_image_url;

	unsigned server_port_num;
	//如果当前可用线程Init时为0，直接使用单线程
	bool use_single_thread = false;

	int max_time_work_num;
	BangumiBotVaribel() = default;
	std::string not_found_pic_path;

	//初始化
	void Init(const char *path) {
		Bangumi_App_Dir = path;
		//酷Q Pr/data/app/cc.sirokuma.Bangumi/
		Bangumi_Img_Dir = Bangumi_App_Dir.substr(0, Bangumi_App_Dir.find("data\\app")+5) + "image\\";
		ini_include_path = Bangumi_App_Dir + Bangumi_directory_name;
		ReadPath();
		//判断配置文件中是否已经设定
		if (threadpool_size == 0) {
			//线程池数量
			const unsigned int DEFAULT_THREAD_POOL_SIZE = 2;
			//通常在并行应用程序中用于查找最佳线程数数的通用公式是计算机乘以2的处理器数
			//使用std::thread::hardware_concurrency静态方法来获取处理器的数量
			//但是,此方法可能返回0
			threadpool_size = std::thread::hardware_concurrency() * 2;

			if (threadpool_size == 0)
				threadpool_size = DEFAULT_THREAD_POOL_SIZE;

			//一个是主线程,一个是HTTP默认开启的ios.run()异步处理线程
			curr_thread_size = threadpool_size - 2;

			//assert(curr_thread_size > 0);
			//CheckAccess(cache_path);
		}
		else
		{
			if (threadpool_size > 1) {
				curr_thread_size = threadpool_size - 2;
			}
			else {
				curr_thread_size = 0;
			}
		}
		//判断是否使用单线程
		if (curr_thread_size == 0)
		{
			use_single_thread = true;
		}


	}

	//返回配置信息
	bangumi::string GetConf() {
		bangumi::string ret_str;
		ret_str << "BGM_Client_Id: " << bangumi_client_id
			>> "BGM_Client_Secret: " << bangumi_client_secret
			>> "BGM_API_URL: " << bangumi_api_url
			>> "SQL_Url: " << sql_url
			>> "Max_Subject_Map: " << max_subject_map
			>> "Max_User_Map: " << max_user_map
			>> "SQL_DB: " << sql_db
			>> "SQL_User_Name: " << sql_user_name
			>> "SQL_Password: " << sql_password
			>> "SQL_MAX_POOL: " << sqlpool_size
			>> "Owner_QQ: " << owner_qq
			>> "Cache_Path: " << cache_path
			>> "Image_Path: " << Bangumi_Img_Dir
			//>> "Max_List: " << max_list
			>> "Redirect_URL: " << redirect_url
			>> "Card_Image_URL: " << card_image_url
			>> "Max_Time_Work_Num: " << max_time_work_num
			>> "Max_Thread_Pool_Num: " << threadpool_size
			>> "Cur_Thread_Pool_Num: " << curr_thread_size
			//>> "Card_Image_URL: " << card_image_url
			>> "Not_Found_Pic_Path: " << not_found_pic_path
			>> "Server_Port_Num: " << server_port_num
			>> "Use_Single_Thread: " << (use_single_thread ? "Yes" : "No");

		return ret_str;
	}

	//检查可用线程
	inline bool CheckThreadSize() {
		//首先判断是否是单线程
		if (use_single_thread)
			return false;
		//创建时会在声明域内自动保持加锁
		std::unique_lock<std::mutex>
			thread_lock(thread_size_mux);
		if (curr_thread_size < 1) {
			return false;
		}
		else {
			--curr_thread_size;
			return true;
		}
		//解锁(析构也会自动解锁)
		//thread_lock.unlock();
	}

	//线程完毕后增加可用线程
	inline void AddAVAThreadSize(){
		std::unique_lock<std::mutex>
			thread_lock(thread_size_mux);
		++curr_thread_size;
	}
};

//global variable 
//BGM的配置
static BangumiBotVaribel bgm;
//global variable 
//存放Subject以及User的Map

static std::map<size_t, bangumi::BangumiSubject> BGMSubject;
static std::map<size_t, bangumi::BangumiUser> BGMUser;




#endif
