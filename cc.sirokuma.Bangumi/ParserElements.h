#include <set>
#include <unordered_set>
#include <unordered_map>
#include <list>
#include <functional>
#include "Bangumi.h"

#ifndef PARSERELEMENTS_H
#define	PARSERELEMENTS_H
//һ�����������ӷ�(, .)�ֿ������һ����ָ���,����ʹ��һ��id(���Լ���id���ͷ�),����: 123,*AngleBeats

const static std::map<BgmRetType, std::function<decltype(CQ_sendPrivateMsg)>> SendMsg
{
	{ BgmRetType::Private, std::cref(CQ_sendPrivateMsg) },
	{ BgmRetType::Discuss, std::cref(CQ_sendDiscussMsg) },
	{ BgmRetType::Group, std::cref(CQ_sendGroupMsg) }
};
//************
//ˢ�»��������ʶ��ǰ׺
const static std::set<const char *> pre_code_refresh{ "::" };
//����ʶ��ǰ׺(��)
const static std::set<const char *> pre_code_single{ ":",";","=","\\" };
//����ʶ��ǰ׺(˫)
const static std::set<const char *> pre_code_muli{ "��","��" };
//��Ҫ���ڴ����������ָ��ʱʹ��
const static std::string Default_Key("Subject");
//************
//unordered_set��ֱ�ӳ�ʼ���к�д��Ĭ����set��ǰ��
//�������ӷ��� , .
const static std::unordered_set<char> code_link{ ':'/*,'.',','*/,';','=','\\' };
//ָ� //ע��������ʶ��ָ��Ĺؼ�,˳���ܷŷ�
//�����Ԫ�����ȼ���,��˳��ַ����ں���
const static std::unordered_map<std::string, BgmCode> instruct
{
	{ "conf", BgmCode::Conf },
	{ "��", BgmCode::Reg },
	{ "tml", BgmCode::TML },
	{ "ʱ���", BgmCode::TML },
	{ "reg", BgmCode::Reg },
	{ "����", BgmCode::Help},
	{ "help", BgmCode::Help },
	{ "�û�", BgmCode::User },
	{ "user", BgmCode::User },
	{ "moe", BgmCode::MOE },
	{ "dmhy", BgmCode::DMHY },
	{ "��ǩ", BgmCode::Tag},
	{ "tag", BgmCode::Tag },
	{ "����", BgmCode::Search },
	{ "?", BgmCode::Search },
	{ "��", BgmCode::Search },
	//{ "!", BgmCode::List },
	{ "��ѯ", BgmCode::Subject },
	{ "��Ŀ", BgmCode::Subject },
	{ "acg", BgmCode::Subject},
	{ "�ղ�", BgmCode::Collect },
	{ "co", BgmCode::Collect },
	{ "����", BgmCode::Up },
	{ "up", BgmCode::Up },
	{ "++", BgmCode::Up },
	{ "rank", BgmCode::Statis},
	{ "ͳ��", BgmCode::Statis},
	{ Default_Key, BgmCode::Subject}
};
//�������ӷ���+ �ո� / ,
const static std::unordered_set<char> para_link{ ' '/*,'+','/'*//*,',','.' */};

//************
//����Ҫsize_t���͵Ĳ���������
const static std::set<BgmCode> instruct_no_need_int{ BgmCode::TML,BgmCode::User,BgmCode::Tag, BgmCode::Search,BgmCode::Statis/*,"!"*/ };
//��Ҫstring���Ͳ���������
const static std::set<BgmCode> instruct_need_str{ BgmCode::DMHY,BgmCode::MOE };
//����ʹ��Last Subject�������ʱ���ã�
//const static std::set<std::string> instruct_may_use_last{ Default_Key,"acg", "co", "up", "++" };
//����ʹ�ø��Ӳ���������
const static std::set<BgmCode> instruct_may_use_complex_param{ BgmCode::Subject,BgmCode::Collect,BgmCode::Up ,BgmCode::List };
//ָ���Ӧ�ĺ��� //ָ���������˳���޹�
const static std::map<BgmCode, std::function<void (const bangumi::BGMCodeParam &, const std::set<size_t>&, const std::set<std::string>&)> >instruct_func_map
{
	{ BgmCode::Help, std::cref(bangumi::BOT_Help)},
	{ BgmCode::User, std::cref(bangumi::BGM_API_User) },
	{ BgmCode::Tag, std::cref(bangumi::BGM_API_Tag)},
	{ BgmCode::TML, std::cref(bangumi::BGM_TML)},
	{ BgmCode::Search, std::cref(bangumi::BGM_API_Search) },
	{ BgmCode::Statis, std::cref(bangumi::BOT_Statis) },
	{ BgmCode::Collect, std::cref(bangumi::BGM_API_Collection) },
	{ BgmCode::Up, std::cref(bangumi::BGM_API_Update) },
	{ BgmCode::Conf, std::cref(bangumi::BOT_Read_Ini) },
	{ BgmCode::Reg, std::cref(bangumi::BGM_API_Auth) },
	{ BgmCode::Subject, std::cref(bangumi::BGM_API_Subject) },
	{ BgmCode::DMHY, std::bind(bangumi::BGM_RSS,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3,BgmCode::DMHY) },
	{ BgmCode::MOE, std::bind(bangumi::BGM_RSS,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3,BgmCode::MOE) }
};
//������Ӧ�Ľ�������
const static std::map<std::string, std::function<bangumi::ComplexParam(const std::string&, BgmCode)>>paramter_func_map
{
	{ "-", std::cref(bangumi::PARA_Resolve_Line) },
	{ "*", std::cref(bangumi::PARA_Resolve_One_Star) },
	{ "**", std::cref(bangumi::PARA_Resolve_Two_Star) },
	{ "+", std::cref(bangumi::PARA_Resolve_Plus) },
	{ "/", std::cref(bangumi::PARA_Resolve_Virgule) },
	{ "#", std::cref(bangumi::PARA_Resolve_Sharp) },
	{ "%", std::cref(bangumi::PARA_Resolve_Percent) }
};

//************
//id���ͷ�
const static std::unordered_set<std::string> id_unref{ "-","*","**","+","/","#","%" };
//�ո���������
//const static std::unordered_set<char> para_blank{ '_' };

//************
//�����
//static std::list<Code> code_pool;

//���ȸ����������ӷ��ָ���������ÿ�����






#endif
