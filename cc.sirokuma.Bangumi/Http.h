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

//ǰ������
class HTTPClient;
class HTTPRequest;
class HTTPResponse;
//ö��
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

//�ص���������
//TODO:��ȷ�ĺ���ǩ��
typedef void(*BGMCallback)(std::shared_ptr<HTTPRequest>, bangumi::BGMRetParam, int ec);
//��Request֮��洢��Ӧ��Ϣ��Ҫ
class HTTPResponse {
	//����HTTPRequestΪ��Ԫ
	//HttpRequest���б����ʵ������
	friend class HTTPRequest;
	//���ڶ�ȡResponse�еı�����Ϣ
	friend void GetResponseContent(std::shared_ptr<HTTPRequest> request, std::string &s, bool isChunk, int ec);

	HTTPResponse()
		//����streambuf����׼��������
		:m_response_stream(&m_response_buf)
	{}
public:
	//ȡ����Ӧ������ͷ����״̬��
	unsigned int get_status_code()const {
		return m_status_code;
	}
	//ȡ����Ӧ������ͷ����״̬��Ϣ
	const std::string& get_status_message()const {
		return m_status_message;
	}
	//������Ӧ���ĵ�ͷ���и��������Լ���Ӧ��ֵ
	const std::map<std::string, std::string>& get_headers() {
		return m_headers;
	}
	//����һ��std���еı�׼�����������asio::streambuf�е���Ϣ
	const std::istream&get_response()const {
		return m_response_stream;
	}
	////�������
	//void Clear() {
	//	m_status_code = 0;
	//	m_status_message = "";
	//	m_headers.clear();
	//	std::string t;
	//	while (std::getline(m_response_stream,t)) {
	//		//���buffer
	//	}
	//	m_response_stream.clear();
	//	//m_response_buf.consume(m_response_buf.size());
	//	
	//}
private:
	//��ȡasio::streambuf����Ϊ������ɴ�socket�ж�ȡ���ݵ�buffer��
	//���մ�ų�ͷ��������(ͷ������Ϣ����istream pop��)
	boost::asio::basic_streambuf<std::allocator<char>>&get_response_buf() {
		return m_response_buf;
	}
	//������Ӧ���ĵ�״̬��
	void set_status_code(unsigned int status_code) {
		m_status_code = status_code;
	}
	//������Ӧ���ĵ�״̬��Ϣ
	void set_status_message(const std::string& status_message) {
		m_status_message = status_message;
	}
	//�����Ӧ���ĵ�ͷ����Ϣ
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
//������
//�̳�enable_shared_from_this,��Ҫ������on_finish�е��ú���ʱ�������������ָ��,����һЩ���ܵĴ���
class HTTPRequest : public std::enable_shared_from_this<HTTPRequest> {
	//��Ԫ����HTTPClient
	friend class HTTPClient;
	//���ڸ���ʵ����Ϣ����request����
	friend std::string request_message(std::shared_ptr<HTTPRequest> request, HTTP_WAY way, std::string header, std::string content);
	//���ڶ�ȡResponse�еı�����Ϣ
	friend void GetResponseContent(std::shared_ptr<HTTPRequest> request, std::string &s, bool isChunk, int ec);
	//����Ĭ�϶˿�Ϊ80
	static const unsigned int DEFAULT_PORT = 80;
	//���캯��
	//@para1: io_serivce
	//@para2: id�����̵߳ı�ʶ
	//���캯���л��ʼ��Socket,DNS������(Resolver)

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

	//���ع��캯���Ѵ���һ���̶�URL����
	//ͬʱҲ�����Host��ֵ
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
	//�����������е�Host
	void set_host(const std::string& host) {
		m_host = host;
	}
	//���ö˿ں�
	void set_port(unsigned int port) {
		m_port = port;
	}
	//����URIͳһ��Դ��ʶ��
	void set_uri(const std::string& uri) {
		m_uri = uri;
	}
	//����������
	void set_request(std::string request) {
		m_request_buf = request;
	}
	//����Response
	const HTTPResponse& get_response_class() {
		return m_response;
	}
	//����Host
	std::string get_host() const {
		return m_host;
	}
	//����Request
	std::string get_request() const{
		return m_request_buf;
	}
	//����Port
	unsigned int get_port() const {
		return m_port;
	}
	//����URI
	const std::string& get_uri() const {
		return m_uri;
	}
	//�����߳�ID
	unsigned int get_id() const {
		return m_id;
	}
	//���ûص�����
	void set_callback(BGMCallback callback) {
		m_callback = callback;
	}
	//���ûظ��ṹ��
	void set_ret_param(bangumi::BGMRetParam param) {
		m_bgm_ret_param = param;
	}
	//ִ�д˴�����
	void execute() {
		//��֤��ȷ������
		assert(m_port > 0);
		assert(m_host.length() > 0);
		assert(m_uri.length() > 0);

		//�����һ���־�����
		if (m_isPersistent) {
			//ֱ�ӵ������ӽ����Ļص�����
			on_connection_established(boost::system::error_code(), m_it);
			return;
		}

		//��������½���
		if (m_withoutResolver) {
			//ֱ�ӵ��ý�����Ļص�����
			//ʹ��Ĭ�ϳ�ʼ��error_code
			//��ʱm_it�Ѿ���Client�����˴˴�������
			on_host_name_resolved(boost::system::error_code(), m_it);
			return;
		}

		//����DNS����,����������ѯ
		//asio::ip::tcp::resolver::query resolver_query(m_host,
		//	std::to_string(m_port),
		//	//��flag����˵���ڶ���������һ�����ֶ˿�
		//	asio::ip::tcp::resolver::query::numeric_service);


		asio::ip::tcp::resolver::query resolver_query(m_host, std::to_string(m_port));
		asio::ip::tcp::resolver::iterator start = m_resolver.resolve(resolver_query);
		asio::ip::tcp::resolver::iterator end;
		//test
		for (auto it = start; it != end; ++it) {
			std::cout << it->endpoint().address() << std::endl;
		}

		//����������,ͬʱ��ɺ����on_host_name_resolved
		//�ᴫ�ݽ������IP������
		m_resolver.async_resolve(resolver_query,
			[this](const boost::system::error_code& ec,
				asio::ip::tcp::resolver::iterator iterator)
		{
#ifndef NDEBUG
			std::string str2 = "async_resolve -> Callback  : ��������";
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

		//���ӵ���һ����Ч��IP
		asio::async_connect(m_sock,
			iterator,
			[this](const boost::system::error_code& ec,
				asio::ip::tcp::resolver::iterator iterator)
		{
#ifndef NDEBUG
			std::string str2 = "async_connect -> Callback  : ������ЧIP";
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
		//[]��ת����public��set_request������
		//����������
		//m_request_buf += "GET " + m_uri + " HTTP/1.1\r\n";
		//m_request_buf += "Host: " + m_host + "\r\n";
		//m_request_buf += "\r\n";
		//���������Ϊ��,�׳��쳣
		if (m_request_buf.empty()) {
			throw boost::system::system_error(bangumi_bot_errors::empty_request);
		}


		//�첽����
		asio::async_write(m_sock,
			asio::buffer(m_request_buf),
			[this](const boost::system::error_code& ec,
				std::size_t bytes_transferred)
		{
#ifndef NDEBUG
			std::string str2 = "async_write -> Callback  : �첽��������";
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
		//���ﲻ��ֱ��shutdown(send)
		//���ܻ�����ղ���Response
		//ԭ���������Ϊ���첽���������shutdown����ִ�����async_write()����
		//m_sock.shutdown(asio::ip::tcp::socket::shutdown_send);

		//
		//ע�������Ѿ���ȡ�����е�ͷ��
		//�йر��ĵĽṹ https://www.cnblogs.com/rainydayfmb/p/5319318.html
		asio::async_read_until(m_sock,
			m_response.get_response_buf(),
			"\r\n",
			[this](const boost::system::error_code& ec,
				std::size_t bytes_transferred)
		{
			//[ע��]
			//���Ϊ�˱�֤���ͷ��׽�����Դ֮ǰ��ȷ�رշ���
			//�ڻص�������shutdown()
			//������ǳ־����������shutdown
			//[ע��]�������shutdown,����ᵼ�¿��ڶ�ȡͷ����...
			if (!m_isPersistent)
				m_sock.shutdown(asio::ip::tcp::socket::shutdown_send);

			//�ƻ���debug
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


			//ʵ���������ֱ�Ӷ��������е�ͷ��
			//������Ϊread_until������(���ܻᳬ��)
			//istream��rdbuf()����Ҳ���ƶ�����ǰ��

#ifndef NDEBUG
			std::string str2 = "async_read_until -> Callback : ����״̬�ж�ȡ���";
			CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTP", str2.c_str());
#endif
			on_status_line_received(ec, bytes_transferred);
			//ֱ�ӵ��ö�ȡ��ͷ���Ļص�Ҳûʲô����(��Ϊread_until������),����ֻ�ǲ���,������ȷ��˼��
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
		//����״̬��
		std::string http_version;
		std::string str_status_code;
		std::string status_message;
		std::istream response_stream(
			&m_response.get_response_buf());
		response_stream >> http_version;

		if (http_version != "HTTP/1.1") {
			//�����
			on_finish(bangumi_bot_errors::http_error);
			return;
		}
		response_stream >> str_status_code;
		//״̬��
		unsigned int status_code = 200;
		try {
			//��ת��Ϊֵ����
			status_code = std::stoul(str_status_code);
		}
		catch (std::logic_error&) {
			//ת��ʧ��
			on_finish(bangumi_bot_errors::http_error);
			return;
		}
		//getline(buf, 1024)��ȱʡ�Ƕ���\n�������ͻ�� \r �������ε�buf���棻
		//getline(buf, 1024, '\r')������ \n ������һ�ε�buf���棻
		//getline(buf, 1024, "\r\n")������ʱ�򱨴�ע��\r\n��˫���ţ�
		//������\r\n��β�����ȶ�ȡ\r֮ǰ�ٺ���\n
		std::getline(response_stream, status_message, '\r');
		//������һ���ַ�(\n)
		response_stream.get();

		//std::istream xxx(&m_response.get_response_buf());

		m_response.set_status_code(status_code);
		m_response.set_status_message(status_message);

		//���������,��ʱ�Ѿ����������״̬��
		//����ĵ�����Ϊ�˶�ȡ��Ӧͷ(ʵ���ϴ�ʱ�Ѿ���ȡ�����,����Ϊ��(streambuf)��û����ǰ��,�������ڻ�����)
		asio::async_read_until(m_sock,
			m_response.get_response_buf(),
			"\r\n\r\n",
			[this](
				const boost::system::error_code& ec,
				std::size_t bytes_transferred)
		{
#ifndef NDEBUG
			std::string str2 = "async_read_until-2 -> Callback : ����ͷ����ȡ���";
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
		//������Ӧͷ
		std::string header, header_name, header_value;
		std::istream response_stream(
			&m_response.get_response_buf());
		while (true) {
			std::getline(response_stream, header, '\r');
			//������һ���ַ�(\n)
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
				//�����Ӧͷ�洢����Ӧ��HTTPResponse������
				m_response.add_header(header_name,
					header_value);
			}
		}
		//�ض���Location 301 ���
		//When301();
		try {
			m_response.get_headers().at("Location");
			//������,301����
			//����ص���������Ҳ�ɴ���һЩ�쳣����,����Ҫ���ڻظ�
			//�˴������ظ�
			bgm_error = bangumi_bot_errors::maybe_301_maybe_limit;
			m_callback(shared_from_this(), m_bgm_ret_param, bgm_error);
			//
			on_finish(boost::system::system_error(bangumi_bot_errors::api_301_error).code());
			//
		}
		catch (std::out_of_range) {
			//û������,����
		}

		//��ʱ�Ƕ�ȡ��Ӧ����
		asio::async_read(m_sock,
			m_response.get_response_buf(),
			[this](
				const boost::system::error_code& ec,
				std::size_t bytes_transferred)
		{
#ifndef NDEBUG
			std::string str2 = "async_read_until-3 -> Callback : �������Ķ�ȡ���";
			CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTP", str2.c_str());
#endif
			//��󣬵���on_response_body_received����������֪ͨ���յ�������Ӧ��Ϣ��
			//��ΪHTTP��������������������Ӧ��Ϣ����󲿷�֮��
			//�ر����׽��ֵķ��Ͳ���,�����ڿͻ���,���Ķ�ȡ�����������
			//����������asio::error::eof
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
			std::string x = "����on_finish����";
			CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTP-ON-FINISH", x.c_str());
		}
#endif
		if (ec.value() != 0) {
			//TODO:�쳣��������
			//ת���ϲ㴦��

#ifndef NDEBUG
			std::string x = "Error occured! Error code = "
				+ std::to_string(ec.value()) + ". Message: " + ec.message();
			CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTP-ON-FINISH", x.c_str());
#endif
			//���ﲻ���㽻��Func���쳣����
			if (bgm_error == 0)
				RemoveSelf();
			throw boost::system::system_error(ec);
		}

		//û��������ִ���Զ���ص��������ڴ�����Ϣ�ȵ�
		if (m_callback != nullptr) {

#ifndef NDEBUG
			{
				std::string x = "ִ�лص�������m_callback";
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTP-ON-FINISH", x.c_str());
			}
#endif
			if(!m_bgm_ret_param.empty())
				//����ص���������Ҳ�ɴ���һЩ�쳣����,����Ҫ���ڻظ�
				m_callback(shared_from_this(), m_bgm_ret_param, bgm_error);
			else {
				if (bgm_error == 0)
					RemoveSelf();
				throw boost::system::system_error(bangumi_bot_errors::empty_bgm_ret_param);
			}
		}
			
		else {
			//�쳣����ʹ��new
			//TODO,ֻ�ǲ���
			if(bgm_error==0)
				RemoveSelf();
			throw boost::system::system_error(bangumi_bot_errors::empty_callback);
		}

	}
	
	//301������
	//void When301();

	//�쳣�������������
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

	//���ڴ���HTTP֮���Bot����
	BGMCallback m_callback;
	//���ڽ��ջظ���Ϣ�Ľṹ��
	bangumi::BGMRetParam m_bgm_ret_param;

	//ʵ�ʴ���iosʵ��λ��HTTPClient��
	asio::io_service &m_ios;
	//Ϊ�˼����ظ���DNS����,ֱ����ÿ���״�DNS����ʱ�洢������
	//��ΪĬ�Ϲ��캯����ʾһ���������Ľ�β,�����ж��Ƿ�Ϊ��
	//asio::ip::tcp::resolver::iterator iterator;
	//asio::ip::tcp::resolver::iterator iterator_end;
	//�Ƿ����½���DNS
	bool m_withoutResolver;
	//�Ƿ��ǳ־�����
	bool m_isPersistent;
	//������IP��ַ
	asio::ip::tcp::resolver::iterator m_it;
	//������Ϣ�洢
	int bgm_error = 0;
};
//HTTP��װ��
class HTTPClient {
public:
	HTTPClient() {
		//���ȶ���һ��work����ȷ����û�й�����첽����ʱ
		//�¼�ѭ�����̲߳����˳���ѭ��
		//һ��ֻҪ�Ƿ�װ��asio���첽���ܵĶ���Ҫ
		//һ��ʼ��ʼ��ʱ��û�й�����첽����
		m_work.reset(new boost::asio::io_service::work(m_ios));

		//m_thread.reset(new boost::thread([this]() {
		//	//�µ��߳������Լ����쳣����
		//	//��ص�����������ios.run()���õ�
		//	try {
		//		m_ios.run();
		//	}
		//	catch (boost::system::system_error& ec)
		//	{
		//		std::string test = "HTTPClient���첽�в�׽������,����ID:" + std::to_string(ec.code().value()) + " ������Ϣ:" + ec.what();
		//		CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-IOS", test.c_str());
		//	}
		//}));
		m_thread.reset(new boost::thread([this]() {
			RunIos();
		}));


		//TODO����һ���־�������
		//HTTPRequest xx
		//xx ->m_isPreXXX = true;

	}
private:
	//����m_ios.run()
	void RunIos() {
		try {
			m_ios.run();
		}
		catch (boost::system::system_error& ec)
		{
			std::string test = "HTTPClient���첽�в�׽������,����ID:" + std::to_string(ec.code().value()) + " ������Ϣ:" + ec.what();
			CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-IOS", test.c_str());
			RunIos();
		}
	}
public:
	//��������HttpServer�ĳ�ʼ��
	asio::io_service& GetIOS() {
		return m_ios;
	}
	//[ע��]һ������������,���캯���л���һ��ûȷ��״̬��thread������
	~HTTPClient() {
		this->close();
	}

	//�������ڽ����Զ���ĵ�request
	std::shared_ptr<HTTPRequest>
		create_request(unsigned int id)
	{
		auto x = std::shared_ptr<HTTPRequest>(
			new HTTPRequest(m_ios, id));
		//���û�л��Ӧ�������Զ�����,at�򲻻Ტ���׳��쳣
		m_requests_map[id] = x;
		return x;
	}
	//�������ڽ���ָ����request(ָ����������������)
	std::shared_ptr<HTTPRequest>
		create_request_fixed(unsigned int id)
	{
		//Ϊ�˷�ֹm_it�ݽ�,���ÿ��ʹ�ø���һ��it
		asio::ip::tcp::resolver::iterator m_it = fix_it;
		if (m_it != asio::ip::tcp::resolver::iterator()) {
			auto x = std::shared_ptr<HTTPRequest>(
				new HTTPRequest(m_ios, id, m_url, m_it));
			m_requests_map[id] = x;
			return x;
		}
		else {
			//�׳��쳣
			if (m_url.empty())
				throw boost::system::system_error(bangumi_bot_errors::without_url_in_client);
			else
				throw boost::system::system_error(bangumi_bot_errors::dns_resolve_error_in_client);
		}

	}
	//ȡ���߳�ID��
	int GetID() {
		//������,�˳�����������Զ����н���
		std::unique_lock<std::mutex>
			thread_lock(id_mutex);
		//������map.size()>=this->id,����һ�����е�(nullptr)��id����
		//��������һ��ID��
		++this->id;
		//���ҿ��е�IDλ��
		//��map���������������ĵ�ǰID����,��ǰ��ID��Ϊnullptr�Ŀ����Ժܴ�,ֱ�Ӵ�0�ſ�ʼ����
		if (2 * id < m_requests_map.size()) {
			unsigned int try_id = -1;
			while (m_requests_map[++try_id] != nullptr)
			{
			}
			return try_id;
		}
		//���������ʱֱ�ӷ���m_requests_map.size()
		else {

			return m_requests_map.size();
		}

		//return id;
	}
	//ɾ��ID��Ӧ���߳�
	void RemoveID(unsigned int this_id) {
		std::unique_lock<std::mutex>
			thread_lock(id_mutex);
		m_requests_map[this_id] = nullptr;
		--this->id;
#ifndef NDEBUG
		{
			bangumi::string debug_msg;
			debug_msg << "RemoveID: " << this->id
				>> "��ǰ�صĴ�С(Remove��): " << m_requests_map.size();
			for (auto &x : m_requests_map) {
				debug_msg >> std::to_string(x.first) << ": " << (x.second == nullptr ? "Null" : "*");
			}
			debug_msg >> "=====";
			CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTP", debug_msg);
		}
#endif
	}
	void close() {
		//����m_work����
		m_work.reset(NULL);
		//�ȴ��߳��˳�
		//�߳������й��������ɺ������˳��¼�ѭ��
		m_thread->join();
	}
	//��bgm��ʼ��,ͬʱInit BGM�еĹ̶�URL�Ľ���
	void Init(BangumiBotVaribel* bgm) {
		m_bgm = bgm;
		//����·��
		Bangumi_App_Dir = m_bgm->Bangumi_App_Dir;
		//��������url
		resolve_url(m_bgm->bangumi_api_url);
		//�����и���ĸ�ֵ
	}

	//ʹ��ͬ��HTTP����,��������ͼƬ����
	void SyncHTTPRequest(std::string host, std::string request, std::string save_path, std::string save_name) {
		//https://www.cnblogs.com/ribavnu/p/5084458.html
		//ͼƬһ�㲻��ʹ��Chunk����
		//һ�㶯̬��վ��ʹ��Chunk
		//���ȴ���һ��DNS����
		asio::ip::tcp::resolver res(m_ios);
		//���������
		asio::ip::tcp::resolver::query resolve_query(host, "80", asio::ip::tcp::resolver::query::numeric_service);
		//ͬ��DNS���� 
		//ʹ�ò��׳��쳣�汾,������������׳��Լ����쳣
		boost::system::error_code e;
		asio::ip::tcp::resolver::iterator it = res.resolve(resolve_query, e);
		if (e.value() != 0) {
			throw boost::system::system_error(bangumi_bot_errors::sync_http_dns_resolve_error);
		}

		//����һ��socket,����open��,������������׳��Լ����쳣
		asio::ip::tcp::socket sock(m_ios);
		sock.open(asio::ip::tcp::v4(), e);
		if (e.value() != 0) {
			throw boost::system::system_error(bangumi_bot_errors::sync_http_sokect_open_filed);
		}
		//�����Ľ�β
		asio::ip::tcp::resolver::iterator end;

		//���ӿ��õ�ip
		while (it != end) {
			//ʹ�ò��׳��쳣�汾
			sock.connect(*it++, e);
			//����д���,���׳��Լ����쳣
			if (e.value() != 0) {
				if (it != end)
					continue;
				else {
					throw boost::system::system_error(bangumi_bot_errors::sync_http_dns_connect_error);
					//break;
				}

			}

			//��ʱ�Ѿ���ȷ���ӵ���һ��IP
			asio::write(sock, asio::buffer(request));

			//����buffer
			asio::streambuf buf;

			//����ֻ���յ�ͷ������
			//TODO:�ֿ�Content-Length��Chunk�����������
			//[ע��]��Ȼread_until���ܳ���,���Ƿ���read_size���ϸ�׼ȷ��<=���ж�ȡ������
			//������buf.consume()��������������Ƴ�
			size_t n = asio::read_until(sock, buf, "\r\n\r\n");
			//DEBUG
			//std::cout << "Received: " << n << std::endl;

			//���յ�ͷ�����ַ���
			std::string ss = //boost::locale::conv::from_utf(
				std::string(asio::buffers_begin(buf.data()), asio::buffers_begin(buf.data()) + n);//, "GBK");
			//DEBUG
			//std::cout << "++++++\n"
			//	<< ss <<"\n++++++"
			//	<< std::endl;

			size_t dis = asio::buffers_end(buf.data()) - asio::buffers_begin(buf.data());
			size_t has_read = dis - n;
			//�����������е��Ѷ��ֽ�
			buf.consume(n);
			//DEBUG
			//std::cout << "--------\n"
			//	<< "dis = " << dis << std::endl
			//	<< "n = " << n << std::endl
			//	<< "has_read = " << has_read 
			//	<<"\n--------"<< std::endl;

			////�����ļ���׺��
			//auto type_site = ss.find("Content-Type:");
			//if (type_site == std::string::npos)
			//{
			//	throw boost::system::system_error(bangumi_bot_errors::bad_response_header);
			//}
			//auto type_site_return = ss.find("\r\n", type_site + 13);
			//auto type_site_return_2 = ss.find(";", type_site + 13);
			////Ҳ����һ����;�ָ�������ϵ�charset�����
			//if (type_site_return_2 < type_site_return)
			//	type_site_return = type_site_return_2;
			////�ļ���׺��
			//std::string file_postfix;
			//try {
			//	std::string without_black = ss.substr(type_site + 13, type_site_return - type_site - 13);
			//	//ȥ���׿ո�
			//	without_black.erase(0, without_black.find_first_not_of(" "));
			//	//�����ļ��ĺ�׺��,�����׳��쳣
			//	file_postfix = Content_Type.at(without_black);

			//}
			//catch (std::exception &)
			//{
			//	throw boost::system::system_error(bangumi_bot_errors::bad_response_header);
			//}

			//assert(!file_postfix.empty());
			//�ļ���
			std::string &file_name = save_name;// +file_postfix;
			//�ļ�
			std::string file;

			//��������
			auto length_site = ss.find("Content-Length:");
			auto chunk_site = ss.find("Transfer-Encoding:");
			if (chunk_site != std::string::npos) {
				//���һ:
				//Transfer-Encoding:
				//read_until���һ��chunk
				size_t nn = asio::read_until(sock, buf, "\r\n0");

				//DEBUG
				//std::cout << "Received: " << nn << std::endl;
				//DEBUG
				//std::string s(asio::buffers_begin(buf.data()), asio::buffers_end(buf.data()));
				//std::cout << "====" << std::endl;
				//std::cout << s << std::endl;
				//std::cout << "====" << std::endl;

				//�������ͷ�����ֽ�
				std::istream input(&buf);
				getline(input, std::string());

				//����д��file��
				file = { boost::asio::buffers_begin(buf.data()),
					boost::asio::buffers_end(buf.data()) };
				//������һ��0�ַ�(Transfer-Encoding��chunkģʽ��0������־)
				file[file.find_last_of('0')] = '\0';


			}
			else if (length_site != std::string::npos)
			{
				//�����:
				//Content-Length:
				//��ʱ����ר��ȥ��Chunk���е�ͷ����β��
				auto length_site_return = ss.find("\r\n", length_site + 15);
				try {
					//�����׳��Ƿ������쳣
					int content_length = std::stoi(ss.substr(length_site + 15, length_site_return - length_site - 15));
					//��ȡʣ�������(ָ������)
					size_t nn = asio::read(sock, buf, asio::transfer_exactly(content_length - has_read));
					//DEBUG
					//std::cout << "Received: " << nn << std::endl;
					//DEBUG
					//std::string s(asio::buffers_begin(buf.data()), asio::buffers_end(buf.data()));
					//std::cout << "====" << std::endl;
					//std::cout << s << std::endl;
					//std::cout << "====" << std::endl;


					//����д��file��
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



			//[ע��]����ԡ��ı�����ʽ���ļ�������ȡ�ļ���ʱ��ϵͳ�Ὣ���е�"/r/n"ת����"/n"
			//��д���ļ���ʱ��ϵͳ�Ὣ"/n"ת����"/r/n"д��
			//��˴˴�ʹ�ö�����д��ͼƬ
			/*boost::filesystem*/std::ofstream fstream(save_path + file_name, std::ios::binary);
			fstream << file;
			fstream.close();

			break;

			//buf.consume(n);

			//����������
			//boost::this_thread::sleep(boost::posix_time::seconds(1));
			//try {
			//boost::system::error_code code;
			//size_t nn = asio::read_until(sock, buf ,"\r\n0");
			//Content-Length�ĳ��Ȱ���ͷ��
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
			//ʹ����������������,�޷��������
			//std::string s = //boost::locale::conv::from_utf(
				//boost::asio::buffer_cast<char>(buf.data());//, "GBK");
			//ʹ��asio�ĺ�����ȡ
			//std::string s(asio::buffers_begin(buf.data()), asio::buffers_end(buf.data()));
			//std::cout << s << std::endl;
			//std::cout << "====" << std::endl;
			//break;
		}

		//���رշ�������
		sock.shutdown(asio::ip::tcp::socket::shutdown_send);
	}
	//ʹ��ͬ��HTTP����,��Ҫ����BGMAuth����
	std::string SyncBGMHTTPRequest(std::string request) {
		//https://www.cnblogs.com/ribavnu/p/5084458.html
		//ͼƬһ�㲻��ʹ��Chunk����
		//һ�㶯̬��վ��ʹ��Chunk
		//���ȴ���һ��DNS����
		//asio::ip::tcp::resolver res(m_ios);
		//���������
		//asio::ip::tcp::resolver::query resolve_query(host, "80", asio::ip::tcp::resolver::query::numeric_service);
		//ͬ��DNS���� 
		//ʹ�ò��׳��쳣�汾,������������׳��Լ����쳣
		boost::system::error_code e;
		//asio::ip::tcp::resolver::iterator it = res.resolve(resolve_query, e);
		//if (e.value() != 0) {
		//	throw boost::system::system_error(bangumi_bot_errors::sync_http_dns_resolve_error);
		//}

		//����һ��socket,����open��,������������׳��Լ����쳣
		asio::ip::tcp::socket sock(m_ios);
		sock.open(asio::ip::tcp::v4(), e);
		if (e.value() != 0) {
			throw boost::system::system_error(bangumi_bot_errors::sync_http_sokect_open_filed);
		}
		//�����Ľ�β


		//ֱ��ʹ�������iterator
		asio::ip::tcp::resolver::iterator end;
		//asio::ip::tcp::resolver::iterator fix_it;
		asio::ip::tcp::resolver::iterator m_it = fix_it;
		//���ӿ��õ�ip
		while (m_it != end) {
			//ʹ�ò��׳��쳣�汾
			sock.connect(*m_it++, e);
			//����д���,���׳��Լ����쳣
			if (e.value() != 0) {
				if (m_it != end)
					continue;
				else {
					throw boost::system::system_error(bangumi_bot_errors::sync_http_dns_connect_error);
					//break;
				}

			}

			//��ʱ�Ѿ���ȷ���ӵ���һ��IP
			asio::write(sock, asio::buffer(request));

			//����buffer
			asio::streambuf buf;

			//����ֻ���յ�ͷ������
			//�ֿ�Content-Length��Chunk�����������
			//[ע��]��Ȼread_until���ܳ���,���Ƿ���read_size���ϸ�׼ȷ��<=���ж�ȡ������
			//������buf.consume()��������������Ƴ�
			size_t n = asio::read_until(sock, buf, "\r\n\r\n");
			//DEBUG
			//std::cout << "Received: " << n << std::endl;

			//���յ�ͷ�����ַ���
			std::string ss = 
				std::string(asio::buffers_begin(buf.data()), asio::buffers_begin(buf.data()) + n);
			if (ss.find("1.1 301 ") != std::string::npos) {
				//301�ض�������
				return "";
			}
			size_t dis = asio::buffers_end(buf.data()) - asio::buffers_begin(buf.data());
			size_t has_read = dis - n;
			//�����������е��Ѷ��ֽ�
			buf.consume(n);
			//DEBUG
			//std::cout << "--------\n"
			//	<< "dis = " << dis << std::endl
			//	<< "n = " << n << std::endl
			//	<< "has_read = " << has_read 
			//	<<"\n--------"<< std::endl;


			//Ĭ��ΪChunk
			

			size_t nn = asio::read_until(sock, buf, "\r\n0");
			std::string content;
			try {
				//�������һ��
				
				std::istream input(&buf);
				getline(input, std::string());
				//s = boost::locale::conv::from_utf(
				//	std::string(std::istreambuf_iterator<char>((request->m_response).m_response_stream), {}), "GBK");
				content = 
					std::string(boost::asio::buffers_begin(buf.data()),
						boost::asio::buffers_end(buf.data()));

				
				content.erase(content.find_last_of('0'));

				//�����Chunk�ϰ�
				//������Getcontentһ��
				if (true) {
					size_t chunk_delim = 0;
					//
					chunk_delim = content.find("\r\n");
					//
					while (chunk_delim != std::string::npos) {
						//һ������һ��"\r\n"
						auto over = content.find("\r\n", chunk_delim);
						if (over != std::string::npos)
							content.erase(chunk_delim, over - chunk_delim + 2);
						else {
#ifndef NDEBUG
							{
								bangumi::string debug_msg;
								debug_msg << "Content��ѭ������Contentʧ��" << ": Find���з���������";
								CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTP-GetResponseContent", debug_msg);
							}
#endif
						}
						//��ͼ������һ��
						chunk_delim = content.find("\r\n");
					}
				}
			}
			catch (std::exception) {
				throw boost::system::system_error(bangumi_bot_errors::http_get_contents_error);
			}
			//���رշ�������
			sock.shutdown(asio::ip::tcp::socket::shutdown_send);

			return content;

		}

		//���رշ�������
		//sock.shutdown(asio::ip::tcp::socket::shutdown_send);

		//
		return "";
	}
private:
	//����Ϊ˽�к���
	void resolve_url(std::string url, unsigned int port = 80) {
		m_url = url;
		m_port = port;

		//����
		asio::ip::tcp::resolver resolver(m_ios);

		asio::ip::tcp::resolver::query resolve_query(m_url,
			std::to_string(m_port), asio::ip::tcp::resolver::numeric_service);

		//����ѡ��ͬ������
		//��ֹ�첽˳������������
		try {
			fix_it = resolver.resolve(resolve_query);
		}
		catch (boost::system::system_error&) {
			//�׳��쳣
			if (m_url.empty())
				throw boost::system::system_error(bangumi_bot_errors::without_url_in_client);
			else
				throw boost::system::system_error(bangumi_bot_errors::dns_resolve_error_in_client);
		}
#ifndef NDEBUG
		asio::ip::tcp::resolver::iterator end;
		std::string test = "������DNS:\n";
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
	//Ϊ�˷�ֹ�߳��˳������´������߳�������ֹ
	//��˽��������̴߳洢
	std::map<int,std::shared_ptr<HTTPRequest>> m_requests_map;
	//Ϊ��֧������,��Ҫ����ID
	unsigned int id = 0;
	std::mutex id_mutex;
	//��Ϊ��ǰӦ�ô���������ֻ��Ҫ��һ��url������Դ����
	//��˼�¼asio::ip::tcp::resolver::iterator�������ؽ���
	asio::ip::tcp::resolver::iterator fix_it;
	std::string m_url;
	unsigned int m_port;
public:
	//ȫ�������ļ�bgm
	BangumiBotVaribel* m_bgm;
};


//����������
//ǰ����Ҫrequest��Ϣ�㹻(uri,host)
//ע��header����һ������Դ�һ��\r\n
inline std::string request_message(std::shared_ptr<HTTPRequest> request, HTTP_WAY way, std::string header = "", std::string content = "") {
	switch (way)
	{
		//��Ϊ��һ����ֵ,ʹ��std::move����ֵ
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


//ȡ����Ӧ���ĵ�����
inline void GetResponseContent(std::shared_ptr<HTTPRequest> request, std::string &s, bool isChunk = true,
	int ec=0 )
{
	//https://stackoverflow.com/questions/3203452/how-to-read-entire-stream-into-a-stdstring
	//std::string s(std::istreambuf_iterator<char>((request->m_response).m_response_stream), {});
	//https://faithandbrave.hateblo.jp/entry/20110324/1300950590
	//�÷������ȡ�������ݰ���֮ǰ��ͷ��
	//std::string s = boost::asio::buffer_cast<const char* >((request->m_response).m_response_buf.data());
	//https://stackoverflow.com/questions/28929699/boostasio-read-n-bytes-from-socket-to-streambuf
	//std::string s(boost::asio::buffers_begin((request->m_response).m_response_buf.data()),
	//	boost::asio::buffers_end((request->m_response).m_response_buf.data()));
	//https://www.boost.org/doc/libs/1_67_0/libs/locale/doc/html/charset_handling.html
	//GBK������Ҳ֧������
	//Transfer-Encoding��chunkģʽ������
	//http://jizhao.blog.chinaunix.net/uid-28458801-id-5064196.html	
	//chunk���뽫���ݷֳ�һ��һ��ķ�����Chunked���뽫ʹ�����ɸ�Chunk�������ɣ�
	//��һ����������Ϊ0 ��chunk��ʾ������ÿ��Chunk��Ϊͷ�������������֣�ͷ������ָ�����ĵ��ַ�������ʮ�����Ƶ����� ��
	//��������λ��һ�㲻д�������Ĳ��־���ָ�����ȵ�ʵ�����ݣ�������֮���ûس�����(CRLF) ����
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
		//�������һ��
		if (isChunk)
			getline((request->m_response).m_response_stream, std::string());
		//s = boost::locale::conv::from_utf(
		//	std::string(std::istreambuf_iterator<char>((request->m_response).m_response_stream), {}), "GBK");
		s = //boost::locale::conv::from_utf(
			std::string(boost::asio::buffers_begin((request->m_response).m_response_buf.data()),
				boost::asio::buffers_end((request->m_response).m_response_buf.data()));//, "GBK");

		//������һ���������301�ض�������
		//auto maybe_301 = s.find("Location: ");
		//if (maybe_301 != std::string::npos) {
		//	//��ȷ�������ض���
		//	//
		//	auto location_num = s.find("\r", maybe_301);
		//	std::string new_uri = s.substr(maybe_301 + 10, location_num - maybe_301 - 10);
		//	request->set_uri(new_uri);
		//	//����m_response_buf
		//	(request->m_response).m_response_buf.consume((request->m_response).m_response_buf.size());
		//	//һ����Subject���ض���,ֱ��ʹ��GET
		//	request->set_request(request_message(request, HTTP_WAY::GET));
		//	//Callback�Ѿ��������,���ֱ��exec����
		//	request->execute();
		//}


		//������һ��0�ַ�(Transfer-Encoding��chunkģʽ��0������־)
		//[ע��] ����ʹ��'\0'���ַ�������0, property_tree����Ϊdata������������read_json
		//s[s.find_last_of('0')] = '\0';
		//s[361] = *R"(\)";
		if (isChunk)
			s.erase(s.find_last_of('0'));

		//ע��:��Search�������س���Json����,�������Ҫѭ������content
		
		if (isChunk) {
			size_t chunk_delim = 0;
			//
			chunk_delim = s.find("\r\n");
			//
			while (chunk_delim != std::string::npos) {
				//һ������һ��"\r\n"
				auto over = s.find("\r\n", chunk_delim);
				if(over!=std::string::npos)
					s.erase(chunk_delim, over - chunk_delim + 2);
				else {
#ifndef NDEBUG
					{
						bangumi::string debug_msg;
						debug_msg << "Content��ѭ������Contentʧ��" << ": Find���з���������";
						CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTP-GetResponseContent", debug_msg);
					}
#endif
				}
				//��ͼ������һ��
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


	
	//���string�����е�ת���'\'(��ʵ���յ���\u��ֻ��\u������\\u)
	//s.erase(std::remove(s.begin(), s.end(), '\\'), s.end());
	//s = boost::locale::conv::from_utf(s, "Unicode");
	//boost::asio::basic_streambuf<std::allocator<wchar_t>>::const_buffers_type cbt = (request->m_response).m_response_buf.data();
	//std::string request_data(boost::asio::buffers_begin(cbt), boost::asio::buffers_end(cbt));
	//
	//std::cout << "xxxxx\n" << s1 << "xxxxxx\n" << std::endl;
}

//��װ��������HTTP
//uri:�Ѿ�������url,���Լ�дΪ/user/XXX��
//void HTTPSendRequestFixed(std::string &uri, std::string &request) {
//
//	std::shared_ptr<HTTPRequest> request_one =
//		http_client.create_request_fixed(1);
//	//create fixed����²���Ҫset_host
//	//request_one->set_host("api.bgm.tv");
//	request_one->set_uri("/user/wz97315");
//	//����ͷ��
//	request_one->set_request(request_message(request_one, HTTP_WAY::GET));
//	//ִ���첽����
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

//·������
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
//save_path�Ѿ������˿�Q�е��ṩ��cache_pathĿ¼Ϊ��Ŀ¼,�����Ѿ������һ��/
//[ע��]��ʹ��SUBJECT_PIC_PATH�Ⱥ�����Ϊsave_path
//save_path����ʹ�ú궨��
//save_name�ǲ�����׺����
//ע�ⲻ�����ļ�·��,Ҳ�����Զ�����,������ֶ�ȷ���ļ��д���
std::pair<DownloadStatus, std::shared_ptr<boost::thread>> HTTPDownload(HTTPClient& http_client, std::string host, std::string uri, std::string save_path, std::string save_name) {

	//TODO��ʱû�в��Ժ�������,ͬʱҲ���ǵ���ʱ�ò���
	//
	//dir_path == Cache/User/
	std::string dir_path = (http_client.m_bgm->cache_path) + save_path;
	boost::filesystem::path pic_path;
	std::string absolute_dir_path;
	if (uri.find_last_of(".jpg") != std::string::npos) {
		save_name += ".jpg";
		//����Ӧ�ü��Ͼ���·������ļ��Ĵ����� D:\Program\��Q Pro\data\image\Cache\User\???.???
		pic_path = http_client.m_bgm->Bangumi_Img_Dir + (dir_path + save_name);
	}
	else if (uri.find_last_of(".png") != std::string::npos) {
		save_name += ".png";
		pic_path = http_client.m_bgm->Bangumi_Img_Dir + (dir_path + save_name);
	}
	else {
		//����ֱ���׳�����
		throw boost::system::system_error(bangumi_bot_errors::pic_download_with_unknown_url);
	}
	//��������·��(��Ե�) Cache/User/???.???
	//file_path = dir_path + save_name;
	//����·�� D:/Program/��Q Pro/data/image/Cache/User/
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
	//����ļ��Ƿ��Ѿ�����
	//����Ϊ�˷���ֻ��Կ��ܳ��ֵ��ļ���׺
	//TODO:�����ļ�ʱ���������Ƿ����
	if (boost::filesystem::exists(pic_path)) {
		//ֱ�ӽ���
		//�������ͼ����FUNC�е��õ�API�����е���join()����ͼƬ�������
		return{ DownloadStatus::HasFinished, nullptr };
	}
	//request 
	std::string request =
		"GET " + uri + " HTTP/1.1\r\n"
		"Host: " + host + "\r\n" "\r\n";
	//����һ���µĽ���
	if (bgm.CheckThreadSize()) {
		//�п��п��õĽ���
		std::shared_ptr<boost::thread> download_thread
		(new boost::thread([&http_client, host, uri, request, absolute_dir_path, save_name]() {
#ifndef NDEBUG
			std::ostringstream oss;
			oss << boost::this_thread::get_id();
			std::string idAsString = oss.str();
			std::string test = "�����µ��߳� ID: " + idAsString;
			CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTP-DOWNLOAD", test.c_str());

#endif
			try {
				//�˴�ʹ��ͬ��HTTP����������
				http_client.SyncHTTPRequest(host, request, absolute_dir_path, save_name);
			}
			catch (boost::system::system_error& e) {
				std::string test = "���߳�ͼƬ����ʧ��,����ID:" + std::to_string(e.code().value()) + " ������Ϣ:" + e.what();
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTP-DOWNLOAD", test.c_str());
			}
			bgm.AddAVAThreadSize();
	}));
		//�����̵߳�����,�����̵߳���join()
		return{ DownloadStatus::MultiThread, download_thread };
}
	else {
		//ʹ�õ��߳�

		////ʹ��HTTP����
		//std::shared_ptr<HTTPRequest> request_one =
		//	http_client.create_request(1);
		////boost::thread use([request_one]() {
		//request_one->set_host(host);
		//request_one->set_uri(uri);
		//request_one->set_request(request_message(request_one, HTTP_WAY::GET));
		//request_one->execute();

		try {
			//�˴�ʹ��ͬ��HTTP����������
			http_client.SyncHTTPRequest(host, request, absolute_dir_path, save_name);
		}
		catch (boost::system::system_error& e) {
			std::string test = "���߳�ͼƬ����ʧ��,����ID:" + std::to_string(e.code().value()) + " ������Ϣ:" + e.what();
			CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTP-DOWNLOAD", test.c_str());
		};

		return{ DownloadStatus::SingleThread, nullptr };
	}

}

//����֧��http
//�˺������׳��쳣
//save_path��ʹ�ú궨��
std::pair<DownloadStatus, std::shared_ptr<boost::thread>> PicDownload
(HTTPClient& http_client, std::string http_url, std::string save_path, std::string save_name, std::string &file_path, bool refresh = false) {
	if (http_url.empty()) {
		//˵����ͼƬ�����ڣ�null
		//ʹ��Ĭ�ϵ�404ͼƬ
		file_path = bgm.not_found_pic_path;
		//ֱ�ӷ���
		return{ DownloadStatus::SingleThread, nullptr };
	}
	//dir_path == Cache/User/
	std::string dir_path = (http_client.m_bgm->cache_path) + save_path;
	boost::filesystem::path pic_path; 
	std::string absolute_dir_path;
	if (http_url.find_last_of(".jpg") != std::string::npos) {
		save_name += ".jpg";
		//����Ӧ�ü��Ͼ���·������ļ��Ĵ����� D:\Program\��Q Pro\data\image\Cache\User\???.???
		pic_path = http_client.m_bgm->Bangumi_Img_Dir + (dir_path + save_name );
	}
	else if (http_url.find_last_of(".png") != std::string::npos) {
		save_name += ".png";
		pic_path = http_client.m_bgm->Bangumi_Img_Dir + (dir_path + save_name );
	}
	else {
		//����ֱ���׳�����
		throw boost::system::system_error(bangumi_bot_errors::pic_download_with_unknown_url);
	}
	//��������·��(��Ե�) Cache/User/???.???
	file_path = dir_path + save_name;
	//����·�� D:/Program/��Q Pro/data/image/Cache/User/
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
	//����ļ��Ƿ��Ѿ�����
	//����Ϊ�˷���ֻ��Կ��ܳ��ֵ��ļ���׺
	//TODO:�����ļ�ʱ���������Ƿ����
	if (boost::filesystem::exists(pic_path)&&!refresh) {
		//ֱ�ӽ���
		//�������ͼ����FUNC�е��õ�API�����е���join()����ͼƬ�������
#ifndef NDEBUG
		{
			bangumi::string debug_msg;
			debug_msg << "ͼƬ����: " << pic_path.string();
			CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-PIC-DOWNLOAD", debug_msg);
		}
#endif
		return{ DownloadStatus::HasFinished, nullptr };
	}
	std::string uri;
	std::string host;
	//����һ��http_url
	//��������пո�
	boost::erase_all(http_url," ");
	//����
	//http://lain.bgm.tv/pic/user/l/000/09/29/92981.jpg?r=1553752474
	auto site = http_url.find("http:");
	if (site != std::string::npos) {
		//����ɾ��http://
		http_url.erase(site, 7);
		
		//�Ƴ�����Ĳ���
		//http_url.erase(http_url.find_first_of('?'));
		//
		auto delim = http_url.find_first_of('/');
		host = http_url.substr(0, delim);
		//Ҫ����/
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
	//����һ���µĽ���
	if (bgm.CheckThreadSize()) {
		//�п��п��õĽ���
		std::shared_ptr<boost::thread> download_thread
		(new boost::thread([&http_client, host, uri, request, absolute_dir_path, save_name]() {
#ifndef NDEBUG
			{
				std::ostringstream oss;
				oss << boost::this_thread::get_id();
				std::string idAsString = oss.str();
				std::string test = "�����µ��߳� ID: " + idAsString +
					"\n�ܳش�С: " + std::to_string(bgm.threadpool_size) + "\n"
					"���ô�С: " + std::to_string(bgm.curr_thread_size);
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTP-DOWNLOAD", test.c_str());
			}
#endif
			try {
				//�˴�ʹ��ͬ��HTTP����������
				http_client.SyncHTTPRequest(host, request, absolute_dir_path, save_name);
			}
			catch (boost::system::system_error& e) {
				std::string test = "���߳�ͼƬ����ʧ��,����ID:" + std::to_string(e.code().value()) + " ������Ϣ:" + e.what();
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTP-DOWNLOAD", test.c_str());
			}
			bgm.AddAVAThreadSize();
		}));
		//�����̵߳�����,�����̵߳���join()
		return{ DownloadStatus::MultiThread, download_thread };
	}
	else {
		//ʹ�õ��߳�

		////ʹ��HTTP����
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
			debug_msg << "ʹ�õ��߳�����";
			CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-PIC-DOWNLOAD", debug_msg);
		}
#endif

		try {
			//�˴�ʹ��ͬ��HTTP����������
			http_client.SyncHTTPRequest(host, request, absolute_dir_path, save_name);
		}
		catch (boost::system::system_error& e) {
			std::string test = "���߳�ͼƬ����ʧ��,����ID:" + std::to_string(e.code().value()) + " ������Ϣ:" + e.what();
			CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTP-DOWNLOAD", test.c_str());
		};

		return{ DownloadStatus::SingleThread, nullptr };
	}

}


#endif


