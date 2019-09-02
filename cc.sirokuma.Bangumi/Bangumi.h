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
//ת����
CodeConverter code_converter("utf-8", "gb18030");
//ת����
CodeConverter code_encoder("gb18030", "utf-8");
//����ͼ�λ�������
#define PHY_PROGRESS_MAX_NUM 16
#define PROCRESS_MAX_NUM 12
inline std::string CalPhyProgress(int current_num, int max_num) {
	if (current_num==0 || max_num ==0)
	{
		return "";
	}
	if (max_num <= PHY_PROGRESS_MAX_NUM) {
		//�����������ֱ�Ӽ������Ǽ���
		std::string ret(current_num, '>');
		return std::move(ret);
	}
	int n = std::ceil(PHY_PROGRESS_MAX_NUM * current_num / max_num);
	std::string ret(n,'>');
	return std::move(ret);
}
namespace bangumi {
	//	//UTF-8�ַ�תGBK
	//#define UTF82GBK(s)  boost::locale::conv::from_utf(s, "GBK")
		//UTF-8�ַ�תGB18030
#define UTF82GBK(s)  code_converter.Conv(s)
	//
	const std::string Bangumi_Type[] = {
		"", //0
		"�鼮",
		"����",
		"����",
		"��Ϸ",
		"",
		"����Ԫ" //6
	};
	const std::string Bangumi_Type_State[] = {
		"", //0
		"��",
		"��",
		"��",
		"��",
		"",
		"��" //6
	};
	const std::string Bangumi_Weekday[] = {
		"",
		"����һ/������",
		"���ڶ�/������",
		"������/ˮ����",
		"������/ľ����",
		"������/������",
		"������/������",
		"������/������"
	};
	//���ڴ���ͳһʹ��UTF8
	const std::map< int, std::string> Bangumi_Collect_Status = {
		{ 1, code_encoder.Conv("�뿴/��/��/��") },
		{ 2, code_encoder.Conv("��/��/��/����") },
		{ 3, code_encoder.Conv("�ڿ�/��/��/��") },
		{ 4, code_encoder.Conv("����") },
		{ 5, code_encoder.Conv("����") }
	};
	const std::map< std::string, std::string > Bangumi_Status = {
		{ "wish", "�뿴/��/��/��" },
		{ "collect", "��/��/��/����" },
		{ "do", "�ڿ�/��/��/��" },
		{ "on_hold", "����" },
		{ "dropped", "����" }
	};
	//ǰ������MSG
	class Msg;
	//ǰ������,Http.h����Ҫ
	//ǰ������
	class Code;
	//�������Ϣ�ṹ��
	struct BGMCodeExtraVar {
		BGMCodeExtraVar(unsigned c, bool r)
			:countdown(c), refresh(r)
		{}
		BGMCodeExtraVar()
			:countdown(0), refresh(false)
		{}
		//����ʱ
		unsigned countdown = 0;
		//�Ƿ���дͼƬ����
		bool refresh = false;
		//�Ƿ�ʹ��sql����
		//bool useSQLCache = false;
		//bool useSQL;
	};
	//Code����Ϣ�ظ�֮�䴫�ݵĽṹ��
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
		//�������Ϣ
		BGMCodeExtraVar extra;
	};


	//��Ҫ������Ϣ�������
	//����bangumi��Ŀ�����Ϣ
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
		//<<������
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
			//����Ϊ0��С��λ
			std::string temp = std::to_string(ufloat);
			//С�����λ��
			auto cir = temp.find_last_of('.');
			//С������һ��0��λ��
			auto zero = temp.find_first_of('0', cir);
			if (zero != std::string::npos) {
				//��������0����
				if (zero == cir + 1) {
					//0.000�����
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
		//>>������ ���<<����һ������
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
		// /������
		//��ʽ����ת��
		operator const char*() const {
			return this->c_str();
		}


	};
	//��ݺ�
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

	//һ����Ϣ�ظ��Ľӿ�
	struct Msg_Interface
	{
		virtual bangumi::string Get() = 0;
	};
	//�ղؽṹ��
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
			//�뿴�û���:  246
			//�ڿ��û���:  826
			//�����û���:  1144
			//�����û���:  75
			//�����û���:  101
			std::string x = Bangumi_Type_State[type];
			bangumi::string ret;
			ret << "��" << x << "�û���:  " << wish
				>> "��" << x << "�û���:  " << doing
				>> x << "���û���:  " << collect
				>> "�����û���:  " << on_hold
				>> "�����û���:  " << dropped;
			return std::move(ret);
		}


		int wish = 0;
		int collect = 0;
		int doing = 0;
		int on_hold = 0;
		int dropped = 0;
		int type = 2;
	};
	//һ��ظ���Ϣ��
	//��Ҫ������ʾ�û���Ϣ����
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
	//ר�����Auth����Ϣ��,��Ҫ����token��Ϣ����
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
	//TODO:�����������Ļ�������:һ������EXEC(),�Զ�����һ��json��������ɶ��Լ��Ĺ���,����ʹ��һ������������������
	//TODO:���ܵĻ�ΪSUBJECT�����һ�����ݽṹ����¼USER��SUBJECT�Ĺ���
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
			//[ͼƬ]
			//������:  ��ʹ���ٵ��������
			//ԭ��:  ˽����ʹ���褤���ꤿ��
			//����:  12
			//��������:  2019-01-08
			//��������:  ���ڶ�/������
			//����:  ����      ID: 249637
			//���:  ......
			//
			//����:  542
			//����:  7.7      ������: 1324
			//
			//�뿴�û���:  246
			//�ڿ��û���:  826
			//�����û���:  1144
			//�����û���:  75
			//�����û���:  101
			//��Ŀ��ҳ:  http://bgm.tv/subject/249637
			bangumi::string ret;
			ret << "[CQ:image,file=" << image_file << "]";
			STRRET("������:  ", UTF82GBK(name_cn));
				//>> "������:  " << name_cn
			ret >> "ԭ��:  " << UTF82GBK(name);
			RET("����:  ", eps);
				//>> "����:  " << eps
			STREQUALRET("��������:  ", boost::gregorian::to_iso_extended_string(air_date), "not-a-date-time");
				//>> "��������:  " << boost::gregorian::to_iso_extended_string(air_date)
			EXRET("��������:  ", Bangumi_Weekday[air_weekday], air_weekday);
				//>> "��������:  " << Bangumi_Weekday[air_weekday]
			ret >> "����:  " << Bangumi_Type[type] << "    ID:  " << subject_id;
			////���ﴦ��һ��summary�е�\n
			//size_t temp = summary.find("\r\n");
			//while (temp!=std::string::npos) {
			//	summary[temp] = ' ';
			//	summary[temp+1] = ' ';
			//	temp = summary.find("\r\n", temp + 2);
			//}
			STRRET("���: ", UTF82GBK(summary));
				//>> "���: " << summary
			ret >> ""
				>> "����:  " << (rank == 0 ? "��" : std::to_string(rank))
				>> "����:  " << rating_score << "    ������:  " << rating_num
				>> "------\n<������ " << get_current_time() << ">";
			//�������ֲַ�
			for (int i = 1; i < 10; ++i) {
				ret >> "��   " << i << "��| ";
				ret << CalPhyProgress(detail_score[i], detail_score[0])
					<< ' ' << detail_score[i] << "��";
			}
			//��Ϊλ����ͬ10����
			ret >> "�� " << 10 << "��| ";
			ret << CalPhyProgress(detail_score[10], detail_score[0])
				<< ' ' << detail_score[10] << "��";

			ret	>> ""
				>> collection.Get()
				>> "��Ŀ��ҳ:  " << url;
			return std::move(ret);
		}
		//������������ʱ�Ļظ�
		bangumi::string SearchGet(int num) {
			//
			//_____<1>_____
			//[ͼƬ]
			//��Ӱ֮���� - ��ŵĳ�� -
			//< ��Ӱ�Υ��ߥ�饯�� - ��Ť��� - >
			//< -��Ϸ->ID: 196972
			//��Ŀ��ҳ : http ://bgm.tv/subject/196972
			bangumi::string ret;
			ret >> "_____<" << num << ">_____";
			ret >> "[CQ:image,file=" << image_file << "]";
			STRRET("", UTF82GBK(name_cn));
			ret >> "< " << UTF82GBK(name) << " >";
			STREQUALRET("��������:  ", boost::gregorian::to_iso_extended_string(air_date), "not-a-date-time");
			ret >> "����:  " << Bangumi_Type[type] << "    ID:  " << subject_id;
			
			//ret >> ""
			ret >> "����:  " << (rank == 0 ? "��" : std::to_string(rank))
				>> "����:  " << rating_score << "    ������:  " << rating_num;
				//>> ""
				//>> collection.Get();
			ret	>> "��Ŀ��ҳ:  " << url;
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
		//��Ŀ�����ĸ��µ�����
		std::string refresh_time;
	};
	//��Ŀ�½ڵȽṹ��
	struct BangumiSubjectCollection :public Msg_Interface
	{
	public:
		//Ĭ�Ϲ��캯��
		BangumiSubjectCollection()
			:name_cn(""),
			name(""),
			file_path("")
		{}

		//�ڲ�����eps������ܼ���
		void UpdateEpsCounts() {
			eps_counts = air_eps.size() + unair_eps.size();
			sp_eps_counts = sp_air_eps.size() + sp_unair_eps.size();
		}
		//�����Ѿ����͵Ļ���
		int GetEpsAiredCount() {
			return air_eps.size();
		}
		//����δ���͵Ļ���
		int GetEpsUnAiredCount() {
			return unair_eps.size();
		}
		//����TV���ܻ���
		int GetEpsCount(){
			return eps_counts;
		}
		//�����Ѿ�����SP�Ļ���
		int GetSPEpsAiredCount() {
			return sp_air_eps.size();
		}
		//����δ����SP�Ļ���
		int GetSPEpsUnAiredCount() {
			return sp_unair_eps.size();
		}
		//����SP���ܻ���
		int GetSPEpsCount() {
			return sp_eps_counts;
		}
		//����SP+TV���ܻ���
		int GetAllEpsCount() {
			return sp_eps_counts+eps_counts;
		}
		//�Ƿ���Ч
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
			//�ų�û�л�ȡ����Ч�ܻ��������
			if (GetAllEpsCount() == 0)
			{
				return "";
			}
			bangumi::string res;
			if (before != -1)
			{
				res << "����ǰ: |" << std::string(std::ceil(PROCRESS_MAX_NUM * before / GetAllEpsCount()), '>') << " " << before
					<< '\n';
			}	
			res << "�ѹۿ�: |" << std::string(std::ceil(PROCRESS_MAX_NUM * curr_eps / GetAllEpsCount()), '>')<< " " << curr_eps
				>> "�ѷ���: |" << std::string(std::ceil(PROCRESS_MAX_NUM * GetEpsAiredCount() / GetAllEpsCount()), '>') << " " << GetEpsAiredCount()
				>> "�ܻ���: |" << std::string(PROCRESS_MAX_NUM, '>')<<" "<<GetAllEpsCount();
			return std::move(res);
		}
		//��Ч��
		bool valid = true;
	public:
		bangumi::string Get() override {
			//[ͼƬ]
			//����״̬:  3/12  
			//�ѷ���TV:  
			// ep1. XXXX
			// ep2. BBBB
			// ep3. CCCC
			//δ����TV:
			// ep4. KKKK
			//�ѷ���SP: 
			// ep6.5 JJJJ
			//δ����SP:
			// ep7.5 LLLL
			//��Ŀ��ҳ:  https://bgm.tv/subject/
			//
			bangumi::string ret;
			//ret << '< ' << subject_id << ' >';
			ret << "[CQ:image,file=" << file_path << "]";
			STRRET("", name_cn);
			ret >> "< " << name << " >";
			if (GetEpsCount() == 0) {
				ret >> "�� δ��¼�½���Ϣ...";
			}
			else {
				ret >> "����״̬:  " << GetEpsAiredCount() << '/' << GetEpsCount();
				if (GetEpsAiredCount() != 0) {
					int output_num = 3;
					ret >> "-----------"
						>> "�ѷ���TV:";
					int n = GetEpsAiredCount();
					//�ӵ͵������
					int start_pos = n - output_num;
					if (start_pos < 0) {
						start_pos = 0;
						output_num = n;
					}
					if (curr_eps != 0&& curr_eps<start_pos) {
						start_pos = curr_eps;
					}
					for (int i = start_pos; i < start_pos + output_num; ++i) {

						ret >> "�� " << air_eps[i];
						//������Ϣ
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
						>> "δ����TV:";
					int n = GetEpsUnAiredCount() < output_num ? GetEpsUnAiredCount() : output_num;
					for (int i = 0; i < n; ++i) {
						ret >> "�� " << unair_eps[i];
						//������Ϣ
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
						>> "�ѷ���SP:";
					int n = GetSPEpsAiredCount();
					//�ӵ͵������
					int start_pos = n - output_num;
					if (start_pos < 0)
						start_pos = 0;
					for (int i = start_pos; i < n; ++i) {
						ret >> "�� " << sp_air_eps[i];
						if (sp_air_eps_info.size() == sp_air_eps.size()) {
							//������Ϣ
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
						>> "δ����SP:";
					int n = GetSPEpsUnAiredCount() < output_num ? GetSPEpsUnAiredCount() : output_num;
					for (int i = 0; i < n; ++i) {
						ret >> "�� " << sp_unair_eps[i];
						//������Ϣ
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

			ret >> "��Ŀ��ҳ: https://bgm.tv/subject/" << subject_id;
			return std::move(ret);
		}
	};
	//�û����ӽ��ȵĽṹ��,λ��User����
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

		//Ĭ�Ϲ��캯��
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

		//Progressһʱ�ǰ��Ʒ,��ҪSubject����EPS����
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
				ret += "��";
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
			//[ͼƬ]
			//[Sirokuma] �ղ�Ϊ [����]
			//��ɶ�: 1/1 
			//[����]
			//����:  9   
			//�²�:  ����...����ɳ
			//Sirokuma ����ҳ:  http://bgm.tv/user/wz97315
			bangumi::string ret;
			ret <<" �ղ�Ϊ [" << UTF82GBK(status_name) << "]";
			STRRET("��ɶ�:  ", progress);
			if (to_add_str != "") {
				ret >> to_add_str;
			}
			if (rating != 0) {
				ret >> "����:  " << GetRateStr();
			}
			//RET("����:  ", rating);
			STRRET("�²�:  ", UTF82GBK(comment));
			//ret >> "�û���ҳ: " << url;
			return std::move(ret);
		}
		bangumi::string UpdateGet() {
			//[ͼƬ]
			//[Sirokuma] �ղ�Ϊ [����]
			//��ɶ�: 1/1 
			//[����]
			//����:  9   
			//�²�:  ����...����ɳ
			//Sirokuma ����ҳ:  http://bgm.tv/user/wz97315
			bangumi::string ret;
			ret << " �ղ�Ϊ [" << UTF82GBK(status_name) << "]";
			STRRET("��ɶ�:  ", progress);
			if (to_add_str != "") {
				ret >> to_add_str;
			}
			RET("����:  ", rating);
			STRRET("�²�:  ", UTF82GBK(comment));
			//ret >> "�û���ҳ: " << url;
			return std::move(ret);
		}
	};
	//TODO:ΪUSER�����һ�����ݽṹ����¼ID�Ͷ�ӦSubject�ļ�¼��Ϣ
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

		//�������ӵ�Ĭ�Ϲ��캯��
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
			//[ͼƬ]
			//Sirokuma @wz97315
			//�û�ǩ��:qiangqiang!
			//�û���ҳ:http://bgm.tv/user/wz97315
			bangumi::string ret;
			ret << '<' << user_id << '>'
				>> "[CQ:image,file=" << ava_file << "]"
				>> UTF82GBK(nick_name) << " @" << user_name;
			STRRET("�û�ǩ��: ", UTF82GBK(sign));
			ret >> "�û���ҳ: " << url;
			return std::move(ret);
		}

		bangumi::string ProgressGet() {
			bangumi::string ret;
			if (ava_file.empty() && nick_name.empty()) {
				ret >> '\n'
					<< "<δ�ղ�>";
				return std::move(ret);
			}
			ret >> "\n[CQ:image,file=" << ava_file << "]"
				>> "[" << UTF82GBK(nick_name) << "]";
			if (VerifyProgress()) {

				ret	<< progress.Get()
					>> "�û���ҳ: " << url;
			}
			else {
				ret << " δ�ղش���Ŀ"
					>> "�û���ҳ: " << url;
				
			}
			return std::move(ret);
		}
		bangumi::string UpdateGet() {
			bangumi::string ret;
			ret >> "\n[CQ:image,file=" << ava_file << "]"
				>> "[" << UTF82GBK(nick_name) << "]";
			if (VerifyProgress()) {

				ret << progress.UpdateGet()
					>> "�û���ҳ: " << url;
			}
			else {
				ret << " δ�ղش���Ŀ"
					>> "�û���ҳ: " << url;

			}
			return std::move(ret);
		}
	};

	//���Ӳ����ṹ��
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
		//Subject��
		//std::string tag_role_comment;
		bool use_last_subject_id = false;
		bool single = false;
		bool add_tag = false;
		bool add_role = false;
		bool add_comment = false;
		bool add_air_status = false;
		bool add_staff = false;
		bool add_attach = false;
		//�ͽ��������˽�ɫʱΪ��
		//bool only_role = false;
		bool NeedAdd() const{
			return add_tag || add_role || add_comment || add_air_status || add_staff || add_attach;
		}

		//Search��
		int search_type = 2;
		int search_start_pos = 0;
		int search_max_num = 5;

		//Collection Status
		//�ղ�״̬
		//	1 = wish = ����
		//	2 = collect = ����
		//	3 = do = ����
		//	4 = on_hold = ����
		//	5 = dropped = ����
		std::string collection_status;
		int collection_rating = 0;
		std::string collection_comment;
		std::string collection_tags;

		//Update��
		int update_watched_eps = 0;
		int update_eps_shift = 0;
		bool update_air = false;
		bool update_fin = false;
		//Ϊ�˸��õ�Update����֧�ּ���
		//BangumiSubjectCollection subject_html_data;

		//tag��
		std::string tag_keyword;
		std::string tag_airtime;
		int tag_page = 0;

		//RSS��
		std::string rss_keyword;
		int rss_max_items = 5;

		//�û���ѯ�ղ���
		//	a = ����
		//	c = ��
		//	g = ��Ϸ
		//	m = ����
		//	r = ����Ԫ
		//
		//	wish td = ����
		//	collect fin = ����
		//	do on = ����
		//	hold = ����
		//	drop = ����
		//
		//	 = �ղ�ʱ��
		//	rate = ����
		//	date = ����ʱ��
		//	title = ����
		std::string bangumi_user;
		std::string ucollection_subject_type = "anime";
		std::string ucollection_co_type = "collect";
		int ucollection_page = 1;
		std::string ucollection_order_type;
		std::string ucollection_tag;
	};
	//��Ϣ�ظ��򵥽ṹ��
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
		//�������Ϣ
		BGMCodeExtraVar extra;
		//���Ӳ����ṹ
		ComplexParam complex_param;
		//�ɹ�����Ϣ
		bangumi::string success_msg;
		//�������Ϣ
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

	//��Msg����֮����bangumi::string�ĺ���
	//string& string::operator<<(const Msg &msg) {
	//	*this += msg.Get();
	//	return *this;
	//}


	//����������������
	//���ڽ���һ����Ŀ�ķ�ʽ
	extern ComplexParam PARA_Resolve_One_Star(const std::string&, BgmCode);
	extern ComplexParam PARA_Resolve_Two_Star(const std::string&, BgmCode);
	extern ComplexParam PARA_Resolve_Line(const std::string&, BgmCode);
	extern ComplexParam PARA_Resolve_Plus(const std::string&, BgmCode);
	extern ComplexParam PARA_Resolve_Virgule(const std::string&, BgmCode);
	extern ComplexParam PARA_Resolve_Sharp(const std::string&, BgmCode);
	extern ComplexParam PARA_Resolve_Percent(const std::string&, BgmCode);

	//ǰ������
	struct BGMCodeParam;
	//BOT��Ϣ��������
	//������Ϣ��ȡ
	extern void BOT_Read_Ini(const BGMCodeParam &, const std::set<size_t>&, const std::set<std::string>&);
	//Help��Ϣ
	extern void BOT_Help(const BGMCodeParam &, const std::set<size_t>&, const std::set<std::string>&);
	//ͳ����Ϣ
	extern void BOT_Statis(const BGMCodeParam &, const std::set<size_t>&, const std::set<std::string>&);

	//BangumiAPI��������
	//User��Ϣ
	extern void BGM_API_User(const BGMCodeParam &, const std::set<size_t>&, const std::set<std::string>&);
	//Subject��Ϣ
	extern void BGM_API_Subject(const BGMCodeParam &, const std::set<size_t>&, const std::set<std::string>&);
	//��Bangumi��QQ
	extern void BGM_API_Auth(const BGMCodeParam &, const std::set<size_t>&, const std::set<std::string>&);
	//�����ղ�API
	extern void BGM_API_Collection(const BGMCodeParam &, const std::set<size_t>&, const std::set<std::string>&);
	//���½���API
	extern void BGM_API_Update(const BGMCodeParam &, const std::set<size_t>&, const std::set<std::string>&);
	//����API
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