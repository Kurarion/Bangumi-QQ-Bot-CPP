#include "GlobalVariable.h"
#include "boost/asio.hpp"
#include "boost/thread.hpp"
#include <iostream>


using namespace boost;
#ifndef BANGUMI_HTTP_H
#define BANGUMI_HTTP_H

//#ifndef NDEBUG
//bangumi::string debug_code_set = "CodeSet: ";
//DEBUG_STRING_SET(code_set, debug_code_set);
//CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-BGMCode-AnalyseCode", debug_code_set);
//#endif

//前向声明
class HTTPClient;
class HTTPRequest;
class HTTPResponse;
//枚举
enum class HTTP_WAY {
	POST,
	GET
};
//Content-Type
//static const std::map<std::string, std::string> Content_Type{
//	{ "image/png",".png" },
//	{ "image/jpeg",".jpg" },
//	{ "text/html",".jpg" }
//};

//回调函数类型
//TODO:正确的函数签名
typedef void(*BGMCallback)(std::shared_ptr<HTTPRequest>, bangumi::BGMRetParam, int ec);
//在Request之后存储响应消息需要
class HTTPResponse {
	//设置HTTPRequest为友元
	//HttpRequest中有本类的实例对象
	friend class HTTPRequest;
	//用于读取Response中的报文信息
	friend void GetResponseContent(std::shared_ptr<HTTPRequest> request, std::string &s, bool isChunk, int ec);

	HTTPResponse()
		//关联streambuf到标准输入流中
		:m_response_stream(&m_response_buf)
	{}
public:
	//取得响应报文中头部的状态码
	unsigned int get_status_code()const {
		return m_status_code;
	}
	//取得响应报文中头部的状态消息
	const std::string& get_status_message()const {
		return m_status_message;
	}
	//返回响应报文的头部中各项属性以及对应的值
	const std::map<std::string, std::string>& get_headers() {
		return m_headers;
	}
	//返回一个std域中的标准输入流以输出asio::streambuf中的信息
	const std::istream&get_response()const {
		return m_response_stream;
	}
	////清空自身
	//void Clear() {
	//	m_status_code = 0;
	//	m_status_message = "";
	//	m_headers.clear();
	//	std::string t;
	//	while (std::getline(m_response_stream,t)) {
	//		//清空buffer
	//	}
	//	m_response_stream.clear();
	//	//m_response_buf.consume(m_response_buf.size());
	//	
	//}
private:
	//获取asio::streambuf以作为参数完成从socket中读取数据到buffer中
	//最终存放除头部的内容(头部等信息会随istream pop出)
	boost::asio::basic_streambuf<std::allocator<char>>&get_response_buf() {
		return m_response_buf;
	}
	//设置响应报文的状态码
	void set_status_code(unsigned int status_code) {
		m_status_code = status_code;
	}
	//设置响应报文的状态信息
	void set_status_message(const std::string& status_message) {
		m_status_message = status_message;
	}
	//添加响应报文的头部信息
	void add_header(const std::string& name, const std::string& value) {
		m_headers[name] = value;
	}

private:
	unsigned int m_status_code;
	std::string m_status_message;

	std::map<std::string, std::string>m_headers;
	boost::asio::basic_streambuf<std::allocator<char>> m_response_buf;
	std::istream m_response_stream;
};
//请求类
//继承enable_shared_from_this,主要用于在on_finish中调用函数时返回自身的智能指针,避免一些可能的错误
class HTTPRequest : public std::enable_shared_from_this<HTTPRequest> {
	//友元类是HTTPClient
	friend class HTTPClient;
	//用于根据实例信息生成request报文
	friend std::string request_message(std::shared_ptr<HTTPRequest> request, HTTP_WAY way, std::string header, std::string content);
	//用于读取Response中的报文信息
	friend void GetResponseContent(std::shared_ptr<HTTPRequest> request, std::string &s, bool isChunk, int ec);
	//设置默认端口为80
	static const unsigned int DEFAULT_PORT = 80;
	//构造函数
	//@para1: io_serivce
	//@para2: id代表线程的标识
	//构造函数中会初始化Socket,DNS解析类(Resolver)

	HTTPRequest(asio::io_service& ios, unsigned int id) :
		m_port(DEFAULT_PORT),
		m_id(id),
		m_sock(ios),
		m_resolver(ios),
		m_ios(ios),
		m_withoutResolver(false),
		m_isPersistent(false),
		m_callback(nullptr)
	{}

	//重载构造函数已创建一个固定URL请求
	//同时也无需给Host赋值
	HTTPRequest(asio::io_service& ios, unsigned int id, std::string url, const boost::asio::ip::tcp::resolver::iterator& it) :
		m_port(DEFAULT_PORT),
		m_id(id),
		m_sock(ios),
		m_resolver(ios),
		m_ios(ios),
		m_withoutResolver(true),
		m_isPersistent(false),
		m_it(it),
		m_host(url),
		m_callback(nullptr)
	{}
public:
	//设置请求报文中的Host
	void set_host(const std::string& host) {
		m_host = host;
	}
	//设置端口号
	void set_port(unsigned int port) {
		m_port = port;
	}
	//设置URI统一资源标识符
	void set_uri(const std::string& uri) {
		m_uri = uri;
	}
	//设置请求报文
	void set_request(std::string request) {
		m_request_buf = request;
	}
	//返回Response
	const HTTPResponse& get_response_class() {
		return m_response;
	}
	//返回Host
	std::string get_host() const {
		return m_host;
	}
	//返回Request
	std::string get_request() const{
		return m_request_buf;
	}
	//返回Port
	unsigned int get_port() const {
		return m_port;
	}
	//返回URI
	const std::string& get_uri() const {
		return m_uri;
	}
	//返回线程ID
	unsigned int get_id() const {
		return m_id;
	}
	//设置回调函数
	void set_callback(BGMCallback callback) {
		m_callback = callback;
	}
	//设置回复结构体
	void set_ret_param(bangumi::BGMRetParam param) {
		m_bgm_ret_param = param;
	}
	//执行此次请求
	void execute() {
		//保证正确的请求
		assert(m_port > 0);
		assert(m_host.length() > 0);
		assert(m_uri.length() > 0);

		//如果是一个持久连接
		if (m_isPersistent) {
			//直接调用连接建立的回调函数
			on_connection_established(boost::system::error_code(), m_it);
			return;
		}

		//如果不重新解析
		if (m_withoutResolver) {
			//直接调用解析后的回调函数
			//使用默认初始的error_code
			//此时m_it已经被Client检查过了此处无需检查
			on_host_name_resolved(boost::system::error_code(), m_it);
			return;
		}

		//解析DNS域名,创建解析查询
		//asio::ip::tcp::resolver::query resolver_query(m_host,
		//	std::to_string(m_port),
		//	//此flag参数说明第二个参数是一个数字端口
		//	asio::ip::tcp::resolver::query::numeric_service);


		asio::ip::tcp::resolver::query resolver_query(m_host, std::to_string(m_port));
		asio::ip::tcp::resolver::iterator start = m_resolver.resolve(resolver_query);
		asio::ip::tcp::resolver::iterator end;
		//test
		for (auto it = start; it != end; ++it) {
			std::cout << it->endpoint().address() << std::endl;
		}

		//解析主机名,同时完成后调用on_host_name_resolved
		//会传递解析后的IP迭代器
		m_resolver.async_resolve(resolver_query,
			[this](const boost::system::error_code& ec,
				asio::ip::tcp::resolver::iterator iterator)
		{
#ifndef NDEBUG
			std::string str2 = "async_resolve -> Callback  : 域名解析";
			std::cout << str2 << std::endl;
			CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTP", str2.c_str());
#endif
			on_host_name_resolved(ec, iterator);
		});
	}



private:
	void on_host_name_resolved(
		const boost::system::error_code& ec,
		asio::ip::tcp::resolver::iterator iterator)
	{
		if (ec.value() != 0) {
			on_finish(ec);
			return;
		}

		//连接到第一个有效的IP
		asio::async_connect(m_sock,
			iterator,
			[this](const boost::system::error_code& ec,
				asio::ip::tcp::resolver::iterator iterator)
		{
#ifndef NDEBUG
			std::string str2 = "async_connect -> Callback  : 连接有效IP";
			CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTP", str2.c_str());
#endif
			on_connection_established(ec, iterator);
		});
	}

	void on_connection_established(
		const boost::system::error_code& ec,
		asio::ip::tcp::resolver::iterator iterator)
	{
		if (ec.value() != 0) {
			on_finish(ec);
			return;
		}
		//
		//[]已转移至public的set_request中设置
		//设置请求报文
		//m_request_buf += "GET " + m_uri + " HTTP/1.1\r\n";
		//m_request_buf += "Host: " + m_host + "\r\n";
		//m_request_buf += "\r\n";
		//如果请求报文为空,抛出异常
		if (m_request_buf.empty()) {
			throw boost::system::system_error(bangumi_bot_errors::empty_request);
		}


		//异步发送
		asio::async_write(m_sock,
			asio::buffer(m_request_buf),
			[this](const boost::system::error_code& ec,
				std::size_t bytes_transferred)
		{
#ifndef NDEBUG
			std::string str2 = "async_write -> Callback  : 异步发送请求";
			str2 += "\n";
			str2 += m_request_buf;
			CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTP", str2.c_str());
#endif
			on_request_sent(ec, bytes_transferred);
		});
	}

	void on_request_sent(const boost::system::error_code& ec,
		std::size_t bytes_transferred)
	{
		if (ec.value() != 0) {
			on_finish(ec);
			return;
		}
		//这里不能直接shutdown(send)
		//可能会造成收不到Response
		//原因可能是因为在异步中以下这个shutdown函数执行相比async_write()更先
		//m_sock.shutdown(asio::ip::tcp::socket::shutdown_send);

		//
		//注意这里已经读取了所有的头部
		//有关报文的结构 https://www.cnblogs.com/rainydayfmb/p/5319318.html
		asio::async_read_until(m_sock,
			m_response.get_response_buf(),
			"\r\n",
			[this](const boost::system::error_code& ec,
				std::size_t bytes_transferred)
		{
			//[注意]
			//因此为了保证在释放套接字资源之前正确关闭发送
			//在回调函数中shutdown()
			//如果不是持久连接则调用shutdown
			//[注意]这里必须shutdown,否则会导致卡在读取头部中...
			if (!m_isPersistent)
				m_sock.shutdown(asio::ip::tcp::socket::shutdown_send);

			//破坏性debug
			//std::istream input(&m_response.get_response_buf());
			//std::vector<std::string> x;
			//while (input) {
			//	std::string temp;
			//	input >> temp;
			//	x.push_back(std::move(temp));

			//}
			//for each (auto c in x)
			//{

			//	DMESSAGE(c);
			//}
			//std::istream xxx(&m_response.get_response_buf());


			//实测在这里就直接读完了所有的头部
			//这是因为read_until的特性(可能会超读)
			//istream的rdbuf()方法也会推动流的前进

#ifndef NDEBUG
			std::string str2 = "async_read_until -> Callback : 报文状态行读取完成";
			CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTP", str2.c_str());
#endif
			on_status_line_received(ec, bytes_transferred);
			//直接调用读取完头部的回调也没什么问题(因为read_until的特性),但这只是测试,并非正确的思想
			//on_headers_received(ec,bytes_transferred);
		});
	}

	void on_status_line_received(
		const boost::system::error_code& ec,
		std::size_t bytes_transferred)
	{

		if (ec.value() != 0) {
			on_finish(ec);
			return;
		}
		//解析状态行
		std::string http_version;
		std::string str_status_code;
		std::string status_message;
		std::istream response_stream(
			&m_response.get_response_buf());
		response_stream >> http_version;

		if (http_version != "HTTP/1.1") {
			//错误的
			on_finish(bangumi_bot_errors::http_error);
			return;
		}
		response_stream >> str_status_code;
		//状态码
		unsigned int status_code = 200;
		try {
			//先转换为值类型
			status_code = std::stoul(str_status_code);
		}
		catch (std::logic_error&) {
			//转换失败
			on_finish(bangumi_bot_errors::http_error);
			return;
		}
		//getline(buf, 1024)，缺省是读到\n，这样就会把 \r 读到本次的buf里面；
		//getline(buf, 1024, '\r')，则会把 \n 读到下一次的buf里面；
		//getline(buf, 1024, "\r\n")则编译的时候报错；注意\r\n是双引号；
		//因此针对\r\n结尾的行先读取\r之前再忽略\n
		std::getline(response_stream, status_message, '\r');
		//忽略下一个字符(\n)
		response_stream.get();

		//std::istream xxx(&m_response.get_response_buf());

		m_response.set_status_code(status_code);
		m_response.set_status_message(status_message);

		//理想情况下,此时已经解析完成了状态行
		//这里的调用是为了读取响应头(实际上此时已经读取完成了,但因为流(streambuf)并没有向前走,数据仍在缓冲中)
		asio::async_read_until(m_sock,
			m_response.get_response_buf(),
			"\r\n\r\n",
			[this](
				const boost::system::error_code& ec,
				std::size_t bytes_transferred)
		{
#ifndef NDEBUG
			std::string str2 = "async_read_until-2 -> Callback : 报文头部读取完成";
			CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTP", str2.c_str());
#endif
			on_headers_received(ec,
				bytes_transferred);
		});
	}

	void on_headers_received(const boost::system::error_code& ec,
		std::size_t bytes_transferred)
	{
		if (ec.value() != 0) {
			on_finish(ec);
			return;
		}
		//解析响应头
		std::string header, header_name, header_value;
		std::istream response_stream(
			&m_response.get_response_buf());
		while (true) {
			std::getline(response_stream, header, '\r');
			//忽略下一个字符(\n)
			response_stream.get();

			if (header == "")
				break;
			size_t separator_pos = header.find(':');
			if (separator_pos != std::string::npos) {
				header_name = header.substr(0,
					separator_pos);
				if (separator_pos < header.length() - 1)
					header_value =
					header.substr(separator_pos + 1);
				else
					header_value = "";
				//最后将响应头存储到响应类HTTPResponse对象中
				m_response.add_header(header_name,
					header_value);
			}
		}
		//重定向Location 301 检测
		//When301();
		try {
			m_response.get_headers().at("Location");
			//不正常,301错误
			//这个回调函数本身也可处理一些异常错误,但主要用于回复
			//此处用作回复
			bgm_error = bangumi_bot_errors::maybe_301_maybe_limit;
			m_callback(shared_from_this(), m_bgm_ret_param, bgm_error);
			//
			on_finish(boost::system::system_error(bangumi_bot_errors::api_301_error).code());
			//
		}
		catch (std::out_of_range) {
			//没有问题,正常
		}

		//此时是读取响应内容
		asio::async_read(m_sock,
			m_response.get_response_buf(),
			[this](
				const boost::system::error_code& ec,
				std::size_t bytes_transferred)
		{
#ifndef NDEBUG
			std::string str2 = "async_read_until-3 -> Callback : 报文正文读取完成";
			CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTP", str2.c_str());
#endif
			//最后，调用on_response_body_received（）方法，通知已收到整个响应消息。
			//因为HTTP服务器可能在它发送响应消息的最后部分之后
			//关闭其套接字的发送部分,所以在客户端,最后的读取操作可能完成
			//错误代码等于asio::error::eof
			on_response_body_received(ec,
				bytes_transferred);
		});

	}

	void on_response_body_received(
		const boost::system::error_code& ec,
		std::size_t bytes_transferred)
	{
		if (ec == asio::error::eof)
			on_finish(boost::system::error_code());
		else
			on_finish(ec);
	}

	void on_finish(const boost::system::error_code& ec)
	{
#ifndef NDEBUG
		{
			std::string x = "进入on_finish处理";
			CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTP-ON-FINISH", x.c_str());
		}
#endif
		if (ec.value() != 0) {
			//TODO:异常处理问题
			//转交上层处理

#ifndef NDEBUG
			std::string x = "Error occured! Error code = "
				+ std::to_string(ec.value()) + ". Message: " + ec.message();
			CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTP-ON-FINISH", x.c_str());
#endif
			//这里不打算交给Func的异常处理
			if (bgm_error == 0)
				RemoveSelf();
			throw boost::system::system_error(ec);
		}

		//没有问题则执行自定义回调函数用于处理消息等等
		if (m_callback != nullptr) {

#ifndef NDEBUG
			{
				std::string x = "执行回调处理函数m_callback";
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTP-ON-FINISH", x.c_str());
			}
#endif
			if(!m_bgm_ret_param.empty())
				//这个回调函数本身也可处理一些异常错误,但主要用于回复
				m_callback(shared_from_this(), m_bgm_ret_param, bgm_error);
			else {
				if (bgm_error == 0)
					RemoveSelf();
				throw boost::system::system_error(bangumi_bot_errors::empty_bgm_ret_param);
			}
		}
			
		else {
			//异常不能使用new
			//TODO,只是测试
			if(bgm_error==0)
				RemoveSelf();
			throw boost::system::system_error(bangumi_bot_errors::empty_callback);
		}

	}
	
	//301处理函数
	//void When301();

	//异常后清除自身索引
	void RemoveSelf();
private:
	std::string m_host;
	unsigned int m_port;
	std::string m_uri;

	unsigned int m_id;
	std::string m_request_buf;

	asio::ip::tcp::socket m_sock;
	asio::ip::tcp::resolver m_resolver;

	HTTPResponse m_response;

	//用于处理HTTP之后的Bot操作
	BGMCallback m_callback;
	//用于接收回复信息的结构体
	bangumi::BGMRetParam m_bgm_ret_param;

	//实际创建ios实例位于HTTPClient中
	asio::io_service &m_ios;
	//为了减少重复的DNS解析,直接在每次首次DNS解析时存储迭代器
	//因为默认构造函数表示一个迭代器的结尾,依此判断是否为空
	//asio::ip::tcp::resolver::iterator iterator;
	//asio::ip::tcp::resolver::iterator iterator_end;
	//是否重新解析DNS
	bool m_withoutResolver;
	//是否是持久连接
	bool m_isPersistent;
	//解析的IP地址
	asio::ip::tcp::resolver::iterator m_it;
	//错误信息存储
	int bgm_error = 0;
};
//HTTP封装类
class HTTPClient {
public:
	HTTPClient() {
		//首先定义一个work对象确保在没有挂起的异步操作时
		//事件循环的线程不会退出此循环
		//一般只要是封装了asio的异步功能的都需要
		//一开始初始化时并没有挂起的异步操作
		m_work.reset(new boost::asio::io_service::work(m_ios));

		//m_thread.reset(new boost::thread([this]() {
		//	//新的线程须有自己的异常处理
		//	//因回调函数都是由ios.run()调用的
		//	try {
		//		m_ios.run();
		//	}
		//	catch (boost::system::system_error& ec)
		//	{
		//		std::string test = "HTTPClient中异步中捕捉到错误,错误ID:" + std::to_string(ec.code().value()) + " 错误信息:" + ec.what();
		//		CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-IOS", test.c_str());
		//	}
		//}));
		m_thread.reset(new boost::thread([this]() {
			RunIos();
		}));


		//TODO创建一个持久连接类
		//HTTPRequest xx
		//xx ->m_isPreXXX = true;

	}
private:
	//不断m_ios.run()
	void RunIos() {
		try {
			m_ios.run();
		}
		catch (boost::system::system_error& ec)
		{
			std::string test = "HTTPClient中异步中捕捉到错误,错误ID:" + std::to_string(ec.code().value()) + " 错误信息:" + ec.what();
			CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-IOS", test.c_str());
			RunIos();
		}
	}
public:
	//仅仅用于HttpServer的初始化
	asio::io_service& GetIOS() {
		return m_ios;
	}
	//[注意]一定别忘记析构,构造函数中还有一个没确定状态的thread运行中
	~HTTPClient() {
		this->close();
	}

	//创建用于解析自定义的的request
	std::shared_ptr<HTTPRequest>
		create_request(unsigned int id)
	{
		auto x = std::shared_ptr<HTTPRequest>(
			new HTTPRequest(m_ios, id));
		//如果没有会对应的索引自动创建,at则不会并会抛出异常
		m_requests_map[id] = x;
		return x;
	}
	//创建用于解析指定的request(指定由其他函数决定)
	std::shared_ptr<HTTPRequest>
		create_request_fixed(unsigned int id)
	{
		//为了防止m_it递进,因此每次使用复制一个it
		asio::ip::tcp::resolver::iterator m_it = fix_it;
		if (m_it != asio::ip::tcp::resolver::iterator()) {
			auto x = std::shared_ptr<HTTPRequest>(
				new HTTPRequest(m_ios, id, m_url, m_it));
			m_requests_map[id] = x;
			return x;
		}
		else {
			//抛出异常
			if (m_url.empty())
				throw boost::system::system_error(bangumi_bot_errors::without_url_in_client);
			else
				throw boost::system::system_error(bangumi_bot_errors::dns_resolve_error_in_client);
		}

	}
	//取得线程ID号
	int GetID() {
		//先上锁,退出域后析构会自动进行解锁
		std::unique_lock<std::mutex>
			thread_lock(id_mutex);
		//现在有map.size()>=this->id,查找一个空闲的(nullptr)的id索引
		//首先增加一个ID数
		++this->id;
		//查找空闲的ID位置
		//当map的容量大于两倍的当前ID总数,则靠前的ID号为nullptr的可能性很大,直接从0号开始查找
		if (2 * id < m_requests_map.size()) {
			unsigned int try_id = -1;
			while (m_requests_map[++try_id] != nullptr)
			{
			}
			return try_id;
		}
		//当两者相近时直接返回m_requests_map.size()
		else {

			return m_requests_map.size();
		}

		//return id;
	}
	//删除ID对应的线程
	void RemoveID(unsigned int this_id) {
		std::unique_lock<std::mutex>
			thread_lock(id_mutex);
		m_requests_map[this_id] = nullptr;
		--this->id;
#ifndef NDEBUG
		{
			bangumi::string debug_msg;
			debug_msg << "RemoveID: " << this->id
				>> "当前池的大小(Remove后): " << m_requests_map.size();
			for (auto &x : m_requests_map) {
				debug_msg >> std::to_string(x.first) << ": " << (x.second == nullptr ? "Null" : "*");
			}
			debug_msg >> "=====";
			CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTP", debug_msg);
		}
#endif
	}
	void close() {
		//销毁m_work对象
		m_work.reset(NULL);
		//等待线程退出
		//线程在所有挂起操作完成后立即退出事件循环
		m_thread->join();
	}
	//以bgm初始化,同时Init BGM中的固定URL的解析
	void Init(BangumiBotVaribel* bgm) {
		m_bgm = bgm;
		//工作路径
		Bangumi_App_Dir = m_bgm->Bangumi_App_Dir;
		//设置请求url
		resolve_url(m_bgm->bangumi_api_url);
		//可能有更多的赋值
	}

	//使用同步HTTP请求,现主用在图片下载
	void SyncHTTPRequest(std::string host, std::string request, std::string save_path, std::string save_name) {
		//https://www.cnblogs.com/ribavnu/p/5084458.html
		//图片一般不会使用Chunk编码
		//一般动态网站会使用Chunk
		//首先创建一个DNS解析
		asio::ip::tcp::resolver res(m_ios);
		//待解析语句
		asio::ip::tcp::resolver::query resolve_query(host, "80", asio::ip::tcp::resolver::query::numeric_service);
		//同步DNS解析 
		//使用不抛出异常版本,当出现问题改抛出自己的异常
		boost::system::error_code e;
		asio::ip::tcp::resolver::iterator it = res.resolve(resolve_query, e);
		if (e.value() != 0) {
			throw boost::system::system_error(bangumi_bot_errors::sync_http_dns_resolve_error);
		}

		//创建一个socket,调用open打开,当出现问题改抛出自己的异常
		asio::ip::tcp::socket sock(m_ios);
		sock.open(asio::ip::tcp::v4(), e);
		if (e.value() != 0) {
			throw boost::system::system_error(bangumi_bot_errors::sync_http_sokect_open_filed);
		}
		//迭代的结尾
		asio::ip::tcp::resolver::iterator end;

		//连接可用的ip
		while (it != end) {
			//使用不抛出异常版本
			sock.connect(*it++, e);
			//如果有错误,改抛出自己的异常
			if (e.value() != 0) {
				if (it != end)
					continue;
				else {
					throw boost::system::system_error(bangumi_bot_errors::sync_http_dns_connect_error);
					//break;
				}

			}

			//此时已经正确连接到了一个IP
			asio::write(sock, asio::buffer(request));

			//接收buffer
			asio::streambuf buf;

			//首先只接收到头部结束
			//TODO:分开Content-Length和Chunk编码两种情况
			//[注意]虽然read_until可能超读,但是返回read_size是严格准确的<=所有读取的内容
			//当调用buf.consume()会从输入序列中移除
			size_t n = asio::read_until(sock, buf, "\r\n\r\n");
			//DEBUG
			//std::cout << "Received: " << n << std::endl;

			//接收的头部的字符串
			std::string ss = //boost::locale::conv::from_utf(
				std::string(asio::buffers_begin(buf.data()), asio::buffers_begin(buf.data()) + n);//, "GBK");
			//DEBUG
			//std::cout << "++++++\n"
			//	<< ss <<"\n++++++"
			//	<< std::endl;

			size_t dis = asio::buffers_end(buf.data()) - asio::buffers_begin(buf.data());
			size_t has_read = dis - n;
			//清除输入队列中的已读字节
			buf.consume(n);
			//DEBUG
			//std::cout << "--------\n"
			//	<< "dis = " << dis << std::endl
			//	<< "n = " << n << std::endl
			//	<< "has_read = " << has_read 
			//	<<"\n--------"<< std::endl;

			////解析文件后缀名
			//auto type_site = ss.find("Content-Type:");
			//if (type_site == std::string::npos)
			//{
			//	throw boost::system::system_error(bangumi_bot_errors::bad_response_header);
			//}
			//auto type_site_return = ss.find("\r\n", type_site + 13);
			//auto type_site_return_2 = ss.find(";", type_site + 13);
			////也存在一个用;分隔后面接上的charset的情况
			//if (type_site_return_2 < type_site_return)
			//	type_site_return = type_site_return_2;
			////文件后缀名
			//std::string file_postfix;
			//try {
			//	std::string without_black = ss.substr(type_site + 13, type_site_return - type_site - 13);
			//	//去除首空格
			//	without_black.erase(0, without_black.find_first_not_of(" "));
			//	//设置文件的后缀名,可能抛出异常
			//	file_postfix = Content_Type.at(without_black);

			//}
			//catch (std::exception &)
			//{
			//	throw boost::system::system_error(bangumi_bot_errors::bad_response_header);
			//}

			//assert(!file_postfix.empty());
			//文件名
			std::string &file_name = save_name;// +file_postfix;
			//文件
			std::string file;

			//解析编码
			auto length_site = ss.find("Content-Length:");
			auto chunk_site = ss.find("Transfer-Encoding:");
			if (chunk_site != std::string::npos) {
				//情况一:
				//Transfer-Encoding:
				//read_until最后一个chunk
				size_t nn = asio::read_until(sock, buf, "\r\n0");

				//DEBUG
				//std::cout << "Received: " << nn << std::endl;
				//DEBUG
				//std::string s(asio::buffers_begin(buf.data()), asio::buffers_end(buf.data()));
				//std::cout << "====" << std::endl;
				//std::cout << s << std::endl;
				//std::cout << "====" << std::endl;

				//首先清除头部的字节
				std::istream input(&buf);
				getline(input, std::string());

				//内容写入file中
				file = { boost::asio::buffers_begin(buf.data()),
					boost::asio::buffers_end(buf.data()) };
				//清除最后一个0字符(Transfer-Encoding：chunk模式的0结束标志)
				file[file.find_last_of('0')] = '\0';


			}
			else if (length_site != std::string::npos)
			{
				//情况二:
				//Content-Length:
				//此时不必专门去除Chunk特有的头部和尾部
				auto length_site_return = ss.find("\r\n", length_site + 15);
				try {
					//可能抛出非法参数异常
					int content_length = std::stoi(ss.substr(length_site + 15, length_site_return - length_site - 15));
					//读取剩余的内容(指定长度)
					size_t nn = asio::read(sock, buf, asio::transfer_exactly(content_length - has_read));
					//DEBUG
					//std::cout << "Received: " << nn << std::endl;
					//DEBUG
					//std::string s(asio::buffers_begin(buf.data()), asio::buffers_end(buf.data()));
					//std::cout << "====" << std::endl;
					//std::cout << s << std::endl;
					//std::cout << "====" << std::endl;


					//内容写入file中
					file = { asio::buffers_begin(buf.data()), asio::buffers_end(buf.data()) };
				}
				catch (std::invalid_argument &)
				{
					throw boost::system::system_error(bangumi_bot_errors::bad_response_header);
				}
			}
			else {
				throw boost::system::system_error(bangumi_bot_errors::bad_response_header);
			}



			//[注意]如果以“文本”方式打开文件，当读取文件的时候，系统会将所有的"/r/n"转换成"/n"
			//当写入文件的时候，系统会将"/n"转换成"/r/n"写入
			//因此此处使用二进制写入图片
			/*boost::filesystem*/std::ofstream fstream(save_path + file_name, std::ios::binary);
			fstream << file;
			fstream.close();

			break;

			//buf.consume(n);

			//最后接收内容
			//boost::this_thread::sleep(boost::posix_time::seconds(1));
			//try {
			//boost::system::error_code code;
			//size_t nn = asio::read_until(sock, buf ,"\r\n0");
			//Content-Length的长度包含头部
			//size_t nn = asio::read(sock, buf, asio::transfer_exactly(2647 - has_read));
			//std::cout << "Received: " << nn << std::endl;
			//if (code == asio::error::eof)
			//	std::cout << "Code ";
			//}
			//catch (boost::system::system_error &ec) {
			//	if(ec.code() == asio::error::eof)
			//		std::cout << "Code = " << ec.code().value() << " Message = " << ec.what();
			//}

			//std::cout << "====" << std::endl;
			//使用以下语句会有问题,无法正常输出
			//std::string s = //boost::locale::conv::from_utf(
				//boost::asio::buffer_cast<char>(buf.data());//, "GBK");
			//使用asio的函数读取
			//std::string s(asio::buffers_begin(buf.data()), asio::buffers_end(buf.data()));
			//std::cout << s << std::endl;
			//std::cout << "====" << std::endl;
			//break;
		}

		//最后关闭发送连接
		sock.shutdown(asio::ip::tcp::socket::shutdown_send);
	}
	//使用同步HTTP请求,主要用于BGMAuth方面
	std::string SyncBGMHTTPRequest(std::string request) {
		//https://www.cnblogs.com/ribavnu/p/5084458.html
		//图片一般不会使用Chunk编码
		//一般动态网站会使用Chunk
		//首先创建一个DNS解析
		//asio::ip::tcp::resolver res(m_ios);
		//待解析语句
		//asio::ip::tcp::resolver::query resolve_query(host, "80", asio::ip::tcp::resolver::query::numeric_service);
		//同步DNS解析 
		//使用不抛出异常版本,当出现问题改抛出自己的异常
		boost::system::error_code e;
		//asio::ip::tcp::resolver::iterator it = res.resolve(resolve_query, e);
		//if (e.value() != 0) {
		//	throw boost::system::system_error(bangumi_bot_errors::sync_http_dns_resolve_error);
		//}

		//创建一个socket,调用open打开,当出现问题改抛出自己的异常
		asio::ip::tcp::socket sock(m_ios);
		sock.open(asio::ip::tcp::v4(), e);
		if (e.value() != 0) {
			throw boost::system::system_error(bangumi_bot_errors::sync_http_sokect_open_filed);
		}
		//迭代的结尾


		//直接使用自身的iterator
		asio::ip::tcp::resolver::iterator end;
		//asio::ip::tcp::resolver::iterator fix_it;
		asio::ip::tcp::resolver::iterator m_it = fix_it;
		//连接可用的ip
		while (m_it != end) {
			//使用不抛出异常版本
			sock.connect(*m_it++, e);
			//如果有错误,改抛出自己的异常
			if (e.value() != 0) {
				if (m_it != end)
					continue;
				else {
					throw boost::system::system_error(bangumi_bot_errors::sync_http_dns_connect_error);
					//break;
				}

			}

			//此时已经正确连接到了一个IP
			asio::write(sock, asio::buffer(request));

			//接收buffer
			asio::streambuf buf;

			//首先只接收到头部结束
			//分开Content-Length和Chunk编码两种情况
			//[注意]虽然read_until可能超读,但是返回read_size是严格准确的<=所有读取的内容
			//当调用buf.consume()会从输入序列中移除
			size_t n = asio::read_until(sock, buf, "\r\n\r\n");
			//DEBUG
			//std::cout << "Received: " << n << std::endl;

			//接收的头部的字符串
			std::string ss = 
				std::string(asio::buffers_begin(buf.data()), asio::buffers_begin(buf.data()) + n);
			if (ss.find("1.1 301 ") != std::string::npos) {
				//301重定向问题
				return "";
			}
			size_t dis = asio::buffers_end(buf.data()) - asio::buffers_begin(buf.data());
			size_t has_read = dis - n;
			//清除输入队列中的已读字节
			buf.consume(n);
			//DEBUG
			//std::cout << "--------\n"
			//	<< "dis = " << dis << std::endl
			//	<< "n = " << n << std::endl
			//	<< "has_read = " << has_read 
			//	<<"\n--------"<< std::endl;


			//默认为Chunk
			

			size_t nn = asio::read_until(sock, buf, "\r\n0");
			std::string content;
			try {
				//首先输出一行
				
				std::istream input(&buf);
				getline(input, std::string());
				//s = boost::locale::conv::from_utf(
				//	std::string(std::istreambuf_iterator<char>((request->m_response).m_response_stream), {}), "GBK");
				content = 
					std::string(boost::asio::buffers_begin(buf.data()),
						boost::asio::buffers_end(buf.data()));

				
				content.erase(content.find_last_of('0'));

				//额外的Chunk合包
				//保持与Getcontent一致
				if (true) {
					size_t chunk_delim = 0;
					//
					chunk_delim = content.find("\r\n");
					//
					while (chunk_delim != std::string::npos) {
						//一定有下一个"\r\n"
						auto over = content.find("\r\n", chunk_delim);
						if (over != std::string::npos)
							content.erase(chunk_delim, over - chunk_delim + 2);
						else {
#ifndef NDEBUG
							{
								bangumi::string debug_msg;
								debug_msg << "Content中循环处理Content失败" << ": Find换行符出现问题";
								CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTP-GetResponseContent", debug_msg);
							}
#endif
						}
						//试图查找下一个
						chunk_delim = content.find("\r\n");
					}
				}
			}
			catch (std::exception) {
				throw boost::system::system_error(bangumi_bot_errors::http_get_contents_error);
			}
			//最后关闭发送连接
			sock.shutdown(asio::ip::tcp::socket::shutdown_send);

			return content;

		}

		//最后关闭发送连接
		//sock.shutdown(asio::ip::tcp::socket::shutdown_send);

		//
		return "";
	}
private:
	//设置为私有函数
	void resolve_url(std::string url, unsigned int port = 80) {
		m_url = url;
		m_port = port;

		//解析
		asio::ip::tcp::resolver resolver(m_ios);

		asio::ip::tcp::resolver::query resolve_query(m_url,
			std::to_string(m_port), asio::ip::tcp::resolver::numeric_service);

		//这里选择同步解析
		//防止异步顺序可能引起错误
		try {
			fix_it = resolver.resolve(resolve_query);
		}
		catch (boost::system::system_error&) {
			//抛出异常
			if (m_url.empty())
				throw boost::system::system_error(bangumi_bot_errors::without_url_in_client);
			else
				throw boost::system::system_error(bangumi_bot_errors::dns_resolve_error_in_client);
		}
#ifndef NDEBUG
		asio::ip::tcp::resolver::iterator end;
		std::string test = "解析的DNS:\n";
		for (auto it = fix_it; it != end; ++it) {
			test += it->endpoint().address().to_string();
			test += "\n";
		}
		CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTP-DNS-RESOLVER", test.c_str());

#endif


	}
	std::string Bangumi_App_Dir;
	asio::io_service m_ios;
	std::unique_ptr<boost::asio::io_service::work> m_work;
	std::unique_ptr<boost::thread> m_thread;
	//为了防止线程退出导致新创建的线程意外中止
	//因此将创建的线程存储
	std::map<int,std::shared_ptr<HTTPRequest>> m_requests_map;
	//为了支持清理,需要锁和ID
	unsigned int id = 0;
	std::mutex id_mutex;
	//因为当前应用大多数情况下只需要对一个url进行资源访问
	//因此记录asio::ip::tcp::resolver::iterator来减少重解析
	asio::ip::tcp::resolver::iterator fix_it;
	std::string m_url;
	unsigned int m_port;
public:
	//全局配置文件bgm
	BangumiBotVaribel* m_bgm;
};


//生成请求报文
//前提需要request信息足够(uri,host)
//注意header参数一定最后自带一个\r\n
inline std::string request_message(std::shared_ptr<HTTPRequest> request, HTTP_WAY way, std::string header = "", std::string content = "") {
	switch (way)
	{
		//因为是一个右值,使用std::move来赋值
	case HTTP_WAY::GET:
		return  std::move("GET " + request->m_uri + " HTTP/1.1\r\n"
			"Host: " + request->m_host + "\r\n" + header + "\r\n" + content);
		break;
	case HTTP_WAY::POST:
		return  std::move("POST " + request->m_uri + " HTTP/1.1\r\n"
			"Host: " + request->m_host + "\r\n" + header + "\r\n" + content);
		break;
	default:
		throw boost::system::system_error(bangumi_bot_errors::bad_request_message);
		break;
	}

}


//取得响应报文的内容
inline void GetResponseContent(std::shared_ptr<HTTPRequest> request, std::string &s, bool isChunk = true,
	int ec=0 )
{
	//https://stackoverflow.com/questions/3203452/how-to-read-entire-stream-into-a-stdstring
	//std::string s(std::istreambuf_iterator<char>((request->m_response).m_response_stream), {});
	//https://faithandbrave.hateblo.jp/entry/20110324/1300950590
	//该方法会读取所有内容包括之前的头部
	//std::string s = boost::asio::buffer_cast<const char* >((request->m_response).m_response_buf.data());
	//https://stackoverflow.com/questions/28929699/boostasio-read-n-bytes-from-socket-to-streambuf
	//std::string s(boost::asio::buffers_begin((request->m_response).m_response_buf.data()),
	//	boost::asio::buffers_end((request->m_response).m_response_buf.data()));
	//https://www.boost.org/doc/libs/1_67_0/libs/locale/doc/html/charset_handling.html
	//GBK编码中也支持日文
	//Transfer-Encoding：chunk模式来传输
	//http://jizhao.blog.chinaunix.net/uid-28458801-id-5064196.html	
	//chunk编码将数据分成一块一块的发生。Chunked编码将使用若干个Chunk串连而成，
	//由一个标明长度为0 的chunk标示结束。每个Chunk分为头部和正文两部分，头部内容指定正文的字符总数（十六进制的数字 ）
	//和数量单位（一般不写），正文部分就是指定长度的实际内容，两部分之间用回车换行(CRLF) 隔开
	if (ec!=0) {
		throw boost::system::system_error(static_cast<bangumi_bot_errors::bgm_error_codes>(ec));
	}
#ifndef NDEBUG
	{
		bangumi::string debug_msg;
		debug_msg << "GetResponseContent-buffer:"
			>> boost::asio::buffer_cast<const char*>((request->m_response).m_response_buf.data());
		CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTP-GetResponseContent", debug_msg);
	}
#endif
	try {
		//首先输出一行
		if (isChunk)
			getline((request->m_response).m_response_stream, std::string());
		//s = boost::locale::conv::from_utf(
		//	std::string(std::istreambuf_iterator<char>((request->m_response).m_response_stream), {}), "GBK");
		s = //boost::locale::conv::from_utf(
			std::string(boost::asio::buffers_begin((request->m_response).m_response_buf.data()),
				boost::asio::buffers_end((request->m_response).m_response_buf.data()));//, "GBK");

		//这里有一定机会出现301重定向问题
		//auto maybe_301 = s.find("Location: ");
		//if (maybe_301 != std::string::npos) {
		//	//的确发生的重定向
		//	//
		//	auto location_num = s.find("\r", maybe_301);
		//	std::string new_uri = s.substr(maybe_301 + 10, location_num - maybe_301 - 10);
		//	request->set_uri(new_uri);
		//	//清理m_response_buf
		//	(request->m_response).m_response_buf.consume((request->m_response).m_response_buf.size());
		//	//一般是Subject的重定向,直接使用GET
		//	request->set_request(request_message(request, HTTP_WAY::GET));
		//	//Callback已经设置完毕,因此直接exec即可
		//	request->execute();
		//}


		//清除最后一个0字符(Transfer-Encoding：chunk模式的0结束标志)
		//[注意] 不能使用'\0'空字符来代替0, property_tree会认为data后有垃圾不能read_json
		//s[s.find_last_of('0')] = '\0';
		//s[361] = *R"(\)";
		if (isChunk)
			s.erase(s.find_last_of('0'));

		//注意:像Search这样返回长的Json会多段,因此仍需要循环处理content
		
		if (isChunk) {
			size_t chunk_delim = 0;
			//
			chunk_delim = s.find("\r\n");
			//
			while (chunk_delim != std::string::npos) {
				//一定有下一个"\r\n"
				auto over = s.find("\r\n", chunk_delim);
				if(over!=std::string::npos)
					s.erase(chunk_delim, over - chunk_delim + 2);
				else {
#ifndef NDEBUG
					{
						bangumi::string debug_msg;
						debug_msg << "Content中循环处理Content失败" << ": Find换行符出现问题";
						CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTP-GetResponseContent", debug_msg);
					}
#endif
				}
				//试图查找下一个
				chunk_delim = s.find("\r\n");
			}
		}
	}
	catch (std::exception) {
		throw boost::system::system_error(bangumi_bot_errors::http_get_contents_error);
	}

	//std::string x("\"\\u9535\u9535!\"");
	//std::string ss;
	//escape(x);
	//std::cout <<"0000000000 ="<< x << std::endl;
	//s.erase(366,2);
	//std::cout  << std::endl;
	//std::cout << R"(\\u9535\u9535!)";


	
	//清除string内所有的转义符'\'(事实上收到的\u就只是\u而不是\\u)
	//s.erase(std::remove(s.begin(), s.end(), '\\'), s.end());
	//s = boost::locale::conv::from_utf(s, "Unicode");
	//boost::asio::basic_streambuf<std::allocator<wchar_t>>::const_buffers_type cbt = (request->m_response).m_response_buf.data();
	//std::string request_data(boost::asio::buffers_begin(cbt), boost::asio::buffers_end(cbt));
	//
	//std::cout << "xxxxx\n" << s1 << "xxxxxx\n" << std::endl;
}

//封装函数请求HTTP
//uri:已经包含了url,可以简写为/user/XXX等
//void HTTPSendRequestFixed(std::string &uri, std::string &request) {
//
//	std::shared_ptr<HTTPRequest> request_one =
//		http_client.create_request_fixed(1);
//	//create fixed情况下不需要set_host
//	//request_one->set_host("api.bgm.tv");
//	request_one->set_uri("/user/wz97315");
//	//设置头部
//	request_one->set_request(request_message(request_one, HTTP_WAY::GET));
//	//执行异步处理
//	request_one->execute();
//}
//void HTTPSendRequest(std::string &url, std::string &) {
//	
//	std::shared_ptr<HTTPRequest> request_one =
//		http_client.create_request(1);
//
//	request_one->set_host("api.bgm.tv");
//	request_one->set_uri("/user/wz97315");
//}

//路径定义
#define SUBJECT_PIC_PATH "Subject\\"
#define USER_PIC_PATH "User\\"
#define CHARACTER_PIC_PATH "Character\\"
#define TAG_PIC_PATH "Tag\\"
enum class DownloadStatus
{
	SingleThread,
	MultiThread,
	HasFinished
};
//save_path已经包含了酷Q中的提供的cache_path目录为基目录,并且已经有最后一个/
//[注意]请使用SUBJECT_PIC_PATH等宏来作为save_path
//save_path建议使用宏定义
//save_name是不带后缀名的
//注意不会检查文件路径,也不会自动创建,因此请手动确保文件夹存在
std::pair<DownloadStatus, std::shared_ptr<boost::thread>> HTTPDownload(HTTPClient& http_client, std::string host, std::string uri, std::string save_path, std::string save_name) {

	//TODO暂时没有测试函数功能,同时也考虑到暂时用不到
	//
	//dir_path == Cache/User/
	std::string dir_path = (http_client.m_bgm->cache_path) + save_path;
	boost::filesystem::path pic_path;
	std::string absolute_dir_path;
	if (uri.find_last_of(".jpg") != std::string::npos) {
		save_name += ".jpg";
		//这里应该加上绝对路径检查文件的存在性 D:\Program\酷Q Pro\data\image\Cache\User\???.???
		pic_path = http_client.m_bgm->Bangumi_Img_Dir + (dir_path + save_name);
	}
	else if (uri.find_last_of(".png") != std::string::npos) {
		save_name += ".png";
		pic_path = http_client.m_bgm->Bangumi_Img_Dir + (dir_path + save_name);
	}
	else {
		//否则直接抛出错误
		throw boost::system::system_error(bangumi_bot_errors::pic_download_with_unknown_url);
	}
	//返回下载路径(相对的) Cache/User/???.???
	//file_path = dir_path + save_name;
	//绝对路径 D:/Program/酷Q Pro/data/image/Cache/User/
	absolute_dir_path = http_client.m_bgm->Bangumi_Img_Dir + dir_path;
#ifndef NDEBUG
	{
		bangumi::string debug_msg;
		debug_msg //<< "file_path = " << file_path
			>> "pic_path = " << pic_path.string()
			>> "absolute_dir_path = " << absolute_dir_path;
		CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-PIC-DOWNLOAD", debug_msg);
	}
#endif
	//检查文件是否已经存在
	//这里为了方便只针对可能出现的文件后缀
	//TODO:根据文件时间来决定是否更新
	if (boost::filesystem::exists(pic_path)) {
		//直接结束
		//这里的意图是在FUNC中调用的API函数中调用join()等侍图片下载完毕
		return{ DownloadStatus::HasFinished, nullptr };
	}
	//request 
	std::string request =
		"GET " + uri + " HTTP/1.1\r\n"
		"Host: " + host + "\r\n" "\r\n";
	//创建一个新的进程
	if (bgm.CheckThreadSize()) {
		//有空闲可用的进程
		std::shared_ptr<boost::thread> download_thread
		(new boost::thread([&http_client, host, uri, request, absolute_dir_path, save_name]() {
#ifndef NDEBUG
			std::ostringstream oss;
			oss << boost::this_thread::get_id();
			std::string idAsString = oss.str();
			std::string test = "开启新的线程 ID: " + idAsString;
			CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTP-DOWNLOAD", test.c_str());

#endif
			try {
				//此处使用同步HTTP请求来处理
				http_client.SyncHTTPRequest(host, request, absolute_dir_path, save_name);
			}
			catch (boost::system::system_error& e) {
				std::string test = "多线程图片下载失败,错误ID:" + std::to_string(e.code().value()) + " 错误信息:" + e.what();
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTP-DOWNLOAD", test.c_str());
			}
			bgm.AddAVAThreadSize();
	}));
		//返回线程的索引,令主线程调用join()
		return{ DownloadStatus::MultiThread, download_thread };
}
	else {
		//使用单线程

		////使用HTTP下载
		//std::shared_ptr<HTTPRequest> request_one =
		//	http_client.create_request(1);
		////boost::thread use([request_one]() {
		//request_one->set_host(host);
		//request_one->set_uri(uri);
		//request_one->set_request(request_message(request_one, HTTP_WAY::GET));
		//request_one->execute();

		try {
			//此处使用同步HTTP请求来处理
			http_client.SyncHTTPRequest(host, request, absolute_dir_path, save_name);
		}
		catch (boost::system::system_error& e) {
			std::string test = "多线程图片下载失败,错误ID:" + std::to_string(e.code().value()) + " 错误信息:" + e.what();
			CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTP-DOWNLOAD", test.c_str());
		};

		return{ DownloadStatus::SingleThread, nullptr };
	}

}

//仅仅支持http
//此函数会抛出异常
//save_path请使用宏定义
std::pair<DownloadStatus, std::shared_ptr<boost::thread>> PicDownload
(HTTPClient& http_client, std::string http_url, std::string save_path, std::string save_name, std::string &file_path, bool refresh = false) {
	if (http_url.empty()) {
		//说明此图片不存在，null
		//使用默认的404图片
		file_path = bgm.not_found_pic_path;
		//直接返回
		return{ DownloadStatus::SingleThread, nullptr };
	}
	//dir_path == Cache/User/
	std::string dir_path = (http_client.m_bgm->cache_path) + save_path;
	boost::filesystem::path pic_path; 
	std::string absolute_dir_path;
	if (http_url.find_last_of(".jpg") != std::string::npos) {
		save_name += ".jpg";
		//这里应该加上绝对路径检查文件的存在性 D:\Program\酷Q Pro\data\image\Cache\User\???.???
		pic_path = http_client.m_bgm->Bangumi_Img_Dir + (dir_path + save_name );
	}
	else if (http_url.find_last_of(".png") != std::string::npos) {
		save_name += ".png";
		pic_path = http_client.m_bgm->Bangumi_Img_Dir + (dir_path + save_name );
	}
	else {
		//否则直接抛出错误
		throw boost::system::system_error(bangumi_bot_errors::pic_download_with_unknown_url);
	}
	//返回下载路径(相对的) Cache/User/???.???
	file_path = dir_path + save_name;
	//绝对路径 D:/Program/酷Q Pro/data/image/Cache/User/
	absolute_dir_path = http_client.m_bgm->Bangumi_Img_Dir + dir_path;
#ifndef NDEBUG
	{
		bangumi::string debug_msg;
		debug_msg << "file_path = " << file_path
			>> "pic_path = " << pic_path.string()
			>> "absolute_dir_path = " << absolute_dir_path;
		CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-PIC-DOWNLOAD", debug_msg);
	}
#endif
	//检查文件是否已经存在
	//这里为了方便只针对可能出现的文件后缀
	//TODO:根据文件时间来决定是否更新
	if (boost::filesystem::exists(pic_path)&&!refresh) {
		//直接结束
		//这里的意图是在FUNC中调用的API函数中调用join()等侍图片下载完毕
#ifndef NDEBUG
		{
			bangumi::string debug_msg;
			debug_msg << "图片命中: " << pic_path.string();
			CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-PIC-DOWNLOAD", debug_msg);
		}
#endif
		return{ DownloadStatus::HasFinished, nullptr };
	}
	std::string uri;
	std::string host;
	//解析一下http_url
	//先清除所有空格
	boost::erase_all(http_url," ");
	//例子
	//http://lain.bgm.tv/pic/user/l/000/09/29/92981.jpg?r=1553752474
	auto site = http_url.find("http:");
	if (site != std::string::npos) {
		//首先删除http://
		http_url.erase(site, 7);
		
		//移除后面的参数
		//http_url.erase(http_url.find_first_of('?'));
		//
		auto delim = http_url.find_first_of('/');
		host = http_url.substr(0, delim);
		//要加上/
		uri = http_url.substr(delim);
		
		
		//Debug
		std::cout << "host = " << host
			<< "\nuri = " << uri << std::endl;
	}
	else if (http_url.find("https:") != std::string::npos) {
		throw boost::system::system_error(bangumi_bot_errors::pic_download_with_https);
	}
	else {
		throw boost::system::system_error(bangumi_bot_errors::pic_download_with_unknown_url);
	}
	//request 
	std::string request =
		"GET " + uri + " HTTP/1.1\r\n"
		"Host: " + host + "\r\n" "\r\n";
	//创建一个新的进程
	if (bgm.CheckThreadSize()) {
		//有空闲可用的进程
		std::shared_ptr<boost::thread> download_thread
		(new boost::thread([&http_client, host, uri, request, absolute_dir_path, save_name]() {
#ifndef NDEBUG
			{
				std::ostringstream oss;
				oss << boost::this_thread::get_id();
				std::string idAsString = oss.str();
				std::string test = "开启新的线程 ID: " + idAsString +
					"\n总池大小: " + std::to_string(bgm.threadpool_size) + "\n"
					"可用大小: " + std::to_string(bgm.curr_thread_size);
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTP-DOWNLOAD", test.c_str());
			}
#endif
			try {
				//此处使用同步HTTP请求来处理
				http_client.SyncHTTPRequest(host, request, absolute_dir_path, save_name);
			}
			catch (boost::system::system_error& e) {
				std::string test = "多线程图片下载失败,错误ID:" + std::to_string(e.code().value()) + " 错误信息:" + e.what();
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTP-DOWNLOAD", test.c_str());
			}
			bgm.AddAVAThreadSize();
		}));
		//返回线程的索引,令主线程调用join()
		return{ DownloadStatus::MultiThread, download_thread };
	}
	else {
		//使用单线程

		////使用HTTP下载
		//std::shared_ptr<HTTPRequest> request_one =
		//	http_client.create_request(1);
		////boost::thread use([request_one]() {
		//request_one->set_host(host);
		//request_one->set_uri(uri);
		//request_one->set_request(request_message(request_one, HTTP_WAY::GET));
		//request_one->execute();
#ifndef NDEBUG
		{
			bangumi::string debug_msg;
			debug_msg << "使用单线程下载";
			CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-PIC-DOWNLOAD", debug_msg);
		}
#endif

		try {
			//此处使用同步HTTP请求来处理
			http_client.SyncHTTPRequest(host, request, absolute_dir_path, save_name);
		}
		catch (boost::system::system_error& e) {
			std::string test = "多线程图片下载失败,错误ID:" + std::to_string(e.code().value()) + " 错误信息:" + e.what();
			CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTP-DOWNLOAD", test.c_str());
		};

		return{ DownloadStatus::SingleThread, nullptr };
	}

}


#endif


