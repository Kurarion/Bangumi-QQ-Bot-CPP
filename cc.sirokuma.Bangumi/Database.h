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
//封装了原本的c版本Res
class BGMSQLResult {
	friend class SQLPool;
public:
	//一般构造函数
	BGMSQLResult(MYSQL_RES* _res)
		:mysql_res(_res),
		rows(mysql_fetch_row(_res))
	{
		//rows = mysql_fetch_row(_res);
	}
	//默认构造函数
	BGMSQLResult()
		:mysql_res(NULL),
		rows(NULL)
	{
		//rows = mysql_fetch_row(_res);
	}
	//析构函数会释放res
	~BGMSQLResult()
	{
		mysql_free_result(mysql_res);
	}
	//读取Rows数据
	const char* operator[] (int i) {

		//重载[]运算符
		return *(rows + i);
	}
	//读取第N行,注意这是不可逆操作
	//从0开始计数,参数表示向下推几行
	void FetchRow(int num) {
		for (int i = num; i > 0; --i)
			RefreshRow();
	}

private:
	//在mysql_res被赋值后更新rows
	void RefreshRow() {
		rows = mysql_fetch_row(mysql_res);
	}

	MYSQL_RES*& operator() () {
		//重载(void)
		return mysql_res;
	}
private:
	MYSQL_RES* mysql_res;
	MYSQL_ROW rows;
};
#define NULL_RES 

//一个SQL连接
class SQLConnection {
	friend class SQLPool;
public:
	SQLConnection(const std::string& address, const std::string& user_name,
		const std::string& user_password, const std::string& db_name)
		:bFree(true)
	{
		//初始化mysql
		mysql = mysql_init(NULL);
		//进行数据库连接
		if (!mysql_real_connect(mysql, address.c_str(), user_name.c_str(), user_password.c_str(),
			db_name.c_str(), 0, "/tmp/mysql.sock", 0))

#ifndef NDEBUG
			//提示错误信息
			show_error(mysql);
#else
		{
			//bAva = false;
			CQ_addLog(ac, CQLOG_ERROR, "Bot-SQL", "数据库连接失败!");
		}

#endif
		else {
			//bAva = true;
			//设置字符集
			SetCharacter();
			CQ_addLog(ac, CQLOG_INFOSUCCESS, "Bot-SQL", "数据库连接成功!");
		}
	}
	~SQLConnection() {
		bFree = false;
		//delete mysql
		mysql_close(mysql);
		//delete result
		//mysql_free_result(result);
	}
	//隐式转换bool
	//operator bool() {
	//	return bAva;
	//}
private:
	//设置字符集
	bool SetCharacter() {
		
#ifndef NDEBUG	
		auto x = mysql_set_character_set(mysql, "UTF8");
		std::string str = "字符集设置:";
		if (x) {
			str += "失败";
		}
		else {
			str += "成功";
		}
		CQ_addLog(ac, CQLOG_INFO, "Bot-SQL", str.c_str());
		return (x == 0 ? true : false);
#else
		return ((mysql_set_character_set(mysql, "UTF8") == 0 )? true : false);
#endif
		
	}
	//设置查询语句
	//void SetQuery(std::string i_query) {
	//	query = i_query;
	//}
	//执行当前查询语句
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
	//执行参数查询语句
	unsigned long ExecQuery(const std::string& i_query, MYSQL_RES* &o_result) {
		bFree = false;
		//
		//SetFree();
		//检测连接性
		if (!IfBreakReconnect()) {
			//连接失败
			CQ_addLog(ac, CQLOG_ERROR, "Bot-SQL", "SQL 重连失败!");
			o_result = nullptr;
			return CON_FAILED;
		}
		//query
		mysql_real_query(mysql, i_query.c_str(), i_query.length());
		//save result
		o_result = mysql_store_result(mysql);
		//返回受影响的行数
		return static_cast<unsigned long>(mysql_affected_rows(mysql));
	}
	//"重载"版本,不需要结果的查询语句
	unsigned long ExecQueryNoRes(const std::string& i_query) {
		bFree = false;
		//
		//SetFree();
		//检测连接性
		if (!IfBreakReconnect()) {
			//连接失败
			CQ_addLog(ac, CQLOG_ERROR, "Bot-SQL", "SQL 重连失败!");
			
			return CON_FAILED;
		}
		//query
		mysql_real_query(mysql, i_query.c_str(), i_query.length());
		//返回受影响的行数
		return static_cast<unsigned long>(mysql_affected_rows(mysql));
	}

	//返回此连接的空闲情况
	bool IsFree() {

		return bFree;
	}
	//可能的重新连接
	bool IfBreakReconnect() {
		//0是ping成功
		if (mysql_ping(mysql)) {
			//返回重连的结果
			//返回0表成功
			//ping失败
			CQ_addLog(ac, CQLOG_INFO, "Bot-SQL", "SQL ping失败!");
			//需要定义MYSQL_OPT_RECONNECT,否则返回错误
			//似乎重连有问题,直接初始连接
			//return ((mariadb_reconnect(mysql)!=0)?true:false);
			//首先关闭mysql
			mysql_close(mysql);
			//然后初始化mysql
			mysql = mysql_init(NULL);
			//进行数据库连接(没办法地使用全局变量bgm)
			if (!mysql_real_connect(mysql, bgm.sql_url.c_str(), bgm.sql_user_name.c_str(),
				bgm.sql_password.c_str(), bgm.sql_db.c_str(), 0, "/tmp/mysql.sock", 0)) {
				//失败
				return false;
			}
			else {
				//成功
				CQ_addLog(ac, CQLOG_INFO, "Bot-SQL", "SQL 重连成功!");
				//设置字符集
				SetCharacter();
				return true;
			}
		}
		//成功ping
		return true;
		//
	}
	//取得上次query的结果
	//MYSQL_RES* GetResult() {
	//	return result;
	//}
	//清空result 标记free
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
//连接池
class SQLPool
{
public:
	SQLPool() = default;

	void Init(BangumiBotVaribel* bgm) {
		this_bgm = bgm;

		max_connections = this_bgm->sqlpool_size;
		//首先先清理
		pools.clear();
		pools.shrink_to_fit();
		//分配至少maxconnections的空间供sql语句使用
		pools.reserve(max_connections);
		//初始化
		for (unsigned int i = 0; i < max_connections; ++i)
			pools.push_back(std::make_shared<SQLConnection>(this_bgm->sql_url, this_bgm->sql_user_name,
				this_bgm->sql_password, this_bgm->sql_db));
	}

	//执行参数查询语句
	unsigned long ExecQuery(const std::string& i_query, MYSQL_RES* &o_result) {
		//
		std::shared_ptr<SQLConnection> current_con = SelectFreeCon();
		//检测有效性
		//if (!(*current_con)) {
		//	return CON_FAILED;
		//}
		//
		auto affect_num = current_con->ExecQuery(i_query, o_result);
		//这里使用拷贝 因为在类中也可能delete result,同时使用free_result释放
		//current_result = std::make_shared<MYSQL_RES>(*(current_con->GetResult()));
		//Free Current_con
		current_con->SetFree();
		//返回受影响的行数
		return affect_num;
		//return current_result;

	}
	unsigned long ExecQuery(const std::string& i_query, BGMSQLResult&o_result) {
		//
		std::shared_ptr<SQLConnection> current_con = SelectFreeCon();
		//检测有效性
		//if (!(*current_con)) {
		//	return CON_FAILED;
		//}
		//
		auto affect_num = current_con->ExecQuery(i_query, o_result());
		//这里使用拷贝 因为在类中也可能delete result,同时使用free_result释放
		//current_result = std::make_shared<MYSQL_RES>(*(current_con->GetResult()));
		//Free Current_con
		current_con->SetFree();
		//更新BGMSQLResult中的Rows
		o_result.RefreshRow();
		//返回受影响的行数
		return affect_num;
		//return current_result;

	}
	//重载版本,执行参数查询语句,不需要结果版本,很少使用,需要结果检查操作的成功性
	unsigned long ExecQueryNoRes(const std::string& i_query) {
		//
		std::shared_ptr<SQLConnection> current_con = SelectFreeCon();
		//检测有效性
		//if (!(*current_con)) {
		//	return CON_FAILED;
		//}
		//
		auto affect_num = current_con->ExecQueryNoRes(i_query);
		//这里使用拷贝 因为在类中也可能delete result,同时使用free_result释放
		//current_result = std::make_shared<MYSQL_RES>(*(current_con->GetResult()));
		//Free Current_con
		current_con->SetFree();
		//返回受影响的行数
		return affect_num;
		//return current_result;

	}
private:
	//一个连接池中的连接数
	unsigned int max_connections;
	//直接使用全局可能会出现临时Connection的各种问题
	BangumiBotVaribel* this_bgm;
	//sql连接池
	std::vector<std::shared_ptr<SQLConnection>> pools;
	//临时sql连接池
	//std::vector<std::shared_ptr<SQLConnection>> temp_pools;
	//选择当前Free的Con
	std::shared_ptr<SQLConnection> SelectFreeCon() {
		for (unsigned i = 0; i < max_connections; ++i) {
			if (!pools[i]->IsFree())
			{
				continue;
			}
			if(pools[i])
				
			//找到
			return pools[i];
		}
		//没有找到
		//临时申请一个
		//这里很有问题:如果先创建一个temp的shared_ptr再使用push_back(std::move)
		//就会导致对象丢失
		//可参考:https://stackoverflow.com/questions/29643974/using-stdmove-with-stdshared-ptr
		//temp_pools.push_back(std::make_shared<SQLConnection>(this_bgm.sql_url, this_bgm.sql_user_name,
		//	this_bgm.sql_password, this_bgm.sql_db));
		CQ_addLog(ac, CQLOG_INFOSUCCESS, "Bot-SQL", "申请临时SQL连接...");
		return std::make_shared<SQLConnection>(this_bgm->sql_url, this_bgm->sql_user_name,
			this_bgm->sql_password, this_bgm->sql_db);

	}
};

//检查影响的行数并抛出异常,需要由调用者catch处理
inline void SQLCheckResult(unsigned long affect_rows_num) {
	//通过返回的影响的行数可以判断sql是否正确执行了
	//连接失败返回-2(CON_FAILED)
	//如果没有则返回-1(QUERY_FAILED)
	//如果正确执行了,但没有实际效果,比如搜索一个不存在的row则返回0
	//否则"大于0"
	if (affect_rows_num == CON_FAILED) {
		throw boost::system::system_error(bangumi_bot_errors::sql_connect_error);
	} else	if (affect_rows_num == QUERY_FAILED) {
		throw boost::system::system_error(bangumi_bot_errors::sql_query_filed);
	}
}








#endif