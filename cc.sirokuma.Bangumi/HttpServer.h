#pragma once
#include "Init.h"
#include "ParserElements.h"
#include <iostream>

using namespace boost;
//typedef void(*BGMServerCallback)(unsigned,std::string,int64_t,std::string);

//加密QQ函数
std::string EncryptQQ(const int64_t &qq) {
	auto &table = num_bimap.left;

	std::string result;
	std::string qq_str = std::to_string(qq);
	for (auto& c : qq_str) {
		result += table.at(c);
	}
	return std::move(result);
}
//解密QQ函数
int64_t DecryptQQ(const std::string &unqq) {
	auto &table = num_bimap.right;

	std::string result;
	for (auto& c : unqq) {
		result += table.at(c);
	}
	return std::stoull(result);

}
//根据QQ生成一个状态
std::string EncryptState(const int64_t &qq) {
	auto &table = num_bimap.left;

	std::string result;
	std::string qq_str = std::to_string(qq);
	result += table.at(qq_str[1]);
	result += table.at(qq_str[2]);
	result += table.at(qq_str[3]);
	result += table.at(qq_str[4]);
	return std::move(result);
}
//https://stackoverflow.com/questions/154536/encode-decode-urls-in-c
//urlencode
std::string url_encode(const std::string &value) {
	std::ostringstream escaped;
	escaped.fill('0');
	escaped << std::hex;

	for (std::string::const_iterator i = value.begin(), n = value.end(); i != n; ++i) {
		std::string::value_type c = (*i);

		// Keep alphanumeric and other accepted characters intact
		if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
			escaped << c;
			continue;
		}

		// Any other characters are percent-encoded
		escaped << std::uppercase;
		escaped << '%' << std::setw(2) << int((unsigned char)c);
		escaped << std::nouppercase;
	}

	return escaped.str();
}
//获得定向网址
std::string GetRedirectUrl(int64_t qq) {
	std::string unqq = EncryptQQ(qq);
	std::string state = EncryptState(qq);

	//std::string redirect_url = bgm.redirect_url;
	std::string redirect_url = bgm.redirect_url + "?nu=" + unqq + "&state=" + state;
	//进行URL编码,否则会导致回调地址无法携带参数->? &
	redirect_url = url_encode(redirect_url);
	return std::move(redirect_url);
}



class Service {

public:
	//构造函数
	Service(std::shared_ptr<boost::asio::ip::tcp::socket> sock) :
		m_sock(sock),
		m_response_status_code(200) // 假定成功
	{};


	void start_handling() {

#ifndef NDEBUG
		{
			bangumi::string debug_msg;
			debug_msg << "创建一个start_handling流程成功";
			CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTPServer", debug_msg);
		}
#endif
		//异步读取第一行,实际上一般已经读完了全部
		asio::async_read_until(*m_sock.get(),
			m_request,
			"\r\n",
			[this](
				const boost::system::error_code& ec,
				std::size_t bytes_transferred)
		{
			//第一行读取完的后数据处理
			on_request_line_received(ec,
				bytes_transferred);
		});
	}

private:
	void on_request_line_received(
		const boost::system::error_code& ec,
		std::size_t bytes_transferred)
	{
#ifndef NDEBUG
		{
			bangumi::string debug_msg;
			debug_msg << "on_request_line_received:读取的字符:" << std::string(boost::asio::buffers_begin(m_request.data()),
				boost::asio::buffers_end(m_request.data()));;
			CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTPServer", debug_msg);
		}
#endif
		if (ec.value() != 0) {
			////有读取时有错误发生
			////直接向上抛出
			//throw ec;


			m_response_status_code = 0;
			//一旦有错误就on_finish结束
			on_finish();
			return;

		}
		//解析请求行
		std::string request_line;
		std::istream request_stream(&m_request);
		std::getline(request_stream, request_line, '\r');
		//移除下一个\n字符
		request_stream.get();
		//解析请求
		std::string request_method;
		std::istringstream request_line_stream(request_line);
		request_line_stream >> request_method;
		//只接收Get方法
		if (request_method.compare("GET") != 0) {
			//有问题on_finish
			m_response_status_code = 0;
			on_finish();
			return;
		}
		//请求的资源,主要以这个为关键
		request_line_stream >> m_requested_resource;
		std::string request_http_version;
		request_line_stream >> request_http_version;
		//Http版本 //不检查,php中的file get contents是1.0的...
		//if (request_http_version.compare("HTTP/1.1") != 0) {
		//	//有问题on_finish
		//	m_response_status_code = 0;
		//	on_finish();
		//	return;
		//}
#ifndef NDEBUG
		{
			bangumi::string debug_msg;
			debug_msg << "收到请求"
				>> "request_method = " << request_method
				>> "request_http_version = " << request_http_version
				>> "m_requested_resource = " << m_requested_resource;
			CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTPServer", debug_msg);
		}
#endif
		process_request();
		send_response();
		on_finish();
		//接下来应当读取头部
		//asio::async_read_until(*m_sock.get(),
		//	m_request,
		//	"\r\n\r\n",
		//	[this](
		//		const boost::system::error_code& ec,
		//		std::size_t bytes_transferred)
		//{
		//	on_headers_received(ec,
		//		bytes_transferred);
		//});

	}


	void process_request() {

		//GET /oauth/authorize?client_id=$client_id&response_type=code&redirect_uri=$encode_url&state=$state HTTP/1.1
		//bangumi_apply_auth.php?code=$user_code&to_code=$to_code&state=$state
		//解析uri
		size_t start_pos;
		size_t temp;
		//先找到?
		auto first_para_start = m_requested_resource.find_first_of('?');
		//找请求到的code起始位置
		start_pos = m_requested_resource.find("code=", first_para_start) + 5;
		//找到下一个$
		temp = m_requested_resource.find_first_of('&', start_pos);
		code = m_requested_resource.substr(start_pos, temp - start_pos);

		//找请求到的to_code起始位置(加密后的qq)
		start_pos = m_requested_resource.find("to_code=", temp) + 8;
		//找到下一个$
		temp = m_requested_resource.find_first_of('&', start_pos);
		std::string unqq = m_requested_resource.substr(start_pos, temp - start_pos);
		if (unqq.empty()) {
			//说明是错误的访问，即没有登陆导致的信息丢失
			//非法的请求
			//直接清除
#ifndef NDEBUG
			{
				bangumi::string debug_msg;
				debug_msg << "存在非法的reg请求";
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTPServer", debug_msg);
			}
#endif
			m_sock->shutdown(
				asio::ip::tcp::socket::shutdown_receive);
			//delete this;
			return;
		}
		//解密QQ
		qq = DecryptQQ(unqq);


		//找请求到的state起始位置(验证信息)
		start_pos = m_requested_resource.find("state=", temp) + 6;
		//结束位置
		//temp = m_requested_resource.length();
		//sate是固定的位数
		state = m_requested_resource.substr(start_pos, 4);
		if (state.empty()) {
			//说明是错误的访问，即没有登陆导致的信息丢失
			//非法的请求
			//直接清除
#ifndef NDEBUG
			{
				bangumi::string debug_msg;
				debug_msg << "存在非法的reg请求";
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTPServer", debug_msg);
			}
#endif
			m_sock->shutdown(
				asio::ip::tcp::socket::shutdown_receive);
			//delete this;
			return;
		}
		//验证
		if (EncryptState(qq) != state) {
			//非法的请求
			//直接清除
#ifndef NDEBUG
			{
				bangumi::string debug_msg;
				debug_msg << "存在非法的reg请求";
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTPServer", debug_msg);
			}
#endif
			m_sock->shutdown(
				asio::ip::tcp::socket::shutdown_receive);
			//delete this;
			return;
		}

#ifndef NDEBUG
		{
			bangumi::string debug_msg;
			debug_msg << "HTTPServer解析请求成功!";
			debug_msg >> "qq = " << std::to_string(qq)
				>> "state = " << state
				>> "code = " << code;
			CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTPServer", debug_msg);
		}
#endif
	}

	void send_response() {
		m_sock->shutdown(
			asio::ip::tcp::socket::shutdown_receive);

		//不管结果如何直接返回200
		asio::const_buffer response_buffer = asio::buffer("HTTP/1.1 200 OK");


		//异步写入
		asio::async_write(*m_sock.get(),
			response_buffer,
			[this](
				const boost::system::error_code& ec,
				std::size_t bytes_transferred)
		{
#ifndef NDEBUG
			{
				bangumi::string debug_msg;
				debug_msg << "成功向PHP发送200响应!";
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTPServer", debug_msg);
			}
#endif
			//不关心结果
		});
	}


	//结束
	void on_finish() {
		//
		if (m_response_status_code != 200) {
			//说明失败
#ifndef NDEBUG
			{
				bangumi::string debug_msg;
				debug_msg << "on_finish失败:读取的字符:" << std::string(boost::asio::buffers_begin(m_request.data()),
					boost::asio::buffers_end(m_request.data()));;
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTPServer", debug_msg);
			}
#endif

#ifndef NDEBUG
			{
				bangumi::string debug_msg;
				debug_msg << "on_finish 失败处理";
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTPServer", debug_msg);
			}
#endif
			bangumi::string msg = "失败";
			SendMsg.at(BgmRetType::Private)(ac, qq, msg);
		}
		else {
			//Main Func
			//说明成功
			//接着继续请求获得一个token
			//=============================
			bangumi::string content;
			content << "grant_type=authorization_code"
				<< "&code=" << code
				<< "&redirect_uri=" << GetRedirectUrl(qq)
				<< "&client_id=" << bgm.bangumi_client_id
				<< "&client_secret=" << bgm.bangumi_client_secret;

			//post中的www编码其实是为了防止URI过长而将参数放在了content中,POST时一定不能忘记使用Content-Length指定大小
			//否则会出现不能正常读取参数的问题
			bangumi::string header("Cache-Control: no-cache\r\n"
				"Content-Type: application/x-www-form-urlencoded\r\n");
			header << "Content-Length: " << content.length() << "\r\n";

			

			//创建一个请求
			std::shared_ptr<HTTPRequest> request_one =
				http_client.create_request_fixed(http_client.GetID());

			//必要的信息都拥有,直接构造一个bangumi::BGMRetParam
			request_one->set_ret_param(bangumi::BGMRetParam{ BgmRetType::Private , qq, 0, 0, "", bangumi::BGMCodeExtraVar() });
			request_one->set_host("bgm.tv");
			request_one->set_uri("/oauth/access_token");
			//只有搜索条目时需要Cookie: chii_searchDateLine
			request_one->set_request(request_message(request_one, HTTP_WAY::POST,
				header,
				content));
			//设置回调函数
			request_one->set_callback([](std::shared_ptr<HTTPRequest>request_one, bangumi::BGMRetParam param,int ec) {
				//要回复的string
				bangumi::string msg;
				//回调函数:此时已经完成了响应报文的读取
				std::string json;
				try {
					GetResponseContent(request_one, json,true,ec);
#ifndef NDEBUG
					{
						bangumi::string debug_msg;
						debug_msg << "收到响应的Json"
							>> json.c_str();
						CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTP", debug_msg);
					}
#endif

					//一个标记已注册用户
					bool registed = false;
					//绑定的是否是同一个ID
					bool same_bgm_id = true;

					auto auth = Resolve::Resolve_Auth(json);
					//检查是否存在已注册的QQ用户

					bangumi::string pre_query;
					pre_query << "SELECT user_id, user_qq, user_bangumi FROM bgm_users WHERE "
						<< "user_qq = " << std::to_string(param.qq);
					//MYSQL_RES* pre_result = nullptr;
					BGMSQLResult pre_result;
					unsigned long pre_affect_rows_num = sql_pool.ExecQuery(pre_query, pre_result);
					try
					{
						SQLCheckResult(pre_affect_rows_num);
						//当影响行数大于0时才有注册
						if (pre_affect_rows_num > 0) {
							//一个标记已注册用户
							registed = true;
							//使用fetch_row各方面相对较好
							//MYSQL_ROW rows = mysql_fetch_row(pre_result);
							//判断前后两个ID是否一致
							if (std::to_string(auth.user_id) != std::string(pre_result[2])) {
								same_bgm_id = false;
							}
							//已注册的QQ提示消息
							msg << "原[" << pre_result[0] << "]号契约"
								<< ": QQ: " << pre_result[1]
								<< " x Bangumi-ID: " << pre_result[2];
							//注意最后再free,free以后rows也会失效
							//mysql_free_result(pre_result);
						}

					}
					catch (boost::system::system_error& e)
					{
#ifndef NDEBUG
						{
							bangumi::string debug_msg;
							debug_msg << "Pre-SQLCheckResult中查询失败:"
								<< std::to_string(pre_affect_rows_num)
								>> e.what();

							CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Auth-SQL-Pre-SQLCheckResult", debug_msg);
						}
#endif	
						//首先free
						//mysql_free_result(pre_result);
						//msg << "此次契约好像出了点小问题...\n";
						//throw e;
					}


					//向SQL更新
					//++++++++++++++++++++
					bangumi::string query;
					//query << "insert into bgm_users(user_qq,user_bangumi,user_access_token,user_refresh_token)"
					//	<< "values(" << std::to_string(param.qq) << ", " << std::to_string(auth.user_id)
					//	<< ", " << auth.access_token << ", " << auth.refresh_token << ")";
					//INSERT INTO bgm_users VALUES(NULL,123456789,12332,"d22a58aacee7925860014521b37a8cf78aa4afd9","f606d13c8493ca12f256fb8a43c54f36b2edd666",0,0,NULL,'1000-01-01 00:00:00',0)
					if (registed) {
						//再次注册
						query << "UPDATE bgm_users SET "
							<< "user_bangumi="<<auth.user_id<<","
							<< "user_access_token='" << auth.access_token <<"',"
							<< "user_refresh_token='" << auth.refresh_token <<"'"
							<< " WHERE user_qq="<<param.qq;
					}
					else {
						//首次注册
						query << "INSERT INTO bgm_users(user_id,user_qq,user_bangumi,user_access_token,user_refresh_token)"
							<< "VALUES("
							<< "NULL,"
							<< std::to_string(param.qq) << ","
							<< std::to_string(auth.user_id) << ","
							<< "'" << auth.access_token << "',"
							<< "'" << auth.refresh_token << "')";
							//<< "0)";
							//<< "0,"
							//<< "NULL,"
							//<< "'1000-01-01 00:00:00',"
							//<< "0)";
					}


#ifndef NDEBUG
					{
						bangumi::string debug_msg;
						debug_msg << "SQL语句:"
							>> query;
						CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Auth-SQL", debug_msg);
					}
#endif
					//std::string query("CREATE TABLE debug_example (id int not null, my_name varchar(50)");
					//用于存储结果
					BGMSQLResult result;
					unsigned long affect_rows_num = sql_pool.ExecQuery(query, result);
					try
					{
						SQLCheckResult(affect_rows_num);

#ifndef NDEBUG
						{
							bangumi::string debug_msg;
							debug_msg << "影响的行数:"
								<< std::to_string(affect_rows_num);
							CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Auth-SQL", debug_msg);
						}
#endif
						//mysql_free_result(result);
						//成功的QQ消息构造
						if (registed) {
							//再次注册
							if (same_bgm_id) {
								//相同ID的刷新行为
								msg << " 已续签!";
							}
							else {
								//更改绑定行为
								msg << " 失效..."
									>> "--------"
									>> "现契约: QQ: " << std::to_string(param.qq) 
									<< " x Bangumi-ID: " << auth.user_id << " 生效中!";
							}

						}
						else {
							//首次注册
							msg << "QQ: " << std::to_string(param.qq)
								<< " x Bangumi-ID: " << auth.user_id
								<< " 完成缔约!";
						}

					}
					catch (boost::system::system_error& e)
					{
#ifndef NDEBUG
						{
							bangumi::string debug_msg;
							debug_msg << "SQLCheckResult检查有问题: "
								<< std::to_string(affect_rows_num);

							CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Auth-SQL-SQLCheckResult", debug_msg);
						}
#endif	
						//首先free再次抛出
						//mysql_free_result(result);
						//失败的QQ消息构造
						msg << "此次契约好像出了点小问题...\n";
						throw e;
					}
					//++++++++++++++++++++



				}
				catch (boost::system::system_error & e) {
					//如果发生了异常,使用异常回复(包括没有找到用户等)
					//这里不使用Reply类而是直接赋值回复
					//此处可能触发授权失败回复，即使已经完成了，因此无视这样的异常
					if (e.code() != boost::system::system_error(bangumi_bot_errors::auth_request_error).code()) {

						msg << e.what();
						//msg << "[" << param.cur_str << "]...";
					}
				}

				//发送回复
				if (!msg.empty()) {
					SendMsg.at(BgmRetType::Private)(ac, param.qq, msg);
				}
				//从httpclient移除引用从而析构自身
				
				http_client.RemoveID(request_one->get_id());
			});
			request_one->execute();
			//=============================

			//回复QQ消息 DEBUG
			//bangumi::string msg;
			//msg << "code = " << code
			//	>> "state = " << state;
			//SendMsg.at(BgmRetType::Private)(ac, qq, msg);
		}
		//清除自身
		delete this;
	}


private:
	std::shared_ptr<boost::asio::ip::tcp::socket> m_sock;
	boost::asio::streambuf m_request;
	//std::map<std::string, std::string> m_request_headers;
	std::string m_requested_resource;
	//std::unique_ptr<char[]> m_resource_buffer;
	unsigned int m_response_status_code;
	//std::size_t m_resource_size_bytes;
	//std::string m_response_headers;
	//std::string m_response_status_line;

	int64_t qq = 0;
	std::string code = "";
	std::string state = "";
};
//一个TCP套接字接收器
class Acceptor {
public:
	//接收器构造函数
	Acceptor(asio::io_service& ios, unsigned short port_num)
		:m_ios(ios)
		//m_acceptor(m_ios, asio::ip::tcp::endpoint(asio::ip::address_v4::any(), port_num))
	{
		m_acceptor.reset(new asio::ip::tcp::acceptor(m_ios, asio::ip::tcp::endpoint(asio::ip::address_v4::any(), port_num)));
		m_isStopped.store(false);
	}
	//开始处理
	void Start() {
#ifndef NDEBUG
		{
			bangumi::string debug_msg;
			debug_msg << "Acceptor Start~";
			CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTPServer-Acceptor", debug_msg);
		}
#endif
		//接收器不同于socket,除了open之外还需要listen
		m_acceptor->listen();
		InitAccept();
	}

	void Stop() {
		//m_acceptor->cancel();
		m_isStopped.store(true);
	}
private:
	void InitAccept() {
		//首先创建一个sock以当监听到一个连接时用于处理数据传递
		std::shared_ptr<asio::ip::tcp::socket>
			sock(new asio::ip::tcp::socket(m_ios));

		//异步accept,并将上面的sock用于数据传递
		m_acceptor->async_accept(*sock,
			[this, sock](const boost::system::error_code& error) {
			//成功accept的回调函数
#ifndef NDEBUG
				{
					bangumi::string debug_msg;
					debug_msg << "Acceptor接受了一个请求";
					CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTPServer-Acceptor", debug_msg);
				}
#endif
				if (!m_isStopped.load())
				{
#ifndef NDEBUG
					{
						bangumi::string debug_msg;
						debug_msg << "m_isStopped 为 false: 执行onAccept";
						CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTPServer-Acceptor", debug_msg);
					}
#endif
					onAccept(error, sock);
					return;
				}
				else {
#ifndef NDEBUG
					{
						bangumi::string debug_msg;
						debug_msg << "m_isStopped 为 true: 不执行onAccept并直接退出";
						CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTPServer-Acceptor", debug_msg);
					}
#endif
					return;
				}

		});

		//同步也有问题
		//同步accept
		//boost::system::error_code error;
		//m_acceptor.accept(*sock, error);
		//onAccept(error, sock);
	}

	void onAccept(const boost::system::error_code&ec,
		std::shared_ptr<asio::ip::tcp::socket>sock) {
		if (ec.value() == 0) {
			//没有错误的话,正常进入流程
			(new Service(sock))->start_handling();
		}

		else {
#ifndef NDEBUG
			{
				bangumi::string debug_msg;
				debug_msg << "error code = " << std::to_string(ec.value())
					>> "error msg = " << ec.message();
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTPServer-Acceptor-onAccept", debug_msg);
			}
#endif
			//直接返回
			return;
		}
		//如果没有调用Stop可以一次处理多个请求,每一个请求一个Init
		//不使用stop了,在有效时间内能无限响应
		if (!m_isStopped.load() && m_acceptor != nullptr) {
			InitAccept();
		}
		else {
			//此处不应该由自己关闭,应当由异步ios处理并关闭,否则那边会异常
			//if (m_acceptor != nullptr)
			//	m_acceptor->close();

		}
	}
private:
	asio::io_service &m_ios;
	std::unique_ptr<asio::ip::tcp::acceptor> m_acceptor;
	std::atomic<bool> m_isStopped;
};

class HTTPServer {
public:
	HTTPServer(asio::io_service& ios) :m_ios(ios)
	{
		//m_work.reset(new asio::io_service::work(m_ios));
		//无法在此处进行计时器的初始化
		//timer = std::shared_ptr<asio::steady_timer>(new asio::steady_timer(m_ios));
	}
	~HTTPServer() {
		Stop();
	}
	void Start(unsigned short port_num) {
#ifndef NDEBUG
		{
			bangumi::string debug_msg;
			debug_msg << "HTTPServer Start~";
			CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTPServer", debug_msg);
		}
#endif
		if (m_timer==nullptr) {
			std::shared_ptr<asio::steady_timer> timer(new asio::steady_timer(m_ios));
			m_timer = timer;
		}
		if (acc != nullptr) {
			//如果已经有在Start了,则忽视
			return;
		}
		acc.reset(new Acceptor(m_ios, port_num));
		acc->Start();
		//定时Stop
		m_timer->expires_from_now(boost::asio::chrono::minutes(5));
		m_timer->async_wait([this](const boost::system::error_code&) {


#ifndef NDEBUG
			{
				bangumi::string debug_msg;
				debug_msg << "HTTPServer 由于超时而Stop~";
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTPServer-Start", debug_msg);
			}
#endif
			this->Stop();
		});
		

		//这里由于已经有了HTTP_Client的ios在run(),因此不再创建Ios
		//for (unsigned int i = 0; i < thread_pool_size; i++) {
		//	std::unique_ptr<std::thread> th(
		//		//https://blog.csdn.net/jlusuoya/article/details/75299096
		//		//捕获列表
		//		new std::thread([this]() {
		//		m_ios.run();
		//	})
		//	);
		//	m_thread_pool.push_back(std::move(th));
		//}

		//在Start设置运行时间
		//boost::this_thread::sleep(boost::posix_time::seconds(60));
		//


	}

	void Stop() {
		if (acc != nullptr) {
			acc->Stop();
			acc.reset(nullptr);
		}

		//m_ios.stop();
#ifndef NDEBUG
		{
			bangumi::string debug_msg;
			debug_msg << "HTTPServer Stop~";
			CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTPServer", debug_msg);
		}
#endif
		//for (auto&th : m_thread_pool) {
		//	th->join();
		//}
	}


private:
	//m_ios由HttpClient传递构造
	asio::io_service& m_ios;
	//借用HttpClient的ios原因,不需要work
	//std::unique_ptr<asio::io_service::work>m_work;
	std::unique_ptr<Acceptor>acc;
	//std::vector<std::unique_ptr<std::thread>>m_thread_pool;
	std::shared_ptr<asio::steady_timer> m_timer;
};

//创建全局变量
HTTPServer http_server(http_client.GetIOS());


int Test_HTTPServer() {
	//const unsigned int DEFAULT_THREAD_POOL_SIZE = 2;

	try {
		HTTPServer srv(http_client.GetIOS());

		//通常在并行应用程序中用于查找最佳线程数数的通用公式是计算机乘以2的处理器数
		//使用std::thread::hardware_concurrency静态方法来获取处理器的数量
		//但是,此方法可能返回0
		//unsigned int thread_pool_size =
		//	std::thread::hardware_concurrency() * 2;

		//if (thread_pool_size == 0)
		//	thread_pool_size = DEFAULT_THREAD_POOL_SIZE;

		srv.Start(bgm.server_port_num);

		//std::this_thread::sleep_for(std::chrono::seconds(60));
		//boost::this_thread::sleep(boost::posix_time::seconds(60));

		//srv.Stop();
	}
	catch (system::system_error&e) {
		std::cout << "Error code = "
			<< e.code() << ", Message = "
			<< e.what();

		return e.code().value();
	}

	return 0;
}