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



}



#endif