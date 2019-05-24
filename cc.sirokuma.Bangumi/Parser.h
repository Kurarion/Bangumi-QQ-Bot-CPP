#include "Functions.h"
//��Ҫ���ص�ʱ��
#include "boost/date_time/posix_time/posix_time.hpp"
#ifndef BANGUMI_PARSER_H
#define BANGUMI_PARSER_H
//ģ�庯������ʽ����/��ʱ����,��ǰֻ���ڱ��ļ���ʹ�ù�ģ�庯��
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
	//С��0˵�����ǵ��ֽڵ�
	if (temp[0] < 0) {
		for (const auto& i : pre_code_muli) {
#ifndef NDEBUG
			CQ_addLog(ac, CQLOG_DEBUG, "Pre_code", i);
#endif

			if (temp[0] == i[0] && temp[1] == i[1]) {
				//��������msg��ǰ�ƶ�һλ
				//Ϊ�˺���codeʶ�𲻳�������
				++origin;
				return{ true,false };
			}
		}

	}
	//����0˵���ǵ��ֽڵĴ���
	else {
		//ʶ������˫�ַ�ǰ׺
		for (const auto& i : pre_code_refresh) {
#ifndef NDEBUG
			CQ_addLog(ac, CQLOG_DEBUG, "Pre_code", i);
#endif

			if (temp[0] == i[0] && temp[1] == i[1]) {
				//��������msg��ǰ�ƶ�һλ
				//Ϊ�˺���codeʶ�𲻳�������
				++origin;
				return{ true,true };
			}
		}

		//ʶ���ַ���ǰ׺
		for (const auto& i : pre_code_single) {
#ifndef NDEBUG
			CQ_addLog(ac, CQLOG_DEBUG, "Pre_code", i);
#endif
			if (temp[0] == i[0])
				return{ true,false };
		}
	}
	return {false,false};
	//if (test_str == ":" || test_str == "��" || test_str == "\\" || test_str == "=")
	//	return true;
	//else
	//	return false;
}
//����ԭmsg�еļ�ʱ,������countdown
inline std::pair<unsigned,std::string> AnalyseTimeWork(const char *msg) {
	std::string temp = msg;
	//��ǰʹ��_��Ϊ��ʱ�ķ���
	size_t t_pos = temp.find_last_of('_');
	unsigned countdown = 0;
	if (t_pos != std::string::npos) {
		//������������
		try {
			countdown = std::stoi(temp.substr(t_pos + 1));
			//��ʱӦ���ų�ԭmsg��&֮���������Ϣ
			return{ countdown, temp.substr(0,t_pos) };

		}
		catch (std::invalid_argument&) {
			//����������ַ�����һ����
			//��ʶ��
		}
	}
	//��󷵻�countdown
	return {countdown,msg};
}
//�����ݿ��и���ָ��ʹ�����
void UpdateCodeType(std::set<BgmCode>&pool,int64_t qq) {
	if (pool.empty()){
		//�����Ϊ�գ�ֱ�ӷ���
		return;
	}
	//��ǰʱ��
	boost::posix_time::ptime currentTime = boost::posix_time::second_clock::universal_time() + boost::posix_time::hours(8);
	boost::gregorian::date current_date = currentTime.date();
	std::string current_date_str(boost::gregorian::to_iso_extended_string(current_date));
	//��ѯ���
	bangumi::string query;
	query << "INSERT INTO bgm_users(user_id, user_qq, user_bangumi, user_access_token, user_refresh_token) "
		<< "VALUE(NULL, " << qq << ", 0, '', '') ON DUPLICATE KEY UPDATE ";
//��ݺ�
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
	//�����ϸ�������,ʹ�ö�������ʱ��
	
	std::string curr_time = boost::posix_time::to_iso_extended_string(currentTime);
	curr_time[curr_time.find_first_of('T')] = ' ';

	query << "BgmCode_Last_Date = '"<< curr_time <<"'";
	//���ִ��SQL��ѯ
	auto affect_rows_num = sql_pool.ExecQueryNoRes(query);
	try
	{
		SQLCheckResult(affect_rows_num);
	}
	catch (boost::system::system_error& e)
	{

		{
			bangumi::string debug_msg;
			debug_msg << "����ָ��ʹ�����ʧ��:"
				<< std::to_string(affect_rows_num)
				>> e.what();

			CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-UpdateCodeType", debug_msg);
		}

	}

	
	
}
inline void Parsing(int32_t subType, int32_t msgId, int64_t fromQQ, const char *msg, bool refresh) {
	//���δ���������list
	std::list<bangumi::Code> code_pool;
	//��������ı�ʶ��set
	std::set<BgmCode> code_type;
	//��ʱ���[Ŀǰ����Ӧ����˽��]
	auto time_result = AnalyseTimeWork(msg);
	//��������Ĳ�������
	bangumi::BGMCodeParam code_param(bangumi::BGMCodeExtraVar{time_result.first,refresh}, time_result.second.c_str(), code_pool, code_type, BgmRetType::Private, fromQQ);
	//����BGMCode��������
	bangumi::BGMCode input(code_param);
#ifndef NDEBUG
	CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot", input.c_str());
#endif
	//����BGMCode��������,���������code_pool
	input.AnalyseCode();
	//��ÿһ��Code����ִ�з���bangumi::string
	for (auto& i : code_pool) {
		i.ExecuteCode();
//#ifndef NDEBUG
	CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Instructs", i.Print());

//#endif

	}

	//����code_type,��¼���������ʹ�ô���
	UpdateCodeType(code_type,fromQQ);

}
inline void ParsingM(int32_t subType, int32_t msgId, int64_t fromDiscussGroup, BgmRetType DiscussOrGroup, int64_t fromQQ, const char *msg, bool refresh) {
	//���δ���������list
	std::list<bangumi::Code> code_pool;
	//��������ı�ʶ��set
	std::set<BgmCode> code_type;
	//��������Ĳ�������
	bangumi::BGMCodeParam code_param(bangumi::BGMCodeExtraVar{ 0,refresh }, msg, code_pool, code_type, DiscussOrGroup, fromQQ, fromDiscussGroup);
	//����BGMCode��������
	bangumi::BGMCode input(code_param);
#ifndef NDEBUG
	CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot", input.c_str());
#endif
	//����BGMCode��������,���������code_pool
	input.AnalyseCode();
	//��ÿһ��Code����ִ�з���bangumi::string
	for (auto& i : code_pool) {
		i.ExecuteCode();
//#ifndef NDEBUG
		CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Instructs", i.Print());

//#endif

	}

	//����code_type,��¼���������ʹ�ô���
	UpdateCodeType(code_type, fromQQ);

}



//��������AT������
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
//�����û�Ȩ��ֵ����
inline bool Update_Right(int64_t fromQQ, int right) {
	//��Ȩ
	bangumi::string open_query;
	open_query << "UPDATE bgm_users SET "
		<< "dmhy_open=" << right
		<< " WHERE user_qq=" << fromQQ;
	//Ӱ�������
	unsigned long affect_rows_num = sql_pool.ExecQueryNoRes(open_query);
	try
	{
		SQLCheckResult(affect_rows_num);
		return true;
	}
	catch (boost::system::system_error&)
	{
		//δ֪���
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
		//���Ӵ�������
		return false;
	}

	//ȥ�����ܵĿո�ǰ׺
	size_t site = msg.find_first_not_of(' ');
	if (site!=std::string::npos)
	{
		msg = msg.substr(site);
	}	

	const char * user_answer = msg.c_str();
	//����ǰ׺ʶ��
	auto verify = VerifyMsg(user_answer[0], user_answer[1], user_answer);
	//���δ���������list
	std::list<bangumi::Code> code_pool;
	if (verify.first)//��֤����
	{
		//��������ı�ʶ��set
		std::set<BgmCode> code_type;
		//��ʱ���[Ŀǰ����Ӧ����˽��]
		auto time_result = AnalyseTimeWork(user_answer);
		//��������Ĳ�������
		if (retType == BgmRetType::Private) {
			bangumi::BGMCodeParam code_param(bangumi::BGMCodeExtraVar{ time_result.first,verify.second }, time_result.second.c_str(), code_pool, code_type, BgmRetType::Private, fromQQ);
			bangumi::BGMCode input(code_param);
			//����BGMCode��������,���������code_pool
			input.AnalyseCode();
		}
		else {
			bangumi::BGMCodeParam code_param(bangumi::BGMCodeExtraVar{ 0,verify.second }, user_answer, code_pool, code_type, retType, fromQQ, fromDiscussGroup);
			bangumi::BGMCode input(code_param);
			//����BGMCode��������,���������code_pool
			input.AnalyseCode();
		}
		//��ʱ�Ѿ��õ����û�������code


	}
	else {
		//ָ��ǰ׺ʶ��ʧ��

		//�ڴ˴������
		//return true;
	}

	//������û����﷨����
	//======
	//Get�û���Ȩ��
	bangumi::string query;
	query << "SELECT dmhy_open "
		<< "FROM bgm_users "
		<< "WHERE user_qq="
		<< fromQQ;
	//��ѯ���
	BGMSQLResult result;
	//Ӱ�������
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
			debug_msg << "SQL��ѯʧ��:"
				<< std::to_string(affect_rows_num)
				>> e.what();

			CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-BOT-Statis", debug_msg);
		}
#endif	
		//����
		return false;
	}

	//��ݺ�
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

	//ǰ�Ĵ����
#define Handle_top(curr_right_num)\
if (has_updated) {\
	MARU();\
	ret << question[curr_right_num];\
	AT_SEND_MSG(ret);\
	return true;\
}\
else {\
bool is_right_answer = false;


	//��Ĵ����
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
	//���е�ǰ��״̬���
	bool has_updated = false;
	//
	if (affect_rows_num > 0) {
		//���Ȩ��
		switch (result[0][0])
		{
		case '0':
		{
			//��֤�Ƿ���ȷ�ظ�
			bool is_right_answer = false;
			//--����𰸵���֤--
			is_right_answer = msg.find("OK") != std::string::npos;
			//--
			if (is_right_answer) {
				has_updated = true;
				Update_Right(fromQQ, 1);
			}
			else {
				//��������
				AT_SEND_MSG(question[0]);
				return true;
			}

		}
		case '1': 
		{
			if (has_updated) {
				//�Ǵ���һ���׶ι�����
				//��������
				bangumi::string ret;
				ret << "[CQ:image,file=" << bgm.goodjob_pic << "]\n";
				ret << "[CQ:at,qq=" << fromQQ << "] ";
				ret << "OK~\n���ȵ�һ�����⣺\n";
				ret << question[1]; 
				AT_SEND_MSG(ret); 
				return true;
			}
			else {
				//�û��Ѿ��յ���������
				//�����û��Ļش�
				//��֤�Ƿ���ȷ�ظ�
				bool is_right_answer = false;
				//--����𰸵���֤--
				//����1��help
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
					//�û��ش����
					BATSU();
					ret << question[1]; 
					AT_SEND_MSG(ret); 
					return true;
				}
			}

		}
		case '2':
		{
			Handle_top(2);
			//--����𰸵���֤--
			//����2��
			for (auto&code : code_pool) {
				if (code.GetType() == BgmCode::Tag) {
					//�����û���ָ�����
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
		case '3':
		{
			Handle_top(3);
			//--����𰸵���֤--
			bool stage1 = false;
			bool stage2 = false;
			bool stage3 = false;
			//���ȼ��ǿ�Ƹ���
			if (verify.second)
			{
				for (auto&code : code_pool) {
					if (code.GetType() == BgmCode::User) {
						//�����û���ָ�����
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
				//������
				if (stage1&&stage2&&stage3)
				{
					is_right_answer = true;
				}
			}
			//--
			Handle_down(3);
		}
		case '4':
		{
			Handle_top(4);
			//--����𰸵���֤--
			//����4��
			if (!code_pool.empty())
			{
				auto& code = code_pool.front();
				if (code.GetType() == BgmCode::Subject)
				{
					//�����û���ָ�����
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
		case '5':
		{
			Handle_top(5);
			//--����𰸵���֤--
			//����5��
			if (!code_pool.empty())
			{
				auto& code = code_pool.front();
				if (code.GetType() == BgmCode::Search)
				{
					//�����û���ָ�����
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
		case '6':
		{
			Handle_top(6);
			//--����𰸵���֤--
			//����6��
			if (!code_pool.empty())
			{
				auto& code = code_pool.front();
				if (code.GetType() == BgmCode::Up)
				{
					//�����û���ָ�����
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
		case '7':
		{
			Handle_top(7);
			//--����𰸵���֤--
			//����7��
			if (!code_pool.empty())
			{
				auto& code = code_pool.front();
				if (code.GetType() == BgmCode::Up)
				{
					//�����û���ָ�����
					for (auto& param : code.GetSTRParam()) {
						//�����׼��
						standard_answer[7].id = bangumi::PARA_Resolve_Sharp({35,65,110,103,101,108,43,66,101,97,116,115,45,50}, BgmCode::Up).id;
						//��֤
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
		case '8':
		{
			Handle_top(8);
			//--����𰸵���֤--
			//����8��
			if (!code_pool.empty())
			{
				auto& code = code_pool.front();
				if (code.GetType() == BgmCode::Collect)
				{
					//�����û���ָ�����
					for (auto& param : code.GetSTRParam()) {
						//�����׼��
						standard_answer[8].id = bangumi::PARA_Resolve_Percent({ 37,114,101,119,114,105,116,101,42,103,42,50 }, BgmCode::Collect).id;
						//��֤
						if (bangumi::ResolveCollectPara(param) == standard_answer[8]) {
							is_right_answer = true;
							break;
						}
					}
				}
			}
			//--
			Handle_down(8);
		}
		case '9':
		{
			if (has_updated) {
				//�Ǵ���һ���׶ι�����
				bangumi::string ret;
				ret << "[CQ:image,file=" << bgm.goodjob_pic << "]\n";
				ret << "[CQ:at,qq=" << fromQQ << "] ";
				ret << test_passed;
				AT_SEND_MSG(ret);
				return true;
			}
			//���ͻظ�
			bangumi::string ret;
			ret << "[CQ:image,file=" << bgm.goodjob_pic << "]\n"
				<< "[CQ:at,qq=" << fromQQ << "] "
				<< "����ͨ���˿��ˣ�\nEnjoy it~";
			AT_SEND_MSG(ret);
			return true;
		}
		default:
		{
			//����
			return false;
		}
		}
	}
	else
	{
		//����
		return false;
	}
	//======
}


#endif