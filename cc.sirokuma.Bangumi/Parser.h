#include "Functions.h"

#ifndef BANGUMI_PARSER_H
#define BANGUMI_PARSER_H



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



}



#endif