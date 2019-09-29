#pragma once
//#include "Http.h"
//#include "Database.h"
#include "Init.h"
#include "boost/date_time/posix_time/posix_time.hpp"
#include <iostream>
#include <bitset>



namespace Resolve {
	//为了方便书写使用宏定义
	//将wstring转换为string
#define ResolveConv(ws) boost::locale::conv::from_utf(ws, "UTF8")
#define UTF16ToGB(ws) code_converter.Conv(boost::locale::conv::from_utf(ws, "UTF8"))
	//解析User
	std::pair<std::vector<std::shared_ptr<boost::thread>>, bangumi::BangumiUser&>
		Resolve_User(std::string json, bool refresh) {
		try
		{
			//完全不需要在底层来对\u进行解码,property_tree会进行这项工作的,前提是使用wptree
			//将string转换wstring直接使用C++ 11的构造函数
			std::wstring wjson(json.begin(), json.end());
			//宽解析树
			boost::property_tree::wptree pt;
			//因为解析需要一个流输入,因此使用stringstream,也支持文件流读取
			std::wistringstream input(wjson);
			//解析json
			boost::property_tree::read_json(input, pt);
			//解析元素,[注意]:解析后的都是UTF编码,并非GBK,后面会用Locale进行转换


			//首先判断是否有效
			//TODO:完成无效回复
			int code = pt.get<int>(L"code", 200);
			if (code != 200) {
				//请求是无效的
#ifndef NDEBUG
				{
					bangumi::string debug_msg;
					debug_msg << "未找到该用户";
					CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Resolve-User", debug_msg);
				}
#endif
				//std::shared_ptr<bangumi::Msg_Interface> msg(new bangumi::Reply("查找用户失败..."));
				//return{ ThreadVector, bangumi::Reply("查找用户失败...") };
				//直接抛出异常来给上层(Function)提供构造信息
				throw boost::system::system_error(bangumi_bot_errors::empty_user);
			}

			size_t wid = pt.get<size_t>(L"id", 0);
			std::wstring wurl = pt.get<std::wstring>(L"url", L"???");
			std::wstring wuser_name = pt.get<std::wstring>(L"username", L"???");
			std::wstring wnick_name = pt.get<std::wstring>(L"nickname", L"???");
			std::wstring wava_url = pt.get<std::wstring>(L"avatar.large", L"");
			std::wstring wsign = pt.get<std::wstring>(L"sign", L"");		
			//int wuser_group = pt.get<int>(L"usergroup", 10);
			//一个转换的例子
			//std::string sign = ResolveConv(sign);

			//返回的参数: 线程
			std::vector<std::shared_ptr<boost::thread>> ThreadVector;
			//Pic url参数先处理
			std::string file_path;
			
			auto result = PicDownload(http_client, ResolveConv(wava_url), USER_PIC_PATH, std::to_string(wid), file_path, refresh);
			//返回的线程保存到返回值中
			if (result.first == DownloadStatus::MultiThread)
			{
				//多线程
				ThreadVector.push_back(result.second);		
			}
			//创建User结构体对象
			//是否刷新缓存
			auto &user = BangumiAddUser(wid, ResolveConv(wurl), ResolveConv(wuser_name),
				ResolveConv(wnick_name), file_path, ResolveConv(wsign));
			//返回结构体
			return{ ThreadVector,user };
			
		}
		catch (boost::system::system_error & e)
		{
			//Boost方面的问题
			//同时可以通过这个catch来构造失败消息
			throw e;
		}
		//property_tree在解析失败时并不会抛出system_error而是exception
		catch (std::exception &e) {
#ifndef NDEBUG
			{
				bangumi::string debug_msg;
				debug_msg << e.what();
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Resolve-User", debug_msg);
			}
#endif
			//解析树的问题
			throw boost::system::system_error(bangumi_bot_errors::json_resolve_error);
			//std::cout << "Error! Code = "
			//	<< "Message = " << e.what() << std::endl;
		}
	}
	
	//解析Subject
	std::pair<std::vector<std::shared_ptr<boost::thread>>, bangumi::BangumiSubject&>
		Resolve_Subject(std::string json, bool refresh) {
		try
		{
			//完全不需要在底层来对\u进行解码,property_tree会进行这项工作的,前提是使用wptree
			//将string转换wstring直接使用C++ 11的构造函数
			std::wstring wjson(json.begin(), json.end());
			//宽解析树
			boost::property_tree::wptree pt;
			//因为解析需要一个流输入,因此使用stringstream,也支持文件流读取
			std::wistringstream input(wjson);
			//解析json
			boost::property_tree::read_json(input, pt);
			//解析元素,[注意]:解析后的都是UTF编码,并非GBK,后面会用Locale进行转换


			//首先判断是否有效
			//
			int code = pt.get<int>(L"code", 200);
			if (code != 200) {
				//请求是无效的
#ifndef NDEBUG
				{
					bangumi::string debug_msg;
					debug_msg << "未找到该条目";
					CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Resolve-Subject", debug_msg);
				}
#endif

				//直接抛出异常来给上层(Function)提供构造信息
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
				//TODO:API中并没有给出,可以通过HTML获取

			}
			std::string wair_date_str = ResolveConv(pt.get<std::wstring>(L"air_date", L"0000-00-00"));
			//默认是一个无效的日期
			boost::gregorian::date wair_date;
			if (wair_date_str.compare("0000-00-00") != 0) {
				//如果json中是一个有效的日期,则赋值
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
			//添加评价分段人数
			int detail_score[11] = {0};
			detail_score[1] = pt.get<int>(L"rating.count.1", 0);
			detail_score[2] = pt.get<int>(L"rating.count.2", 0);
			detail_score[3] = pt.get<int>(L"rating.count.3", 0);
			detail_score[4] = pt.get<int>(L"rating.count.4", 0);
			detail_score[5] = pt.get<int>(L"rating.count.5", 0);
			detail_score[6] = pt.get<int>(L"rating.count.6", 0);
			detail_score[7] = pt.get<int>(L"rating.count.7", 0);
			detail_score[8] = pt.get<int>(L"rating.count.8", 0);
			detail_score[9] = pt.get<int>(L"rating.count.9", 0);
			detail_score[10] = pt.get<int>(L"rating.count.10", 0);
			//求最大的分段数
			for (int i = 1; i < 11; ++i) {
				if (detail_score[0] < detail_score[i])
					detail_score[0] = detail_score[i];
			}
			
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

			//一个转换的例子
			//std::string sign = ResolveConv(sign);

			//返回的参数: 线程
			std::vector<std::shared_ptr<boost::thread>> ThreadVector;
			//Pic url参数先处理
			std::string file_path;

			auto result = PicDownload(http_client, ResolveConv(wimage_url), SUBJECT_PIC_PATH, std::to_string(wid), file_path, refresh);
			//返回的线程保存到返回值中
			if (result.first == DownloadStatus::MultiThread)
			{
				//多线程
				ThreadVector.push_back(result.second);
			}
			//创建User结构体对象
			//是否刷新缓存
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
				wcollection,
				detail_score);
			//返回结构体
			return{ ThreadVector,subject };

		}
		catch (boost::system::system_error & e)
		{
			//Boost方面的问题
			//同时可以通过这个catch来构造失败消息
			throw e;
		}
		//property_tree在解析失败时并不会抛出system_error而是exception
		catch (std::exception &e) {
#ifndef NDEBUG
			{
				bangumi::string debug_msg;
				debug_msg << e.what();
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Resolve-Subject", debug_msg);
			}
#endif
			//解析树的问题
			throw boost::system::system_error(bangumi_bot_errors::json_resolve_error);
			//std::cout << "Error! Code = "
			//	<< "Message = " << e.what() << std::endl;
		}
	}

	//解析Auth
	bangumi::AuthReply
		Resolve_Auth(std::string json) {
		try
		{
			//由于Auth百分百没有中文长字符,直接使用默认的ptree
			boost::property_tree::ptree pt;
			//因为解析需要一个流输入,因此使用stringstream,也支持文件流读取
			std::istringstream input(json);
			//解析json
			boost::property_tree::read_json(input, pt);
			//解析元素,[注意]:解析后的都是UTF编码,并非GBK,后面会用Locale进行转换


			//首先判断是否有效
			std::string error = pt.get<std::string>("error", "");
			if (error.compare("")!=0) {
				//请求是无效的
				std::string error_msg = pt.get<std::string>("error_description", "");
#ifndef NDEBUG
				{
					bangumi::string debug_msg;
					debug_msg << "Oauth Post请求失败"
						>> "错误: " << error_msg;
					CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Resolve-Auth", debug_msg);
				}
#endif

				//直接抛出异常来给上层(Function)提供构造信息
				throw boost::system::system_error(bangumi_bot_errors::auth_request_error);
			}

		
			std::string access_token = pt.get<std::string>("access_token", "");
			std::string token_type = pt.get<std::string>("token_type", "");
			size_t user_id = pt.get<size_t>("user_id", 0);
			std::string refresh_token = pt.get<std::string>("refresh_token", "");
			

			//返回结构体
			return bangumi::AuthReply(access_token,token_type,user_id, refresh_token);

		}
		catch (boost::system::system_error & e)
		{
			//Boost方面的问题
			//同时可以通过这个catch来构造失败消息
			throw e;
		}
		//property_tree在解析失败时并不会抛出system_error而是exception
		catch (std::exception &e) {
#ifndef NDEBUG
			{
				bangumi::string debug_msg;
				debug_msg << e.what();
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Resolve-Auth", debug_msg);
			}
#endif
			//解析树的问题
			throw boost::system::system_error(bangumi_bot_errors::json_resolve_error);
			//std::cout << "Error! Code = "
			//	<< "Message = " << e.what() << std::endl;
		}
	}


	//解析Auth status
	//如果过期则返回0,否则返回对应的bangumi_id
	size_t
		Resolve_Auth_Status(std::string json) {
		//{
		//	"access_token": "f1b237xxxxxxxxxxxxxxxxxx2f9ac6de6b",
		//		"client_id" : "bgm101xxxxxxxxxxd09805b",
		//		"user_id" : 423387,
		//		"expires" : 1556354941,
		//		"scope" : null
		//}
		//{
		//	"error": "invalid_token",
		//		"error_description" : "The access token provided has expired"
		//}
		try
		{
			//由于Auth百分百没有中文长字符,直接使用默认的ptree
			boost::property_tree::ptree pt;
			//因为解析需要一个流输入,因此使用stringstream,也支持文件流读取
			std::istringstream input(json);
			//解析json
			boost::property_tree::read_json(input, pt);
			//解析元素,[注意]:解析后的都是UTF编码,并非GBK,后面会用Locale进行转换


			//首先判断是否有效
			std::string error = pt.get<std::string>("error", "");
			if (error.compare("") != 0) {
				//请求是无效的
				std::string error_msg = pt.get<std::string>("error_description", "");
#ifndef NDEBUG
				{
					bangumi::string debug_msg;
					debug_msg << "Oauth Post请求失败"
						>> "错误: " << error_msg;
					CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Resolve-Auth-Status", debug_msg);
				}
#endif
				//直接返回
				return 0;
				//直接抛出异常来给上层(Function)提供构造信息
				//throw boost::system::system_error(bangumi_bot_errors::auth_error);
			}

			size_t user_id = pt.get<size_t>("user_id", 0);
			//std::string refresh_token = pt.get<std::string>("refresh_token", "");



			//返回结构体
			return user_id;

		}
		catch (boost::system::system_error & e)
		{
			//Boost方面的问题
			//同时可以通过这个catch来构造失败消息
			throw e;
		}
		//property_tree在解析失败时并不会抛出system_error而是exception
		catch (std::exception &e) {
#ifndef NDEBUG
			{
				bangumi::string debug_msg;
				debug_msg << e.what();
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Resolve-Auth-Status", debug_msg);
			}
#endif
			//解析树的问题
			throw boost::system::system_error(bangumi_bot_errors::json_resolve_error);
			//std::cout << "Error! Code = "
			//	<< "Message = " << e.what() << std::endl;
		}
	}

	//解析Auth Refresh
	bangumi::AuthReply
		Resolve_Auth_Refresh(std::string json) {
		try
		{
			//由于Auth百分百没有中文长字符,直接使用默认的ptree
			boost::property_tree::ptree pt;
			//因为解析需要一个流输入,因此使用stringstream,也支持文件流读取
			std::istringstream input(json);
			//解析json
			boost::property_tree::read_json(input, pt);
			//解析元素,[注意]:解析后的都是UTF编码,并非GBK,后面会用Locale进行转换


			//首先判断是否有效
			std::string error = pt.get<std::string>("error", "");
			if (error.compare("") != 0) {
				//请求是无效的
				std::string error_msg = pt.get<std::string>("error_description", "");
#ifndef NDEBUG
				{
					bangumi::string debug_msg;
					debug_msg << "Oauth Post请求失败"
						>> "错误: " << error_msg;
					CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Resolve-Auth", debug_msg);
				}
#endif

				//直接抛出异常来给上层(Function)提供构造信息
				throw boost::system::system_error(bangumi_bot_errors::auth_request_error);
			}


			std::string access_token = pt.get<std::string>("access_token", "");
			//std::string token_type = pt.get<std::string>("token_type", "");
			//size_t user_id = pt.get<size_t>("user_id", 0);
			std::string refresh_token = pt.get<std::string>("refresh_token", "");



			//返回结构体
			return bangumi::AuthReply(access_token, "", 0, refresh_token);

		}
		catch (boost::system::system_error & e)
		{
			//Boost方面的问题
			//同时可以通过这个catch来构造失败消息
			throw e;
		}
		//property_tree在解析失败时并不会抛出system_error而是exception
		catch (std::exception &e) {
#ifndef NDEBUG
			{
				bangumi::string debug_msg;
				debug_msg << e.what();
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Resolve-Auth-Refresh", debug_msg);
			}
#endif
			//解析树的问题
			throw boost::system::system_error(bangumi_bot_errors::json_resolve_error);
			//std::cout << "Error! Code = "
			//	<< "Message = " << e.what() << std::endl;
		}
	}


	//解析User fixed collection
	std::pair<std::vector<std::shared_ptr<boost::thread>>, bangumi::BangumiUser&>
		Resolve_User_Process(std::string json, bool refresh) {
		//{
		//	"status": {
		//		"id": 3,
		//		"type" : "do",
		//		"name" : "在看"
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
		//	"request": "/collection/21811?access_token=088e77fb1xxxxxxxxxxxxxxxxxxxx06c33a45d",
		//		"code" : 400,
		//		"error" : "40001 Error: Nothing found with that ID"
		//}
		try
		{

			//完全不需要在底层来对\u进行解码,property_tree会进行这项工作的,前提是使用wptree
			//将string转换wstring直接使用C++ 11的构造函数
			std::wstring wjson(json.begin(), json.end());
			//宽解析树
			boost::property_tree::wptree pt;
			//因为解析需要一个流输入,因此使用stringstream,也支持文件流读取
			std::wistringstream input(wjson);
			//解析json
			boost::property_tree::read_json(input, pt);
			//解析元素,[注意]:解析后的都是UTF编码,并非GBK,后面会用Locale进行转换

			//返回的参数: 线程
			std::vector<std::shared_ptr<boost::thread>> ThreadVector;
			//进度类
			bangumi::BangumiUserProgress user_progress;
			//首先判断是否有效
			int code = pt.get<int>(L"code", 200);
			std::wstring werror= pt.get<std::wstring>(L"error", L"");
			if (werror == L"invalid_token") {
				//用户的token暂时无效,需要刷新
				//向上抛出异常
				throw boost::system::system_error(bangumi_bot_errors::access_token_invalid);
			}
			if (code != 200) {
				//用户没有收藏此条目
#ifndef NDEBUG
				{
					bangumi::string debug_msg;
					debug_msg << "用户没有收藏此条目";
					CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Resolve-User-Process", debug_msg);
				}
#endif
				//std::shared_ptr<bangumi::Msg_Interface> msg(new bangumi::Reply("查找用户失败..."));
				//return{ ThreadVector, bangumi::Reply("查找用户失败...") };
				//直接抛出异常来给上层(Function)提供构造信息
				throw boost::system::system_error(bangumi_bot_errors::user_not_collect_this_subject);
			}
			else {
				//解析进度部分

				std::wstring status_name = pt.get<std::wstring>(L"status.name", L"");
				int rating = pt.get<int>(L"rating", 0);
				std::wstring comment = pt.get<std::wstring>(L"comment", L"");
				int ep_status = pt.get<int>(L"ep_status", 0);
				std::string progress ("0/");
				if (ep_status != 0) {
					//TODO:获取总集数
					//目前是计划在上层函数那里取得
					progress = std::to_string(ep_status) + "/";
				}
				user_progress = { ResolveConv(status_name), rating, ResolveConv(comment), progress };

				

				//解析user部分
				size_t wid = pt.get<size_t>(L"user.id", 0);

				if (refresh == false) {
					//如果不刷新
					try {
						auto &bgm_user = BangumiPreFindUser(wid);
						bgm_user.SetProgress(user_progress);
						//为user添加Progress
						return{ ThreadVector,bgm_user };

					}
					catch (std::out_of_range) {
						//没有找到
						//直接进行下一步
					}
				}


				std::wstring wurl = pt.get<std::wstring>(L"user.url", L"???");
				std::wstring wuser_name = pt.get<std::wstring>(L"user.username", L"???");
				std::wstring wnick_name = pt.get<std::wstring>(L"user.nickname", L"???");
				std::wstring wava_url = pt.get<std::wstring>(L"user.avatar.large", L"");
				std::wstring wsign = pt.get<std::wstring>(L"user.sign", L"");
				//int wuser_group = pt.get<int>(L"usergroup", 10);
				//一个转换的例子
				//std::string sign = ResolveConv(sign);


				//Pic url参数先处理
				std::string file_path;

				auto result = PicDownload(http_client, ResolveConv(wava_url), USER_PIC_PATH, std::to_string(wid), file_path, refresh);
				//返回的线程保存到返回值中
				if (result.first == DownloadStatus::MultiThread)
				{
					//多线程
					ThreadVector.push_back(result.second);
				}
				//创建User结构体对象
				//是否刷新缓存
				auto &user = BangumiAddUser(wid, ResolveConv(wurl), ResolveConv(wuser_name),
					ResolveConv(wnick_name), file_path, ResolveConv(wsign));
				//为user添加Progress
				user.SetProgress(user_progress);

				//返回结构体
				return{ ThreadVector,user };
			}

		}
		catch (boost::system::system_error & e)
		{
			//Boost方面的问题
			//同时可以通过这个catch来构造失败消息
			throw e;
		}
		//property_tree在解析失败时并不会抛出system_error而是exception
		catch (std::exception &e) {
#ifndef NDEBUG
			{
				bangumi::string debug_msg;
				debug_msg << e.what();
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Resolve-User", debug_msg);
			}
#endif
			//解析树的问题
			throw boost::system::system_error(bangumi_bot_errors::json_resolve_error);
			//std::cout << "Error! Code = "
			//	<< "Message = " << e.what() << std::endl;
		}
	}

	//解析Search
	std::pair<std::vector<std::shared_ptr<boost::thread>>, bangumi::string>
		Resolve_Search(std::string json, bool refresh) {
#define API_SEARCH_MAX_NUM 10
		try
		{
			if (json.find("<!DOCTYPE") != std::string::npos) {
				//说明这次搜索是没有可返回的结果的,也就是不存在相关条目
				//抛出异常返回消息
				throw boost::system::system_error(bangumi_bot_errors::search_failed);
			}
			//完全不需要在底层来对\u进行解码,property_tree会进行这项工作的,前提是使用wptree
			//将string转换wstring直接使用C++ 11的构造函数
			std::wstring wjson(json.begin(), json.end());
			//宽解析树
			boost::property_tree::wptree pt;
			//因为解析需要一个流输入,因此使用stringstream,也支持文件流读取
			std::wistringstream input(wjson);
			//解析json
			boost::property_tree::read_json(input, pt);
			//解析元素,[注意]:解析后的都是UTF编码,并非GBK,后面会用Locale进行转换


			//首先判断是否有效
			//实际如果失败是会返回一个HTML页面的
			int code = pt.get<int>(L"code", 200);
			if (code != 200) {
				//请求是无效的
#ifndef NDEBUG
				{
					bangumi::string debug_msg;
					debug_msg << "搜索失败...";
					CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Resolve-Search", debug_msg);
				}
#endif

				//直接抛出异常来给上层(Function)提供构造信息
				throw boost::system::system_error(bangumi_bot_errors::empty_subject);
			}
			
			//
			size_t results = pt.get<size_t>(L"results", 0);
			// get_child得到数组对象
			boost::property_tree::wptree list = pt.get_child(L"list");
			boost::property_tree::wptree::iterator pos = list.begin();
			//返回的参数: 线程
			std::vector<std::shared_ptr<boost::thread>> ThreadVector;
			//返回的参数: 消息
			bangumi::string msg;
			//总计相符条目数: 1000
			msg << "总计相符条目数: " << results << "\n";
			int subject_pos = 0;
			for (; pos != list.end(); ++pos)
			{
				if (subject_pos >= API_SEARCH_MAX_NUM) {
					//限制最大结果数
					break;
				}
				//使用局部变量
				auto &pt = pos->second;
				//json 的 Subject数组解析
				//每一个都是一个Subject mini

				size_t wid = pt.get<size_t>(L"id", 0);
				std::wstring wurl = pt.get<std::wstring>(L"url", L"");
				int wtype = pt.get<size_t>(L"type", 2);
				std::wstring wname = pt.get<std::wstring>(L"name", L"");
				std::wstring wname_cn = pt.get<std::wstring>(L"name_cn", L"");
				std::wstring wsummary = pt.get<std::wstring>(L"summary", L"");
				int weps = pt.get<int>(L"eps", 0);
				if (weps == 0) {
					//TODO:API中并没有给出,可以通过HTML获取

				}
				std::string wair_date_str = ResolveConv(pt.get<std::wstring>(L"air_date", L"0000-00-00"));
				//默认是一个无效的日期
				boost::gregorian::date wair_date;
				if (wair_date_str.compare("0000-00-00") != 0) {
					//如果json中是一个有效的日期,则赋值
					wair_date = boost::gregorian::from_string(wair_date_str);
				}
				int wair_weekday = pt.get<int>(L"air_weekday", 0);
				int wrating_num = pt.get<int>(L"rating.total", 0);
				//添加评价分段人数
				int detail_score[11] = { 0 };
				detail_score[1] = pt.get<int>(L"rating.count.1", 0);
				detail_score[2] = pt.get<int>(L"rating.count.2", 0);
				detail_score[3] = pt.get<int>(L"rating.count.3", 0);
				detail_score[4] = pt.get<int>(L"rating.count.4", 0);
				detail_score[5] = pt.get<int>(L"rating.count.5", 0);
				detail_score[6] = pt.get<int>(L"rating.count.6", 0);
				detail_score[7] = pt.get<int>(L"rating.count.7", 0);
				detail_score[8] = pt.get<int>(L"rating.count.8", 0);
				detail_score[9] = pt.get<int>(L"rating.count.9", 0);
				detail_score[10] = pt.get<int>(L"rating.count.10", 0);
				//求最大的分段数
				for (int i = 1; i < 11; ++i) {
					if (detail_score[0] < detail_score[i])
						detail_score[0] = detail_score[i];
				}
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

				//一个转换的例子
				//std::string sign = ResolveConv(sign);

				//Pic url参数先处理
				std::string file_path;

				auto result = PicDownload(http_client, ResolveConv(wimage_url), SUBJECT_PIC_PATH, std::to_string(wid), file_path, refresh);
				//返回的线程保存到返回值中
				if (result.first == DownloadStatus::MultiThread)
				{
					//多线程
					ThreadVector.push_back(result.second);
				}
				//创建User结构体对象
				//是否刷新缓存
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
					wcollection,
					detail_score);
				//返回结构体
				//return{ ThreadVector,subject };
				msg << subject.SearchGet(++subject_pos);

			}
			//处理完成所有的Subject后
			//由于统一是Subject的引用,只好在这个函数内直接返回Subject需要返回的内容SearchGet
			
			



			return{ ThreadVector,std::move(msg) };


		}
		catch (boost::system::system_error & e)
		{
			//Boost方面的问题
			//同时可以通过这个catch来构造失败消息
			throw e;
		}
		//property_tree在解析失败时并不会抛出system_error而是exception
		catch (std::exception &e) {
#ifndef NDEBUG
			{
				bangumi::string debug_msg;
				debug_msg << e.what();
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Resolve-Search", debug_msg);
			}
#endif
			//解析树的问题
			throw boost::system::system_error(bangumi_bot_errors::json_resolve_error);
			//std::cout << "Error! Code = "
			//	<< "Message = " << e.what() << std::endl;
		}
	}

	//解析Search Single 主要用于解析单个Search
	std::pair<std::vector<std::shared_ptr<boost::thread>>, bangumi::BangumiSubject&>
		Resolve_Search_Singel(std::string json, bool refresh) {
		try
		{
			if (json.find("<!DOCTYPE") != std::string::npos) {
				//说明这次搜索是没有可返回的结果的,也就是不存在相关条目
				//抛出异常返回消息
				throw boost::system::system_error(bangumi_bot_errors::search_failed);
			}
			//完全不需要在底层来对\u进行解码,property_tree会进行这项工作的,前提是使用wptree
			//将string转换wstring直接使用C++ 11的构造函数
			std::wstring wjson(json.begin(), json.end());
			//宽解析树
			boost::property_tree::wptree pt;
			//因为解析需要一个流输入,因此使用stringstream,也支持文件流读取
			std::wistringstream input(wjson);
			//解析json
			boost::property_tree::read_json(input, pt);
			//解析元素,[注意]:解析后的都是UTF编码,并非GBK,后面会用Locale进行转换


			//首先判断是否有效
			//实际如果失败是会返回一个HTML页面的
			int code = pt.get<int>(L"code", 200);
			if (code != 200) {
				//请求是无效的
#ifndef NDEBUG
				{
					bangumi::string debug_msg;
					debug_msg << "搜索失败...";
					CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Resolve-Search-Single", debug_msg);
				}
#endif

				//直接抛出异常来给上层(Function)提供构造信息
				throw boost::system::system_error(bangumi_bot_errors::search_failed);
			}

			//
			size_t results = pt.get<size_t>(L"results", 0);
			// get_child得到数组对象
			boost::property_tree::wptree list = pt.get_child(L"list");
			boost::property_tree::wptree::iterator pos = list.begin();
			//返回的参数: 线程
			std::vector<std::shared_ptr<boost::thread>> ThreadVector;
			//返回的参数: 消息
			//bangumi::string msg;
			//总计相符条目数: 1000
			//msg << "总计相符条目数: " << results << "\n";
			//int subject_pos = 0;

			//因为使用函数的前提是search_num = 1
			//[弃]只处理第一个结果的Subject 
			//记录数组
			std::map<boost::gregorian::date, bangumi::BangumiSubject&> subject_air_map;
			while(pos!=list.end())
			{
				//使用局部变量
				auto &pt = pos->second;
				//json 的 Subject数组解析
				//每一个都是一个Subject mini

				size_t wid = pt.get<size_t>(L"id", 0);
				std::wstring wurl = pt.get<std::wstring>(L"url", L"");
				int wtype = pt.get<size_t>(L"type", 2);
				std::wstring wname = pt.get<std::wstring>(L"name", L"");
				std::wstring wname_cn = pt.get<std::wstring>(L"name_cn", L"");
				std::wstring wsummary = pt.get<std::wstring>(L"summary", L"");
				int weps = pt.get<int>(L"eps", 0);
				if (weps == 0) {
					//TODO:API中并没有给出,可以通过HTML获取

				}
				std::string wair_date_str = ResolveConv(pt.get<std::wstring>(L"air_date", L"0000-00-00"));
				//默认是一个无效的日期
				boost::gregorian::date wair_date;
				if (wair_date_str.compare("0000-00-00") != 0) {
					//如果json中是一个有效的日期,则赋值
					wair_date = boost::gregorian::from_string(wair_date_str);
				}
				int wair_weekday = pt.get<int>(L"air_weekday", 0);
				int wrating_num = pt.get<int>(L"rating.total", 0);
				//添加评价分段人数
				int detail_score[11] = { 0 };
				detail_score[1] = pt.get<int>(L"rating.count.1", 0);
				detail_score[2] = pt.get<int>(L"rating.count.2", 0);
				detail_score[3] = pt.get<int>(L"rating.count.3", 0);
				detail_score[4] = pt.get<int>(L"rating.count.4", 0);
				detail_score[5] = pt.get<int>(L"rating.count.5", 0);
				detail_score[6] = pt.get<int>(L"rating.count.6", 0);
				detail_score[7] = pt.get<int>(L"rating.count.7", 0);
				detail_score[8] = pt.get<int>(L"rating.count.8", 0);
				detail_score[9] = pt.get<int>(L"rating.count.9", 0);
				detail_score[10] = pt.get<int>(L"rating.count.10", 0);
				//求最大的分段数
				for (int i = 1; i < 11; ++i) {
					if (detail_score[0] < detail_score[i])
						detail_score[0] = detail_score[i];
				}
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

				//一个转换的例子
				//std::string sign = ResolveConv(sign);

				//Pic url参数先处理
				std::string file_path;

				auto result = PicDownload(http_client, ResolveConv(wimage_url), SUBJECT_PIC_PATH, std::to_string(wid), file_path, refresh);
				//返回的线程保存到返回值中
				if (result.first == DownloadStatus::MultiThread)
				{
					//多线程
					ThreadVector.push_back(result.second);
				}
				//创建User结构体对象
				//是否刷新缓存
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
					wcollection,
					detail_score);
				//返回结构体
				//return{ ThreadVector,subject };
				//插入map
				subject_air_map.emplace(wair_date, subject);
				//msg << subject.SearchGet(++subject_pos);
				++pos;

			}
			//else {
			//	//没有一条结果
			//	//实际可能不存在这种情况,但还是为了以防万一抛出一个异常
			//	//throw boost::system::system_error(bangumi_bot_errors::search_failed);
			//}
			//处理完成所有的Subject后
			//由于统一是Subject的引用,只好在这个函数内直接返回Subject需要返回的内容SearchGet


			//最后返回结构体
			if (!subject_air_map.empty())
			{
				return{ ThreadVector,subject_air_map.rbegin()->second };
			}
			else {

				//没有一条结果
				//实际可能不存在这种情况,但还是为了以防万一抛出一个异常
				throw boost::system::system_error(bangumi_bot_errors::search_failed);
			}
			


			//return{ ThreadVector,std::move(msg) };


		}
		catch (boost::system::system_error & e)
		{
			//Boost方面的问题
			//同时可以通过这个catch来构造失败消息
			throw e;
		}
		//property_tree在解析失败时并不会抛出system_error而是exception
		catch (std::exception &e) {
#ifndef NDEBUG
			{
				bangumi::string debug_msg;
				debug_msg << e.what();
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Resolve-Search", debug_msg);
			}
#endif
			//解析树的问题
			throw boost::system::system_error(bangumi_bot_errors::json_resolve_error);
			//std::cout << "Error! Code = "
			//	<< "Message = " << e.what() << std::endl;
		}
	}

	//解析Collection 和 progress相似
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

			//完全不需要在底层来对\u进行解码,property_tree会进行这项工作的,前提是使用wptree
			//将string转换wstring直接使用C++ 11的构造函数
			std::wstring wjson(json.begin(), json.end());
			//宽解析树
			boost::property_tree::wptree pt;
			//因为解析需要一个流输入,因此使用stringstream,也支持文件流读取
			std::wistringstream input(wjson);
			//解析json
			boost::property_tree::read_json(input, pt);
			//解析元素,[注意]:解析后的都是UTF编码,并非GBK,后面会用Locale进行转换

			//返回的参数: 线程
			std::vector<std::shared_ptr<boost::thread>> ThreadVector;
			//进度类
			bangumi::BangumiUserProgress user_progress;
			//首先判断是否有效
			int code = pt.get<int>(L"code", 200);
			std::wstring werror = pt.get<std::wstring>(L"error", L"");
			if (werror == L"invalid_token") {
				//用户的token暂时无效,需要刷新
				//向上抛出异常
				throw boost::system::system_error(bangumi_bot_errors::access_token_invalid);
			}
			if (code != 200) {
				//用户没有收藏此条目
#ifndef NDEBUG
				{
					bangumi::string debug_msg;
					debug_msg << "条目收藏失败";
					CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Resolve-Collection", debug_msg);
				}
#endif
				//std::shared_ptr<bangumi::Msg_Interface> msg(new bangumi::Reply("查找用户失败..."));
				//return{ ThreadVector, bangumi::Reply("查找用户失败...") };
				//直接抛出异常来给上层(Function)提供构造信息
				throw boost::system::system_error(bangumi_bot_errors::subject_collect_failed);
			}
			else {
				//解析进度部分

				//std::wstring status_name = pt.get<std::wstring>(L"status.name", L"");
				//由于收藏返回的name是Null 应当从id入手
				int status_id = pt.get<int>(L"status.id", 0);
				std::string status_name;
				try {
					status_name = bangumi::Bangumi_Collect_Status.at(status_id);
				}
				catch (std::out_of_range&) {
					//未知的状态
					status_name = "???";
				}
				
				int rating = pt.get<int>(L"rating", 0);
				std::wstring comment = pt.get<std::wstring>(L"comment", L"");
				//int ep_status = pt.get<int>(L"ep_status", 0);
				//由于这个API仅仅返回0,直接设定progress = "" 不输出
				std::string progress("");
				//if (ep_status != 0) {
				//	//TODO:获取总集数
				//	//目前是计划在上层函数那里取得
				//	progress = std::to_string(ep_status) + "/";
				//}
				//[注意]里面的字符全部是UTF-8
				user_progress = { status_name, rating, ResolveConv(comment), progress };



				//解析user部分
				size_t wid = pt.get<size_t>(L"user.id", 0);

				if (refresh == false) {
					//如果不刷新
					try {
						auto &bgm_user = BangumiPreFindUser(wid);
						bgm_user.SetProgress(user_progress);
						//为user添加Progress
						return{ ThreadVector,bgm_user };

					}
					catch (std::out_of_range) {
						//没有找到
						//直接进行下一步
					}
				}


				std::wstring wurl = pt.get<std::wstring>(L"user.url", L"???");
				std::wstring wuser_name = pt.get<std::wstring>(L"user.username", L"???");
				std::wstring wnick_name = pt.get<std::wstring>(L"user.nickname", L"???");
				std::wstring wava_url = pt.get<std::wstring>(L"user.avatar.large", L"");
				std::wstring wsign = pt.get<std::wstring>(L"user.sign", L"");
				//int wuser_group = pt.get<int>(L"usergroup", 10);
				//一个转换的例子
				//std::string sign = ResolveConv(sign);


				//Pic url参数先处理
				std::string file_path;

				auto result = PicDownload(http_client, ResolveConv(wava_url), USER_PIC_PATH, std::to_string(wid), file_path, refresh);
				//返回的线程保存到返回值中
				if (result.first == DownloadStatus::MultiThread)
				{
					//多线程
					ThreadVector.push_back(result.second);
				}
				//创建User结构体对象
				//是否刷新缓存
				auto &user = BangumiAddUser(wid, ResolveConv(wurl), ResolveConv(wuser_name),
					ResolveConv(wnick_name), file_path, ResolveConv(wsign));
				//为user添加Progress
				user.SetProgress(user_progress);

				//返回结构体
				return{ ThreadVector,user };
			}

		}
		catch (boost::system::system_error & e)
		{
			//Boost方面的问题
			//同时可以通过这个catch来构造失败消息
			throw e;
		}
		//property_tree在解析失败时并不会抛出system_error而是exception
		catch (std::exception &e) {
#ifndef NDEBUG
			{
				bangumi::string debug_msg;
				debug_msg << e.what();
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Resolve-User", debug_msg);
			}
#endif
			//解析树的问题
			throw boost::system::system_error(bangumi_bot_errors::json_resolve_error);
			//std::cout << "Error! Code = "
			//	<< "Message = " << e.what() << std::endl;
		}
	}

	//解析Update 
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
			//解析树
			boost::property_tree::ptree pt;
			//因为解析需要一个流输入,因此使用stringstream,也支持文件流读取
			std::istringstream input(json);
			//解析json
			boost::property_tree::read_json(input, pt);
			//解析元素,[注意]:解析后的都是UTF编码,并非GBK,后面会用Locale进行转换
			int code = pt.get<int>("code", 400);
			if (code == 202) {
				//说明更新成功,否则失败
				return true;
			}
			else {
				//失败的更新
				return false;
			}
		

		}
		catch (boost::system::system_error & e)
		{
			//Boost方面的问题
			//同时可以通过这个catch来构造失败消息
			throw e;
		}
		//property_tree在解析失败时并不会抛出system_error而是exception
		catch (std::exception &e) {
#ifndef NDEBUG
			{
				bangumi::string debug_msg;
				debug_msg << e.what();
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Resolve-User", debug_msg);
			}
#endif
			//解析树的问题
			throw boost::system::system_error(bangumi_bot_errors::json_resolve_error);
			//std::cout << "Error! Code = "
			//	<< "Message = " << e.what() << std::endl;
		}
	}

	//解析Subject中的Tag
	inline bangumi::string ResolveSubjectTag(const std::string &html) {
		bangumi::string ret("\n<-标签->:\n");
		//直接从1000处搜索
		try {
			size_t tag_start = html.find("class=\"subject_tag_section\">", 1000);
			//判断是否存在
			if (tag_start == std::string::npos) {
				ret = "未收录标签...";
				return ret;
			}
			//因为只是最后检测,为了防止出界,先缩小最后的范围
			size_t tag_end = html.find("</small></a></div>", tag_start) - 11;
			size_t single_tag_start = tag_start;
			size_t single_tag_end;
			while (single_tag_start < tag_end) {
				single_tag_start = html.find("<span>", single_tag_start) + 6;
				single_tag_end = html.find("</span>", single_tag_start);
				//插入Tag的名称
				ret << html.substr(single_tag_start, single_tag_end - single_tag_start);
				single_tag_start = html.find("\">", single_tag_end) + 2;
				single_tag_end = html.find("</small>", single_tag_start);
				//插入Tag的数量
				ret << '<' << html.substr(single_tag_start, single_tag_end - single_tag_start) << "> | ";

				single_tag_start = single_tag_end;
			}
			ret.erase(ret.find_last_of('|'), 1);


			return std::move(ret);
		}
		catch (const std::exception&)
		{
			return "访问失败...";
		}
	}

	//解析Subject中的角色
	inline std::pair<bangumi::string,bangumi::string> ResolveSubjectCharacter(const std::string &html, bool refresh = false) {
		bangumi::string ret("\n<-角色->:\n");
		try {
			//bangumi::string ret2;
			//直接从1500处搜索
			size_t character_start = html.find("class=\"subject_section clearit\">", 1500);
			//判断是否存在
			if (character_start == std::string::npos) {
				return{ "未收录角色...","" };
			}
			//因为只是最后检测,为了防止出界,先缩小最后的范围
			size_t character_end = html.find("class=\"more\">", character_start) - 11;
			//默认开始位置
			size_t single_character_start = character_start;
			//结尾设置为第一个character开始位置
			size_t single_character_end = html.find("title=\"", single_character_start) + 7;
			size_t temp;
			size_t pic_start;
			size_t pic_end;
			size_t job_start;
			size_t job_end;
			size_t cv_start;
			size_t cv_end;
			size_t pic_name_end;
			//PIC图片下载线程
			std::vector<std::shared_ptr<boost::thread>> ThreadVector;
			//分割成两个消息 防止过长无法发送
			int num = 0;
			const int ONE_MAX_NUM = 4;
			while (single_character_end < character_end) {
				++num;
				//
				single_character_start = single_character_end;
				//其实就是下一个Character的开始
				single_character_end = html.find("title=\"", single_character_start) + 7;
				//
				std::string name;
				std::string pic_url;
				std::string job_tip;
				std::string cv;
				std::string pic_save_name;
				//PICDownload的文件保存位置
				std::string file_path;
				//url
				size_t character_url_ex;
				//name的结尾位置
				temp = html.find("\"", single_character_start);
				name = html.substr(single_character_start, temp - single_character_start);
				//为了加快速度,直接加50的搜索进度
				pic_start = html.find("url('//", temp + 50) + 7;
				pic_end = html.find("')\"", pic_start);
				pic_url = "http://" + html.substr(pic_start, pic_end - pic_start);
				//找到文件名的开头
				temp = pic_url.find_last_of('/') + 1;
				//找到文件名结尾
				pic_name_end = pic_url.find_last_of('.');
				//图片名字
				pic_save_name = pic_url.substr(temp, pic_name_end - temp);
				//Character的ID
				try {
					character_url_ex = std::stoul(pic_save_name);
				}
				catch (boost::system::system_error&) {
					//忽略
				}
				//将图片大小换成m类型
				temp = pic_url.find("/s/");
				pic_url[temp + 1] = 'm';
				//下载图片
#ifndef NDEBUG
				{
					bangumi::string debug_msg;
					debug_msg << "这是次数开始 > " << num;
					CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-R-PIC-DOWNLOAD", debug_msg);
				}
#endif
				auto result = PicDownload(http_client, pic_url, CHARACTER_PIC_PATH, pic_save_name, file_path, refresh);
#ifndef NDEBUG
				{
					bangumi::string debug_msg;
					debug_msg << "这是次数结束 > " << num;
					CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-R-PIC-DOWNLOAD", debug_msg);;
				}
#endif
				//返回的线程保存到返回值中
				if (result.first == DownloadStatus::MultiThread)
				{
					//将下载线程压入线程池中
					ThreadVector.push_back(result.second);
				}
				//演员位置
				job_start = html.find("job_tip\">", pic_end + 10) + 9;
				job_end = html.find("</s", job_start);
				job_tip = html.substr(job_start, job_end - job_start);
				//CV(可能没有)
				cv = html.substr(job_end, single_character_end - job_end);
				cv_start = cv.find("arring\">");
				if (cv_start != std::string::npos) {
					cv_end = cv.find("</a>", cv_start);
					cv = cv.substr(cv_start + 8, cv_end - cv_start - 8);
				}
				else {
					cv = "";
				}
				//ret回复
				//if (num < ONE_MAX_NUM) {
				ret << "[CQ:image,file=" << std::move(file_path) << "]"
					>> '[' << std::move(job_tip) << ']'
					>> std::move(name);
				if (!cv.empty())
					ret >> "CV:  " << std::move(cv);
				ret >> "角色主页: https://bgm.tv/character/" << character_url_ex;
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
			//等待图片下载完成
			for (auto &t : ThreadVector) {
				if (t != nullptr&&t->joinable())
					t->join();
			}
			//如果不为空
			if (!ret.empty()) {
				ret[ret.length() - 1] = ' ';
			}
			return{ std::move(ret),"" };
		}
		catch (const std::exception&)
		{
			return{ "访问失败...","" };
		}
	}

	//解析Subject中的吐槽
	inline bangumi::string ResolveSubjectComment(const std::string &raw_html, bool refresh = false) {
		bangumi::string ret("\n<-吐槽->:\n");
		try {
			//直接从1500处搜索
			size_t comment_start = raw_html.find("\"comment_box\"", 2500);
			//判断是否存在,其实一定存在,实际上可能是会员才能看到的条目
			if (comment_start == std::string::npos) {
				ret = "\n未收录吐槽...\n";
				return ret;
			}
			//因为只是最后检测,为了防止出界,先缩小最后的范围
			size_t comment_end = raw_html.find("class=\"more\">", comment_start) - 11;
			//直接
			std::string html = raw_html.substr(comment_start, comment_end - comment_start);
			//
			//第一个开始位置
			size_t single_comment_start = html.find("url('//") + 7;
			//
			//size_t single_comment_end ;
			//变量
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
			//PIC图片下载线程
			std::vector<std::shared_ptr<boost::thread>> ThreadVector;

			while ((single_comment_start - 7) != std::string::npos) {
				//临时变量
				std::string name;
				std::string pic_url;
				std::string comment_time;
				std::string comment;
				char star = '0';
				std::string pic_save_name;
				//PICDownload的文件保存位置
				std::string file_path;
				//头像图片Start=====

				//找到头像的结尾
				pic_end = html.find("')\"", single_comment_start);
				pic_url = "http://" + html.substr(single_comment_start, pic_end - single_comment_start);
				//找到文件名的开头
				temp = pic_url.find_last_of('/') + 1;
				//找到文件名结尾
				pic_name_end = pic_url.find_last_of('.');
				//图片名字
				pic_save_name = pic_url.substr(temp, pic_name_end - temp);
				//将图片大小换成l类型
				temp = pic_url.find("/s/");
				pic_url[temp + 1] = 'l';
				//下载图片
				auto result = PicDownload(http_client, pic_url, USER_PIC_PATH, pic_save_name, file_path, refresh);
				//返回的线程保存到返回值中
				if (result.first == DownloadStatus::MultiThread)
				{
					//将下载线程压入线程池中
					ThreadVector.push_back(result.second);
				}
				//头像图片OVER=====

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
				temp = html.find(" stars", time_end);
				if (temp != std::string::npos&&temp < content_start) {
					star = html[temp + 6];
					if (html[temp+7]!='"')
					{
						//说明10星
						star += html[temp + 7];
					}
				}
				//star end===

				//下个头像URL
				single_comment_start = html.find("url('//", single_comment_start) + 7;

				//ret回复
				ret << "[CQ:image,file=" << std::move(file_path) << "]"
					>> std::move(name) << ' ' << std::move(comment_time);
				if (star != '0') {
					ret >> ">>评分: " << star;
				}
				ret >> ">>" << std::move(comment);

				ret << "\n";

			}
			//等待图片下载完成
			for (auto &t : ThreadVector) {
				if (t != nullptr&&t->joinable())
					t->join();
			}

			return std::move(ret);
		}
		catch (const std::exception&)
		{
			return "访问失败...";
		}
	}

	//解析Subject中的部分信息
	inline bangumi::BangumiSubjectCollection ResolveSubjectCollection(const std::string &html, size_t subject_id, bool refresh = false) {
#define  MAX_SUBJECT_COLLECTION_EPS 150
		//返回的结果
		//Debug
		//{
		//	std::ofstream text_input("html.txt");
		//	text_input << html;
		//	text_input.close();
		//}
		////
		bangumi::BangumiSubjectCollection result;
		result.subject_id = subject_id;
		try {
			//直接从400处搜索
			size_t main_start = html.find("id=\"main\"", 400);
			//判断是否存在,其实一定存在,实际上可能是会员才能看到的条目
			if (main_start == std::string::npos) {
				result.valid = false;
				return result;
			}
			//main之后是title
			std::string name_cn;
			size_t title_start = html.find("title=\"", main_start);
			//如果存在
			if (title_start != std::string::npos) {
				size_t title_end = html.find("\"", title_start + 7);
				name_cn = html.substr(title_start + 7, title_end - title_start - 7);
			}
			result.name_cn = name_cn;

			//中文名之后是原名
			std::string name;
			size_t name_start = html.find("viewed\">", main_start);
			//如果存在
			if (name_start != std::string::npos) {
				size_t name_end = html.find("</a>", name_start);
				name = html.substr(name_start + 8, name_end - name_start - 8);
			}
			result.name = name;

			//图片
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
			//PIC图片下载线程
			std::vector<std::shared_ptr<boost::thread>> ThreadVector;
			if (pic_start != std::string::npos) {
				//存在图片
				size_t pic_end = info_str.find("\"", pic_start + 6);
				std::string pic_url = "http:" + info_str.substr(pic_start + 6, pic_end - pic_start - 6);

				//图片名字 (就是subject_id)
				std::string pic_save_name = std::to_string(subject_id);

				//下载图片
				auto result = PicDownload(http_client, pic_url, SUBJECT_PIC_PATH, pic_save_name, file_path, refresh);

				//返回的线程保存到返回值中
				if (result.first == DownloadStatus::MultiThread)
				{
					//将下载线程压入线程池中
					ThreadVector.push_back(result.second);
				}
			}
			else {
				//使用缺省图片
				//下载图片
				auto result = PicDownload(http_client, "", SUBJECT_PIC_PATH, "", file_path, refresh);
			}
			result.file_path = file_path;
			//图片OVER=====


			//章节
			int eps_num = 1;
			size_t chapter_start = html.find("prg_list\">", main_start);
			//章节详细
			size_t detail_start = html.find("subject_prg_content", chapter_start);
			std::string detail_chapter_html;
			//如果存在详细章节
			if (detail_start != std::string::npos) {
				size_t detail_end = html.find("</span></div></div>", detail_start);
				detail_chapter_html = html.substr(detail_start + 21, detail_end - detail_start - 21);
			}
			////Debug
			//{
			//	std::ofstream text_input("detail_chapter_html.txt");
			//	text_input << detail_chapter_html;
			//	text_input.close();
			//}
			if (chapter_start != std::string::npos) {

				size_t chapter_end = html.find("</ul>", chapter_start);
				std::string chapter = html.substr(chapter_start + 10, chapter_end - chapter_start - 10);
				//Debug
				//{
				//	std::ofstream text_input("chapter_html.txt");
				//	text_input << chapter;
				//	text_input.close();
				//}
				//检查是否有SP
				bool have_sp = false;
				size_t sp_pos = chapter.find("subtitle");
				if (sp_pos != std::string::npos) {
					//说明存在SP
					have_sp = true;
				}
				//循环处理每一个章节
				size_t ep_status = chapter.find("epBtn");
				size_t ep_title_start;
				size_t ep_title_end;
				size_t detail_ep_info_start;
				size_t detail_ep_info_end;
				std::string detail_ep_info;
				size_t info_content_start;
				//size_t info_content_end;
				std::string info_content;
				std::string ep_name;
				//记录放送状态的标志
				int ep_state;
				//循环处理章节
				while (ep_status != std::string::npos&& eps_num < MAX_SUBJECT_COLLECTION_EPS) {
					++eps_num;
					//
					//title
					ep_title_start = chapter.find("title=\"", ep_status);
					ep_title_end = chapter.find("\"", ep_title_start + 7);
					//ep name
					ep_name = chapter.substr(ep_title_start + 7, ep_title_end - ep_title_start - 7);
					//判断放送状态
					if (chapter[ep_status + 5] == 'A') {
						//说明是epBtnAir
						if (have_sp&&ep_status > sp_pos) {
							//说明是aired的sp
							result.sp_air_eps.push_back(ep_name);
							ep_state = 3;
						}
						else {
							//说明是aired的tv
							result.air_eps.push_back(ep_name);
							ep_state = 1;
						}
					}
					else {
						//否则是epBtnNA
						if (have_sp&&ep_status > sp_pos) {
							//说明是unaired的sp
							result.sp_unair_eps.push_back(ep_name);
							ep_state = 4;
						}
						else {
							//说明是unaired的tv
							result.unair_eps.push_back(ep_name);
							ep_state = 2;
						}
					}
					//详细章节的判断
					if (!detail_chapter_html.empty()) {
						//确定此ep的info编号
						detail_ep_info_start = chapter.find("rginfo_", ep_title_end);
						detail_ep_info_end = chapter.find("\"", detail_ep_info_start + 7);
						detail_ep_info = chapter.substr(detail_ep_info_start, detail_ep_info_end - detail_ep_info_start);
						//确定此ep的info信息
						info_content_start = detail_chapter_html.find(detail_ep_info);
						if (info_content_start != std::string::npos) {
							//如果存在
							bool save = false;
							//<hr结束
							for (int i = 0; info_content_start + i < detail_chapter_html.length(); ++i) {
								if (detail_chapter_html[info_content_start + i] == '>')
								{
									save = true;
									continue;
								}
								if (detail_chapter_html[info_content_start + i] == '<')
								{
									if (detail_chapter_html[info_content_start + i + 1] == 'b')
									{
										//说明是<br
										save = false;
										++i;
										info_content += ' ';
										continue;
									}
									if (detail_chapter_html[info_content_start + i + 1] == 'h')
									{
										//说明是<hr
										//加讨论
										save = false;
										++i;
										while (true) {
											++i;
											//</s结束
											if (detail_chapter_html[info_content_start + i] == '>')
											{
												while (detail_chapter_html[info_content_start + i + 1] == ' ') {
													//排除掉空格
													++i;
												}
												save = true;
												continue;
											}
											if (detail_chapter_html[info_content_start + i] == '<')
											{
												if (detail_chapter_html[info_content_start + i + 1] == '/'&&
													detail_chapter_html[info_content_start + i + 2] == 's') {
													//直接break
													break;
												}
												//其他情况直接false save
												save = false;
												continue;
											}
											if (save) {
												//如果不在<>内则直接保存
												info_content += detail_chapter_html[info_content_start + i];
											}

										}
										//保存数据
										switch (ep_state)
										{
										case 1:
											result.air_eps_info.push_back(info_content);
											break;
										case 2:
											result.unair_eps_info.push_back(info_content);
											break;
										case 3:
											result.sp_air_eps_info.push_back(info_content);
											break;
										case 4:
											result.sp_unair_eps_info.push_back(info_content);
											break;
										default:
											break;
										}
										//清空
										info_content = "";
										//结束
										break;
									}
									//其他情况直接false save
									save = false;
									continue;
								}
								if (save) {
									//如果不在<>内则直接保存
									//首先排除字符中的空格
									if (detail_chapter_html[info_content_start + i] != ' ')
										info_content += detail_chapter_html[info_content_start + i];
									else
										info_content += '_';
								}

							}
						}
						else {
							//几乎不可能的事情
							//但还是填入一个空的数据 
							info_content = "未知...";
							//保存数据
							switch (ep_state)
							{
							case 1:
								result.air_eps_info.push_back(info_content);
								break;
							case 2:
								result.unair_eps_info.push_back(info_content);
								break;
							case 3:
								result.sp_air_eps_info.push_back(info_content);
								break;
							case 4:
								result.sp_unair_eps_info.push_back(info_content);
								break;
							default:
								break;
							}
						}

					}
					//下一个ep_status
					ep_status = chapter.find("epBtn", ep_title_start);

				}
				//更新一下数量
				result.UpdateEpsCounts();

			}


			//章节Over


			//等待图片下载完成
			for (auto &t : ThreadVector) {
				if (t != nullptr&&t->joinable())
					t->join();
			}

		}
		catch (std::exception&) {
			//
		}
		return result;
	}

	//解析Tag信息
	//只解析一页
	inline bangumi::string ResolveTag(const std::string &raw_html, bool refresh = false) {
#define BGMTagMaxPageNum = 2
		try {
			//查找tag目录的开头
			size_t section_start = raw_html.find("section", 1000);
			//可能有意外问题
			if (section_start == std::string::npos) {
				//一般是无法打开html 502
				return "";
			}
			//查找结尾
			size_t section_end = raw_html.find("board", section_start);
			//有效的html
			std::string html = raw_html.substr(section_start, section_end - section_start);
			//单个条目的起始位置
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
			//PIC图片下载线程
			std::vector<std::shared_ptr<boost::thread>> ThreadVector;
			while (subject_start != std::string::npos) {

				//subject id
				size_t href_end = html.find("\"", subject_start + 5);

				subject_id = html.substr(subject_start + 5, href_end - subject_start - 5);
				//最后循环查找下一个item
				//同时限定此条目的查找范围
				subject_start = html.find("item_", subject_start + 10);

				//
				//std::cout << html<<std::endl;


				//中文名
				size_t name_cn_start = html.find("ass=\"l\">", href_end);
				size_t name_cn_end = html.find("</a>", name_cn_start + 8);
				name_cn = html.substr(name_cn_start + 8, name_cn_end - name_cn_start - 8);

				//由于有没有图片的风险，因此将判断放在中文名之后
				//图片下载地址
				size_t src_start = html.find("src=\"/", href_end);
				//可以真的没有图片
				if (src_start < name_cn_start) {
					size_t src_end = html.find("\"", src_start + 5);
					std::string pre_url = html.substr(src_start + 5, src_end - src_start - 5);
					std::string pic_subject_id = subject_id;
					if (pre_url[1] == 'i'&&pre_url[2] == 'm') {
						//img/no_icon
						pre_url = "//bgm.tv" + pre_url;
						pic_subject_id = "no_icon";
					}
					else {
						//将图片大小换成l类型
						temp = pre_url.find("/s/");
						pre_url[temp + 1] = 'm';
					}
					pic_url = "http:" + pre_url;
					//#ifndef NDEBUG
					//			{
					//				bangumi::string debug_msg;
					//				debug_msg << "图片下载地址 " << pic_url;
					//				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-TAG-PIC-DOWNLOAD", debug_msg);
					//			}
					//#endif
					//开始下载图片
					//下载图片
					auto result = PicDownload(http_client, pic_url, TAG_PIC_PATH, pic_subject_id, file_path, refresh);
					//返回的线程保存到返回值中
					if (result.first == DownloadStatus::MultiThread)
					{
						//将下载线程压入线程池中
						ThreadVector.push_back(result.second);
					}
				}
				else {
					//直接使用404图片
					auto result = PicDownload(http_client, "", TAG_PIC_PATH, subject_id, file_path, refresh);
					//返回的线程保存到返回值中
					//if (result.first == DownloadStatus::MultiThread)
					//{
					//	//将下载线程压入线程池中
					//	ThreadVector.push_back(result.second);
					//}
				}

				//原名(可能没有)
				name = "";
				size_t name_start = html.find("ass=\"grey\">", name_cn_end);
				if (name_start < subject_start) {
					//在此条目的范围内有效
					size_t name_end = html.find("</small>", name_start + 11);
					name = html.substr(name_start + 11, name_end - name_start - 11);
				}

				//info tips
				size_t tips_start = html.find("tip\">", name_cn_end);
				size_t tips_end = html.find("</p>", tips_start + 5);
				tips = html.substr(tips_start + 5, tips_end - tips_start - 5);
				//评分(可能没有)
				rate = "";
				size_t rate_start = html.find("fade\">", tips_end);
				if (rate_start < subject_start) {
					//在此条目的范围内有效
					size_t rate_end = html.find("<", rate_start + 6);
					rate = html.substr(rate_start + 6, rate_end - rate_start - 6);
				}
				//评分2(可能没有)
				rate_num = "";
				size_t rate2_start = html.find("tip_j\">", tips_end);
				if (rate2_start < subject_start) {
					//在此条目的范围内有效
					size_t rate2_end = html.find("<", rate2_start + 7);
					rate_num = html.substr(rate2_start + 7, rate2_end - rate2_start - 7);
				}


				//ret回复
				ret << "[CQ:image,file=" << std::move(file_path) << "]"
					>> "ID: " << subject_id
					>> std::move(name_cn);
				if (!name.empty()) {
					ret << " <" << std::move(name) << ">";
				}

				if (!tips.empty()) {
					//tips自带\n
					ret << tips;
				}
				if (!rate.empty()) {
					ret >> "评分: " << rate << " " << rate_num;
				}
				else if (!rate_num.empty()) {
					ret >> rate_num;
				}
				ret << "\n";



			}

			//等待图片下载完成
			for (auto &t : ThreadVector) {
				if (t != nullptr&&t->joinable())
					t->join();
			}
			//如果不为空
			if (!ret.empty()) {
				ret[ret.length() - 1] = ' ';
			}

			return ret;
		}
		catch (std::exception&) {
			return "访问失败...";
		}
	}

	//解析Staff
	inline bangumi::string ResolveStaff(const std::string &raw_html, size_t subject_id, bool refresh = false) {
		//查找封面
		//图片
		try {

			size_t info_start = raw_html.find("\"bangumiInfo\"", 800);
			if (info_start == std::string::npos) {
				//一般是无法打开html 502
				return "未收录Staff...";
			}
			size_t infobox_end = raw_html.find("id=\"infobox\"", info_start);
			std::string info_str = raw_html.substr(info_start + 13, infobox_end - info_start - 13);

			//
			size_t pic_start = info_str.find("href=\"");
			std::string file_path;
			//PIC图片下载线程
			std::vector<std::shared_ptr<boost::thread>> ThreadVector;
			if (pic_start != std::string::npos) {
				//存在图片
				size_t pic_end = info_str.find("\"", pic_start + 6);
				std::string pic_url = "http:" + info_str.substr(pic_start + 6, pic_end - pic_start - 6);

				//图片名字 (就是subject_id)
				std::string pic_save_name = std::to_string(subject_id);

				//下载图片
				auto result = PicDownload(http_client, pic_url, SUBJECT_PIC_PATH, pic_save_name, file_path, refresh);

				//返回的线程保存到返回值中
				if (result.first == DownloadStatus::MultiThread)
				{
					//将下载线程压入线程池中
					ThreadVector.push_back(result.second);
				}
			}
			else {
				//使用缺省图片
				//下载图片
				auto result = PicDownload(http_client, "", SUBJECT_PIC_PATH, "", file_path, refresh);
			}

			//图片OVER=====


			//查找Staff的开头
			size_t staff_start = raw_html.find("id=\"infobox\"", 800);
			//可能有意外问题
			if (staff_start == std::string::npos) {
				//一般是无法打开html 502
				return "";
			}
			size_t staff_end = raw_html.find("</ul>", staff_start);
			std::string staff_str = raw_html.substr(staff_start + 13, staff_end - staff_start - 13);
			//std::cout << staff_str << std::endl;
			//处理时需要的参数
			std::istringstream output(staff_str);
			std::string line_str;
			//判断是否在<>内,<>内的消息不需要
			bool drop = false;
			//返回的结果
			bangumi::string result;
			//首先插入封面
			result << "[CQ:image,file=" << file_path << "]\n";
			//循环处理第一行
			while (getline(output, line_str)) {
				//先验
				if (line_str.empty()) {
					continue;
				}
				//循环处理
				for (auto&c : line_str) {
					if (c == '<') {
						drop = true;
						continue;
					}
					if (c == '>') {
						drop = false;
						continue;
					}
					if (drop) {
						continue;
					}
					else {
						result << c;
					}
				}
				//换行
				result << '\n';
			}
			if (!result.empty()) {
				//去除最后的换行
				result[result.length() - 1] = ' ';
			}

			return result;

		}
		catch (const std::exception&)
		{
			return "访问失败...";
		}
	}

	//解析RSS
	inline std::pair<std::vector<bangumi::string>,bangumi::string> ResolveRSS(std::string &xml, BgmCode rss_type,int max_num,bangumi::string pre_str="", bool refresh = false) {
		//一条消息最多的条目数
#define RSS_ONE_MESSAGE_NUM 5
#define RSS_ALL_NUM 10
		//返回的结果
		//bangumi::string ret;
		std::vector<bangumi::string> ret;
		//首先填充一个
		ret.emplace_back(bangumi::string());
		//为其添加前缀字符
		ret[0] << pre_str;
		//处理xml为完整的xml
		try {
			if (xml.rfind("</rss>") == std::string::npos) {
				size_t last_item = xml.rfind("<item>");
				size_t over_last_item = xml.rfind("</item>");
				if (over_last_item > last_item) {
					last_item = over_last_item + 7;
				}
				//xml = xml.substr(0, last_item);
				xml.erase(last_item);
				xml += "</channel></rss>";
			}
		}
		catch(std::exception&e){
			ret[0] << "访问失败...";
			return{ ret,"" };
		}

		//窄字符
		boost::property_tree::ptree pt;
		std::istringstream input(xml);
		try {
			boost::property_tree::read_xml(input, pt);
		}
		catch (std::exception&e) {
			ret[0] << "访问失败...";
			return{ ret,"" };
		}
		//日期的处理
		//std::string strDateTime = "Fri, 17 May 2019 00:30:27 GMT";
		//"Fri, 17 May 2019 09:40:26 +0800"
		std::string date_format;
		switch (rss_type)
		{
		case BgmCode::DMHY:
			date_format = "%a, %d %b %Y %H:%M:%S +0800";
			break;
		case BgmCode::MOE:
			date_format = "%a, %d %b %Y %H:%M:%S GMT";
			break;
		default:
			break;
		}
		//所有下载图片的URL
		//主要用于防重复下载
		std::set<std::string> pic_dl_urls;
		//url对应path
		std::map<std::string, std::string> url2path;
		//PIC图片下载线程
		std::vector<std::shared_ptr<boost::thread>> ThreadVector;
		//开始解析
		//精简信息
		bangumi::string miniMess;
		std::vector<std::string> title_vec;
		std::vector<std::string> magn_vec;

		switch (rss_type)
		{

		case BgmCode::DMHY:
		{
			try
			{
				//直接赋值channel节点
				pt = pt.get_child("rss.channel");
				//编号
				int num = 1;
				int num_in_mssage = 1;
				//遍历item
				for (ptree::assoc_iterator iter2 = pt.find("item"); iter2 != pt.not_found(); ++iter2)
				{
					if (iter2->first != "item") {
						//如果此节点并非一个item节点
						break;
					}
					if (num == max_num + 1) {
						//到达用户需要
						break;
					}
					if (num_in_mssage == RSS_ONE_MESSAGE_NUM + 1) {
						//单条消息上限
						//填充一个string
						ret.emplace_back(bangumi::string());
						//重置
						num_in_mssage = 1;
					}
					if(num == RSS_ALL_NUM + 1) {
						//上限后直接
						break;
					}
					//标题
					std::string title = iter2->second.get<std::string>("title").data();
					//描述
					std::string description = iter2->second.get<std::string>("description").data();
					//图片网址
					std::string pic_url;
					//
					std::string pic_file_path = bgm.not_found_pic_path;
					//从描述中查找第一个图片
					size_t pic_src = description.find(".gif\"");
					size_t pic_src_start;
					if (pic_src != std::string::npos) {

					}
					else if (pic_src = description.find(".jpg\""), pic_src != std::string::npos) {

					}
					else if (pic_src = description.find(".png\""), pic_src != std::string::npos) {

					}
					if (pic_src != std::string::npos) {
						//向前找到起始src
						pic_src_start = description.rfind("src=\"http", pic_src);
						if (pic_src_start != std::string::npos) {
							pic_url = description.substr(pic_src_start + 5, pic_src + 4 - pic_src_start - 5);
						}
						//
						if (!pic_url.empty()) {
							if (pic_dl_urls.count(pic_url)==0) {
								//不存在
								pic_dl_urls.emplace(pic_url);
								//如果图片存在的话，进行下载操作
								auto result = HTTPSPicDownload(http_client, pic_url, OTHER_PIC_PATH, pic_file_path, refresh);
								//保存对应
								url2path.emplace(make_pair(pic_url, pic_file_path));
								//返回的线程保存到返回值中
								if (result.first == DownloadStatus::MultiThread)
								{
									//将下载线程压入线程池中
									ThreadVector.push_back(result.second);
								}
							}
							else {
								try {
									pic_file_path = url2path.at(pic_url);
								}
								catch (std::exception&) {}
								
							}
						}
					}
					//网址
					std::string link = iter2->second.get<std::string>("link").data();
					//发布日期
					std::string pubDate = iter2->second.get<std::string>("pubDate").data();
					//创建一个DATE类
					//std::string strDateTime = "Fri, 17 May 2019 00:30:27 +0800";
					boost::posix_time::ptime pub_time;
					std::stringstream ss(pubDate);
					boost::posix_time::time_input_facet* input_facet = new boost::posix_time::time_input_facet(date_format);
					ss.imbue(std::locale(ss.getloc(), input_facet));
					ss >> pub_time;
					std::string pubDate_ = boost::posix_time::to_iso_extended_string(pub_time);
					pubDate_[pubDate_.find_first_of('T')] = ' ';
					//发布人
					std::string author = iter2->second.get<std::string>("author").data();
					//发布类别
					std::string category = iter2->second.get<std::string>("category").data();
					//BT地址
					std::string bt_url = iter2->second.find("enclosure")->second.get<std::string>("<xmlattr>.url");
					//去除附加参数
					size_t extra = bt_url.find_first_of('&');
					if(extra!=std::string::npos)
						bt_url.erase(extra);
					//std::cout << title << description << link << pubDate;
					//std::cout << "\n================\n";
					//std::cout << bt_url;
					//std::cout << "\n================\n";
					//boost::this_thread::sleep(boost::posix_time::seconds(6));

					ret[ret.size()-1] << "----------------"
						>> "[CQ:image,file=" << pic_file_path << ']'
						>> "---编号[ " << num << " ]---"
						>> title
						>> "URL:" >> link
						>> "-----"
						>> "发布时间: " << pubDate_
						>> "种子链接:" >> bt_url
						>> "-----"
						>> "发布人: [ " << author << " ]"
						>> "资源分类: [" << category << "]\n";
						
					++num_in_mssage;
					++num;
					//压入数组
					title_vec.emplace_back(title);
					magn_vec.emplace_back(bt_url);
				}
				if (num == 1) {
					//如果没有正确处理一个
					//抛出一个异常
					ret[0] << "暂无资源...";
					return{ ret,miniMess };
				}
			}
			catch (std::exception& e)
			{
				//不存在此节点
				//自动忽略
				//std::cout << e.what();
				ret[0] << "...";
				return{ ret,miniMess };
			}
		}
			break;
		case BgmCode::MOE:
		{
			try
			{
				//直接赋值channel节点
				pt = pt.get_child("rss.channel");
				//编号
				int num = 1;
				int num_in_mssage = 1;
				//遍历item
				for (ptree::assoc_iterator iter2 = pt.find("item"); iter2 != pt.not_found(); ++iter2)
				{
#ifndef NDEBUG
					{
						bangumi::string debug_msg;
						debug_msg << "当前处理: " << num;
						CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-RSS-Resolve", debug_msg);
					}
#endif
					if (iter2->first != "item") {
						//如果此节点并非一个item节点
						break;
					}
					if (num == max_num + 1) {
						//到达用户需要
						break;
					}
					if (num_in_mssage == RSS_ONE_MESSAGE_NUM + 1) {
						//单条消息上限
						//填充一个string
						ret.emplace_back(bangumi::string());
						//重置
						num_in_mssage = 1;
					}
					if (num == RSS_ALL_NUM + 1) {
						//上限后直接
						break;
					}
					//标题
					std::string title = iter2->second.get<std::string>("title").data();
					//描述
					std::string description = iter2->second.get<std::string>("description").data();
					//图片网址
					std::string pic_url;
					//默认404
					std::string pic_file_path = bgm.not_found_pic_path;
					//从描述中查找第一个图片
					size_t pic_src = description.find(".gif\"");
					size_t pic_src_start;
					if (pic_src != std::string::npos) {

					}else if (pic_src = description.find(".jpg\""), pic_src != std::string::npos){

					}
					else if (pic_src = description.find(".png\""), pic_src != std::string::npos) {

					}
					if (pic_src != std::string::npos) {
						//向前找到起始src
						pic_src_start = description.rfind("src=\"http", pic_src);
						if (pic_src_start != std::string::npos) {
							pic_url = description.substr(pic_src_start + 5, pic_src + 4 - pic_src_start - 5);
						}
						if (!pic_url.empty()) {
							if (pic_dl_urls.count(pic_url) == 0) {
								//不存在
								pic_dl_urls.emplace(pic_url);
								//如果图片存在的话，进行下载操作
								auto result = HTTPSPicDownload(http_client, pic_url, OTHER_PIC_PATH, pic_file_path, refresh);
								//保存对应
								url2path.emplace(make_pair(pic_url, pic_file_path));
								//返回的线程保存到返回值中
								if (result.first == DownloadStatus::MultiThread)
								{
									//将下载线程压入线程池中
									ThreadVector.push_back(result.second);
								}
							}
							else {
								try {
									pic_file_path = url2path.at(pic_url);
								}
								catch (std::exception&) {}

							}
						}
					}

					//网址
					std::string link = iter2->second.get<std::string>("link").data();
					//发布日期
					std::string pubDate = iter2->second.get<std::string>("pubDate").data();
					//创建一个DATE类
					//std::string strDateTime = "Fri, 17 May 2019 00:30:27 GMT";
					boost::posix_time::ptime pub_time;
					std::stringstream ss(pubDate);
					boost::posix_time::time_input_facet* input_facet = new boost::posix_time::time_input_facet(date_format);
					ss.imbue(std::locale(ss.getloc(), input_facet));
					ss >> pub_time;
					//delete input_facet;
					//原为格林时间
					//使用东八区
					pub_time += boost::posix_time::hours(8);
					std::string pubDate_ = boost::posix_time::to_iso_extended_string(pub_time);
					pubDate_[pubDate_.find_first_of('T')] = ' ';
					//bt链接
					std::string bt_url = iter2->second.find("enclosure")->second.get<std::string>("<xmlattr>.url");
					//进行URLENCODE
					std::string encode_url = url_encode(bt_url);
					//std::cout << encode_url;


					//消息拼接
					ret[ret.size() - 1] << "----------------"
						>> "[CQ:image,file=" << pic_file_path << ']'
						>> "---编号[ " << num << " ]---"
						>> title
						>> "URL:" >> link
						>> "-----"
						>> "发布时间: " << pubDate_
						>> "种子链接:" >> encode_url << '\n';

					++num;
					++num_in_mssage;
					//压入数组
					title_vec.emplace_back(title);
					magn_vec.emplace_back(encode_url);
				}
				if (num == 1) {
					//如果没有正确处理一个
					//抛出一个异常
					ret[0] << "暂无资源...";
					return{ ret,miniMess };
				}
			}
			catch (std::exception& e)
			{
				//不存在此节点
				//自动忽略
				//std::cout << e.what();
				ret[0] << "...";
				return{ ret,miniMess };
			}
		}
			break;
		default:
			break;
		}
		//生成精简信息
		miniMess << "----<全部资源>----";
		for (auto& t : title_vec) {
			miniMess >> t;
		}
		miniMess >> "----<链接>----";
		for (auto& m : magn_vec) {
			miniMess >> m;
		}

#ifndef NDEBUG
		{
			bangumi::string debug_msg;
			debug_msg << "HTTPS处理图片开始" << "[" << ThreadVector.size() << "]";
			CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-RSS-Resolve", debug_msg);
		}
#endif
		//等待图片下载完成
		for (auto &t : ThreadVector) {
#ifndef NDEBUG
			{
				bangumi::string debug_msg;
				debug_msg << "图片完结+1";
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-RSS-Resolve", debug_msg);
			}
#endif
			if (t != nullptr&&t->joinable())
				t->join();
		}
#ifndef NDEBUG
		{
			bangumi::string debug_msg;
			debug_msg << "HTTPS处理图片完结";
			CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-RSS-Resolve", debug_msg);
		}
#endif
		//如果不为空
		for (auto& str : ret) {
			if (!str.empty()) {
				str[str.length() - 1] = ' ';
			}
		}
		//释放资源
		//delete input_facet;
		//返回
		return{ ret,miniMess };
	}

	//解析关联条目
	inline bangumi::string ResolveAttach(const std::string &raw_html, size_t subject_id, bool refresh = false) {
		//查找封面
		//图片
		try {

			size_t attach_start = raw_html.find("关联条目", 800);
			if (attach_start == std::string::npos) {
				//一般是无法打开html 502
				return "未收录关联条目...";
			}
			size_t attach_end = raw_html.find("</li></ul>", attach_start);
			std::string attach_str = raw_html.substr(attach_start + 9, attach_end - attach_start - 9);
			//返回的结果
			bangumi::string result;
			result << "[条目: " << subject_id << "]";
			result >> "<-关联条目->:\n";
			//开始循环查找
			//起始的位置
			size_t one_start = attach_str.find("sub\">");
			//PIC图片下载线程
			std::vector<std::shared_ptr<boost::thread>> ThreadVector;

			while (one_start != std::string::npos) {
				//直接加上字符数
				one_start += 5;
				//关联的名字
				std::string attach_name;
				//填充关联的名字
				while (attach_str[one_start]!='<') {
					attach_name += attach_str[one_start];
					++one_start;
				}
				//查找图片的url
				//
				size_t pic_start = attach_str.find("url('", one_start);
				//图片路径
				std::string file_path;
				//此subject_id
				size_t this_subject_id;
				//temp
				size_t temp;
				if (pic_start != std::string::npos) {
					//图片名字 (就是subject_id)
					std::string pic_save_name;
					//存在图片
					size_t pic_end = attach_str.find("'", pic_start + 5);
					std::string pre_url = attach_str.substr(pic_start + 5, pic_end - pic_start - 5);
					if (pre_url[1] == 'i'&&pre_url[2] == 'm') {
						//img/no_icon
						pre_url = "//bgm.tv" + pre_url;
						//图片名字
						pic_save_name = "no_icon";
					}
					else {
						//将图片大小换成l类型
						temp = pre_url.find("/m/");
						pre_url[temp + 1] = 'l';
						//获取图片中的名字
						temp = pre_url.find_last_of('/');
						this_subject_id = std::stoul(pre_url.substr(temp + 1));
						//图片名字 (就是subject_id)
						pic_save_name = std::to_string(this_subject_id);
					}
					std::string pic_url = "http:" + pre_url;


					//下载图片
					auto result = PicDownload(http_client, pic_url, SUBJECT_PIC_PATH, pic_save_name, file_path, refresh);

					//返回的线程保存到返回值中
					if (result.first == DownloadStatus::MultiThread)
					{
						//将下载线程压入线程池中
						ThreadVector.push_back(result.second);
					}
				}
				else {
					//使用缺省图片
					//下载图片
					auto result = PicDownload(http_client, "", SUBJECT_PIC_PATH, "", file_path, refresh);
				}
				//图片OVER=====
				//此关联条目的标题
				size_t title_start;
				title_start = attach_str.find("title\">", one_start);
				//关联的标题
				std::string attach_title;
				if (title_start!=std::string::npos)
				{
					//如果存在
					title_start += 7;
					//填充关联的标题
					while (attach_str[title_start] != '<') {
						attach_title += attach_str[title_start];
						++title_start;
					}
				}
				//结果输出
				//首先判断同一类
				if (attach_name.empty())
				{
					//result >> '[' << attach_name << ']';
					result >> "[CQ:image,file=" << file_path << "]";
					result >> attach_title;
					result >> "条目主页: https://bgm.tv/subject/" << this_subject_id;
					result >> "--------";
				}
				else
				{
					result >> '[' << attach_name << ']';
					result >> "[CQ:image,file=" << file_path << "]";
					result >> attach_title;
					result >> "条目主页: https://bgm.tv/subject/" << this_subject_id;
					result >> "--------";
				}
				//继续查找下一个关联条目
				one_start = attach_str.find("sub\">", one_start);
			}

			//等待图片下载完成
			for (auto &t : ThreadVector) {
				if (t != nullptr&&t->joinable())
					t->join();
			}

			return result;

		}
		catch (const std::exception&)
		{
			return "访问失败...";
		}
	}

	//解析时间线
	inline bangumi::string ResolveTimeLine(const std::string &raw_html, bool refresh = false) {
		try {

			size_t tml_start = raw_html.find("\"timeline\">", 800);
			if (tml_start == std::string::npos) {
				//一般是无法打开html 502
				return "访问失败...";
			}
			size_t tml_end = raw_html.find("tmlPager", tml_start);
			std::string tml_str = raw_html.substr(tml_start + 10, tml_end - tml_start - 19);
			//PIC图片下载线程
			std::vector<std::shared_ptr<boost::thread>> ThreadVector;
			//返回的结果
			bangumi::string result;
			bangumi::string tml;
			bangumi::string temp_url;
			bangumi::string temp_file_path;
			bangumi::string temp_pic_name;
			//是否写入
			bool write = false;
			bool h_after_line = false;
			//开始循环查找
			for (int c = 0; c < tml_str.length(); ++c) {
				//如果是一个标签的开始
				if (tml_str[c] == '<') {
					//首先之后不进行写入,同时也全部0
					write = false;
					//预读一位
					switch (tml_str[++c])
					{
					case 's':
					{
						//特殊
						while (++c) {
							if (tml_str[c + 1] == '>')
								break;
							if (tml_str[c + 1] != '(') {
								//进行评分与吐槽的判断
								if (tml_str[c + 1] == 'a'&&tml_str[c + 2] == 'r'&&tml_str[c + 3] == 's'&&tml_str[c-2]==' ') {
									c += 3;
									bangumi::string star;
									while (tml_str[++c] != '"') {
										star << tml_str[c];
									}
									tml << "评分: " <<std::move(star);
									break;
								}
								continue;
							}
							//进行图片的判断	
							else if (tml_str[c + 2] == '\'') {
								//说明到了\\lain.xxx
								temp_url << "http:";
								c += 2;
								while (tml_str[++c]!='\'') {
									temp_url << tml_str[c];
								}
#ifndef NDEBUG
								{
									bangumi::string debug_msg;
									debug_msg << "temp_url: "<<temp_url;
									CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Timeline", debug_msg);
								}
#endif
								//进行图片的下载
								//将图片大小换成l类型
								size_t	temp = temp_url.find("/m/");
								temp_url[temp + 1] = 'l';
								//获取图片中的名字
								temp = temp_url.find_last_of('/');
								size_t temp2 = temp_url.find_last_of('.');
								temp_pic_name = temp_url.substr(temp + 1, temp2 - temp - 1);

								//
								//下载图片
								auto result = PicDownload(http_client, temp_url, USER_PIC_PATH, temp_pic_name, temp_file_path, refresh);

								//返回的线程保存到返回值中
								if (result.first == DownloadStatus::MultiThread)
								{
									//将下载线程压入线程池中
									ThreadVector.push_back(result.second);
								}

								//保存图片消息
								if (h_after_line) {
									tml << "\n[CQ:image,file=" << temp_file_path << "]\n";
									h_after_line = false;
								}
								else
									tml << "[CQ:image,file=" << temp_file_path << "]\n";
							}
						}
					}
					break;
					case 'd':
					{
						//换行
						//简单判断一下图片问题 i
						if(tml_str[c+11]!='i')
							tml>> ">>";
					}
					break;
					case 'h':
					{
						tml << "● ";
						h_after_line = true;
					}
					break;
					case 'p':
					{
						//时间
						//简单判断一下图片问题 i
						tml >> "@ ";
					}
					break;
					case '/':
					{
						//判断是否结束一个记录
						if (tml_str[c + 1] == 'l') {
							//此条时间线保存
							result << tml;
							result >> "-------\n";
							//清空时间线
							tml = "";
							temp_url = "";
							temp_file_path = "";
						}
					}
					break;
					default:
						break;
					}
					//继续下一个
					continue;
				}
				//如果是一个标签的结束
				if (tml_str[c] == '>') {
					//置写入为真
					write = true;
					//继续下一个
					continue;
				}
				//判断是否需要写入
				if (write) {
					if (tml_str[c]!='\n')
					{
						if (tml_str[c] == '/'&&tml_str[c + 1] == '/')
						{
							c += 1;
							continue;
						}
						tml << tml_str[c];
					}
					
				}
			}
			//等待图片下载完成
			for (auto &t : ThreadVector) {
				if (t != nullptr&&t->joinable())
					t->join();
			}
			//去除换行
			if (!result.empty()) {
				result[result.length() - 1] = ' ';
			}
			return result;

		}
		catch (const std::exception&)
		{
			return "访问失败...";
		}
	}

	//解析具体用户时间线RSS
	inline bangumi::string ResolveTimeLineRSS(std::string &xml, std::string user_id, bool refresh = false) {
		//返回的结果
		//bangumi::string ret;
		bangumi::string ret;

		//窄字符
		boost::property_tree::ptree pt;
		std::istringstream input(xml);
		try {
			boost::property_tree::read_xml(input, pt);
		}
		catch (std::exception&e) {
			ret << "访问失败...";
			return ret;
		}
		//用户头像
		std::string & json = GetHtml("/user/" + user_id, bgm.bangumi_api_url);
		//宽解析树
		boost::property_tree::ptree json_pt;
		//因为解析需要一个流输入,因此使用stringstream,也支持文件流读取
		std::istringstream json_input(json);
		//解析json
		boost::property_tree::read_json(json_input, json_pt);
		//
		size_t id = json_pt.get<size_t>("id", 0);
		std::string url = json_pt.get<std::string>("avatar.large", "");
		std::string pic_name = std::to_string(id);

		//PIC图片下载线程
		std::vector<std::shared_ptr<boost::thread>> ThreadVector;
		//
		std::string pic_file_path;
		//下载图片
		if (!url.empty()) {
			//下载图片
			auto result = PicDownload(http_client, url, USER_PIC_PATH, pic_name, pic_file_path, refresh);

			//返回的线程保存到返回值中
			if (result.first == DownloadStatus::MultiThread)
			{
				//将下载线程压入线程池中
				ThreadVector.push_back(result.second);
			}

			//保存图片消息
			ret << "[CQ:image,file=" << pic_file_path << "]";

		}
		//日期的处理
		//std::string strDateTime = "Fri, 17 May 2019 00:30:27 GMT";
		//"Fri, 17 May 2019 09:40:26 +0800"
		std::string date_format;
		date_format = "%a, %d %b %Y %H:%M:%S +0000";
		//所有下载图片的URL

		//开始解析
		try
		{
			//直接赋值channel节点
			pt = pt.get_child("rss.channel");
			//标题
			std::string all_title = pt.get<std::string>("title").data();
			if (all_title[all_title.length()-1])
			{
				all_title[all_title.length() - 1] = ' ';
			}
			ret << all_title;
			//主页
			std::string user_url = pt.get<std::string>("link").data();
			//是否有时间线
			bool have_time_line = false;
			//遍历item
			for (ptree::assoc_iterator iter2 = pt.find("item"); iter2 != pt.not_found(); ++iter2)
			{
				if (iter2->first != "item") {
					//如果此节点并非一个item节点
					break;
				}
				if (!have_time_line)
				{
					have_time_line = true;
				}
				//标题
				std::string title = iter2->second.get<std::string>("title").data();
				//描述
				//std::string description = iter2->second.get<std::string>("description").data();
				//网址
				//std::string link = iter2->second.get<std::string>("link").data();
				//发布日期
				std::string pubDate = iter2->second.get<std::string>("pubDate").data();
				//创建一个DATE类
				//std::string strDateTime = "Fri, 17 May 2019 00:30:27 +0000";
				boost::posix_time::ptime pub_time;
				std::stringstream ss(pubDate);
				boost::posix_time::time_input_facet* input_facet = new boost::posix_time::time_input_facet(date_format);
				ss.imbue(std::locale(ss.getloc(), input_facet));
				ss >> pub_time;
				pub_time += boost::posix_time::hours(8);
				std::string pubDate_ = boost::posix_time::to_iso_extended_string(pub_time);
				pubDate_[pubDate_.find_first_of('T')] = ' ';
				////发布人
				//std::string author = iter2->second.get<std::string>("author").data();
				////发布类别
				//std::string category = iter2->second.get<std::string>("category").data();
				////BT地址
				//std::string bt_url = iter2->second.find("enclosure")->second.get<std::string>("<xmlattr>.url");
				////去除附加参数
				//size_t extra = bt_url.find_first_of('&');
				//if (extra != std::string::npos)
				//	bt_url.erase(extra);
				//std::cout << title << description << link << pubDate;
				//std::cout << "\n================\n";
				//std::cout << bt_url;
				//std::cout << "\n================\n";
				//boost::this_thread::sleep(boost::posix_time::seconds(6));

				ret >> "-------"
					>> title
					>> "@ " << pubDate_;
					

			}
			if (!have_time_line) {
				//如果没有正确处理一个
				//抛出一个异常
				ret << "暂无时间线...";
				return ret;
			}
			ret >> "-------\n用户主页: " << user_url;
		}
		catch (std::exception& e)
		{
			//不存在此节点
			//自动忽略
			//std::cout << e.what();
			ret << "访问失败...";
			return ret;
		}

		//等待图片下载完成
		for (auto &t : ThreadVector) {
			if (t != nullptr&&t->joinable())
				t->join();
		}
		//释放资源
		//delete input_facet;
		//返回
		return ret;
	}

	bangumi::string Resolve_User_Collection_Sum(std::string &json) {
		//[
		//{
		//	"type": 1,
		//		"name" : "book",
		//		"name_cn" : "书籍",
		//		"collects" : [
		//		{
		//		"status": {
		//			"id": 3,
		//				"type" : "do",
		//				"name" : "在读"
		//			},
		//			"count" : 2
		//		}
		//		]
		//},

		//null
		try
		{
			bangumi::string ret("● 收藏统计:");
			//完全不需要在底层来对\u进行解码,property_tree会进行这项工作的,前提是使用wptree
			//将string转换wstring直接使用C++ 11的构造函数
			std::wstring wjson(json.begin(), json.end());
			//宽解析树
			boost::property_tree::wptree pt;
			//因为解析需要一个流输入,因此使用stringstream,也支持文件流读取
			std::wistringstream input(wjson);
			//解析json
			boost::property_tree::read_json(input, pt);
			//解析元素,[注意]:解析后的都是UTF编码,并非GBK,后面会用Locale进行转换


			//首先判断是否有效
			//TODO:完成无效回复
			int code = pt.get<int>(L"code", 200);
			if (code != 200) {
				//请求是无效的
#ifndef NDEBUG
				{
					bangumi::string debug_msg;
					debug_msg << "未找到该用户";
					CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Resolve-User", debug_msg);
				}
#endif
				//std::shared_ptr<bangumi::Msg_Interface> msg(new bangumi::Reply("查找用户失败..."));
				//return{ ThreadVector, bangumi::Reply("查找用户失败...") };
				//直接抛出异常来给上层(Function)提供构造信息
				throw boost::system::system_error(bangumi_bot_errors::empty_user);
			}
			//取得列表
			boost::property_tree::wptree& list = pt;
			boost::property_tree::wptree::iterator pos = list.begin();
			//提前遍历一遍查找最大数
			int collect_max_num = 0;
			for (; pos != list.end(); ++pos) {
				//使用局部变量
				auto &pt = pos->second;
				//取得收藏列表
				boost::property_tree::wptree collects_list = pt.get_child(L"collects");
				boost::property_tree::wptree::iterator collects_pos = collects_list.begin();
				//遍历收藏列表
				for (; collects_pos != collects_list.end(); ++collects_pos) {

					//使用局部变量
					auto &pt = collects_pos->second;
					//状态
					auto &status = pt.get_child(L"status");

					int counts = pt.get<int>(L"count", 0);
					if (collect_max_num < counts)
					{
						collect_max_num = counts;
					}
				}
			}
			pos = list.begin();

			//遍历种类列表
			for (; pos != list.end(); ++pos) {
				//使用局部变量
				auto &pt = pos->second;

				//std::wstring name = pt.get<std::wstring>(L"name", L"???");
				std::wstring name_cn = pt.get<std::wstring>(L"name_cn", L"???");
				//写入消息
				//注意是UTF-16格式
				ret >> "● " << UTF16ToGB(name_cn) ;
				//取得收藏列表
				boost::property_tree::wptree collects_list = pt.get_child(L"collects");
				boost::property_tree::wptree::iterator collects_pos = collects_list.begin();
				//遍历收藏列表
				for (; collects_pos != collects_list.end(); ++collects_pos) {
					//使用局部变量
					auto &pt = collects_pos->second;
					//状态
					auto &status = pt.get_child(L"status");

					//std::wstring name = pt.get<std::wstring>(L"name", L"???");
					std::wstring status_name = status.get<std::wstring>(L"name", L"???");

					int counts = pt.get<int>(L"count", 0);

					//添加信息
					ret >> UTF16ToGB(status_name) << "| "<<CalPhyProgress(counts, collect_max_num)<<' ' << counts;

				}
			}
			
			//返回消息
			return ret;

		}
		catch (boost::system::system_error & e)
		{
			//Boost方面的问题
			//同时可以通过这个catch来构造失败消息
			throw e;
		}
		//property_tree在解析失败时并不会抛出system_error而是exception
		catch (std::exception &e) {
#ifndef NDEBUG
			{
				bangumi::string debug_msg;
				debug_msg << e.what();
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Resolve-User", debug_msg);
			}
#endif
			//解析树的问题
			throw boost::system::system_error(bangumi_bot_errors::json_resolve_error);
			//std::cout << "Error! Code = "
			//	<< "Message = " << e.what() << std::endl;
		}


	}

	//解析用户收藏信息
	//只解析一页
	inline std::pair<bangumi::string,bangumi::string> ResolveUserCollection(const std::string &raw_html, bool refresh = false) {
#define ONE_MAX_SUBJECT_IN_CO 12
		try {
			//查找收藏目录的开头
			size_t section_start = raw_html.find("browserItemList", 800);
			//可能有意外问题
			if (section_start == std::string::npos) {
				//一般是无法打开html 502
				return {"无法访问...", ""	};
			}
			//查找结尾
			size_t section_end = raw_html.find("footer", section_start);
			//有效的html
			std::string html = raw_html.substr(section_start, section_end - section_start);
			//单个条目的起始位置
			size_t subject_start = html.find("item_");
			//
			bangumi::string ret[2];
			int current_n = 0;
			int current_sub_num = 0;
			//
			std::string subject_id;
			std::string pic_url;
			std::string file_path;
			std::string name_cn;
			std::string name;
			std::string tips;
			std::string rate;
			std::string collect_time;
			std::string comment;
			size_t temp;
			//
			//PIC图片下载线程
			std::vector<std::shared_ptr<boost::thread>> ThreadVector;
			while (subject_start != std::string::npos) {
				++current_sub_num;
				if (current_sub_num > ONE_MAX_SUBJECT_IN_CO)
				{
					current_n = 1;
					current_sub_num = 0;
				}
				//subject id
				size_t href_end = html.find("\"", subject_start + 5);

				subject_id = html.substr(subject_start + 5, href_end - subject_start - 5);
				//最后循环查找下一个item
				//同时限定此条目的查找范围
				subject_start = html.find("item_", subject_start + 10);

				//
				//std::cout << html<<std::endl;


				//中文名
				size_t name_cn_start = html.find("ass=\"l\">", href_end);
				size_t name_cn_end = html.find("</a>", name_cn_start + 8);
				name_cn = html.substr(name_cn_start + 8, name_cn_end - name_cn_start - 8);

				//由于有没有图片的风险，因此将判断放在中文名之后
				//图片下载地址
				size_t src_start = html.find("src=\"/", href_end);
				//可以真的没有图片
				if (src_start < name_cn_start) {
					size_t src_end = html.find("\"", src_start + 5);
					std::string pre_url = html.substr(src_start + 5, src_end - src_start - 5);
					std::string pic_subject_id = subject_id;
					if (pre_url[1] == 'i'&&pre_url[2] == 'm') {
						//img/no_icon
						pre_url = "//bgm.tv" + pre_url;
						pic_subject_id = "no_icon";
					}
					else {
						//将图片大小换成l类型
						temp = pre_url.find("/s/");
						pre_url[temp + 1] = 'l';
					}
					pic_url = "http:" + pre_url;
					//#ifndef NDEBUG
					//			{
					//				bangumi::string debug_msg;
					//				debug_msg << "图片下载地址 " << pic_url;
					//				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-TAG-PIC-DOWNLOAD", debug_msg);
					//			}
					//#endif
					//开始下载图片
					//下载图片
					auto result = PicDownload(http_client, pic_url, SUBJECT_PIC_PATH, pic_subject_id, file_path, refresh);
					//返回的线程保存到返回值中
					if (result.first == DownloadStatus::MultiThread)
					{
						//将下载线程压入线程池中
						ThreadVector.push_back(result.second);
					}
				}
				else {
					//直接使用404图片
					auto result = PicDownload(http_client, "", TAG_PIC_PATH, subject_id, file_path, refresh);
					//返回的线程保存到返回值中
					//if (result.first == DownloadStatus::MultiThread)
					//{
					//	//将下载线程压入线程池中
					//	ThreadVector.push_back(result.second);
					//}
				}

				//原名(可能没有)
				name = "";
				size_t name_start = html.find("ass=\"grey\">", name_cn_end);
				if (name_start < subject_start) {
					//在此条目的范围内有效
					size_t name_end = html.find("</small>", name_start + 11);
					name = html.substr(name_start + 11, name_end - name_start - 11);
				}

				//info tips
				size_t tips_start = html.find("tip\">", name_cn_end);
				size_t tips_end = html.find("</p>", tips_start + 5);
				tips = html.substr(tips_start + 5, tips_end - tips_start - 5);
				//评分(可能没有)
				rate = "";
				size_t rate_start = html.find("sstars", tips_end);
				if (rate_start < subject_start) {
					//在此条目的范围内有效
					size_t rate_end = html.find(" ", rate_start);
					rate = html.substr(rate_start + 6, rate_end - rate_start - 6);
				}
				//收藏日期
				collect_time = "";
				size_t collect_time_start = html.find("tip_j\">", tips_end);
				if (collect_time_start < subject_start) {
					//在此条目的范围内有效
					size_t collect_time_end = html.find("<", collect_time_start + 7);
					collect_time = html.substr(collect_time_start + 7, collect_time_end - collect_time_start - 7);
				}
				//吐槽
				comment = "";
				size_t comment_start = html.find("\"text\">", tips_end+16);
				if (comment_start < subject_start)
				{
					//在此条目的范围内有效
					size_t comment_end = html.find("<", comment_start + 7);
					comment = html.substr(comment_start + 7, comment_end - comment_start - 7);
				}


				//ret回复
				ret[current_n] << "[CQ:image,file=" << std::move(file_path) << "]"
					>> "ID: " << subject_id
					>> std::move(name_cn);
				if (!name.empty()) {
					ret[current_n] << " <" << std::move(name) << ">";
				}
				//条目的简介
				if (!tips.empty()) {
					//tips自带\n
					tips[0] = ' ';
					ret[current_n] >> "简介:"<<tips;
				}
				//评分
				if (!rate.empty()) {
					try {
						int num = std::stoi(rate);
						std::string temp;
						for (int i = 0; i<num ;++i)
						{
							temp += "★";
						}
						ret[current_n] >> "评分: " << temp <<' '<< rate;
					}
					catch (std::exception&) {

					}
				}
				//吐槽
				if (!comment.empty()) {
					ret[current_n] >> "吐槽: " << comment;
				}
				//收藏时间
				if (!collect_time.empty()) {
					ret[current_n] >>"收藏于 <"<< collect_time<<'>';
				}
				ret[current_n] << "\n";



			}

			//判断当前页是否有结果
			if (current_sub_num == 0 && current_n == 0) {
				ret[0] << "没有相关收藏结果...\n";
			}
			//等待图片下载完成
			for (auto &t : ThreadVector) {
				if (t != nullptr&&t->joinable())
					t->join();
			}
			//如果不为空
			if (!ret[0].empty()) {
				ret[0][ret[0].length() - 1] = ' ';
			}
			//如果不为空
			if (!ret[1].empty()) {
				ret[1][ret[1].length() - 1] = ' ';
			}

			return{ ret[0],ret[1] };
		}
		catch (std::exception&) {
			return{ "访问失败...","" };
		}
	}

}