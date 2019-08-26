#include "Functions.h"
//需要本地的时间
//#include "boost/date_time/posix_time/posix_time.hpp"
#ifndef BANGUMI_PARSER_H
#define BANGUMI_PARSER_H
//模板函数的显式声明/暂时不用,当前只有在本文件中使用过模板函数
//extern template void bangumi::BGMCode::Match(const std::unordered_set<char>&, std::set<size_t>&);
//extern template void bangumi::BGMCode::Match(const std::unordered_set<string>&, std::map<size_t, std::string>&);

//const map<>
inline std::pair<bool,bool> VerifyMsg(const char &first, const char &second, const char *&origin) {
	char temp[3];
	temp[0] = first;
	temp[1] = second;
	temp[2] = '\0';
#ifndef NDEBUG
	CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot", temp);
	CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot", std::to_string((int)temp[1]).c_str());
#endif
	//std::string test_str(temp);
	//小于0说明不是单字节的
	if (temp[0] < 0) {
		//识别特殊双字符前缀
		for (const auto& i : pre_code_refresh) {
#ifndef NDEBUG
			CQ_addLog(ac, CQLOG_DEBUG, "Pre_code", i);
#endif

			if (temp[0] == i[0] && temp[1] == i[1]) {
				//首先设置msg向前移动一位
				//为了后面code识别不出现问题
				++origin;
				return{ true,true };
			}
		}

		for (const auto& i : pre_code_muli) {
#ifndef NDEBUG
			CQ_addLog(ac, CQLOG_DEBUG, "Pre_code", i);
#endif

			if (temp[0] == i[0] && temp[1] == i[1]) {
				//首先设置msg向前移动一位
				//为了后面code识别不出现问题
				++origin;
				return{ true,false };
			}
		}

	}
	//大于0说明是单字节的处理
	else {
		//识别特殊双或单字符前缀
		for (const auto& i : pre_code_refresh) {
#ifndef NDEBUG
			CQ_addLog(ac, CQLOG_DEBUG, "Pre_code", i);
#endif
			if (i[1] != '\0') {
				if (temp[0] == i[0] && temp[1] == i[1]) {
					//首先设置msg向前移动一位
					//为了后面code识别不出现问题
					++origin;
					return{ true,true };
				}
			}
			else {
				if (temp[0] == i[0]) {
					//首先设置msg向前移动一位
					//为了后面code识别不出现问题
					//++origin;
					return{ true,true };
				}
			}

		}

		//识别单字符的前缀
		for (const auto& i : pre_code_single) {
#ifndef NDEBUG
			CQ_addLog(ac, CQLOG_DEBUG, "Pre_code", i);
#endif
			if (temp[0] == i[0])
				return{ true,false };
		}
	}
	return {false,false};
	//if (test_str == ":" || test_str == "：" || test_str == "\\" || test_str == "=")
	//	return true;
	//else
	//	return false;
}
//分析原msg中的计时,并返回countdown
inline std::pair<unsigned,std::string> AnalyseTimeWork(const char *msg) {
	std::string temp = msg;
	//去除所有回车
	boost::erase_all(temp, "\r\n");
	//当前使用_作为计时的符号
	size_t t_pos = temp. find_last_of('_');
	unsigned countdown = 0;
	if (t_pos != std::string::npos) {
		//如果有这个符号
		try {
			countdown = std::stoi(temp.substr(t_pos + 1));
			//此时应当排除原msg的&之后的所有消息
			return{ countdown, temp.substr(0,t_pos) };

		}
		catch (std::invalid_argument&) {
			//结果可能是字符串的一部分
			//不识别
		}
	}
	//最后返回countdown
	return {countdown,temp};
}
//向数据库中更新指令使用情况
void UpdateCodeType(std::set<BgmCode>&pool,int64_t qq) {
	if (pool.empty()){
		//如果池为空，直接返回
		return;
	}
	//当前时间
	boost::posix_time::ptime currentTime = boost::posix_time::second_clock::universal_time() + boost::posix_time::hours(8);
	boost::gregorian::date current_date = currentTime.date();
	std::string current_date_str(boost::gregorian::to_iso_extended_string(current_date));
	//最后加上更新日期,使用东八区的时间

	std::string curr_time = boost::posix_time::to_iso_extended_string(currentTime);
	curr_time[curr_time.find_first_of('T')] = ' ';
	//查询语句
	bangumi::string query;
	query << "INSERT INTO bgm_users(user_id, user_qq, user_bangumi, user_access_token, user_refresh_token, dmhy_lastpubDate) "
		<< "VALUE(NULL, " << qq << ", 0, '', '', '"
		<< curr_time
		<< "') ON DUPLICATE KEY UPDATE ";
//便捷宏
#define IF_QUERY(name)\
name<<"=IF(BgmCode_Last_Date<'"<<current_date_str<<"',0,"<<name<<"),"

	query << IF_QUERY("TBgmCode_subject")
		<< IF_QUERY("TBgmCode_search")
		<< IF_QUERY("TBgmCode_collect")
		<< IF_QUERY("TBgmCode_user")
		<< IF_QUERY("TBgmCode_up")
		<< IF_QUERY("TBgmCode_reg")
		<< IF_QUERY("TBgmCode_help")
		<< IF_QUERY("TBgmCode_tag")
		<< IF_QUERY("TBgmCode_statis")
		<< IF_QUERY("TBgmCode_unknow");
	for (auto& type:pool)
	{
		switch (type)
		{
		case BgmCode::Subject:
			query << "BgmCode_subject = BgmCode_subject+1,";
			query << "TBgmCode_subject = TBgmCode_subject+1,";
			break;
		case BgmCode::Search:
			query << "BgmCode_search = BgmCode_search+1,";
			query << "TBgmCode_search = TBgmCode_search+1,";
			break;
		case BgmCode::User:
			query << "BgmCode_user = BgmCode_user+1,";
			query << "TBgmCode_user = TBgmCode_user+1,";
			break;
		//case BgmCode::List:
		//	break;
		//case BgmCode::BGM:
		//	break;
		//case BgmCode::DMHY:
		//	break;
		//case BgmCode::MOE:
		//	break;
		case BgmCode::Up:
			query << "BgmCode_up = BgmCode_up+1,";
			query << "TBgmCode_up = TBgmCode_up+1,";
			break;
		case BgmCode::Collect:
			query << "BgmCode_collect = BgmCode_collect+1,";
			query << "TBgmCode_collect = TBgmCode_collect+1,";
			break;
		//case BgmCode::Conf:
		//	break;
		case BgmCode::Reg:
			query << "BgmCode_reg = BgmCode_reg+1,";
			query << "TBgmCode_reg = TBgmCode_reg+1,";
			break;
		case BgmCode::Help:
			query << "BgmCode_help = BgmCode_help+1,";
			query << "TBgmCode_help = TBgmCode_help+1,";
			break;
		case BgmCode::Tag:
			query << "BgmCode_tag = BgmCode_tag+1,";
			query << "TBgmCode_tag = TBgmCode_tag+1,";
			break;
		case BgmCode::Statis:
			query << "BgmCode_statis = BgmCode_statis+1,";
			query << "TBgmCode_statis = TBgmCode_statis+1,";
			break;
		case BgmCode::Unknow:
			query << "BgmCode_unknow = BgmCode_unknow+1,";
			query << "TBgmCode_unknow = TBgmCode_unknow+1,";
			break;
		default:
			break;
		}
	}


	query << "BgmCode_Last_Date = '"<< curr_time <<"'";
	//最后执行SQL查询
	auto affect_rows_num = sql_pool.ExecQueryNoRes(query);
	try
	{
		SQLCheckResult(affect_rows_num);
	}
	catch (boost::system::system_error& e)
	{

		{
			bangumi::string debug_msg;
			debug_msg << "更新指令使用情况失败:"
				<< std::to_string(affect_rows_num)
				>> e.what();

			CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-UpdateCodeType", debug_msg);
		}

	}

	
	
}
inline void Parsing(int32_t subType, int32_t msgId, int64_t fromQQ, const char *msg, bool refresh) {
	//本次处理的命令池list
	std::list<bangumi::Code> code_pool;
	//本次命令的标识符set
	std::set<BgmCode> code_type;
	//计时检测[目前仅仅应用于私聊]
	auto time_result = AnalyseTimeWork(msg);
	//本次命令的参数对象
	bangumi::BGMCodeParam code_param(bangumi::BGMCodeExtraVar{time_result.first,refresh}, time_result.second.c_str(), code_pool, code_type, BgmRetType::Private, fromQQ);
	//构造BGMCode处理命令
	bangumi::BGMCode input(code_param);
#ifndef NDEBUG
	CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot", input.c_str());
#endif
	//调用BGMCode分析命令,将结果存入code_pool
	input.AnalyseCode();
	//对每一个Code进行执行返回bangumi::string
	for (auto& i : code_pool) {
		i.ExecuteCode();
//#ifndef NDEBUG
	CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Instructs", i.Print());

//#endif

	}

	//处理code_type,记录各种命令的使用次数
	UpdateCodeType(code_type,fromQQ);

}
inline void ParsingM(int32_t subType, int32_t msgId, int64_t fromDiscussGroup, BgmRetType DiscussOrGroup, int64_t fromQQ, const char *msg, bool refresh) {
	//本次处理的命令池list
	std::list<bangumi::Code> code_pool;
	//本次命令的标识符set
	std::set<BgmCode> code_type;
	//计时检测
	auto time_result = AnalyseTimeWork(msg);
	//本次命令的参数对象
	bangumi::BGMCodeParam code_param(bangumi::BGMCodeExtraVar{ time_result.first,refresh }, time_result.second.c_str(), code_pool, code_type, DiscussOrGroup, fromQQ, fromDiscussGroup);
	//构造BGMCode处理命令
	bangumi::BGMCode input(code_param);
#ifndef NDEBUG
	CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot", input.c_str());
#endif
	//调用BGMCode分析命令,将结果存入code_pool
	input.AnalyseCode();
	//对每一个Code进行执行返回bangumi::string
	for (auto& i : code_pool) {
		i.ExecuteCode();
//#ifndef NDEBUG
		CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Instructs", i.Print());

//#endif

	}

	//处理code_type,记录各种命令的使用次数
	UpdateCodeType(code_type, fromQQ);

}



//仅仅用于AT函数中
#define AT_SEND_MSG(msg)\
{\
int64_t to_someone;\
switch (retType)\
{\
case BgmRetType::Private:\
	to_someone = fromQQ;\
	break;\
case BgmRetType::Group:\
case BgmRetType::Discuss:\
	to_someone = fromDiscussGroup;\
	break;\
default:\
	break;\
}\
SendMsg.at(retType)(ac, to_someone, msg);\
}
//更新用户权限值函数
inline bool Update_Right(int64_t fromQQ, int right) {
	//授权
	bangumi::string open_query;
	open_query << "UPDATE bgm_users SET "
		<< "dmhy_open=" << right
		<< " WHERE user_qq=" << fromQQ;
	//影响的行数
	unsigned long affect_rows_num = sql_pool.ExecQueryNoRes(open_query);
	try
	{
		SQLCheckResult(affect_rows_num);
		return true;
	}
	catch (boost::system::system_error&)
	{
		//未知情况
		return false;
	}
}

inline bool ParsingAt(std::string &msg, int64_t fromDiscussGroup, int64_t fromQQ, BgmRetType retType) {
	//
	size_t pos = msg.find(at_me_cq);
	if (pos!=std::string::npos)
	{
		msg.erase(pos, at_me_cq.length());
	}
	else if (pos = msg.find(at_me_1), pos != std::string::npos) {
		msg.erase(pos, at_me_1.length());
	}
	else if (pos = msg.find(at_me_2), pos != std::string::npos) {
		msg.erase(pos, at_me_2.length());
	}
	else {
		//忽视此条处理
		return false;
	}

	//去除可能的空格前缀
	size_t site = msg.find_first_not_of(' ');
	if (site!=std::string::npos)
	{
		msg = msg.substr(site);
	}	

	const char * user_answer = msg.c_str();
	//进行前缀识别
	auto verify = VerifyMsg(user_answer[0], user_answer[1], user_answer);
	//本次处理的命令池list
	std::list<bangumi::Code> code_pool;
	if (verify.first)//验证过滤
	{
		//本次命令的标识符set
		std::set<BgmCode> code_type;
		//计时检测[目前仅仅应用于私聊]
		auto time_result = AnalyseTimeWork(user_answer);
		//本次命令的参数对象
		if (retType == BgmRetType::Private) {
			bangumi::BGMCodeParam code_param(bangumi::BGMCodeExtraVar{ time_result.first,verify.second }, time_result.second.c_str(), code_pool, code_type, BgmRetType::Private, fromQQ);
			bangumi::BGMCode input(code_param);
			//调用BGMCode分析命令,将结果存入code_pool
			input.AnalyseCode();
		}
		else {
			bangumi::BGMCodeParam code_param(bangumi::BGMCodeExtraVar{ 0,verify.second }, user_answer, code_pool, code_type, retType, fromQQ, fromDiscussGroup);
			bangumi::BGMCode input(code_param);
			//调用BGMCode分析命令,将结果存入code_pool
			input.AnalyseCode();
		}
		//此时已经得到了用户的所有code


	}
	else {
		//指令前缀识别失败

		//在此处理结束
		//return true;
	}

	//完成了用户的语法检测后
	//======
	//Get用户的权限
	bangumi::string query;
	query << "SELECT dmhy_open "
		<< "FROM bgm_users "
		<< "WHERE user_qq="
		<< fromQQ;
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
		//忽视
		return false;
	}

	//便捷宏
#define MARU()\
bangumi::string ret;\
ret<<"[CQ:image,file="<<bgm.maru_answer<<"]\n"\
	"[CQ:at,qq=" << fromQQ << "] "\
	<<right_answer;

#define BATSU()\
bangumi::string ret;\
ret<<"[CQ:image,file="<<bgm.batsu_answer<<"]\n"\
	"[CQ:at,qq=" << fromQQ << "] "\
	<<error_answer;

	//前的处理宏
#define Handle_top(curr_right_num)\
if (has_updated) {\
	MARU();\
	ret << question[curr_right_num];\
	AT_SEND_MSG(ret);\
	return true;\
}\
else {\
bool is_right_answer = false;


	//后的处理宏
#define Handle_down(curr_right_num)\
if (is_right_answer) {\
	has_updated = true;\
	Update_Right(fromQQ, curr_right_num+1);\
}\
else {\
	BATSU();\
	ret << question[curr_right_num];\
	AT_SEND_MSG(ret);\
	return true;\
}\
}
	//后的处理宏
#define EXHandle_down(curr_right_num, to_return_num)\
if (is_right_answer) {\
	has_updated = true;\
	Update_Right(fromQQ, curr_right_num+1);\
}\
else {\
	BATSU();\
	ret << question[to_return_num];\
	AT_SEND_MSG(ret);\
	return true;\
}\
}
	//进行当前的状态检查
	bool has_updated = false;
	//
	if (affect_rows_num > 0) {
		//检查权限
		switch (std::stoi(result[0]))
		{
		case 0:
		{
			//验证是否正确回复
			bool is_right_answer = false;
			//--问题答案的验证--
			is_right_answer = msg.find("OK") != std::string::npos;
			//--
			if (is_right_answer) {
				has_updated = true;
				Update_Right(fromQQ, 1);
			}
			else {
				//发送问题
				AT_SEND_MSG(question[0]);
				return true;
			}

		}
		case 1: 
		{
			if (has_updated) {
				//是从上一个阶段过来的
				//发送问题
				bangumi::string ret;
				ret << "[CQ:image,file=" << bgm.goodjob_pic << "]\n";
				ret << "[CQ:at,qq=" << fromQQ << "] ";
				ret << "OK~\n首先第一个问题：\n";
				ret << question[1]; 
				AT_SEND_MSG(ret); 
				return true;
			}
			else {
				//用户已经收到过了问题
				//处理用户的回答
				//验证是否正确回复
				bool is_right_answer = false;
				//--问题答案的验证--
				//问题1：help
				for (auto&code : code_pool) {
					if (code.GetType() == BgmCode::Help) {
						is_right_answer = true;
						break;
					}
				}
				//--
				if (is_right_answer) {
					has_updated = true;
					Update_Right(fromQQ, 2);
				}
				else {
					//用户回答错误
					BATSU();
					ret << question[1]; 
					AT_SEND_MSG(ret); 
					return true;
				}
			}

		}
		case 2:
		{
			Handle_top(2);
			//--问题答案的验证--
			//问题2：
			for (auto&code : code_pool) {
				if (code.GetType() == BgmCode::Tag) {
					//进行用户的指令解析
					for (auto& param : code.GetSTRParam()) {
						if (bangumi::ResolveTagPara(param) == standard_answer[2]) {
							is_right_answer = true;
							break;
						}
					}
				}
				if(is_right_answer)
					break;
			}
			//--
			Handle_down(2);
		}
		case 3:
		{
			Handle_top(3);
			//--问题答案的验证--
			bool stage1 = false;
			bool stage2 = false;
			bool stage3 = false;
			//首先检查强制更新
			if (verify.second)
			{
				for (auto&code : code_pool) {
					if (code.GetType() == BgmCode::User) {
						//进行用户的指令解析
						for (auto& param : code.GetSTRParam()) {
							if (param == "1")
								stage2 = true;
							else if (param == "92981")
								stage3 = true;
						}
					}
					else if (code.GetType() == BgmCode::Statis) {
						for (auto& param : code.GetSTRParam()) {
							if (param.find("me") != std::string::npos) {
								stage1 = true;
							}
						}
					}
				}
				//结果检测
				if (stage1&&stage2&&stage3)
				{
					is_right_answer = true;
				}
			}
			//--
			Handle_down(3);
		}
		case 4:
		{
			Handle_top(4);
			//--问题答案的验证--
			//问题4：
			if (!code_pool.empty())
			{
				auto& code = code_pool.front();
				if (code.GetType() == BgmCode::Subject)
				{
					//进行用户的指令解析
					for (auto& param : code.GetSTRParam()) {
						if (bangumi::ResolveSubjectPara(param) == standard_answer[4]) {
							is_right_answer = true;
							break;
						}
					}
				}
			}
			//--
			Handle_down(4);
		}
		case 5:
		{
			Handle_top(5);
			//--问题答案的验证--
			//问题5：
			if (!code_pool.empty())
			{
				auto& code = code_pool.front();
				if (code.GetType() == BgmCode::Search)
				{
					//进行用户的指令解析
					for (auto& param : code.GetSTRParam()) {
						if (bangumi::ResolveSearchPara(param) == standard_answer[5]) {
							is_right_answer = true;
							break;
						}
					}
				}
			}
			//--
			Handle_down(5);
		}
		case 6:
		{
			Handle_top(6);
			//--问题答案的验证--
			//问题6：
			if (!code_pool.empty())
			{
				auto& code = code_pool.front();
				if (code.GetType() == BgmCode::Up)
				{
					//进行用户的指令解析
					for (auto& param : code.GetSTRParam()) {
						if (bangumi::ResolveUpdatePara(param) == standard_answer[6]) {
							is_right_answer = true;
							break;
						}
					}
				}
			}
			//--
			Handle_down(6);
		}
		case 7:
		{
			Handle_top(7);
			//--问题答案的验证--
			//问题7：
			if (!code_pool.empty())
			{
				auto& code = code_pool.front();
				if (code.GetType() == BgmCode::Up)
				{
					//进行用户的指令解析
					for (auto& param : code.GetSTRParam()) {
						//计算标准答案
						standard_answer[7].id = bangumi::PARA_Resolve_Sharp({35,65,110,103,101,108,43,66,101,97,116,115,45,50}, BgmCode::Up).id;
						//验证
						if (bangumi::ResolveUpdatePara(param) == standard_answer[7]) {
							is_right_answer = true;
							break;
						}
					}
				}
			}
			//--
			Handle_down(7);
		}
		case 8:
		{
			Handle_top(8);
			//--问题答案的验证--
			//问题8：
			if (!code_pool.empty())
			{
				auto& code = code_pool.front();
				if (code.GetType() == BgmCode::Collect)
				{
					//进行用户的指令解析
					for (auto& param : code.GetSTRParam()) {
						//计算标准答案
						standard_answer[8].id = bangumi::PARA_Resolve_Percent({ 37,114,101,119,114,105,116,101,42,103,42,50 }, BgmCode::Collect).id;
						//验证
						if (bangumi::ResolveCollectPara(param) == standard_answer[8]) {
							is_right_answer = true;
							break;
						}
					}
				}
			}
			//--
			EXHandle_down(998,8);
		}
		case 999:
		{
			if (has_updated) {
				//是从上一个阶段过来的
				bangumi::string ret;
				ret << "[CQ:image,file=" << bgm.goodjob_pic << "]\n";
				ret << "[CQ:at,qq=" << fromQQ << "] ";
				ret << test_passed;
				AT_SEND_MSG(ret);
				return true;
			}
			//发送回复
			bangumi::string ret;
			ret << "[CQ:image,file=" << bgm.goodjob_pic << "]\n"
				<< "[CQ:at,qq=" << fromQQ << "] "
				<< "您已通过了考核！\nEnjoy it~";
			AT_SEND_MSG(ret);
			return true;
		}
		default:
		{
			//忽视
			return false;
		}
		}
	}
	else
	{
		//忽视
		return false;
	}
	//======
}
struct PicResult
{
	std::string romaji;
	std::string title;
	std::string syoukai;
	std::string eps_num;
	std::string eptitle;
	std::string progress;

	bangumi::string toString() {
		bangumi::string res;
		res << "<匹配结果>"// << romaji << "(罗马音)"
			>> "原名:  " << title
			>> syoukai
			>> "匹配话数:  第 " << eps_num << "话"
			>> "此话标题:  " << eptitle
			>> "所在时刻:  " << progress;
		return std::move(res);
	}
};
//识图相关
inline bool ParsingPic(int32_t subType, int32_t msgId, std::string &msg, int64_t fromDiscussGroup, int64_t fromQQ, BgmRetType retType) {
	//进行响应的前缀
	size_t pos = msg.find(at_me_cq);
	if (pos != std::string::npos)
	{
		//msg.erase(pos, at_me_cq.length());
	}
	else if (pos = msg.find(pic_find_2), pos != std::string::npos) {
		//msg.erase(pos, at_me_1.length());
	}
	else if (pos = msg.find(pic_find_1), pos != std::string::npos) {
		//msg.erase(pos, at_me_2.length());
	}
	else {
		//忽视此条处理
		return false;
	}
	bool db_default = true;
	if (msg[0] == 'x')
	{
		//
		db_default = false;
	}
	//进行接收图片的识别
	size_t pic_pos = msg.find(pre_get_image);
	if (pic_pos == std::string::npos)
	{
		return false;
	}
	//[CQ:image,file=1.jpg]
	std::string recv_pic_name =msg.substr(pic_pos+pre_get_image.length(), msg.find(']', pic_pos) - pic_pos - pre_get_image.length());
	//cqimg文件读取
	ptree ipt;
	read_ini(bgm.Bangumi_Img_Dir + recv_pic_name + ".cqimg", ipt);
	//获取图片的url
	std::string image_url = ipt.get<std::string>("image.url");
	ipt.clear();
//	std::string image_url = "https://c2cpicdw.qpic.cn/offpic_new/597320012//8808f6f9-80c1-448e-847b-b18218586526/0?vuin=272242684&term=2";
	//请求Nao
	//saucenao.com/search.php?db=999&api_key=????&dbmaski=32768&numres=1&url=https://iqdb.org/thu/thu_5318b582.jpg
	bangumi::string uri("/search.php?numres=1&db=");
	if (db_default)
	{
		uri << "21";
	}
	else
	{
		uri << "22";
	}
	
	
	uri << "&api_key=" << bgm.nao_api;
	uri << "&url=" << image_url;
	std::string request = "GET " + uri + " HTTP/1.1\r\n"
		"Host: " "saucenao.com" "\r\n" "\r\n";
	//https请求
	std::string page_result;
	try {
		//std::string html = boost::locale::conv::from_utf(http_client.SyncBGMHTTPRequest(request), "GBK");
		page_result = code_converter.Conv(http_client.SyncHTTPSRequest("saucenao.com", request));

	}
	catch (boost::system::system_error&) {
		AT_SEND_MSG("图片匹配失败...");
		return true;
	}
	//对结果进行处理
	//=====
	size_t tag_start_pos = page_result.find("<div class=\"resulttitle\">");
	if (tag_start_pos == std::string::npos)
	{
		//说明没有匹配
		//直接返回
		AT_SEND_MSG("暂无匹配结果...");
		return true;
	}
	size_t tag_end_pos = page_result.find("</span><br /></div></div></td></tr></table>",tag_start_pos);
	//要处理的结果字符串
	std::string result = page_result.substr(tag_start_pos + 25, tag_end_pos - tag_start_pos - 25);
	//循环处理每一个字符 
	bool is_in = false;
	bangumi::string output_message;
	for (int c = 0; c < result.length(); ++c) {
		if (result[c] == '<')
		{
			is_in = true;
			//尝试判断是否为<br标签
			if (result[c+1] == 'b')
			{
				++c;
				output_message << '\n';
			}
			continue;
		}
		if (result[c] == '>')
		{
			is_in = false;
			continue;
		}
		if (!is_in)
		{
			output_message << result[c];
		}
	}
	//---
	//对原文本进行后续处理
	size_t title_pos = output_message.find("Title: ");
	if (title_pos != std::string::npos)
	{
		if (output_message[title_pos - 1] == 'P')
		{
			//说明是JPTitle
			output_message[title_pos - 2] = ' ';
			output_message[title_pos - 1] = ' ';
			output_message[title_pos] = '\n';

		}
		else {
			//说明是Title
			output_message[title_pos] = '\n';
		}
	}
	//生成一个结构体
	PicResult subject;
	//std::istringstream input(output_message);
	try {
		std::string temp;
		size_t first_return = output_message.find_first_of('\n');
		size_t first_gang = output_message.find_last_of('-', first_return - 1);
		if (first_gang!=std::string::npos)
		{
			subject.eps_num += output_message[first_gang+2];
			int i = 1;
			while (output_message[first_gang + 2 + i]!='\n')
			{
				subject.eps_num += output_message[first_gang + 2 + i];
				++i;
			}
		}

		
		//之后查找三个e: 
		size_t pos = output_message.find("e: ", title_pos)+3;
		size_t end = output_message.find('\n', pos);
		subject.title = output_message.substr(pos, end - pos);
		//此时查找此前最后一行
		size_t pre_end_end = output_message.find_last_of('\n', pos);
		size_t pre_end_start = output_message.find_last_of('\n', pre_end_end-1);
		subject.syoukai = output_message.substr(pre_end_start + 1, pre_end_end - pre_end_start - 1);

		pos = output_message.find("e: ", pos + 3)+3;
		end = output_message.find('\n', pos);
		subject.eptitle = output_message.substr(pos, end - pos);

		pos = output_message.find("e: ", pos + 3)+3;
		//end = output_message.find('\n', pos);
		if (pos == 3)
		{
			//说明没有EPTitle：
			subject.progress = subject.eptitle;
			subject.progress = "未知";
		}
		else {
			subject.progress = output_message.substr(pos);
		}

	}
	catch (std::exception &e) {
		AT_SEND_MSG("信息获取失败...");
		return true;
	}


	//=====
	//首先输出匹配结果 
	AT_SEND_MSG(subject.toString());
	//之后尝试进行条目查询
	if (subject.title !="")
	{
		//替换这个标题中的空格
		std::string title(subject.title);
		size_t blank_pos = title.find(' ');
		while (blank_pos != std::string::npos)
		{
			if (blank_pos >= title.length() / 3) {
				title = title.substr(0, blank_pos);
				break;
			}
			title[blank_pos] = '+';
			blank_pos = title.find(' ', blank_pos + 1);
		}

		//去除可能的?防止误
		size_t what_pos = title.find('?');
		while (what_pos != std::string::npos)
		{
			title[what_pos] = '+';
			what_pos = title.find(' ', what_pos + 1);
		}
		
		//直接进行#刷新式查询
		std::string to_exec_ins = ":#";
		to_exec_ins += title;
		//
		switch (retType)
		{
		case BgmRetType::Private:
			Parsing(subType, msgId, fromQQ, to_exec_ins.c_str(), true);
			break; 
		case BgmRetType::Group:
		case BgmRetType::Discuss:
			ParsingM(subType, msgId, fromDiscussGroup, retType, fromQQ, to_exec_ins.c_str(), true);
			break; 
		default:
			break; 
		}
	}
	return true;
	
}


#endif