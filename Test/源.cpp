#include <iostream>
#include "BangumiInterface.h"


int main() {
	
	int64_t fromQQ = 597320012;
	int32_t msgId = 123456789;
	int32_t subType = 11;
	////const char * msg = "：  123  up 123456,++*1+**Angelco.co **MOMO collect";
	////const char * msg = "：u123456 ,dsfsa,conf 12-15 22 /66 sdf 12 1512,conf";
	////const char * msg = ":djsd* 12da2 123";
	const char * msg = ":reg";
	//const char * msg = ":u wz97315,123";
	Init();


	//测试图片下载
	//test Http_client.SyncHTTPRequest
	//{
	//	std::string x = "GET " "/pic/user/l/000/09/29/92981.jpg" " HTTP/1.1\r\n"
	//		"Host: " "lain.bgm.tv""\r\n" "\r\n" "\r\n";
	//	//"User-Agent: " "Mozilla/4.0 (compatible; MSIE 5.0; Windows NT; DigExt)\r\n"  "\r\n";
	//	//失败,没有使用SSL无法https,而且wordpress中使用的是text/html返回图片
	//	x = "GET " "/res/1.2-ava-600.png" " HTTP/1.1\r\n"
	//		"Host: " "xxxx.xx""\r\n" "\r\n";
	//	//http://lain.bgm.tv/pic/cover/l/7b/d6/239816_BbNdb.jpg
	//	x = "GET " "/pic/cover/l/7b/d6/239816_BbNdb.jpg" " HTTP/1.1\r\n"
	//		"Host: " "lain.bgm.tv""\r\n" "\r\n";
	//	http_client.SyncHTTPRequest("lain.bgm.tv", x, "D:\\Program\\酷Q\\cqsdk-vc-master\\cc.sirokuma.Bangumi\\", "test_pic");
	//}

	//test 自由函数HTTPDOWNLOAD
	//{
	//	std::cout << "总池大小: " << bgm.threadpool_size << std::endl;
	//	std::cout << "可用大小: " << bgm.curr_thread_size << std::endl << std::endl;
	//	auto result = HTTPDownload(http_client, "lain.bgm.tv", "/pic/cover/l/02/9d/254895_tq0Vx.jpg", "", "test_download");
	//	std::cout << "总池大小: " << bgm.threadpool_size << std::endl;
	//	std::cout << "可用大小: " << bgm.curr_thread_size << std::endl << std::endl;
	//	if (result.first == DownloadStatus::MultiThread)
	//	{
	//		//多线程
	//		if (result.second->joinable())
	//		{
	//			result.second->join();
	//		}
	//		std::cout << "总池大小: " << bgm.threadpool_size << std::endl;
	//		std::cout << "可用大小: " << bgm.curr_thread_size << std::endl << std::endl;
	//	}
	//	else if (result.first == DownloadStatus::SingleThread)
	//	{
	//		//单线程或已存在图片
	//		//直接等待
	//	}
	//	else {
	//		std::cout << "Has finished!" << std::endl;
	//	}
	//}

	//测试自由函数PicDownload
	//{
	//	std::cout << "总池大小: " << bgm.threadpool_size << std::endl;
	//	std::cout << "可用大小: " << bgm.curr_thread_size << std::endl << std::endl;
	//	std::string file_path;
	//	//http://lain.bgm.tv/pic/user/l/000/09/29/92981.jpg?r=155375247
	//	//auto result = PicDownload(http_client, "http://lain.bgm.tv/pic/user/l/000/09/29/92981.jpg?r=155375247", "", "test_download3");
	//	auto result = PicDownload(http_client, "http://lain.bgm.tv/pic/cover/l/b9/9b/217660_h3554.jpg", "", "test_download2", file_path);
	//	std::cout << "总池大小: " << bgm.threadpool_size << std::endl;
	//	std::cout << "可用大小: " << bgm.curr_thread_size << std::endl << std::endl;
	//	std::cout << file_path << std::endl;
	//	if (result.first == DownloadStatus::MultiThread)
	//	{
	//		//多线程

	//		if (result.second->joinable())
	//		{
	//			result.second->join();
	//		}
	//		std::cout << "总池大小: " << bgm.threadpool_size << std::endl;
	//		std::cout << "可用大小: " << bgm.curr_thread_size << std::endl << std::endl;
	//	}
	//	else if (result.first == DownloadStatus::SingleThread)
	//	{
	//		//单线程或已存在图片
	//		//直接等待
	//	}
	//	else {
	//		std::cout << "Has finished!" << std::endl;
	//	}	
	//}
	//std::string x = "GET " "/user/wz97315" " HTTP/1.1\r\n"
	//	"Host: " "api.bgm.tv""\r\n" "\r\n";

	//http_client.SyncHTTPRequest("api.bgm.tv", x);
	//C5_Client_ASYN();
	//test_asyn();
	//other test 


	//TEST USER API
	//{
	//	std::shared_ptr<HTTPRequest> request_one =
	//		http_client.create_request_fixed(1);

	//	request_one->set_host("api.bgm.tv");
	//	request_one->set_uri("/user/wz97315");
	//	request_one->set_request(request_message(request_one, HTTP_WAY::GET, "Cookie: chii_searchDateLine = 0;\r\n"));
	//	request_one->execute();
	//	std::string json;
	//	//DEBUG
	//	//DEFAULT_SEND(param.type, json.c_str());
	//	//TODO:完成回调函数
	//	boost::this_thread::sleep(boost::posix_time::seconds(2));

	//	GetResponseContent(request_one, json);
	//	auto pic_threads = Resolve::Resolve_User(json);
	//	bangumi::string msg = pic_threads.second.Get();

	//	//结束的等待
	//	for (auto &t : pic_threads.first) {
	//		if (t != nullptr)
	//			t->join();
	//	}

	//	//
	//	std::cout << "OK" << std::endl;
	//}

//================User===================
//	{
//	//Main Func
//	std::shared_ptr<HTTPRequest> request_one =
//		http_client.create_request_fixed(http_client.GetID());
//
//	request_one->set_ret_param({ BgmRetType::Private, 597320012, 0, 123456, "123456", bangumi::BGMCodeExtraVar()});
//	request_one->set_host("api.bgm.tv");
//	request_one->set_uri("/user/wz97315");
//	request_one->set_request(request_message(request_one, HTTP_WAY::GET, "Cookie: chii_searchDateLine = 0;\r\n"));
//	//设置回调函数
//	request_one->set_callback([](std::shared_ptr<HTTPRequest>request_one, bangumi::BGMRetParam param) {
//		//回调函数:此时已经完成了响应报文的读取
//		std::string json;
//		GetResponseContent(request_one, json);
//#ifndef NDEBUG
//		{
//			bangumi::string debug_msg;
//			debug_msg << "收到响应的Json"
//				>> json.c_str();
//			CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTP", debug_msg);
//		}
//#endif
//
//		bangumi::string msg;
//		try {
//			auto pic_threads = Resolve::Resolve_User(json, param.extra.refresh);
//			msg = pic_threads.second.Get();
//
//			//结束的等待
//			for (auto &t : pic_threads.first) {
//				if (t != nullptr&&t->joinable())
//					t->join();
//			}
//		}
//		catch (boost::system::system_error & e) {
//			//如果发生了异常,使用异常回复(包括没有找到用户等)
//			//这里不使用Reply类而是直接赋值回复
//			msg = e.what();
//			msg << "[" << param.cur_id << "]...";
//		}
//
//		//发送回复
//		DEFAULT_SEND(param.type, msg);
//		//从httpclient移除引用从而析构自身
//		http_client.RemoveID(request_one->get_id());
//#ifndef NDEBUG
//		{
//			bangumi::string debug_msg;
//			debug_msg << "Remove over!"
//				;
//			CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTP", debug_msg);
//		}
//#endif	
//	});
//	request_one->execute();
//#ifndef NDEBUG
//	{
//		bangumi::string debug_msg;
//		debug_msg << "Function Over";
//		CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Func", debug_msg);
//	}
//#endif
//	}
//================Subject===================
//	{
//		//使用BGM API
//		std::shared_ptr<HTTPRequest> request_one =
//			http_client.create_request_fixed(http_client.GetID());
//
//		request_one->set_ret_param({ BgmRetType::Private, 597320012, 0, 123456, "123456", bangumi::BGMCodeExtraVar() });
//		request_one->set_host("api.bgm.tv");
//		//默认就是小的json结构体
//		request_one->set_uri("/subject/42315");
//		//只有搜索条目时需要Cookie: chii_searchDateLine
//		request_one->set_request(request_message(request_one, HTTP_WAY::GET,/* "Cookie: chii_searchDateLine = 0;\r\n"*/""));
//		//设置回调函数
//		request_one->set_callback([](std::shared_ptr<HTTPRequest>request_one, bangumi::BGMRetParam param,int ec) {
//			//回调函数:此时已经完成了响应报文的读取
//			std::string json;
//			GetResponseContent(request_one, json);
//#ifndef NDEBUG
//			{
//				bangumi::string debug_msg;
//				debug_msg << "收到响应的Json"
//					>> json.c_str();
//				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTP", debug_msg);
//			}
//#endif
//			//要回复的string
//			bangumi::string msg;
//			try {
//				auto pic_threads = Resolve::Resolve_Subject(json, param.extra.refresh);
//				msg = pic_threads.second.Get();
//
//				//结束的等待
//				for (auto &t : pic_threads.first) {
//					if (t != nullptr&&t->joinable())
//						t->join();
//				}
//			}
//			catch (boost::system::system_error & e) {
//				//如果发生了异常,使用异常回复(包括没有找到用户等)
//				//这里不使用Reply类而是直接赋值回复
//				msg = e.what();
//				msg << "[" << param.cur_id << "]...";
//			}
//
//			//发送回复
//			DEFAULT_SEND(param.type, msg);
//			//从httpclient移除引用从而析构自身
//			
//			http_client.RemoveID(request_one->get_id());
//		});
//		request_one->execute();
//
//		boost::this_thread::sleep(boost::posix_time::seconds(100));
//	}
//===========================================
// test Auth post 
	//{
	//	bangumi::string content;
	//	content << "grant_type=authorization_code"
	//		<< "&code=" << "91d"
	//		<< "&redirect_uri=" << GetRedirectUrl(597320012)
	//		<< "&client_id=" << "d09805b"
	//		<< "&client_secret=" << "2bac4ceb7";
	//	//创建一个请求
	//	std::shared_ptr<HTTPRequest> request_one =
	//		http_client.create_request_fixed(http_client.GetID());
	//	bangumi::string header("Cache-Control: no-cache\r\n"
	//		"Content-Type: application/x-www-form-urlencoded\r\n");

	//	header << "Content-Length: "<< content.length() <<"\r\n";


	//	//必要的信息都拥有,直接构造一个bangumi::BGMRetParam
	//	request_one->set_ret_param(bangumi::BGMRetParam{ BgmRetType::Private , 597320012, 0, 0, "", bangumi::BGMCodeExtraVar() });
	//	request_one->set_host("bgm.tv");
	//	request_one->set_uri("/oauth/access_token?");
	//	//只有搜索条目时需要Cookie: chii_searchDateLine
	//	request_one->set_request(request_message(request_one, HTTP_WAY::POST,
	//		header,
	//		//"Content-Type: application/json\r\n",
	//		content));
	//	//设置回调函数
	//	request_one->set_callback([](std::shared_ptr<HTTPRequest>request_one, bangumi::BGMRetParam param) {
	//		//回调函数:此时已经完成了响应报文的读取
	//		std::string json;
	//		GetResponseContent(request_one, json);
	//		std::cout << json;
	//		std::cout << std::endl;
	//#ifndef NDEBUG
	//		{
	//			bangumi::string debug_msg;
	//			debug_msg << "收到响应的Json"
	//				>> json.c_str();
	//			CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTP", debug_msg);
	//		}
	//#endif
	//		//要回复的string
	//		bangumi::string msg;
	//		try {
	//			auto auth = Resolve::Resolve_Auth(json);

	//			//向SQL更新
	//			//++++++++++++++++++++

	//			bangumi::string query;
	//			query << "insert into bgm_users(user_qq,user_bangumi,user_access_token,user_refresh_token)"
	//				<< "values(" << std::to_string(param.qq) << ", " << std::to_string(auth.user_id)
	//				<< ", " << auth.access_token << ", " << auth.refresh_token << ")";

//			//std::string query("CREATE TABLE debug_example (id int not null, my_name varchar(50)");
//			//用于存储结果
//			MYSQL_RES* result = nullptr;
//			unsigned long affect_rows_num = sql_pool.ExecQuery(query, result);

//#ifndef NDEBUG
//			{
//				bangumi::string debug_msg;
//				debug_msg << "影响的行数:"
//					<< std::to_string(affect_rows_num);
//				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTP", debug_msg);
//			}
//#endif
//			mysql_free_result(result);
//			//++++++++++++++++++++
//			msg << "QQ: " << std::to_string(param.qq)
//				<< " 与 Bangumi-iD: " << auth.user_id
//				<< " 完成缔约!";

//		}
//		catch (boost::system::system_error & e) {
//			//如果发生了异常,使用异常回复(包括没有找到用户等)
//			//这里不使用Reply类而是直接赋值回复
//			msg = e.what();
//			msg << "[" << param.cur_str << "]...";
//		}

//		//发送回复
//		SendMsg.at(BgmRetType::Private)(ac, param.qq, msg);
//		//从httpclient移除引用从而析构自身
//		http_client.RemoveID(request_one->get_id());
//	});
//	request_one->execute();
//	//=============================
//	//SQL更新

//	//回复QQ消息
//	//bangumi::string msg;
//	//msg << "code = " << code
//	//	>> "state = " << state;
//	//SendMsg.at(BgmRetType::Private)(ac, qq, msg);

//}
//test sql fetch
//{
//    bangumi::string msg;
//
//	bangumi::string pre_query;
//	pre_query << "SELECT user_id, user_qq, user_bangumi FROM bgm_users WHERE "
//	<< "user_qq = " << std::to_string(597320012);
//	MYSQL_RES* pre_result = nullptr;
//	unsigned long pre_affect_rows_num = sql_pool.ExecQuery(pre_query, pre_result);
//	try
//	{
//		SQLCheckResult(pre_affect_rows_num);
//		MYSQL_ROW rows = mysql_fetch_row(pre_result);
//		//MYSQL_FIELD* field = mysql_fetch_field_direct(pre_result, 0);
//		
//		mysql_free_result(pre_result);
//		//已注册的QQ提示消息
//		for (int i = 0; i < 3; ++i) {
//			msg >> *(rows + i);
//		}
//		//msg >> "========";
//		//msg >> field->decimals;
//
//	}
//	catch (boost::system::system_error& )
//	{
//	#ifndef NDEBUG
//		{
//			bangumi::string debug_msg;
//			debug_msg << "Pre-SQLCheckResult未检查到此QQ已注册:"
//				<< std::to_string(pre_affect_rows_num);
//			CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Auth-SQL-Pre-SQLCheckResult", debug_msg);
//		}
//	#endif	
//		//首先free
//		mysql_free_result(pre_result);
//		//msg << "此次契约好像出了点小问题...\n";
//		//throw e;
//	}
//	SendMsg.at(BgmRetType::Private)(ac, 597320012, msg);
//}
//Test Update sql
	//{
	////UPDATE affected_rows SET my_name=\"Monty\" WHERE id=1
	//	bangumi::string pre_query;
	//	pre_query<<"UPDATE bgm_users SET "
	//		<<"user_bangumi=9,"
	//		<<"user_access_token="<<R"("123456789")"
	//		<<" WHERE user_qq=597320012"
	//	/*		pre_query << "SELECT user_id, user_qq, user_bangumi FROM bgm_users WHERE "
	//				<< "user_qq = " << std::to_string(597320012)*/;
	//	BGMSQLResult pre_result;
	//	unsigned long pre_affect_rows_num = sql_pool.ExecQuery(pre_query, pre_result);

	//	//mysql_free_result(pre_result);
	//}
//Test GB18030

	{
		//=====
		//查询语句
		bangumi::string query;
		//SELECT * FROM bgm_subjects WHERE subject_id=1
		query << "SELECT * FROM bgm_subjects WHERE subject_id="
			<< 123456;
		//	<< 42238;
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
				debug_msg << "SQLCheckResult查询失败:"
					<< std::to_string(affect_rows_num)
					>> e.what();

				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-BangumiSQLFindSubject-SQLCheckResult", debug_msg);
			}
#endif	
			//通过异常结束函数
			throw e;
		}
		//如果找到,直接向BGMSubject中Add
		//由于如果使用Refresh,则不会经过这个和Prefind函数,也不会担心有相同的Key而造成无法覆盖
		//这些都会在Add函数中处理
		if (affect_rows_num > 0) {
			//如果行数不为0,则存在

			//
			try {
#ifndef NDEBUG
				{
					bangumi::string debug_msg;
					debug_msg << "SQL中Prefind Subject";
					for (int i = 0; i < 18; ++i) {
						debug_msg >> std::to_string(i) << ": " << result[i];
					}
					CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-BangumiSQLFindSubject", debug_msg);
				}
#endif


				std::string name = result[3];

				std::string out = code_converter.Conv(name);
				std::cout << "utf-8-->gb2312 in=" << name << ",out=" << out << std::endl;
				//=====
				////std::string x = "시진핑 총서기가 인터넷안전정보화작업 ";
				//std::wstring xx = L"시진핑 총서기가 인터넷안전정보화작업 ";
				//std::string y = boost::locale::conv::from_utf(xx, "gb18030");
				////y = boost::locale::conv::between(x, "gb18030","gbk", boost::locale::conv::method_type::skip);
				////y = utf8_to_gb2312(x);
				//std::cout <<std::endl << y << std::endl;
				//std::wcout << xx << std::endl ;
				//SendMsg.at(BgmRetType::Private)(ac, 597320012, xx.c_str());
			}
			catch (boost::system::system_error&e) {
				std::cout << "Error: " << e.what();
			}
			catch (std::runtime_error&e) {
				std::cout << "Error: " << e.what();
			}

		}
		
	}
//
const char* raw_str="0000";
//std::string str;
std::list<bangumi::Code> code_pool;
std::set<BgmCode> bgm_code;
BgmRetType type = BgmRetType::Private;
const int64_t qq = 597320012;
const int64_t group = 0;
//额外的信息
bangumi::BGMCodeExtraVar extra;
extra.refresh = true;
bangumi::BGMCodeParam param(extra,raw_str,code_pool,bgm_code,type,qq,group);

std::set<size_t> param_id;
std::set<std::string> param_str;
//param_id.emplace(1851);
//param_str.emplace("240760/fin/8/和上一季一样精彩!");
//param_str.emplace("%Angel+beats/s");
param_str.emplace("wz97315");
//Test API Search
{
	//BGM_API_Subject(param, param_id, param_str);
	//BGM_API_Search(param, param_id, param_str);
	//BGM_API_Collection(param, param_id, param_str);
	//BGM_API_Update(param, param_id, param_str);
	//BGM_API_Tag(param, param_id, param_str);
}
{
	//TestTimeWoker();
	//param.extra.countdown = 15;
	//time_worker.AddAPIFunc(std::cref(bangumi::BGM_API_User),param, param_id, param_str);
	//time_worker.Print();
	//boost::this_thread::sleep(boost::posix_time::seconds(20));
	//time_worker.Print();
	//boost::this_thread::sleep(boost::posix_time::seconds(100));
}
//
//{
//	Parsing(11, 1, 597320012, ":++249637 _10", false);
//	boost::this_thread::sleep(boost::posix_time::seconds(150));
//}
{
	//Parsing(11, 1, 597320012, ":tag 原创/2019-4/2", false);
	//Parsing(11, 1, 597320012, ":++ /fin/8.5", false);
	//Parsing(11, 1, 597320012, ":1user5", false);
	//boost::this_thread::sleep(boost::posix_time::seconds(150));
	std::string html = bangumi::GetSubjectHtml(249637);

	bangumi::string staff = Resolve::ResolveStaff(html);

	std::cout << staff << std::endl;
}
//Test API Subject
//{
//	BGM_API_Subject(param, param_id, param_str);
//}
//{
//	//117206
//	//277249
//	//66410
//	std::string html = bangumi::GetSubjectHtml(218711);
//	//std::cout << html;
//
//	//bangumi::string res = Resolve::ResolveSubjectTag(html);
//	//bangumi::string res = Resolve::ResolveSubjectCharacter(html);
//	bangumi::string res = Resolve::ResolveSubjectComment(html);
//	std::cout << res;
//}
//Test for reg
//{
//	BGM_API_Auth(param, param_id, param_str);
//}
//test iconv
//
//{
//	char *in_utf8 = "姝ｅ?ㄥ??瑁?";
//	char *in_gb2312 = "正在安装";
//	char out[255];
//
//	CodeConverter cc("utf-8", "gb18030");
//	cc.convert(in_utf8, strlen(in_utf8), out, 255);
//	std::cout << "utf-8-->gb2312 in=" << in_utf8 << ",out=" << out << std::endl;
//
//	// gb2312-->utf-8
//	CodeConverter cc2 = CodeConverter("gb18030", "utf-8");
//	cc2.convert(in_gb2312, strlen(in_gb2312), out, 255);
//	std::cout << "gb2312-->utf-8 in=" << in_gb2312 << ",out=" << out << std::endl;
//}
	std::system("pause");
}