#pragma once
#include "Init.h"
#include "ParserElements.h"
#include <iostream>

using namespace boost;
//typedef void(*BGMServerCallback)(unsigned,std::string,int64_t,std::string);

//����QQ����
std::string EncryptQQ(const int64_t &qq) {
	auto &table = num_bimap.left;

	std::string result;
	std::string qq_str = std::to_string(qq);
	for (auto& c : qq_str) {
		result += table.at(c);
	}
	return std::move(result);
}
//����QQ����
int64_t DecryptQQ(const std::string &unqq) {
	auto &table = num_bimap.right;

	std::string result;
	for (auto& c : unqq) {
		result += table.at(c);
	}
	return std::stoull(result);

}
//����QQ����һ��״̬
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
//��ö�����ַ
std::string GetRedirectUrl(int64_t qq) {
	std::string unqq = EncryptQQ(qq);
	std::string state = EncryptState(qq);

	//std::string redirect_url = bgm.redirect_url;
	std::string redirect_url = bgm.redirect_url + "?nu=" + unqq + "&state=" + state;
	//����URL����,����ᵼ�»ص���ַ�޷�Я������->? &
	redirect_url = url_encode(redirect_url);
	return std::move(redirect_url);
}



class Service {

public:
	//���캯��
	Service(std::shared_ptr<boost::asio::ip::tcp::socket> sock) :
		m_sock(sock),
		m_response_status_code(200) // �ٶ��ɹ�
	{};


	void start_handling() {

#ifndef NDEBUG
		{
			bangumi::string debug_msg;
			debug_msg << "����һ��start_handling���̳ɹ�";
			CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTPServer", debug_msg);
		}
#endif
		//�첽��ȡ��һ��,ʵ����һ���Ѿ�������ȫ��
		asio::async_read_until(*m_sock.get(),
			m_request,
			"\r\n",
			[this](
				const boost::system::error_code& ec,
				std::size_t bytes_transferred)
		{
			//��һ�ж�ȡ��ĺ����ݴ���
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
			debug_msg << "on_request_line_received:��ȡ���ַ�:" << std::string(boost::asio::buffers_begin(m_request.data()),
				boost::asio::buffers_end(m_request.data()));;
			CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTPServer", debug_msg);
		}
#endif
		if (ec.value() != 0) {
			////�ж�ȡʱ�д�����
			////ֱ�������׳�
			//throw ec;


			m_response_status_code = 0;
			//һ���д����on_finish����
			on_finish();
			return;

		}
		//����������
		std::string request_line;
		std::istream request_stream(&m_request);
		std::getline(request_stream, request_line, '\r');
		//�Ƴ���һ��\n�ַ�
		request_stream.get();
		//��������
		std::string request_method;
		std::istringstream request_line_stream(request_line);
		request_line_stream >> request_method;
		//ֻ����Get����
		if (request_method.compare("GET") != 0) {
			//������on_finish
			m_response_status_code = 0;
			on_finish();
			return;
		}
		//�������Դ,��Ҫ�����Ϊ�ؼ�
		request_line_stream >> m_requested_resource;
		std::string request_http_version;
		request_line_stream >> request_http_version;
		//Http�汾 //�����,php�е�file get contents��1.0��...
		//if (request_http_version.compare("HTTP/1.1") != 0) {
		//	//������on_finish
		//	m_response_status_code = 0;
		//	on_finish();
		//	return;
		//}
#ifndef NDEBUG
		{
			bangumi::string debug_msg;
			debug_msg << "�յ�����"
				>> "request_method = " << request_method
				>> "request_http_version = " << request_http_version
				>> "m_requested_resource = " << m_requested_resource;
			CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTPServer", debug_msg);
		}
#endif
		process_request();
		send_response();
		on_finish();
		//������Ӧ����ȡͷ��
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
		//����uri
		size_t start_pos;
		size_t temp;
		//���ҵ�?
		auto first_para_start = m_requested_resource.find_first_of('?');
		//�����󵽵�code��ʼλ��
		start_pos = m_requested_resource.find("code=", first_para_start) + 5;
		//�ҵ���һ��$
		temp = m_requested_resource.find_first_of('&', start_pos);
		code = m_requested_resource.substr(start_pos, temp - start_pos);

		//�����󵽵�to_code��ʼλ��(���ܺ��qq)
		start_pos = m_requested_resource.find("to_code=", temp) + 8;
		//�ҵ���һ��$
		temp = m_requested_resource.find_first_of('&', start_pos);
		std::string unqq = m_requested_resource.substr(start_pos, temp - start_pos);
		if (unqq.empty()) {
			//˵���Ǵ���ķ��ʣ���û�е�½���µ���Ϣ��ʧ
			//�Ƿ�������
			//ֱ�����
#ifndef NDEBUG
			{
				bangumi::string debug_msg;
				debug_msg << "���ڷǷ���reg����";
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTPServer", debug_msg);
			}
#endif
			m_sock->shutdown(
				asio::ip::tcp::socket::shutdown_receive);
			//delete this;
			return;
		}
		//����QQ
		qq = DecryptQQ(unqq);


		//�����󵽵�state��ʼλ��(��֤��Ϣ)
		start_pos = m_requested_resource.find("state=", temp) + 6;
		//����λ��
		//temp = m_requested_resource.length();
		//sate�ǹ̶���λ��
		state = m_requested_resource.substr(start_pos, 4);
		if (state.empty()) {
			//˵���Ǵ���ķ��ʣ���û�е�½���µ���Ϣ��ʧ
			//�Ƿ�������
			//ֱ�����
#ifndef NDEBUG
			{
				bangumi::string debug_msg;
				debug_msg << "���ڷǷ���reg����";
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTPServer", debug_msg);
			}
#endif
			m_sock->shutdown(
				asio::ip::tcp::socket::shutdown_receive);
			//delete this;
			return;
		}
		//��֤
		if (EncryptState(qq) != state) {
			//�Ƿ�������
			//ֱ�����
#ifndef NDEBUG
			{
				bangumi::string debug_msg;
				debug_msg << "���ڷǷ���reg����";
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
			debug_msg << "HTTPServer��������ɹ�!";
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

		//���ܽ�����ֱ�ӷ���200
		asio::const_buffer response_buffer = asio::buffer("HTTP/1.1 200 OK");


		//�첽д��
		asio::async_write(*m_sock.get(),
			response_buffer,
			[this](
				const boost::system::error_code& ec,
				std::size_t bytes_transferred)
		{
#ifndef NDEBUG
			{
				bangumi::string debug_msg;
				debug_msg << "�ɹ���PHP����200��Ӧ!";
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTPServer", debug_msg);
			}
#endif
			//�����Ľ��
		});
	}


	//����
	void on_finish() {
		//
		if (m_response_status_code != 200) {
			//˵��ʧ��
#ifndef NDEBUG
			{
				bangumi::string debug_msg;
				debug_msg << "on_finishʧ��:��ȡ���ַ�:" << std::string(boost::asio::buffers_begin(m_request.data()),
					boost::asio::buffers_end(m_request.data()));;
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTPServer", debug_msg);
			}
#endif

#ifndef NDEBUG
			{
				bangumi::string debug_msg;
				debug_msg << "on_finish ʧ�ܴ���";
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTPServer", debug_msg);
			}
#endif
			bangumi::string msg = "ʧ��";
			SendMsg.at(BgmRetType::Private)(ac, qq, msg);
		}
		else {
			//Main Func
			//˵���ɹ�
			//���ż���������һ��token
			//=============================
			bangumi::string content;
			content << "grant_type=authorization_code"
				<< "&code=" << code
				<< "&redirect_uri=" << GetRedirectUrl(qq)
				<< "&client_id=" << bgm.bangumi_client_id
				<< "&client_secret=" << bgm.bangumi_client_secret;

			//post�е�www������ʵ��Ϊ�˷�ֹURI������������������content��,POSTʱһ����������ʹ��Content-Lengthָ����С
			//�������ֲ���������ȡ����������
			bangumi::string header("Cache-Control: no-cache\r\n"
				"Content-Type: application/x-www-form-urlencoded\r\n");
			header << "Content-Length: " << content.length() << "\r\n";

			

			//����һ������
			std::shared_ptr<HTTPRequest> request_one =
				http_client.create_request_fixed(http_client.GetID());

			//��Ҫ����Ϣ��ӵ��,ֱ�ӹ���һ��bangumi::BGMRetParam
			request_one->set_ret_param(bangumi::BGMRetParam{ BgmRetType::Private , qq, 0, 0, "", bangumi::BGMCodeExtraVar() });
			request_one->set_host("bgm.tv");
			request_one->set_uri("/oauth/access_token");
			//ֻ��������Ŀʱ��ҪCookie: chii_searchDateLine
			request_one->set_request(request_message(request_one, HTTP_WAY::POST,
				header,
				content));
			//���ûص�����
			request_one->set_callback([](std::shared_ptr<HTTPRequest>request_one, bangumi::BGMRetParam param,int ec) {
				//Ҫ�ظ���string
				bangumi::string msg;
				//�ص�����:��ʱ�Ѿ��������Ӧ���ĵĶ�ȡ
				std::string json;
				try {
					GetResponseContent(request_one, json,true,ec);
#ifndef NDEBUG
					{
						bangumi::string debug_msg;
						debug_msg << "�յ���Ӧ��Json"
							>> json.c_str();
						CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTP", debug_msg);
					}
#endif

					//һ�������ע���û�
					bool registed = false;
					//�󶨵��Ƿ���ͬһ��ID
					bool same_bgm_id = true;

					auto auth = Resolve::Resolve_Auth(json);
					//����Ƿ������ע���QQ�û�

					bangumi::string pre_query;
					pre_query << "SELECT user_id, user_qq, user_bangumi FROM bgm_users WHERE "
						<< "user_qq = " << std::to_string(param.qq);
					//MYSQL_RES* pre_result = nullptr;
					BGMSQLResult pre_result;
					unsigned long pre_affect_rows_num = sql_pool.ExecQuery(pre_query, pre_result);
					try
					{
						SQLCheckResult(pre_affect_rows_num);
						//��Ӱ����������0ʱ����ע��
						if (pre_affect_rows_num > 0) {
							//һ�������ע���û�
							registed = true;
							//ʹ��fetch_row��������ԽϺ�
							//MYSQL_ROW rows = mysql_fetch_row(pre_result);
							//�ж�ǰ������ID�Ƿ�һ��
							if (std::to_string(auth.user_id) != std::string(pre_result[2])) {
								same_bgm_id = false;
							}
							//��ע���QQ��ʾ��Ϣ
							msg << "ԭ[" << pre_result[0] << "]����Լ"
								<< ": QQ: " << pre_result[1]
								<< " x Bangumi-ID: " << pre_result[2];
							//ע�������free,free�Ժ�rowsҲ��ʧЧ
							//mysql_free_result(pre_result);
						}

					}
					catch (boost::system::system_error& e)
					{
#ifndef NDEBUG
						{
							bangumi::string debug_msg;
							debug_msg << "Pre-SQLCheckResult�в�ѯʧ��:"
								<< std::to_string(pre_affect_rows_num)
								>> e.what();

							CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Auth-SQL-Pre-SQLCheckResult", debug_msg);
						}
#endif	
						//����free
						//mysql_free_result(pre_result);
						//msg << "�˴���Լ������˵�С����...\n";
						//throw e;
					}


					//��SQL����
					//++++++++++++++++++++
					bangumi::string query;
					//query << "insert into bgm_users(user_qq,user_bangumi,user_access_token,user_refresh_token)"
					//	<< "values(" << std::to_string(param.qq) << ", " << std::to_string(auth.user_id)
					//	<< ", " << auth.access_token << ", " << auth.refresh_token << ")";
					//INSERT INTO bgm_users VALUES(NULL,123456789,12332,"d22a58aacee7925860014521b37a8cf78aa4afd9","f606d13c8493ca12f256fb8a43c54f36b2edd666",0,0,NULL,'1000-01-01 00:00:00',0)
					if (registed) {
						//�ٴ�ע��
						query << "UPDATE bgm_users SET "
							<< "user_bangumi="<<auth.user_id<<","
							<< "user_access_token='" << auth.access_token <<"',"
							<< "user_refresh_token='" << auth.refresh_token <<"'"
							<< " WHERE user_qq="<<param.qq;
					}
					else {
						//�״�ע��
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
						debug_msg << "SQL���:"
							>> query;
						CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Auth-SQL", debug_msg);
					}
#endif
					//std::string query("CREATE TABLE debug_example (id int not null, my_name varchar(50)");
					//���ڴ洢���
					BGMSQLResult result;
					unsigned long affect_rows_num = sql_pool.ExecQuery(query, result);
					try
					{
						SQLCheckResult(affect_rows_num);

#ifndef NDEBUG
						{
							bangumi::string debug_msg;
							debug_msg << "Ӱ�������:"
								<< std::to_string(affect_rows_num);
							CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Auth-SQL", debug_msg);
						}
#endif
						//mysql_free_result(result);
						//�ɹ���QQ��Ϣ����
						if (registed) {
							//�ٴ�ע��
							if (same_bgm_id) {
								//��ͬID��ˢ����Ϊ
								msg << " ����ǩ!";
							}
							else {
								//���İ���Ϊ
								msg << " ʧЧ..."
									>> "--------"
									>> "����Լ: QQ: " << std::to_string(param.qq) 
									<< " x Bangumi-ID: " << auth.user_id << " ��Ч��!";
							}

						}
						else {
							//�״�ע��
							msg << "QQ: " << std::to_string(param.qq)
								<< " x Bangumi-ID: " << auth.user_id
								<< " ��ɵ�Լ!";
						}

					}
					catch (boost::system::system_error& e)
					{
#ifndef NDEBUG
						{
							bangumi::string debug_msg;
							debug_msg << "SQLCheckResult���������: "
								<< std::to_string(affect_rows_num);

							CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Auth-SQL-SQLCheckResult", debug_msg);
						}
#endif	
						//����free�ٴ��׳�
						//mysql_free_result(result);
						//ʧ�ܵ�QQ��Ϣ����
						msg << "�˴���Լ������˵�С����...\n";
						throw e;
					}
					//++++++++++++++++++++



				}
				catch (boost::system::system_error & e) {
					//����������쳣,ʹ���쳣�ظ�(����û���ҵ��û���)
					//���ﲻʹ��Reply�����ֱ�Ӹ�ֵ�ظ�
					//�˴����ܴ�����Ȩʧ�ܻظ�����ʹ�Ѿ�����ˣ���������������쳣
					if (e.code() != boost::system::system_error(bangumi_bot_errors::auth_request_error).code()) {

						msg << e.what();
						//msg << "[" << param.cur_str << "]...";
					}
				}

				//���ͻظ�
				if (!msg.empty()) {
					SendMsg.at(BgmRetType::Private)(ac, param.qq, msg);
				}
				//��httpclient�Ƴ����ôӶ���������
				
				http_client.RemoveID(request_one->get_id());
			});
			request_one->execute();
			//=============================

			//�ظ�QQ��Ϣ DEBUG
			//bangumi::string msg;
			//msg << "code = " << code
			//	>> "state = " << state;
			//SendMsg.at(BgmRetType::Private)(ac, qq, msg);
		}
		//�������
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
//һ��TCP�׽��ֽ�����
class Acceptor {
public:
	//���������캯��
	Acceptor(asio::io_service& ios, unsigned short port_num)
		:m_ios(ios)
		//m_acceptor(m_ios, asio::ip::tcp::endpoint(asio::ip::address_v4::any(), port_num))
	{
		m_acceptor.reset(new asio::ip::tcp::acceptor(m_ios, asio::ip::tcp::endpoint(asio::ip::address_v4::any(), port_num)));
		m_isStopped.store(false);
	}
	//��ʼ����
	void Start() {
#ifndef NDEBUG
		{
			bangumi::string debug_msg;
			debug_msg << "Acceptor Start~";
			CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTPServer-Acceptor", debug_msg);
		}
#endif
		//��������ͬ��socket,����open֮�⻹��Ҫlisten
		m_acceptor->listen();
		InitAccept();
	}

	void Stop() {
		//m_acceptor->cancel();
		m_isStopped.store(true);
	}
private:
	void InitAccept() {
		//���ȴ���һ��sock�Ե�������һ������ʱ���ڴ������ݴ���
		std::shared_ptr<asio::ip::tcp::socket>
			sock(new asio::ip::tcp::socket(m_ios));

		//�첽accept,���������sock�������ݴ���
		m_acceptor->async_accept(*sock,
			[this, sock](const boost::system::error_code& error) {
			//�ɹ�accept�Ļص�����
#ifndef NDEBUG
				{
					bangumi::string debug_msg;
					debug_msg << "Acceptor������һ������";
					CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTPServer-Acceptor", debug_msg);
				}
#endif
				if (!m_isStopped.load())
				{
#ifndef NDEBUG
					{
						bangumi::string debug_msg;
						debug_msg << "m_isStopped Ϊ false: ִ��onAccept";
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
						debug_msg << "m_isStopped Ϊ true: ��ִ��onAccept��ֱ���˳�";
						CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTPServer-Acceptor", debug_msg);
					}
#endif
					return;
				}

		});

		//ͬ��Ҳ������
		//ͬ��accept
		//boost::system::error_code error;
		//m_acceptor.accept(*sock, error);
		//onAccept(error, sock);
	}

	void onAccept(const boost::system::error_code&ec,
		std::shared_ptr<asio::ip::tcp::socket>sock) {
		if (ec.value() == 0) {
			//û�д���Ļ�,������������
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
			//ֱ�ӷ���
			return;
		}
		//���û�е���Stop����һ�δ���������,ÿһ������һ��Init
		//��ʹ��stop��,����Чʱ������������Ӧ
		if (!m_isStopped.load() && m_acceptor != nullptr) {
			InitAccept();
		}
		else {
			//�˴���Ӧ�����Լ��ر�,Ӧ�����첽ios�����ر�,�����Ǳ߻��쳣
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
		//�޷��ڴ˴����м�ʱ���ĳ�ʼ��
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
			//����Ѿ�����Start��,�����
			return;
		}
		acc.reset(new Acceptor(m_ios, port_num));
		acc->Start();
		//��ʱStop
		m_timer->expires_from_now(boost::asio::chrono::minutes(5));
		m_timer->async_wait([this](const boost::system::error_code&) {


#ifndef NDEBUG
			{
				bangumi::string debug_msg;
				debug_msg << "HTTPServer ���ڳ�ʱ��Stop~";
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-HTTPServer-Start", debug_msg);
			}
#endif
			this->Stop();
		});
		

		//���������Ѿ�����HTTP_Client��ios��run(),��˲��ٴ���Ios
		//for (unsigned int i = 0; i < thread_pool_size; i++) {
		//	std::unique_ptr<std::thread> th(
		//		//https://blog.csdn.net/jlusuoya/article/details/75299096
		//		//�����б�
		//		new std::thread([this]() {
		//		m_ios.run();
		//	})
		//	);
		//	m_thread_pool.push_back(std::move(th));
		//}

		//��Start��������ʱ��
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
	//m_ios��HttpClient���ݹ���
	asio::io_service& m_ios;
	//����HttpClient��iosԭ��,����Ҫwork
	//std::unique_ptr<asio::io_service::work>m_work;
	std::unique_ptr<Acceptor>acc;
	//std::vector<std::unique_ptr<std::thread>>m_thread_pool;
	std::shared_ptr<asio::steady_timer> m_timer;
};

//����ȫ�ֱ���
HTTPServer http_server(http_client.GetIOS());


int Test_HTTPServer() {
	//const unsigned int DEFAULT_THREAD_POOL_SIZE = 2;

	try {
		HTTPServer srv(http_client.GetIOS());

		//ͨ���ڲ���Ӧ�ó��������ڲ�������߳�������ͨ�ù�ʽ�Ǽ��������2�Ĵ�������
		//ʹ��std::thread::hardware_concurrency��̬��������ȡ������������
		//����,�˷������ܷ���0
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