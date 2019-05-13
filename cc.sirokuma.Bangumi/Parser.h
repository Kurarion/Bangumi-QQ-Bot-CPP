#include "Functions.h"
//需要本地的时间
#include "boost/date_time/posix_time/posix_time.hpp"
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
	//当前使用_作为计时的符号
	size_t t_pos = temp.find_last_of('_');
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
	return {countdown,msg};
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
	//查询语句
	bangumi::string query;
	query << "INSERT INTO bgm_users(user_id, user_qq, user_bangumi, user_access_token, user_refresh_token) "
		<< "VALUE(NULL, " << qq << ", 0, '', '') ON DUPLICATE KEY UPDATE ";
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
	//最后加上更新日期,使用东八区的时间
	
	std::string curr_time = boost::posix_time::to_iso_extended_string(currentTime);
	curr_time[curr_time.find_first_of('T')] = ' ';

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
	//本次命令的参数对象
	bangumi::BGMCodeParam code_param(bangumi::BGMCodeExtraVar{ 0,refresh }, msg, code_pool, code_type, DiscussOrGroup, fromQQ, fromDiscussGroup);
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



#endif