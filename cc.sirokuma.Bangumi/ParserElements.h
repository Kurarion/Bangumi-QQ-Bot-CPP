#include <set>
#include <unordered_set>
#include <unordered_map>
#include <list>
#include <functional>
#include "Bangumi.h"

#ifndef PARSERELEMENTS_H
#define	PARSERELEMENTS_H
//一条以命令连接符(, .)分开的语句一般以指令开关,或者使用一个id(可以加上id解释符),例如: 123,*AngleBeats

const static std::map<BgmRetType, std::function<decltype(CQ_sendPrivateMsg)>> SendMsg
{
	{ BgmRetType::Private, std::cref(CQ_sendPrivateMsg) },
	{ BgmRetType::Discuss, std::cref(CQ_sendDiscussMsg) },
	{ BgmRetType::Group, std::cref(CQ_sendGroupMsg) }
};
//************
//刷新缓存的命令识别前缀
const static std::set<const char *> pre_code_refresh{ "::" };
//命令识别前缀(单)
const static std::set<const char *> pre_code_single{ ":",";","=","\\" };
//命令识别前缀(双)
const static std::set<const char *> pre_code_muli{ "：","；" };
//主要用于处理无命令符指令时使用
const static std::string Default_Key("Subject");
//************
//unordered_set的直接初始化中后写的默认在set的前列
//命令连接符是 , .
const static std::unordered_set<char> code_link{ ':'/*,'.',','*/,';','=','\\' };
//指令集 //注意这里是识别指令的关键,顺序不能放反
//后面的元素优先级高,因此长字符放在后面
const static std::unordered_map<std::string, BgmCode> instruct
{
	{ "conf", BgmCode::Conf },
	{ "绑定", BgmCode::Reg },
	{ "tml", BgmCode::TML },
	{ "时光机", BgmCode::TML },
	{ "reg", BgmCode::Reg },
	{ "帮助", BgmCode::Help},
	{ "help", BgmCode::Help },
	{ "用户", BgmCode::User },
	{ "user", BgmCode::User },
	{ "moe", BgmCode::MOE },
	{ "dmhy", BgmCode::DMHY },
	{ "标签", BgmCode::Tag},
	{ "tag", BgmCode::Tag },
	{ "搜索", BgmCode::Search },
	{ "?", BgmCode::Search },
	{ "？", BgmCode::Search },
	//{ "!", BgmCode::List },
	{ "查询", BgmCode::Subject },
	{ "条目", BgmCode::Subject },
	{ "acg", BgmCode::Subject},
	{ "收藏", BgmCode::Collect },
	{ "co", BgmCode::Collect },
	{ "更新", BgmCode::Up },
	{ "up", BgmCode::Up },
	{ "++", BgmCode::Up },
	{ "rank", BgmCode::Statis},
	{ "统计", BgmCode::Statis},
	{ Default_Key, BgmCode::Subject}
};
//参数连接符是+ 空格 / ,
const static std::unordered_set<char> para_link{ ' '/*,'+','/'*//*,',','.' */};

//************
//不需要size_t类型的参数的命令
const static std::set<BgmCode> instruct_no_need_int{ BgmCode::TML,BgmCode::User,BgmCode::Tag, BgmCode::Search,BgmCode::Statis/*,"!"*/ };
//需要string类型参数的命令
const static std::set<BgmCode> instruct_need_str{ BgmCode::DMHY,BgmCode::MOE };
//可能使用Last Subject的命令（暂时不用）
//const static std::set<std::string> instruct_may_use_last{ Default_Key,"acg", "co", "up", "++" };
//可能使用复杂参数的命令
const static std::set<BgmCode> instruct_may_use_complex_param{ BgmCode::Subject,BgmCode::Collect,BgmCode::Up ,BgmCode::List };
//指令对应的函数 //指令调用与存放顺序无关
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
//参数对应的解析函数
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
//id解释符
const static std::unordered_set<std::string> id_unref{ "-","*","**","+","/","#","%" };
//空格参数代替符
//const static std::unordered_set<char> para_blank{ '_' };

//************
//命令池
//static std::list<Code> code_pool;

//首先根据命令连接符分隔单独处理每个语句






#endif
