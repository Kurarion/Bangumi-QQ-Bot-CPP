//#include "Http.h"
//#include "Database.h"
#include "Init.h"
#include "Resolve.h"
#include "HttpServer.h"
#include "BaseDataStruct.h"
#include"boost/format.hpp"

#ifndef BANGUMI_FUNCTION_H
#define BANGUMI_FUNCTION_H
//默认处理宏
//一个函数可能发送多个消息,在宏函数中使用{}域限定即可解决变量重定义的问题
//鉴于当前多个消息几乎不常用,暂不修改
#define DEFAULT_SEND(type,msg)\
{\
int64_t to_someone;\
switch (type)\
{\
case BgmRetType::Private:\
	to_someone = param.qq;\
	break;\
case BgmRetType::Group:\
case BgmRetType::Discuss:\
	to_someone = param.group;\
	break;\
default:\
	break;\
}\
SendMsg.at(type)(ac, to_someone, msg);\
}
//基本功能函数
namespace bangumi {
	bangumi::AuthReply RefreshToken(std::string refresh_token, int64_t qq) {
		//重新Refresh
		bangumi::string content1;
		content1 << "grant_type=refresh_token"
			<< "&refresh_token=" << refresh_token
			<< "&redirect_uri=" << bgm.redirect_url
			<< "&client_id=" << bgm.bangumi_client_id
			<< "&client_secret=" << bgm.bangumi_client_secret;

		//post中的www编码其实是为了防止URI过长而将参数放在了content中,POST时一定不能忘记使用Content-Length指定大小
		//否则会出现不能正常读取参数的问题
		bangumi::string header1("Cache-Control: no-cache\r\n"
			"Content-Type: application/x-www-form-urlencoded\r\n");
		header1 << "Content-Length: " << content1.length() << "\r\n";

		std::string request1 = "POST "  "/oauth/access_token"  " HTTP/1.1\r\n"
			"Host: " "bgm.tv" "\r\n" + header1 + "\r\n" + content1;

		std::string json1 = http_client.SyncBGMHTTPRequest(request1);

#ifndef NDEBUG
		{
			bangumi::string debug_msg;
			debug_msg << "收到响应的Json"
				>> json1.c_str();
			CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Func-RefreshToken", debug_msg);
		}
#endif

		auto auth = Resolve::Resolve_Auth_Refresh(json1);

		bangumi::string update_query;

		update_query << "UPDATE bgm_users SET "
			//<< "user_bangumi=" << auth.user_id << ","
			<< "user_access_token='" << auth.access_token << "',"
			<< "user_refresh_token='" << auth.refresh_token << "'"
			<< " WHERE user_qq=" << qq;

		//如果可以的话,开启一个线程去执行
		//创建一个新的进程
		if (bgm.CheckThreadSize()) {
			//有空闲可用的进程
			std::shared_ptr<boost::thread> sql_thread
			(new boost::thread([update_query]() {
#ifndef NDEBUG
				{
					std::ostringstream oss;
					oss << boost::this_thread::get_id();
					std::string idAsString = oss.str();
					std::string test = "开启新的线程 ID: " + idAsString +
						"\n总池大小: " + std::to_string(bgm.threadpool_size) + "\n"
						"可用大小: " + std::to_string(bgm.curr_thread_size);
					CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Subject-SQL-MultiThread", test.c_str());
				}
#endif

				//此处进行SQL插入
				auto affect_rows_num = sql_pool.ExecQueryNoRes(update_query);
				try
				{
					SQLCheckResult(affect_rows_num);
				}
				catch (boost::system::system_error& e)
				{
#ifndef NDEBUG
					{
						bangumi::string debug_msg;
						debug_msg << "VerifyToken失败:"
							<< std::to_string(affect_rows_num)
							>> e.what();

						CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Func-VerifyToken", debug_msg);
					}
#endif	
					//这里不结束
					//通过异常结束函数
					//throw e;
				}

				bgm.AddAVAThreadSize();
#ifndef NDEBUG
				{
					bangumi::string debug_str;
					debug_str << "VerifyToken线程完成";
					CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Func-VerifyToken", debug_str);
				}
#endif
			}));
			//直接分离线程
			sql_thread->detach();
		}
		else {
			//没有可用的线程,同步执行
			auto affect_rows_num = sql_pool.ExecQueryNoRes(update_query);
			try
			{
				SQLCheckResult(affect_rows_num);
			}
			catch (boost::system::system_error& e)
			{
#ifndef NDEBUG
				{
					bangumi::string debug_msg;
					debug_msg << "VerifyToken失败:"
						<< std::to_string(affect_rows_num)
						>> e.what();

					CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Func-VerifyToken", debug_msg);
				}
#endif	
				//这里不结束
				//通过异常结束函数
				//throw e;
			}
		}

		return auth;
	}

	std::pair<size_t,std::pair<std::string,std::string>> VerifyToken(const BGMRetParam &param,bool focus_verify = false) {
		//查询语句
		bangumi::string query;
		//SELECT * FROM bgm_subjects WHERE subject_id=1
		query << "SELECT user_bangumi,user_access_token,user_refresh_token FROM bgm_users WHERE user_qq="
			<< param.qq;
		//查询结果
		BGMSQLResult result;
		//影响的行数
		unsigned long affect_rows_num = sql_pool.ExecQuery(query, result);
		try
		{
			SQLCheckResult(affect_rows_num);
		}
		catch (boost::system::system_error& e)
		{
#ifndef NDEBUG
			{
				bangumi::string debug_msg;
				debug_msg << "SQLVerifyTkoken查询失败:"
					<< std::to_string(affect_rows_num)
					>> e.what();

				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-VerifyToken", debug_msg);
			}
#endif	
			//通过异常结束函数
			throw e;
		}
		if (affect_rows_num > 0) {
			//数据库中存储的user_id
			size_t sql_bangumi_id_num;
			try {
				sql_bangumi_id_num = std::stoul(result[0]);
				//return{ sql_bangumi_id_num,result[1] };
			}
			catch (std::invalid_argument&e)
			{
#ifndef NDEBUG
				{
					bangumi::string debug_msg;
					debug_msg << "Func VerifyAccessToken stoul failed!"

						>> e.what();

					CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Func-VerifyToken", debug_msg);
				}
#endif	
				return{ 0,{ result[1],result[2] } };
			}
			//如果行数不为0,则存在
			//检查access token的有效性
			//否则使用refresh
			//.....是否强制RefreshToken
			if (focus_verify) {

				//暂时想不到哪里需要,先暂存
//				if (false) {
//					//==检查有效性
//					//xxx内容
//					bangumi::string content;
//					content << "access_token=" << result[1];
//
//					//post中的www编码其实是为了防止URI过长而将参数放在了content中,POST时一定不能忘记使用Content-Length指定大小
//					//否则会出现不能正常读取参数的问题
//					bangumi::string header("Cache-Control: no-cache\r\n"
//						"Content-Type: application/x-www-form-urlencoded\r\n");
//					header << "Content-Length: " << content.length() << "\r\n";
//
//					std::string request = "POST "  "/oauth/token_status"  " HTTP/1.1\r\n"
//						"Host: " "bgm.tv" "\r\n" + header + "\r\n" + content;
//					try {
//						std::string json = http_client.SyncBGMHTTPRequest(request);
//#ifndef NDEBUG
//						{
//							bangumi::string debug_msg;
//							debug_msg << "收到响应的Json"
//								>> json.c_str();
//							CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Func-VerifyToken", debug_msg);
//						}
//#endif
//						size_t status_bangumi_id = Resolve::Resolve_Auth_Status(json);
//					}
//					catch (boost::system::system_error&) {
//						//重新验证出现问题
//
//					}
//
//				}

				//一定会refresh
				size_t status_bangumi_id = 0;
				//判断BangumiID的一致性
				//try {
				std::string sql_bangumi_id = result[0];
				if (status_bangumi_id == 0) {
					//说明已经失效
					//重新Refresh
					try {
						auto auth = RefreshToken(result[2], param.qq);

						return{ sql_bangumi_id_num,{auth.access_token,auth.refresh_token } };
					}
					catch (boost::system::system_error&) {
						//重新验证失败
						return{ sql_bangumi_id_num,{ "","" } };
					}
				}
				if (std::to_string(status_bangumi_id) != sql_bangumi_id) {
					//说明两者不一致
					//提醒用户重新绑定
					throw boost::system::system_error(bangumi_bot_errors::auth_without_same_id);
				}
				//}
				//catch (std::invalid_argument) {
				//	//ID转换失败
				//	throw boost::system::system_error();
				//}
			}
			//此时说明ID与AccessToken一致
			//并且处于有效期
			//返回access_token
#ifndef NDEBUG
			{
				bangumi::string debug_msg;
				debug_msg << "Func VerifyAccessToken OK!"
					>> "user_id: " << sql_bangumi_id_num
					>> "access_token: " << result[1]
					>> "refresh_token: " << result[2];
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Func-VerifyToken", debug_msg);
			}
#endif	
			return{ sql_bangumi_id_num,{result[1],result[2] } };
			

			

		}
		else {
			//说明未注册用户
			//直接返回空字符串
			return{ 0,{"","" }};
		}

	}

	std::pair<bangumi::string,bangumi::BangumiUser> GetUserSubjectProgress(size_t subject_id, int eps, size_t user_id, int64_t qq,
		std::string access_token, std::string refresh_token, bool refresh, bool final_time = false) {

		bangumi::string uri;
		uri <<"/collection/"<< subject_id << "?access_token=" << access_token;

		std::string request = "GET " + uri + " HTTP/1.1\r\n"
			"Host: " "api.bgm.tv" "\r\n" "\r\n";
		try {
			std::string json = http_client.SyncBGMHTTPRequest(request);
#ifndef NDEBUG
		{
			bangumi::string debug_msg;
			debug_msg << "收到响应的Json"
				>> json.c_str();
			CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Func-GetUserSubjectProgress", debug_msg);
		}
#endif

			auto resolve_result = Resolve::Resolve_User_Process(json, refresh);

			//返回的变量
			bangumi::string msg;
			//加上Subject的总集数
			resolve_result.second.progress.AddEps(eps);

			msg = resolve_result.second.ProgressGet();

			//结束的等待
			for (auto &t : resolve_result.first) {
				if (t != nullptr&&t->joinable())
					t->join();
			}

			return{ std::move(msg), resolve_result.second };
		}
		catch (boost::system::system_error&e) {
			if (e.code() == boost::system::system_error(bangumi_bot_errors::user_not_collect_this_subject).code()) {
				//就是json内没有user_id信息,并且用户没有收藏这个subject
				//尝试从缓存中查找User 如果没有直接返回 类似<未收藏>的信息
				try {
					auto &bgm_user = BangumiPreFindUser(user_id);
					BangumiUserProgress user_progress;
					bgm_user.SetProgress(user_progress);
					//为user添加Progress
					return{ bgm_user.ProgressGet(),bgm_user };

				}
				catch (std::out_of_range) {
					//没有找到
					//直接返回
					return{ e.what(),bangumi::BangumiUser() };
				}
			}
			else if (e.code() == boost::system::system_error(bangumi_bot_errors::access_token_invalid).code()) {
				if (!final_time) {
					try {

						auto auth = RefreshToken(refresh_token, qq);
						return GetUserSubjectProgress(subject_id, eps, user_id, qq,
							auth.access_token, auth.refresh_token, refresh, true);
					}
					catch (boost::system::system_error&) {
						//直接返回
						return{ "",bangumi::BangumiUser() };
					}
				}
				else {
#ifndef NDEBUG
					{
						bangumi::string debug_msg;
						debug_msg << "用户授权刷新后仍有问题..."
							>> e.what();
						CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Func-GetUserSubjectProgress", debug_msg);
					}
#endif	
					return{ "\n\n授权发生了问题,请重新绑定Bangumi-ID...", bangumi::BangumiUser() };
				}
				
			}
			else{
#ifndef NDEBUG
				{
					bangumi::string debug_msg;
					debug_msg << "GetUserSubjectProgress 发生其他错误"
						>> e.what();
					CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Func-GetUserSubjectProgress", debug_msg);
				}
#endif	
				return{ "",bangumi::BangumiUser() };
			}
		}


	}

	//同步Http下载条目Html
	std::string GetSubjectHtml(size_t subject_id) {
		bangumi::string uri;
		uri << "/subject/" << subject_id ;

		std::string request = "GET " + uri + " HTTP/1.1\r\n"
			"Host: " "bgm.tv" "\r\n" "\r\n";
		try {
			//std::string html = boost::locale::conv::from_utf(http_client.SyncBGMHTTPRequest(request), "GBK");
			std::string html = code_converter.Conv(http_client.SyncBGMHTTPRequest(request));

			return std::move(html);
		}
		catch (boost::system::system_error&) {
			return "";
		}

	}
	std::string GetTimeLine() {
		std::string request = "GET " "/timeline"" HTTP/1.1\r\n"
			"Host: " "bgm.tv" "\r\n" "\r\n";
		try {
			//std::string html = boost::locale::conv::from_utf(http_client.SyncBGMHTTPRequest(request), "GBK");
			std::string html = code_converter.Conv(http_client.SyncBGMHTTPRequest(request));

			return std::move(html);
		}
		catch (boost::system::system_error&) {
			return "";
		}

	}
	std::string GetUserTimeLine(std::string uid) {
		std::string request = "GET " "/feed/user/"+ uid +"/timeline HTTP/1.1\r\n"
			"Host: " "bgm.tv" "\r\n" "\r\n";
		try {
			//std::string html = boost::locale::conv::from_utf(http_client.SyncBGMHTTPRequest(request), "GBK");
			std::string html = code_converter.Conv(http_client.SyncBGMHTTPRequest(request));

			return std::move(html);
		}
		catch (boost::system::system_error&) {
			return "";
		}

	}

	//同步Https下载一个HTML页面
	std::string GetHttpsHtml(std::string host,std::string uri) {
		//
		std::string request = "GET " + uri + " HTTP/1.1\r\n"
			"Host: " +host+ "\r\n" "\r\n";
		try {
			//std::string html = boost::locale::conv::from_utf(http_client.SyncBGMHTTPRequest(request), "GBK");
			std::string html = code_converter.Conv(http_client.SyncHTTPSRequest(host, request));

			return std::move(html);
		}
		catch (boost::system::system_error&) {
			return "";
		}

	}
	std::string GetBGMAPI(bangumi::string uri) {
		std::string request = "GET " + uri+" HTTP/1.1\r\n"
			"Host: " +bgm.bangumi_api_url +"\r\n" "\r\n";
		try {
			//std::string html = boost::locale::conv::from_utf(http_client.SyncBGMHTTPRequest(request), "GBK");
			std::string html = code_converter.Conv(http_client.SyncBGMHTTPRequest(request));

			return std::move(html);
		}
		catch (boost::system::system_error&) {
			return "";
		}

	}
	//用户收藏概括
	bangumi::string GetUserSumCollections(std::string bangumi_user) {
		//
		bangumi::string uri("/user/");
		uri << bangumi_user;
		uri << "/collections/status?app_id=" << bgm.bangumi_client_id;
		try {
			//进行请求
			auto& json = GetBGMAPI(uri);
			//进行解析
			return Resolve::Resolve_User_Collection_Sum(json);
		}
		catch (boost::system::system_error&) {
			return "收藏获取失败...";
		}
		catch (std::exception&) {
			return "收藏获取失败";
		}

	}
	//end of bangumi
}


namespace bangumi {
	//取得用户LastSubjectID
	//未注册或没有last返回0
	inline size_t GetLastSubjectID(int64_t qq) {
		//last subject id
		size_t subject_id = 0;
		//查询语句
		bangumi::string query;
		//SELECT * FROM bgm_subjects WHERE subject_id=1
		query << "SELECT user_last_searched FROM bgm_users WHERE user_qq="
			<< qq;
		//查询结果
		BGMSQLResult result;
		//影响的行数
		unsigned long affect_rows_num = sql_pool.ExecQuery(query, result);
		try
		{
			SQLCheckResult(affect_rows_num);
		}
		catch (boost::system::system_error& e)
		{
#ifndef NDEBUG
			{
				bangumi::string debug_msg;
				debug_msg << "GetLastSubjectID查询失败:"
					<< std::to_string(affect_rows_num)
					>> e.what();

				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-GetLastSubjectID", debug_msg);
			}
#endif	
			//通过异常结束函数
			throw e;
		}
		if (affect_rows_num > 0) {
			//行数大于0,有效注册用户

			try {
				subject_id = std::stoul(result[0]);
				//return{ sql_bangumi_id_num,result[1] };
			}
			catch (std::invalid_argument&e)
			{
#ifndef NDEBUG
				{
					bangumi::string debug_msg;
					debug_msg << "Func GetLastSubjectID stoul failed!"

						>> e.what();

					CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Func-GetLastSubjectID", debug_msg);
				}
#endif	
				
			}
		}
		//返回id
		return subject_id;
	}

	//解析*参数
	inline ComplexParam PARA_Resolve_One_Star(const std::string& str, BgmCode code) {
		return 999;
	}
	//解析**参数
	inline ComplexParam PARA_Resolve_Two_Star(const std::string& str, BgmCode code) {
		return 999;
	}
	//解析-参数
	inline ComplexParam PARA_Resolve_Line(const std::string& str, BgmCode code) {
		return 999;
	}
	//解析+参数
	inline ComplexParam PARA_Resolve_Plus(const std::string& str, BgmCode code) {
		//暂时使用一个数字ID+参数,以后可能增加复合解析使用
		//123456+tgc
		auto& npos = std::string::npos;
		//因为可能有+冲突只识别最后一个+号
		auto delim = str.find_last_of('+');
		if (delim == npos) {
			throw boost::system::system_error(bangumi_bot_errors::invalid_param);
		}
		//
		size_t id = 0;
		//构建结果对象
		//返回的结果
		ComplexParam ret;

		if (delim == 0) {
			//说明前面没有参数,使用last subject
			ret.use_last_subject_id = true;
		}
		else {
			//前面的参数
			try {
				id = std::stoul(str.substr(0, delim));
			}
			catch (std::invalid_argument&) {
				//使用了其他解析
				if (code == BgmCode::Subject) {
					//首先分割字符串 排除掉最后的+防止影响解析[决定使用了#就不能使用+trc,防止难以理解和复杂]
					//std::string pre_param = str.substr(0, delim);
					auto sharp_res = PARA_Resolve_Sharp(str, code);
					if (sharp_res.id != 0) {
						//说明正确解析了
						id = sharp_res.id;
						//为ID赋值[决定使用了#就不能使用+trc,防止难以理解和复杂]
						ret.id = id;
						//直接return 
						return ret;
					}
					else {
						//也许使用了%
						auto percent_res = PARA_Resolve_Percent(str,code);
						if (percent_res.id != 0) {
							//说明正确解析了
							id = percent_res.id;
							//因为percent会和+冲突,就不能使用+trc,防止难以理解和复杂
							ret.id = id;
							//直接return 
							return ret;
						}
					}
					//否则什么也不做
				}

			}
		}
		//为ID赋值
		ret.id = id;

		//后面的参数
		std::string param = str.substr(delim + 1, str.length() - delim - 1);


		//便捷宏
#define HAVE(c1)\
param.find_first_of(c1)!=npos
		//对应不同命令解析
		switch (code)
		{
		case BgmCode::Subject:
			//Subject命令时的处理
			if (HAVE('t')) {
				ret.add_tag = true;
			}
			if (HAVE('c')) {
				ret.add_comment = true;
			}
			if (HAVE('r')) {
				ret.add_role = true;
				//ret.only_role = (!ret.add_tag && !ret.add_comment);
			}
			if (HAVE('s')) {
				ret.add_air_status = true;
			}
			if (HAVE('o')) {
				ret.add_staff = true;
			}
			if (HAVE('a')) {
				ret.add_attach = true;
			}
			break;
		case BgmCode::Search:
			break;
		case BgmCode::User:
			break;
		case BgmCode::List:
			break;
		case BgmCode::BGM:
			break;
		case BgmCode::DMHY:
			break;
		case BgmCode::MOE:
			break;
		case BgmCode::Up:
			break;
		case BgmCode::Collect:
			break;
		case BgmCode::Conf:
			break;
		case BgmCode::Reg:
			break;
		case BgmCode::Unknow:
			break;
		default:
			break;
		}

		return ret;

	}
	//解析Search的前缀+-号
	inline std::string& PARA_Resolve_Search_KeyWord(std::string& pre_param) {
		//是一个Search的命令
		//加号可以不替换为空格,但减号前需要一个空格
		size_t sub_pos = 0;
		sub_pos = pre_param.find_first_of('-', sub_pos);
		while (sub_pos != std::string::npos) {
			//URL中可以用+表示空格
			pre_param.insert(sub_pos, "+");
			//继续搜索下一个位置
			sub_pos = pre_param.find_first_of('-', sub_pos + 2);
		}
		//最后返回结果
		return pre_param;
	}
	//解析/参数
	inline ComplexParam PARA_Resolve_Virgule(const std::string& str, BgmCode code) {
		//暂时使用一个数字ID/参数,以后可能增加复合解析使用
		//123456/tgc
		auto& npos = std::string::npos;
		auto delim = str.find_first_of('/');
		if (delim == npos) {
			throw boost::system::system_error(bangumi_bot_errors::invalid_param);
		}
		//
		size_t id = 0;
		//构建结果对象
		//返回的结果
		ComplexParam ret;

		if (delim == 0) {
			//说明前面没有参数,使用last subject
			ret.use_last_subject_id = true;
		}
		else {
			//前面的参数
			std::string pre_param = str.substr(0, delim);
			try {
				if (code != BgmCode::Search&&code != BgmCode::Tag &&code != BgmCode::BGM)
					id = std::stoul(pre_param);
				else
					throw std::invalid_argument("no need int");
			}
			catch (std::invalid_argument&) {
				//TODO:使用了其他解析或原生的String类型
				//SearchAPI
				if (code == BgmCode::Search) {
					//如果是一个Search的命令
					PARA_Resolve_Search_KeyWord(pre_param);
					ret.str = pre_param;
				}
				else 
				//UserCollection
				if (code == BgmCode::BGM) {
					//如果是一个User Collection的命令
					ret.bangumi_user = pre_param;
				}else
				
				//SubjectAPI
				if (code == BgmCode::Subject||code == BgmCode::Collect||code == BgmCode::Up) {
					//使用了其他解析
					//使用分割的字符串 排除掉最后的+防止影响解析
					//Subject可以使用/因为不与+,-冲突
					//Collect也可以,前缀只能是一个#
					auto sharp_res = PARA_Resolve_Sharp(pre_param, code);
					if (sharp_res.id != 0) {
						//说明正确解析了
						id = sharp_res.id;
					}
					else {
						//尝试使用%解析
						auto percent_res = PARA_Resolve_Percent(pre_param, code);
						if (percent_res.id != 0) {
							//说明正确解析了
							id = percent_res.id;
							//因为percent不会和/冲突,因此继续前进
						}
					}
					//否则什么也不做
				}
				else
				//Tag API
				if (code == BgmCode::Tag) {
					ret.tag_keyword = pre_param;
				}
				else
				//Rss API
				if (code == BgmCode::RSS) {
					//
					auto sharp_res = PARA_Resolve_Sharp(pre_param, code);
					if (sharp_res.id != 0) {
						//说明正确解析了
						id = sharp_res.id;
					}
					else {
						//尝试使用%解析
						auto percent_res = PARA_Resolve_Percent(pre_param, code);
						if (percent_res.id != 0) {
							//说明正确解析了
							id = percent_res.id;
							//因为percent不会和/冲突,因此继续前进
						}
						else {
							//否则直接给予赋值
							////首先替换+至空格//无需多此一举
							//auto replace_plus_pos = pre_param.find_last_of('+');
							////复制str变量
							//std::string after_replace_str = pre_param;
							//while (replace_plus_pos != std::string::npos) {
							//	after_replace_str[replace_plus_pos] = ' ';
							//	replace_plus_pos = after_replace_str.find_last_of('+', replace_plus_pos);
							//}
							ret.rss_keyword = pre_param;
						}
					}
					

				}
				
			}
		}

		//为ID赋值
		ret.id = id;


		//后面的参数
		std::string param = str.substr(delim + 1, str.length() - delim - 1);
		std::string orign_param(param);
		//将参数转换为小写
		std::transform(param.begin(), param.end(), param.begin(), ::tolower);
		//便捷宏
#define HAVE(c1)\
param.find_first_of(c1)!=npos
#define EXHAVE(first,c1,c2)\
first.find_first_of(c1)!=npos||first.find_first_of(c2)!=npos
#define STRHAVE(s1,s2)\
param.find(s1)!=npos||param.find(s2)!=npos
#define EXSTRHAVE(sss,s1,s2)\
sss.find(s1)!=npos||sss.find(s2)!=npos
		//对应不同命令解析
		switch (code)
		{
		case BgmCode::Subject:
			//Subject命令时的处理
			//例如: 123456/trc(三个参数都加上 标签,角色,吐槽)
			//默认不是附加构上
			ret.single = true;
			if (HAVE('t')) {
				ret.add_tag = true;
			}
			if (HAVE('c')) {
				ret.add_comment = true;
			}
			if (HAVE('r')) {
				ret.add_role = true;
				//ret.only_role = (!ret.add_tag && !ret.add_comment);
			}
			if (HAVE('s')) {
				ret.add_air_status = true;
			}
			if (HAVE('o')) {
				ret.add_staff = true;
			}
			if (HAVE('a')) {
				ret.add_attach = true;
			}
			break;
		case BgmCode::Search:
		{
			//Search命令时的处理
			//例如: Angel+Beats-ova(+表示空格-表示排除)/2(类型)/5(数量)/0(从第几个开始)
			//首先是类型
			//类型和数量之间的/
			auto delim1 = param.find_first_of('/');
			//数量和起始位置之间的/
			auto delim2 = param.find_first_of('/', delim1+1);
			//第一个参数
			std::string first_param;
			if (delim1 != npos)
				first_param = param.substr(0, delim1);
			else
				first_param = param;
			//从第一个参数中查找
			if (!first_param.empty()) {
				if (EXHAVE(first_param, 'a', '2')) {
					//动画
					ret.search_type = 2;
				}
				else if (EXHAVE(first_param, 'g', '4')) {
					//游戏
					ret.search_type = 4;
				}
				else if (EXHAVE(first_param, 'c', '1')) {
					//书
					ret.search_type = 1;
				}
				else if (EXHAVE(first_param, 'x', '0')) {
					//全部的分类
					ret.search_type = 0;
				}
				else if (EXHAVE(first_param, 'r', '6')) {
					//三次元
					ret.search_type = 6;
				}
				else if (EXHAVE(first_param, 'm', '3')) {
					//音乐
					ret.search_type = 3;
				}
			}
			//第二个参数
			std::string second_param;
			if (delim1 != npos)
				second_param = param.substr(delim1+1);
			//只接受数字字符,并且如果有的话,否则默认
			if (!second_param.empty() && delim1 + 1 < param.length()) {
				try {
					int temp_num;
					temp_num = std::stoi(second_param);
					ret.search_max_num = temp_num;
				}
				catch (std::invalid_argument&) {
					//直接默认参数
				}
			}
			//第三个参数
			std::string third_param;
			if (delim2 != npos)
				third_param = param.substr(delim2+1);
			//只接受数字字符,并且如果有的话,否则默认
			if (!third_param.empty() && delim2 + 1 < param.length()) {
				try {
					int temp_num;
					temp_num = std::stoi(third_param);
					ret.search_start_pos = temp_num;
				}
				catch (std::invalid_argument&) {
					//直接默认参数
				}
			}
		}

			break;
		case BgmCode::User:
			break;
		case BgmCode::List:
			break;
		case BgmCode::BGM:
		{
			//分隔符
			size_t delim;
			size_t str_start = 0;
			//参数数组
			std::string para[5];
			int para_num = -1;
			//第一个连接符
			delim = param.find_first_of('/');
			while (delim != npos)
			{
				//参数
				para[++para_num] = param.substr(str_start, delim - str_start);
				str_start = delim + 1;
				delim = param.find_first_of('/', delim + 1);
			}
			//存入最后一个参数
			para[++para_num] = param.substr(str_start);

			//处理识别出的参数
			for (int i = 0;i<5;++i)
			{
				if (para[i].empty())
				{
					continue;
				}
				else {
					auto& s = para[i];
					switch (i)
					{
					case 0:
					{
						//条目类型
						if (EXHAVE(s, 'a', '2')) {
							//动画
							ret.ucollection_subject_type = "anime";
						}
						else if (EXHAVE(s, 'g', '4')) {
							//游戏
							ret.ucollection_subject_type = "game";
						}
						else if (EXHAVE(s, 'c', '1')) {
							//书
							ret.ucollection_subject_type = "book";
						}
						else if (EXHAVE(s, 'r', '6')) {
							//三次元
							ret.ucollection_subject_type = "real";
						}
						else if (EXHAVE(s, 'm', '3')) {
							//音乐
							ret.ucollection_subject_type = "music";
						}
					}
						break;
					case 1:
					{
						//收藏状态
						if (EXSTRHAVE(s,"collect", "fin")) {
							ret.ucollection_co_type = "collect";
						}
						else if (EXSTRHAVE(s,"wish", "td")) {
							ret.ucollection_co_type = "wish";
						}
						else if (EXSTRHAVE(s,"do", "on")) {
							ret.ucollection_co_type = "do";
						}
						else if (s.find("hold") != npos) {
							ret.ucollection_co_type = "on_hold";
						}
						else if (s.find("drop") != npos) {
							ret.ucollection_co_type = "dropped";
						}
					}
						break;
					case 2:
					{
						//页码
						try {
							ret.ucollection_page = std::stoi(s);
						}
						catch (std::exception&) {
						}
					}
						break;
					case 3:
					{
						//排序方式
						if (s.find("date") != npos) {
							ret.ucollection_order_type = "date";
						}
						else if (s.find("rate") != npos) {
							ret.ucollection_order_type = "rate";
						}
						else if (s.find("title") != npos) {
							ret.ucollection_order_type = "title";
						}
					}
						break;
					case 4:
					{
						//标签
						ret.ucollection_tag = s;
					}
						break;
					default:
						break;
					}
				}
			}


		}
			break;
		case BgmCode::DMHY:
			break;
		case BgmCode::MOE:
			break;
		case BgmCode::RSS:
		{
			//话数与最大个数的连接符
			auto delim = param.find_first_of('/');
			//第一个参数
			std::string first_param;
			if (delim != npos)
				first_param = param.substr(0, delim);
			else
				first_param = param;
			//第二个参数(只取最大个数)
			std::string second_param;
			if (delim != npos)
				second_param = param.substr(delim + 1);
			//处理搜索的话数
			if (!first_param.empty()) {
				//=====原Update中的识别=====	
				size_t plus_pos = first_param.find_first_of('+');
				size_t sub_pos = first_param.find_first_of('-');
				//是否是+号 否则-号
				bool isPlus = false;
				//首个可识别符号的位置
				size_t end_pos = sub_pos;
				//判断
				if (plus_pos < sub_pos) {
					end_pos = plus_pos;
					isPlus = true;
				}
				//不包括符号的字符串
				std::string param_without_sign;
				if (end_pos != std::string::npos) {
					param_without_sign = first_param.substr(0, end_pos);
				}
				else {
					param_without_sign = first_param;
				}
				//对符号的处理,如果有
				if (end_pos != std::string::npos) {
					try {

						ret.update_eps_shift = std::stoi(first_param.substr(end_pos + 1));
						if (!isPlus) {
							//减的偏移
							ret.update_eps_shift = -ret.update_eps_shift;
						}

					}
					catch (std::exception&) {
						//转换失败,直接继续
						//也可能是内存访问冲突,越界
					}
				}
				//对无符号的str处理
				if (param_without_sign.find("air") != npos) {
					ret.update_air = true;
				}
				else if (param_without_sign.find("fin") != npos) {
					ret.update_fin = true;
				}
				else {
					//可能是数字
					try {
						ret.update_watched_eps = std::stoi(param_without_sign);
					}
					catch (std::invalid_argument&) {
						//否则设置什么也不干:这里是默认不加话数
						//ret.update_watched_eps = 1;
					}

				}
				//=====原Update中的识别 Over=====
			}
			//处理最大的个数
			if (!second_param.empty()) {
				try {
					ret.rss_max_items = std::stoi(second_param);
				}
				catch (std::invalid_argument&) {
					//不做什么
				}
			}
		}
			break;
		case BgmCode::Up:
		{
			//例如: #Angel/2(更新到两集)
			//例如: #Angel/+2(向后更新两集进度)
			//例如: #Angel+1/+2(SQL中第二个匹配结果)
			//例如: #Angel+1/air(更新到已放送的最新的一集)
			//例如: #Angel+1/air-1(更新到已放送的最新的前一集>0)
			//例如: #Angel+1/fin(更新到最后一集,同时自动标记为collect状态)	
			//例如: #Angel+1/fin/8/吐槽(同时评分+吐槽)	
			//size_t plus_pos = param.find_first_of('+');
			//size_t sub_pos = param.find_first_of('-');
			////是否是+号 否则-号
			//bool isPlus = false;
			////首个可识别符号的位置
			//size_t end_pos = sub_pos;
			////判断
			//if (plus_pos < sub_pos) {
			//	end_pos = plus_pos;
			//	isPlus = true;
			//}
			////不包括符号的字符串
			//std::string param_without_sign;
			//if (end_pos != std::string::npos) {
			//	param_without_sign = param.substr(0, end_pos);
			//}
			//else {
			//	param_without_sign = param;
			//}
			////对符号的处理,如果有
			//if (end_pos != std::string::npos){
			//	try {

			//		ret.update_eps_shift = std::stoi(param.substr(end_pos + 1));
			//		if (!isPlus) {
			//			//减的偏移
			//			ret.update_eps_shift = -ret.update_eps_shift;
			//		}

			//	}
			//	catch (std::exception&) {
			//		//转换失败,直接继续
			//		//也可能是内存访问冲突,越界
			//	}
			//}
			////对无符号的str处理
			//if (param_without_sign.find("air")!=npos) {
			//	ret.update_air = true;
			//}
			//else if (param_without_sign.find("fin")!=npos) {
			//	ret.update_fin = true;
			//}
			//else {
			//	//可能是数字
			//	try {
			//		ret.update_watched_eps = std::stoi(param_without_sign);
			//	}
			//	catch (std::invalid_argument&) {
			//		//否则设置什么也不干:由用户自己的进度决定
			//		//ret.update_watched_eps = 1;
			//	}
			//	
			//}
			
			//
			//多参数
			//更新的话数和评分之间的/
			auto delim1 = param.find_first_of('/');
			//评分和评论位置之间的/
			auto delim2 = param.find_first_of('/', delim1 + 1);
			//第一个参数
			std::string first_param;
			if (delim1 != npos)
				first_param = param.substr(0, delim1);
			else
				first_param = param;
			//从第一个参数中查找
			if (!first_param.empty()) {
				
				//=====原Update中的识别=====	
				size_t plus_pos = first_param.find_first_of('+');
				size_t sub_pos = first_param.find_first_of('-');
				//是否是+号 否则-号
				bool isPlus = false;
				//首个可识别符号的位置
				size_t end_pos = sub_pos;
				//判断
				if (plus_pos < sub_pos) {
					end_pos = plus_pos;
					isPlus = true;
				}
				//不包括符号的字符串
				std::string param_without_sign;
				if (end_pos != std::string::npos) {
					param_without_sign = first_param.substr(0, end_pos);
				}
				else {
					param_without_sign = first_param;
				}
				//对符号的处理,如果有
				if (end_pos != std::string::npos) {
					try {

						ret.update_eps_shift = std::stoi(first_param.substr(end_pos + 1));
						if (!isPlus) {
							//减的偏移
							ret.update_eps_shift = -ret.update_eps_shift;
						}

					}
					catch (std::exception&) {
						//转换失败,直接继续
						//也可能是内存访问冲突,越界
					}
				}
				//对无符号的str处理
				if (param_without_sign.find("air") != npos) {
					ret.update_air = true;
				}
				else if (param_without_sign.find("fin") != npos) {
					ret.update_fin = true;
				}
				else {
					//可能是数字
					try {
						ret.update_watched_eps = std::stoi(param_without_sign);
					}
					catch (std::invalid_argument&) {
						//否则设置什么也不干:由用户自己的进度决定
						//ret.update_watched_eps = 1;
					}

				}
				//=====原Update中的识别 Over=====
			}
			//第二个参数
			std::string second_param;
			if (delim1 != npos)
				second_param = param.substr(delim1 + 1);
			//只接受数字字符,并且如果有的话,否则默认
			if (!second_param.empty() && delim1 + 1 < param.length()) {
				try {
					int temp_num;
					temp_num = std::stoi(second_param);

					//限定范围
					if (temp_num < 0) {
						temp_num = 0;
					}
					else if (temp_num > 10) {
						temp_num = 10;
					}

					//赋值
					ret.collection_rating = temp_num;
				}
				catch (std::invalid_argument&) {
					//直接默认参数
				}
			}
			//第三个参数
			std::string third_param;
			if (delim2 != npos)
				third_param = orign_param.substr(delim2 + 1);
			//字符串,并且如果有的话,否则默认
			if (!third_param.empty() && delim2 + 1 < param.length()) {
				ret.collection_comment = third_param;
			}
		}
			break;
		case BgmCode::Collect:
		{
			//例如: 123456(默认更新为在看)
			//例如: 1234/do(更新为在看)
			//例如: 1234/fin(更新为看完)
			//例如: 1234/fin/8/还不错
			//收藏状态
			//	1 = wish = 想做
			//	2 = collect = 做过
			//	3 = do = 在做
			//	4 = on_hold = 搁置
			//	5 = dropped = 抛弃
			//if (STRHAVE("collect","fin")) {
			//	ret.collection_status = "collect";
			//}
			//else if (STRHAVE("wish","to")) {
			//	ret.collection_status = "wish";
			//}
			//else if (STRHAVE("do","on")) {
			//	ret.collection_status = "do";
			//}
			//else if (param.find("hold")!=npos) {
			//	ret.collection_status = "on_hold";
			//}
			//else if (param.find("drop")!=npos) {
			//	ret.collection_status = "dropped";
			//}

			//多参数
			//状态和评分之间的/
			auto delim1 = param.find_first_of('/');
			//评分和评论位置之间的/
			auto delim2 = param.find_first_of('/', delim1 + 1);
			//第一个参数
			std::string first_param;
			if (delim1 != npos)
				first_param = param.substr(0, delim1);
			else
				first_param = param;
			//从第一个参数中查找
			if (!first_param.empty()) {
				if (STRHAVE("collect", "fin")) {
					ret.collection_status = "collect";
				}
				else if (STRHAVE("wish", "td")) {
					ret.collection_status = "wish";
				}
				else if (STRHAVE("do", "on")) {
					ret.collection_status = "do";
				}
				else if (param.find("hold") != npos) {
					ret.collection_status = "on_hold";
				}
				else if (param.find("drop") != npos) {
					ret.collection_status = "dropped";
				}
			}
			//第二个参数
			std::string second_param;
			if (delim1 != npos)
				second_param = param.substr(delim1 + 1);
			//只接受数字字符,并且如果有的话,否则默认
			if (!second_param.empty() && delim1 + 1 < param.length()) {
				try {
					int temp_num;
					temp_num = std::stoi(second_param);
					
					//限定范围
					if (temp_num < 0) {
						temp_num = 0;
					}
					else if (temp_num > 10) {
						temp_num = 10;
					}

					//赋值
					ret.collection_rating = temp_num;
				}
				catch (std::invalid_argument&) {
					//直接默认参数
				}
			}
			//第三个参数
			std::string third_param;
			if (delim2 != npos)
				third_param = orign_param.substr(delim2 + 1);
			//字符串,并且如果有的话,否则默认
			if (!third_param.empty() && delim2 + 1 < param.length()) {
				ret.collection_comment = third_param;
			}
			//评论和标签位置之间的/
			auto delim3 = third_param.find_first_of('/');
			//第四个参数
			std::string fourth_param;
			if (delim3 != npos) {
				ret.collection_comment = third_param.substr(0, delim3);
				fourth_param = third_param.substr(delim3 + 1);
				ret.collection_tags = fourth_param;
			}

		}
			break;
		case BgmCode::Conf:
			break;
		case BgmCode::Reg:
			break;
		case BgmCode::Tag:
		{
			//airtime与page的连接符
			auto delim = param.find_first_of('/');
			//第一个参数
			std::string first_param;
			if (delim != npos)
				first_param = param.substr(0, delim);
			else
				first_param = param;
			//第二个参数(只取页码)
			std::string second_param;
			if (delim != npos)
				second_param = param.substr(delim + 1);
			//处理
			ret.tag_airtime = first_param;
			if (!second_param.empty()) {
				try {
					ret.tag_page = std::stoi(second_param);
				}
				catch (std::invalid_argument&) {
					//不做什么
				}
			}

		}
			break;
		case BgmCode::Unknow:
			break;
		default:
			break;
		}

		return ret;
	}
	//解析#参数
	inline ComplexParam PARA_Resolve_Sharp(const std::string& str, BgmCode code) {
		if (code == BgmCode::Up || code == BgmCode::Subject || code == BgmCode::Collect ||code == BgmCode::RSS) {
			//SELECT * FROM bgm_subjects WHERE CONCAT(name_cn,name) LIKE CONCAT('%ク%') ORDER BY subject_id DESC
			//类似于这样的SQL搜索
			//单线程SQL
			size_t sharp_pos = str.find_first_of('#');
			if (sharp_pos!=std::string::npos){
				//返回的subject id
				size_t subject_id = 0;
				//只识别#之后的最后一个-号
				size_t sub_pos = str.find_last_of('-');
				//默认是返回第一行的结果
				unsigned wanted_num = 0;
				if (sub_pos != std::string::npos) {
					//说明有指定了第几个结果
					//但如果越界就给出最后一行结果的id
					try {
						std::string temp_stoi = str.substr(sub_pos + 1);
						wanted_num = std::stoi(temp_stoi);
					}
					catch (std::invalid_argument&) {
						//说明无法转换
						//处理: 将+号转换为%

						//同步转换时也匹配+号
					}
				}
				//首先将其从gb18030转换到utf-8,数据库内的编码为UTF8
				bangumi::string search_key;
				//到现在为止所有的+号都不是一个特殊的,只是代表%
				//进行+->%的替换
				auto replace_plus_pos = str.find_last_of('+');
				//复制str变量
				std::string after_replace_str = str;
				while (replace_plus_pos != std::string::npos) {
					after_replace_str[replace_plus_pos] = '%';
					replace_plus_pos = after_replace_str.find_last_of('+', replace_plus_pos);
				}
				//判断是否有+号
				if (wanted_num==0){
					search_key << code_encoder.Conv(after_replace_str.substr(sharp_pos + 1));
				} 
				else{
					search_key << code_encoder.Conv(after_replace_str.substr(sharp_pos + 1, sub_pos - sharp_pos -1));
				}

#ifndef NDEBUG
				{
					bangumi::string debug_msg;
					debug_msg << "#解析中:"
						>> "search_key: " << search_key
						>> "wanted_num: " << wanted_num;
					CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-PARA_Resolve_Sharp", debug_msg);
				}
#endif	

				bangumi::string query;
				//优先排序动画类型,之后是ID降序
				query << "SELECT subject_id FROM bgm_subjects "
					<< "WHERE CONCAT(name_cn,name) "
					<< "LIKE '%" << search_key << "%' "
					<< "ORDER BY FIELD(type,2,4,6,1,3), "
					<< "subject_id DESC";
				//查询结果
				BGMSQLResult result;
#ifndef NDEBUG
				{
					bangumi::string debug_msg;
					debug_msg << "#解析语句:"
						<< query;
					CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-PARA_Resolve_Sharp", debug_msg);
				}
#endif	
				//影响的行数
				unsigned long affect_rows_num = sql_pool.ExecQuery(query, result);
				try
				{
					SQLCheckResult(affect_rows_num);
				}
				catch (boost::system::system_error& e)
				{
#ifndef NDEBUG
					{
						bangumi::string debug_msg;
						debug_msg << "#解析查询失败:"
							<< std::to_string(affect_rows_num)
							>> e.what();

						CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-PARA_Resolve_Sharp", debug_msg);
					}
#endif	
					//通过异常结束函数
					throw e;
				}
				if (affect_rows_num > 0) {
#ifndef NDEBUG
					{
						bangumi::string debug_msg;
						debug_msg << "#解析查询结果:"
							<< std::to_string(affect_rows_num);

						CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-PARA_Resolve_Sharp", debug_msg);
					}
#endif	
					if (wanted_num != 0) {
						//如果有这个参数重新映射1->0,2->1
						--wanted_num;
					}
					//行数大于0,说明有匹配项
					if (wanted_num > affect_rows_num - 1) {
						//如果need的大于SQL中有的记录数
						//直接将其赋值
						wanted_num = affect_rows_num - 1;
					}
					try {
						//注意fetchrows函数一次只取一行,因此需要变更到想要的行
						result.FetchRow(wanted_num);
						//尝试赋值,一定正确
						subject_id = std::stoul(result[0]);
						
						
					}
					catch (std::invalid_argument&e)
					{
#ifndef NDEBUG
						{
							bangumi::string debug_msg;
							debug_msg << "Func PARA_Resolve_Sharp stoul failed!"

								>> e.what();

							CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Func-PARA_Resolve_Sharp", debug_msg);
						}
#endif	

					}
					//返回subject_id
					return subject_id;
				}
				else {
					//说明没有匹配项
					//判断是否是0条目
					if (subject_id == 0) {
						//使用%解析
						std::string temp_str = str;
						temp_str[sharp_pos] = '%';
						return bangumi::PARA_Resolve_Percent(temp_str, code);
					}
					//返回0
					return 0;
				}
			}
			else {
				//不包含一个#
				//直接返回0
				return 0;
			}
		}
		else {
			//不支持此命令该参数解析
			return 0;
		}
	}
	//解析%参数
	inline ComplexParam PARA_Resolve_Percent(const std::string& str, BgmCode code) {
		if (code == BgmCode::Up || code == BgmCode::Subject || code == BgmCode::Collect||code == BgmCode::RSS) {
			size_t percent_pos = str.find_first_of('%');
			//如果没有找到%直接返回0
			if (percent_pos == std::string::npos) {
				return 0;
			}
			size_t star_pos = str.find_first_of('*');
			//创建一个临时变量用于调用函数解析+-
			std::string pre_param;
			if (star_pos!=std::string::npos){
				pre_param = str.substr(percent_pos + 1, star_pos - percent_pos - 1);
			}
			else {
				pre_param = str.substr(percent_pos + 1);
			}
			//构造一个复杂参数对象
			bangumi::ComplexParam complex_param;
			//设定默认的Search参数
			complex_param.search_max_num = 2;
			complex_param.search_start_pos = 0;
			complex_param.search_type = 2;
			//如果存在/则赋值
			//对其它参数进行解析
			
			if(star_pos!=std::string::npos)
			{
				auto &npos = std::string::npos;
				std::string param = str.substr(star_pos + 1);
				//Search命令时的处理
				//例如: Angel+Beats-ova(+表示空格-表示排除)/2(类型)/5(数量)/0(从第几个开始)
				//首先是类型
				//类型和数量之间的/
				auto delim1 = param.find_first_of('*');
				//第一个参数
				std::string first_param;
				if (delim1 != npos)
					first_param = param.substr(0, delim1);
				else
					first_param = param;
				//从第一个参数中查找
				if (!first_param.empty()) {
					if (EXHAVE(first_param, 'a', '2')) {
						//动画
						complex_param.search_type = 2;
					}
					else if (EXHAVE(first_param, 'g', '4')) {
						//游戏
						complex_param.search_type = 4;
					}
					else if (EXHAVE(first_param, 'c', '1')) {
						//书
						complex_param.search_type = 1;
					}
					else if (EXHAVE(first_param, 'x', '1')) {
						//全部的分类
						complex_param.search_type = 0;
					}
					else if (EXHAVE(first_param, 'r', '6')) {
						//三次元
						complex_param.search_type = 6;
					}
					else if (EXHAVE(first_param, 'm', '3')) {
						//音乐
						complex_param.search_type = 3;
					}
				}
				//第二个参数
				std::string second_param;
				if (delim1 != npos)
					second_param = param.substr(delim1 + 1);
				//只接受数字字符,并且如果有的话,否则默认
				if (!second_param.empty() && delim1 + 1 < param.length()) {
					try {
						int temp_num;
						temp_num = std::stoi(second_param);
						complex_param.search_start_pos = temp_num;
					}
					catch (std::invalid_argument&) {
						//直接默认参数
					}
				}

			}

			//搜索的关键字
			std::string keyword = PARA_Resolve_Search_KeyWord(pre_param);
			

			//使用BGM API Search
			//这里只能单线程请求
			try {

				//根据复杂参数结构体增加参数
				bangumi::string extra_search_param;
				extra_search_param << "?type=" << complex_param.search_type
					<< "&responseGroup=large"
					<< "&start=" << complex_param.search_start_pos
					<< "&max_results=" << complex_param.search_max_num;
				bangumi::string uri;
				uri = "/search/subject/" + code_encoder.Conv(keyword) + extra_search_param;

				bangumi::string request;
				request << "GET " << uri << " HTTP/1.1\r\n"
					<< "Host: api.bgm.tv\r\n"
					<< "Cookie: chii_searchDateLine=0;\r\n\r\n";
#ifndef NDEBUG
				{
					bangumi::string debug_msg;
					debug_msg << "请求的Request"
						>> request;
					CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTP", debug_msg);
				}
#endif
				try {
					//单线程请求
					std::string json = http_client.SyncBGMHTTPRequest(request);
					//解析
				
#ifndef NDEBUG
					{
						bangumi::string debug_msg;
						debug_msg << "收到响应的Json"
							>> json.c_str();
						CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTP", debug_msg);
					}
#endif
					//这里默认不使用缓存
					auto resolve_res = Resolve::Resolve_Search_Singel(json,true);
					std::vector<std::shared_ptr<boost::thread>> pic_threads;

					//图片的线程
					pic_threads = resolve_res.first;
					//传递subject id, 经过了ResolveSearch已经加入了Subject池中
					complex_param.id = resolve_res.second.subject_id;

					//结束的等待
					for (auto &t : pic_threads) {
						if (t != nullptr&&t->joinable())
							t->join();
					}

					return complex_param;

				}
				catch (boost::system::system_error & e) {
					//如果发生了异常,使用异常回复(包括没有找到用户等)
					//这里不使用Reply类而是直接赋值回复
#ifndef NDEBUG
					{
						bangumi::string debug_msg;
						debug_msg << "解析%参数发生异常"
							>> e.what();
						CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTP", debug_msg);
					}
#endif
					return complex_param;
				}


			}
			catch (boost::system::system_error& e) {
#ifndef NDEBUG
				{
					bangumi::string debug_msg;
					debug_msg << "FUNC-API-PARA_Resolve_Percent Request Error"
						>> e.what();
					CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTP-API-PARA_Resolve_Percent", debug_msg);
				}
#endif
				return complex_param;
			}

			return complex_param;
		}
		else {
			//不支持此命令该参数解析
			return 0;
		}
	}

	//用于解析Subject API的参数封装
	inline ComplexParam ResolveSubjectPara(const std::string& str) {
		BgmCode code = BgmCode::Subject;
		try {
			//优先/ 
			return PARA_Resolve_Virgule(str, code);
		}
		catch (boost::system::system_error&) {
#ifndef NDEBUG
			{
				bangumi::string debug_msg;
				debug_msg << "PARA_Resolve_Plus 失败";
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-ResolveSubjectPara-Plus", debug_msg);
			}
#endif		
		}

		try {
			//之后是+
			return PARA_Resolve_Plus(str, code);
		}
		catch (boost::system::system_error&) {
#ifndef NDEBUG
			{
				bangumi::string debug_msg;
				debug_msg << "PARA_Resolve_Virgule 失败";
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-ResolveSubjectPara-Virgule", debug_msg);
			}
#endif		
			//都不能解析
			//最后直接抛出异常 不解析此参数
			//throw e;
			//最后交给#和%
			auto sharp_res = PARA_Resolve_Sharp(str, code);
			if (sharp_res.id != 0) {
				return sharp_res;
			}
			else {
				auto percent_res = PARA_Resolve_Percent(str, code);
				return percent_res;
			}
			
		}

	}

	//用于解析Search API的参数封装
	inline ComplexParam ResolveSearchPara(const std::string& str) {
		BgmCode code = BgmCode::Search;
		try {
			//只有'/'的解析
			return PARA_Resolve_Virgule(str, code);
		}
		catch (boost::system::system_error&) {
#ifndef NDEBUG
			{
				bangumi::string debug_msg;
				debug_msg << "PARA_Resolve_Virgule 失败";
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-ResolveSubjectPara-Virgule", debug_msg);
			}
#endif		
			//构建结果对象
			//返回的结果
			ComplexParam ret;
			std::string pre_param(str);
			ret.str = PARA_Resolve_Search_KeyWord(pre_param);
			return ret;
			//都不能解析
			//最后直接抛出异常 不解析此参数
			//throw e;
		}

	}

	//用于解析Collection API的参数封装
	inline ComplexParam ResolveCollectPara(const std::string& str) {
		BgmCode code = BgmCode::Collect;
		try {
			//只有'/'的解析,解析/do等
			return PARA_Resolve_Virgule(str, code);
		}
		catch (boost::system::system_error&) {
#ifndef NDEBUG
			{
				bangumi::string debug_msg;
				debug_msg << "PARA_Resolve_Virgule 失败";
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-ResolveCollectPara-Virgule", debug_msg);
			}
#endif		
			//构建结果对象
			//返回的结果
			//ComplexParam ret;
			//返回使用#解析
			//return PARA_Resolve_Sharp(str, code);
			//最后交给#和%
			auto sharp_res = PARA_Resolve_Sharp(str, code);
			if (sharp_res.id != 0) {
				return sharp_res;
			}
			else {
				auto percent_res = PARA_Resolve_Percent(str, code);
				return percent_res;
			}
			//return ret;
			//都不能解析
			//最后直接抛出异常 不解析此参数
			//throw e;
		}

	}

	//用于解析Update API的参数封装
	inline ComplexParam ResolveUpdatePara(const std::string& str) {
		BgmCode code = BgmCode::Up;
		try {
			//只有'/'的解析
			return PARA_Resolve_Virgule(str, code);
		}
		catch (boost::system::system_error&) {
#ifndef NDEBUG
			{
				bangumi::string debug_msg;
				debug_msg << "PARA_Resolve_Virgule 失败";
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-ResolveUpdatePara-Virgule", debug_msg);
			}
#endif		
			//构建结果对象
			//返回的结果
			//返回使用#解析
			//return PARA_Resolve_Sharp(str, code);
			//最后交给#和%
			auto sharp_res = PARA_Resolve_Sharp(str, code);
			if (sharp_res.id != 0) {
				return sharp_res;
			}
			else {
				auto percent_res = PARA_Resolve_Percent(str, code);
				return percent_res;
			}
			//都不能解析
			//最后直接抛出异常 不解析此参数
			//throw e;
		}

	}

	//用于解析Tag API的参数封装
	inline ComplexParam ResolveTagPara(const std::string& str) {
		BgmCode code = BgmCode::Tag;
		try {
			//只有'/'的解析
			return PARA_Resolve_Virgule(str, code);
		}
		catch (boost::system::system_error&e) {
#ifndef NDEBUG
			{
				bangumi::string debug_msg;
				debug_msg << "PARA_Resolve_Virgule 失败";
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-ResolveTagPara-Virgule", debug_msg);
			}
#endif		
			//都不能解析
			//最后直接抛出异常 不解析此参数
			throw e;
		}
	}

	//用于解析RSS的参数封装
	inline ComplexParam ResolveRSSPara(const std::string& str) {
		BgmCode code = BgmCode::RSS;
		try {
			//只有'/'的解析
			return PARA_Resolve_Virgule(str, code);
		}
		catch (boost::system::system_error&) {
#ifndef NDEBUG
			{
				bangumi::string debug_msg;
				debug_msg << "PARA_Resolve_Virgule 失败";
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-ResolveRSSPara-Virgule", debug_msg);
			}
#endif		
			//构建结果对象
			//返回的结果
			//返回使用#解析
			//return PARA_Resolve_Sharp(str, code);
			//最后交给#和%
			auto sharp_res = PARA_Resolve_Sharp(str, code);
			if (sharp_res.id != 0) {
				return sharp_res;
			}
			else {
				auto percent_res = PARA_Resolve_Percent(str, code);
				if (sharp_res.id !=0){
					return percent_res;
				}
				else {
					//否则直接给予赋值
					//首先替换+至空格//不用多此一举,最后的Get还需要用+代替空格
					//auto replace_plus_pos = str.find_last_of('+');
					////复制str变量
					//std::string after_replace_str = str;
					//while (replace_plus_pos != std::string::npos) {
					//	after_replace_str[replace_plus_pos] = ' ';
					//	replace_plus_pos = after_replace_str.find_last_of('+', replace_plus_pos);
					//}
					ComplexParam ret;
					ret.rss_keyword = str;
					return ret;
				}
			}
			//都不能解析
			//最后直接抛出异常 不解析此参数
			//throw e;
		}

	}

	//用于解析User Collection的参数封装
	inline ComplexParam ResolveUserCollectionPara(const std::string& str) {
		BgmCode code = BgmCode::BGM;
		try {
			//只有'/'的解析
			return PARA_Resolve_Virgule(str, code);
		}
		catch (boost::system::system_error&e) {
#ifndef NDEBUG
			{
				bangumi::string debug_msg;
				debug_msg << "PARA_Resolve_Virgule 失败";
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-ResolveTagPara-Virgule", debug_msg);
			}
#endif		
			//都不能解析
			//最后直接抛出异常 不解析此参数
			throw e;
		}
	}


	//Bot: 读取Bangumi娘的配置信息
	inline void BOT_Read_Ini(const BGMCodeParam & param, const std::set<size_t>& parameters_id, const std::set<std::string>& parameters_str) {
		//TEST CQ:shared
		//DEFAULT_SEND(param.type, "[CQ:share,url=http://bgm.tv/subject/220566,title=杀戮的天使,content=简介,image=http://lain.bgm.tv/pic/cover/c/1a/b7/220566_0CMxK.jpg]");
		//TEST CQ:music
		//DEFAULT_SEND(type, "[CQ:music,type=custom,url=http://bgm.tv/subject/220566,audio=https://bangumi.moe/,title=杀戮的天使,content=简介,image=http://lain.bgm.tv/pic/cover/c/1a/b7/220566_0CMxK.jpg]");
		if(std::to_string(param.qq)==bgm.owner_qq&&param.type == BgmRetType::Private)
			DEFAULT_SEND(param.type, bgm.GetConf());



	}
	//Bot: Help信息
	inline void BOT_Help(const BGMCodeParam & param, const std::set<size_t>& parameters_id, const std::set<std::string>& parameters_str) {
		
		bangumi::string help_msg;

		//
		help_msg << "[CQ:image,file=" << bgm.help_pic << "]";
		help_msg >> "使用指南: " << "https://bangumi.irisu.cc/";
		
		DEFAULT_SEND(param.type, help_msg);


	}
	//Bot: 统计信息
	inline void BOT_Statis(const BGMCodeParam & param, const std::set<size_t>& parameters_id, const std::set<std::string>& parameters_str) {
		//参数
		//例如：rank me 返回自己的使用统计
		//例如：rank 默认返回今日的使用排行
		//参数解析
		//类型
		enum class StatisParam
		{
			me,
			default
		};
		//返回me
		StatisParam current_param = StatisParam::default;
		//只有str类型的参数，只接受一个参数
		for (auto&p: parameters_str){
			if (p.find("me")!=std::string::npos){
				current_param = StatisParam::me;
			}
		}

		//Main
		const unsigned max_list_rank = 5;
		//回复的消息
		bangumi::string msg;
		//用户基本信息
		std::string user_id;
		std::string user_qq;
		std::string bangumi_id;
		//用户头像
		std::string image_path = (bgm.cache_path) + USER_PIC_PATH;
		std::string image_file;
		//总使用情况
		std::string all_type[10];
		int all_times = 0;
		std::string today_type[10];
		int today_times = 0;
		//今日使用情况

		//查询语句
		bangumi::string query;
		switch (current_param)
		{
		case StatisParam::me:
		{
			//前置信息
			//msg << "今日使用排行：";
			query << "SELECT user_id,user_qq,user_bangumi,"
				<< "BgmCode_subject,BgmCode_search,BgmCode_user,BgmCode_up,BgmCode_collect,BgmCode_reg,BgmCode_help,BgmCode_tag,BgmCode_statis,BgmCode_unknow,"
				<< "TBgmCode_subject,TBgmCode_search,TBgmCode_user,TBgmCode_up,TBgmCode_collect,TBgmCode_reg,TBgmCode_help,TBgmCode_tag,TBgmCode_statis,TBgmCode_unknow,dmhy_open "
				<< "FROM bgm_users "
				<< "WHERE user_qq="
				<< param.qq;
		}
			break;
		case StatisParam::default:
		{
			//前置信息
			msg << "今日使用排行：";
			//当前时间
			boost::posix_time::ptime current_time = boost::posix_time::second_clock::universal_time() + boost::posix_time::hours(8);
			boost::gregorian::date current_date = current_time.date();
			std::string current_date_str(boost::gregorian::to_iso_extended_string(current_date));

			query << "SELECT user_id,user_qq,user_bangumi,"
				<< "BgmCode_subject,BgmCode_search,BgmCode_user,BgmCode_up,BgmCode_collect,BgmCode_reg,BgmCode_help,BgmCode_tag,BgmCode_statis,BgmCode_unknow,"
				<< "TBgmCode_subject,TBgmCode_search,TBgmCode_user,TBgmCode_up,TBgmCode_collect,TBgmCode_reg,TBgmCode_help,TBgmCode_tag,TBgmCode_statis,TBgmCode_unknow "
				<< "FROM bgm_users "
				<< "WHERE BgmCode_Last_Date>'"
				<< current_date_str
				<< "' ORDER BY TBgmCode_subject+TBgmCode_search+TBgmCode_user+TBgmCode_up+TBgmCode_collect+TBgmCode_reg+TBgmCode_help+TBgmCode_tag+TBgmCode_statis+TBgmCode_unknow DESC";

		}
			break;
		default:
			break;
		}
		//SQL查询
#ifndef NDEBUG
		{
			bangumi::string debug_msg;
			debug_msg << "SQL查询语句:"
				<< query;

			CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-BOT-Statis", debug_msg);
		}
#endif	
		//查询结果
		BGMSQLResult result;
		//影响的行数
		unsigned long affect_rows_num = sql_pool.ExecQuery(query, result);
		try
		{
			SQLCheckResult(affect_rows_num);
		}
		catch (boost::system::system_error& e)
		{
#ifndef NDEBUG
			{
				bangumi::string debug_msg;
				debug_msg << "SQL查询失败:"
					<< std::to_string(affect_rows_num)
					>> e.what();

				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-BOT-Statis", debug_msg);
			}
#endif	
			//通过异常结束函数
			//throw e;
			//直接返回
			msg = "查询失败...";
			DEFAULT_SEND(param.type, msg);
			return;
		}
		if (affect_rows_num > 0) {
			//说明有结果
			switch (current_param)
			{
			case StatisParam::me:
			{
				//用户的基本信息
				user_id = result[0];
				user_qq = result[1];
				bangumi_id = result[2];
				//用户总使用
				for (unsigned j = 0; j < 10; ++j) {
					all_type[j] = result[j + 3];
					all_times += std::stoi(all_type[j]);
				}
				//用户今日使用

				for (unsigned j = 0; j < 10; ++j) {
					today_type[j] = result[j + 13];
					today_times += std::stoi(today_type[j]);
				}
				//头像路径
				if (bangumi_id != "0") {
					image_file = image_path + bangumi_id + ".jpg";
				}
				else {
					image_file = bgm.not_found_ava_path;
				}
				//format
				boost::format fmt("%5d/%-5d");
				//进行权限检测
				if (bangumi_id[0] != '0' && result[23][0] == '9') {
					//通过考核的玩家
					msg << "<BGMer>";
				}
				else {
					//尚无权限
					msg << "-----------";
				}
				//回复消息拼接
				msg >> "[CQ:image,file=" << image_file << "]"
					>> "QQ: " << user_qq << " x BGM: " << bangumi_id //<< "  [" << user_id << ']'
					>> "今日使用： " << today_times << "    总计使用： " << all_times
					>> "-----------"
					>> "条目：" << (fmt%today_type[0] % all_type[0]).str()
					<< "搜索：" << (fmt%today_type[1] % all_type[1]).str()
					>> "用户：" << (fmt%today_type[2] % all_type[2]).str()
					<< "更新：" << (fmt%today_type[3] % all_type[3]).str()
					>> "收藏：" << (fmt%today_type[4] % all_type[4]).str()
					<< "绑定：" << (fmt%today_type[5] % all_type[5]).str()
					>> "帮助：" << (fmt%today_type[6] % all_type[6]).str()
					<< "标签：" << (fmt%today_type[7] % all_type[7]).str()
					>> "统计：" << (fmt%today_type[8] % all_type[8]).str()
					<< "未知：" << (fmt%today_type[9] % all_type[9]).str()
					>> "-----------";
			}
				break;
			case StatisParam::default:
			{
				//如果结果大于最大输出，则修正
				if (affect_rows_num > max_list_rank) {
					affect_rows_num = max_list_rank;
				}
				//format
				boost::format fmt("%5d/%-5d");
				//循环处理
				for (unsigned i = 0; i < affect_rows_num; ++i) {
					//用户的基本信息
					user_id = result[0];
					user_qq = result[1];
					bangumi_id = result[2];
					//用户总使用
					for (unsigned j = 0; j < 10; ++j) {
						all_type[j] = result[j + 3];
						all_times += std::stoi(all_type[j]);
					}
					//用户今日使用
					for (unsigned j = 0; j < 10; ++j) {
						today_type[j] = result[j + 13];
						today_times += std::stoi(today_type[j]);
					}
					//头像路径
					if (bangumi_id != "0") {
						image_file = image_path + bangumi_id + ".jpg";
					}
					else {
						image_file = bgm.not_found_ava_path;
					}

					//回复消息拼接
					msg >> "____<" << (i + 1) << ">____"
						>> "[CQ:image,file=" << image_file << "]"
						>> "QQ: " << user_qq << " x BGM: " << bangumi_id //<< "  [" << user_id << ']'
						>> "今日使用： "<<today_times<<"    总计使用： "<<all_times
						>> "-----------"
						>> "条目：" << (fmt%today_type[0] % all_type[0]).str()
						<< "搜索：" << (fmt%today_type[1] % all_type[1]).str()
						>> "用户：" << (fmt%today_type[2] % all_type[2]).str()
						<< "更新：" << (fmt%today_type[3] % all_type[3]).str()
						>> "收藏：" << (fmt%today_type[4] % all_type[4]).str()
						<< "绑定：" << (fmt%today_type[5] % all_type[5]).str()
						>> "帮助：" << (fmt%today_type[6] % all_type[6]).str()
						<< "标签：" << (fmt%today_type[7] % all_type[7]).str()
						>> "统计：" << (fmt%today_type[8] % all_type[8]).str()
						<< "未知：" << (fmt%today_type[9] % all_type[9]).str()
						//>> "------------"
						>> "============";

					//取下一行
					result.FetchRow(1);
					//重置次数
					all_times = 0;
					today_times = 0;
				}

			}
				break;
			default:
				break;
			}

			
		}
		else {
			switch (current_param)
			{
			case StatisParam::me:
			{
				msg = "我好像没找到你诶...";
			}
				break;
			case StatisParam::default:
			{
				//说明今日没有人使用过机器人
				msg = "今天又是寂寞没人理的一天...";
			}
				break;
			default:
				break;
			}

		}

		//发送消息
		DEFAULT_SEND(param.type, msg);
	}
	
	//API: 返回Bangumi用户信息
	inline void BGM_API_User(const BGMCodeParam & param, const std::set<size_t>& parameters_id, const std::set<std::string>& parameters_str) {

		//bangumi::string ret("<User>");
		//ret >> "ID:\n";
		//for (const auto &i : parameters_id)
		//	ret << i << " ";
		//ret >> "STR:\n";
		//std::string name;
		//for (const auto &i : parameters_str)
		//	ret << i << " ";

		//DEFAULT_SEND(param.type, ret);

		//对每一个识别的ID进行main func
		for (const auto& name : parameters_str) {
			//Main Func
			//===============================
			std::string bangumi_name;
			if (name=="#")
			{
				auto verify_result = VerifyToken(param); 
				size_t &user_id = verify_result.first; 
				bangumi_name = std::to_string(user_id);
			}
			else {
				bangumi_name = name;
			}
			//尝试从缓存中查找
			//如果不需要强制refresh
			if (!param.extra.refresh)
			{
				try {
					size_t temp_id = std::stoul(name);
					bangumi::string msg = BangumiPreFindUser(temp_id).Get();
					//获取用户收藏信息
					msg>>'\n'<<GetUserSumCollections(std::to_string(temp_id));
					DEFAULT_SEND(param.type, msg);
#ifndef NDEBUG
					{
						bangumi::string debug_msg;
						debug_msg << "BGMUser预先查找命中";
						CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-BangumiPreFindUser", debug_msg);
					}
#endif
					continue;
				}
				catch (std::out_of_range&) {
#ifndef NDEBUG
					{
						bangumi::string debug_msg;
						debug_msg << "未找到缓存";
						CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-PreFind-BGMUser", debug_msg);
					}
#endif
				}
				catch (std::invalid_argument&) {
#ifndef NDEBUG
					{
						bangumi::string debug_msg;
						debug_msg << "ID无法转换为数字";
						CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-PreFind-BGMUser", debug_msg);
					}
#endif
				}
				catch (std::exception&) {
#ifndef NDEBUG
					{
						bangumi::string debug_msg;
						debug_msg << "未知的错误";
						CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-PreFind-BGMUser", debug_msg);
					}
#endif
				}
			}
			//===============================
			//使用BGM API
			std::shared_ptr<HTTPRequest> request_one =
				http_client.create_request_fixed(http_client.GetID());

			request_one->set_ret_param(bangumi::BGMRetParam{ param,0,bangumi_name });
			request_one->set_host("api.bgm.tv");
			request_one->set_uri("/user/" + bangumi_name);
			//只有搜索条目时需要Cookie: chii_searchDateLine
			request_one->set_request(request_message(request_one, HTTP_WAY::GET,/* "Cookie: chii_searchDateLine=0;\r\n"*/""));
			//设置回调函数
			request_one->set_callback([](std::shared_ptr<HTTPRequest>request_one, bangumi::BGMRetParam param, int ec) {
				//要回复的string
				bangumi::string msg;
				//回调函数:此时已经完成了响应报文的读取
				std::string json;
				try {
					GetResponseContent(request_one, json, true, ec);
#ifndef NDEBUG
					{
						bangumi::string debug_msg;
						debug_msg << "收到响应的Json"
							>> json.c_str();
						CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTP", debug_msg);
					}
#endif


					auto pic_threads = Resolve::Resolve_User(json, param.extra.refresh);
					msg = pic_threads.second.Get();

					//获取用户收藏信息
					msg >> '\n' << GetUserSumCollections(param.cur_str);

					//结束的等待
					for (auto &t : pic_threads.first) {
						if (t != nullptr&&t->joinable())
							t->join();
					}
				}
				catch (boost::system::system_error & e) {
					//如果发生了异常,使用异常回复(包括没有找到用户等)
					//这里不使用Reply类而是直接赋值回复
					msg = e.what();
					msg << "[" << param.cur_str << "]...";
				}

				//发送回复
				DEFAULT_SEND(param.type, msg);
				//从httpclient移除引用从而析构自身

				http_client.RemoveID(request_one->get_id());
			});
			request_one->execute();
		}

		//boost::this_thread::sleep(boost::posix_time::seconds(10));
//		std::string json;
//
//		//TODO:完成回调函数
//		boost::this_thread::sleep(boost::posix_time::seconds(2));
//
//		GetResponseContent(request_one, json);
//		//DEBUG
//		//DEFAULT_SEND(param.type, json.c_str());
//#ifndef NDEBUG
//		{
//			bangumi::string debug_msg;
//			debug_msg<<"收到响应的Json"
//				>>json.c_str();
//			CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTP", debug_msg);
//		}
//#endif
//		auto pic_threads = Resolve::Resolve_User(json);
//		bangumi::string msg = pic_threads.second.Get();
//
//		//结束的等待
//		for (auto &t : pic_threads.first) {
//			if(t != nullptr&&t->joinable())
//				t->join();
//		}
//		//发送回复
//		DEFAULT_SEND(param.type, msg);
	}
	//API: 返回Bangumi条目信息
	inline void BGM_API_Subject(const BGMCodeParam & param, const std::set<size_t>& parameters_id, const std::set<std::string>& parameters_str) {
		//last_subject 主要用于更新last_subject用
		size_t last_subject_id = 0;
		//sql中的last_subject
		size_t sql_last_subject_id = 0;
		//构建一个复杂参数类的Vector
		std::vector<ComplexParam> paramters;
		
		//排除空语句
		if (parameters_str.empty() && parameters_id.empty()) {
			//这是不进行last判断
			//由于空消息体,各种方面都不合适,还是选择直接返回,因为空消息也走不到这里
			//同时设置code_type清除Subject指令
			param.bgm_code.erase(BgmCode::Subject);
			//同时添加一个Unknow的指令类型
			param.bgm_code.emplace(BgmCode::Unknow);
			return;
		}
		//bangumi::string ret("<Subject>");
		//ret >> "ID:\n";
		//for (const auto& i : parameters_id)
		//	ret << i << " ";
		//ret >> "STR:\n";
		//for (const auto& i : parameters_str)
		//	ret << i << " ";

		//DEFAULT_SEND(param.type, ret);
		//用于存放最后一个Subject_id以用作last_subject(前提是没有使用last_subject)

		for (const auto& i : parameters_id) {
			paramters.emplace_back(i);
		}
		//是否已经请求过last_subject的记号
		bool has_sql_last_subject = false;

		//压入解析参数
		for (const auto& i : parameters_str) {
			try {
				auto temp = ResolveSubjectPara(i);
				if (temp.use_last_subject_id) {
					if (!has_sql_last_subject) {
						//标记已经sql过了last Subject
						has_sql_last_subject = true;
						//sql last subject
						sql_last_subject_id = GetLastSubjectID(param.qq);
					}

					temp.id = sql_last_subject_id;
				}
				paramters.emplace_back(temp);

			}
			catch(boost::system::system_error& e){
				//直接continue
#ifndef NDEBUG
				{
					bangumi::string debug_msg;
					debug_msg << "ComplexParam Failed!"
						>> e.what();
					CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Func-Subject-API", debug_msg);
				}
#endif	
				continue;
			}
			
		}
		//注意参数名
#define ComplexParamRet(subject_id,complex_param,type,refresh)\
std::string html = bangumi::GetSubjectHtml(subject_id);\
bangumi::string res;\
bangumi::string res1;\
bangumi::string res2;\
bangumi::string res3;\
bangumi::string res4;\
if(complex_param.add_tag||complex_param.add_comment){\
	res<<"[条目: "<<subject_id<<"]\n";\
	if (complex_param.add_tag)\
		res << Resolve::ResolveSubjectTag(html) >> "-------------";\
	if (complex_param.add_comment)\
		res << Resolve::ResolveSubjectComment(html,refresh) << "-------------";\
}\
if (complex_param.add_role){\
	res1<<"[条目: "<<subject_id<<"]\n";\
	auto roles = Resolve::ResolveSubjectCharacter(html,refresh);\
	if(!roles.first.empty())\
		res1 << roles.first; \
	/*else*/\
	/*	res1 >>"未收录角色...";*/\
	/*res2 << roles.second;*/ \
} \
if (complex_param.add_air_status){\
	/*关联用户信息 */\
	auto verify_result = VerifyToken(param);\
	size_t &user_id = verify_result.first;\
	std::string &access_token = verify_result.second.first;\
	std::string &refresh_token = verify_result.second.second;\
	if (!access_token.empty()) {\
		/*解析条目状态*/\
		auto& resolved_subject = Resolve::ResolveSubjectCollection(html, subject_id, refresh);\
		if(resolved_subject.GetEpsCount() == 0){\
		/*如果请求失败,重新请求一次*/\
		resolved_subject = Resolve::ResolveSubjectCollection(html, subject_id, false);\
		}\
		/*如果有此注册用户*/\
		auto& progress_struct =  GetUserSubjectProgress(subject_id, resolved_subject.GetEpsCount(), user_id,\
			param.qq, access_token, refresh_token, refresh);\
		/*取得进度的str*/\
		auto& user_progress = progress_struct.second.progress.progress;\
		int curr_eps = 0;\
		/*转换为数字类型*/\
		try {\
			curr_eps = std::stoi(user_progress);\
		}\
		catch(std::exception&) {}\
		resolved_subject.SetCurrentEps(curr_eps);\
		res3 << resolved_subject.Get();\
		/*加上进度信息*/\
		/*res3 << progress_struct.first;*/\
		progress_struct.second.progress.SetExStr(resolved_subject.GetExStr());\
		res3 << progress_struct.second.ProgressGet();\
	}else{\
		auto& resolved_subject = Resolve::ResolveSubjectCollection(html, subject_id, refresh);\
		if(resolved_subject.GetEpsCount() == 0){\
		/*如果请求失败,重新请求一次*/\
		resolved_subject = Resolve::ResolveSubjectCollection(html, subject_id, false);\
		}\
		res3 << resolved_subject.Get();\
	}\
}\
if (complex_param.add_staff)\
	res2<< Resolve::ResolveStaff(html,subject_id,refresh);\
if (complex_param.add_attach)\
	res4<< Resolve::ResolveAttach(html,subject_id,refresh);\
if (!res.empty())\
	DEFAULT_SEND(type, res);\
if (!res1.empty())\
	DEFAULT_SEND(type, res1);\
if (!res2.empty())\
	DEFAULT_SEND(type, res2);\
if (!res3.empty())\
	DEFAULT_SEND(type, res3);\
if (!res4.empty())\
	DEFAULT_SEND(type, res4);


		//对每一个识别的ID进行main func
		for (const auto& complex_param : paramters) {
			const auto& subject_id = complex_param.id;
			//为last_subject赋值
			last_subject_id = subject_id;
			//如果id = 0直接继续
			if (subject_id == 0)
			{
				bangumi::string error_msg = boost::system::system_error(bangumi_bot_errors::empty_subject).what();
				error_msg << "[" << subject_id << "]";
				//发送回复
				DEFAULT_SEND(param.type, error_msg);
				//continue
				continue;
			}
			//Main Func
			//===============================
			//首先检查是否是single
			if (complex_param.single) {
				//======Complex_param=======
				if (complex_param.NeedAdd()) {
					ComplexParamRet(subject_id, complex_param, param.type, param.extra.refresh);
#ifndef NDEBUG
					{
						bangumi::string debug_msg;
						debug_msg << "res=" << res
							>> "res1=" << res1
							>> "res2=" << res2
							>> "res3=" << res3
							>> "refresh=" << std::to_string(param.extra.refresh);
						CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-MSG", debug_msg);
					}
#endif
				}
				//直接continue;
				continue;
			}
			//尝试从缓存中查找
			//如果不需要强制refresh
			if (!param.extra.refresh)
			{
				try {
					auto& subject = BangumiPreFindSubject(subject_id);
					bangumi::string msg = subject.Get();
					//====Access Token====
					//关联用户信息
					auto verify_result = VerifyToken(param);
					size_t &user_id = verify_result.first;
					std::string &access_token = verify_result.second.first;
					std::string &refresh_token = verify_result.second.second;
					if (!access_token.empty()) {
						//如果有此注册用户
						//加上进度信息
						msg << GetUserSubjectProgress(subject.subject_id, subject.eps, user_id, 
							param.qq, access_token,refresh_token, param.extra.refresh).first;

					}
#ifndef NDEBUG
					{
						bangumi::string debug_msg;
						debug_msg << msg;
						CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-MSG", debug_msg);
					}
#endif
					DEFAULT_SEND(param.type, msg);
					//======Complex_param=======
					if (complex_param.NeedAdd()) {
						ComplexParamRet(subject_id, complex_param, param.type, param.extra.refresh);

					}
#ifndef NDEBUG
					{
						bangumi::string debug_msg;
						debug_msg << "BGMSubject预先查找命中";
						CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-BangumiPreFindSubject", debug_msg);
					}
#endif
					continue;
				}
				catch (boost::system::system_error & e) {
#ifndef NDEBUG
					{
						bangumi::string debug_msg;
						debug_msg << e.what();
						CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-BangumiPreFindSubject", debug_msg);
					}
#endif
				}
				catch (std::out_of_range&) {
#ifndef NDEBUG
					{
						bangumi::string debug_msg;
						debug_msg << "未找到缓存";
						CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-PreFind-BGMSubject", debug_msg);
					}
#endif
				}
				//				catch (std::invalid_argument&) {
				//#ifndef NDEBUG
				//					{
				//						bangumi::string debug_msg;
				//						debug_msg << "ID无法转换为数字";
				//						CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-PreFind-BGMUser", debug_msg);
				//					}
				//#endif
				//				}
				catch (std::exception&) {
#ifndef NDEBUG
					{
						bangumi::string debug_msg;
						debug_msg << "未知的错误";
						CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-PreFind-BGMSubject", debug_msg);
					}
#endif
				}
			}
			//===============================
			//尝试从SQL中读取缓存
			if (!param.extra.refresh)
			{
				try {
					auto& subject = BangumiSQLFindSubject(subject_id);
					bangumi::string msg = subject.Get();
					//====Access Token====
					//关联用户信息
					auto verify_result = VerifyToken(param);
					size_t &user_id = verify_result.first;
					std::string &access_token = verify_result.second.first;
					std::string &refresh_token = verify_result.second.second;
					if (!access_token.empty()) {
						//如果有此注册用户
						//加上进度信息
						msg << GetUserSubjectProgress(subject.subject_id, subject.eps, user_id,
							param.qq, access_token,refresh_token, param.extra.refresh).first;

					}

#ifndef NDEBUG
					{
						bangumi::string debug_msg;
						debug_msg << msg;
						CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-MSG", debug_msg);
					}
#endif
					DEFAULT_SEND(param.type, msg);
					//======Complex_param=======
					if (complex_param.NeedAdd()) {
						ComplexParamRet(subject_id, complex_param, param.type, param.extra.refresh);
					}
#ifndef NDEBUG
					{
						bangumi::string debug_msg;
						debug_msg << "BGMSubject SQL命中";
						CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-BangumiSQLFindSubject", debug_msg);
					}
#endif
					continue;
				}
				catch (boost::system::system_error&e) {
#ifndef NDEBUG
					{
						bangumi::string debug_msg;
						debug_msg << "尝试从SQL中读取Subject缓存失败";
						debug_msg >> "失败信息: " << e.what();
						CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-SQLFind-BGMUser", debug_msg);
					}
#endif
				}
			}
			//===============================
			//使用BGM API
			try {
				std::shared_ptr<HTTPRequest> request_one =
					http_client.create_request_fixed(http_client.GetID());

				//[注意]这里需要给BGMRetParam额外的信息
				bangumi::BGMRetParam bgm_param{ param,subject_id,"" };
				//因为Subject中的异步操作需要一些complex_param中的参数
				bgm_param.complex_param = complex_param;
				request_one->set_ret_param(bgm_param);
				request_one->set_host("api.bgm.tv");
				//默认就是小的json结构体
				request_one->set_uri("/subject/" + std::to_string(subject_id));
				//只有搜索条目时需要Cookie: chii_searchDateLine
				request_one->set_request(request_message(request_one, HTTP_WAY::GET,/* "Cookie: chii_searchDateLine=0;\r\n"*/""));
				//设置回调函数
				request_one->set_callback([](std::shared_ptr<HTTPRequest>request_one, bangumi::BGMRetParam param, int ec) {
					//要回复的string
					bangumi::string msg;
					//复杂回复结构体
					auto& complex_param = param.complex_param;
					//回调函数:此时已经完成了响应报文的读取
					std::string json;
					try {
						GetResponseContent(request_one, json, true, ec);
#ifndef NDEBUG
						{
							bangumi::string debug_msg;
							debug_msg << "收到响应的Json"
								>> json.c_str();
							CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTP", debug_msg);
						}
#endif

						auto pic_threads = Resolve::Resolve_Subject(json, param.extra.refresh);
						msg = pic_threads.second.Get();

						//====Access Token====
						//关联用户信息
						auto verify_result = VerifyToken(param);
						size_t &user_id = verify_result.first;
						std::string &access_token = verify_result.second.first;
						std::string &refresh_token = verify_result.second.second;
						if (!access_token.empty()) {
							//如果有此注册用户
							//加上进度信息
							msg << GetUserSubjectProgress(param.cur_id, pic_threads.second.eps, user_id, 
								param.qq, access_token, refresh_token, param.extra.refresh).first;

						}


						//结束的等待
						for (auto &t : pic_threads.first) {
							if (t != nullptr&&t->joinable())
								t->join();

						}
					}
					catch (boost::system::system_error & e) {
						//如果发生了异常,使用异常回复(包括没有找到用户等)
						//这里不使用Reply类而是直接赋值回复
						msg = e.what();
						msg << "[" << param.cur_id << "]...";
					}
#ifndef NDEBUG
					{
						bangumi::string debug_msg;
						debug_msg << msg;
						CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-MSG", debug_msg);
					}
#endif
					//发送回复
					DEFAULT_SEND(param.type, msg);
					//======Complex_param=======
					if (complex_param.NeedAdd()) {
						ComplexParamRet(param.cur_id, complex_param, param.type, param.extra.refresh);

					}

					//从httpclient移除引用从而析构自身

					http_client.RemoveID(request_one->get_id());
				});
				request_one->execute();

			}
			catch (boost::system::system_error& e) {
#ifndef NDEBUG
				{
					bangumi::string debug_msg;
					debug_msg << "FUNC-API-Subject Request Error"
						>> e.what();
					CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTP-API-Subject", debug_msg);
				}
#endif
			}
		}

		//结束之后更新last_subject
		if (last_subject_id != 0) {
			//简单一点直接单线程
			//查询语句
			bangumi::string update_query;

			update_query << "UPDATE bgm_users SET "
				//<< "user_bangumi=" << auth.user_id << ","
				<< "user_last_searched=" << last_subject_id
				<< " WHERE user_qq=" << param.qq;
			auto affect_rows_num = sql_pool.ExecQueryNoRes(update_query);
			try
			{
				SQLCheckResult(affect_rows_num);
			}
			catch (boost::system::system_error& e)
			{
#ifndef NDEBUG
				{
					bangumi::string debug_msg;
					debug_msg << "Update Last Subject失败:"
						<< std::to_string(affect_rows_num)
						>> e.what();

					CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Func-Subject-API", debug_msg);
				}
#endif	
			}
		}
	}
	//API: 绑定QQ与Bangumi
	inline void BGM_API_Auth(const BGMCodeParam & param, const std::set<size_t>& parameters_id, const std::set<std::string>& parameters_str) {

		if (param.type != BgmRetType::Private) {
			DEFAULT_SEND(param.type, "绑定功能请私聊~");
			return;
		}

		std::string redirect_url = GetRedirectUrl(param.qq);

		//https://bgm.tv/oauth/authorize?client_id=bgm2435affad00821e3&response_type=code&redirect_uri=http%3A%2F%2Fwww.irisu.cc%2Fbangumi.php%3Fnu%3D0dyvwccxw&state=dcyw
		bangumi::string request_url = "http://bgm.tv/oauth/authorize?response_type=code";
		request_url << "&client_id=" << bgm.bangumi_client_id
			<< "&redirect_uri=" << redirect_url;
		bangumi::string card_msg;
		card_msg << "[CQ:share,url=" << request_url
			<< ",title=点击与我签订契约吧~"
			<< ",content=契约后可在QQ上实现更多功能~"
			<< ",image=" << bgm.card_image_url
			<< "]";
		//发送一个卡片消息
		DEFAULT_SEND(param.type, card_msg);

		//开启服务器
		try {

			//Start中内置定时结束
			http_server.Start(bgm.server_port_num);

		}
		catch (boost::system::system_error& e) {
#ifndef NDEBUG
			{
				bangumi::string debug_msg;
				debug_msg << "HTTPServer Error";
				debug_msg >> "error message = " << e.what();
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTPServer", debug_msg);
			}
#endif			
			DEFAULT_SEND(param.type, "好像出了点小问题...");
		}
	}

	//API: 管理收藏API
	inline void BGM_API_Collection(const BGMCodeParam & param, const std::set<size_t>& parameters_id, const std::set<std::string>& parameters_str) {
		//参数是一个STR或一个INT
		//可以使用#Angel参数
		//例如: #Angel(使用#解析参数: 先使用LIKE模糊搜索SQL中Subject(id从高往低排,保证取最新匹配),如果没有结果使用SearchAPI或HTML解析评价最多的那个条目,同时update到SQL中)
		//例如: 123456(默认更新为在看)
		//例如: 1234/do(更新为在看)
		//例如: 1234/fin(更新为看完)
		//例如: /fin(更新last为看完)
		//例如: <空> (更新last为在看)
		//构建一个复杂参数类的Vector
		std::vector<ComplexParam> paramters;
		//last_subject 主要用于更新last_subject用
		size_t last_subject_id = 0;
		//last_subject_id
		size_t sql_last_subject_id = 0;
		//排除空语句
		if (parameters_str.empty() && parameters_id.empty()) {
			//Last判断
			try {
				sql_last_subject_id = GetLastSubjectID(param.qq);
				if (sql_last_subject_id != 0) {
					paramters.emplace_back(sql_last_subject_id);
				}
				else {
					return;
				}
			}
			catch(boost::system::system_error& e){
				//LastSubjectGet失败
#ifndef NDEBUG
				{
					bangumi::string debug_msg;
					debug_msg << "GetLastSubjectFailed!"
						>> e.what();
					CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Func-Collection-API", debug_msg);
				}
#endif	
				return;
			}	
		}
			
		//bangumi::string ret("<Collect>");
		//ret >> "ID:\n";
		//for (const auto& i : parameters_id)
		//	ret << i << " ";
		//ret >> "STR:\n";
		//for (const auto& i : parameters_str)
		//	ret << i << " ";

		//DEFAULT_SEND(param.type, ret);


		for (const auto& i : parameters_id) {
			paramters.emplace_back(i);
		}
		//判断是否已经sql查询过last_subject了
		bool has_sql_last_subject = false;
		//压入解析参数(STR参数)
		for (const auto& i : parameters_str) {
			try {
				//auto temp = ResolveCollectPara(i);
				////判断是否使用Last_subject
				////压入paramters
				//paramters.emplace_back(temp);
				auto temp = ResolveCollectPara(i);
				if (temp.use_last_subject_id) {
					if (!has_sql_last_subject) {
						//标记已经sql过了last Subject
						has_sql_last_subject = true;
						//sql last subject
						sql_last_subject_id = GetLastSubjectID(param.qq);
					}

					temp.id = sql_last_subject_id;
				}
				paramters.emplace_back(temp);
			}
			catch (boost::system::system_error&e) {
				//因为str可以不使用complex_param
				//因此str怎样都是一个合法的参数
				//因此直接构造一个默认的
				//因为只有INT有效,STR如不能正确转换为ID就忽视
#ifndef NDEBUG
				{
					bangumi::string debug_msg;
					debug_msg << "ComplexParam Failed!"
						>> e.what();
					CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Func-Collection-API", debug_msg);
				}
#endif		
			}

		}

		////用户没有last_subject的标识
		//bool user_have_not_last = false;

		//对每一个识别的ID进行main func
		for (const auto& complex_param : paramters) {
			const size_t &subject_id = complex_param.id;
			//更新上次使用的Last subject
			last_subject_id = subject_id;
//			//判断是否use_last_subject
//			if (complex_param.use_last_subject_id) {
//				//判断是否已经读取过last_subject
//				if (last_subject_id == 0&&!user_have_not_last) {
//					try {
//						last_subject_id = GetLastSubjectID(param.qq);
//					}
//					catch (boost::system::system_error& e) {
//#ifndef NDEBUG
//						{
//							bangumi::string debug_msg;
//							debug_msg << "GetLastSubjectFailed! in for"
//								>> e.what();
//							CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Func-Collection-API", debug_msg);
//						}
//#endif	
//						last_subject_id = 0;
//					}
//					if (last_subject_id == 0)
//					{
//						//用户没有last_subject的标识
//						user_have_not_last = true;
//					}
//				}
//					
//				subject_id = last_subject_id;
//			}
//			else {
//				subject_id = complex_param.id;
//			}
			//无效subject回复
			if (subject_id == 0) {
				bangumi::string error_msg = boost::system::system_error(bangumi_bot_errors::empty_subject).what();
				error_msg << "[" << subject_id << "]";
				//发送回复
				DEFAULT_SEND(param.type, error_msg);
				//continue
				continue;
			}
			//======Main Start======
			//====Access Token====
			//关联用户信息
			std::string access_token;
			try {
				auto verify_result = VerifyToken(param);
				//size_t &user_id = verify_result.first;
				access_token = verify_result.second.first;
				//std::string &refresh_token = verify_result.second.second;
				if (access_token.empty()) {
					//未注册用户无法使用此功能
					throw boost::system::system_error(bangumi_bot_errors::you_need_bind_your_bgm_id);
				}
			}
			catch (boost::system::system_error&e) {
				bangumi::string error_msg;
				error_msg << e.what();
				//发送回复
				DEFAULT_SEND(param.type, error_msg);
				//
				continue;
			}
			//处理Tag
			bangumi::string tag_msg;
			if (!complex_param.collection_tags.empty())
			{
				tag_msg << "&tags=";
				std::string temp = code_encoder.Conv(complex_param.collection_tags);
				for (auto &c : temp) {
					if (c=='+')
					{
						tag_msg << "%20";
						continue;
					}
					tag_msg << c;
				}
				
			}
			//只能请求API
			bangumi::string content;
			content << "status=" << complex_param.collection_status
				<< "&rating=" << complex_param.collection_rating
				<< "&comment=" << code_encoder.Conv(complex_param.collection_comment);
			if (!tag_msg.empty())
			{
				content << tag_msg;
			}

			//post中的www编码其实是为了防止URI过长而将参数放在了content中,POST时一定不能忘记使用Content-Length指定大小
			//否则会出现不能正常读取参数的问题
			bangumi::string header("Content-Type: application/x-www-form-urlencoded\r\n");
			header << "Authorization: Bearer " << access_token << "\r\n";
			header << "Content-Length: " << content.length() << "\r\n";

			bangumi::string uri;
			uri << "/collection/"
				<< subject_id
				<< "/update";
#ifndef NDEBUG
			{
				bangumi::string debug_msg;
				debug_msg << "Collection:"
					>> "header = " << header
					>> "content = " << content
					>> "uri = " << uri;

				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Func-Collection-API", debug_msg);
			}
#endif	

			//创建一个请求
			std::shared_ptr<HTTPRequest> request_one =
				http_client.create_request_fixed(http_client.GetID());

			//构造一个bangumi::BGMRetParam
			//由于回调函数中需要accesstoken,因此直接为extra_msg赋值
			bangumi::BGMRetParam bgmretparam{ param,subject_id,"" };
			bgmretparam.extra_msg = access_token;
			//同时也不要忘记给复杂参数赋值
			bgmretparam.complex_param = complex_param;
			request_one->set_ret_param(bgmretparam);
			request_one->set_host("api.bgm.tv");
			request_one->set_uri(uri);
			//只有搜索条目时需要Cookie: chii_searchDateLine
			request_one->set_request(request_message(request_one, HTTP_WAY::POST,
				header,
				content));
			//设置回调函数
			request_one->set_callback([](std::shared_ptr<HTTPRequest>request_one, bangumi::BGMRetParam param, int ec) {
				//要回复的string
				bangumi::string msg;
				//复杂回复结构体
				auto& complex_param = param.complex_param;
				//回调函数:此时已经完成了响应报文的读取
				std::string json;
				try {
					GetResponseContent(request_one, json, true, ec);
#ifndef NDEBUG
					{
						bangumi::string debug_msg;
						debug_msg << "收到响应的Json"
							>> json.c_str();
						CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTP", debug_msg);
					}
#endif
					
					//======API Start======



					
					//从HTML解析需要的信息: 作品名,总集数,图片,当前放送的话数等
					std::string html = bangumi::GetSubjectHtml(param.cur_id);
					
					BangumiSubjectCollection subject_data = Resolve::ResolveSubjectCollection(html, param.cur_id, param.extra.refresh);
					if (subject_data.GetEpsCount() == 0) {
						/*如果请求失败,重新请求一次*/\
						subject_data = Resolve::ResolveSubjectCollection(html, param.cur_id, false);
					}
					if (!subject_data.Valid()) {
						//说明 这个条目是不对非会员开放的
						throw boost::system::system_error(bangumi_bot_errors::maybe_301_maybe_limit);
					}
					
					//返回的消息加上条目的html解析部分信息
					msg << subject_data.Get();
					
					//解析json
					auto resolve_result = Resolve::Resolve_Collect(json, param.extra.refresh);

					//=============判断是否是collect===============
					if (complex_param.collection_status.compare("collect")==0) {
						//说明用户直接标记为了看过
						//因此直接更新此条目的ALLEPS
						//使用同步的HTTP
						//使用CollectionAPI收藏为看完
						try {
							//Collection API 同步
							//
							bangumi::string content;
							content << "watched_eps=" << subject_data.GetEpsCount()
								<< "&watched_vols=" << subject_data.GetEpsCount();


							//post中的www编码其实是为了防止URI过长而将参数放在了content中,POST时一定不能忘记使用Content-Length指定大小
							//否则会出现不能正常读取参数的问题
							bangumi::string header("Content-Type: application/x-www-form-urlencoded\r\n");
							header << "Authorization: Bearer " << param.extra_msg << "\r\n";
							header << "Content-Length: " << content.length() << "\r\n";

							bangumi::string uri;
							uri << "/subject/"
								<< param.cur_id
								<< "/update/watched_eps";
#ifndef NDEBUG
							{
								bangumi::string debug_msg;
								debug_msg << "Update:"
									>> "header = " << header
									>> "content = " << content
									>> "uri = " << uri;

								CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Func-Update-API", debug_msg);
							}
#endif	

							//post中的www编码其实是为了防止URI过长而将参数放在了content中,POST时一定不能忘记使用Content-Length指定大小
							//否则会出现不能正常读取参数的问题
							bangumi::string request1;
							request1 << "POST " << uri << " HTTP/1.1\r\n"
								<< "Host: api.bgm.tv\r\n"
								<< header << "\r\n"
								<< content;

							std::string json1 = http_client.SyncBGMHTTPRequest(request1);

#ifndef NDEBUG
							{
								bangumi::string debug_msg;
								debug_msg << "收到响应的Json"
									>> json1.c_str();
								CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Func-RefreshToken", debug_msg);
							}
#endif

							//解析json
							//其实不用解析也罢,并不关心结果
							auto resolve_result2 = Resolve::Resolve_Update(json1, param.extra.refresh);

							if (resolve_result2) {
								//如果更新进度成功则设置 百分百的完成度
								resolve_result.second.progress.progress = std::to_string(subject_data.GetEpsCount()) + "/" 
									+ std::to_string(subject_data.GetEpsCount());
							}
						}
						catch (boost::system::system_error&e) {
							//发生了错误
#ifndef NDEBUG
							{
								bangumi::string debug_msg;
								debug_msg << "在Collection中同步请求Update API失败"
									>> e.what();
								CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-API-Collect-Update", debug_msg);
							}
#endif
							//直接忽略
						}
					}
					//============================================
					//加上Subject的总集数
					//由于此API固定返回为进度0,因此忽略此信息
					//resolve_result.second.progress.AddEps(subject_data.GetEpsAiredCount());

					//返回的消息加上用户及进度等信息
					msg << resolve_result.second.ProgressGet();

					//结束的等待
					for (auto &t : resolve_result.first) {
						if (t != nullptr&&t->joinable())
							t->join();
					}

						
						

					//======API Over======
				}
				catch (boost::system::system_error & e) {
					//如果发生了异常,使用异常回复(包括没有找到用户等)
					//这里不使用Reply类而是直接赋值回复
					msg << e.what();
					//msg << "[" << param.cur_str << "]...";
				}

				//发送回复
				DEFAULT_SEND(param.type, msg);

				//从httpclient移除引用从而析构自身

				http_client.RemoveID(request_one->get_id());
			});
			request_one->execute();

			//======Main Over======
		}
		//结束之后更新last_subject
		if (last_subject_id != 0) {
			//简单一点直接单线程
			//查询语句
			bangumi::string update_query;

			update_query << "UPDATE bgm_users SET "
				//<< "user_bangumi=" << auth.user_id << ","
				<< "user_last_searched=" << last_subject_id
				<< " WHERE user_qq=" << param.qq;
			auto affect_rows_num = sql_pool.ExecQueryNoRes(update_query);
			try
			{
				SQLCheckResult(affect_rows_num);
			}
			catch (boost::system::system_error& e)
			{
#ifndef NDEBUG
				{
					bangumi::string debug_msg;
					debug_msg << "Update Last Subject失败:"
						<< std::to_string(affect_rows_num)
						>> e.what();

					CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Func-Subject-API", debug_msg);
				}
#endif	
			}
		}

	}
	//API: 更新进度API
	inline void BGM_API_Update(const BGMCodeParam & param, const std::set<size_t>& parameters_id, const std::set<std::string>& parameters_str) {
		//++或up两个命令
		//参数是一个int 或 str, str只能使用/附加参数
		//例如: 123456(更新进度+1(首先请求API得知),如果没有收藏(失败的返回),则强制收藏为在看(do))
		//例如: #Angel(使用#解析参数: 先使用LIKE模糊搜索SQL中Subject(id从高往低排,保证取最新匹配),如果没有结果使用SearchAPI或HTML解析评价最多的那个条目,同时update到SQL中,但并不更新相当于请求了默认的Search)
		//例如: #Angel/2(更新到两集)
		//例如: #Angel/+2(向后更新两集进度)
		//例如: #Angel+1/+2(SQL中第二个匹配结果)
		//例如: #Angel+1/air(更新到已放送的最新的一集)
		//例如: #Angel+1/air-1(更新到已放送的最新的前一集>0)
		//例如: #Angel+1/fin(更新到最后一集,同时自动标记为collect状态)
		//例如: /fin(省略参数)
		//例如: /fin/8/吐槽(同时评分8分,和吐槽)
		//可以使用计时系统
		//计时由全局的类进行run(),每分钟进行一次Check()Vector<TimeWork>,TimeWork拥有API函数需要的参数的拷贝和函数的指针
		//暂时不进行存储计时任务
		//首先检查TimeWorker是否,目前仅用于Update命令
		if (param.extra.countdown != 0) {
			//说明是一个延迟任务
			bangumi::string delay_code_msg;

			if (time_worker.GetCurrentListNum() <= bgm.max_time_work_num) {
				//已经读取过countdown,设置为0
				BGMCodeParam new_param(param);
				new_param.extra.countdown = 0;
				time_worker.AddAPIFunc(std::cref(BGM_API_Update), new_param, parameters_id, parameters_str, param.extra.countdown);
				delay_code_msg << "此更新的结果将于[" << param.extra.countdown << "]分钟后返回."
					>> "如果超时仍没有回复结果, 说明发生了点意外状况, 请手动更新...";
			}
			else {
				delay_code_msg << "很抱歉, 当前延时命令已达上限, 请手动更新...";
			}

			//通知用户是一个延迟命令
			DEFAULT_SEND(param.type, delay_code_msg);
			return;
		}
		//构建一个复杂参数类的Vector
		std::vector<ComplexParam> paramters;
		//last_subject 主要用于更新last_subject用
		size_t last_subject_id = 0;
		//last_subject_id
		size_t sql_last_subject_id = 0;

		//排除空语句
		if (parameters_str.empty() && parameters_id.empty()) {
			//Last判断
			try {
				sql_last_subject_id = GetLastSubjectID(param.qq);
				if (sql_last_subject_id != 0) {
					paramters.emplace_back(sql_last_subject_id);
				}
				else {
					return;
				}
			}
			catch (boost::system::system_error& e) {
				//LastSubjectGet失败
#ifndef NDEBUG
				{
					bangumi::string debug_msg;
					debug_msg << "GetLastSubjectFailed!"
						>> e.what();
					CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Func-Update-API", debug_msg);
				}
#endif	
				return;
			}
		}
			
		//bangumi::string ret("<Update>");
		//ret >> "ID:\n";
		//for (const auto& i : parameters_id)
		//	ret << i << " ";
		//ret >> "STR:\n";
		//for (const auto& i : parameters_str)
		//	ret << i << " ";

		//DEFAULT_SEND(param.type, ret);

		for (const auto& i : parameters_id) {
			paramters.emplace_back(i);
		}

		//判断是否已经sql查询过last_subject了
		bool has_sql_last_subject = false;
		//压入解析参数(STR参数)
		for (const auto& i : parameters_str) {
			try {
				//auto temp = ResolveCollectPara(i);
				////判断是否使用Last_subject
				////压入paramters
				//paramters.emplace_back(temp);
				auto temp = ResolveUpdatePara(i);
				if (temp.use_last_subject_id) {
					if (!has_sql_last_subject) {
						//标记已经sql过了last Subject
						has_sql_last_subject = true;
						//sql last subject
						sql_last_subject_id = GetLastSubjectID(param.qq);
					}

					temp.id = sql_last_subject_id;
				}
				paramters.emplace_back(temp);
			}
			catch (boost::system::system_error&e) {
				//因为str可以不使用complex_param
				//因此str怎样都是一个合法的参数
				//因此直接构造一个默认的
				//因为只有INT有效,STR如不能正确转换为ID就忽视
#ifndef NDEBUG
				{
					bangumi::string debug_msg;
					debug_msg << "ComplexParam Failed!"
						>> e.what();
					CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Func-Update-API", debug_msg);
				}
#endif		
			}

		}
		//--------------------------------------------
		//对每一个识别的ID进行main func
		for (auto& complex_param : paramters) {
			const auto& subject_id = complex_param.id;
			//更新last subject
			last_subject_id = subject_id;
			//无效的ID
			if (subject_id == 0) {
				bangumi::string error_msg = boost::system::system_error(bangumi_bot_errors::empty_subject).what();
				error_msg << "[" << subject_id << "]";
				//发送回复
				DEFAULT_SEND(param.type, error_msg);
				//continue
				continue;
			}
			//======Main Start======
			//====Access Token====
			//关联用户信息
			size_t user_id;
			std::string access_token;
			std::string refresh_token;
			try {
				auto verify_result = VerifyToken(param);
				user_id = verify_result.first;
				access_token = verify_result.second.first;
				refresh_token = verify_result.second.second;
				if (access_token.empty()) {
					//未注册用户无法使用此功能
					throw boost::system::system_error(bangumi_bot_errors::you_need_bind_your_bgm_id);
				}
			}
			catch (boost::system::system_error&e) {
				bangumi::string error_msg;
				error_msg << e.what();
				//发送回复
				DEFAULT_SEND(param.type, error_msg);
				//
				continue;

			}
			try {
				//从HTML解析需要的信息: 作品名,总集数,图片,当前放送的话数等
				std::string html = bangumi::GetSubjectHtml(subject_id);
				BangumiSubjectCollection subject_data = Resolve::ResolveSubjectCollection(html, subject_id, param.extra.refresh);
				if (subject_data.GetEpsCount() == 0)
				{
					//如果请求的信息集合总话数为0则再请求一次以防止更新失败
					subject_data = Resolve::ResolveSubjectCollection(html, subject_id, false);
				}
				if (!subject_data.Valid()) {
					//说明 这个条目是不对非会员开放的
					throw boost::system::system_error(bangumi_bot_errors::maybe_301_maybe_limit);
				}

				//此用户的个人信息
				bangumi::BangumiUser this_bgm_user;
				//此用户对此条目的进度
				//bangumi::BangumiUserProgress user_progress;
				//也要在此处请求用户的进度
				if (!access_token.empty()) {
					//如果有此注册用户
					//获取User,其中包括进度信息
					auto progress_res = GetUserSubjectProgress(subject_id, subject_data.eps_counts, user_id,
						param.qq, access_token, refresh_token, param.extra.refresh);
					this_bgm_user = progress_res.second;
					//user_progress = this_bgm_user.progress;
				}

				//用户的进度(有效)
				int user_had_finished = 0;
				if (this_bgm_user.progress.valid) {
					//说明此用户已经收藏此条目
					try {
						user_had_finished = std::stoi(this_bgm_user.progress.progress);
					}
					catch (std::invalid_argument&) {
						//有问题,无法转换为数字
						//直接跳过
					}
				}
				else {
					//该用户没有收藏条目
					//if (this_bgm_user.user_id != 0) {
					//	//说明存在用户的缓存
					//}
					//else {
					//	//不存在用户的缓存
					//}
					//
					try {
						//Collection API 同步
						//
						bangumi::string content1;
						content1 << "status=do"
							<< "&rating="
							<< "&comment=";

						//post中的www编码其实是为了防止URI过长而将参数放在了content中,POST时一定不能忘记使用Content-Length指定大小
						//否则会出现不能正常读取参数的问题
						bangumi::string header1("Content-Type: application/x-www-form-urlencoded\r\n");
						header1 << "Authorization: Bearer " << access_token << "\r\n";
						header1 << "Content-Length: " << content1.length() << "\r\n";

						bangumi::string uri1;
						uri1 << "/collection/"
							<< subject_id
							<< "/update";
#ifndef NDEBUG
						{
							bangumi::string debug_msg;
							debug_msg << "Collection:"
								>> "header = " << header1
								>> "content = " << content1
								>> "uri = " << uri1;

							CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Func-Collection-API", debug_msg);
						}
#endif	


						//post中的www编码其实是为了防止URI过长而将参数放在了content中,POST时一定不能忘记使用Content-Length指定大小
						//否则会出现不能正常读取参数的问题
						bangumi::string request1;
						request1 << "POST " << uri1 << " HTTP/1.1\r\n"
							<< "Host: api.bgm.tv\r\n"
							<< header1 << "\r\n"
							<< content1;

						std::string json1 = http_client.SyncBGMHTTPRequest(request1);

#ifndef NDEBUG
						{
							bangumi::string debug_msg;
							debug_msg << "收到响应的Json"
								>> json1.c_str();
							CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Func-RefreshToken", debug_msg);
						}
#endif

						//解析json
						auto resolve_result = Resolve::Resolve_Collect(json1, param.extra.refresh);

						//加上Subject的总集数
						//由于此API固定返回为进度0,因此忽略此信息
						//resolve_result.second.progress.AddEps(subject_data.GetEpsAiredCount());

						//同时为User和UserProgress更新
						this_bgm_user = resolve_result.second;
						//user_progress = this_bgm_user.progress;

						//结束的等待
						for (auto &t : resolve_result.first) {
							if (t != nullptr&&t->joinable())
								t->join();
						}



					}
					catch (boost::system::system_error&e) {
						//发生了错误
#ifndef NDEBUG
						{
							bangumi::string debug_msg;
							debug_msg << "在Update中同步请求Collection API失败"
								>> e.what();
							CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-API-Update-Collect", debug_msg);
						}
#endif
						bangumi::string error_msg;
						error_msg << e.what();
						//发送回复
						DEFAULT_SEND(param.type, error_msg);
						//
						continue;
					}
				}
				//此时保证了用户收藏了条目
				//并且得到了有效的User对象及其Progress成员
				//用户自身的进度
				int &user_self_watched_eps = user_had_finished;
				//将要更新的进度
				int to_update_eps = 0;
				//=========计算进度==========
				if (complex_param.update_air) {
					//使用air
					to_update_eps = subject_data.GetEpsAiredCount();
				}
				else if (complex_param.update_fin) {
					//使用eps总集 不包括SP
					to_update_eps = subject_data.GetEpsCount();
				}
				else if (complex_param.update_watched_eps != 0) {

					//使用内置的进度参数
					to_update_eps = complex_param.update_watched_eps;
				}
				else {
					//使用用户的自己的进度+1
					to_update_eps = user_self_watched_eps + 1;
					//为了与常识相合
					if (complex_param.update_eps_shift != 0)
						--complex_param.update_eps_shift;
				}
				//最后加上偏移
				to_update_eps += complex_param.update_eps_shift;
				//最后再限制范围
				if (to_update_eps > subject_data.GetAllEpsCount()) {
					to_update_eps = subject_data.GetAllEpsCount();
				}
				if (to_update_eps < 0) {
					to_update_eps = 0;
				}

				//=========计算进度Over==========
				//判断如果进度已经超过了TV的总EPS则直接收藏为看完
				if (to_update_eps >= subject_data.GetEpsCount() && subject_data.GetEpsCount() != 0) {
					//使用CollectionAPI收藏为看完
					try {
						//Collection API 同步
						//
						bangumi::string content1;
						content1 << "status=collect"
							<< "&rating=" << complex_param.collection_rating
							<< "&comment=" << code_encoder.Conv(complex_param.collection_comment);

						//post中的www编码其实是为了防止URI过长而将参数放在了content中,POST时一定不能忘记使用Content-Length指定大小
						//否则会出现不能正常读取参数的问题
						bangumi::string header1("Content-Type: application/x-www-form-urlencoded\r\n");
						header1 << "Authorization: Bearer " << access_token << "\r\n";
						header1 << "Content-Length: " << content1.length() << "\r\n";

						bangumi::string uri1;
						uri1 << "/collection/"
							<< subject_id
							<< "/update";
#ifndef NDEBUG
						{
							bangumi::string debug_msg;
							debug_msg << "Collection:"
								>> "header = " << header1
								>> "content = " << content1
								>> "uri = " << uri1;

							CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Func-Collection-API", debug_msg);
						}
#endif	


						//post中的www编码其实是为了防止URI过长而将参数放在了content中,POST时一定不能忘记使用Content-Length指定大小
						//否则会出现不能正常读取参数的问题
						bangumi::string request1;
						request1 << "POST " << uri1 << " HTTP/1.1\r\n"
							<< "Host: api.bgm.tv\r\n"
							<< header1 << "\r\n"
							<< content1;

						std::string json1 = http_client.SyncBGMHTTPRequest(request1);

#ifndef NDEBUG
						{
							bangumi::string debug_msg;
							debug_msg << "收到响应的Json"
								>> json1.c_str();
							CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Func-RefreshToken", debug_msg);
						}
#endif

						//解析json
						auto resolve_result = Resolve::Resolve_Collect(json1, param.extra.refresh);

						//加上Subject的总集数
						//由于此API固定返回为进度0,因此忽略此信息
						//resolve_result.second.progress.AddEps(subject_data.GetEpsAiredCount());

						//同时为User和UserProgress更新
						this_bgm_user = resolve_result.second;
						//user_progress = this_bgm_user.progress;

						//结束的等待
						for (auto &t : resolve_result.first) {
							if (t != nullptr&&t->joinable())
								t->join();
						}



					}
					catch (boost::system::system_error&e) {
						//发生了错误
#ifndef NDEBUG
						{
							bangumi::string debug_msg;
							debug_msg << "在Update中同步请求Collection API失败"
								>> e.what();
							CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-API-Update-Collect", debug_msg);
						}
#endif
						bangumi::string error_msg;
						error_msg << e.what();
						//发送回复
						DEFAULT_SEND(param.type, error_msg);
						//
						continue;
					}
				}
				//==============================


				//同时更新到User对象中的进度对象中
				this_bgm_user.progress.progress = std::to_string(to_update_eps) + '/';
				this_bgm_user.progress.AddEps(subject_data.GetEpsCount());
				subject_data.SetCurrentEps(to_update_eps);
				this_bgm_user.progress.SetExStr(subject_data.GetExStr(user_self_watched_eps));
				//最后可以得出成功后的回复
				bangumi::string success_msg;
				bangumi::string extra_msg;
				success_msg << subject_data.Get()
					<< this_bgm_user.UpdateGet();
				extra_msg << subject_data.Get();

				//只能请求API
				bangumi::string content;
				content << "watched_eps=" << to_update_eps
					<< "&watched_vols=" << to_update_eps;


				//post中的www编码其实是为了防止URI过长而将参数放在了content中,POST时一定不能忘记使用Content-Length指定大小
				//否则会出现不能正常读取参数的问题
				bangumi::string header("Content-Type: application/x-www-form-urlencoded\r\n");
				header << "Authorization: Bearer " << access_token << "\r\n";
				header << "Content-Length: " << content.length() << "\r\n";

				bangumi::string uri;
				uri << "/subject/"
					<< subject_id
					<< "/update/watched_eps";
#ifndef NDEBUG
				{
					bangumi::string debug_msg;
					debug_msg << "Update:"
						>> "header = " << header
						>> "content = " << content
						>> "uri = " << uri;

					CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Func-Update-API", debug_msg);
				}
#endif	
				//创建一个请求
				std::shared_ptr<HTTPRequest> request_one =
					http_client.create_request_fixed(http_client.GetID());

				//构造一个bangumi::BGMRetParam
				//[注意]这里需要给BGMRetParam额外的信息
				bangumi::BGMRetParam bgm_param{ param,subject_id,"" };
				//先给complex_param中的bangumi_html_data赋值
				//complex_param.subject_html_data = subject_data;
				//因为Update中的异步操作需要一些complex_param中的参数bangumi_html_data
				bgm_param.success_msg = success_msg;
				bgm_param.extra_msg = extra_msg;
				request_one->set_ret_param(bgm_param);
				request_one->set_host("api.bgm.tv");
				request_one->set_uri(uri);
				//只有搜索条目时需要Cookie: chii_searchDateLine
				request_one->set_request(request_message(request_one, HTTP_WAY::POST,
					header,
					content));
				//设置回调函数
				request_one->set_callback([](std::shared_ptr<HTTPRequest>request_one, bangumi::BGMRetParam param, int ec) {
					//要回复的string
					bangumi::string msg;
					//复杂回复结构体
					auto& complex_param = param.complex_param;
					//Bangumi Subject Html Data and success msg
					bangumi::string& extra_msg = param.extra_msg;
					bangumi::string& success_msg = param.success_msg;

					//回调函数:此时已经完成了响应报文的读取
					std::string json;
					try {
						GetResponseContent(request_one, json, true, ec);
#ifndef NDEBUG
						{
							bangumi::string debug_msg;
							debug_msg << "收到响应的Json"
								>> json.c_str();
							CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTP", debug_msg);
						}
#endif

						//======API Start======


						//从HTML解析需要的信息: 作品名,总集数,图片,当前放送的话数等
						//改到了requst创建之前进行
						//std::string html = bangumi::GetSubjectHtml(param.cur_id);
						//BangumiSubjectCollection subject_data = Resolve::ResolveSubjectCollection(html, param.cur_id, param.extra.refresh);
						//if (!subject_data.Valid()) {
						//	//说明 这个条目是不对非会员开放的
						//	throw boost::system::system_error(bangumi_bot_errors::maybe_301_maybe_limit);
						//}

						//解析json

						auto resolve_result = Resolve::Resolve_Update(json, param.extra.refresh);
						if (resolve_result) {
							//说明更新进度成功
							//返回的消息加上条目的html解析部分信息
							msg << success_msg;
						}
						else {
							//说明更新进度失败:目前已知失败的原因只有用户没有收藏和502,不过没有收藏是不可能的,只能是502或更新了相同的进度

							throw boost::system::system_error(bangumi_bot_errors::update_with_the_same_eps);
						}
						//加上Subject的总集数
						//由于此API固定返回为进度0,因此忽略此信息
						//resolve_result.second.progress.AddEps(subject_data.GetEpsAiredCount());


						//结束的等待
						//for (auto &t : resolve_result.first) {
						//	if (t != nullptr&&t->joinable())
						//		t->join();
						//}



						//======API Over======
					}
					catch (boost::system::system_error & e) {
						//如果发生了异常,使用异常回复(包括没有找到用户等)
						//这里不使用Reply类而是直接赋值回复
						msg << extra_msg << "\n\n";
						msg << e.what();
						//msg << "[" << param.cur_str << "]...";
					}

					//发送回复
					DEFAULT_SEND(param.type, msg);

					//从httpclient移除引用从而析构自身

					http_client.RemoveID(request_one->get_id());
				});
				request_one->execute();

				//======Main Over======
			}
			catch (boost::system::system_error& e) {
				//发送回复
				DEFAULT_SEND(param.type, e.what());
				//
				continue;
			}


		}

		//结束之后更新last_subject
		if (last_subject_id != 0) {
			//简单一点直接单线程
			//查询语句
			bangumi::string update_query;

			update_query << "UPDATE bgm_users SET "
				//<< "user_bangumi=" << auth.user_id << ","
				<< "user_last_searched=" << last_subject_id
				<< " WHERE user_qq=" << param.qq;
			auto affect_rows_num = sql_pool.ExecQueryNoRes(update_query);
			try
			{
				SQLCheckResult(affect_rows_num);
			}
			catch (boost::system::system_error& e)
			{
#ifndef NDEBUG
				{
					bangumi::string debug_msg;
					debug_msg << "Update Last Subject失败:"
						<< std::to_string(affect_rows_num)
						>> e.what();

					CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Func-Subject-API", debug_msg);
				}
#endif	
			}
		}

	}
	//API: 搜索API
	inline void BGM_API_Search(const BGMCodeParam & param, const std::set<size_t>& parameters_id, const std::set<std::string>& parameters_str) {
		//参数是一个STR, Search只接受STR参数, 和User一样, 只解析/附加参数,
		//例如: Angel+Beats-ova(+表示空格-表示排除)/2(类型)/5(数量)/0(从第几个开始)
		//先搜索SQL,否则使用API的Large,同时Save到SQL中

		//https://api.bgm.tv/search/subject/%E6%9C%88%E5%BD%B1?responseGroup=large&start=0&max_results=6
		//首先将字符转UTF-8搜索
		//排除空语句
		if (parameters_str.empty() && parameters_id.empty())
			return;
		//bangumi::string ret("<Search>");
		//ret >> "ID:\n";
		//for (const auto& i : parameters_id)
		//	ret << i << " ";
		//ret >> "STR:\n";
		//for (const auto& i : parameters_str)
		//	ret << i << " ";

		//DEFAULT_SEND(param.type, ret);

		//构建一个复杂参数类的Vector
		std::vector<ComplexParam> paramters;
		//压入解析参数(只有STR参数)
		for (const auto& i : parameters_str) {
			try {
				auto temp = ResolveSearchPara(i);
				//压入paramters
				paramters.emplace_back(temp);
			}
			catch (boost::system::system_error&) {
				//因为str可以不使用complex_param
				//因此str怎样都是一个合法的参数
				//因此直接构造一个默认的
				paramters.emplace_back(i);
			}

		}

	
		//对每一个识别的ID进行main func
		for (const auto& complex_param : paramters) {
			const auto& subject_str = complex_param.str;
			//Main Func
			//使用BGM API
			try {
				std::shared_ptr<HTTPRequest> request_one =
					http_client.create_request_fixed(http_client.GetID());

				request_one->set_ret_param(bangumi::BGMRetParam{ param,0,subject_str });
				request_one->set_host("api.bgm.tv");
				//注意转换为HTML 的 URLENCODE 实际就是utf- 8
				//根据复杂参数结构体增加参数
				bangumi::string extra_search_param;
				extra_search_param <<"?type="<< complex_param.search_type
					<< "&responseGroup=large"
					<< "&start=" << complex_param.search_start_pos
					<< "&max_results=" << complex_param.search_max_num;
				request_one->set_uri("/search/subject/" + code_encoder.Conv(subject_str) + extra_search_param);
				//只有搜索条目时需要Cookie: chii_searchDateLine, 也为防止限制搜索
				request_one->set_request(request_message(request_one, HTTP_WAY::GET, "Cookie: chii_searchDateLine=0;\r\n", ""));
				//设置回调函数
				request_one->set_callback([](std::shared_ptr<HTTPRequest>request_one, bangumi::BGMRetParam param, int ec) {
					//要回复的string
					bangumi::string msg;
					//复杂回复结构体
					auto& complex_param = param.complex_param;
					//回调函数:此时已经完成了响应报文的读取
					std::string json;
					try {
						GetResponseContent(request_one, json, true, ec);
#ifndef NDEBUG
						{
							bangumi::string debug_msg;
							debug_msg << "收到响应的Json"
								>> json.c_str();
							CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTP", debug_msg);
						}
#endif

						auto resolve_res = Resolve::Resolve_Search(json, param.extra.refresh);
						std::vector<std::shared_ptr<boost::thread>> pic_threads;

						//图片的线程
						pic_threads = resolve_res.first;
						//返回的Search结果
						msg << resolve_res.second;
						
						//结束的等待
						for (auto &t : pic_threads) {
							if (t != nullptr&&t->joinable())
								t->join();
						}

					}
					catch (boost::system::system_error & e) {
						//如果发生了异常,使用异常回复(包括没有找到用户等)
						//这里不使用Reply类而是直接赋值回复
						msg = e.what();
						msg << "[" << param.cur_str << "]";
					}
#ifndef NDEBUG
					{
						bangumi::string debug_msg;
						debug_msg << msg;
						CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-MSG", debug_msg);
					}
#endif
					//发送回复
					DEFAULT_SEND(param.type, msg);
					//从httpclient移除引用从而析构自身

					http_client.RemoveID(request_one->get_id());
				});
				request_one->execute();
		}
			catch (boost::system::system_error& e) {
#ifndef NDEBUG
				{
					bangumi::string debug_msg;
					debug_msg << "FUNC-API-Subject Request Error"
						>> e.what();
					CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTP-API-Subject", debug_msg);
				}
#endif
			}
			}
		}
	//API: 返回Tag信息
	inline void BGM_API_Tag(const BGMCodeParam & param, const std::set<size_t>& parameters_id, const std::set<std::string>& parameters_str) {

		//bangumi::string ret("<Tag>");
		//ret >> "ID:\n";
		//for (const auto &i : parameters_id)
		//	ret << i << " ";
		//ret >> "STR:\n";
		//std::string name;
		//for (const auto &i : parameters_str)
		//	ret << i << " ";

		//DEFAULT_SEND(param.type, ret);

		//排除空语句
		if (parameters_str.empty() && parameters_id.empty()) {
			bangumi::string error;
			error << "缺少参数...";
			//回复消息
			DEFAULT_SEND(param.type, error);
			return;
		}
			
			

		//构建一个复杂参数类的Vector
		std::vector<ComplexParam> paramters;
		//压入解析参数(只有STR参数)
		for (const auto& i : parameters_str) {
			try {
				auto temp = ResolveTagPara(i);
				//压入paramters
				paramters.emplace_back(temp);
			}
			catch (boost::system::system_error&) {
				//因为str可以不使用complex_param
				//因此str怎样都是一个合法的参数
				//因此直接构造一个默认的
				bangumi::ComplexParam temp;
				temp.tag_keyword = i;
				paramters.emplace_back(temp);
			}

		}


		//http://bgm.tv/anime/tag/%E6%90%9E%E7%AC%91/?sort=date
		//http://bgm.tv/anime/browser/tv/airtime/2019-4
		//http://bgm.tv/anime/tag/%E6%97%A5%E5%B8%B8/airtime/2019-4

		//示例参数 热血/2019-4 表示2019-4月中的标签有热血的番组
		//如果uri中年月格式错误则会默认全部
		//如果仅仅有tag没有指定月份则使用日期排序只取前两页面
		//首先下载html
		//对每一个识别的ID进行main func
		for (const auto& complex_param : paramters) {

			//根据信息构造uri
			bangumi::string uri2;
			
			//关键字为空
			//后来发现只有%会有问题，造成asio的read_until不能正常进行
			if (complex_param.tag_keyword.empty()|| complex_param.tag_keyword=="%"){
				//直接连接airtime
				uri2 << "browser/tv/airtime/"
					<< complex_param.tag_airtime;
			}
			else {
				//关键字不为空
				uri2 << "tag/";
				uri2 << code_encoder.Conv(complex_param.tag_keyword);
				//连接airtime
				uri2 << "/airtime/"
					<< complex_param.tag_airtime;
			}
			//如果airtime为空
			if (complex_param.tag_airtime.empty()){
				//则需要按日期排序
				uri2 << "?sort=date";
				//如果page有效
				if (complex_param.tag_page != 0) {
					uri2 << "&page="<< complex_param.tag_page;
				}
			}
			else {
				//如果page有效
				if (complex_param.tag_page != 0) {
					uri2 << "?page=" << complex_param.tag_page;
				}
			}
#ifndef NDEBUG
			{
				bangumi::string debug_msg;
				debug_msg << "请求的URI为: " << uri2;
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-API-Tag", debug_msg);
			}
#endif
			try {
				//Main Func
				bangumi::string uri;
				uri << "/anime/" << uri2;

				std::string request = "GET " + uri + " HTTP/1.1\r\n"
					"Host: " "bgm.tv" "\r\n" "\r\n";

				//请求html
				std::string html = code_converter.Conv(http_client.SyncBGMHTTPRequest(request));

				//解析html
				bangumi::string ret = Resolve::ResolveTag(html, param.extra.refresh);

				//失败消息
				if (ret.empty()) {
					ret << "未查找到相关条目...";
				}

				//回复消息
				DEFAULT_SEND(param.type, ret);
			}
			catch (boost::system::system_error& e) {
				//回复消息
				DEFAULT_SEND(param.type, e.what());
				continue;
			}

		}
	}

	//RSS
	inline void BGM_RSS(const BGMCodeParam & param, const std::set<size_t>& parameters_id, const std::set<std::string>& parameters_str, BgmCode rss_type) {
		//只取HTML的放送状态即可（有名字与原名），以及ep
		//例子:dmhy 一拳超人+6/5  (以“一拳超人 6”为关键字，最新结果5个)
		//例子:moe 


		//bangumi::string ret("<Tag>");
		//ret >> "ID:\n";
		//for (const auto &i : parameters_id)
		//	ret << i << " ";
		//ret >> "STR:\n";
		//std::string name;
		//for (const auto &i : parameters_str)
		//	ret << i << " ";

		//DEFAULT_SEND(param.type, ret);

		//构建一个复杂参数类的Vector
		std::vector<ComplexParam> paramters;
		//last_subject_id
		size_t sql_last_subject_id = 0;
		//排除空语句
		if (parameters_str.empty() && parameters_id.empty()) {
			//Last判断
			try {
				sql_last_subject_id = GetLastSubjectID(param.qq);
				if (sql_last_subject_id != 0) {
					paramters.emplace_back(sql_last_subject_id);
				}
				else {
					return;
				}
			}
			catch (boost::system::system_error& e) {
				//LastSubjectGet失败
#ifndef NDEBUG
				{
					bangumi::string debug_msg;
					debug_msg << "GetLastSubjectFailed!"
						>> e.what();
					CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Func-RSS-API", debug_msg);
				}
#endif	
				return;
			}
		}
		//检查权限
		bangumi::string query;
		query << "SELECT dmhy_open "
			<< "FROM bgm_users "
			<< "WHERE user_qq="
			<< param.qq;
		//查询结果
		BGMSQLResult result;
		//影响的行数
		unsigned long affect_rows_num = sql_pool.ExecQuery(query, result);
		try
		{
			SQLCheckResult(affect_rows_num);
		}
		catch (boost::system::system_error& e)
		{
#ifndef NDEBUG
			{
				bangumi::string debug_msg;
				debug_msg << "SQL查询失败:"
					<< std::to_string(affect_rows_num)
					>> e.what();

				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-BOT-Statis", debug_msg);
			}
#endif	
			return;
		}
		if (affect_rows_num > 0) {
			//检查权限
			if (result[0][0] == '0')
			{
				//发送回复
				DEFAULT_SEND(param.type, "请尝试@我或喊[\"BGM娘\"/\"bgm娘\"]来接受我的考核~");
				return;
			}
			else if (result[0][0] == '9') {
				//什么也不做
				//进行后续的处理
			}
			else {
				//发送回复
				DEFAULT_SEND(param.type, "考核仍在进行中~");
				return;
			}
		}
		else
		{
			return;
		}
		//压入解析参数(id)
		for (const auto& i : parameters_id) {
			paramters.emplace_back(i);
		}
		//判断是否已经sql查询过last_subject了
		bool has_sql_last_subject = false;
		//压入解析参数(str)
		for (const auto& i : parameters_str) {
			try {
				auto temp = ResolveRSSPara(i);
				//先判断是否使用Last Subject
				if (temp.use_last_subject_id) {
					if (!has_sql_last_subject) {
						//标记已经sql过了last Subject
						has_sql_last_subject = true;
						//sql last subject
						sql_last_subject_id = GetLastSubjectID(param.qq);
					}

					temp.id = sql_last_subject_id;
				}
				//压入paramters
				paramters.emplace_back(temp);
			}
			catch (boost::system::system_error&) {
;
			}
		}

		//==========Main Func=============
		//对每一个识别的ID进行main func
		for (auto& complex_param : paramters) {
			//基础的常用赋值
			const auto& subject_id = complex_param.id;
			//无效的ID
			if (subject_id == 0&& complex_param.rss_keyword.empty()) {
				bangumi::string error_msg = boost::system::system_error(bangumi_bot_errors::empty_subject).what();
				error_msg << "[" << subject_id << "]";
				//发送回复
				DEFAULT_SEND(param.type, error_msg);
				//continue
				continue;
			}



			//使用的RSS源
			bangumi::string rss_host;
			bangumi::string rss_uri;
			//要回复的消息前缀
			bangumi::string pre_ret;

			switch (rss_type)
			{
			case BgmCode::MOE:
			{
				rss_host << "bangumi.moe";
				rss_uri << "/rss/search/";
				pre_ret << "资源来源: [ 萌番组 ]"
					"\n关键字: ";
			}
				break;
			case BgmCode::DMHY:
			{
				rss_host << "share.dmhy.org";
				rss_uri << "/topics/rss/rss.xml?keyword=";
				pre_ret << "资源来源: [ 动漫花园 ]"
					"\n关键字: ";
			}
				break;
			default:
				break;
			}
			//https://share.dmhy.org/topics/rss/rss.xml
			//https://share.dmhy.org/topics/rss/rss.xml?keyword=123
			//https://bangumi.moe/rss/search/123
			//首先使用单线程下载页面


			try
			{
				bangumi::string keyword;
				//如果需要html的放送状态请求
				if (complex_param.rss_keyword.empty()) {
					//从HTML解析需要的信息: 作品名,总集数,图片,当前放送的话数等
					std::string html = bangumi::GetSubjectHtml(subject_id);
					BangumiSubjectCollection subject_data = Resolve::ResolveSubjectCollection(html, subject_id, param.extra.refresh);
					if (!subject_data.Valid()) {
						//说明 这个条目是不对非会员开放的
						throw boost::system::system_error(bangumi_bot_errors::maybe_301_maybe_limit);
					}

					//将要更新的进度
					float to_rss_eps = 0;
					//=========计算进度==========
					if (complex_param.update_air) {
						if (subject_data.GetEpsAiredCount() != 0) {
							//已放送的
							try {
								//取ep.XX
								to_rss_eps = std::stof(subject_data.air_eps[subject_data.GetEpsAiredCount() - 1].substr(3));
							}
							catch (std::exception&e) {
								//使用eps总集 不包括SP
								to_rss_eps = subject_data.GetEpsAiredCount();
							}
						}
					}
					else if (complex_param.update_fin) {

						if (subject_data.GetEpsUnAiredCount() != 0) {
							//说明有未放送的
							try {
								//取ep.XX
								to_rss_eps = std::stof(subject_data.unair_eps[subject_data.GetEpsUnAiredCount() - 1].substr(3));
							}
							catch (std::exception&e) {
								//使用eps总集 不包括SP
								to_rss_eps = subject_data.GetEpsCount();
							}
						}
						else {
							//说明已经放送完毕
							try {
								//取ep.XX
								to_rss_eps = std::stof(subject_data.air_eps[subject_data.GetEpsAiredCount() - 1].substr(3));
							}
							catch (std::exception&e) {
								//使用eps总集 不包括SP
								to_rss_eps = subject_data.GetEpsCount();
							}
						}

					}
					else if (complex_param.update_watched_eps != 0) {

						//使用内置的进度参数
						to_rss_eps = complex_param.update_watched_eps;
					}
					else {
						//否则什么也不加

					}
					//最后加上偏移
					to_rss_eps = (int)(to_rss_eps + .6) + complex_param.update_eps_shift;

					//给uri赋值
					if (!complex_param.rss_keyword.empty()) {
						//如果这个关键字不为空就直接使用这个
						keyword << complex_param.rss_keyword;
					}
					else {
						//否则使用html解析后的name
						if (!(subject_data.name_cn.empty())) {
							std::string temp(subject_data.name_cn);
							////转换空格到+
							//size_t blank_site = temp.find_first_of(' ');
							//while (blank_site!=std::string::npos)
							//{
							//	temp[blank_site] = '+';
							//	blank_site = temp.find_first_of(' ', blank_site);
							//}
							//直接请求空格以前的关键字
							size_t blank_site = temp.find_first_of(' ');
							if (blank_site != std::string::npos)
							{
								temp.erase(blank_site);
							}
							//只适用于中文的处理
							std::string temp_(temp);
							if (temp_.size() > 4 * 2) {
								//4个字以上
								temp = "";
								//取前4个字
								for (int i = 0; i < 8; ++i) {
									if (temp_[i] < 0)
									{
										temp += temp_[i];
										continue;
									}
									else {
										break;
									}
								}
								//可能出现其他问题就还原
								if (temp.empty())
								{
									temp = temp_;
								}

							}
							keyword << temp;
						}
						else {
							std::string temp(subject_data.name);
							//转换空格到+
							//size_t blank_site = temp.find_first_of(' ');
							//while (blank_site != std::string::npos)
							//{
							//	temp[blank_site] = '+';
							//	blank_site = temp.find_first_of(' ', blank_site);
							//}
							//直接请求空格以前的关键字
							size_t blank_site = temp.find_first_of(' ');
							if (blank_site != std::string::npos)
							{
								temp.erase(blank_site);
							}
							keyword << temp;
						}

					}
					//之后加上话数
					if ((int)to_rss_eps != 0) {

						keyword << '+' << (int)(to_rss_eps);
						//dmhy的搜索问题使用｜增加关键字
						if (to_rss_eps < 10)
						{
							keyword << "|0" << std::to_string((int)(to_rss_eps));
						}
					}
				}
				else {
					int to_rss_eps = 0;
					if (complex_param.update_watched_eps != 0) {

						//使用内置的进度参数
						to_rss_eps = complex_param.update_watched_eps;
					}
					else {
						//否则什么也不加

					}
					//最后加上偏移
					to_rss_eps = (int)(to_rss_eps + .6) + complex_param.update_eps_shift;

					//给uri赋值
					keyword << complex_param.rss_keyword;
					//之后加上话数
					if (to_rss_eps != 0) {
						keyword << '+' << std::to_string(to_rss_eps);
						//dmhy的搜索问题使用｜增加关键字
						if (to_rss_eps<10)
						{
							keyword << "|0" << std::to_string(to_rss_eps);
						}
					}
						
				}

				//最大的条目数
				int max_items = complex_param.rss_max_items;
				//替换+到空格
				size_t plus_site = keyword.find_first_of('+');
				while (plus_site!=std::string::npos)
				{
					keyword[plus_site] = ' ';
					plus_site = keyword.find_first_of('+', plus_site+1);
				}
				rss_uri << code_encoder.Conv(keyword);
				//前缀消息
				pre_ret << "[ " << keyword << " ]\n\n";

#ifndef NDEBUG
				{
					bangumi::string debug_msg;
					debug_msg << "请求的RSS为: " << rss_host << rss_uri;
					CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-RSS", debug_msg);
				}
#endif

#ifndef NDEBUG
				{
					bangumi::string debug_msg;
					debug_msg << "请求的rss_html为: "
						>> "uri" << url_encode(rss_uri);
					CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-RSS", debug_msg);
				}
#endif
				//读取HTML页面
				std::string rss_html = GetHttpsHtml(rss_host, url_encode(rss_uri));
#ifndef NDEBUG
				{
					bangumi::string debug_msg;
					debug_msg << "请求的rss_html为: " << rss_html.size()
						>> "uri" << code_encoder.Conv(rss_uri);
					CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-RSS", debug_msg);
				}
#endif
				//解析HTML
				auto ret_vec = Resolve::ResolveRSS(rss_html,rss_type, complex_param.rss_max_items, pre_ret, param.extra.refresh);
				//消息数组
				for (auto& ret : ret_vec){
					//回复消息
					DEFAULT_SEND(param.type, ret);
				}
				
			}
			catch (const boost::system::system_error&e)
			{
				//回复消息
				DEFAULT_SEND(param.type, e.what());
				//continue;
			}
		
		}
		





		
	}

	//API: 返回时光机信息
	inline void BGM_TML(const BGMCodeParam & param, const std::set<size_t>& parameters_id, const std::set<std::string>& parameters_str) {

		//bangumi::string ret("<User>");
		//ret >> "ID:\n";
		//for (const auto &i : parameters_id)
		//	ret << i << " ";
		//ret >> "STR:\n";
		//std::string name;
		//for (const auto &i : parameters_str)
		//	ret << i << " ";

		//DEFAULT_SEND(param.type, ret);
		if (parameters_str.empty())
		{
			try {
				//说明是全局的时光机
				std::string html = bangumi::GetTimeLine();
				bangumi::string time_str = Resolve::ResolveTimeLine(html);
				//发送回复
				DEFAULT_SEND(param.type, time_str);
			}
			catch (boost::system::system_error&) {
				//发送回复
				DEFAULT_SEND(param.type, "发生了点问题...");
			}
			//直接返回
			return;
		}
		//对每一个识别的ID进行main func
		for (const auto& name : parameters_str) {
			//====
			std::string bangumi_name;
			if (name == "#")
			{
				auto verify_result = VerifyToken(param);
				size_t &user_id = verify_result.first;
				bangumi_name = std::to_string(user_id);
			}
			else {
				bangumi_name = name;
			}
			//=====
			try {
				std::string html = bangumi::GetUserTimeLine(bangumi_name);
				bangumi::string tml = Resolve::ResolveTimeLineRSS(html, bangumi_name);
				//发送回复
				DEFAULT_SEND(param.type, tml);
			}
			catch (boost::system::system_error&) {
				//发送回复
				DEFAULT_SEND(param.type, "发生了点问题...");
			}
		}

	}

	//API: 返回用户收藏
	inline void BGM_User_Collection(const BGMCodeParam & param, const std::set<size_t>& parameters_id, const std::set<std::string>& parameters_str) {

		//bangumi::string ret("<Tag>");
		//ret >> "ID:\n";
		//for (const auto &i : parameters_id)
		//	ret << i << " ";
		//ret >> "STR:\n";
		//std::string name;
		//for (const auto &i : parameters_str)
		//	ret << i << " ";

		//DEFAULT_SEND(param.type, ret);

		//排除空语句
		if (parameters_str.empty() && parameters_id.empty()) {
			bangumi::string error;
			error << "缺少参数...";
			//回复消息
			DEFAULT_SEND(param.type, error);
			return;
		}



		//构建一个复杂参数类的Vector
		std::vector<ComplexParam> paramters;
		//压入解析参数(只有STR参数)
		for (const auto& i : parameters_str) {
			try {
				auto temp = ResolveUserCollectionPara(i);
				//压入paramters
				paramters.emplace_back(temp);
			}
			catch (boost::system::system_error&) {
				//因为str可以不使用complex_param
				//因此str怎样都是一个合法的参数
				//因此直接构造一个默认的
				bangumi::ComplexParam temp;
				temp.bangumi_user = i;
				paramters.emplace_back(temp);
			}

		}


		//http://bgm.tv/anime/list/wz97315/collect
		//http://bgm.tv/game/list/wz97315/do?page=2
		//http://bgm.tv/game/list/wz97315/do?orderby=rate
		//http://bgm.tv/anime/list/wz97315/collect?tag=%E7%BB%9D%E8%B5%9E
		
		//{用户ID}/{条目类型}/{收藏状态}/{页码}/{排序方式}/{标签}

		//首先下载html
		//对每一个识别的ID进行main func
		for (const auto& complex_param : paramters) {
			//返回消息
			bangumi::string ret;

			//根据信息构造uri
			bangumi::string uri2;
			
			//条目类型
			uri2 << '/' << complex_param.ucollection_subject_type << "/list/";

			//用户名
			std::string bangumi_id;
			//是否使用自己绑定的bangumi id
			if (complex_param.use_last_subject_id || complex_param.bangumi_user == "#") {
				auto verify_result = VerifyToken(param);
				size_t &user_id = verify_result.first;
				bangumi_id = std::to_string(user_id);
			}
			else {
				bangumi_id = complex_param.bangumi_user;
			}
			//用户头像
			std::string & json = GetHtml("/user/" + bangumi_id, bgm.bangumi_api_url);
			//宽解析树
			boost::property_tree::ptree json_pt;
			//因为解析需要一个流输入,因此使用stringstream,也支持文件流读取
			std::istringstream json_input(json);
			//解析json
			boost::property_tree::read_json(json_input, json_pt);
			//
			size_t id = json_pt.get<size_t>("id", 0);
			bangumi_id = json_pt.get<std::string>("username", "");
			std::string url = json_pt.get<std::string>("avatar.large", "");
			std::string pic_name = std::to_string(id);
			//PIC图片下载线程
			std::vector<std::shared_ptr<boost::thread>> ThreadVector;
			//
			std::string pic_file_path;
			//下载图片
			if (!url.empty()) {
				//下载图片
				auto result = PicDownload(http_client, url, USER_PIC_PATH, pic_name, pic_file_path, param.extra.refresh);

				//返回的线程保存到返回值中
				if (result.first == DownloadStatus::MultiThread)
				{
					//将下载线程压入线程池中
					ThreadVector.push_back(result.second);
				}

				//保存图片消息
				ret << "[CQ:image,file=" << pic_file_path << "]";

			}

			//等待图片下载完成
			for (auto &t : ThreadVector) {
				if (t != nullptr&&t->joinable())
					t->join();
			}

			//加用户名
			uri2 << bangumi_id << '/';


			//收藏类型
			uri2 << complex_param.ucollection_co_type;

			//页码
			uri2 << "?page=" << complex_param.ucollection_page;

			//排序方式
			if (!complex_param.ucollection_order_type.empty())
			{
				uri2 << "&orderby=" << complex_param.ucollection_order_type;
			}

			//标签
			if (!complex_param.ucollection_tag.empty()) {
				uri2 << "&tag=" << code_encoder.Conv(complex_param.ucollection_tag);
			}


#ifndef NDEBUG
			{
				bangumi::string debug_msg;
				debug_msg << "请求的URI为: " << uri2;
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-User-Collection", debug_msg);
			}
#endif
			try {
				//Main Func

				std::string request = "GET " + uri2 + " HTTP/1.1\r\n"
					"Host: " "bgm.tv" "\r\n" "\r\n";

				//请求html
				std::string html = code_converter.Conv(http_client.SyncBGMHTTPRequest(request));

				//消息头构造
				ret >> '[' << bangumi_id << "] ";
				switch (complex_param.ucollection_co_type[0])
				{
				case 'w':
					ret << "想看/玩/听";
					break;
				case 'c':
					ret << "看/玩/听过";
					break;
				case 'd':
					if (complex_param.ucollection_co_type[1] == 'o')
						ret << "在看/玩/听";
					else
						ret << "抛弃";
					break;
				case 'o':
					ret << "搁置";
					break;
				default:
					break;
				}
				ret << " 的 ";
				switch (complex_param.ucollection_subject_type[0])
				{
				case 'a':
					ret << "[动画]";
					break;
				case 'b':
					ret << "[书]";
					break;
				case 'g':
					ret << "[游戏]";
					break;
				case 'm':
					ret << "[音乐]";
					break;
				case 'r':
					ret << "[三次元]";
					break;
				default:
					break;
				}

				ret >> "------";

				bangumi::string ret1(ret);
				//解析html
				auto& result = Resolve::ResolveUserCollection(html, param.extra.refresh);
				ret << '\n';
				ret << result.first;
				//回复消息
				DEFAULT_SEND(param.type, ret);
				if (!result.second.empty()) {
					ret1 << "[续]\n";
					ret1 << result.second;
					//回复消息
					DEFAULT_SEND(param.type, ret1);
				}
					

			}
			catch (boost::system::system_error& e) {
				//回复消息
				DEFAULT_SEND(param.type, e.what());
				continue;
			}

		}
	}

}
#endif
