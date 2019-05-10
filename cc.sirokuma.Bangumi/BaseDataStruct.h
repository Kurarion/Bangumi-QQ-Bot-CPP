#include "BangumiInfo.h"
#include "ParserElements.h"
#include <string>
#include <vector>

#ifndef BASEDATASTRUCT_H
#define BASEDATASTRUCT_H
#ifndef NDEBUG
//debug�ַ�����
#define DEBUG_STRING_SET(container,str)\
for(auto i:container){\
str<<i<<" ";\
}

#define DEBUG_STRING_MAP(container,str)\
for(auto i:container){\
str<<i.first<<":"<<i.second.first<<" ";\
}

#endif
namespace bangumi {
	//#ifndef NDEBUG
	//	int ac = -1;
	//#endif

	class BGMCode :public std::string
	{
	public:
		//using std::string::string;
		//BGMCode(const char* str, std::list<Code>& curr_code_pool, std::set<BgmCode>& curr_bgm_code, BgmRetType type, const int64_t& iqq, const int64_t& igroup = 0)
		//	:std::string(str), code_pool(curr_code_pool), bgm_code(curr_bgm_code), ret_type(type), qq(iqq), group(igroup)
		//{
		//	//ת��ΪСд
		//	std::transform(this->begin(), this->end(), this->begin(), ::tolower);
		//}
		BGMCode(BGMCodeParam& parameter)
			:std::string(parameter.str),param(parameter)
		{
			//ת��ΪСд
			std::transform(this->begin(), this->end(), this->begin(), ::tolower);
		}
		//ƥ�䱾����Ϣ
		//����Code�ಢ����Code����

		void AnalyseCode() {

			//ƥ���������
			Match(code_link, code_set);
#ifndef NDEBUG
			bangumi::string debug_code_set = "CodeSet: ";
			DEBUG_STRING_SET(code_set, debug_code_set);
			CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-BGMCode-AnalyseCode", debug_code_set);
#endif
			//ƥ�����е������ʶ��
			Match(instruct, instruct_map);
#ifndef NDEBUG
			bangumi::string debug_instruct_map = "InstructMap: ";
			DEBUG_STRING_MAP(instruct_map, debug_instruct_map);
			CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-BGMCode-AnalyseCode", debug_instruct_map);
#endif
			//ƥ������Ͳ���֮������ӷ�
			Match(para_link, para_link_set);
#ifndef NDEBUG
			bangumi::string debug_para_link_set = "ParaLinkSet: ";
			DEBUG_STRING_SET(para_link_set, debug_para_link_set);
			CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-BGMCode-AnalyseCode", debug_para_link_set);
#endif
			//�������ϲ����
			//���֪���еĹؼ��ֵ���ʼλ���Լ������ʼλ��
			//�������ָ�ÿ��ָ��

			size_t sen_beg = 0;
			//����������
			//ָ���
			auto instruct_iter = instruct_map.cbegin();
			//������
			auto para_link_iter = para_link_set.cbegin();
			//��¼�������Ϣ
			std::string current_ins;
			size_t current_ins_site;
			BgmCode current_ins_Code;
			//��¼����֮������ӷ�
			std::set<size_t> current_para_link_set;
			//ѭ������һ��Code
			//���ȷ��������
			for (const auto& sen_end : code_set) {
				//�����ָ��
				if (sen_end == sen_beg) {
					continue;
				}
				//����Ĳ���,��Ϊһ������ֻ����һ�������,�����if
				if (instruct_iter != instruct_map.cend() && instruct_iter->first < sen_end) {
					//һ�������ܳ������������,��˺����ϸ������������,Ҳ����˵ֻʹ��һ��ָ���г��ֵĵ�һ��ָ��
					while (instruct_iter->first < sen_beg) {
						++instruct_iter;
					}
					//��֪��ǰ�������ʼλ��
					current_ins_site = instruct_iter->first;
					//��֪��ǰ���������
					current_ins = (instruct_iter->second).first;
					//��֪��ǰ����ı�ʶ��
					current_ins_Code = (instruct_iter->second).second;
					////ѹ��ָ��ı�ʶ��
					//bgm_code.insert(current_ins_Code);
					//������һ��ָ��,�´�ѭ������
					if (instruct_iter != instruct_map.cend())
						++instruct_iter;
				}
				else {//�����ֻ��ID
					////�����Ų鵱ǰCode�Ƿ���Ч
					//bool valid = false;
					//for (auto i : id_unref) 
					//	if (this->find_first_of(i, sen_beg) != std::string::npos)
					//	{
					//		valid = true;
					//		break;
					//	}
					//if (!valid)
					//{
					//	//TODO
					//	//��ȷ�Ų�

					//	//���Ա������
					//	//�������¼
					//	current_ins_Code = BgmCode::Unknow;
					//	//ѹ��ָ��ı�ʶ��
					//	bgm_code.insert(current_ins_Code);
					//	continue;
					//}
					//�����ȱʧָ��,��site = -1
					current_ins_site = -1;
					current_ins = Default_Key;
					current_ins_Code = instruct.at(Default_Key);
					////ѹ��ָ��ı�ʶ��
					//bgm_code.insert(current_ins_Code);
				}

				//����֮������ӷ�����[ֻ��һ��char]
				//���¸���һ�����ӵķ�Χ��ϲ����ָ��������λ��,��λ�÷��䵽ÿһ�������
				while (para_link_iter != para_link_set.cend() && *para_link_iter < sen_end) {
					current_para_link_set.insert(*para_link_iter);
					if (para_link_iter != para_link_set.cend())
						++para_link_iter;
				}
#ifndef NDEBUG
				bangumi::string create_info;
				create_info >> "sen_beg: " << sen_beg + 1
					>> "sen_end: " << sen_end
					>> "current_ins: " << current_ins
					>> "current_ins_site: " << current_ins_site
					>> "current_para_link_set: ";
				DEBUG_STRING_SET(current_para_link_set, create_info);

				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-BGMCode-AnalyseCode-CreateCode", create_info);
#endif

				//Code(*this, first, cur_end);
				//�������м�������(beg+1����ΪҪ���˵�code���֮��ķָ���)
				param.code_pool.emplace_back(param, sen_beg + 1, sen_end,
					current_ins, current_ins_site, current_ins_Code, current_para_link_set);

				//������һ�����
				sen_beg = sen_end;

				//��ʼ��link_set
				current_para_link_set.clear();
			}
		}

	private:
		//����������
		BGMCodeParam &param;
		//�����Ǵ�����ص���ɲ��ֵĸ���ʶ��λ�õ�set/map��
		//Ϊ�˴������insertһ��ĩβ
		std::set<size_t> code_set{ this->length() };
		std::map<size_t, std::pair<std::string, BgmCode> > instruct_map;
		std::set<size_t> para_link_set;
		//������Ϣ����[����BGMCodeParam����]
		//BgmRetType ret_type;

		//����ص�����[����BGMCodeParam����]
		//std::list<Code>& code_pool;
		//std::set<BgmCode>& bgm_code;
		//const int64_t& qq;
		//const int64_t& group;

		//����set�������ݷָ�һ�����:
		//para1: ��Ҫʶ���set
		//para2: �洢ʶ����(λ��)��set��
		template<typename T>
		void Match(const T &identif_set, std::set<size_t>& ret_set) {

			for (const auto &is : identif_set) {
				size_t current_low = -1;
				while (current_low = (*this).find(is, current_low + 1), current_low != std::string::npos)
					ret_set.insert(current_low);
			}
		}
		//ָ���ʶ��
		void Match(const std::unordered_map<std::string, BgmCode> &identif_map, std::map<size_t, std::pair<std::string, BgmCode>>& ret_map) {

			for (const auto &is : identif_map) {
				size_t current_low = -1;
				while (current_low = (*this).find(is.first, current_low + 1), current_low != std::string::npos) {
					ret_map.insert({ current_low, is });
				}

			}
		}

	};


	class Code
	{
	public:
		//���캯��
		//Para:
		//first => �����Ϣ�Ĵ��������ʼλ��
		//second => �����Ϣ������Ľ���λ��
		//pa_li_site => ����λ��
		//instru_name => ������
		//instru_site => ����λ��
		Code(BGMCodeParam& Parameter, size_t first, size_t second,
			std::string instru_name, size_t instru_site, BgmCode icode_type, std::set<size_t> pa_li_site)
			:param(Parameter), beg(first), end(second),
			instruct_name(instru_name), instruct_site(instru_site),
			para_link_site(pa_li_site),code_type(icode_type)
		{
			//��ʼ������
			MakePara();
			UnPara();
		}
		//��ӡCode���
		bangumi::string Print() {

			bangumi::string ret = "\n------";
			ret >> param.str.substr(beg, end - beg)
				>> "ָ��: " << instruct_name
				<< "  λ��:(" << instruct_site << "," << ((instruct_name.length() + instruct_site) - 1) << ")"
				>> "�������ӷ�λ��(��������): ";
			for (const auto& i : para_link_site)
				ret << i << " ";

			ret >> "����: ";
			for (const auto& i : parameters)
				ret << i << " ";

			ret >> "�����(ID): ";
			for (const auto& i : unparameters_id)
				ret << i << " ";

			ret >> "�����(STR): ";
			for (const auto& i : unparameters_str)
				ret << i << " ";

			ret >> "------";
			ret << '\n';


			return ret;
		}

		//ִ��code
		inline void ExecuteCode() {
			//��Ϊһ��������Ӧ�Ĺؼ���,����Ҫ�쳣���
			//����code��Ӧ����ִ��
			//������Code����һ��Unkonw״̬
			if (code_type != BgmCode::Unknow)
				instruct_func_map.at(code_type)(param, unparameters_id, unparameters_str);
		}
	private:
		const size_t beg;
		const size_t end;
		const std::string instruct_name;
		//һ��Codeֻ����һ������
		const size_t instruct_site;
		//һ��Code�����ж������ //֮����������Codeָ���ռλ
		//�����ָ�λ�ü�
		std::set<size_t> para_link_site;
		//Code��Type
		BgmCode code_type;
		//[����BGMCodeParam����]
		////Msg��Type
		//BgmRetType msg_type;
		////bgmCode��
		//std::set<BgmCode>& bgm_code;
		////�û���qq
		//const int64_t &qq;
		////���Ե�Ⱥ��������
		//const int64_t &group;
		//const std::string& sentence;
		BGMCodeParam &param;
		//Code�뺯���Ķ�Ӧ��
		const static std::map<std::string, std::function<bangumi::string(int64_t, int64_t, std::string)>> code_map;

		//�洢��ǰ����Ĳ���
		std::list<std::string> parameters;
		//������Ĳ���
		std::set<size_t> unparameters_id;
		std::set<std::string> unparameters_str;
		//��bgm_code��ѹ�����Code������
		void InsertBgmCode() {

			param.bgm_code.insert(code_type);
		}
		//�������е���Ϣ������õĲ���,���洢��parameters��
		void MakePara() {
			//�����Ĳ���
			std::vector<std::pair<size_t, size_t>> params_sites;
			//�����ȱʧָ��default�Ļ�����
			if (instruct_site != -1) {
				//����ȱʧָ��
				for (auto i = 0; i != instruct_name.length(); ++i)
					para_link_site.insert(instruct_site + i);
			}
			else {//ȱʧָ��default

			}

			//���params_sites(�������ӷ���λ��)
			auto first = beg;
			//������Ҵ˵���Code�Ĳ������ӷ�
			for (auto i = beg; i < end; ++i) {
				//��һ��û�ҵ�����һ��pair��first
				if (para_link_site.find(i) == para_link_site.cend()) {
					first = i;
					//ֱ���ڶ����ҵ�
					while (para_link_site.find(i) == para_link_site.cend() && i != end) {
						++i;
					}
					if (first != i)
						params_sites.push_back({ first,i });
				}
				else {
					continue;
				}

			}
			//ǰ�ڴ������
			//��������parameter����Ӳ���
			for (const auto& p : params_sites)
				parameters.push_back(param.str.substr(p.first, p.second - p.first));

		}
		//��������
		//��ÿ���������н���
		void UnPara() {
			//�������ҪInt���͵�����
			if (instruct_no_need_int.find(instruct_name) == instruct_no_need_int.cend())
			{
				for (const auto& para : parameters) {
					//Ĭ��û��ʹ�ý�����
					try {
						//�����ж��Ƿ�ʹ���˲������
						for (const auto& i : id_unref) {
							if (para.find(i) != std::string::npos)
							{
								//��������APIFUNCTION��������
								unparameters_str.insert(para);
								throw std::invalid_argument("");
							}
						}
						//���û�н������Ϳ��ܷ�ת��Ϊint��
						//[ע��]:123def�ǿ���ת����123��
						size_t bgm_id = std::stol(para);
						unparameters_id.insert(bgm_id);
					}
					//���������һ���ַ�������
					catch (std::exception&) {
						//����Ƿ��ҵ��˽��������
						//bool decode_found = false;
						//���ʹ���˽��������
						//for (const auto& i : id_unref) {
						//	if (para.find(i) != std::string::npos)
						//	{
						//		//����ҵ�,�Թ�֮����ַ�������
						//		decode_found = true;

						//		//��������������������
						//		//���Ҳ�Ƿ��ض�Ӧ������Bangumi Subject ID
						//		size_t result_id = paramter_func_map.at(i)(i, para);
						//		//size_t result_id = 999;
						//		if (result_id != 0)
						//			unparameters_id.insert(result_id);
						//		break;
						//	}

						//}
						//if (!decode_found)
						//�ַ�������,�����ڸ��������������
						if (instruct_need_str.find(instruct_name) != instruct_need_str.cend()) {
							unparameters_str.insert(para);
						}
					}
				}


			}
			else {//ֻ��Ҫ�ַ������Ͳ���������
				for (const auto& para : parameters)
					unparameters_str.insert(para);
				//��bgm_code��ѹ�����Code������
				InsertBgmCode();
				//ֱ��return
				return;
			}

			//���û��int����,Ҳû��str����,����Ƿ���ʹ��last������
			if (unparameters_id.empty()&&unparameters_str.empty()) {
				//�����һ�����ܵ�ʹ��last����������
				//��ʱ����
				//if (instruct_may_use_last.find(instruct_name) == instruct_may_use_last.cend())
				//{
				//	//TODO
				//	//����õ�last subject id
				//	//��unparameters_id�в��� last id
				//}

				//����ر��subject����û�������
				//���lastҲû��˵����һ����Ч��Code
				if (code_type == BgmCode::Subject)
					//�������沽����û��һ��int���Ͳ�����Ϊδ֪������
					if (unparameters_id.empty()) {
						code_type = BgmCode::Unknow;
					}
			}
			//��bgm_code��ѹ�����Code������
			InsertBgmCode();
		}
	};

}
#endif
