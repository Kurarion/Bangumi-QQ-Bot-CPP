//#include "Http.h"
//#include "Database.h"
#include "Init.h"
#include "Resolve.h"
#include "HttpServer.h"
#include "BaseDataStruct.h"
#include"boost/format.hpp"

#ifndef BANGUMI_FUNCTION_H
#define BANGUMI_FUNCTION_H
//Ĭ�ϴ����
//һ���������ܷ��Ͷ����Ϣ,�ں꺯����ʹ��{}���޶����ɽ�������ض��������
//���ڵ�ǰ�����Ϣ����������,�ݲ��޸�
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
//�������ܺ���
namespace bangumi {
	bangumi::AuthReply RefreshToken(std::string refresh_token, int64_t qq) {
		//����Refresh
		bangumi::string content1;
		content1 << "grant_type=refresh_token"
			<< "&refresh_token=" << refresh_token
			<< "&redirect_uri=" << bgm.redirect_url
			<< "&client_id=" << bgm.bangumi_client_id
			<< "&client_secret=" << bgm.bangumi_client_secret;

		//post�е�www������ʵ��Ϊ�˷�ֹURI������������������content��,POSTʱһ����������ʹ��Content-Lengthָ����С
		//�������ֲ���������ȡ����������
		bangumi::string header1("Cache-Control: no-cache\r\n"
			"Content-Type: application/x-www-form-urlencoded\r\n");
		header1 << "Content-Length: " << content1.length() << "\r\n";

		std::string request1 = "POST "  "/oauth/access_token"  " HTTP/1.1\r\n"
			"Host: " "bgm.tv" "\r\n" + header1 + "\r\n" + content1;

		std::string json1 = http_client.SyncBGMHTTPRequest(request1);

#ifndef NDEBUG
		{
			bangumi::string debug_msg;
			debug_msg << "�յ���Ӧ��Json"
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

		//������ԵĻ�,����һ���߳�ȥִ��
		//����һ���µĽ���
		if (bgm.CheckThreadSize()) {
			//�п��п��õĽ���
			std::shared_ptr<boost::thread> sql_thread
			(new boost::thread([update_query]() {
#ifndef NDEBUG
				{
					std::ostringstream oss;
					oss << boost::this_thread::get_id();
					std::string idAsString = oss.str();
					std::string test = "�����µ��߳� ID: " + idAsString +
						"\n�ܳش�С: " + std::to_string(bgm.threadpool_size) + "\n"
						"���ô�С: " + std::to_string(bgm.curr_thread_size);
					CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Subject-SQL-MultiThread", test.c_str());
				}
#endif

				//�˴�����SQL����
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
						debug_msg << "VerifyTokenʧ��:"
							<< std::to_string(affect_rows_num)
							>> e.what();

						CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Func-VerifyToken", debug_msg);
					}
#endif	
					//���ﲻ����
					//ͨ���쳣��������
					//throw e;
				}

				bgm.AddAVAThreadSize();
#ifndef NDEBUG
				{
					bangumi::string debug_str;
					debug_str << "VerifyToken�߳����";
					CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Func-VerifyToken", debug_str);
				}
#endif
			}));
			//ֱ�ӷ����߳�
			sql_thread->detach();
		}
		else {
			//û�п��õ��߳�,ͬ��ִ��
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
					debug_msg << "VerifyTokenʧ��:"
						<< std::to_string(affect_rows_num)
						>> e.what();

					CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Func-VerifyToken", debug_msg);
				}
#endif	
				//���ﲻ����
				//ͨ���쳣��������
				//throw e;
			}
		}

		return auth;
	}

	std::pair<size_t,std::pair<std::string,std::string>> VerifyToken(const BGMRetParam &param,bool focus_verify = false) {
		//��ѯ���
		bangumi::string query;
		//SELECT * FROM bgm_subjects WHERE subject_id=1
		query << "SELECT user_bangumi,user_access_token,user_refresh_token FROM bgm_users WHERE user_qq="
			<< param.qq;
		//��ѯ���
		BGMSQLResult result;
		//Ӱ�������
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
				debug_msg << "SQLVerifyTkoken��ѯʧ��:"
					<< std::to_string(affect_rows_num)
					>> e.what();

				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-VerifyToken", debug_msg);
			}
#endif	
			//ͨ���쳣��������
			throw e;
		}
		if (affect_rows_num > 0) {
			//���ݿ��д洢��user_id
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
			//���������Ϊ0,�����
			//���access token����Ч��
			//����ʹ��refresh
			//.....�Ƿ�ǿ��RefreshToken
			if (focus_verify) {

				//��ʱ�벻��������Ҫ,���ݴ�
//				if (false) {
//					//==�����Ч��
//					//xxx����
//					bangumi::string content;
//					content << "access_token=" << result[1];
//
//					//post�е�www������ʵ��Ϊ�˷�ֹURI������������������content��,POSTʱһ����������ʹ��Content-Lengthָ����С
//					//�������ֲ���������ȡ����������
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
//							debug_msg << "�յ���Ӧ��Json"
//								>> json.c_str();
//							CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Func-VerifyToken", debug_msg);
//						}
//#endif
//						size_t status_bangumi_id = Resolve::Resolve_Auth_Status(json);
//					}
//					catch (boost::system::system_error&) {
//						//������֤��������
//
//					}
//
//				}

				//һ����refresh
				size_t status_bangumi_id = 0;
				//�ж�BangumiID��һ����
				//try {
				std::string sql_bangumi_id = result[0];
				if (status_bangumi_id == 0) {
					//˵���Ѿ�ʧЧ
					//����Refresh
					try {
						auto auth = RefreshToken(result[2], param.qq);

						return{ sql_bangumi_id_num,{auth.access_token,auth.refresh_token } };
					}
					catch (boost::system::system_error&) {
						//������֤ʧ��
						return{ sql_bangumi_id_num,{ "","" } };
					}
				}
				if (std::to_string(status_bangumi_id) != sql_bangumi_id) {
					//˵�����߲�һ��
					//�����û����°�
					throw boost::system::system_error(bangumi_bot_errors::auth_without_same_id);
				}
				//}
				//catch (std::invalid_argument) {
				//	//IDת��ʧ��
				//	throw boost::system::system_error();
				//}
			}
			//��ʱ˵��ID��AccessTokenһ��
			//���Ҵ�����Ч��
			//����access_token
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
			//˵��δע���û�
			//ֱ�ӷ��ؿ��ַ���
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
			debug_msg << "�յ���Ӧ��Json"
				>> json.c_str();
			CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Func-GetUserSubjectProgress", debug_msg);
		}
#endif

			auto resolve_result = Resolve::Resolve_User_Process(json, refresh);

			//���صı���
			bangumi::string msg;
			//����Subject���ܼ���
			resolve_result.second.progress.AddEps(eps);

			msg = resolve_result.second.ProgressGet();

			//�����ĵȴ�
			for (auto &t : resolve_result.first) {
				if (t != nullptr&&t->joinable())
					t->join();
			}

			return{ std::move(msg), resolve_result.second };
		}
		catch (boost::system::system_error&e) {
			if (e.code() == boost::system::system_error(bangumi_bot_errors::user_not_collect_this_subject).code()) {
				//����json��û��user_id��Ϣ,�����û�û���ղ����subject
				//���Դӻ����в���User ���û��ֱ�ӷ��� ����<δ�ղ�>����Ϣ
				try {
					auto &bgm_user = BangumiPreFindUser(user_id);
					BangumiUserProgress user_progress;
					bgm_user.SetProgress(user_progress);
					//Ϊuser���Progress
					return{ bgm_user.ProgressGet(),bgm_user };

				}
				catch (std::out_of_range) {
					//û���ҵ�
					//ֱ�ӷ���
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
						//ֱ�ӷ���
						return{ "",bangumi::BangumiUser() };
					}
				}
				else {
#ifndef NDEBUG
					{
						bangumi::string debug_msg;
						debug_msg << "�û���Ȩˢ�º���������..."
							>> e.what();
						CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Func-GetUserSubjectProgress", debug_msg);
					}
#endif	
					return{ "\n\n��Ȩ����������,�����°�Bangumi-ID...", bangumi::BangumiUser() };
				}
				
			}
			else{
#ifndef NDEBUG
				{
					bangumi::string debug_msg;
					debug_msg << "GetUserSubjectProgress ������������"
						>> e.what();
					CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Func-GetUserSubjectProgress", debug_msg);
				}
#endif	
				return{ "",bangumi::BangumiUser() };
			}
		}


	}

	//ͬ��Http������ĿHtml
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

	//ͬ��Https����һ��HTMLҳ��
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
	//�û��ղظ���
	bangumi::string GetUserSumCollections(std::string bangumi_user) {
		//
		bangumi::string uri("/user/");
		uri << bangumi_user;
		uri << "/collections/status?app_id=" << bgm.bangumi_client_id;
		try {
			//��������
			auto& json = GetBGMAPI(uri);
			//���н���
			return Resolve::Resolve_User_Collection_Sum(json);
		}
		catch (boost::system::system_error&) {
			return "�ղػ�ȡʧ��...";
		}
		catch (std::exception&) {
			return "�ղػ�ȡʧ��";
		}

	}
	//end of bangumi
}


namespace bangumi {
	//ȡ���û�LastSubjectID
	//δע���û��last����0
	inline size_t GetLastSubjectID(int64_t qq) {
		//last subject id
		size_t subject_id = 0;
		//��ѯ���
		bangumi::string query;
		//SELECT * FROM bgm_subjects WHERE subject_id=1
		query << "SELECT user_last_searched FROM bgm_users WHERE user_qq="
			<< qq;
		//��ѯ���
		BGMSQLResult result;
		//Ӱ�������
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
				debug_msg << "GetLastSubjectID��ѯʧ��:"
					<< std::to_string(affect_rows_num)
					>> e.what();

				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-GetLastSubjectID", debug_msg);
			}
#endif	
			//ͨ���쳣��������
			throw e;
		}
		if (affect_rows_num > 0) {
			//��������0,��Чע���û�

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
		//����id
		return subject_id;
	}

	//����*����
	inline ComplexParam PARA_Resolve_One_Star(const std::string& str, BgmCode code) {
		return 999;
	}
	//����**����
	inline ComplexParam PARA_Resolve_Two_Star(const std::string& str, BgmCode code) {
		return 999;
	}
	//����-����
	inline ComplexParam PARA_Resolve_Line(const std::string& str, BgmCode code) {
		return 999;
	}
	//����+����
	inline ComplexParam PARA_Resolve_Plus(const std::string& str, BgmCode code) {
		//��ʱʹ��һ������ID+����,�Ժ�������Ӹ��Ͻ���ʹ��
		//123456+tgc
		auto& npos = std::string::npos;
		//��Ϊ������+��ͻֻʶ�����һ��+��
		auto delim = str.find_last_of('+');
		if (delim == npos) {
			throw boost::system::system_error(bangumi_bot_errors::invalid_param);
		}
		//
		size_t id = 0;
		//�����������
		//���صĽ��
		ComplexParam ret;

		if (delim == 0) {
			//˵��ǰ��û�в���,ʹ��last subject
			ret.use_last_subject_id = true;
		}
		else {
			//ǰ��Ĳ���
			try {
				id = std::stoul(str.substr(0, delim));
			}
			catch (std::invalid_argument&) {
				//ʹ������������
				if (code == BgmCode::Subject) {
					//���ȷָ��ַ��� �ų�������+��ֹӰ�����[����ʹ����#�Ͳ���ʹ��+trc,��ֹ�������͸���]
					//std::string pre_param = str.substr(0, delim);
					auto sharp_res = PARA_Resolve_Sharp(str, code);
					if (sharp_res.id != 0) {
						//˵����ȷ������
						id = sharp_res.id;
						//ΪID��ֵ[����ʹ����#�Ͳ���ʹ��+trc,��ֹ�������͸���]
						ret.id = id;
						//ֱ��return 
						return ret;
					}
					else {
						//Ҳ��ʹ����%
						auto percent_res = PARA_Resolve_Percent(str,code);
						if (percent_res.id != 0) {
							//˵����ȷ������
							id = percent_res.id;
							//��Ϊpercent���+��ͻ,�Ͳ���ʹ��+trc,��ֹ�������͸���
							ret.id = id;
							//ֱ��return 
							return ret;
						}
					}
					//����ʲôҲ����
				}

			}
		}
		//ΪID��ֵ
		ret.id = id;

		//����Ĳ���
		std::string param = str.substr(delim + 1, str.length() - delim - 1);


		//��ݺ�
#define HAVE(c1)\
param.find_first_of(c1)!=npos
		//��Ӧ��ͬ�������
		switch (code)
		{
		case BgmCode::Subject:
			//Subject����ʱ�Ĵ���
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
	//����Search��ǰ׺+-��
	inline std::string& PARA_Resolve_Search_KeyWord(std::string& pre_param) {
		//��һ��Search������
		//�Ӻſ��Բ��滻Ϊ�ո�,������ǰ��Ҫһ���ո�
		size_t sub_pos = 0;
		sub_pos = pre_param.find_first_of('-', sub_pos);
		while (sub_pos != std::string::npos) {
			//URL�п�����+��ʾ�ո�
			pre_param.insert(sub_pos, "+");
			//����������һ��λ��
			sub_pos = pre_param.find_first_of('-', sub_pos + 2);
		}
		//��󷵻ؽ��
		return pre_param;
	}
	//����/����
	inline ComplexParam PARA_Resolve_Virgule(const std::string& str, BgmCode code) {
		//��ʱʹ��һ������ID/����,�Ժ�������Ӹ��Ͻ���ʹ��
		//123456/tgc
		auto& npos = std::string::npos;
		auto delim = str.find_first_of('/');
		if (delim == npos) {
			throw boost::system::system_error(bangumi_bot_errors::invalid_param);
		}
		//
		size_t id = 0;
		//�����������
		//���صĽ��
		ComplexParam ret;

		if (delim == 0) {
			//˵��ǰ��û�в���,ʹ��last subject
			ret.use_last_subject_id = true;
		}
		else {
			//ǰ��Ĳ���
			std::string pre_param = str.substr(0, delim);
			try {
				if (code != BgmCode::Search&&code != BgmCode::Tag &&code != BgmCode::BGM)
					id = std::stoul(pre_param);
				else
					throw std::invalid_argument("no need int");
			}
			catch (std::invalid_argument&) {
				//TODO:ʹ��������������ԭ����String����
				//SearchAPI
				if (code == BgmCode::Search) {
					//�����һ��Search������
					PARA_Resolve_Search_KeyWord(pre_param);
					ret.str = pre_param;
				}
				else 
				//UserCollection
				if (code == BgmCode::BGM) {
					//�����һ��User Collection������
					ret.bangumi_user = pre_param;
				}else
				
				//SubjectAPI
				if (code == BgmCode::Subject||code == BgmCode::Collect||code == BgmCode::Up) {
					//ʹ������������
					//ʹ�÷ָ���ַ��� �ų�������+��ֹӰ�����
					//Subject����ʹ��/��Ϊ����+,-��ͻ
					//CollectҲ����,ǰ׺ֻ����һ��#
					auto sharp_res = PARA_Resolve_Sharp(pre_param, code);
					if (sharp_res.id != 0) {
						//˵����ȷ������
						id = sharp_res.id;
					}
					else {
						//����ʹ��%����
						auto percent_res = PARA_Resolve_Percent(pre_param, code);
						if (percent_res.id != 0) {
							//˵����ȷ������
							id = percent_res.id;
							//��Ϊpercent�����/��ͻ,��˼���ǰ��
						}
					}
					//����ʲôҲ����
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
						//˵����ȷ������
						id = sharp_res.id;
					}
					else {
						//����ʹ��%����
						auto percent_res = PARA_Resolve_Percent(pre_param, code);
						if (percent_res.id != 0) {
							//˵����ȷ������
							id = percent_res.id;
							//��Ϊpercent�����/��ͻ,��˼���ǰ��
						}
						else {
							//����ֱ�Ӹ��踳ֵ
							////�����滻+���ո�//������һ��
							//auto replace_plus_pos = pre_param.find_last_of('+');
							////����str����
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

		//ΪID��ֵ
		ret.id = id;


		//����Ĳ���
		std::string param = str.substr(delim + 1, str.length() - delim - 1);
		std::string orign_param(param);
		//������ת��ΪСд
		std::transform(param.begin(), param.end(), param.begin(), ::tolower);
		//��ݺ�
#define HAVE(c1)\
param.find_first_of(c1)!=npos
#define EXHAVE(first,c1,c2)\
first.find_first_of(c1)!=npos||first.find_first_of(c2)!=npos
#define STRHAVE(s1,s2)\
param.find(s1)!=npos||param.find(s2)!=npos
#define EXSTRHAVE(sss,s1,s2)\
sss.find(s1)!=npos||sss.find(s2)!=npos
		//��Ӧ��ͬ�������
		switch (code)
		{
		case BgmCode::Subject:
			//Subject����ʱ�Ĵ���
			//����: 123456/trc(�������������� ��ǩ,��ɫ,�²�)
			//Ĭ�ϲ��Ǹ��ӹ���
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
			//Search����ʱ�Ĵ���
			//����: Angel+Beats-ova(+��ʾ�ո�-��ʾ�ų�)/2(����)/5(����)/0(�ӵڼ�����ʼ)
			//����������
			//���ͺ�����֮���/
			auto delim1 = param.find_first_of('/');
			//��������ʼλ��֮���/
			auto delim2 = param.find_first_of('/', delim1+1);
			//��һ������
			std::string first_param;
			if (delim1 != npos)
				first_param = param.substr(0, delim1);
			else
				first_param = param;
			//�ӵ�һ�������в���
			if (!first_param.empty()) {
				if (EXHAVE(first_param, 'a', '2')) {
					//����
					ret.search_type = 2;
				}
				else if (EXHAVE(first_param, 'g', '4')) {
					//��Ϸ
					ret.search_type = 4;
				}
				else if (EXHAVE(first_param, 'c', '1')) {
					//��
					ret.search_type = 1;
				}
				else if (EXHAVE(first_param, 'x', '0')) {
					//ȫ���ķ���
					ret.search_type = 0;
				}
				else if (EXHAVE(first_param, 'r', '6')) {
					//����Ԫ
					ret.search_type = 6;
				}
				else if (EXHAVE(first_param, 'm', '3')) {
					//����
					ret.search_type = 3;
				}
			}
			//�ڶ�������
			std::string second_param;
			if (delim1 != npos)
				second_param = param.substr(delim1+1);
			//ֻ���������ַ�,��������еĻ�,����Ĭ��
			if (!second_param.empty() && delim1 + 1 < param.length()) {
				try {
					int temp_num;
					temp_num = std::stoi(second_param);
					ret.search_max_num = temp_num;
				}
				catch (std::invalid_argument&) {
					//ֱ��Ĭ�ϲ���
				}
			}
			//����������
			std::string third_param;
			if (delim2 != npos)
				third_param = param.substr(delim2+1);
			//ֻ���������ַ�,��������еĻ�,����Ĭ��
			if (!third_param.empty() && delim2 + 1 < param.length()) {
				try {
					int temp_num;
					temp_num = std::stoi(third_param);
					ret.search_start_pos = temp_num;
				}
				catch (std::invalid_argument&) {
					//ֱ��Ĭ�ϲ���
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
			//�ָ���
			size_t delim;
			size_t str_start = 0;
			//��������
			std::string para[5];
			int para_num = -1;
			//��һ�����ӷ�
			delim = param.find_first_of('/');
			while (delim != npos)
			{
				//����
				para[++para_num] = param.substr(str_start, delim - str_start);
				str_start = delim + 1;
				delim = param.find_first_of('/', delim + 1);
			}
			//�������һ������
			para[++para_num] = param.substr(str_start);

			//����ʶ����Ĳ���
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
						//��Ŀ����
						if (EXHAVE(s, 'a', '2')) {
							//����
							ret.ucollection_subject_type = "anime";
						}
						else if (EXHAVE(s, 'g', '4')) {
							//��Ϸ
							ret.ucollection_subject_type = "game";
						}
						else if (EXHAVE(s, 'c', '1')) {
							//��
							ret.ucollection_subject_type = "book";
						}
						else if (EXHAVE(s, 'r', '6')) {
							//����Ԫ
							ret.ucollection_subject_type = "real";
						}
						else if (EXHAVE(s, 'm', '3')) {
							//����
							ret.ucollection_subject_type = "music";
						}
					}
						break;
					case 1:
					{
						//�ղ�״̬
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
						//ҳ��
						try {
							ret.ucollection_page = std::stoi(s);
						}
						catch (std::exception&) {
						}
					}
						break;
					case 3:
					{
						//����ʽ
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
						//��ǩ
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
			//�����������������ӷ�
			auto delim = param.find_first_of('/');
			//��һ������
			std::string first_param;
			if (delim != npos)
				first_param = param.substr(0, delim);
			else
				first_param = param;
			//�ڶ�������(ֻȡ������)
			std::string second_param;
			if (delim != npos)
				second_param = param.substr(delim + 1);
			//���������Ļ���
			if (!first_param.empty()) {
				//=====ԭUpdate�е�ʶ��=====	
				size_t plus_pos = first_param.find_first_of('+');
				size_t sub_pos = first_param.find_first_of('-');
				//�Ƿ���+�� ����-��
				bool isPlus = false;
				//�׸���ʶ����ŵ�λ��
				size_t end_pos = sub_pos;
				//�ж�
				if (plus_pos < sub_pos) {
					end_pos = plus_pos;
					isPlus = true;
				}
				//���������ŵ��ַ���
				std::string param_without_sign;
				if (end_pos != std::string::npos) {
					param_without_sign = first_param.substr(0, end_pos);
				}
				else {
					param_without_sign = first_param;
				}
				//�Է��ŵĴ���,�����
				if (end_pos != std::string::npos) {
					try {

						ret.update_eps_shift = std::stoi(first_param.substr(end_pos + 1));
						if (!isPlus) {
							//����ƫ��
							ret.update_eps_shift = -ret.update_eps_shift;
						}

					}
					catch (std::exception&) {
						//ת��ʧ��,ֱ�Ӽ���
						//Ҳ�������ڴ���ʳ�ͻ,Խ��
					}
				}
				//���޷��ŵ�str����
				if (param_without_sign.find("air") != npos) {
					ret.update_air = true;
				}
				else if (param_without_sign.find("fin") != npos) {
					ret.update_fin = true;
				}
				else {
					//����������
					try {
						ret.update_watched_eps = std::stoi(param_without_sign);
					}
					catch (std::invalid_argument&) {
						//��������ʲôҲ����:������Ĭ�ϲ��ӻ���
						//ret.update_watched_eps = 1;
					}

				}
				//=====ԭUpdate�е�ʶ�� Over=====
			}
			//�������ĸ���
			if (!second_param.empty()) {
				try {
					ret.rss_max_items = std::stoi(second_param);
				}
				catch (std::invalid_argument&) {
					//����ʲô
				}
			}
		}
			break;
		case BgmCode::Up:
		{
			//����: #Angel/2(���µ�����)
			//����: #Angel/+2(��������������)
			//����: #Angel+1/+2(SQL�еڶ���ƥ����)
			//����: #Angel+1/air(���µ��ѷ��͵����µ�һ��)
			//����: #Angel+1/air-1(���µ��ѷ��͵����µ�ǰһ��>0)
			//����: #Angel+1/fin(���µ����һ��,ͬʱ�Զ����Ϊcollect״̬)	
			//����: #Angel+1/fin/8/�²�(ͬʱ����+�²�)	
			//size_t plus_pos = param.find_first_of('+');
			//size_t sub_pos = param.find_first_of('-');
			////�Ƿ���+�� ����-��
			//bool isPlus = false;
			////�׸���ʶ����ŵ�λ��
			//size_t end_pos = sub_pos;
			////�ж�
			//if (plus_pos < sub_pos) {
			//	end_pos = plus_pos;
			//	isPlus = true;
			//}
			////���������ŵ��ַ���
			//std::string param_without_sign;
			//if (end_pos != std::string::npos) {
			//	param_without_sign = param.substr(0, end_pos);
			//}
			//else {
			//	param_without_sign = param;
			//}
			////�Է��ŵĴ���,�����
			//if (end_pos != std::string::npos){
			//	try {

			//		ret.update_eps_shift = std::stoi(param.substr(end_pos + 1));
			//		if (!isPlus) {
			//			//����ƫ��
			//			ret.update_eps_shift = -ret.update_eps_shift;
			//		}

			//	}
			//	catch (std::exception&) {
			//		//ת��ʧ��,ֱ�Ӽ���
			//		//Ҳ�������ڴ���ʳ�ͻ,Խ��
			//	}
			//}
			////���޷��ŵ�str����
			//if (param_without_sign.find("air")!=npos) {
			//	ret.update_air = true;
			//}
			//else if (param_without_sign.find("fin")!=npos) {
			//	ret.update_fin = true;
			//}
			//else {
			//	//����������
			//	try {
			//		ret.update_watched_eps = std::stoi(param_without_sign);
			//	}
			//	catch (std::invalid_argument&) {
			//		//��������ʲôҲ����:���û��Լ��Ľ��Ⱦ���
			//		//ret.update_watched_eps = 1;
			//	}
			//	
			//}
			
			//
			//�����
			//���µĻ���������֮���/
			auto delim1 = param.find_first_of('/');
			//���ֺ�����λ��֮���/
			auto delim2 = param.find_first_of('/', delim1 + 1);
			//��һ������
			std::string first_param;
			if (delim1 != npos)
				first_param = param.substr(0, delim1);
			else
				first_param = param;
			//�ӵ�һ�������в���
			if (!first_param.empty()) {
				
				//=====ԭUpdate�е�ʶ��=====	
				size_t plus_pos = first_param.find_first_of('+');
				size_t sub_pos = first_param.find_first_of('-');
				//�Ƿ���+�� ����-��
				bool isPlus = false;
				//�׸���ʶ����ŵ�λ��
				size_t end_pos = sub_pos;
				//�ж�
				if (plus_pos < sub_pos) {
					end_pos = plus_pos;
					isPlus = true;
				}
				//���������ŵ��ַ���
				std::string param_without_sign;
				if (end_pos != std::string::npos) {
					param_without_sign = first_param.substr(0, end_pos);
				}
				else {
					param_without_sign = first_param;
				}
				//�Է��ŵĴ���,�����
				if (end_pos != std::string::npos) {
					try {

						ret.update_eps_shift = std::stoi(first_param.substr(end_pos + 1));
						if (!isPlus) {
							//����ƫ��
							ret.update_eps_shift = -ret.update_eps_shift;
						}

					}
					catch (std::exception&) {
						//ת��ʧ��,ֱ�Ӽ���
						//Ҳ�������ڴ���ʳ�ͻ,Խ��
					}
				}
				//���޷��ŵ�str����
				if (param_without_sign.find("air") != npos) {
					ret.update_air = true;
				}
				else if (param_without_sign.find("fin") != npos) {
					ret.update_fin = true;
				}
				else {
					//����������
					try {
						ret.update_watched_eps = std::stoi(param_without_sign);
					}
					catch (std::invalid_argument&) {
						//��������ʲôҲ����:���û��Լ��Ľ��Ⱦ���
						//ret.update_watched_eps = 1;
					}

				}
				//=====ԭUpdate�е�ʶ�� Over=====
			}
			//�ڶ�������
			std::string second_param;
			if (delim1 != npos)
				second_param = param.substr(delim1 + 1);
			//ֻ���������ַ�,��������еĻ�,����Ĭ��
			if (!second_param.empty() && delim1 + 1 < param.length()) {
				try {
					int temp_num;
					temp_num = std::stoi(second_param);

					//�޶���Χ
					if (temp_num < 0) {
						temp_num = 0;
					}
					else if (temp_num > 10) {
						temp_num = 10;
					}

					//��ֵ
					ret.collection_rating = temp_num;
				}
				catch (std::invalid_argument&) {
					//ֱ��Ĭ�ϲ���
				}
			}
			//����������
			std::string third_param;
			if (delim2 != npos)
				third_param = orign_param.substr(delim2 + 1);
			//�ַ���,��������еĻ�,����Ĭ��
			if (!third_param.empty() && delim2 + 1 < param.length()) {
				ret.collection_comment = third_param;
			}
		}
			break;
		case BgmCode::Collect:
		{
			//����: 123456(Ĭ�ϸ���Ϊ�ڿ�)
			//����: 1234/do(����Ϊ�ڿ�)
			//����: 1234/fin(����Ϊ����)
			//����: 1234/fin/8/������
			//�ղ�״̬
			//	1 = wish = ����
			//	2 = collect = ����
			//	3 = do = ����
			//	4 = on_hold = ����
			//	5 = dropped = ����
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

			//�����
			//״̬������֮���/
			auto delim1 = param.find_first_of('/');
			//���ֺ�����λ��֮���/
			auto delim2 = param.find_first_of('/', delim1 + 1);
			//��һ������
			std::string first_param;
			if (delim1 != npos)
				first_param = param.substr(0, delim1);
			else
				first_param = param;
			//�ӵ�һ�������в���
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
			//�ڶ�������
			std::string second_param;
			if (delim1 != npos)
				second_param = param.substr(delim1 + 1);
			//ֻ���������ַ�,��������еĻ�,����Ĭ��
			if (!second_param.empty() && delim1 + 1 < param.length()) {
				try {
					int temp_num;
					temp_num = std::stoi(second_param);
					
					//�޶���Χ
					if (temp_num < 0) {
						temp_num = 0;
					}
					else if (temp_num > 10) {
						temp_num = 10;
					}

					//��ֵ
					ret.collection_rating = temp_num;
				}
				catch (std::invalid_argument&) {
					//ֱ��Ĭ�ϲ���
				}
			}
			//����������
			std::string third_param;
			if (delim2 != npos)
				third_param = orign_param.substr(delim2 + 1);
			//�ַ���,��������еĻ�,����Ĭ��
			if (!third_param.empty() && delim2 + 1 < param.length()) {
				ret.collection_comment = third_param;
			}
			//���ۺͱ�ǩλ��֮���/
			auto delim3 = third_param.find_first_of('/');
			//���ĸ�����
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
			//airtime��page�����ӷ�
			auto delim = param.find_first_of('/');
			//��һ������
			std::string first_param;
			if (delim != npos)
				first_param = param.substr(0, delim);
			else
				first_param = param;
			//�ڶ�������(ֻȡҳ��)
			std::string second_param;
			if (delim != npos)
				second_param = param.substr(delim + 1);
			//����
			ret.tag_airtime = first_param;
			if (!second_param.empty()) {
				try {
					ret.tag_page = std::stoi(second_param);
				}
				catch (std::invalid_argument&) {
					//����ʲô
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
	//����#����
	inline ComplexParam PARA_Resolve_Sharp(const std::string& str, BgmCode code) {
		if (code == BgmCode::Up || code == BgmCode::Subject || code == BgmCode::Collect ||code == BgmCode::RSS) {
			//SELECT * FROM bgm_subjects WHERE CONCAT(name_cn,name) LIKE CONCAT('%��%') ORDER BY subject_id DESC
			//������������SQL����
			//���߳�SQL
			size_t sharp_pos = str.find_first_of('#');
			if (sharp_pos!=std::string::npos){
				//���ص�subject id
				size_t subject_id = 0;
				//ֻʶ��#֮������һ��-��
				size_t sub_pos = str.find_last_of('-');
				//Ĭ���Ƿ��ص�һ�еĽ��
				unsigned wanted_num = 0;
				if (sub_pos != std::string::npos) {
					//˵����ָ���˵ڼ������
					//�����Խ��͸������һ�н����id
					try {
						std::string temp_stoi = str.substr(sub_pos + 1);
						wanted_num = std::stoi(temp_stoi);
					}
					catch (std::invalid_argument&) {
						//˵���޷�ת��
						//����: ��+��ת��Ϊ%

						//ͬ��ת��ʱҲƥ��+��
					}
				}
				//���Ƚ����gb18030ת����utf-8,���ݿ��ڵı���ΪUTF8
				bangumi::string search_key;
				//������Ϊֹ���е�+�Ŷ�����һ�������,ֻ�Ǵ���%
				//����+->%���滻
				auto replace_plus_pos = str.find_last_of('+');
				//����str����
				std::string after_replace_str = str;
				while (replace_plus_pos != std::string::npos) {
					after_replace_str[replace_plus_pos] = '%';
					replace_plus_pos = after_replace_str.find_last_of('+', replace_plus_pos);
				}
				//�ж��Ƿ���+��
				if (wanted_num==0){
					search_key << code_encoder.Conv(after_replace_str.substr(sharp_pos + 1));
				} 
				else{
					search_key << code_encoder.Conv(after_replace_str.substr(sharp_pos + 1, sub_pos - sharp_pos -1));
				}

#ifndef NDEBUG
				{
					bangumi::string debug_msg;
					debug_msg << "#������:"
						>> "search_key: " << search_key
						>> "wanted_num: " << wanted_num;
					CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-PARA_Resolve_Sharp", debug_msg);
				}
#endif	

				bangumi::string query;
				//�������򶯻�����,֮����ID����
				query << "SELECT subject_id FROM bgm_subjects "
					<< "WHERE CONCAT(name_cn,name) "
					<< "LIKE '%" << search_key << "%' "
					<< "ORDER BY FIELD(type,2,4,6,1,3), "
					<< "subject_id DESC";
				//��ѯ���
				BGMSQLResult result;
#ifndef NDEBUG
				{
					bangumi::string debug_msg;
					debug_msg << "#�������:"
						<< query;
					CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-PARA_Resolve_Sharp", debug_msg);
				}
#endif	
				//Ӱ�������
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
						debug_msg << "#������ѯʧ��:"
							<< std::to_string(affect_rows_num)
							>> e.what();

						CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-PARA_Resolve_Sharp", debug_msg);
					}
#endif	
					//ͨ���쳣��������
					throw e;
				}
				if (affect_rows_num > 0) {
#ifndef NDEBUG
					{
						bangumi::string debug_msg;
						debug_msg << "#������ѯ���:"
							<< std::to_string(affect_rows_num);

						CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-PARA_Resolve_Sharp", debug_msg);
					}
#endif	
					if (wanted_num != 0) {
						//����������������ӳ��1->0,2->1
						--wanted_num;
					}
					//��������0,˵����ƥ����
					if (wanted_num > affect_rows_num - 1) {
						//���need�Ĵ���SQL���еļ�¼��
						//ֱ�ӽ��丳ֵ
						wanted_num = affect_rows_num - 1;
					}
					try {
						//ע��fetchrows����һ��ֻȡһ��,�����Ҫ�������Ҫ����
						result.FetchRow(wanted_num);
						//���Ը�ֵ,һ����ȷ
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
					//����subject_id
					return subject_id;
				}
				else {
					//˵��û��ƥ����
					//�ж��Ƿ���0��Ŀ
					if (subject_id == 0) {
						//ʹ��%����
						std::string temp_str = str;
						temp_str[sharp_pos] = '%';
						return bangumi::PARA_Resolve_Percent(temp_str, code);
					}
					//����0
					return 0;
				}
			}
			else {
				//������һ��#
				//ֱ�ӷ���0
				return 0;
			}
		}
		else {
			//��֧�ִ�����ò�������
			return 0;
		}
	}
	//����%����
	inline ComplexParam PARA_Resolve_Percent(const std::string& str, BgmCode code) {
		if (code == BgmCode::Up || code == BgmCode::Subject || code == BgmCode::Collect||code == BgmCode::RSS) {
			size_t percent_pos = str.find_first_of('%');
			//���û���ҵ�%ֱ�ӷ���0
			if (percent_pos == std::string::npos) {
				return 0;
			}
			size_t star_pos = str.find_first_of('*');
			//����һ����ʱ�������ڵ��ú�������+-
			std::string pre_param;
			if (star_pos!=std::string::npos){
				pre_param = str.substr(percent_pos + 1, star_pos - percent_pos - 1);
			}
			else {
				pre_param = str.substr(percent_pos + 1);
			}
			//����һ�����Ӳ�������
			bangumi::ComplexParam complex_param;
			//�趨Ĭ�ϵ�Search����
			complex_param.search_max_num = 2;
			complex_param.search_start_pos = 0;
			complex_param.search_type = 2;
			//�������/��ֵ
			//�������������н���
			
			if(star_pos!=std::string::npos)
			{
				auto &npos = std::string::npos;
				std::string param = str.substr(star_pos + 1);
				//Search����ʱ�Ĵ���
				//����: Angel+Beats-ova(+��ʾ�ո�-��ʾ�ų�)/2(����)/5(����)/0(�ӵڼ�����ʼ)
				//����������
				//���ͺ�����֮���/
				auto delim1 = param.find_first_of('*');
				//��һ������
				std::string first_param;
				if (delim1 != npos)
					first_param = param.substr(0, delim1);
				else
					first_param = param;
				//�ӵ�һ�������в���
				if (!first_param.empty()) {
					if (EXHAVE(first_param, 'a', '2')) {
						//����
						complex_param.search_type = 2;
					}
					else if (EXHAVE(first_param, 'g', '4')) {
						//��Ϸ
						complex_param.search_type = 4;
					}
					else if (EXHAVE(first_param, 'c', '1')) {
						//��
						complex_param.search_type = 1;
					}
					else if (EXHAVE(first_param, 'x', '1')) {
						//ȫ���ķ���
						complex_param.search_type = 0;
					}
					else if (EXHAVE(first_param, 'r', '6')) {
						//����Ԫ
						complex_param.search_type = 6;
					}
					else if (EXHAVE(first_param, 'm', '3')) {
						//����
						complex_param.search_type = 3;
					}
				}
				//�ڶ�������
				std::string second_param;
				if (delim1 != npos)
					second_param = param.substr(delim1 + 1);
				//ֻ���������ַ�,��������еĻ�,����Ĭ��
				if (!second_param.empty() && delim1 + 1 < param.length()) {
					try {
						int temp_num;
						temp_num = std::stoi(second_param);
						complex_param.search_start_pos = temp_num;
					}
					catch (std::invalid_argument&) {
						//ֱ��Ĭ�ϲ���
					}
				}

			}

			//�����Ĺؼ���
			std::string keyword = PARA_Resolve_Search_KeyWord(pre_param);
			

			//ʹ��BGM API Search
			//����ֻ�ܵ��߳�����
			try {

				//���ݸ��Ӳ����ṹ�����Ӳ���
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
					debug_msg << "�����Request"
						>> request;
					CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTP", debug_msg);
				}
#endif
				try {
					//���߳�����
					std::string json = http_client.SyncBGMHTTPRequest(request);
					//����
				
#ifndef NDEBUG
					{
						bangumi::string debug_msg;
						debug_msg << "�յ���Ӧ��Json"
							>> json.c_str();
						CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTP", debug_msg);
					}
#endif
					//����Ĭ�ϲ�ʹ�û���
					auto resolve_res = Resolve::Resolve_Search_Singel(json,true);
					std::vector<std::shared_ptr<boost::thread>> pic_threads;

					//ͼƬ���߳�
					pic_threads = resolve_res.first;
					//����subject id, ������ResolveSearch�Ѿ�������Subject����
					complex_param.id = resolve_res.second.subject_id;

					//�����ĵȴ�
					for (auto &t : pic_threads) {
						if (t != nullptr&&t->joinable())
							t->join();
					}

					return complex_param;

				}
				catch (boost::system::system_error & e) {
					//����������쳣,ʹ���쳣�ظ�(����û���ҵ��û���)
					//���ﲻʹ��Reply�����ֱ�Ӹ�ֵ�ظ�
#ifndef NDEBUG
					{
						bangumi::string debug_msg;
						debug_msg << "����%���������쳣"
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
			//��֧�ִ�����ò�������
			return 0;
		}
	}

	//���ڽ���Subject API�Ĳ�����װ
	inline ComplexParam ResolveSubjectPara(const std::string& str) {
		BgmCode code = BgmCode::Subject;
		try {
			//����/ 
			return PARA_Resolve_Virgule(str, code);
		}
		catch (boost::system::system_error&) {
#ifndef NDEBUG
			{
				bangumi::string debug_msg;
				debug_msg << "PARA_Resolve_Plus ʧ��";
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-ResolveSubjectPara-Plus", debug_msg);
			}
#endif		
		}

		try {
			//֮����+
			return PARA_Resolve_Plus(str, code);
		}
		catch (boost::system::system_error&) {
#ifndef NDEBUG
			{
				bangumi::string debug_msg;
				debug_msg << "PARA_Resolve_Virgule ʧ��";
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-ResolveSubjectPara-Virgule", debug_msg);
			}
#endif		
			//�����ܽ���
			//���ֱ���׳��쳣 �������˲���
			//throw e;
			//��󽻸�#��%
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

	//���ڽ���Search API�Ĳ�����װ
	inline ComplexParam ResolveSearchPara(const std::string& str) {
		BgmCode code = BgmCode::Search;
		try {
			//ֻ��'/'�Ľ���
			return PARA_Resolve_Virgule(str, code);
		}
		catch (boost::system::system_error&) {
#ifndef NDEBUG
			{
				bangumi::string debug_msg;
				debug_msg << "PARA_Resolve_Virgule ʧ��";
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-ResolveSubjectPara-Virgule", debug_msg);
			}
#endif		
			//�����������
			//���صĽ��
			ComplexParam ret;
			std::string pre_param(str);
			ret.str = PARA_Resolve_Search_KeyWord(pre_param);
			return ret;
			//�����ܽ���
			//���ֱ���׳��쳣 �������˲���
			//throw e;
		}

	}

	//���ڽ���Collection API�Ĳ�����װ
	inline ComplexParam ResolveCollectPara(const std::string& str) {
		BgmCode code = BgmCode::Collect;
		try {
			//ֻ��'/'�Ľ���,����/do��
			return PARA_Resolve_Virgule(str, code);
		}
		catch (boost::system::system_error&) {
#ifndef NDEBUG
			{
				bangumi::string debug_msg;
				debug_msg << "PARA_Resolve_Virgule ʧ��";
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-ResolveCollectPara-Virgule", debug_msg);
			}
#endif		
			//�����������
			//���صĽ��
			//ComplexParam ret;
			//����ʹ��#����
			//return PARA_Resolve_Sharp(str, code);
			//��󽻸�#��%
			auto sharp_res = PARA_Resolve_Sharp(str, code);
			if (sharp_res.id != 0) {
				return sharp_res;
			}
			else {
				auto percent_res = PARA_Resolve_Percent(str, code);
				return percent_res;
			}
			//return ret;
			//�����ܽ���
			//���ֱ���׳��쳣 �������˲���
			//throw e;
		}

	}

	//���ڽ���Update API�Ĳ�����װ
	inline ComplexParam ResolveUpdatePara(const std::string& str) {
		BgmCode code = BgmCode::Up;
		try {
			//ֻ��'/'�Ľ���
			return PARA_Resolve_Virgule(str, code);
		}
		catch (boost::system::system_error&) {
#ifndef NDEBUG
			{
				bangumi::string debug_msg;
				debug_msg << "PARA_Resolve_Virgule ʧ��";
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-ResolveUpdatePara-Virgule", debug_msg);
			}
#endif		
			//�����������
			//���صĽ��
			//����ʹ��#����
			//return PARA_Resolve_Sharp(str, code);
			//��󽻸�#��%
			auto sharp_res = PARA_Resolve_Sharp(str, code);
			if (sharp_res.id != 0) {
				return sharp_res;
			}
			else {
				auto percent_res = PARA_Resolve_Percent(str, code);
				return percent_res;
			}
			//�����ܽ���
			//���ֱ���׳��쳣 �������˲���
			//throw e;
		}

	}

	//���ڽ���Tag API�Ĳ�����װ
	inline ComplexParam ResolveTagPara(const std::string& str) {
		BgmCode code = BgmCode::Tag;
		try {
			//ֻ��'/'�Ľ���
			return PARA_Resolve_Virgule(str, code);
		}
		catch (boost::system::system_error&e) {
#ifndef NDEBUG
			{
				bangumi::string debug_msg;
				debug_msg << "PARA_Resolve_Virgule ʧ��";
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-ResolveTagPara-Virgule", debug_msg);
			}
#endif		
			//�����ܽ���
			//���ֱ���׳��쳣 �������˲���
			throw e;
		}
	}

	//���ڽ���RSS�Ĳ�����װ
	inline ComplexParam ResolveRSSPara(const std::string& str) {
		BgmCode code = BgmCode::RSS;
		try {
			//ֻ��'/'�Ľ���
			return PARA_Resolve_Virgule(str, code);
		}
		catch (boost::system::system_error&) {
#ifndef NDEBUG
			{
				bangumi::string debug_msg;
				debug_msg << "PARA_Resolve_Virgule ʧ��";
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-ResolveRSSPara-Virgule", debug_msg);
			}
#endif		
			//�����������
			//���صĽ��
			//����ʹ��#����
			//return PARA_Resolve_Sharp(str, code);
			//��󽻸�#��%
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
					//����ֱ�Ӹ��踳ֵ
					//�����滻+���ո�//���ö��һ��,����Get����Ҫ��+����ո�
					//auto replace_plus_pos = str.find_last_of('+');
					////����str����
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
			//�����ܽ���
			//���ֱ���׳��쳣 �������˲���
			//throw e;
		}

	}

	//���ڽ���User Collection�Ĳ�����װ
	inline ComplexParam ResolveUserCollectionPara(const std::string& str) {
		BgmCode code = BgmCode::BGM;
		try {
			//ֻ��'/'�Ľ���
			return PARA_Resolve_Virgule(str, code);
		}
		catch (boost::system::system_error&e) {
#ifndef NDEBUG
			{
				bangumi::string debug_msg;
				debug_msg << "PARA_Resolve_Virgule ʧ��";
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-ResolveTagPara-Virgule", debug_msg);
			}
#endif		
			//�����ܽ���
			//���ֱ���׳��쳣 �������˲���
			throw e;
		}
	}


	//Bot: ��ȡBangumi���������Ϣ
	inline void BOT_Read_Ini(const BGMCodeParam & param, const std::set<size_t>& parameters_id, const std::set<std::string>& parameters_str) {
		//TEST CQ:shared
		//DEFAULT_SEND(param.type, "[CQ:share,url=http://bgm.tv/subject/220566,title=ɱ¾����ʹ,content=���,image=http://lain.bgm.tv/pic/cover/c/1a/b7/220566_0CMxK.jpg]");
		//TEST CQ:music
		//DEFAULT_SEND(type, "[CQ:music,type=custom,url=http://bgm.tv/subject/220566,audio=https://bangumi.moe/,title=ɱ¾����ʹ,content=���,image=http://lain.bgm.tv/pic/cover/c/1a/b7/220566_0CMxK.jpg]");
		if(std::to_string(param.qq)==bgm.owner_qq&&param.type == BgmRetType::Private)
			DEFAULT_SEND(param.type, bgm.GetConf());



	}
	//Bot: Help��Ϣ
	inline void BOT_Help(const BGMCodeParam & param, const std::set<size_t>& parameters_id, const std::set<std::string>& parameters_str) {
		
		bangumi::string help_msg;

		//
		help_msg << "[CQ:image,file=" << bgm.help_pic << "]";
		help_msg >> "ʹ��ָ��: " << "https://bangumi.irisu.cc/";
		
		DEFAULT_SEND(param.type, help_msg);


	}
	//Bot: ͳ����Ϣ
	inline void BOT_Statis(const BGMCodeParam & param, const std::set<size_t>& parameters_id, const std::set<std::string>& parameters_str) {
		//����
		//���磺rank me �����Լ���ʹ��ͳ��
		//���磺rank Ĭ�Ϸ��ؽ��յ�ʹ������
		//��������
		//����
		enum class StatisParam
		{
			me,
			default
		};
		//����me
		StatisParam current_param = StatisParam::default;
		//ֻ��str���͵Ĳ�����ֻ����һ������
		for (auto&p: parameters_str){
			if (p.find("me")!=std::string::npos){
				current_param = StatisParam::me;
			}
		}

		//Main
		const unsigned max_list_rank = 5;
		//�ظ�����Ϣ
		bangumi::string msg;
		//�û�������Ϣ
		std::string user_id;
		std::string user_qq;
		std::string bangumi_id;
		//�û�ͷ��
		std::string image_path = (bgm.cache_path) + USER_PIC_PATH;
		std::string image_file;
		//��ʹ�����
		std::string all_type[10];
		int all_times = 0;
		std::string today_type[10];
		int today_times = 0;
		//����ʹ�����

		//��ѯ���
		bangumi::string query;
		switch (current_param)
		{
		case StatisParam::me:
		{
			//ǰ����Ϣ
			//msg << "����ʹ�����У�";
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
			//ǰ����Ϣ
			msg << "����ʹ�����У�";
			//��ǰʱ��
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
		//SQL��ѯ
#ifndef NDEBUG
		{
			bangumi::string debug_msg;
			debug_msg << "SQL��ѯ���:"
				<< query;

			CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-BOT-Statis", debug_msg);
		}
#endif	
		//��ѯ���
		BGMSQLResult result;
		//Ӱ�������
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
				debug_msg << "SQL��ѯʧ��:"
					<< std::to_string(affect_rows_num)
					>> e.what();

				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-BOT-Statis", debug_msg);
			}
#endif	
			//ͨ���쳣��������
			//throw e;
			//ֱ�ӷ���
			msg = "��ѯʧ��...";
			DEFAULT_SEND(param.type, msg);
			return;
		}
		if (affect_rows_num > 0) {
			//˵���н��
			switch (current_param)
			{
			case StatisParam::me:
			{
				//�û��Ļ�����Ϣ
				user_id = result[0];
				user_qq = result[1];
				bangumi_id = result[2];
				//�û���ʹ��
				for (unsigned j = 0; j < 10; ++j) {
					all_type[j] = result[j + 3];
					all_times += std::stoi(all_type[j]);
				}
				//�û�����ʹ��

				for (unsigned j = 0; j < 10; ++j) {
					today_type[j] = result[j + 13];
					today_times += std::stoi(today_type[j]);
				}
				//ͷ��·��
				if (bangumi_id != "0") {
					image_file = image_path + bangumi_id + ".jpg";
				}
				else {
					image_file = bgm.not_found_ava_path;
				}
				//format
				boost::format fmt("%5d/%-5d");
				//����Ȩ�޼��
				if (bangumi_id[0] != '0' && result[23][0] == '9') {
					//ͨ�����˵����
					msg << "<BGMer>";
				}
				else {
					//����Ȩ��
					msg << "-----------";
				}
				//�ظ���Ϣƴ��
				msg >> "[CQ:image,file=" << image_file << "]"
					>> "QQ: " << user_qq << " x BGM: " << bangumi_id //<< "  [" << user_id << ']'
					>> "����ʹ�ã� " << today_times << "    �ܼ�ʹ�ã� " << all_times
					>> "-----------"
					>> "��Ŀ��" << (fmt%today_type[0] % all_type[0]).str()
					<< "������" << (fmt%today_type[1] % all_type[1]).str()
					>> "�û���" << (fmt%today_type[2] % all_type[2]).str()
					<< "���£�" << (fmt%today_type[3] % all_type[3]).str()
					>> "�ղأ�" << (fmt%today_type[4] % all_type[4]).str()
					<< "�󶨣�" << (fmt%today_type[5] % all_type[5]).str()
					>> "������" << (fmt%today_type[6] % all_type[6]).str()
					<< "��ǩ��" << (fmt%today_type[7] % all_type[7]).str()
					>> "ͳ�ƣ�" << (fmt%today_type[8] % all_type[8]).str()
					<< "δ֪��" << (fmt%today_type[9] % all_type[9]).str()
					>> "-----------";
			}
				break;
			case StatisParam::default:
			{
				//������������������������
				if (affect_rows_num > max_list_rank) {
					affect_rows_num = max_list_rank;
				}
				//format
				boost::format fmt("%5d/%-5d");
				//ѭ������
				for (unsigned i = 0; i < affect_rows_num; ++i) {
					//�û��Ļ�����Ϣ
					user_id = result[0];
					user_qq = result[1];
					bangumi_id = result[2];
					//�û���ʹ��
					for (unsigned j = 0; j < 10; ++j) {
						all_type[j] = result[j + 3];
						all_times += std::stoi(all_type[j]);
					}
					//�û�����ʹ��
					for (unsigned j = 0; j < 10; ++j) {
						today_type[j] = result[j + 13];
						today_times += std::stoi(today_type[j]);
					}
					//ͷ��·��
					if (bangumi_id != "0") {
						image_file = image_path + bangumi_id + ".jpg";
					}
					else {
						image_file = bgm.not_found_ava_path;
					}

					//�ظ���Ϣƴ��
					msg >> "____<" << (i + 1) << ">____"
						>> "[CQ:image,file=" << image_file << "]"
						>> "QQ: " << user_qq << " x BGM: " << bangumi_id //<< "  [" << user_id << ']'
						>> "����ʹ�ã� "<<today_times<<"    �ܼ�ʹ�ã� "<<all_times
						>> "-----------"
						>> "��Ŀ��" << (fmt%today_type[0] % all_type[0]).str()
						<< "������" << (fmt%today_type[1] % all_type[1]).str()
						>> "�û���" << (fmt%today_type[2] % all_type[2]).str()
						<< "���£�" << (fmt%today_type[3] % all_type[3]).str()
						>> "�ղأ�" << (fmt%today_type[4] % all_type[4]).str()
						<< "�󶨣�" << (fmt%today_type[5] % all_type[5]).str()
						>> "������" << (fmt%today_type[6] % all_type[6]).str()
						<< "��ǩ��" << (fmt%today_type[7] % all_type[7]).str()
						>> "ͳ�ƣ�" << (fmt%today_type[8] % all_type[8]).str()
						<< "δ֪��" << (fmt%today_type[9] % all_type[9]).str()
						//>> "------------"
						>> "============";

					//ȡ��һ��
					result.FetchRow(1);
					//���ô���
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
				msg = "�Һ���û�ҵ�����...";
			}
				break;
			case StatisParam::default:
			{
				//˵������û����ʹ�ù�������
				msg = "�������Ǽ�įû�����һ��...";
			}
				break;
			default:
				break;
			}

		}

		//������Ϣ
		DEFAULT_SEND(param.type, msg);
	}
	
	//API: ����Bangumi�û���Ϣ
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

		//��ÿһ��ʶ���ID����main func
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
			//���Դӻ����в���
			//�������Ҫǿ��refresh
			if (!param.extra.refresh)
			{
				try {
					size_t temp_id = std::stoul(name);
					bangumi::string msg = BangumiPreFindUser(temp_id).Get();
					//��ȡ�û��ղ���Ϣ
					msg>>'\n'<<GetUserSumCollections(std::to_string(temp_id));
					DEFAULT_SEND(param.type, msg);
#ifndef NDEBUG
					{
						bangumi::string debug_msg;
						debug_msg << "BGMUserԤ�Ȳ�������";
						CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-BangumiPreFindUser", debug_msg);
					}
#endif
					continue;
				}
				catch (std::out_of_range&) {
#ifndef NDEBUG
					{
						bangumi::string debug_msg;
						debug_msg << "δ�ҵ�����";
						CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-PreFind-BGMUser", debug_msg);
					}
#endif
				}
				catch (std::invalid_argument&) {
#ifndef NDEBUG
					{
						bangumi::string debug_msg;
						debug_msg << "ID�޷�ת��Ϊ����";
						CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-PreFind-BGMUser", debug_msg);
					}
#endif
				}
				catch (std::exception&) {
#ifndef NDEBUG
					{
						bangumi::string debug_msg;
						debug_msg << "δ֪�Ĵ���";
						CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-PreFind-BGMUser", debug_msg);
					}
#endif
				}
			}
			//===============================
			//ʹ��BGM API
			std::shared_ptr<HTTPRequest> request_one =
				http_client.create_request_fixed(http_client.GetID());

			request_one->set_ret_param(bangumi::BGMRetParam{ param,0,bangumi_name });
			request_one->set_host("api.bgm.tv");
			request_one->set_uri("/user/" + bangumi_name);
			//ֻ��������Ŀʱ��ҪCookie: chii_searchDateLine
			request_one->set_request(request_message(request_one, HTTP_WAY::GET,/* "Cookie: chii_searchDateLine=0;\r\n"*/""));
			//���ûص�����
			request_one->set_callback([](std::shared_ptr<HTTPRequest>request_one, bangumi::BGMRetParam param, int ec) {
				//Ҫ�ظ���string
				bangumi::string msg;
				//�ص�����:��ʱ�Ѿ��������Ӧ���ĵĶ�ȡ
				std::string json;
				try {
					GetResponseContent(request_one, json, true, ec);
#ifndef NDEBUG
					{
						bangumi::string debug_msg;
						debug_msg << "�յ���Ӧ��Json"
							>> json.c_str();
						CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTP", debug_msg);
					}
#endif


					auto pic_threads = Resolve::Resolve_User(json, param.extra.refresh);
					msg = pic_threads.second.Get();

					//��ȡ�û��ղ���Ϣ
					msg >> '\n' << GetUserSumCollections(param.cur_str);

					//�����ĵȴ�
					for (auto &t : pic_threads.first) {
						if (t != nullptr&&t->joinable())
							t->join();
					}
				}
				catch (boost::system::system_error & e) {
					//����������쳣,ʹ���쳣�ظ�(����û���ҵ��û���)
					//���ﲻʹ��Reply�����ֱ�Ӹ�ֵ�ظ�
					msg = e.what();
					msg << "[" << param.cur_str << "]...";
				}

				//���ͻظ�
				DEFAULT_SEND(param.type, msg);
				//��httpclient�Ƴ����ôӶ���������

				http_client.RemoveID(request_one->get_id());
			});
			request_one->execute();
		}

		//boost::this_thread::sleep(boost::posix_time::seconds(10));
//		std::string json;
//
//		//TODO:��ɻص�����
//		boost::this_thread::sleep(boost::posix_time::seconds(2));
//
//		GetResponseContent(request_one, json);
//		//DEBUG
//		//DEFAULT_SEND(param.type, json.c_str());
//#ifndef NDEBUG
//		{
//			bangumi::string debug_msg;
//			debug_msg<<"�յ���Ӧ��Json"
//				>>json.c_str();
//			CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTP", debug_msg);
//		}
//#endif
//		auto pic_threads = Resolve::Resolve_User(json);
//		bangumi::string msg = pic_threads.second.Get();
//
//		//�����ĵȴ�
//		for (auto &t : pic_threads.first) {
//			if(t != nullptr&&t->joinable())
//				t->join();
//		}
//		//���ͻظ�
//		DEFAULT_SEND(param.type, msg);
	}
	//API: ����Bangumi��Ŀ��Ϣ
	inline void BGM_API_Subject(const BGMCodeParam & param, const std::set<size_t>& parameters_id, const std::set<std::string>& parameters_str) {
		//last_subject ��Ҫ���ڸ���last_subject��
		size_t last_subject_id = 0;
		//sql�е�last_subject
		size_t sql_last_subject_id = 0;
		//����һ�����Ӳ������Vector
		std::vector<ComplexParam> paramters;
		
		//�ų������
		if (parameters_str.empty() && parameters_id.empty()) {
			//���ǲ�����last�ж�
			//���ڿ���Ϣ��,���ַ��涼������,����ѡ��ֱ�ӷ���,��Ϊ����ϢҲ�߲�������
			//ͬʱ����code_type���Subjectָ��
			param.bgm_code.erase(BgmCode::Subject);
			//ͬʱ���һ��Unknow��ָ������
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
		//���ڴ�����һ��Subject_id������last_subject(ǰ����û��ʹ��last_subject)

		for (const auto& i : parameters_id) {
			paramters.emplace_back(i);
		}
		//�Ƿ��Ѿ������last_subject�ļǺ�
		bool has_sql_last_subject = false;

		//ѹ���������
		for (const auto& i : parameters_str) {
			try {
				auto temp = ResolveSubjectPara(i);
				if (temp.use_last_subject_id) {
					if (!has_sql_last_subject) {
						//����Ѿ�sql����last Subject
						has_sql_last_subject = true;
						//sql last subject
						sql_last_subject_id = GetLastSubjectID(param.qq);
					}

					temp.id = sql_last_subject_id;
				}
				paramters.emplace_back(temp);

			}
			catch(boost::system::system_error& e){
				//ֱ��continue
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
		//ע�������
#define ComplexParamRet(subject_id,complex_param,type,refresh)\
std::string html = bangumi::GetSubjectHtml(subject_id);\
bangumi::string res;\
bangumi::string res1;\
bangumi::string res2;\
bangumi::string res3;\
bangumi::string res4;\
if(complex_param.add_tag||complex_param.add_comment){\
	res<<"[��Ŀ: "<<subject_id<<"]\n";\
	if (complex_param.add_tag)\
		res << Resolve::ResolveSubjectTag(html) >> "-------------";\
	if (complex_param.add_comment)\
		res << Resolve::ResolveSubjectComment(html,refresh) << "-------------";\
}\
if (complex_param.add_role){\
	res1<<"[��Ŀ: "<<subject_id<<"]\n";\
	auto roles = Resolve::ResolveSubjectCharacter(html,refresh);\
	if(!roles.first.empty())\
		res1 << roles.first; \
	/*else*/\
	/*	res1 >>"δ��¼��ɫ...";*/\
	/*res2 << roles.second;*/ \
} \
if (complex_param.add_air_status){\
	/*�����û���Ϣ */\
	auto verify_result = VerifyToken(param);\
	size_t &user_id = verify_result.first;\
	std::string &access_token = verify_result.second.first;\
	std::string &refresh_token = verify_result.second.second;\
	if (!access_token.empty()) {\
		/*������Ŀ״̬*/\
		auto& resolved_subject = Resolve::ResolveSubjectCollection(html, subject_id, refresh);\
		if(resolved_subject.GetEpsCount() == 0){\
		/*�������ʧ��,��������һ��*/\
		resolved_subject = Resolve::ResolveSubjectCollection(html, subject_id, false);\
		}\
		/*����д�ע���û�*/\
		auto& progress_struct =  GetUserSubjectProgress(subject_id, resolved_subject.GetEpsCount(), user_id,\
			param.qq, access_token, refresh_token, refresh);\
		/*ȡ�ý��ȵ�str*/\
		auto& user_progress = progress_struct.second.progress.progress;\
		int curr_eps = 0;\
		/*ת��Ϊ��������*/\
		try {\
			curr_eps = std::stoi(user_progress);\
		}\
		catch(std::exception&) {}\
		resolved_subject.SetCurrentEps(curr_eps);\
		res3 << resolved_subject.Get();\
		/*���Ͻ�����Ϣ*/\
		/*res3 << progress_struct.first;*/\
		progress_struct.second.progress.SetExStr(resolved_subject.GetExStr());\
		res3 << progress_struct.second.ProgressGet();\
	}else{\
		auto& resolved_subject = Resolve::ResolveSubjectCollection(html, subject_id, refresh);\
		if(resolved_subject.GetEpsCount() == 0){\
		/*�������ʧ��,��������һ��*/\
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


		//��ÿһ��ʶ���ID����main func
		for (const auto& complex_param : paramters) {
			const auto& subject_id = complex_param.id;
			//Ϊlast_subject��ֵ
			last_subject_id = subject_id;
			//���id = 0ֱ�Ӽ���
			if (subject_id == 0)
			{
				bangumi::string error_msg = boost::system::system_error(bangumi_bot_errors::empty_subject).what();
				error_msg << "[" << subject_id << "]";
				//���ͻظ�
				DEFAULT_SEND(param.type, error_msg);
				//continue
				continue;
			}
			//Main Func
			//===============================
			//���ȼ���Ƿ���single
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
				//ֱ��continue;
				continue;
			}
			//���Դӻ����в���
			//�������Ҫǿ��refresh
			if (!param.extra.refresh)
			{
				try {
					auto& subject = BangumiPreFindSubject(subject_id);
					bangumi::string msg = subject.Get();
					//====Access Token====
					//�����û���Ϣ
					auto verify_result = VerifyToken(param);
					size_t &user_id = verify_result.first;
					std::string &access_token = verify_result.second.first;
					std::string &refresh_token = verify_result.second.second;
					if (!access_token.empty()) {
						//����д�ע���û�
						//���Ͻ�����Ϣ
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
						debug_msg << "BGMSubjectԤ�Ȳ�������";
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
						debug_msg << "δ�ҵ�����";
						CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-PreFind-BGMSubject", debug_msg);
					}
#endif
				}
				//				catch (std::invalid_argument&) {
				//#ifndef NDEBUG
				//					{
				//						bangumi::string debug_msg;
				//						debug_msg << "ID�޷�ת��Ϊ����";
				//						CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-PreFind-BGMUser", debug_msg);
				//					}
				//#endif
				//				}
				catch (std::exception&) {
#ifndef NDEBUG
					{
						bangumi::string debug_msg;
						debug_msg << "δ֪�Ĵ���";
						CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-PreFind-BGMSubject", debug_msg);
					}
#endif
				}
			}
			//===============================
			//���Դ�SQL�ж�ȡ����
			if (!param.extra.refresh)
			{
				try {
					auto& subject = BangumiSQLFindSubject(subject_id);
					bangumi::string msg = subject.Get();
					//====Access Token====
					//�����û���Ϣ
					auto verify_result = VerifyToken(param);
					size_t &user_id = verify_result.first;
					std::string &access_token = verify_result.second.first;
					std::string &refresh_token = verify_result.second.second;
					if (!access_token.empty()) {
						//����д�ע���û�
						//���Ͻ�����Ϣ
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
						debug_msg << "BGMSubject SQL����";
						CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-BangumiSQLFindSubject", debug_msg);
					}
#endif
					continue;
				}
				catch (boost::system::system_error&e) {
#ifndef NDEBUG
					{
						bangumi::string debug_msg;
						debug_msg << "���Դ�SQL�ж�ȡSubject����ʧ��";
						debug_msg >> "ʧ����Ϣ: " << e.what();
						CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-SQLFind-BGMUser", debug_msg);
					}
#endif
				}
			}
			//===============================
			//ʹ��BGM API
			try {
				std::shared_ptr<HTTPRequest> request_one =
					http_client.create_request_fixed(http_client.GetID());

				//[ע��]������Ҫ��BGMRetParam�������Ϣ
				bangumi::BGMRetParam bgm_param{ param,subject_id,"" };
				//��ΪSubject�е��첽������ҪһЩcomplex_param�еĲ���
				bgm_param.complex_param = complex_param;
				request_one->set_ret_param(bgm_param);
				request_one->set_host("api.bgm.tv");
				//Ĭ�Ͼ���С��json�ṹ��
				request_one->set_uri("/subject/" + std::to_string(subject_id));
				//ֻ��������Ŀʱ��ҪCookie: chii_searchDateLine
				request_one->set_request(request_message(request_one, HTTP_WAY::GET,/* "Cookie: chii_searchDateLine=0;\r\n"*/""));
				//���ûص�����
				request_one->set_callback([](std::shared_ptr<HTTPRequest>request_one, bangumi::BGMRetParam param, int ec) {
					//Ҫ�ظ���string
					bangumi::string msg;
					//���ӻظ��ṹ��
					auto& complex_param = param.complex_param;
					//�ص�����:��ʱ�Ѿ��������Ӧ���ĵĶ�ȡ
					std::string json;
					try {
						GetResponseContent(request_one, json, true, ec);
#ifndef NDEBUG
						{
							bangumi::string debug_msg;
							debug_msg << "�յ���Ӧ��Json"
								>> json.c_str();
							CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTP", debug_msg);
						}
#endif

						auto pic_threads = Resolve::Resolve_Subject(json, param.extra.refresh);
						msg = pic_threads.second.Get();

						//====Access Token====
						//�����û���Ϣ
						auto verify_result = VerifyToken(param);
						size_t &user_id = verify_result.first;
						std::string &access_token = verify_result.second.first;
						std::string &refresh_token = verify_result.second.second;
						if (!access_token.empty()) {
							//����д�ע���û�
							//���Ͻ�����Ϣ
							msg << GetUserSubjectProgress(param.cur_id, pic_threads.second.eps, user_id, 
								param.qq, access_token, refresh_token, param.extra.refresh).first;

						}


						//�����ĵȴ�
						for (auto &t : pic_threads.first) {
							if (t != nullptr&&t->joinable())
								t->join();

						}
					}
					catch (boost::system::system_error & e) {
						//����������쳣,ʹ���쳣�ظ�(����û���ҵ��û���)
						//���ﲻʹ��Reply�����ֱ�Ӹ�ֵ�ظ�
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
					//���ͻظ�
					DEFAULT_SEND(param.type, msg);
					//======Complex_param=======
					if (complex_param.NeedAdd()) {
						ComplexParamRet(param.cur_id, complex_param, param.type, param.extra.refresh);

					}

					//��httpclient�Ƴ����ôӶ���������

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

		//����֮�����last_subject
		if (last_subject_id != 0) {
			//��һ��ֱ�ӵ��߳�
			//��ѯ���
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
					debug_msg << "Update Last Subjectʧ��:"
						<< std::to_string(affect_rows_num)
						>> e.what();

					CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Func-Subject-API", debug_msg);
				}
#endif	
			}
		}
	}
	//API: ��QQ��Bangumi
	inline void BGM_API_Auth(const BGMCodeParam & param, const std::set<size_t>& parameters_id, const std::set<std::string>& parameters_str) {

		if (param.type != BgmRetType::Private) {
			DEFAULT_SEND(param.type, "�󶨹�����˽��~");
			return;
		}

		std::string redirect_url = GetRedirectUrl(param.qq);

		//https://bgm.tv/oauth/authorize?client_id=bgm2435affad00821e3&response_type=code&redirect_uri=http%3A%2F%2Fwww.irisu.cc%2Fbangumi.php%3Fnu%3D0dyvwccxw&state=dcyw
		bangumi::string request_url = "http://bgm.tv/oauth/authorize?response_type=code";
		request_url << "&client_id=" << bgm.bangumi_client_id
			<< "&redirect_uri=" << redirect_url;
		bangumi::string card_msg;
		card_msg << "[CQ:share,url=" << request_url
			<< ",title=�������ǩ����Լ��~"
			<< ",content=��Լ�����QQ��ʵ�ָ��๦��~"
			<< ",image=" << bgm.card_image_url
			<< "]";
		//����һ����Ƭ��Ϣ
		DEFAULT_SEND(param.type, card_msg);

		//����������
		try {

			//Start�����ö�ʱ����
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
			DEFAULT_SEND(param.type, "������˵�С����...");
		}
	}

	//API: �����ղ�API
	inline void BGM_API_Collection(const BGMCodeParam & param, const std::set<size_t>& parameters_id, const std::set<std::string>& parameters_str) {
		//������һ��STR��һ��INT
		//����ʹ��#Angel����
		//����: #Angel(ʹ��#��������: ��ʹ��LIKEģ������SQL��Subject(id�Ӹ�������,��֤ȡ����ƥ��),���û�н��ʹ��SearchAPI��HTML�������������Ǹ���Ŀ,ͬʱupdate��SQL��)
		//����: 123456(Ĭ�ϸ���Ϊ�ڿ�)
		//����: 1234/do(����Ϊ�ڿ�)
		//����: 1234/fin(����Ϊ����)
		//����: /fin(����lastΪ����)
		//����: <��> (����lastΪ�ڿ�)
		//����һ�����Ӳ������Vector
		std::vector<ComplexParam> paramters;
		//last_subject ��Ҫ���ڸ���last_subject��
		size_t last_subject_id = 0;
		//last_subject_id
		size_t sql_last_subject_id = 0;
		//�ų������
		if (parameters_str.empty() && parameters_id.empty()) {
			//Last�ж�
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
				//LastSubjectGetʧ��
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
		//�ж��Ƿ��Ѿ�sql��ѯ��last_subject��
		bool has_sql_last_subject = false;
		//ѹ���������(STR����)
		for (const auto& i : parameters_str) {
			try {
				//auto temp = ResolveCollectPara(i);
				////�ж��Ƿ�ʹ��Last_subject
				////ѹ��paramters
				//paramters.emplace_back(temp);
				auto temp = ResolveCollectPara(i);
				if (temp.use_last_subject_id) {
					if (!has_sql_last_subject) {
						//����Ѿ�sql����last Subject
						has_sql_last_subject = true;
						//sql last subject
						sql_last_subject_id = GetLastSubjectID(param.qq);
					}

					temp.id = sql_last_subject_id;
				}
				paramters.emplace_back(temp);
			}
			catch (boost::system::system_error&e) {
				//��Ϊstr���Բ�ʹ��complex_param
				//���str��������һ���Ϸ��Ĳ���
				//���ֱ�ӹ���һ��Ĭ�ϵ�
				//��Ϊֻ��INT��Ч,STR�粻����ȷת��ΪID�ͺ���
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

		////�û�û��last_subject�ı�ʶ
		//bool user_have_not_last = false;

		//��ÿһ��ʶ���ID����main func
		for (const auto& complex_param : paramters) {
			const size_t &subject_id = complex_param.id;
			//�����ϴ�ʹ�õ�Last subject
			last_subject_id = subject_id;
//			//�ж��Ƿ�use_last_subject
//			if (complex_param.use_last_subject_id) {
//				//�ж��Ƿ��Ѿ���ȡ��last_subject
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
//						//�û�û��last_subject�ı�ʶ
//						user_have_not_last = true;
//					}
//				}
//					
//				subject_id = last_subject_id;
//			}
//			else {
//				subject_id = complex_param.id;
//			}
			//��Чsubject�ظ�
			if (subject_id == 0) {
				bangumi::string error_msg = boost::system::system_error(bangumi_bot_errors::empty_subject).what();
				error_msg << "[" << subject_id << "]";
				//���ͻظ�
				DEFAULT_SEND(param.type, error_msg);
				//continue
				continue;
			}
			//======Main Start======
			//====Access Token====
			//�����û���Ϣ
			std::string access_token;
			try {
				auto verify_result = VerifyToken(param);
				//size_t &user_id = verify_result.first;
				access_token = verify_result.second.first;
				//std::string &refresh_token = verify_result.second.second;
				if (access_token.empty()) {
					//δע���û��޷�ʹ�ô˹���
					throw boost::system::system_error(bangumi_bot_errors::you_need_bind_your_bgm_id);
				}
			}
			catch (boost::system::system_error&e) {
				bangumi::string error_msg;
				error_msg << e.what();
				//���ͻظ�
				DEFAULT_SEND(param.type, error_msg);
				//
				continue;
			}
			//����Tag
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
			//ֻ������API
			bangumi::string content;
			content << "status=" << complex_param.collection_status
				<< "&rating=" << complex_param.collection_rating
				<< "&comment=" << code_encoder.Conv(complex_param.collection_comment);
			if (!tag_msg.empty())
			{
				content << tag_msg;
			}

			//post�е�www������ʵ��Ϊ�˷�ֹURI������������������content��,POSTʱһ����������ʹ��Content-Lengthָ����С
			//�������ֲ���������ȡ����������
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

			//����һ������
			std::shared_ptr<HTTPRequest> request_one =
				http_client.create_request_fixed(http_client.GetID());

			//����һ��bangumi::BGMRetParam
			//���ڻص���������Ҫaccesstoken,���ֱ��Ϊextra_msg��ֵ
			bangumi::BGMRetParam bgmretparam{ param,subject_id,"" };
			bgmretparam.extra_msg = access_token;
			//ͬʱҲ��Ҫ���Ǹ����Ӳ�����ֵ
			bgmretparam.complex_param = complex_param;
			request_one->set_ret_param(bgmretparam);
			request_one->set_host("api.bgm.tv");
			request_one->set_uri(uri);
			//ֻ��������Ŀʱ��ҪCookie: chii_searchDateLine
			request_one->set_request(request_message(request_one, HTTP_WAY::POST,
				header,
				content));
			//���ûص�����
			request_one->set_callback([](std::shared_ptr<HTTPRequest>request_one, bangumi::BGMRetParam param, int ec) {
				//Ҫ�ظ���string
				bangumi::string msg;
				//���ӻظ��ṹ��
				auto& complex_param = param.complex_param;
				//�ص�����:��ʱ�Ѿ��������Ӧ���ĵĶ�ȡ
				std::string json;
				try {
					GetResponseContent(request_one, json, true, ec);
#ifndef NDEBUG
					{
						bangumi::string debug_msg;
						debug_msg << "�յ���Ӧ��Json"
							>> json.c_str();
						CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTP", debug_msg);
					}
#endif
					
					//======API Start======



					
					//��HTML������Ҫ����Ϣ: ��Ʒ��,�ܼ���,ͼƬ,��ǰ���͵Ļ�����
					std::string html = bangumi::GetSubjectHtml(param.cur_id);
					
					BangumiSubjectCollection subject_data = Resolve::ResolveSubjectCollection(html, param.cur_id, param.extra.refresh);
					if (subject_data.GetEpsCount() == 0) {
						/*�������ʧ��,��������һ��*/\
						subject_data = Resolve::ResolveSubjectCollection(html, param.cur_id, false);
					}
					if (!subject_data.Valid()) {
						//˵�� �����Ŀ�ǲ��Էǻ�Ա���ŵ�
						throw boost::system::system_error(bangumi_bot_errors::maybe_301_maybe_limit);
					}
					
					//���ص���Ϣ������Ŀ��html����������Ϣ
					msg << subject_data.Get();
					
					//����json
					auto resolve_result = Resolve::Resolve_Collect(json, param.extra.refresh);

					//=============�ж��Ƿ���collect===============
					if (complex_param.collection_status.compare("collect")==0) {
						//˵���û�ֱ�ӱ��Ϊ�˿���
						//���ֱ�Ӹ��´���Ŀ��ALLEPS
						//ʹ��ͬ����HTTP
						//ʹ��CollectionAPI�ղ�Ϊ����
						try {
							//Collection API ͬ��
							//
							bangumi::string content;
							content << "watched_eps=" << subject_data.GetEpsCount()
								<< "&watched_vols=" << subject_data.GetEpsCount();


							//post�е�www������ʵ��Ϊ�˷�ֹURI������������������content��,POSTʱһ����������ʹ��Content-Lengthָ����С
							//�������ֲ���������ȡ����������
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

							//post�е�www������ʵ��Ϊ�˷�ֹURI������������������content��,POSTʱһ����������ʹ��Content-Lengthָ����С
							//�������ֲ���������ȡ����������
							bangumi::string request1;
							request1 << "POST " << uri << " HTTP/1.1\r\n"
								<< "Host: api.bgm.tv\r\n"
								<< header << "\r\n"
								<< content;

							std::string json1 = http_client.SyncBGMHTTPRequest(request1);

#ifndef NDEBUG
							{
								bangumi::string debug_msg;
								debug_msg << "�յ���Ӧ��Json"
									>> json1.c_str();
								CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Func-RefreshToken", debug_msg);
							}
#endif

							//����json
							//��ʵ���ý���Ҳ��,�������Ľ��
							auto resolve_result2 = Resolve::Resolve_Update(json1, param.extra.refresh);

							if (resolve_result2) {
								//������½��ȳɹ������� �ٷְٵ���ɶ�
								resolve_result.second.progress.progress = std::to_string(subject_data.GetEpsCount()) + "/" 
									+ std::to_string(subject_data.GetEpsCount());
							}
						}
						catch (boost::system::system_error&e) {
							//�����˴���
#ifndef NDEBUG
							{
								bangumi::string debug_msg;
								debug_msg << "��Collection��ͬ������Update APIʧ��"
									>> e.what();
								CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-API-Collect-Update", debug_msg);
							}
#endif
							//ֱ�Ӻ���
						}
					}
					//============================================
					//����Subject���ܼ���
					//���ڴ�API�̶�����Ϊ����0,��˺��Դ���Ϣ
					//resolve_result.second.progress.AddEps(subject_data.GetEpsAiredCount());

					//���ص���Ϣ�����û������ȵ���Ϣ
					msg << resolve_result.second.ProgressGet();

					//�����ĵȴ�
					for (auto &t : resolve_result.first) {
						if (t != nullptr&&t->joinable())
							t->join();
					}

						
						

					//======API Over======
				}
				catch (boost::system::system_error & e) {
					//����������쳣,ʹ���쳣�ظ�(����û���ҵ��û���)
					//���ﲻʹ��Reply�����ֱ�Ӹ�ֵ�ظ�
					msg << e.what();
					//msg << "[" << param.cur_str << "]...";
				}

				//���ͻظ�
				DEFAULT_SEND(param.type, msg);

				//��httpclient�Ƴ����ôӶ���������

				http_client.RemoveID(request_one->get_id());
			});
			request_one->execute();

			//======Main Over======
		}
		//����֮�����last_subject
		if (last_subject_id != 0) {
			//��һ��ֱ�ӵ��߳�
			//��ѯ���
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
					debug_msg << "Update Last Subjectʧ��:"
						<< std::to_string(affect_rows_num)
						>> e.what();

					CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Func-Subject-API", debug_msg);
				}
#endif	
			}
		}

	}
	//API: ���½���API
	inline void BGM_API_Update(const BGMCodeParam & param, const std::set<size_t>& parameters_id, const std::set<std::string>& parameters_str) {
		//++��up��������
		//������һ��int �� str, strֻ��ʹ��/���Ӳ���
		//����: 123456(���½���+1(��������API��֪),���û���ղ�(ʧ�ܵķ���),��ǿ���ղ�Ϊ�ڿ�(do))
		//����: #Angel(ʹ��#��������: ��ʹ��LIKEģ������SQL��Subject(id�Ӹ�������,��֤ȡ����ƥ��),���û�н��ʹ��SearchAPI��HTML�������������Ǹ���Ŀ,ͬʱupdate��SQL��,�����������൱��������Ĭ�ϵ�Search)
		//����: #Angel/2(���µ�����)
		//����: #Angel/+2(��������������)
		//����: #Angel+1/+2(SQL�еڶ���ƥ����)
		//����: #Angel+1/air(���µ��ѷ��͵����µ�һ��)
		//����: #Angel+1/air-1(���µ��ѷ��͵����µ�ǰһ��>0)
		//����: #Angel+1/fin(���µ����һ��,ͬʱ�Զ����Ϊcollect״̬)
		//����: /fin(ʡ�Բ���)
		//����: /fin/8/�²�(ͬʱ����8��,���²�)
		//����ʹ�ü�ʱϵͳ
		//��ʱ��ȫ�ֵ������run(),ÿ���ӽ���һ��Check()Vector<TimeWork>,TimeWorkӵ��API������Ҫ�Ĳ����Ŀ����ͺ�����ָ��
		//��ʱ�����д洢��ʱ����
		//���ȼ��TimeWorker�Ƿ�,Ŀǰ������Update����
		if (param.extra.countdown != 0) {
			//˵����һ���ӳ�����
			bangumi::string delay_code_msg;

			if (time_worker.GetCurrentListNum() <= bgm.max_time_work_num) {
				//�Ѿ���ȡ��countdown,����Ϊ0
				BGMCodeParam new_param(param);
				new_param.extra.countdown = 0;
				time_worker.AddAPIFunc(std::cref(BGM_API_Update), new_param, parameters_id, parameters_str, param.extra.countdown);
				delay_code_msg << "�˸��µĽ������[" << param.extra.countdown << "]���Ӻ󷵻�."
					>> "�����ʱ��û�лظ����, ˵�������˵�����״��, ���ֶ�����...";
			}
			else {
				delay_code_msg << "�ܱ�Ǹ, ��ǰ��ʱ�����Ѵ�����, ���ֶ�����...";
			}

			//֪ͨ�û���һ���ӳ�����
			DEFAULT_SEND(param.type, delay_code_msg);
			return;
		}
		//����һ�����Ӳ������Vector
		std::vector<ComplexParam> paramters;
		//last_subject ��Ҫ���ڸ���last_subject��
		size_t last_subject_id = 0;
		//last_subject_id
		size_t sql_last_subject_id = 0;

		//�ų������
		if (parameters_str.empty() && parameters_id.empty()) {
			//Last�ж�
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
				//LastSubjectGetʧ��
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

		//�ж��Ƿ��Ѿ�sql��ѯ��last_subject��
		bool has_sql_last_subject = false;
		//ѹ���������(STR����)
		for (const auto& i : parameters_str) {
			try {
				//auto temp = ResolveCollectPara(i);
				////�ж��Ƿ�ʹ��Last_subject
				////ѹ��paramters
				//paramters.emplace_back(temp);
				auto temp = ResolveUpdatePara(i);
				if (temp.use_last_subject_id) {
					if (!has_sql_last_subject) {
						//����Ѿ�sql����last Subject
						has_sql_last_subject = true;
						//sql last subject
						sql_last_subject_id = GetLastSubjectID(param.qq);
					}

					temp.id = sql_last_subject_id;
				}
				paramters.emplace_back(temp);
			}
			catch (boost::system::system_error&e) {
				//��Ϊstr���Բ�ʹ��complex_param
				//���str��������һ���Ϸ��Ĳ���
				//���ֱ�ӹ���һ��Ĭ�ϵ�
				//��Ϊֻ��INT��Ч,STR�粻����ȷת��ΪID�ͺ���
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
		//��ÿһ��ʶ���ID����main func
		for (auto& complex_param : paramters) {
			const auto& subject_id = complex_param.id;
			//����last subject
			last_subject_id = subject_id;
			//��Ч��ID
			if (subject_id == 0) {
				bangumi::string error_msg = boost::system::system_error(bangumi_bot_errors::empty_subject).what();
				error_msg << "[" << subject_id << "]";
				//���ͻظ�
				DEFAULT_SEND(param.type, error_msg);
				//continue
				continue;
			}
			//======Main Start======
			//====Access Token====
			//�����û���Ϣ
			size_t user_id;
			std::string access_token;
			std::string refresh_token;
			try {
				auto verify_result = VerifyToken(param);
				user_id = verify_result.first;
				access_token = verify_result.second.first;
				refresh_token = verify_result.second.second;
				if (access_token.empty()) {
					//δע���û��޷�ʹ�ô˹���
					throw boost::system::system_error(bangumi_bot_errors::you_need_bind_your_bgm_id);
				}
			}
			catch (boost::system::system_error&e) {
				bangumi::string error_msg;
				error_msg << e.what();
				//���ͻظ�
				DEFAULT_SEND(param.type, error_msg);
				//
				continue;

			}
			try {
				//��HTML������Ҫ����Ϣ: ��Ʒ��,�ܼ���,ͼƬ,��ǰ���͵Ļ�����
				std::string html = bangumi::GetSubjectHtml(subject_id);
				BangumiSubjectCollection subject_data = Resolve::ResolveSubjectCollection(html, subject_id, param.extra.refresh);
				if (subject_data.GetEpsCount() == 0)
				{
					//����������Ϣ�����ܻ���Ϊ0��������һ���Է�ֹ����ʧ��
					subject_data = Resolve::ResolveSubjectCollection(html, subject_id, false);
				}
				if (!subject_data.Valid()) {
					//˵�� �����Ŀ�ǲ��Էǻ�Ա���ŵ�
					throw boost::system::system_error(bangumi_bot_errors::maybe_301_maybe_limit);
				}

				//���û��ĸ�����Ϣ
				bangumi::BangumiUser this_bgm_user;
				//���û��Դ���Ŀ�Ľ���
				//bangumi::BangumiUserProgress user_progress;
				//ҲҪ�ڴ˴������û��Ľ���
				if (!access_token.empty()) {
					//����д�ע���û�
					//��ȡUser,���а���������Ϣ
					auto progress_res = GetUserSubjectProgress(subject_id, subject_data.eps_counts, user_id,
						param.qq, access_token, refresh_token, param.extra.refresh);
					this_bgm_user = progress_res.second;
					//user_progress = this_bgm_user.progress;
				}

				//�û��Ľ���(��Ч)
				int user_had_finished = 0;
				if (this_bgm_user.progress.valid) {
					//˵�����û��Ѿ��ղش���Ŀ
					try {
						user_had_finished = std::stoi(this_bgm_user.progress.progress);
					}
					catch (std::invalid_argument&) {
						//������,�޷�ת��Ϊ����
						//ֱ������
					}
				}
				else {
					//���û�û���ղ���Ŀ
					//if (this_bgm_user.user_id != 0) {
					//	//˵�������û��Ļ���
					//}
					//else {
					//	//�������û��Ļ���
					//}
					//
					try {
						//Collection API ͬ��
						//
						bangumi::string content1;
						content1 << "status=do"
							<< "&rating="
							<< "&comment=";

						//post�е�www������ʵ��Ϊ�˷�ֹURI������������������content��,POSTʱһ����������ʹ��Content-Lengthָ����С
						//�������ֲ���������ȡ����������
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


						//post�е�www������ʵ��Ϊ�˷�ֹURI������������������content��,POSTʱһ����������ʹ��Content-Lengthָ����С
						//�������ֲ���������ȡ����������
						bangumi::string request1;
						request1 << "POST " << uri1 << " HTTP/1.1\r\n"
							<< "Host: api.bgm.tv\r\n"
							<< header1 << "\r\n"
							<< content1;

						std::string json1 = http_client.SyncBGMHTTPRequest(request1);

#ifndef NDEBUG
						{
							bangumi::string debug_msg;
							debug_msg << "�յ���Ӧ��Json"
								>> json1.c_str();
							CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Func-RefreshToken", debug_msg);
						}
#endif

						//����json
						auto resolve_result = Resolve::Resolve_Collect(json1, param.extra.refresh);

						//����Subject���ܼ���
						//���ڴ�API�̶�����Ϊ����0,��˺��Դ���Ϣ
						//resolve_result.second.progress.AddEps(subject_data.GetEpsAiredCount());

						//ͬʱΪUser��UserProgress����
						this_bgm_user = resolve_result.second;
						//user_progress = this_bgm_user.progress;

						//�����ĵȴ�
						for (auto &t : resolve_result.first) {
							if (t != nullptr&&t->joinable())
								t->join();
						}



					}
					catch (boost::system::system_error&e) {
						//�����˴���
#ifndef NDEBUG
						{
							bangumi::string debug_msg;
							debug_msg << "��Update��ͬ������Collection APIʧ��"
								>> e.what();
							CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-API-Update-Collect", debug_msg);
						}
#endif
						bangumi::string error_msg;
						error_msg << e.what();
						//���ͻظ�
						DEFAULT_SEND(param.type, error_msg);
						//
						continue;
					}
				}
				//��ʱ��֤���û��ղ�����Ŀ
				//���ҵõ�����Ч��User������Progress��Ա
				//�û�����Ľ���
				int &user_self_watched_eps = user_had_finished;
				//��Ҫ���µĽ���
				int to_update_eps = 0;
				//=========�������==========
				if (complex_param.update_air) {
					//ʹ��air
					to_update_eps = subject_data.GetEpsAiredCount();
				}
				else if (complex_param.update_fin) {
					//ʹ��eps�ܼ� ������SP
					to_update_eps = subject_data.GetEpsCount();
				}
				else if (complex_param.update_watched_eps != 0) {

					//ʹ�����õĽ��Ȳ���
					to_update_eps = complex_param.update_watched_eps;
				}
				else {
					//ʹ���û����Լ��Ľ���+1
					to_update_eps = user_self_watched_eps + 1;
					//Ϊ���볣ʶ���
					if (complex_param.update_eps_shift != 0)
						--complex_param.update_eps_shift;
				}
				//������ƫ��
				to_update_eps += complex_param.update_eps_shift;
				//��������Ʒ�Χ
				if (to_update_eps > subject_data.GetAllEpsCount()) {
					to_update_eps = subject_data.GetAllEpsCount();
				}
				if (to_update_eps < 0) {
					to_update_eps = 0;
				}

				//=========�������Over==========
				//�ж���������Ѿ�������TV����EPS��ֱ���ղ�Ϊ����
				if (to_update_eps >= subject_data.GetEpsCount() && subject_data.GetEpsCount() != 0) {
					//ʹ��CollectionAPI�ղ�Ϊ����
					try {
						//Collection API ͬ��
						//
						bangumi::string content1;
						content1 << "status=collect"
							<< "&rating=" << complex_param.collection_rating
							<< "&comment=" << code_encoder.Conv(complex_param.collection_comment);

						//post�е�www������ʵ��Ϊ�˷�ֹURI������������������content��,POSTʱһ����������ʹ��Content-Lengthָ����С
						//�������ֲ���������ȡ����������
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


						//post�е�www������ʵ��Ϊ�˷�ֹURI������������������content��,POSTʱһ����������ʹ��Content-Lengthָ����С
						//�������ֲ���������ȡ����������
						bangumi::string request1;
						request1 << "POST " << uri1 << " HTTP/1.1\r\n"
							<< "Host: api.bgm.tv\r\n"
							<< header1 << "\r\n"
							<< content1;

						std::string json1 = http_client.SyncBGMHTTPRequest(request1);

#ifndef NDEBUG
						{
							bangumi::string debug_msg;
							debug_msg << "�յ���Ӧ��Json"
								>> json1.c_str();
							CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Func-RefreshToken", debug_msg);
						}
#endif

						//����json
						auto resolve_result = Resolve::Resolve_Collect(json1, param.extra.refresh);

						//����Subject���ܼ���
						//���ڴ�API�̶�����Ϊ����0,��˺��Դ���Ϣ
						//resolve_result.second.progress.AddEps(subject_data.GetEpsAiredCount());

						//ͬʱΪUser��UserProgress����
						this_bgm_user = resolve_result.second;
						//user_progress = this_bgm_user.progress;

						//�����ĵȴ�
						for (auto &t : resolve_result.first) {
							if (t != nullptr&&t->joinable())
								t->join();
						}



					}
					catch (boost::system::system_error&e) {
						//�����˴���
#ifndef NDEBUG
						{
							bangumi::string debug_msg;
							debug_msg << "��Update��ͬ������Collection APIʧ��"
								>> e.what();
							CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-API-Update-Collect", debug_msg);
						}
#endif
						bangumi::string error_msg;
						error_msg << e.what();
						//���ͻظ�
						DEFAULT_SEND(param.type, error_msg);
						//
						continue;
					}
				}
				//==============================


				//ͬʱ���µ�User�����еĽ��ȶ�����
				this_bgm_user.progress.progress = std::to_string(to_update_eps) + '/';
				this_bgm_user.progress.AddEps(subject_data.GetEpsCount());
				subject_data.SetCurrentEps(to_update_eps);
				this_bgm_user.progress.SetExStr(subject_data.GetExStr(user_self_watched_eps));
				//�����Եó��ɹ���Ļظ�
				bangumi::string success_msg;
				bangumi::string extra_msg;
				success_msg << subject_data.Get()
					<< this_bgm_user.UpdateGet();
				extra_msg << subject_data.Get();

				//ֻ������API
				bangumi::string content;
				content << "watched_eps=" << to_update_eps
					<< "&watched_vols=" << to_update_eps;


				//post�е�www������ʵ��Ϊ�˷�ֹURI������������������content��,POSTʱһ����������ʹ��Content-Lengthָ����С
				//�������ֲ���������ȡ����������
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
				//����һ������
				std::shared_ptr<HTTPRequest> request_one =
					http_client.create_request_fixed(http_client.GetID());

				//����һ��bangumi::BGMRetParam
				//[ע��]������Ҫ��BGMRetParam�������Ϣ
				bangumi::BGMRetParam bgm_param{ param,subject_id,"" };
				//�ȸ�complex_param�е�bangumi_html_data��ֵ
				//complex_param.subject_html_data = subject_data;
				//��ΪUpdate�е��첽������ҪһЩcomplex_param�еĲ���bangumi_html_data
				bgm_param.success_msg = success_msg;
				bgm_param.extra_msg = extra_msg;
				request_one->set_ret_param(bgm_param);
				request_one->set_host("api.bgm.tv");
				request_one->set_uri(uri);
				//ֻ��������Ŀʱ��ҪCookie: chii_searchDateLine
				request_one->set_request(request_message(request_one, HTTP_WAY::POST,
					header,
					content));
				//���ûص�����
				request_one->set_callback([](std::shared_ptr<HTTPRequest>request_one, bangumi::BGMRetParam param, int ec) {
					//Ҫ�ظ���string
					bangumi::string msg;
					//���ӻظ��ṹ��
					auto& complex_param = param.complex_param;
					//Bangumi Subject Html Data and success msg
					bangumi::string& extra_msg = param.extra_msg;
					bangumi::string& success_msg = param.success_msg;

					//�ص�����:��ʱ�Ѿ��������Ӧ���ĵĶ�ȡ
					std::string json;
					try {
						GetResponseContent(request_one, json, true, ec);
#ifndef NDEBUG
						{
							bangumi::string debug_msg;
							debug_msg << "�յ���Ӧ��Json"
								>> json.c_str();
							CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTP", debug_msg);
						}
#endif

						//======API Start======


						//��HTML������Ҫ����Ϣ: ��Ʒ��,�ܼ���,ͼƬ,��ǰ���͵Ļ�����
						//�ĵ���requst����֮ǰ����
						//std::string html = bangumi::GetSubjectHtml(param.cur_id);
						//BangumiSubjectCollection subject_data = Resolve::ResolveSubjectCollection(html, param.cur_id, param.extra.refresh);
						//if (!subject_data.Valid()) {
						//	//˵�� �����Ŀ�ǲ��Էǻ�Ա���ŵ�
						//	throw boost::system::system_error(bangumi_bot_errors::maybe_301_maybe_limit);
						//}

						//����json

						auto resolve_result = Resolve::Resolve_Update(json, param.extra.refresh);
						if (resolve_result) {
							//˵�����½��ȳɹ�
							//���ص���Ϣ������Ŀ��html����������Ϣ
							msg << success_msg;
						}
						else {
							//˵�����½���ʧ��:Ŀǰ��֪ʧ�ܵ�ԭ��ֻ���û�û���ղغ�502,����û���ղ��ǲ����ܵ�,ֻ����502���������ͬ�Ľ���

							throw boost::system::system_error(bangumi_bot_errors::update_with_the_same_eps);
						}
						//����Subject���ܼ���
						//���ڴ�API�̶�����Ϊ����0,��˺��Դ���Ϣ
						//resolve_result.second.progress.AddEps(subject_data.GetEpsAiredCount());


						//�����ĵȴ�
						//for (auto &t : resolve_result.first) {
						//	if (t != nullptr&&t->joinable())
						//		t->join();
						//}



						//======API Over======
					}
					catch (boost::system::system_error & e) {
						//����������쳣,ʹ���쳣�ظ�(����û���ҵ��û���)
						//���ﲻʹ��Reply�����ֱ�Ӹ�ֵ�ظ�
						msg << extra_msg << "\n\n";
						msg << e.what();
						//msg << "[" << param.cur_str << "]...";
					}

					//���ͻظ�
					DEFAULT_SEND(param.type, msg);

					//��httpclient�Ƴ����ôӶ���������

					http_client.RemoveID(request_one->get_id());
				});
				request_one->execute();

				//======Main Over======
			}
			catch (boost::system::system_error& e) {
				//���ͻظ�
				DEFAULT_SEND(param.type, e.what());
				//
				continue;
			}


		}

		//����֮�����last_subject
		if (last_subject_id != 0) {
			//��һ��ֱ�ӵ��߳�
			//��ѯ���
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
					debug_msg << "Update Last Subjectʧ��:"
						<< std::to_string(affect_rows_num)
						>> e.what();

					CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Func-Subject-API", debug_msg);
				}
#endif	
			}
		}

	}
	//API: ����API
	inline void BGM_API_Search(const BGMCodeParam & param, const std::set<size_t>& parameters_id, const std::set<std::string>& parameters_str) {
		//������һ��STR, Searchֻ����STR����, ��Userһ��, ֻ����/���Ӳ���,
		//����: Angel+Beats-ova(+��ʾ�ո�-��ʾ�ų�)/2(����)/5(����)/0(�ӵڼ�����ʼ)
		//������SQL,����ʹ��API��Large,ͬʱSave��SQL��

		//https://api.bgm.tv/search/subject/%E6%9C%88%E5%BD%B1?responseGroup=large&start=0&max_results=6
		//���Ƚ��ַ�תUTF-8����
		//�ų������
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

		//����һ�����Ӳ������Vector
		std::vector<ComplexParam> paramters;
		//ѹ���������(ֻ��STR����)
		for (const auto& i : parameters_str) {
			try {
				auto temp = ResolveSearchPara(i);
				//ѹ��paramters
				paramters.emplace_back(temp);
			}
			catch (boost::system::system_error&) {
				//��Ϊstr���Բ�ʹ��complex_param
				//���str��������һ���Ϸ��Ĳ���
				//���ֱ�ӹ���һ��Ĭ�ϵ�
				paramters.emplace_back(i);
			}

		}

	
		//��ÿһ��ʶ���ID����main func
		for (const auto& complex_param : paramters) {
			const auto& subject_str = complex_param.str;
			//Main Func
			//ʹ��BGM API
			try {
				std::shared_ptr<HTTPRequest> request_one =
					http_client.create_request_fixed(http_client.GetID());

				request_one->set_ret_param(bangumi::BGMRetParam{ param,0,subject_str });
				request_one->set_host("api.bgm.tv");
				//ע��ת��ΪHTML �� URLENCODE ʵ�ʾ���utf- 8
				//���ݸ��Ӳ����ṹ�����Ӳ���
				bangumi::string extra_search_param;
				extra_search_param <<"?type="<< complex_param.search_type
					<< "&responseGroup=large"
					<< "&start=" << complex_param.search_start_pos
					<< "&max_results=" << complex_param.search_max_num;
				request_one->set_uri("/search/subject/" + code_encoder.Conv(subject_str) + extra_search_param);
				//ֻ��������Ŀʱ��ҪCookie: chii_searchDateLine, ҲΪ��ֹ��������
				request_one->set_request(request_message(request_one, HTTP_WAY::GET, "Cookie: chii_searchDateLine=0;\r\n", ""));
				//���ûص�����
				request_one->set_callback([](std::shared_ptr<HTTPRequest>request_one, bangumi::BGMRetParam param, int ec) {
					//Ҫ�ظ���string
					bangumi::string msg;
					//���ӻظ��ṹ��
					auto& complex_param = param.complex_param;
					//�ص�����:��ʱ�Ѿ��������Ӧ���ĵĶ�ȡ
					std::string json;
					try {
						GetResponseContent(request_one, json, true, ec);
#ifndef NDEBUG
						{
							bangumi::string debug_msg;
							debug_msg << "�յ���Ӧ��Json"
								>> json.c_str();
							CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTP", debug_msg);
						}
#endif

						auto resolve_res = Resolve::Resolve_Search(json, param.extra.refresh);
						std::vector<std::shared_ptr<boost::thread>> pic_threads;

						//ͼƬ���߳�
						pic_threads = resolve_res.first;
						//���ص�Search���
						msg << resolve_res.second;
						
						//�����ĵȴ�
						for (auto &t : pic_threads) {
							if (t != nullptr&&t->joinable())
								t->join();
						}

					}
					catch (boost::system::system_error & e) {
						//����������쳣,ʹ���쳣�ظ�(����û���ҵ��û���)
						//���ﲻʹ��Reply�����ֱ�Ӹ�ֵ�ظ�
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
					//���ͻظ�
					DEFAULT_SEND(param.type, msg);
					//��httpclient�Ƴ����ôӶ���������

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
	//API: ����Tag��Ϣ
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

		//�ų������
		if (parameters_str.empty() && parameters_id.empty()) {
			bangumi::string error;
			error << "ȱ�ٲ���...";
			//�ظ���Ϣ
			DEFAULT_SEND(param.type, error);
			return;
		}
			
			

		//����һ�����Ӳ������Vector
		std::vector<ComplexParam> paramters;
		//ѹ���������(ֻ��STR����)
		for (const auto& i : parameters_str) {
			try {
				auto temp = ResolveTagPara(i);
				//ѹ��paramters
				paramters.emplace_back(temp);
			}
			catch (boost::system::system_error&) {
				//��Ϊstr���Բ�ʹ��complex_param
				//���str��������һ���Ϸ��Ĳ���
				//���ֱ�ӹ���һ��Ĭ�ϵ�
				bangumi::ComplexParam temp;
				temp.tag_keyword = i;
				paramters.emplace_back(temp);
			}

		}


		//http://bgm.tv/anime/tag/%E6%90%9E%E7%AC%91/?sort=date
		//http://bgm.tv/anime/browser/tv/airtime/2019-4
		//http://bgm.tv/anime/tag/%E6%97%A5%E5%B8%B8/airtime/2019-4

		//ʾ������ ��Ѫ/2019-4 ��ʾ2019-4���еı�ǩ����Ѫ�ķ���
		//���uri�����¸�ʽ�������Ĭ��ȫ��
		//���������tagû��ָ���·���ʹ����������ֻȡǰ��ҳ��
		//��������html
		//��ÿһ��ʶ���ID����main func
		for (const auto& complex_param : paramters) {

			//������Ϣ����uri
			bangumi::string uri2;
			
			//�ؼ���Ϊ��
			//��������ֻ��%�������⣬���asio��read_until������������
			if (complex_param.tag_keyword.empty()|| complex_param.tag_keyword=="%"){
				//ֱ������airtime
				uri2 << "browser/tv/airtime/"
					<< complex_param.tag_airtime;
			}
			else {
				//�ؼ��ֲ�Ϊ��
				uri2 << "tag/";
				uri2 << code_encoder.Conv(complex_param.tag_keyword);
				//����airtime
				uri2 << "/airtime/"
					<< complex_param.tag_airtime;
			}
			//���airtimeΪ��
			if (complex_param.tag_airtime.empty()){
				//����Ҫ����������
				uri2 << "?sort=date";
				//���page��Ч
				if (complex_param.tag_page != 0) {
					uri2 << "&page="<< complex_param.tag_page;
				}
			}
			else {
				//���page��Ч
				if (complex_param.tag_page != 0) {
					uri2 << "?page=" << complex_param.tag_page;
				}
			}
#ifndef NDEBUG
			{
				bangumi::string debug_msg;
				debug_msg << "�����URIΪ: " << uri2;
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-API-Tag", debug_msg);
			}
#endif
			try {
				//Main Func
				bangumi::string uri;
				uri << "/anime/" << uri2;

				std::string request = "GET " + uri + " HTTP/1.1\r\n"
					"Host: " "bgm.tv" "\r\n" "\r\n";

				//����html
				std::string html = code_converter.Conv(http_client.SyncBGMHTTPRequest(request));

				//����html
				bangumi::string ret = Resolve::ResolveTag(html, param.extra.refresh);

				//ʧ����Ϣ
				if (ret.empty()) {
					ret << "δ���ҵ������Ŀ...";
				}

				//�ظ���Ϣ
				DEFAULT_SEND(param.type, ret);
			}
			catch (boost::system::system_error& e) {
				//�ظ���Ϣ
				DEFAULT_SEND(param.type, e.what());
				continue;
			}

		}
	}

	//RSS
	inline void BGM_RSS(const BGMCodeParam & param, const std::set<size_t>& parameters_id, const std::set<std::string>& parameters_str, BgmCode rss_type) {
		//ֻȡHTML�ķ���״̬���ɣ���������ԭ�������Լ�ep
		//����:dmhy һȭ����+6/5  (�ԡ�һȭ���� 6��Ϊ�ؼ��֣����½��5��)
		//����:moe 


		//bangumi::string ret("<Tag>");
		//ret >> "ID:\n";
		//for (const auto &i : parameters_id)
		//	ret << i << " ";
		//ret >> "STR:\n";
		//std::string name;
		//for (const auto &i : parameters_str)
		//	ret << i << " ";

		//DEFAULT_SEND(param.type, ret);

		//����һ�����Ӳ������Vector
		std::vector<ComplexParam> paramters;
		//last_subject_id
		size_t sql_last_subject_id = 0;
		//�ų������
		if (parameters_str.empty() && parameters_id.empty()) {
			//Last�ж�
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
				//LastSubjectGetʧ��
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
		//���Ȩ��
		bangumi::string query;
		query << "SELECT dmhy_open "
			<< "FROM bgm_users "
			<< "WHERE user_qq="
			<< param.qq;
		//��ѯ���
		BGMSQLResult result;
		//Ӱ�������
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
				debug_msg << "SQL��ѯʧ��:"
					<< std::to_string(affect_rows_num)
					>> e.what();

				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-BOT-Statis", debug_msg);
			}
#endif	
			return;
		}
		if (affect_rows_num > 0) {
			//���Ȩ��
			if (result[0][0] == '0')
			{
				//���ͻظ�
				DEFAULT_SEND(param.type, "�볢��@�һ�[\"BGM��\"/\"bgm��\"]�������ҵĿ���~");
				return;
			}
			else if (result[0][0] == '9') {
				//ʲôҲ����
				//���к����Ĵ���
			}
			else {
				//���ͻظ�
				DEFAULT_SEND(param.type, "�������ڽ�����~");
				return;
			}
		}
		else
		{
			return;
		}
		//ѹ���������(id)
		for (const auto& i : parameters_id) {
			paramters.emplace_back(i);
		}
		//�ж��Ƿ��Ѿ�sql��ѯ��last_subject��
		bool has_sql_last_subject = false;
		//ѹ���������(str)
		for (const auto& i : parameters_str) {
			try {
				auto temp = ResolveRSSPara(i);
				//���ж��Ƿ�ʹ��Last Subject
				if (temp.use_last_subject_id) {
					if (!has_sql_last_subject) {
						//����Ѿ�sql����last Subject
						has_sql_last_subject = true;
						//sql last subject
						sql_last_subject_id = GetLastSubjectID(param.qq);
					}

					temp.id = sql_last_subject_id;
				}
				//ѹ��paramters
				paramters.emplace_back(temp);
			}
			catch (boost::system::system_error&) {
;
			}
		}

		//==========Main Func=============
		//��ÿһ��ʶ���ID����main func
		for (auto& complex_param : paramters) {
			//�����ĳ��ø�ֵ
			const auto& subject_id = complex_param.id;
			//��Ч��ID
			if (subject_id == 0&& complex_param.rss_keyword.empty()) {
				bangumi::string error_msg = boost::system::system_error(bangumi_bot_errors::empty_subject).what();
				error_msg << "[" << subject_id << "]";
				//���ͻظ�
				DEFAULT_SEND(param.type, error_msg);
				//continue
				continue;
			}



			//ʹ�õ�RSSԴ
			bangumi::string rss_host;
			bangumi::string rss_uri;
			//Ҫ�ظ�����Ϣǰ׺
			bangumi::string pre_ret;

			switch (rss_type)
			{
			case BgmCode::MOE:
			{
				rss_host << "bangumi.moe";
				rss_uri << "/rss/search/";
				pre_ret << "��Դ��Դ: [ �ȷ��� ]"
					"\n�ؼ���: ";
			}
				break;
			case BgmCode::DMHY:
			{
				rss_host << "share.dmhy.org";
				rss_uri << "/topics/rss/rss.xml?keyword=";
				pre_ret << "��Դ��Դ: [ ������԰ ]"
					"\n�ؼ���: ";
			}
				break;
			default:
				break;
			}
			//https://share.dmhy.org/topics/rss/rss.xml
			//https://share.dmhy.org/topics/rss/rss.xml?keyword=123
			//https://bangumi.moe/rss/search/123
			//����ʹ�õ��߳�����ҳ��


			try
			{
				bangumi::string keyword;
				//�����Ҫhtml�ķ���״̬����
				if (complex_param.rss_keyword.empty()) {
					//��HTML������Ҫ����Ϣ: ��Ʒ��,�ܼ���,ͼƬ,��ǰ���͵Ļ�����
					std::string html = bangumi::GetSubjectHtml(subject_id);
					BangumiSubjectCollection subject_data = Resolve::ResolveSubjectCollection(html, subject_id, param.extra.refresh);
					if (!subject_data.Valid()) {
						//˵�� �����Ŀ�ǲ��Էǻ�Ա���ŵ�
						throw boost::system::system_error(bangumi_bot_errors::maybe_301_maybe_limit);
					}

					//��Ҫ���µĽ���
					float to_rss_eps = 0;
					//=========�������==========
					if (complex_param.update_air) {
						if (subject_data.GetEpsAiredCount() != 0) {
							//�ѷ��͵�
							try {
								//ȡep.XX
								to_rss_eps = std::stof(subject_data.air_eps[subject_data.GetEpsAiredCount() - 1].substr(3));
							}
							catch (std::exception&e) {
								//ʹ��eps�ܼ� ������SP
								to_rss_eps = subject_data.GetEpsAiredCount();
							}
						}
					}
					else if (complex_param.update_fin) {

						if (subject_data.GetEpsUnAiredCount() != 0) {
							//˵����δ���͵�
							try {
								//ȡep.XX
								to_rss_eps = std::stof(subject_data.unair_eps[subject_data.GetEpsUnAiredCount() - 1].substr(3));
							}
							catch (std::exception&e) {
								//ʹ��eps�ܼ� ������SP
								to_rss_eps = subject_data.GetEpsCount();
							}
						}
						else {
							//˵���Ѿ��������
							try {
								//ȡep.XX
								to_rss_eps = std::stof(subject_data.air_eps[subject_data.GetEpsAiredCount() - 1].substr(3));
							}
							catch (std::exception&e) {
								//ʹ��eps�ܼ� ������SP
								to_rss_eps = subject_data.GetEpsCount();
							}
						}

					}
					else if (complex_param.update_watched_eps != 0) {

						//ʹ�����õĽ��Ȳ���
						to_rss_eps = complex_param.update_watched_eps;
					}
					else {
						//����ʲôҲ����

					}
					//������ƫ��
					to_rss_eps = (int)(to_rss_eps + .6) + complex_param.update_eps_shift;

					//��uri��ֵ
					if (!complex_param.rss_keyword.empty()) {
						//�������ؼ��ֲ�Ϊ�վ�ֱ��ʹ�����
						keyword << complex_param.rss_keyword;
					}
					else {
						//����ʹ��html�������name
						if (!(subject_data.name_cn.empty())) {
							std::string temp(subject_data.name_cn);
							////ת���ո�+
							//size_t blank_site = temp.find_first_of(' ');
							//while (blank_site!=std::string::npos)
							//{
							//	temp[blank_site] = '+';
							//	blank_site = temp.find_first_of(' ', blank_site);
							//}
							//ֱ������ո���ǰ�Ĺؼ���
							size_t blank_site = temp.find_first_of(' ');
							if (blank_site != std::string::npos)
							{
								temp.erase(blank_site);
							}
							//ֻ���������ĵĴ���
							std::string temp_(temp);
							if (temp_.size() > 4 * 2) {
								//4��������
								temp = "";
								//ȡǰ4����
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
								//���ܳ�����������ͻ�ԭ
								if (temp.empty())
								{
									temp = temp_;
								}

							}
							keyword << temp;
						}
						else {
							std::string temp(subject_data.name);
							//ת���ո�+
							//size_t blank_site = temp.find_first_of(' ');
							//while (blank_site != std::string::npos)
							//{
							//	temp[blank_site] = '+';
							//	blank_site = temp.find_first_of(' ', blank_site);
							//}
							//ֱ������ո���ǰ�Ĺؼ���
							size_t blank_site = temp.find_first_of(' ');
							if (blank_site != std::string::npos)
							{
								temp.erase(blank_site);
							}
							keyword << temp;
						}

					}
					//֮����ϻ���
					if ((int)to_rss_eps != 0) {

						keyword << '+' << (int)(to_rss_eps);
						//dmhy����������ʹ�ã����ӹؼ���
						if (to_rss_eps < 10)
						{
							keyword << "|0" << std::to_string((int)(to_rss_eps));
						}
					}
				}
				else {
					int to_rss_eps = 0;
					if (complex_param.update_watched_eps != 0) {

						//ʹ�����õĽ��Ȳ���
						to_rss_eps = complex_param.update_watched_eps;
					}
					else {
						//����ʲôҲ����

					}
					//������ƫ��
					to_rss_eps = (int)(to_rss_eps + .6) + complex_param.update_eps_shift;

					//��uri��ֵ
					keyword << complex_param.rss_keyword;
					//֮����ϻ���
					if (to_rss_eps != 0) {
						keyword << '+' << std::to_string(to_rss_eps);
						//dmhy����������ʹ�ã����ӹؼ���
						if (to_rss_eps<10)
						{
							keyword << "|0" << std::to_string(to_rss_eps);
						}
					}
						
				}

				//������Ŀ��
				int max_items = complex_param.rss_max_items;
				//�滻+���ո�
				size_t plus_site = keyword.find_first_of('+');
				while (plus_site!=std::string::npos)
				{
					keyword[plus_site] = ' ';
					plus_site = keyword.find_first_of('+', plus_site+1);
				}
				rss_uri << code_encoder.Conv(keyword);
				//ǰ׺��Ϣ
				pre_ret << "[ " << keyword << " ]\n\n";

#ifndef NDEBUG
				{
					bangumi::string debug_msg;
					debug_msg << "�����RSSΪ: " << rss_host << rss_uri;
					CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-RSS", debug_msg);
				}
#endif

#ifndef NDEBUG
				{
					bangumi::string debug_msg;
					debug_msg << "�����rss_htmlΪ: "
						>> "uri" << url_encode(rss_uri);
					CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-RSS", debug_msg);
				}
#endif
				//��ȡHTMLҳ��
				std::string rss_html = GetHttpsHtml(rss_host, url_encode(rss_uri));
#ifndef NDEBUG
				{
					bangumi::string debug_msg;
					debug_msg << "�����rss_htmlΪ: " << rss_html.size()
						>> "uri" << code_encoder.Conv(rss_uri);
					CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-RSS", debug_msg);
				}
#endif
				//����HTML
				auto ret_vec = Resolve::ResolveRSS(rss_html,rss_type, complex_param.rss_max_items, pre_ret, param.extra.refresh);
				//��Ϣ����
				for (auto& ret : ret_vec){
					//�ظ���Ϣ
					DEFAULT_SEND(param.type, ret);
				}
				
			}
			catch (const boost::system::system_error&e)
			{
				//�ظ���Ϣ
				DEFAULT_SEND(param.type, e.what());
				//continue;
			}
		
		}
		





		
	}

	//API: ����ʱ�����Ϣ
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
				//˵����ȫ�ֵ�ʱ���
				std::string html = bangumi::GetTimeLine();
				bangumi::string time_str = Resolve::ResolveTimeLine(html);
				//���ͻظ�
				DEFAULT_SEND(param.type, time_str);
			}
			catch (boost::system::system_error&) {
				//���ͻظ�
				DEFAULT_SEND(param.type, "�����˵�����...");
			}
			//ֱ�ӷ���
			return;
		}
		//��ÿһ��ʶ���ID����main func
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
				//���ͻظ�
				DEFAULT_SEND(param.type, tml);
			}
			catch (boost::system::system_error&) {
				//���ͻظ�
				DEFAULT_SEND(param.type, "�����˵�����...");
			}
		}

	}

	//API: �����û��ղ�
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

		//�ų������
		if (parameters_str.empty() && parameters_id.empty()) {
			bangumi::string error;
			error << "ȱ�ٲ���...";
			//�ظ���Ϣ
			DEFAULT_SEND(param.type, error);
			return;
		}



		//����һ�����Ӳ������Vector
		std::vector<ComplexParam> paramters;
		//ѹ���������(ֻ��STR����)
		for (const auto& i : parameters_str) {
			try {
				auto temp = ResolveUserCollectionPara(i);
				//ѹ��paramters
				paramters.emplace_back(temp);
			}
			catch (boost::system::system_error&) {
				//��Ϊstr���Բ�ʹ��complex_param
				//���str��������һ���Ϸ��Ĳ���
				//���ֱ�ӹ���һ��Ĭ�ϵ�
				bangumi::ComplexParam temp;
				temp.bangumi_user = i;
				paramters.emplace_back(temp);
			}

		}


		//http://bgm.tv/anime/list/wz97315/collect
		//http://bgm.tv/game/list/wz97315/do?page=2
		//http://bgm.tv/game/list/wz97315/do?orderby=rate
		//http://bgm.tv/anime/list/wz97315/collect?tag=%E7%BB%9D%E8%B5%9E
		
		//{�û�ID}/{��Ŀ����}/{�ղ�״̬}/{ҳ��}/{����ʽ}/{��ǩ}

		//��������html
		//��ÿһ��ʶ���ID����main func
		for (const auto& complex_param : paramters) {
			//������Ϣ
			bangumi::string ret;

			//������Ϣ����uri
			bangumi::string uri2;
			
			//��Ŀ����
			uri2 << '/' << complex_param.ucollection_subject_type << "/list/";

			//�û���
			std::string bangumi_id;
			//�Ƿ�ʹ���Լ��󶨵�bangumi id
			if (complex_param.use_last_subject_id || complex_param.bangumi_user == "#") {
				auto verify_result = VerifyToken(param);
				size_t &user_id = verify_result.first;
				bangumi_id = std::to_string(user_id);
			}
			else {
				bangumi_id = complex_param.bangumi_user;
			}
			//�û�ͷ��
			std::string & json = GetHtml("/user/" + bangumi_id, bgm.bangumi_api_url);
			//�������
			boost::property_tree::ptree json_pt;
			//��Ϊ������Ҫһ��������,���ʹ��stringstream,Ҳ֧���ļ�����ȡ
			std::istringstream json_input(json);
			//����json
			boost::property_tree::read_json(json_input, json_pt);
			//
			size_t id = json_pt.get<size_t>("id", 0);
			bangumi_id = json_pt.get<std::string>("username", "");
			std::string url = json_pt.get<std::string>("avatar.large", "");
			std::string pic_name = std::to_string(id);
			//PICͼƬ�����߳�
			std::vector<std::shared_ptr<boost::thread>> ThreadVector;
			//
			std::string pic_file_path;
			//����ͼƬ
			if (!url.empty()) {
				//����ͼƬ
				auto result = PicDownload(http_client, url, USER_PIC_PATH, pic_name, pic_file_path, param.extra.refresh);

				//���ص��̱߳��浽����ֵ��
				if (result.first == DownloadStatus::MultiThread)
				{
					//�������߳�ѹ���̳߳���
					ThreadVector.push_back(result.second);
				}

				//����ͼƬ��Ϣ
				ret << "[CQ:image,file=" << pic_file_path << "]";

			}

			//�ȴ�ͼƬ�������
			for (auto &t : ThreadVector) {
				if (t != nullptr&&t->joinable())
					t->join();
			}

			//���û���
			uri2 << bangumi_id << '/';


			//�ղ�����
			uri2 << complex_param.ucollection_co_type;

			//ҳ��
			uri2 << "?page=" << complex_param.ucollection_page;

			//����ʽ
			if (!complex_param.ucollection_order_type.empty())
			{
				uri2 << "&orderby=" << complex_param.ucollection_order_type;
			}

			//��ǩ
			if (!complex_param.ucollection_tag.empty()) {
				uri2 << "&tag=" << code_encoder.Conv(complex_param.ucollection_tag);
			}


#ifndef NDEBUG
			{
				bangumi::string debug_msg;
				debug_msg << "�����URIΪ: " << uri2;
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-User-Collection", debug_msg);
			}
#endif
			try {
				//Main Func

				std::string request = "GET " + uri2 + " HTTP/1.1\r\n"
					"Host: " "bgm.tv" "\r\n" "\r\n";

				//����html
				std::string html = code_converter.Conv(http_client.SyncBGMHTTPRequest(request));

				//��Ϣͷ����
				ret >> '[' << bangumi_id << "] ";
				switch (complex_param.ucollection_co_type[0])
				{
				case 'w':
					ret << "�뿴/��/��";
					break;
				case 'c':
					ret << "��/��/����";
					break;
				case 'd':
					if (complex_param.ucollection_co_type[1] == 'o')
						ret << "�ڿ�/��/��";
					else
						ret << "����";
					break;
				case 'o':
					ret << "����";
					break;
				default:
					break;
				}
				ret << " �� ";
				switch (complex_param.ucollection_subject_type[0])
				{
				case 'a':
					ret << "[����]";
					break;
				case 'b':
					ret << "[��]";
					break;
				case 'g':
					ret << "[��Ϸ]";
					break;
				case 'm':
					ret << "[����]";
					break;
				case 'r':
					ret << "[����Ԫ]";
					break;
				default:
					break;
				}

				ret >> "------";

				bangumi::string ret1(ret);
				//����html
				auto& result = Resolve::ResolveUserCollection(html, param.extra.refresh);
				ret << '\n';
				ret << result.first;
				//�ظ���Ϣ
				DEFAULT_SEND(param.type, ret);
				if (!result.second.empty()) {
					ret1 << "[��]\n";
					ret1 << result.second;
					//�ظ���Ϣ
					DEFAULT_SEND(param.type, ret1);
				}
					

			}
			catch (boost::system::system_error& e) {
				//�ظ���Ϣ
				DEFAULT_SEND(param.type, e.what());
				continue;
			}

		}
	}

}
#endif
