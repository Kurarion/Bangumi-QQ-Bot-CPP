#include <memory>
#include <set>
#include <string>
#include <algorithm>
#include <boost/locale.hpp>
#include "cqp.h"
#include "BangumiExceptions.h"
#include <boost/date_time/gregorian/gregorian.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"
#include "iconv.h"
#ifndef BANGUMI_H
#define BANGUMI_H



enum class BgmRetType {
	Private,
	Group,
	Discuss
};
enum class BgmCode {
	Subject,
	Search,
	User,
	List,
	BGM,
	DMHY,
	MOE,
	Up,
	Collect,
	Conf,
	Reg,
	Help,
	Tag,
	Statis,
	RSS,
	TML,
	Unknow
};
//转码类
CodeConverter code_converter("utf-8", "gb18030");
//转码类
CodeConverter code_encoder("gb18030", "utf-8");
//计算图形化进度条
#define PHY_PROGRESS_MAX_NUM 16
#define PROCRESS_MAX_NUM 12
inline std::string CalPhyProgress(int current_num, int max_num) {
	if (current_num==0 || max_num ==0)
	{
		return "";
	}
	if (max_num <= PHY_PROGRESS_MAX_NUM) {
		//如果个数很少直接几个就是几个
		std::string ret(current_num, '>');
		return std::move(ret);
	}
	int n = std::ceil(PHY_PROGRESS_MAX_NUM * current_num / max_num);
	std::string ret(n,'>');
	return std::move(ret);
}
namespace bangumi {
	//	//UTF-8字符转GBK
	//#define UTF82GBK(s)  boost::locale::conv::from_utf(s, "GBK")
		//UTF-8字符转GB18030
#define UTF82GBK(s)  code_converter.Conv(s)
	//
	const std::string Bangumi_Type[] = {
		"", //0
		"书籍",
		"动画",
		"音乐",
		"游戏",
		"",
		"三次元" //6
	};
	const std::string Bangumi_Type_State[] = {
		"", //0
		"读",
		"看",
		"听",
		"玩",
		"",
		"看" //6
	};
	const std::string Bangumi_Weekday[] = {
		"",
		"星期一/月曜日",
		"星期二/火曜日",
		"星期三/水曜日",
		"星期四/木曜日",
		"星期五/金曜日",
		"星期六/土曜日",
		"星期日/日曜日"
	};
	//由于处理统一使用UTF8
	const std::map< int, std::string> Bangumi_Collect_Status = {
		{ 1, code_encoder.Conv("想看/玩/听/读") },
		{ 2, code_encoder.Conv("看/玩/听/读过") },
		{ 3, code_encoder.Conv("在看/玩/听/读") },
		{ 4, code_encoder.Conv("搁置") },
		{ 5, code_encoder.Conv("抛弃") }
	};
	const std::map< std::string, std::string > Bangumi_Status = {
		{ "wish", "想看/玩/听/读" },
		{ "collect", "看/玩/听/读过" },
		{ "do", "在看/玩/听/读" },
		{ "on_hold", "搁置" },
		{ "dropped", "抛弃" }
	};
	//前向声明MSG
	class Msg;
	//前向声明,Http.h中需要
	//前置声明
	class Code;
	//命令附加信息结构体
	struct BGMCodeExtraVar {
		BGMCodeExtraVar(unsigned c, bool r)
			:countdown(c), refresh(r)
		{}
		BGMCodeExtraVar()
			:countdown(0), refresh(false)
		{}
		//倒计时
		unsigned countdown = 0;
		//是否重写图片缓存
		bool refresh = false;
		//是否使用sql缓存
		//bool useSQLCache = false;
		//bool useSQL;
	};
	//Code与消息回复之间传递的结构体
	struct BGMCodeParam {
		BGMCodeParam(BGMCodeExtraVar iextra, const char* istr, std::list<Code>& icode_pool, std::set<BgmCode>& ibgm_code,
			BgmRetType itype, const int64_t& iqq, const int64_t&igroup = 0)
			:extra(iextra), raw_str(istr), str(istr), code_pool(icode_pool), bgm_code(ibgm_code), type(itype), qq(iqq), group(igroup)
		{}

		const char* raw_str;
		std::string str;
		std::list<Code>& code_pool;
		std::set<BgmCode>& bgm_code;
		BgmRetType type;
		const int64_t qq;
		const int64_t group = 0;
		//额外的信息
		BGMCodeExtraVar extra;
	};


	//主要用于信息输出方便
	//用在bangumi条目相关信息
	class string :public std::string
	{
	public:
		using std::string::string;
		using std::string::operator=;
		using std::string::operator+=;

		string(std::string str)
			:std::string(str)
		{

		}
		string() {}
		//<<操作符
		string& operator<<(const string & str) {
			*this += str;
			return *this;
		}
		string& operator<<(const std::string & str) {
			*this += str;
			return *this;
		}
		string& operator<<(const char ch) {
			*this += ch;
			return *this;
		}
		string& operator<<(const char *cstr) {
			*this += cstr;
			return *this;
		}
		string& operator<<(size_t uint) {
			*this += std::to_string(uint);
			return *this;
		}
		string& operator<<(int uint) {
			*this += std::to_string(uint);
			return *this;
		}
		string& operator<<(int64_t uint) {
			*this += std::to_string(uint);
			return *this;
		}
		string& operator<<(float ufloat) {
			//舍弃为0的小数位
			std::string temp = std::to_string(ufloat);
			//小数点的位置
			auto cir = temp.find_last_of('.');
			//小数点后第一个0的位置
			auto zero = temp.find_first_of('0', cir);
			if (zero != std::string::npos) {
				//有这样的0存在
				if (zero == cir + 1) {
					//0.000的情况
					*this += temp.substr(0, cir);
				}
				else {
					*this += temp.substr(0, zero);
				}

			}
			else {
				*this += temp;
			}

			return *this;
		}
		string& operator<<(const Msg &msg);
		//>>操作符 相比<<多了一个换行
		string& operator >> (const string&str) {
			*this += '\n';
			*this += str;
			return *this;
		}
		string& operator >> (const std::string&str) {
			*this += '\n';
			*this += str;
			return *this;
		}
		string& operator >> (const char *cstr) {
			*this += '\n';
			*this += cstr;
			return *this;
		}
		string& operator >> (const char ch) {
			*this += '\n';
			*this += ch;
			return *this;
		}
		string& operator >> (size_t uint) {
			*this += '\n';
			*this += std::to_string(uint);
			return *this;
		}
		// /操作符
		//隐式类型转换
		operator const char*() const {
			return this->c_str();
		}


	};
	//便捷宏
#define RET(str,var)\
if(var!=0)\
	ret>>str<<var;

#define STRRET(str,var)\
if(var.compare("")!=0)\
	ret>>str<<var;

#define EXRET(str,var,var2)\
if(var2!=0)\
	ret>>str<<var;

#define STREQUALRET(str,var,var2)\
if(var.compare(var2)!=0)\
	ret>>str<<var;

	//一个消息回复的接口
	struct Msg_Interface
	{
		virtual bangumi::string Get() = 0;
	};
	//收藏结构体
	struct Collection :public Msg_Interface
	{
		Collection(int iwish,
			int icollect,
			int idoing,
			int ion_hold,
			int idropped,
			int itype)
			:wish(iwish),
			collect(icollect),
			doing(idoing),
			on_hold(ion_hold),
			dropped(idropped),
			type(itype)
		{}
		Collection() {}

		bangumi::string Get()override {
			//
			//想看用户数:  246
			//在看用户数:  826
			//看过用户数:  1144
			//搁置用户数:  75
			//抛弃用户数:  101
			std::string x = Bangumi_Type_State[type];
			bangumi::string ret;
			ret << "想" << x << "用户数:  " << wish
				>> "在" << x << "用户数:  " << doing
				>> x << "过用户数:  " << collect
				>> "搁置用户数:  " << on_hold
				>> "抛弃用户数:  " << dropped;
			return std::move(ret);
		}


		int wish = 0;
		int collect = 0;
		int doing = 0;
		int on_hold = 0;
		int dropped = 0;
		int type = 2;
	};
	//一般回复消息类
	//主要用于提示用户信息或反馈
	struct Reply :public Msg_Interface
	{
		Reply(bangumi::string s)
			:re(s)
		{}

		bangumi::string re;
		bangumi::string Get() override {
			return re;
		}
	};
	//专门针对Auth的消息类,主要用于token信息返回
	struct AuthReply :public Msg_Interface
	{
		AuthReply(std::string iaccess_token,
		std::string itoken_type,
		size_t iuser_id,
		std::string irefresh_token)
			:access_token(iaccess_token),
			token_type(itoken_type),
			user_id(iuser_id),
			refresh_token(irefresh_token)
		{}
		AuthReply() {}

		std::string access_token = "";
		std::string token_type = "";
		size_t user_id = 0;
		std::string refresh_token = "";


		bangumi::string Get() override {
			return "OAuth2.0 Token AuthReply";
		}
	};
	//classes of bangumi
	//TODO:完成两个基类的基本功能:一个方法EXEC(),自动解析一个json数据来完成对自己的构造,或者使用一个工厂类来创建他们
	//TODO:可能的话为SUBJECT类添加一个数据结构来记录USER和SUBJECT的关联
	struct BangumiSubject :public Msg_Interface
	{
	public:
		BangumiSubject(size_t isubject_id,
			std::string iurl,
			int itype,
			std::string iname,
			std::string iname_cn,
			std::string isummary,
			int ieps,
			boost::gregorian::date iair_date,
			int iair_weekday,
			int irating_num,
			float irating_score,
			int irank,
			std::string iimage_file,
			Collection icollection,
			int idscore[11],
			std::string p_time_str
		)
			:subject_id(isubject_id),
			url(iurl),
			type(itype),
			name(iname),
			name_cn(iname_cn),
			summary(isummary),
			eps(ieps),
			air_date(iair_date),
			air_weekday(iair_weekday),
			rating_num(irating_num),
			rating_score(irating_score),
			rank(irank),
			image_file(iimage_file),
			collection(icollection),
			refresh_time(p_time_str)
		{
			for (int i = 0;i<11;++i)
			{
				detail_score[i] = idscore[i];
			}
		}
		BangumiSubject() {}

		//return subject id
		size_t operator()() { return subject_id; }
		//virtual
		bangumi::string Get() override {
			//[图片]
			//中文名:  天使降临到了我身边
			//原名:  私に天使が舞い降りた！
			//话数:  12
			//放送日期:  2019-01-08
			//放送星期:  星期二/火曜日
			//类型:  动画      ID: 249637
			//简介:  ......
			//
			//排名:  542
			//评分:  7.7      评分数: 1324
			//
			//想看用户数:  246
			//在看用户数:  826
			//看过用户数:  1144
			//搁置用户数:  75
			//抛弃用户数:  101
			//条目主页:  http://bgm.tv/subject/249637
			bangumi::string ret;
			ret << "[CQ:image,file=" << image_file << "]";
			STRRET("中文名:  ", UTF82GBK(name_cn));
				//>> "中文名:  " << name_cn
			ret >> "原名:  " << UTF82GBK(name);
			RET("话数:  ", eps);
				//>> "话数:  " << eps
			STREQUALRET("放送日期:  ", boost::gregorian::to_iso_extended_string(air_date), "not-a-date-time");
				//>> "放送日期:  " << boost::gregorian::to_iso_extended_string(air_date)
			EXRET("放送星期:  ", Bangumi_Weekday[air_weekday], air_weekday);
				//>> "放送星期:  " << Bangumi_Weekday[air_weekday]
			ret >> "类型:  " << Bangumi_Type[type] << "    ID:  " << subject_id;
			////这里处理一下summary中的\n
			//size_t temp = summary.find("\r\n");
			//while (temp!=std::string::npos) {
			//	summary[temp] = ' ';
			//	summary[temp+1] = ' ';
			//	temp = summary.find("\r\n", temp + 2);
			//}
			STRRET("简介: ", UTF82GBK(summary));
				//>> "简介: " << summary
			ret >> ""
				>> "排名:  " << (rank == 0 ? "无" : std::to_string(rank))
				>> "评分:  " << rating_score << "    评分数:  " << rating_num
				>> "------\n<更新于 " << get_current_time() << ">";
			//增加评分分布
			for (int i = 1; i < 10; ++i) {
				ret >> "●   " << i << "分| ";
				ret << CalPhyProgress(detail_score[i], detail_score[0])
					<< ' ' << detail_score[i] << "人";
			}
			//因为位数不同10单独
			ret >> "● " << 10 << "分| ";
			ret << CalPhyProgress(detail_score[10], detail_score[0])
				<< ' ' << detail_score[10] << "人";

			ret	>> ""
				>> collection.Get()
				>> "条目主页:  " << url;
			return std::move(ret);
		}
		//用作搜索命令时的回复
		bangumi::string SearchGet(int num) {
			//
			//_____<1>_____
			//[图片]
			//月影之幻像 - 解放的翅膀 -
			//< 月影のシミュラクル - 解放の羽 - >
			//< -游戏->ID: 196972
			//条目主页 : http ://bgm.tv/subject/196972
			bangumi::string ret;
			ret >> "_____<" << num << ">_____";
			ret >> "[CQ:image,file=" << image_file << "]";
			STRRET("", UTF82GBK(name_cn));
			ret >> "< " << UTF82GBK(name) << " >";
			STREQUALRET("放送日期:  ", boost::gregorian::to_iso_extended_string(air_date), "not-a-date-time");
			ret >> "类型:  " << Bangumi_Type[type] << "    ID:  " << subject_id;
			
			//ret >> ""
			ret >> "排名:  " << (rank == 0 ? "无" : std::to_string(rank))
				>> "评分:  " << rating_score << "    评分数:  " << rating_num;
				//>> ""
				//>> collection.Get();
			ret	>> "条目主页:  " << url;
			return std::move(ret);
		}
	private:
		std::string get_current_time() {
			return refresh_time;
		}
	public:
		size_t subject_id = 0;
		std::string url;
		int type = 0;
		std::string name;
		std::string name_cn;
		std::string summary;
		int eps = 0;
		boost::gregorian::date air_date;
		int air_weekday = 0;
		int rating_num = 0;
		float rating_score = 0;
		int rank = 0;
		std::string image_file;
		Collection collection;
		int detail_score[11];
		//条目的最后的更新的日期
		std::string refresh_time;
	};
	//条目章节等结构体
	struct BangumiSubjectCollection :public Msg_Interface
	{
	public:
		//默认构造函数
		BangumiSubjectCollection()
			:name_cn(""),
			name(""),
			file_path("")
		{}

		//在插入完eps后计算总集数
		void UpdateEpsCounts() {
			eps_counts = air_eps.size() + unair_eps.size();
			sp_eps_counts = sp_air_eps.size() + sp_unair_eps.size();
		}
		//返回已经放送的话数
		int GetEpsAiredCount() {
			return air_eps.size();
		}
		//返回未放送的话数
		int GetEpsUnAiredCount() {
			return unair_eps.size();
		}
		//返回TV的总话数
		int GetEpsCount(){
			return eps_counts;
		}
		//返回已经放送SP的话数
		int GetSPEpsAiredCount() {
			return sp_air_eps.size();
		}
		//返回未放送SP的话数
		int GetSPEpsUnAiredCount() {
			return sp_unair_eps.size();
		}
		//返回SP的总话数
		int GetSPEpsCount() {
			return sp_eps_counts;
		}
		//返回SP+TV的总话数
		int GetAllEpsCount() {
			return sp_eps_counts+eps_counts;
		}
		//是否有效
		bool Valid() {
			return valid;
		}

		size_t subject_id = 0;
		std::string name_cn;
		std::string name;
		std::string file_path;
		std::vector<std::string> air_eps;
		std::vector<std::string> unair_eps;
		std::vector<std::string> sp_air_eps;
		std::vector<std::string> sp_unair_eps;
		std::vector<std::string> air_eps_info;
		std::vector<std::string> unair_eps_info;
		std::vector<std::string> sp_air_eps_info;
		std::vector<std::string> sp_unair_eps_info;
		int eps_counts = 0;
		int sp_eps_counts = 0;
		int curr_eps = 0;

		void SetCurrentEps(const int &curr) {
			curr_eps = curr;
		}
		bangumi::string GetExStr(int before = -1) {
			//排除没有获取到有效总话数的情况
			if (GetAllEpsCount() == 0)
			{
				return "";
			}
			bangumi::string res;
			if (before != -1)
			{
				res << "更新前: |" << std::string(std::ceil(PROCRESS_MAX_NUM * before / GetAllEpsCount()), '>') << " " << before
					<< '\n';
			}	
			res << "已观看: |" << std::string(std::ceil(PROCRESS_MAX_NUM * curr_eps / GetAllEpsCount()), '>')<< " " << curr_eps
				>> "已放送: |" << std::string(std::ceil(PROCRESS_MAX_NUM * GetEpsAiredCount() / GetAllEpsCount()), '>') << " " << GetEpsAiredCount()
				>> "总话数: |" << std::string(PROCRESS_MAX_NUM, '>')<<" "<<GetAllEpsCount();
			return std::move(res);
		}
		//有效性
		bool valid = true;
	public:
		bangumi::string Get() override {
			//[图片]
			//放送状态:  3/12  
			//已放送TV:  
			// ep1. XXXX
			// ep2. BBBB
			// ep3. CCCC
			//未放送TV:
			// ep4. KKKK
			//已放送SP: 
			// ep6.5 JJJJ
			//未放送SP:
			// ep7.5 LLLL
			//条目主页:  https://bgm.tv/subject/
			//
			bangumi::string ret;
			//ret << '< ' << subject_id << ' >';
			ret << "[CQ:image,file=" << file_path << "]";
			STRRET("", name_cn);
			ret >> "< " << name << " >";
			if (GetEpsCount() == 0) {
				ret >> "● 未收录章节信息...";
			}
			else {
				ret >> "放送状态:  " << GetEpsAiredCount() << '/' << GetEpsCount();
				if (GetEpsAiredCount() != 0) {
					int output_num = 3;
					ret >> "-----------"
						>> "已放送TV:";
					int n = GetEpsAiredCount();
					//从低到高输出
					int start_pos = n - output_num;
					if (start_pos < 0) {
						start_pos = 0;
						output_num = n;
					}
					if (curr_eps != 0&& curr_eps<start_pos) {
						start_pos = curr_eps;
					}
					for (int i = start_pos; i < start_pos + output_num; ++i) {

						ret >> "● " << air_eps[i];
						//额外信息
						if (air_eps_info.size() == air_eps.size()) {
							ret >> ">>";
							for (auto&c : air_eps_info[i]) {
								if (c == ' ') {
									ret >> ">>";
								}
								else {
									ret << c;
								}
							}
						}
					}
				}
				if (GetEpsUnAiredCount() != 0) {
					const int output_num = 2;
					ret >> "-----------"
						>> "未放送TV:";
					int n = GetEpsUnAiredCount() < output_num ? GetEpsUnAiredCount() : output_num;
					for (int i = 0; i < n; ++i) {
						ret >> "● " << unair_eps[i];
						//额外信息
						if (unair_eps_info.size() == unair_eps.size()) {
							ret >> ">>";
							for (auto&c : unair_eps_info[i]) {
								if (c == ' ') {
									ret >> ">>";
								}
								else {
									ret << c;
								}
							}
						}
					}
				}
				if (GetSPEpsAiredCount() != 0) {
					int output_num = 3;
					ret >> "-----------"
						>> "已放送SP:";
					int n = GetSPEpsAiredCount();
					//从低到高输出
					int start_pos = n - output_num;
					if (start_pos < 0)
						start_pos = 0;
					for (int i = start_pos; i < n; ++i) {
						ret >> "● " << sp_air_eps[i];
						if (sp_air_eps_info.size() == sp_air_eps.size()) {
							//额外信息
							ret >> ">>";
							for (auto&c : sp_air_eps_info[i]) {
								if (c == ' ') {
									ret >> ">>";
								}
								else {
									ret << c;
								}
							}
						}
					}
				}
				if (GetSPEpsUnAiredCount() != 0) {
					const int output_num = 2;
					ret >> "-----------"
						>> "未放送SP:";
					int n = GetSPEpsUnAiredCount() < output_num ? GetSPEpsUnAiredCount() : output_num;
					for (int i = 0; i < n; ++i) {
						ret >> "● " << sp_unair_eps[i];
						//额外信息
						if (sp_unair_eps_info.size() == sp_unair_eps.size()) {
							ret >> ">>";
							for (auto&c : sp_unair_eps_info[i]) {
								if (c == ' ') {
									ret >> ">>";
								}
								else {
									ret << c;
								}
							}
						}
					}
				}

			}

			ret >> "条目主页: https://bgm.tv/subject/" << subject_id;
			return std::move(ret);
		}
	};
	//用户收视进度的结构体,位于User类内
	struct BangumiUserProgress :public Msg_Interface
	{
	public:
		BangumiUserProgress(
			std::string istatus_name,
			int irating,
			std::string icomment,
			std::string iprogress)
			:status_name(istatus_name),
			rating(irating),
			comment(icomment),
			progress(iprogress),
			valid(true)
		{}

		//默认构造函数
		BangumiUserProgress()
			://url(""),
			//nick_name(""),
			//ava_file(""),
			status_name(""),
			rating(0),
			comment(""),
			progress(""),
			valid(false)
		{}

		//Progress一时是半成品,需要Subject给予EPS参数
		void AddEps(std::string eps) {
			progress += eps;
		}
		void AddEps(int eps) {
			progress += std::to_string(eps);
		}
		void SetExStr(const std::string&to_add_str) {
			this->to_add_str = to_add_str;
		}
		std::string GetRateStr() {
			std::string ret;
			for (int i = 0; i < rating; ++i)
			{
				ret += "★";
			}
			ret += ' ';
			ret += std::to_string(rating);
			return std::move(ret);
		}

		//size_t user_id;
		//std::string url;
		//std::string user_name;
		//std::string nick_name;
		//std::string ava_file;
		//std::string sign;
		std::string status_name;
		int rating;
		std::string comment;
		std::string progress;
		bool valid;
		std::string to_add_str = "";
	public:
		bangumi::string Get() override {
			//[图片]
			//[Sirokuma] 收藏为 [看过]
			//完成度: 1/1 
			//[进度]
			//评分:  9   
			//吐槽:  哈哈...弗利沙
			//Sirokuma 的主页:  http://bgm.tv/user/wz97315
			bangumi::string ret;
			ret <<" 收藏为 [" << UTF82GBK(status_name) << "]";
			STRRET("完成度:  ", progress);
			if (to_add_str != "") {
				ret >> to_add_str;
			}
			if (rating != 0) {
				ret >> "评分:  " << GetRateStr();
			}
			//RET("评分:  ", rating);
			STRRET("吐槽:  ", UTF82GBK(comment));
			//ret >> "用户主页: " << url;
			return std::move(ret);
		}
		bangumi::string UpdateGet() {
			//[图片]
			//[Sirokuma] 收藏为 [看过]
			//完成度: 1/1 
			//[进度]
			//评分:  9   
			//吐槽:  哈哈...弗利沙
			//Sirokuma 的主页:  http://bgm.tv/user/wz97315
			bangumi::string ret;
			ret << " 收藏为 [" << UTF82GBK(status_name) << "]";
			STRRET("完成度:  ", progress);
			if (to_add_str != "") {
				ret >> to_add_str;
			}
			RET("评分:  ", rating);
			STRRET("吐槽:  ", UTF82GBK(comment));
			//ret >> "用户主页: " << url;
			return std::move(ret);
		}
	};
	//TODO:为USER类添加一个数据结构来记录ID和对应Subject的记录信息
	struct BangumiUser :public Msg_Interface
	{
	public:
		BangumiUser(size_t iuser_id,
			std::string iurl,
			std::string iuser_name,
			std::string inick_name,
			std::string iava_file,
			std::string isign)
			:user_id(iuser_id),
			url(iurl),
			user_name(iuser_name),
			nick_name(inick_name),
			ava_file(iava_file),
			sign(isign)
		{}

		//被迫增加的默认构造函数
		BangumiUser()
			:user_id(0),
			url(""),
			user_name(""),
			nick_name(""),
			ava_file(""),
			sign("")
		{}

		void SetProgress(BangumiUserProgress iprogress) {
			progress = iprogress;
		}

		bool VerifyProgress() {
			if (progress.valid)
				return true;
			return false;
		}

		size_t user_id;
		std::string url;
		std::string user_name;
		std::string nick_name;
		std::string ava_file;
		std::string sign;
		BangumiUserProgress progress;
	public:
		bangumi::string Get() override {
			//<92981>
			//[图片]
			//Sirokuma @wz97315
			//用户签名:qiangqiang!
			//用户主页:http://bgm.tv/user/wz97315
			bangumi::string ret;
			ret << '<' << user_id << '>'
				>> "[CQ:image,file=" << ava_file << "]"
				>> UTF82GBK(nick_name) << " @" << user_name;
			STRRET("用户签名: ", UTF82GBK(sign));
			ret >> "用户主页: " << url;
			return std::move(ret);
		}

		bangumi::string ProgressGet() {
			bangumi::string ret;
			if (ava_file.empty() && nick_name.empty()) {
				ret >> '\n'
					<< "<未收藏>";
				return std::move(ret);
			}
			ret >> "\n[CQ:image,file=" << ava_file << "]"
				>> "[" << UTF82GBK(nick_name) << "]";
			if (VerifyProgress()) {

				ret	<< progress.Get()
					>> "用户主页: " << url;
			}
			else {
				ret << " 未收藏此条目"
					>> "用户主页: " << url;
				
			}
			return std::move(ret);
		}
		bangumi::string UpdateGet() {
			bangumi::string ret;
			ret >> "\n[CQ:image,file=" << ava_file << "]"
				>> "[" << UTF82GBK(nick_name) << "]";
			if (VerifyProgress()) {

				ret << progress.UpdateGet()
					>> "用户主页: " << url;
			}
			else {
				ret << " 未收藏此条目"
					>> "用户主页: " << url;

			}
			return std::move(ret);
		}
	};

	//复杂参数结构体
	struct ComplexParam {
		ComplexParam(size_t iid)
			:id(iid),
			collection_status("do")
		{}
		ComplexParam(std::string istr)
			:id(0),
			str(istr),
			collection_status("do")
		{}
		ComplexParam()
			:id(0),
			collection_status("do")
		{}
		bool operator==(const ComplexParam& c) {
			return id == c.id
				&& str == c.str
				&& use_last_subject_id == c.use_last_subject_id
				&& single == c.single
				&& add_tag == c.add_tag
				&& add_role == c.add_role
				&& add_comment == c.add_comment
				&& add_air_status == c.add_air_status
				&& add_staff == c.add_staff
				&& search_type == c.search_type
				&& search_start_pos == c.search_start_pos
				&& search_max_num == c.search_max_num
				&& collection_status == c.collection_status
				&& collection_rating == c.collection_rating
				&& collection_comment == c.collection_comment
				&& update_watched_eps == c.update_watched_eps
				&& update_eps_shift == c.update_eps_shift
				&& update_air == c.update_air
				&& update_fin == c.update_fin
				&& tag_keyword == c.tag_keyword
				&& tag_airtime == c.tag_airtime
				&& tag_page == c.tag_page;
		}
		size_t id;
		std::string str;
		//Subject用
		//std::string tag_role_comment;
		bool use_last_subject_id = false;
		bool single = false;
		bool add_tag = false;
		bool add_role = false;
		bool add_comment = false;
		bool add_air_status = false;
		bool add_staff = false;
		bool add_attach = false;
		//就仅仅附加了角色时为真
		//bool only_role = false;
		bool NeedAdd() const{
			return add_tag || add_role || add_comment || add_air_status || add_staff || add_attach;
		}

		//Search用
		int search_type = 2;
		int search_start_pos = 0;
		int search_max_num = 5;

		//Collection Status
		//收藏状态
		//	1 = wish = 想做
		//	2 = collect = 做过
		//	3 = do = 在做
		//	4 = on_hold = 搁置
		//	5 = dropped = 抛弃
		std::string collection_status;
		int collection_rating = 0;
		std::string collection_comment;
		std::string collection_tags;

		//Update用
		int update_watched_eps = 0;
		int update_eps_shift = 0;
		bool update_air = false;
		bool update_fin = false;
		//为了更好的Update功能支持加入
		//BangumiSubjectCollection subject_html_data;

		//tag用
		std::string tag_keyword;
		std::string tag_airtime;
		int tag_page = 0;

		//RSS用
		std::string rss_keyword;
		int rss_max_items = 5;

		//用户查询收藏用
		//	a = 动画
		//	c = 书
		//	g = 游戏
		//	m = 音乐
		//	r = 三次元
		//
		//	wish td = 想做
		//	collect fin = 做过
		//	do on = 在做
		//	hold = 搁置
		//	drop = 抛弃
		//
		//	 = 收藏时间
		//	rate = 评分
		//	date = 发售时间
		//	title = 名称
		std::string bangumi_user;
		std::string ucollection_subject_type = "anime";
		std::string ucollection_co_type = "collect";
		int ucollection_page = 1;
		std::string ucollection_order_type;
		std::string ucollection_tag;
	};
	//消息回复简单结构体
	struct BGMRetParam {
		BGMRetParam(const BGMCodeParam & param, size_t id, const std::string str)
			:type(param.type),
			qq(param.qq),
			group(param.group),
			extra(param.extra),
			cur_id(id),
			cur_str(str)
		{
			complex_param.id = id;
		}
		BGMRetParam(const BGMCodeParam & param)
			:type(param.type),
			qq(param.qq),
			group(param.group),
			extra(param.extra),
			cur_id(0),
			cur_str("")
		{}
		BGMRetParam()
		{
			type = BgmRetType::Private;
			qq = -1;
			group = 0;
			cur_id = 0;
			cur_str = "";
		}
		BGMRetParam(BgmRetType itype, int64_t iqq, int64_t igroup, size_t id, const std::string str, BGMCodeExtraVar iextra)
			:type(itype), qq(iqq), group(igroup), extra(iextra), cur_id(id),
			cur_str(str)
		{}

		bool empty() {
			return qq == -1;
		}

		BgmRetType type;
		int64_t qq;
		int64_t group = 0;
		size_t cur_id;
		std::string cur_str;
		//额外的信息
		BGMCodeExtraVar extra;
		//复杂参数结构
		ComplexParam complex_param;
		//成功的消息
		bangumi::string success_msg;
		//额外的消息
		bangumi::string extra_msg;
	};

	//class Msg
	//{
	//public:
	//	//return the msg
	//	const char * Get() { return msg; }
	//	const char * Get() const { return msg; }
	//protected:
	//private:
	//	std::shared_ptr<BangumiSubject> this_subject;
	//	std::shared_ptr<BangumiUser> this_user;
	//	const char* msg;
	//};

	//在Msg定义之后定义bangumi::string的函数
	//string& string::operator<<(const Msg &msg) {
	//	*this += msg.Get();
	//	return *this;
	//}


	//参数解析函数声明
	//用于解析一个条目的方式
	extern ComplexParam PARA_Resolve_One_Star(const std::string&, BgmCode);
	extern ComplexParam PARA_Resolve_Two_Star(const std::string&, BgmCode);
	extern ComplexParam PARA_Resolve_Line(const std::string&, BgmCode);
	extern ComplexParam PARA_Resolve_Plus(const std::string&, BgmCode);
	extern ComplexParam PARA_Resolve_Virgule(const std::string&, BgmCode);
	extern ComplexParam PARA_Resolve_Sharp(const std::string&, BgmCode);
	extern ComplexParam PARA_Resolve_Percent(const std::string&, BgmCode);

	//前置声明
	struct BGMCodeParam;
	//BOT信息函数声明
	//配置信息读取
	extern void BOT_Read_Ini(const BGMCodeParam &, const std::set<size_t>&, const std::set<std::string>&);
	//Help信息
	extern void BOT_Help(const BGMCodeParam &, const std::set<size_t>&, const std::set<std::string>&);
	//统计信息
	extern void BOT_Statis(const BGMCodeParam &, const std::set<size_t>&, const std::set<std::string>&);

	//BangumiAPI函数声明
	//User信息
	extern void BGM_API_User(const BGMCodeParam &, const std::set<size_t>&, const std::set<std::string>&);
	//Subject信息
	extern void BGM_API_Subject(const BGMCodeParam &, const std::set<size_t>&, const std::set<std::string>&);
	//绑定Bangumi与QQ
	extern void BGM_API_Auth(const BGMCodeParam &, const std::set<size_t>&, const std::set<std::string>&);
	//管理收藏API
	extern void BGM_API_Collection(const BGMCodeParam &, const std::set<size_t>&, const std::set<std::string>&);
	//更新进度API
	extern void BGM_API_Update(const BGMCodeParam &, const std::set<size_t>&, const std::set<std::string>&);
	//搜索API
	extern void BGM_API_Search(const BGMCodeParam &, const std::set<size_t>&, const std::set<std::string>&);
	//Tag
	extern void BGM_API_Tag(const BGMCodeParam &, const std::set<size_t>&, const std::set<std::string>&);
	//RSS
	extern void BGM_RSS(const BGMCodeParam &, const std::set<size_t>&, const std::set<std::string>&, BgmCode);
	//TML
	extern void BGM_TML(const BGMCodeParam &, const std::set<size_t>&, const std::set<std::string>&);
	//User Collection
	extern void BGM_User_Collection(const BGMCodeParam &, const std::set<size_t>&, const std::set<std::string>&);
	
}



#endif