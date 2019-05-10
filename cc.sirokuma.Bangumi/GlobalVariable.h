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

//Ĭ��Ŀ¼λ�õȵ�


struct BangumiBotVaribel
{
private:
	ptree pt;
	void ReadPath() {
		try {
			//��ȡ����
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
			//������Q����ͼƬ,��ͼƬ�ļ�������data/image�ڲ�,���ע���������
			//cache_path = pt.get<std::string>("BOT.Cache_Path", ini_include_path + Bangumi_cache_name);
			cache_path =  Bangumi_cache_name;
			sqlpool_size = pt.get<unsigned int>("SQL.Max_SQL_Pool_Num", 3);
			//��Coolq��ʾ
			CQ_addLog(ac, CQLOG_INFOSUCCESS, "Bot-Ini", "���ó�ʼ�����!");
			CQ_addLog(ac, CQLOG_INFOSUCCESS, "Bot-Ini", GetConf());
		}
		catch (ini_parser_error e) {
			//��ȡ����
			CQ_addLog(ac, CQLOG_ERROR, "Bot-Ini", "���ö�ȡʧ��");
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
	//�����ǰ�����߳�InitʱΪ0��ֱ��ʹ�õ��߳�
	bool use_single_thread = false;

	int max_time_work_num;
	BangumiBotVaribel() = default;
	std::string not_found_pic_path;

	//��ʼ��
	void Init(const char *path) {
		Bangumi_App_Dir = path;
		//��Q Pr/data/app/cc.sirokuma.Bangumi/
		Bangumi_Img_Dir = Bangumi_App_Dir.substr(0, Bangumi_App_Dir.find("data\\app")+5) + "image\\";
		ini_include_path = Bangumi_App_Dir + Bangumi_directory_name;
		ReadPath();
		//�ж������ļ����Ƿ��Ѿ��趨
		if (threadpool_size == 0) {
			//�̳߳�����
			const unsigned int DEFAULT_THREAD_POOL_SIZE = 2;
			//ͨ���ڲ���Ӧ�ó��������ڲ�������߳�������ͨ�ù�ʽ�Ǽ��������2�Ĵ�������
			//ʹ��std::thread::hardware_concurrency��̬��������ȡ������������
			//����,�˷������ܷ���0
			threadpool_size = std::thread::hardware_concurrency() * 2;

			if (threadpool_size == 0)
				threadpool_size = DEFAULT_THREAD_POOL_SIZE;

			//һ�������߳�,һ����HTTPĬ�Ͽ�����ios.run()�첽�����߳�
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
		//�ж��Ƿ�ʹ�õ��߳�
		if (curr_thread_size == 0)
		{
			use_single_thread = true;
		}


	}

	//����������Ϣ
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

	//�������߳�
	inline bool CheckThreadSize() {
		//�����ж��Ƿ��ǵ��߳�
		if (use_single_thread)
			return false;
		//����ʱ�������������Զ����ּ���
		std::unique_lock<std::mutex>
			thread_lock(thread_size_mux);
		if (curr_thread_size < 1) {
			return false;
		}
		else {
			--curr_thread_size;
			return true;
		}
		//����(����Ҳ���Զ�����)
		//thread_lock.unlock();
	}

	//�߳���Ϻ����ӿ����߳�
	inline void AddAVAThreadSize(){
		std::unique_lock<std::mutex>
			thread_lock(thread_size_mux);
		++curr_thread_size;
	}
};

//global variable 
//BGM������
static BangumiBotVaribel bgm;
//global variable 
//���Subject�Լ�User��Map

static std::map<size_t, bangumi::BangumiSubject> BGMSubject;
static std::map<size_t, bangumi::BangumiUser> BGMUser;




#endif
