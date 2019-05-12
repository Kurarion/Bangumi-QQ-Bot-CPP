#pragma once
//#include "Http.h"
//#include "Database.h"
#include "Init.h"
#include <iostream>



namespace Resolve {
	//Ϊ�˷�����дʹ�ú궨��
	//��wstringת��Ϊstring
#define ResolveConv(ws) boost::locale::conv::from_utf(ws, "UTF8")
	//����User
	std::pair<std::vector<std::shared_ptr<boost::thread>>, bangumi::BangumiUser&>
		Resolve_User(std::string json, bool refresh) {
		try
		{
			//��ȫ����Ҫ�ڵײ�����\u���н���,property_tree������������,ǰ����ʹ��wptree
			//��stringת��wstringֱ��ʹ��C++ 11�Ĺ��캯��
			std::wstring wjson(json.begin(), json.end());
			//�������
			boost::property_tree::wptree pt;
			//��Ϊ������Ҫһ��������,���ʹ��stringstream,Ҳ֧���ļ�����ȡ
			std::wistringstream input(wjson);
			//����json
			boost::property_tree::read_json(input, pt);
			//����Ԫ��,[ע��]:������Ķ���UTF����,����GBK,�������Locale����ת��


			//�����ж��Ƿ���Ч
			//TODO:�����Ч�ظ�
			int code = pt.get<int>(L"code", 200);
			if (code != 200) {
				//��������Ч��
#ifndef NDEBUG
				{
					bangumi::string debug_msg;
					debug_msg << "δ�ҵ����û�";
					CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Resolve-User", debug_msg);
				}
#endif
				//std::shared_ptr<bangumi::Msg_Interface> msg(new bangumi::Reply("�����û�ʧ��..."));
				//return{ ThreadVector, bangumi::Reply("�����û�ʧ��...") };
				//ֱ���׳��쳣�����ϲ�(Function)�ṩ������Ϣ
				throw boost::system::system_error(bangumi_bot_errors::empty_user);
			}

			size_t wid = pt.get<size_t>(L"id", 0);
			std::wstring wurl = pt.get<std::wstring>(L"url", L"???");
			std::wstring wuser_name = pt.get<std::wstring>(L"username", L"???");
			std::wstring wnick_name = pt.get<std::wstring>(L"nickname", L"???");
			std::wstring wava_url = pt.get<std::wstring>(L"avatar.large", L"");
			std::wstring wsign = pt.get<std::wstring>(L"sign", L"");		
			//int wuser_group = pt.get<int>(L"usergroup", 10);
			//һ��ת��������
			//std::string sign = ResolveConv(sign);

			//���صĲ���: �߳�
			std::vector<std::shared_ptr<boost::thread>> ThreadVector;
			//Pic url�����ȴ���
			std::string file_path;
			
			auto result = PicDownload(http_client, ResolveConv(wava_url), USER_PIC_PATH, std::to_string(wid), file_path, refresh);
			//���ص��̱߳��浽����ֵ��
			if (result.first == DownloadStatus::MultiThread)
			{
				//���߳�
				ThreadVector.push_back(result.second);		
			}
			//����User�ṹ�����
			//�Ƿ�ˢ�»���
			auto &user = BangumiAddUser(wid, ResolveConv(wurl), ResolveConv(wuser_name),
				ResolveConv(wnick_name), file_path, ResolveConv(wsign));
			//���ؽṹ��
			return{ ThreadVector,user };
			
		}
		catch (boost::system::system_error & e)
		{
			//Boost���������
			//ͬʱ����ͨ�����catch������ʧ����Ϣ
			throw e;
		}
		//property_tree�ڽ���ʧ��ʱ�������׳�system_error����exception
		catch (std::exception &e) {
#ifndef NDEBUG
			{
				bangumi::string debug_msg;
				debug_msg << e.what();
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Resolve-User", debug_msg);
			}
#endif
			//������������
			throw boost::system::system_error(bangumi_bot_errors::json_resolve_error);
			//std::cout << "Error! Code = "
			//	<< "Message = " << e.what() << std::endl;
		}
	}
	
	//����Subject
	std::pair<std::vector<std::shared_ptr<boost::thread>>, bangumi::BangumiSubject&>
		Resolve_Subject(std::string json, bool refresh) {
		try
		{
			//��ȫ����Ҫ�ڵײ�����\u���н���,property_tree������������,ǰ����ʹ��wptree
			//��stringת��wstringֱ��ʹ��C++ 11�Ĺ��캯��
			std::wstring wjson(json.begin(), json.end());
			//�������
			boost::property_tree::wptree pt;
			//��Ϊ������Ҫһ��������,���ʹ��stringstream,Ҳ֧���ļ�����ȡ
			std::wistringstream input(wjson);
			//����json
			boost::property_tree::read_json(input, pt);
			//����Ԫ��,[ע��]:������Ķ���UTF����,����GBK,�������Locale����ת��


			//�����ж��Ƿ���Ч
			//
			int code = pt.get<int>(L"code", 200);
			if (code != 200) {
				//��������Ч��
#ifndef NDEBUG
				{
					bangumi::string debug_msg;
					debug_msg << "δ�ҵ�����Ŀ";
					CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Resolve-Subject", debug_msg);
				}
#endif

				//ֱ���׳��쳣�����ϲ�(Function)�ṩ������Ϣ
				throw boost::system::system_error(bangumi_bot_errors::empty_subject);
			}

			size_t wid = pt.get<size_t>(L"id", 0);
			std::wstring wurl = pt.get<std::wstring>(L"url", L"");
			int wtype = pt.get<size_t>(L"type", 2);
			std::wstring wname = pt.get<std::wstring>(L"name", L"");
			std::wstring wname_cn = pt.get<std::wstring>(L"name_cn", L"");
			std::wstring wsummary = pt.get<std::wstring>(L"summary", L"");
			int weps = pt.get<int>(L"eps", 0);
			if (weps == 0) {
				//TODO:API�в�û�и���,����ͨ��HTML��ȡ

			}
			std::string wair_date_str = ResolveConv(pt.get<std::wstring>(L"air_date", L"0000-00-00"));
			//Ĭ����һ����Ч������
			boost::gregorian::date wair_date;
			if (wair_date_str.compare("0000-00-00") != 0) {
				//���json����һ����Ч������,��ֵ
				wair_date = boost::gregorian::from_string(wair_date_str);
			}

//#ifndef NDEBUG
//			{
//				bangumi::string debug_msg;
//				debug_msg << "wair_date_str = " << wair_date_str
//					>> "wair_date = " << boost::gregorian::to_iso_extended_string(wair_date);
//				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Resolve-Subject", debug_msg);
//			}
//#endif
			int wair_weekday = pt.get<int>(L"air_weekday", 0);
			int wrating_num = pt.get<int>(L"rating.total", 0);
			float wrating_score = pt.get<float>(L"rating.score", 0);
			int wrank = pt.get<int>(L"rank", 0);
			std::wstring wimage_url = pt.get<std::wstring>(L"images.large", L"");
			bangumi::Collection wcollection;
			wcollection.wish = pt.get<int>(L"collection.wish", 0);
			wcollection.collect = pt.get<int>(L"collection.collect", 0);
			wcollection.doing = pt.get<int>(L"collection.doing", 0);
			wcollection.on_hold = pt.get<int>(L"collection.on_hold", 0);
			wcollection.dropped = pt.get<int>(L"collection.dropped", 0);
			wcollection.type = wtype;

			//һ��ת��������
			//std::string sign = ResolveConv(sign);

			//���صĲ���: �߳�
			std::vector<std::shared_ptr<boost::thread>> ThreadVector;
			//Pic url�����ȴ���
			std::string file_path;

			auto result = PicDownload(http_client, ResolveConv(wimage_url), SUBJECT_PIC_PATH, std::to_string(wid), file_path, refresh);
			//���ص��̱߳��浽����ֵ��
			if (result.first == DownloadStatus::MultiThread)
			{
				//���߳�
				ThreadVector.push_back(result.second);
			}
			//����User�ṹ�����
			//�Ƿ�ˢ�»���
			auto &subject = BangumiAddSubject(wid, 
				ResolveConv(wurl), 
				wtype,
				ResolveConv(wname),
				ResolveConv(wname_cn), 
				ResolveConv(wsummary),
				weps,
				wair_date,
				wair_weekday,
				wrating_num,
				wrating_score,
				wrank,
				file_path, 
				wcollection);
			//���ؽṹ��
			return{ ThreadVector,subject };

		}
		catch (boost::system::system_error & e)
		{
			//Boost���������
			//ͬʱ����ͨ�����catch������ʧ����Ϣ
			throw e;
		}
		//property_tree�ڽ���ʧ��ʱ�������׳�system_error����exception
		catch (std::exception &e) {
#ifndef NDEBUG
			{
				bangumi::string debug_msg;
				debug_msg << e.what();
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Resolve-Subject", debug_msg);
			}
#endif
			//������������
			throw boost::system::system_error(bangumi_bot_errors::json_resolve_error);
			//std::cout << "Error! Code = "
			//	<< "Message = " << e.what() << std::endl;
		}
	}

	//����Auth
	bangumi::AuthReply
		Resolve_Auth(std::string json) {
		try
		{
			//����Auth�ٷְ�û�����ĳ��ַ�,ֱ��ʹ��Ĭ�ϵ�ptree
			boost::property_tree::ptree pt;
			//��Ϊ������Ҫһ��������,���ʹ��stringstream,Ҳ֧���ļ�����ȡ
			std::istringstream input(json);
			//����json
			boost::property_tree::read_json(input, pt);
			//����Ԫ��,[ע��]:������Ķ���UTF����,����GBK,�������Locale����ת��


			//�����ж��Ƿ���Ч
			std::string error = pt.get<std::string>("error", "");
			if (error.compare("")!=0) {
				//��������Ч��
				std::string error_msg = pt.get<std::string>("error_description", "");
#ifndef NDEBUG
				{
					bangumi::string debug_msg;
					debug_msg << "Oauth Post����ʧ��"
						>> "����: " << error_msg;
					CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Resolve-Auth", debug_msg);
				}
#endif

				//ֱ���׳��쳣�����ϲ�(Function)�ṩ������Ϣ
				throw boost::system::system_error(bangumi_bot_errors::auth_request_error);
			}

		
			std::string access_token = pt.get<std::string>("access_token", "");
			std::string token_type = pt.get<std::string>("token_type", "");
			size_t user_id = pt.get<size_t>("user_id", 0);
			std::string refresh_token = pt.get<std::string>("refresh_token", "");



			//���ؽṹ��
			return bangumi::AuthReply(access_token,token_type,user_id,refresh_token);

		}
		catch (boost::system::system_error & e)
		{
			//Boost���������
			//ͬʱ����ͨ�����catch������ʧ����Ϣ
			throw e;
		}
		//property_tree�ڽ���ʧ��ʱ�������׳�system_error����exception
		catch (std::exception &e) {
#ifndef NDEBUG
			{
				bangumi::string debug_msg;
				debug_msg << e.what();
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Resolve-Auth", debug_msg);
			}
#endif
			//������������
			throw boost::system::system_error(bangumi_bot_errors::json_resolve_error);
			//std::cout << "Error! Code = "
			//	<< "Message = " << e.what() << std::endl;
		}
	}


	//����Auth status
	//��������򷵻�0,���򷵻ض�Ӧ��bangumi_id
	size_t
		Resolve_Auth_Status(std::string json) {
		//{
		//	"access_token": "f1b2xxxxxxxxf9ac6de6b",
		//		"client_id" : "bgm10xxxxxx9805b",
		//		"user_id" : 42xxxx,
		//		"expires" : 155xxxx41,
		//		"scope" : null
		//}
		//{
		//	"error": "invalid_token",
		//		"error_description" : "The access token provided has expired"
		//}
		try
		{
			//����Auth�ٷְ�û�����ĳ��ַ�,ֱ��ʹ��Ĭ�ϵ�ptree
			boost::property_tree::ptree pt;
			//��Ϊ������Ҫһ��������,���ʹ��stringstream,Ҳ֧���ļ�����ȡ
			std::istringstream input(json);
			//����json
			boost::property_tree::read_json(input, pt);
			//����Ԫ��,[ע��]:������Ķ���UTF����,����GBK,�������Locale����ת��


			//�����ж��Ƿ���Ч
			std::string error = pt.get<std::string>("error", "");
			if (error.compare("") != 0) {
				//��������Ч��
				std::string error_msg = pt.get<std::string>("error_description", "");
#ifndef NDEBUG
				{
					bangumi::string debug_msg;
					debug_msg << "Oauth Post����ʧ��"
						>> "����: " << error_msg;
					CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Resolve-Auth-Status", debug_msg);
				}
#endif
				//ֱ�ӷ���
				return 0;
				//ֱ���׳��쳣�����ϲ�(Function)�ṩ������Ϣ
				//throw boost::system::system_error(bangumi_bot_errors::auth_error);
			}

			size_t user_id = pt.get<size_t>("user_id", 0);
			//std::string refresh_token = pt.get<std::string>("refresh_token", "");



			//���ؽṹ��
			return user_id;

		}
		catch (boost::system::system_error & e)
		{
			//Boost���������
			//ͬʱ����ͨ�����catch������ʧ����Ϣ
			throw e;
		}
		//property_tree�ڽ���ʧ��ʱ�������׳�system_error����exception
		catch (std::exception &e) {
#ifndef NDEBUG
			{
				bangumi::string debug_msg;
				debug_msg << e.what();
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Resolve-Auth-Status", debug_msg);
			}
#endif
			//������������
			throw boost::system::system_error(bangumi_bot_errors::json_resolve_error);
			//std::cout << "Error! Code = "
			//	<< "Message = " << e.what() << std::endl;
		}
	}

	//����Auth Refresh
	bangumi::AuthReply
		Resolve_Auth_Refresh(std::string json) {
		try
		{
			//����Auth�ٷְ�û�����ĳ��ַ�,ֱ��ʹ��Ĭ�ϵ�ptree
			boost::property_tree::ptree pt;
			//��Ϊ������Ҫһ��������,���ʹ��stringstream,Ҳ֧���ļ�����ȡ
			std::istringstream input(json);
			//����json
			boost::property_tree::read_json(input, pt);
			//����Ԫ��,[ע��]:������Ķ���UTF����,����GBK,�������Locale����ת��


			//�����ж��Ƿ���Ч
			std::string error = pt.get<std::string>("error", "");
			if (error.compare("") != 0) {
				//��������Ч��
				std::string error_msg = pt.get<std::string>("error_description", "");
#ifndef NDEBUG
				{
					bangumi::string debug_msg;
					debug_msg << "Oauth Post����ʧ��"
						>> "����: " << error_msg;
					CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Resolve-Auth", debug_msg);
				}
#endif

				//ֱ���׳��쳣�����ϲ�(Function)�ṩ������Ϣ
				throw boost::system::system_error(bangumi_bot_errors::auth_request_error);
			}


			std::string access_token = pt.get<std::string>("access_token", "");
			//std::string token_type = pt.get<std::string>("token_type", "");
			//size_t user_id = pt.get<size_t>("user_id", 0);
			std::string refresh_token = pt.get<std::string>("refresh_token", "");



			//���ؽṹ��
			return bangumi::AuthReply(access_token, "", 0, refresh_token);

		}
		catch (boost::system::system_error & e)
		{
			//Boost���������
			//ͬʱ����ͨ�����catch������ʧ����Ϣ
			throw e;
		}
		//property_tree�ڽ���ʧ��ʱ�������׳�system_error����exception
		catch (std::exception &e) {
#ifndef NDEBUG
			{
				bangumi::string debug_msg;
				debug_msg << e.what();
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Resolve-Auth-Refresh", debug_msg);
			}
#endif
			//������������
			throw boost::system::system_error(bangumi_bot_errors::json_resolve_error);
			//std::cout << "Error! Code = "
			//	<< "Message = " << e.what() << std::endl;
		}
	}


	//����User fixed collection
	std::pair<std::vector<std::shared_ptr<boost::thread>>, bangumi::BangumiUser&>
		Resolve_User_Process(std::string json, bool refresh) {
		//{
		//	"status": {
		//		"id": 3,
		//		"type" : "do",
		//		"name" : "�ڿ�"
		//	},
		//	"rating" : 0,
		//	"comment" : "",
		//	"private" : 0,
		//		"tag" : [
		//				""
		//			],
		//	"ep_status" : 21,
		//	"lasttouch" : 1529241163,
		//	"user" : {
		//		"id": 423387,
		//		"url" : "http://bgm.tv/user/423387",
		//		"username" : "423387",
		//		"nickname" : "Pariya",
		//		"avatar" : {
		//			"large": "http://lain.bgm.tv/pic/user/l/000/42/33/423387.jpg?r=1549859586",
		//			"medium" : "http://lain.bgm.tv/pic/user/m/000/42/33/423387.jpg?r=1549859586",
		//			"small" : "http://lain.bgm.tv/pic/user/s/000/42/33/423387.jpg?r=1549859586"
		//			},
		//		"sign" : "",
		//		"usergroup" : 10
		//		}
		//}
		//{
		//	"request": "/collection/21811?access_token=088e77fbxxxxxxx7b06c33a45d",
		//		"code" : 400,
		//		"error" : "40001 Error: Nothing found with that ID"
		//}
		try
		{

			//��ȫ����Ҫ�ڵײ�����\u���н���,property_tree������������,ǰ����ʹ��wptree
			//��stringת��wstringֱ��ʹ��C++ 11�Ĺ��캯��
			std::wstring wjson(json.begin(), json.end());
			//�������
			boost::property_tree::wptree pt;
			//��Ϊ������Ҫһ��������,���ʹ��stringstream,Ҳ֧���ļ�����ȡ
			std::wistringstream input(wjson);
			//����json
			boost::property_tree::read_json(input, pt);
			//����Ԫ��,[ע��]:������Ķ���UTF����,����GBK,�������Locale����ת��

			//���صĲ���: �߳�
			std::vector<std::shared_ptr<boost::thread>> ThreadVector;
			//������
			bangumi::BangumiUserProgress user_progress;
			//�����ж��Ƿ���Ч
			int code = pt.get<int>(L"code", 200);
			std::wstring werror= pt.get<std::wstring>(L"error", L"");
			if (werror == L"invalid_token") {
				//�û���token��ʱ��Ч,��Ҫˢ��
				//�����׳��쳣
				throw boost::system::system_error(bangumi_bot_errors::access_token_invalid);
			}
			if (code != 200) {
				//�û�û���ղش���Ŀ
#ifndef NDEBUG
				{
					bangumi::string debug_msg;
					debug_msg << "�û�û���ղش���Ŀ";
					CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Resolve-User-Process", debug_msg);
				}
#endif
				//std::shared_ptr<bangumi::Msg_Interface> msg(new bangumi::Reply("�����û�ʧ��..."));
				//return{ ThreadVector, bangumi::Reply("�����û�ʧ��...") };
				//ֱ���׳��쳣�����ϲ�(Function)�ṩ������Ϣ
				throw boost::system::system_error(bangumi_bot_errors::user_not_collect_this_subject);
			}
			else {
				//�������Ȳ���

				std::wstring status_name = pt.get<std::wstring>(L"status.name", L"");
				int rating = pt.get<int>(L"rating", 0);
				std::wstring comment = pt.get<std::wstring>(L"comment", L"");
				int ep_status = pt.get<int>(L"ep_status", 0);
				std::string progress ("0/");
				if (ep_status != 0) {
					//TODO:��ȡ�ܼ���
					//Ŀǰ�Ǽƻ����ϲ㺯������ȡ��
					progress = std::to_string(ep_status) + "/";
				}
				user_progress = { ResolveConv(status_name), rating, ResolveConv(comment), progress };

				

				//����user����
				size_t wid = pt.get<size_t>(L"user.id", 0);

				if (refresh == false) {
					//�����ˢ��
					try {
						auto &bgm_user = BangumiPreFindUser(wid);
						bgm_user.SetProgress(user_progress);
						//Ϊuser���Progress
						return{ ThreadVector,bgm_user };

					}
					catch (std::out_of_range) {
						//û���ҵ�
						//ֱ�ӽ�����һ��
					}
				}


				std::wstring wurl = pt.get<std::wstring>(L"user.url", L"???");
				std::wstring wuser_name = pt.get<std::wstring>(L"user.username", L"???");
				std::wstring wnick_name = pt.get<std::wstring>(L"user.nickname", L"???");
				std::wstring wava_url = pt.get<std::wstring>(L"user.avatar.large", L"");
				std::wstring wsign = pt.get<std::wstring>(L"user.sign", L"");
				//int wuser_group = pt.get<int>(L"usergroup", 10);
				//һ��ת��������
				//std::string sign = ResolveConv(sign);


				//Pic url�����ȴ���
				std::string file_path;

				auto result = PicDownload(http_client, ResolveConv(wava_url), USER_PIC_PATH, std::to_string(wid), file_path, refresh);
				//���ص��̱߳��浽����ֵ��
				if (result.first == DownloadStatus::MultiThread)
				{
					//���߳�
					ThreadVector.push_back(result.second);
				}
				//����User�ṹ�����
				//�Ƿ�ˢ�»���
				auto &user = BangumiAddUser(wid, ResolveConv(wurl), ResolveConv(wuser_name),
					ResolveConv(wnick_name), file_path, ResolveConv(wsign));
				//Ϊuser���Progress
				user.SetProgress(user_progress);

				//���ؽṹ��
				return{ ThreadVector,user };
			}

		}
		catch (boost::system::system_error & e)
		{
			//Boost���������
			//ͬʱ����ͨ�����catch������ʧ����Ϣ
			throw e;
		}
		//property_tree�ڽ���ʧ��ʱ�������׳�system_error����exception
		catch (std::exception &e) {
#ifndef NDEBUG
			{
				bangumi::string debug_msg;
				debug_msg << e.what();
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Resolve-User", debug_msg);
			}
#endif
			//������������
			throw boost::system::system_error(bangumi_bot_errors::json_resolve_error);
			//std::cout << "Error! Code = "
			//	<< "Message = " << e.what() << std::endl;
		}
	}

	//����Search
	std::pair<std::vector<std::shared_ptr<boost::thread>>, bangumi::string>
		Resolve_Search(std::string json, bool refresh) {
		try
		{
			if (json.find("<!DOCTYPE") != std::string::npos) {
				//˵�����������û�пɷ��صĽ����,Ҳ���ǲ����������Ŀ
				//�׳��쳣������Ϣ
				throw boost::system::system_error(bangumi_bot_errors::search_failed);
			}
			//��ȫ����Ҫ�ڵײ�����\u���н���,property_tree������������,ǰ����ʹ��wptree
			//��stringת��wstringֱ��ʹ��C++ 11�Ĺ��캯��
			std::wstring wjson(json.begin(), json.end());
			//�������
			boost::property_tree::wptree pt;
			//��Ϊ������Ҫһ��������,���ʹ��stringstream,Ҳ֧���ļ�����ȡ
			std::wistringstream input(wjson);
			//����json
			boost::property_tree::read_json(input, pt);
			//����Ԫ��,[ע��]:������Ķ���UTF����,����GBK,�������Locale����ת��


			//�����ж��Ƿ���Ч
			//ʵ�����ʧ���ǻ᷵��һ��HTMLҳ���
			int code = pt.get<int>(L"code", 200);
			if (code != 200) {
				//��������Ч��
#ifndef NDEBUG
				{
					bangumi::string debug_msg;
					debug_msg << "����ʧ��...";
					CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Resolve-Search", debug_msg);
				}
#endif

				//ֱ���׳��쳣�����ϲ�(Function)�ṩ������Ϣ
				throw boost::system::system_error(bangumi_bot_errors::empty_subject);
			}
			
			//
			size_t results = pt.get<size_t>(L"results", 0);
			// get_child�õ��������
			boost::property_tree::wptree list = pt.get_child(L"list");
			boost::property_tree::wptree::iterator pos = list.begin();
			//���صĲ���: �߳�
			std::vector<std::shared_ptr<boost::thread>> ThreadVector;
			//���صĲ���: ��Ϣ
			bangumi::string msg;
			//�ܼ������Ŀ��: 1000
			msg << "�ܼ������Ŀ��: " << results << "\n";
			int subject_pos = 0;
			for (; pos != list.end(); ++pos)
			{
				//ʹ�þֲ�����
				auto &pt = pos->second;
				//json �� Subject�������
				//ÿһ������һ��Subject mini

				size_t wid = pt.get<size_t>(L"id", 0);
				std::wstring wurl = pt.get<std::wstring>(L"url", L"");
				int wtype = pt.get<size_t>(L"type", 2);
				std::wstring wname = pt.get<std::wstring>(L"name", L"");
				std::wstring wname_cn = pt.get<std::wstring>(L"name_cn", L"");
				std::wstring wsummary = pt.get<std::wstring>(L"summary", L"");
				int weps = pt.get<int>(L"eps", 0);
				if (weps == 0) {
					//TODO:API�в�û�и���,����ͨ��HTML��ȡ

				}
				std::string wair_date_str = ResolveConv(pt.get<std::wstring>(L"air_date", L"0000-00-00"));
				//Ĭ����һ����Ч������
				boost::gregorian::date wair_date;
				if (wair_date_str.compare("0000-00-00") != 0) {
					//���json����һ����Ч������,��ֵ
					wair_date = boost::gregorian::from_string(wair_date_str);
				}
				int wair_weekday = pt.get<int>(L"air_weekday", 0);
				int wrating_num = pt.get<int>(L"rating.total", 0);
				float wrating_score = pt.get<float>(L"rating.score", 0);
				int wrank = pt.get<int>(L"rank", 0);
				std::wstring wimage_url = pt.get<std::wstring>(L"images.large", L"");
				bangumi::Collection wcollection;
				wcollection.wish = pt.get<int>(L"collection.wish", 0);
				wcollection.collect = pt.get<int>(L"collection.collect", 0);
				wcollection.doing = pt.get<int>(L"collection.doing", 0);
				wcollection.on_hold = pt.get<int>(L"collection.on_hold", 0);
				wcollection.dropped = pt.get<int>(L"collection.dropped", 0);
				wcollection.type = wtype;

				//һ��ת��������
				//std::string sign = ResolveConv(sign);

				//Pic url�����ȴ���
				std::string file_path;

				auto result = PicDownload(http_client, ResolveConv(wimage_url), SUBJECT_PIC_PATH, std::to_string(wid), file_path, refresh);
				//���ص��̱߳��浽����ֵ��
				if (result.first == DownloadStatus::MultiThread)
				{
					//���߳�
					ThreadVector.push_back(result.second);
				}
				//����User�ṹ�����
				//�Ƿ�ˢ�»���
				auto &subject = BangumiAddSubject(wid,
					ResolveConv(wurl),
					wtype,
					ResolveConv(wname),
					ResolveConv(wname_cn),
					ResolveConv(wsummary),
					weps,
					wair_date,
					wair_weekday,
					wrating_num,
					wrating_score,
					wrank,
					file_path,
					wcollection);
				//���ؽṹ��
				//return{ ThreadVector,subject };
				msg << subject.SearchGet(++subject_pos);

			}
			//����������е�Subject��
			//����ͳһ��Subject������,ֻ�������������ֱ�ӷ���Subject��Ҫ���ص�����SearchGet
			
			



			return{ ThreadVector,std::move(msg) };


		}
		catch (boost::system::system_error & e)
		{
			//Boost���������
			//ͬʱ����ͨ�����catch������ʧ����Ϣ
			throw e;
		}
		//property_tree�ڽ���ʧ��ʱ�������׳�system_error����exception
		catch (std::exception &e) {
#ifndef NDEBUG
			{
				bangumi::string debug_msg;
				debug_msg << e.what();
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Resolve-Search", debug_msg);
			}
#endif
			//������������
			throw boost::system::system_error(bangumi_bot_errors::json_resolve_error);
			//std::cout << "Error! Code = "
			//	<< "Message = " << e.what() << std::endl;
		}
	}

	//����Search Single ��Ҫ���ڽ�������Search
	std::pair<std::vector<std::shared_ptr<boost::thread>>, bangumi::BangumiSubject&>
		Resolve_Search_Singel(std::string json, bool refresh) {
		try
		{
			if (json.find("<!DOCTYPE") != std::string::npos) {
				//˵�����������û�пɷ��صĽ����,Ҳ���ǲ����������Ŀ
				//�׳��쳣������Ϣ
				throw boost::system::system_error(bangumi_bot_errors::search_failed);
			}
			//��ȫ����Ҫ�ڵײ�����\u���н���,property_tree������������,ǰ����ʹ��wptree
			//��stringת��wstringֱ��ʹ��C++ 11�Ĺ��캯��
			std::wstring wjson(json.begin(), json.end());
			//�������
			boost::property_tree::wptree pt;
			//��Ϊ������Ҫһ��������,���ʹ��stringstream,Ҳ֧���ļ�����ȡ
			std::wistringstream input(wjson);
			//����json
			boost::property_tree::read_json(input, pt);
			//����Ԫ��,[ע��]:������Ķ���UTF����,����GBK,�������Locale����ת��


			//�����ж��Ƿ���Ч
			//ʵ�����ʧ���ǻ᷵��һ��HTMLҳ���
			int code = pt.get<int>(L"code", 200);
			if (code != 200) {
				//��������Ч��
#ifndef NDEBUG
				{
					bangumi::string debug_msg;
					debug_msg << "����ʧ��...";
					CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Resolve-Search-Single", debug_msg);
				}
#endif

				//ֱ���׳��쳣�����ϲ�(Function)�ṩ������Ϣ
				throw boost::system::system_error(bangumi_bot_errors::search_failed);
			}

			//
			size_t results = pt.get<size_t>(L"results", 0);
			// get_child�õ��������
			boost::property_tree::wptree list = pt.get_child(L"list");
			boost::property_tree::wptree::iterator pos = list.begin();
			//���صĲ���: �߳�
			std::vector<std::shared_ptr<boost::thread>> ThreadVector;
			//���صĲ���: ��Ϣ
			//bangumi::string msg;
			//�ܼ������Ŀ��: 1000
			//msg << "�ܼ������Ŀ��: " << results << "\n";
			//int subject_pos = 0;

			//��Ϊʹ�ú�����ǰ����search_num = 1
			//ֻ�����һ�������Subject
			if(pos!=list.end())
			{
				//ʹ�þֲ�����
				auto &pt = pos->second;
				//json �� Subject�������
				//ÿһ������һ��Subject mini

				size_t wid = pt.get<size_t>(L"id", 0);
				std::wstring wurl = pt.get<std::wstring>(L"url", L"");
				int wtype = pt.get<size_t>(L"type", 2);
				std::wstring wname = pt.get<std::wstring>(L"name", L"");
				std::wstring wname_cn = pt.get<std::wstring>(L"name_cn", L"");
				std::wstring wsummary = pt.get<std::wstring>(L"summary", L"");
				int weps = pt.get<int>(L"eps", 0);
				if (weps == 0) {
					//TODO:API�в�û�и���,����ͨ��HTML��ȡ

				}
				std::string wair_date_str = ResolveConv(pt.get<std::wstring>(L"air_date", L"0000-00-00"));
				//Ĭ����һ����Ч������
				boost::gregorian::date wair_date;
				if (wair_date_str.compare("0000-00-00") != 0) {
					//���json����һ����Ч������,��ֵ
					wair_date = boost::gregorian::from_string(wair_date_str);
				}
				int wair_weekday = pt.get<int>(L"air_weekday", 0);
				int wrating_num = pt.get<int>(L"rating.total", 0);
				float wrating_score = pt.get<float>(L"rating.score", 0);
				int wrank = pt.get<int>(L"rank", 0);
				std::wstring wimage_url = pt.get<std::wstring>(L"images.large", L"");
				bangumi::Collection wcollection;
				wcollection.wish = pt.get<int>(L"collection.wish", 0);
				wcollection.collect = pt.get<int>(L"collection.collect", 0);
				wcollection.doing = pt.get<int>(L"collection.doing", 0);
				wcollection.on_hold = pt.get<int>(L"collection.on_hold", 0);
				wcollection.dropped = pt.get<int>(L"collection.dropped", 0);
				wcollection.type = wtype;

				//һ��ת��������
				//std::string sign = ResolveConv(sign);

				//Pic url�����ȴ���
				std::string file_path;

				auto result = PicDownload(http_client, ResolveConv(wimage_url), SUBJECT_PIC_PATH, std::to_string(wid), file_path, refresh);
				//���ص��̱߳��浽����ֵ��
				if (result.first == DownloadStatus::MultiThread)
				{
					//���߳�
					ThreadVector.push_back(result.second);
				}
				//����User�ṹ�����
				//�Ƿ�ˢ�»���
				auto &subject = BangumiAddSubject(wid,
					ResolveConv(wurl),
					wtype,
					ResolveConv(wname),
					ResolveConv(wname_cn),
					ResolveConv(wsummary),
					weps,
					wair_date,
					wair_weekday,
					wrating_num,
					wrating_score,
					wrank,
					file_path,
					wcollection);
				//���ؽṹ��
				return{ ThreadVector,subject };
				//msg << subject.SearchGet(++subject_pos);

			}
			else {
				//û��һ�����
				//ʵ�ʿ��ܲ������������,������Ϊ���Է���һ�׳�һ���쳣
				throw boost::system::system_error(bangumi_bot_errors::search_failed);
			}
			//����������е�Subject��
			//����ͳһ��Subject������,ֻ�������������ֱ�ӷ���Subject��Ҫ���ص�����SearchGet





			//return{ ThreadVector,std::move(msg) };


		}
		catch (boost::system::system_error & e)
		{
			//Boost���������
			//ͬʱ����ͨ�����catch������ʧ����Ϣ
			throw e;
		}
		//property_tree�ڽ���ʧ��ʱ�������׳�system_error����exception
		catch (std::exception &e) {
#ifndef NDEBUG
			{
				bangumi::string debug_msg;
				debug_msg << e.what();
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Resolve-Search", debug_msg);
			}
#endif
			//������������
			throw boost::system::system_error(bangumi_bot_errors::json_resolve_error);
			//std::cout << "Error! Code = "
			//	<< "Message = " << e.what() << std::endl;
		}
	}

	//����Collection �� progress����
	std::pair<std::vector<std::shared_ptr<boost::thread>>, bangumi::BangumiUser&>
		Resolve_Collect(std::string json, bool refresh) {
		//{
		//	"status": {
		//		"id": 3,
		//		"type" : "do",
		//		"name" : null
		//	},
		//	"rating" : 12,
		//	"comment" : "",
		//	"private" : 0,
		//	"tag" : [
		//	""
		//	],
		//	"ep_status" : 0,
		//	"lasttouch" : 1556333897,
		//	"user" : {
		//		"id": 423387,
		//		"url" : "http://bgm.tv/user/423387",
		//		"username" : "423387",
		//		"nickname" : "Pariya",
		//		"avatar" : {
		//			"large": "http://lain.bgm.tv/pic/user/l/000/42/33/423387.jpg?r=1549859586",
		//			"medium" : "http://lain.bgm.tv/pic/user/m/000/42/33/423387.jpg?r=1549859586",
		//			"small" : "http://lain.bgm.tv/pic/user/s/000/42/33/423387.jpg?r=1549859586"
		//			},
		//		"sign" : "",
		//		"usergroup" : 10
		//	}
		//}
		try
		{

			//��ȫ����Ҫ�ڵײ�����\u���н���,property_tree������������,ǰ����ʹ��wptree
			//��stringת��wstringֱ��ʹ��C++ 11�Ĺ��캯��
			std::wstring wjson(json.begin(), json.end());
			//�������
			boost::property_tree::wptree pt;
			//��Ϊ������Ҫһ��������,���ʹ��stringstream,Ҳ֧���ļ�����ȡ
			std::wistringstream input(wjson);
			//����json
			boost::property_tree::read_json(input, pt);
			//����Ԫ��,[ע��]:������Ķ���UTF����,����GBK,�������Locale����ת��

			//���صĲ���: �߳�
			std::vector<std::shared_ptr<boost::thread>> ThreadVector;
			//������
			bangumi::BangumiUserProgress user_progress;
			//�����ж��Ƿ���Ч
			int code = pt.get<int>(L"code", 200);
			std::wstring werror = pt.get<std::wstring>(L"error", L"");
			if (werror == L"invalid_token") {
				//�û���token��ʱ��Ч,��Ҫˢ��
				//�����׳��쳣
				throw boost::system::system_error(bangumi_bot_errors::access_token_invalid);
			}
			if (code != 200) {
				//�û�û���ղش���Ŀ
#ifndef NDEBUG
				{
					bangumi::string debug_msg;
					debug_msg << "��Ŀ�ղ�ʧ��";
					CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Resolve-Collection", debug_msg);
				}
#endif
				//std::shared_ptr<bangumi::Msg_Interface> msg(new bangumi::Reply("�����û�ʧ��..."));
				//return{ ThreadVector, bangumi::Reply("�����û�ʧ��...") };
				//ֱ���׳��쳣�����ϲ�(Function)�ṩ������Ϣ
				throw boost::system::system_error(bangumi_bot_errors::subject_collect_failed);
			}
			else {
				//�������Ȳ���

				//std::wstring status_name = pt.get<std::wstring>(L"status.name", L"");
				//�����ղط��ص�name��Null Ӧ����id����
				int status_id = pt.get<int>(L"status.id", 0);
				std::string status_name;
				try {
					status_name = bangumi::Bangumi_Collect_Status.at(status_id);
				}
				catch (std::out_of_range&) {
					//δ֪��״̬
					status_name = "???";
				}
				
				int rating = pt.get<int>(L"rating", 0);
				std::wstring comment = pt.get<std::wstring>(L"comment", L"");
				//int ep_status = pt.get<int>(L"ep_status", 0);
				//�������API��������0,ֱ���趨progress = "" �����
				std::string progress("");
				//if (ep_status != 0) {
				//	//TODO:��ȡ�ܼ���
				//	//Ŀǰ�Ǽƻ����ϲ㺯������ȡ��
				//	progress = std::to_string(ep_status) + "/";
				//}
				//[ע��]������ַ�ȫ����UTF-8
				user_progress = { status_name, rating, ResolveConv(comment), progress };



				//����user����
				size_t wid = pt.get<size_t>(L"user.id", 0);

				if (refresh == false) {
					//�����ˢ��
					try {
						auto &bgm_user = BangumiPreFindUser(wid);
						bgm_user.SetProgress(user_progress);
						//Ϊuser���Progress
						return{ ThreadVector,bgm_user };

					}
					catch (std::out_of_range) {
						//û���ҵ�
						//ֱ�ӽ�����һ��
					}
				}


				std::wstring wurl = pt.get<std::wstring>(L"user.url", L"???");
				std::wstring wuser_name = pt.get<std::wstring>(L"user.username", L"???");
				std::wstring wnick_name = pt.get<std::wstring>(L"user.nickname", L"???");
				std::wstring wava_url = pt.get<std::wstring>(L"user.avatar.large", L"");
				std::wstring wsign = pt.get<std::wstring>(L"user.sign", L"");
				//int wuser_group = pt.get<int>(L"usergroup", 10);
				//һ��ת��������
				//std::string sign = ResolveConv(sign);


				//Pic url�����ȴ���
				std::string file_path;

				auto result = PicDownload(http_client, ResolveConv(wava_url), USER_PIC_PATH, std::to_string(wid), file_path, refresh);
				//���ص��̱߳��浽����ֵ��
				if (result.first == DownloadStatus::MultiThread)
				{
					//���߳�
					ThreadVector.push_back(result.second);
				}
				//����User�ṹ�����
				//�Ƿ�ˢ�»���
				auto &user = BangumiAddUser(wid, ResolveConv(wurl), ResolveConv(wuser_name),
					ResolveConv(wnick_name), file_path, ResolveConv(wsign));
				//Ϊuser���Progress
				user.SetProgress(user_progress);

				//���ؽṹ��
				return{ ThreadVector,user };
			}

		}
		catch (boost::system::system_error & e)
		{
			//Boost���������
			//ͬʱ����ͨ�����catch������ʧ����Ϣ
			throw e;
		}
		//property_tree�ڽ���ʧ��ʱ�������׳�system_error����exception
		catch (std::exception &e) {
#ifndef NDEBUG
			{
				bangumi::string debug_msg;
				debug_msg << e.what();
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Resolve-User", debug_msg);
			}
#endif
			//������������
			throw boost::system::system_error(bangumi_bot_errors::json_resolve_error);
			//std::cout << "Error! Code = "
			//	<< "Message = " << e.what() << std::endl;
		}
	}

	//����Update 
	bool
		Resolve_Update(std::string json, bool refresh) {
		//{
		//	"request": "/subject/226750/update/watched_eps",
		//	"code" : 202,
		//	"error" : "Accepted"
		//}
		//{
		//	"request": "/subject/22/update/watched_eps",
		//	"code" : 400,
		//	"error" : "Bad Request"
		//}
		try
		{
			//������
			boost::property_tree::ptree pt;
			//��Ϊ������Ҫһ��������,���ʹ��stringstream,Ҳ֧���ļ�����ȡ
			std::istringstream input(json);
			//����json
			boost::property_tree::read_json(input, pt);
			//����Ԫ��,[ע��]:������Ķ���UTF����,����GBK,�������Locale����ת��
			int code = pt.get<int>("code", 400);
			if (code == 202) {
				//˵�����³ɹ�,����ʧ��
				return true;
			}
			else {
				//ʧ�ܵĸ���
				return false;
			}
		

		}
		catch (boost::system::system_error & e)
		{
			//Boost���������
			//ͬʱ����ͨ�����catch������ʧ����Ϣ
			throw e;
		}
		//property_tree�ڽ���ʧ��ʱ�������׳�system_error����exception
		catch (std::exception &e) {
#ifndef NDEBUG
			{
				bangumi::string debug_msg;
				debug_msg << e.what();
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Resolve-User", debug_msg);
			}
#endif
			//������������
			throw boost::system::system_error(bangumi_bot_errors::json_resolve_error);
			//std::cout << "Error! Code = "
			//	<< "Message = " << e.what() << std::endl;
		}
	}

	//����Subject�е�Tag
	inline bangumi::string ResolveSubjectTag(const std::string &html) {
		bangumi::string ret("\n<-��ǩ->:\n");
		//ֱ�Ӵ�1000������
		size_t tag_start = html.find("class=\"subject_tag_section\">",1000);
		//�ж��Ƿ����
		if (tag_start == std::string::npos) {
			ret = "δ��¼��ǩ...";
			return ret;
		}
		//��Ϊֻ�������,Ϊ�˷�ֹ����,����С���ķ�Χ
		size_t tag_end = html.find("</small></a></div>", tag_start) - 11;
		size_t single_tag_start = tag_start;
		size_t single_tag_end;
		while (single_tag_start < tag_end) {
			single_tag_start = html.find("<span>", single_tag_start) + 6;
			single_tag_end = html.find("</span>", single_tag_start);
			//����Tag������
			ret << html.substr(single_tag_start , single_tag_end - single_tag_start);
			single_tag_start = html.find("\">", single_tag_end) + 2;
			single_tag_end = html.find("</small>", single_tag_start);
			//����Tag������
			ret <<'<'<<html.substr(single_tag_start , single_tag_end - single_tag_start)<<"> | ";

			single_tag_start = single_tag_end;
		}
		ret.erase(ret.find_last_of('|'),1);
		

		return std::move(ret);
	}

	//����Subject�еĽ�ɫ
	inline std::pair<bangumi::string,bangumi::string> ResolveSubjectCharacter(const std::string &html, bool refresh = false) {
		bangumi::string ret("\n<-��ɫ->:\n");
		//bangumi::string ret2;
		//ֱ�Ӵ�1500������
		size_t character_start = html.find("class=\"subject_section clearit\">",1500);
		//�ж��Ƿ����
		if (character_start == std::string::npos) {
			return{"",""};
		}
		//��Ϊֻ�������,Ϊ�˷�ֹ����,����С���ķ�Χ
		size_t character_end = html.find("class=\"more\">", character_start) - 11;
		//Ĭ�Ͽ�ʼλ��
		size_t single_character_start = character_start;
		//��β����Ϊ��һ��character��ʼλ��
		size_t single_character_end = html.find("title=\"", single_character_start) + 7;
		size_t temp;
		size_t pic_start;
		size_t pic_end;
		size_t job_start;
		size_t job_end;
		size_t cv_start;
		size_t cv_end;
		size_t pic_name_end;
		//PICͼƬ�����߳�
		std::vector<std::shared_ptr<boost::thread>> ThreadVector;
		//�ָ��������Ϣ ��ֹ�����޷�����
		int num = 0;
		const int ONE_MAX_NUM = 4;
		while (single_character_end < character_end) {
			++num;
			//
			single_character_start = single_character_end;
			//��ʵ������һ��Character�Ŀ�ʼ
			single_character_end = html.find("title=\"", single_character_start) + 7;
			//
			std::string name;
			std::string pic_url;
			std::string job_tip;
			std::string cv;
			std::string pic_save_name;
			//PICDownload���ļ�����λ��
			std::string file_path;
			//name�Ľ�βλ��
			temp = html.find("\"", single_character_start);
			name = html.substr(single_character_start, temp - single_character_start);
			//Ϊ�˼ӿ��ٶ�,ֱ�Ӽ�50����������
			pic_start = html.find("url('//", temp + 50) + 7;
			pic_end = html.find("')\"", pic_start);
			pic_url = "http://"+html.substr(pic_start, pic_end - pic_start);
			//�ҵ��ļ����Ŀ�ͷ
			temp = pic_url.find_last_of('/') + 1;
			//�ҵ��ļ�����β
			pic_name_end = pic_url.find_last_of('.');
			//ͼƬ����
			pic_save_name = pic_url.substr(temp, pic_name_end - temp );
			//��ͼƬ��С����m����
			temp = pic_url.find("/s/");
			pic_url[temp + 1] = 'm';
			//����ͼƬ
#ifndef NDEBUG
			{
				bangumi::string debug_msg;
				debug_msg << "���Ǵ�����ʼ > " << num;
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-R-PIC-DOWNLOAD", debug_msg);
			}
#endif
			auto result = PicDownload(http_client, pic_url, CHARACTER_PIC_PATH, pic_save_name, file_path, refresh);
#ifndef NDEBUG
			{
				bangumi::string debug_msg;
				debug_msg << "���Ǵ������� > " << num;
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-R-PIC-DOWNLOAD", debug_msg);;
			}
#endif
			//���ص��̱߳��浽����ֵ��
			if (result.first == DownloadStatus::MultiThread)
			{
				//�������߳�ѹ���̳߳���
				ThreadVector.push_back(result.second);
			}
			//��Աλ��
			job_start = html.find("job_tip\">", pic_end + 10) + 9;
			job_end = html.find("</s", job_start);
			job_tip = html.substr(job_start, job_end - job_start);
			//CV(����û��)
			cv = html.substr(job_end, single_character_end - job_end);
			cv_start = cv.find("arring\">");
			if (cv_start != std::string::npos) {
				cv_end = cv.find("</a>", cv_start);
				cv = cv.substr(cv_start + 8, cv_end - cv_start - 8);
			}
			else {
				cv = "";
			}
			//ret�ظ�
			//if (num < ONE_MAX_NUM) {
				ret << "[CQ:image,file=" << std::move(file_path) << "]"
					>> '[' << std::move(job_tip) << ']'
					>> std::move(name);
				if (!cv.empty())
					ret >> "CV:  " << std::move(cv);
				ret << "\n";
			//}
			//else {
			//	ret2 << "[CQ:image,file=" << std::move(file_path) << "]"
			//		>> '[' << std::move(job_tip) << ']'
			//		>> std::move(name);
			//	if (!cv.empty())
			//		ret2 >> "CV:  " << std::move(cv);
			//	ret2 << "\n";
			//}

	
		}
		//�ȴ�ͼƬ�������
		for (auto &t : ThreadVector) {
			if (t != nullptr&&t->joinable())
				t->join();
		}

		return{ std::move(ret),"" };
	}

	//����Subject�е��²�
	inline bangumi::string ResolveSubjectComment(const std::string &raw_html, bool refresh = false) {
		bangumi::string ret("\n<-�²�->:\n");
		//ֱ�Ӵ�1500������
		size_t comment_start = raw_html.find("\"comment_box\"", 2500);
		//�ж��Ƿ����,��ʵһ������,ʵ���Ͽ����ǻ�Ա���ܿ�������Ŀ
		if (comment_start == std::string::npos) {
			ret = "\nδ��¼�²�...\n";
			return ret;
		}
		//��Ϊֻ�������,Ϊ�˷�ֹ����,����С���ķ�Χ
		size_t comment_end = raw_html.find("class=\"more\">", comment_start) - 11;
		//ֱ��
		std::string html = raw_html.substr(comment_start, comment_end - comment_start);
		//
		//��һ����ʼλ��
		size_t single_comment_start = html.find("url('//") + 7;
		//
		//size_t single_comment_end ;
		//����
		size_t temp;
		//size_t pic_start;
		size_t pic_end;
		size_t name_start;
		size_t name_end;
		size_t time_start;
		size_t time_end;
		size_t content_start;
		size_t content_end;
		size_t pic_name_end;
		//PICͼƬ�����߳�
		std::vector<std::shared_ptr<boost::thread>> ThreadVector;

		while ((single_comment_start-7) !=std::string::npos ) {
			//��ʱ����
			std::string name;
			std::string pic_url;
			std::string comment_time;
			std::string comment;
			char star = '0';
			std::string pic_save_name;
			//PICDownload���ļ�����λ��
			std::string file_path;
			//ͷ��ͼƬStart=====

			//�ҵ�ͷ��Ľ�β
			pic_end = html.find("')\"", single_comment_start);
			pic_url = "http://" + html.substr(single_comment_start, pic_end - single_comment_start);
			//�ҵ��ļ����Ŀ�ͷ
			temp = pic_url.find_last_of('/') + 1;
			//�ҵ��ļ�����β
			pic_name_end = pic_url.find_last_of('.');
			//ͼƬ����
			pic_save_name = pic_url.substr(temp, pic_name_end - temp);
			//��ͼƬ��С����l����
			temp = pic_url.find("/s/");
			pic_url[temp + 1] = 'l';
			//����ͼƬ
			auto result = PicDownload(http_client, pic_url, USER_PIC_PATH, pic_save_name, file_path, refresh);
			//���ص��̱߳��浽����ֵ��
			if (result.first == DownloadStatus::MultiThread)
			{
				//�������߳�ѹ���̳߳���
				ThreadVector.push_back(result.second);
			}
			//ͷ��ͼƬOVER=====

			//name===
			name_start = html.find("class=\"l\">", pic_end) + 10;
			name_end = html.find("</a>", name_start);
			name = html.substr(name_start, name_end - name_start);
			//name end===

			//time===
			time_start = html.find("=\"grey\">", name_end) + 8;
			time_end = html.find("</smal", time_start);
			comment_time = html.substr(time_start, time_end - time_start);
			//time end===

			//comment===
			content_start = html.find("<p>", time_end) + 3;
			content_end = html.find("</p>", content_start);
			comment = html.substr(content_start, content_end - content_start);
			//comment end===

			//star===
			temp = html.find("stars", time_end);
			if (temp != std::string::npos&&temp<content_start) {
				star = html[temp + 5];
			}
			//star end===

			//�¸�ͷ��URL
			single_comment_start = html.find("url('//", single_comment_start) + 7;

			//ret�ظ�
			ret << "[CQ:image,file=" << std::move(file_path) << "]"
				>> std::move(name) << ' ' << std::move(comment_time);
			if (star != '0') {
				ret >> ">>����: " << star;
			}
			ret	>> ">>" << std::move(comment);

			ret << "\n";

		}
		//�ȴ�ͼƬ�������
		for (auto &t : ThreadVector) {
			if (t != nullptr&&t->joinable())
				t->join();
		}

		return std::move(ret);
	}

	//����Subject�еĲ�����Ϣ
	inline bangumi::BangumiSubjectCollection ResolveSubjectCollection(const std::string &html, size_t subject_id, bool refresh = false) {
#define  MAX_SUBJECT_COLLECTION_EPS 120
		//���صĽ��
		bangumi::BangumiSubjectCollection result;
		result.subject_id = subject_id;
		//ֱ�Ӵ�400������
		size_t main_start = html.find("id=\"main\"", 400);
		//�ж��Ƿ����,��ʵһ������,ʵ���Ͽ����ǻ�Ա���ܿ�������Ŀ
		if (main_start == std::string::npos) {
			result.valid = false;
			return result;
		}
		//main֮����title
		std::string name_cn;
		size_t title_start = html.find("title=\"", main_start);
		//�������
		if (title_start != std::string::npos){
			size_t title_end = html.find("\"", title_start + 7);
			name_cn = html.substr(title_start + 7, title_end - title_start - 7);
		}
		result.name_cn = name_cn;

		//������֮����ԭ��
		std::string name;
		size_t name_start = html.find("viewed\">", main_start);
		//�������
		if (name_start != std::string::npos) {
			size_t name_end = html.find("</a>", name_start);
			name = html.substr(name_start + 8, name_end - name_start - 8);
		}
		result.name = name;

		//ͼƬ
		size_t info_start = html.find("\"bangumiInfo\"", main_start);
		if (info_start == std::string::npos) {
			result.valid = false;
			return result;
		}
		size_t infobox_end = html.find("id=\"infobox\"", info_start);
		std::string info_str = html.substr(info_start + 13, infobox_end - info_start - 13);

		//
		size_t pic_start = info_str.find("href=\"");
		std::string file_path;
		//PICͼƬ�����߳�
		std::vector<std::shared_ptr<boost::thread>> ThreadVector;
		if (pic_start != std::string::npos) {
			//����ͼƬ
			size_t pic_end = info_str.find("\"", pic_start + 6);
			std::string pic_url = "http:" + info_str.substr(pic_start + 6, pic_end - pic_start - 6);

			//ͼƬ���� (����subject_id)
			std::string pic_save_name = std::to_string(subject_id);

			//����ͼƬ
			auto result = PicDownload(http_client, pic_url, SUBJECT_PIC_PATH, pic_save_name, file_path, refresh);

			//���ص��̱߳��浽����ֵ��
			if (result.first == DownloadStatus::MultiThread)
			{
				//�������߳�ѹ���̳߳���
				ThreadVector.push_back(result.second);
			}
		}
		result.file_path = file_path;
		//ͼƬOVER=====


		//�½�
		int eps_num = 1;
		size_t chapter_start = html.find("prg_list\">", main_start);
		if (chapter_start != std::string::npos ) {
			
			size_t chapter_end = html.find("</ul>", chapter_start);
			std::string chapter = html.substr(chapter_start + 10, chapter_end - chapter_start - 10);
			//����Ƿ���SP
			bool have_sp = false;
			size_t sp_pos = chapter.find("subtitle");
			if (sp_pos != std::string::npos) {
				//˵������SP
				have_sp = true;
			}
			//ѭ������ÿһ���½�
			size_t ep_status = chapter.find("epBtn");
			size_t ep_title_start;
			size_t ep_title_end;
			std::string ep_name;
			//ѭ�������½�
			while (ep_status!= std::string::npos&& eps_num< MAX_SUBJECT_COLLECTION_EPS){
				++eps_num;
				//
				//title
				ep_title_start = chapter.find("title=\"", ep_status);
				ep_title_end = chapter.find("\"", ep_title_start + 7);
				//ep name
				ep_name = chapter.substr(ep_title_start + 7, ep_title_end - ep_title_start - 7);
				//�жϷ���״̬
				if (chapter[ep_status+5]=='A'){
					//˵����epBtnAir
					if (have_sp&&ep_status > sp_pos) {
						//˵����aired��sp
						result.sp_air_eps.push_back(ep_name);
					}
					else {
						//˵����aired��tv
						result.air_eps.push_back(ep_name);
					}
				}
				else {
					//������epBtnNA
					if (have_sp&&ep_status > sp_pos) {
						//˵����unaired��sp
						result.sp_unair_eps.push_back(ep_name);
					}
					else {
						//˵����unaired��tv
						result.unair_eps.push_back(ep_name);
					}
				}
				//��һ��ep_status
				ep_status = chapter.find("epBtn", ep_title_start);

			}
			//����һ������
			result.UpdateEpsCounts();

		}


		//�½�Over


		//�ȴ�ͼƬ�������
		for (auto &t : ThreadVector) {
			if (t != nullptr&&t->joinable())
				t->join();
		}

		return result;
	}

	//����Tag��Ϣ
	//ֻ����һҳ
	inline bangumi::string ResolveTag(const std::string &raw_html, bool refresh = false) {
#define BGMTagMaxPageNum = 2
		//����tagĿ¼�Ŀ�ͷ
		size_t section_start = raw_html.find("section", 1000);
		//��������������
		if (section_start == std::string::npos) {
			//һ�����޷���html 502
			return "";
		}
		//���ҽ�β
		size_t section_end = raw_html.find("board", section_start);
		//��Ч��html
		std::string html = raw_html.substr(section_start, section_end - section_start);
		//������Ŀ����ʼλ��
		size_t subject_start = html.find("item_");
		//
		bangumi::string ret;
		//
		std::string subject_id;
		std::string pic_url;
		std::string file_path;
		std::string name_cn;
		std::string name;
		std::string tips;
		std::string rate;
		std::string rate_num;
		size_t temp;
		//
		//PICͼƬ�����߳�
		std::vector<std::shared_ptr<boost::thread>> ThreadVector;
		while (subject_start != std::string::npos) {
			
			//subject id
			size_t href_end = html.find("\"", subject_start + 5);
			
			subject_id = html.substr(subject_start + 5, href_end - subject_start - 5);
			//���ѭ��������һ��item
			//ͬʱ�޶�����Ŀ�Ĳ��ҷ�Χ
			subject_start = html.find("item_", subject_start + 10);
				
			//
			//std::cout << html<<std::endl;


			//������
			size_t name_cn_start = html.find("ass=\"l\">", href_end);
			size_t name_cn_end = html.find("</a>", name_cn_start + 8);
			name_cn = html.substr(name_cn_start + 8, name_cn_end - name_cn_start - 8);

			//������û��ͼƬ�ķ��գ���˽��жϷ���������֮��
			//ͼƬ���ص�ַ
			size_t src_start = html.find("src=\"/", href_end);
			//�������û��ͼƬ
			if (src_start  < name_cn_start) {
				size_t src_end = html.find("\"", src_start + 5);
				std::string pre_url = html.substr(src_start + 5, src_end - src_start - 5);
				std::string pic_subject_id = subject_id;
				if (pre_url[1] == 'i'&&pre_url[2] == 'm') {
					//img/no_icon
					pre_url = "//bgm.tv" + pre_url;
					pic_subject_id = "no_icon";
				}
				else {
					//��ͼƬ��С����l����
					temp = pre_url.find("/s/");
					pre_url[temp + 1] = 'm';
				}
				pic_url = "http:" + pre_url;
				//#ifndef NDEBUG
				//			{
				//				bangumi::string debug_msg;
				//				debug_msg << "ͼƬ���ص�ַ " << pic_url;
				//				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-TAG-PIC-DOWNLOAD", debug_msg);
				//			}
				//#endif
				//��ʼ����ͼƬ
				//����ͼƬ
				auto result = PicDownload(http_client, pic_url, TAG_PIC_PATH, pic_subject_id, file_path, refresh);
				//���ص��̱߳��浽����ֵ��
				if (result.first == DownloadStatus::MultiThread)
				{
					//�������߳�ѹ���̳߳���
					ThreadVector.push_back(result.second);
				}
			}
			else {
				//ֱ��ʹ��404ͼƬ
				auto result = PicDownload(http_client, "", TAG_PIC_PATH, subject_id, file_path, refresh);
				//���ص��̱߳��浽����ֵ��
				//if (result.first == DownloadStatus::MultiThread)
				//{
				//	//�������߳�ѹ���̳߳���
				//	ThreadVector.push_back(result.second);
				//}
			}

			//ԭ��(����û��)
			name = "";
			size_t name_start = html.find("ass=\"grey\">", name_cn_end);
			if (name_start < subject_start) {
				//�ڴ���Ŀ�ķ�Χ����Ч
				size_t name_end = html.find("</small>", name_start + 11);
				name = html.substr(name_start + 11, name_end - name_start - 11);
			}
			
			//info tips
			size_t tips_start = html.find("tip\">", name_cn_end);
			size_t tips_end = html.find("</p>", tips_start + 5);
			tips = html.substr(tips_start + 5, tips_end - tips_start - 5);
			//����(����û��)
			rate = "";
			size_t rate_start = html.find("fade\">", tips_end);
			if (rate_start < subject_start) {
				//�ڴ���Ŀ�ķ�Χ����Ч
				size_t rate_end = html.find("<", rate_start + 6);
				rate = html.substr(rate_start + 6, rate_end - rate_start - 6);
			}
			//����2(����û��)
			rate_num = "";
			size_t rate2_start = html.find("tip_j\">", tips_end);
			if (rate2_start < subject_start) {
				//�ڴ���Ŀ�ķ�Χ����Ч
				size_t rate2_end = html.find("<", rate2_start + 7);
				rate_num = html.substr(rate2_start + 7, rate2_end - rate2_start - 7);
			}


			//ret�ظ�
			ret << "[CQ:image,file=" << std::move(file_path) << "]"
				>> "ID: " << subject_id
				>> std::move(name_cn);
			if (!name.empty()) {
				ret << " <" << std::move(name) << ">";
			}

			if (!tips.empty()){
				//tips�Դ�\n
				ret	<< tips;
			}
			if (!rate.empty()) {
				ret >> "����: " <<rate << " " << rate_num;
			}
			else if (!rate_num.empty()) {
				ret >> rate_num;
			}
			ret << "\n";
				

			
		}

		//�ȴ�ͼƬ�������
		for (auto &t : ThreadVector) {
			if (t != nullptr&&t->joinable())
				t->join();
		}
		//�����Ϊ��
		if (!ret.empty()) {
			ret[ret.length() - 1] = ' ';
		}
		
		return ret;
	}

	//����Staff
	inline bangumi::string ResolveStaff(const std::string &raw_html) {
		//����Staff�Ŀ�ͷ
		size_t staff_start = raw_html.find("id=\"infobox\"", 800);
		//��������������
		if (staff_start == std::string::npos) {
			//һ�����޷���html 502
			return "";
		}
		size_t staff_end = raw_html.find("</ul>", staff_start);
		std::string staff_str = raw_html.substr(staff_start + 13, staff_end - staff_start - 13);
		//std::cout << staff_str << std::endl;
		//����ʱ��Ҫ�Ĳ���
		std::istringstream output(staff_str);
		std::string line_str;
		//�ж��Ƿ���<>��,<>�ڵ���Ϣ����Ҫ
		bool drop = false;
		//���صĽ��
		bangumi::string result;
		//ѭ�������һ��
		while (getline(output, line_str)) {
			//����
			if (line_str.empty()){
				continue;
			}
			//ѭ������
			for (auto&c : line_str) {
				if (c == '<'){
					drop = true;
					continue;
				}
				if (c == '>') {
					drop = false;
					continue;
				}
				if (drop){
					continue;
				}
				else {
					result << c;
				}
			}
			//����
			result << '\n';
		}
		if (!result.empty()) {
			//ȥ�����Ļ���
			result[result.length() - 1] = ' ';
		}

		return result;
	}
}