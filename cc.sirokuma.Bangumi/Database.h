#include "GlobalVariable.h"
#include "mysql.h"

#include <memory.h>
#include <iostream>

#ifndef BANGUMI_DATABASE_H
#define BANGUMI_DATABASE_H

#pragma comment(lib,"mariadbclient.lib")
#pragma comment(lib,"shlwapi.lib")

#define QUERY_FAILED -1
#define CON_FAILED -2
//#define MYSQL_OPT_RECONNECT

#ifndef NDEBUG

void show_error(MYSQL *mysql)
{
	bangumi::string msg;
	msg << "Error(" << mysql_errno(mysql) << ") ["
		<< mysql_sqlstate(mysql) << "] \""
		<< mysql_error(mysql) << "\"";
	CQ_addLog(ac, CQLOG_DEBUG, "Bot-SQL", msg);

}
#endif

inline void InitSQL() {

}
//��װ��ԭ����c�汾Res
class BGMSQLResult {
	friend class SQLPool;
public:
	//һ�㹹�캯��
	BGMSQLResult(MYSQL_RES* _res)
		:mysql_res(_res),
		rows(mysql_fetch_row(_res))
	{
		//rows = mysql_fetch_row(_res);
	}
	//Ĭ�Ϲ��캯��
	BGMSQLResult()
		:mysql_res(NULL),
		rows(NULL)
	{
		//rows = mysql_fetch_row(_res);
	}
	//�����������ͷ�res
	~BGMSQLResult()
	{
		mysql_free_result(mysql_res);
	}
	//��ȡRows����
	const char* operator[] (int i) {

		//����[]�����
		return *(rows + i);
	}
	//��ȡ��N��,ע�����ǲ��������
	//��0��ʼ����,������ʾ�����Ƽ���
	void FetchRow(int num) {
		for (int i = num; i > 0; --i)
			RefreshRow();
	}

private:
	//��mysql_res����ֵ�����rows
	void RefreshRow() {
		rows = mysql_fetch_row(mysql_res);
	}

	MYSQL_RES*& operator() () {
		//����(void)
		return mysql_res;
	}
private:
	MYSQL_RES* mysql_res;
	MYSQL_ROW rows;
};
#define NULL_RES 

//һ��SQL����
class SQLConnection {
	friend class SQLPool;
public:
	SQLConnection(const std::string& address, const std::string& user_name,
		const std::string& user_password, const std::string& db_name)
		:bFree(true)
	{
		//��ʼ��mysql
		mysql = mysql_init(NULL);
		//�������ݿ�����
		if (!mysql_real_connect(mysql, address.c_str(), user_name.c_str(), user_password.c_str(),
			db_name.c_str(), 0, "/tmp/mysql.sock", 0))

#ifndef NDEBUG
			//��ʾ������Ϣ
			show_error(mysql);
#else
		{
			//bAva = false;
			CQ_addLog(ac, CQLOG_ERROR, "Bot-SQL", "���ݿ�����ʧ��!");
		}

#endif
		else {
			//bAva = true;
			//�����ַ���
			SetCharacter();
			CQ_addLog(ac, CQLOG_INFOSUCCESS, "Bot-SQL", "���ݿ����ӳɹ�!");
		}
	}
	~SQLConnection() {
		bFree = false;
		//delete mysql
		mysql_close(mysql);
		//delete result
		//mysql_free_result(result);
	}
	//��ʽת��bool
	//operator bool() {
	//	return bAva;
	//}
private:
	//�����ַ���
	bool SetCharacter() {
		
#ifndef NDEBUG	
		auto x = mysql_set_character_set(mysql, "UTF8");
		std::string str = "�ַ�������:";
		if (x) {
			str += "ʧ��";
		}
		else {
			str += "�ɹ�";
		}
		CQ_addLog(ac, CQLOG_INFO, "Bot-SQL", str.c_str());
		return (x == 0 ? true : false);
#else
		return ((mysql_set_character_set(mysql, "UTF8") == 0 )? true : false);
#endif
		
	}
	//���ò�ѯ���
	//void SetQuery(std::string i_query) {
	//	query = i_query;
	//}
	//ִ�е�ǰ��ѯ���
	//void ExecQuery() {
	//	//
	//	ClearResult();
	//	//
	//	IfBreakReconnect();
	//	//query
	//	mysql_real_query(mysql, query.c_str(), query.length());
	//	//save result
	//	result = mysql_store_result(mysql);
	//}
	//ִ�в�����ѯ���
	unsigned long ExecQuery(const std::string& i_query, MYSQL_RES* &o_result) {
		bFree = false;
		//
		//SetFree();
		//���������
		if (!IfBreakReconnect()) {
			//����ʧ��
			CQ_addLog(ac, CQLOG_ERROR, "Bot-SQL", "SQL ����ʧ��!");
			o_result = nullptr;
			return CON_FAILED;
		}
		//query
		mysql_real_query(mysql, i_query.c_str(), i_query.length());
		//save result
		o_result = mysql_store_result(mysql);
		//������Ӱ�������
		return static_cast<unsigned long>(mysql_affected_rows(mysql));
	}
	//"����"�汾,����Ҫ����Ĳ�ѯ���
	unsigned long ExecQueryNoRes(const std::string& i_query) {
		bFree = false;
		//
		//SetFree();
		//���������
		if (!IfBreakReconnect()) {
			//����ʧ��
			CQ_addLog(ac, CQLOG_ERROR, "Bot-SQL", "SQL ����ʧ��!");
			
			return CON_FAILED;
		}
		//query
		mysql_real_query(mysql, i_query.c_str(), i_query.length());
		//������Ӱ�������
		return static_cast<unsigned long>(mysql_affected_rows(mysql));
	}

	//���ش����ӵĿ������
	bool IsFree() {

		return bFree;
	}
	//���ܵ���������
	bool IfBreakReconnect() {
		//0��ping�ɹ�
		if (mysql_ping(mysql)) {
			//���������Ľ��
			//����0��ɹ�
			//pingʧ��
			CQ_addLog(ac, CQLOG_INFO, "Bot-SQL", "SQL pingʧ��!");
			//��Ҫ����MYSQL_OPT_RECONNECT,���򷵻ش���
			//�ƺ�����������,ֱ�ӳ�ʼ����
			//return ((mariadb_reconnect(mysql)!=0)?true:false);
			//���ȹر�mysql
			mysql_close(mysql);
			//Ȼ���ʼ��mysql
			mysql = mysql_init(NULL);
			//�������ݿ�����(û�취��ʹ��ȫ�ֱ���bgm)
			if (!mysql_real_connect(mysql, bgm.sql_url.c_str(), bgm.sql_user_name.c_str(),
				bgm.sql_password.c_str(), bgm.sql_db.c_str(), 0, "/tmp/mysql.sock", 0)) {
				//ʧ��
				return false;
			}
			else {
				//�ɹ�
				CQ_addLog(ac, CQLOG_INFO, "Bot-SQL", "SQL �����ɹ�!");
				//�����ַ���
				SetCharacter();
				return true;
			}
		}
		//�ɹ�ping
		return true;
		//
	}
	//ȡ���ϴ�query�Ľ��
	//MYSQL_RES* GetResult() {
	//	return result;
	//}
	//���result ���free
	void SetFree() {
		//delete current result
		//mysql_free_result(result);
		//set bFree
		bFree = true;
	}

	MYSQL* mysql;
	//std::string query;
	std::atomic<bool> bFree;
	//MYSQL_RES *result;
	//bool bAva;


};
//���ӳ�
class SQLPool
{
public:
	SQLPool() = default;

	void Init(BangumiBotVaribel* bgm) {
		this_bgm = bgm;

		max_connections = this_bgm->sqlpool_size;
		//����������
		pools.clear();
		pools.shrink_to_fit();
		//��������maxconnections�Ŀռ乩sql���ʹ��
		pools.reserve(max_connections);
		//��ʼ��
		for (unsigned int i = 0; i < max_connections; ++i)
			pools.push_back(std::make_shared<SQLConnection>(this_bgm->sql_url, this_bgm->sql_user_name,
				this_bgm->sql_password, this_bgm->sql_db));
	}

	//ִ�в�����ѯ���
	unsigned long ExecQuery(const std::string& i_query, MYSQL_RES* &o_result) {
		//
		std::shared_ptr<SQLConnection> current_con = SelectFreeCon();
		//�����Ч��
		//if (!(*current_con)) {
		//	return CON_FAILED;
		//}
		//
		auto affect_num = current_con->ExecQuery(i_query, o_result);
		//����ʹ�ÿ��� ��Ϊ������Ҳ����delete result,ͬʱʹ��free_result�ͷ�
		//current_result = std::make_shared<MYSQL_RES>(*(current_con->GetResult()));
		//Free Current_con
		current_con->SetFree();
		//������Ӱ�������
		return affect_num;
		//return current_result;

	}
	unsigned long ExecQuery(const std::string& i_query, BGMSQLResult&o_result) {
		//
		std::shared_ptr<SQLConnection> current_con = SelectFreeCon();
		//�����Ч��
		//if (!(*current_con)) {
		//	return CON_FAILED;
		//}
		//
		auto affect_num = current_con->ExecQuery(i_query, o_result());
		//����ʹ�ÿ��� ��Ϊ������Ҳ����delete result,ͬʱʹ��free_result�ͷ�
		//current_result = std::make_shared<MYSQL_RES>(*(current_con->GetResult()));
		//Free Current_con
		current_con->SetFree();
		//����BGMSQLResult�е�Rows
		o_result.RefreshRow();
		//������Ӱ�������
		return affect_num;
		//return current_result;

	}
	//���ذ汾,ִ�в�����ѯ���,����Ҫ����汾,����ʹ��,��Ҫ����������ĳɹ���
	unsigned long ExecQueryNoRes(const std::string& i_query) {
		//
		std::shared_ptr<SQLConnection> current_con = SelectFreeCon();
		//�����Ч��
		//if (!(*current_con)) {
		//	return CON_FAILED;
		//}
		//
		auto affect_num = current_con->ExecQueryNoRes(i_query);
		//����ʹ�ÿ��� ��Ϊ������Ҳ����delete result,ͬʱʹ��free_result�ͷ�
		//current_result = std::make_shared<MYSQL_RES>(*(current_con->GetResult()));
		//Free Current_con
		current_con->SetFree();
		//������Ӱ�������
		return affect_num;
		//return current_result;

	}
private:
	//һ�����ӳ��е�������
	unsigned int max_connections;
	//ֱ��ʹ��ȫ�ֿ��ܻ������ʱConnection�ĸ�������
	BangumiBotVaribel* this_bgm;
	//sql���ӳ�
	std::vector<std::shared_ptr<SQLConnection>> pools;
	//��ʱsql���ӳ�
	//std::vector<std::shared_ptr<SQLConnection>> temp_pools;
	//ѡ��ǰFree��Con
	std::shared_ptr<SQLConnection> SelectFreeCon() {
		for (unsigned i = 0; i < max_connections; ++i) {
			if (!pools[i]->IsFree())
			{
				continue;
			}
			if(pools[i])
				
			//�ҵ�
			return pools[i];
		}
		//û���ҵ�
		//��ʱ����һ��
		//�����������:����ȴ���һ��temp��shared_ptr��ʹ��push_back(std::move)
		//�ͻᵼ�¶���ʧ
		//�ɲο�:https://stackoverflow.com/questions/29643974/using-stdmove-with-stdshared-ptr
		//temp_pools.push_back(std::make_shared<SQLConnection>(this_bgm.sql_url, this_bgm.sql_user_name,
		//	this_bgm.sql_password, this_bgm.sql_db));
		CQ_addLog(ac, CQLOG_INFOSUCCESS, "Bot-SQL", "������ʱSQL����...");
		return std::make_shared<SQLConnection>(this_bgm->sql_url, this_bgm->sql_user_name,
			this_bgm->sql_password, this_bgm->sql_db);

	}
};

//���Ӱ����������׳��쳣,��Ҫ�ɵ�����catch����
inline void SQLCheckResult(unsigned long affect_rows_num) {
	//ͨ�����ص�Ӱ������������ж�sql�Ƿ���ȷִ����
	//����ʧ�ܷ���-2(CON_FAILED)
	//���û���򷵻�-1(QUERY_FAILED)
	//�����ȷִ����,��û��ʵ��Ч��,��������һ�������ڵ�row�򷵻�0
	//����"����0"
	if (affect_rows_num == CON_FAILED) {
		throw boost::system::system_error(bangumi_bot_errors::sql_connect_error);
	} else	if (affect_rows_num == QUERY_FAILED) {
		throw boost::system::system_error(bangumi_bot_errors::sql_query_filed);
	}
}








#endif