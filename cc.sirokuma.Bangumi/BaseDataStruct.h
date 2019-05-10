#include "BangumiInfo.h"
#include "ParserElements.h"
#include <string>
#include <vector>

#ifndef BASEDATASTRUCT_H
#define BASEDATASTRUCT_H
#ifndef NDEBUG
//debug字符串宏
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
		//	//转换为小写
		//	std::transform(this->begin(), this->end(), this->begin(), ::tolower);
		//}
		BGMCode(BGMCodeParam& parameter)
			:std::string(parameter.str),param(parameter)
		{
			//转换为小写
			std::transform(this->begin(), this->end(), this->begin(), ::tolower);
		}
		//匹配本次信息
		//生成Code类并放入Code池中

		void AnalyseCode() {

			//匹配各个命令
			Match(code_link, code_set);
#ifndef NDEBUG
			bangumi::string debug_code_set = "CodeSet: ";
			DEBUG_STRING_SET(code_set, debug_code_set);
			CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-BGMCode-AnalyseCode", debug_code_set);
#endif
			//匹配所有的命令标识符
			Match(instruct, instruct_map);
#ifndef NDEBUG
			bangumi::string debug_instruct_map = "InstructMap: ";
			DEBUG_STRING_MAP(instruct_map, debug_instruct_map);
			CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-BGMCode-AnalyseCode", debug_instruct_map);
#endif
			//匹配参数和参数之间的连接符
			Match(para_link, para_link_set);
#ifndef NDEBUG
			bangumi::string debug_para_link_set = "ParaLinkSet: ";
			DEBUG_STRING_SET(para_link_set, debug_para_link_set);
			CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-BGMCode-AnalyseCode", debug_para_link_set);
#endif
			//经过以上步骤后
			//便可知所有的关键字的起始位置以及命令及起始位置
			//下面具体分割每个指令

			size_t sen_beg = 0;
			//两个迭代器
			//指令的
			auto instruct_iter = instruct_map.cbegin();
			//参数的
			auto para_link_iter = para_link_set.cbegin();
			//记录命令的信息
			std::string current_ins;
			size_t current_ins_site;
			BgmCode current_ins_Code;
			//记录参数之间的连接符
			std::set<size_t> current_para_link_set;
			//循环处理一个Code
			//首先分析命令符
			for (const auto& sen_end : code_set) {
				//如果空指令
				if (sen_end == sen_beg) {
					continue;
				}
				//命令的参数,因为一个命令只能有一个命令符,因此用if
				if (instruct_iter != instruct_map.cend() && instruct_iter->first < sen_end) {
					//一个语句可能出现两个命令符,借此忽略上个语句多出的命令,也就是说只使用一个指令中出现的第一个指令
					while (instruct_iter->first < sen_beg) {
						++instruct_iter;
					}
					//得知当前命令的起始位置
					current_ins_site = instruct_iter->first;
					//得知当前命令的名字
					current_ins = (instruct_iter->second).first;
					//得知当前命令的标识符
					current_ins_Code = (instruct_iter->second).second;
					////压入指令的标识符
					//bgm_code.insert(current_ins_Code);
					//递增下一个指令,下次循环处理
					if (instruct_iter != instruct_map.cend())
						++instruct_iter;
				}
				else {//情况是只有ID
					////粗略排查当前Code是否有效
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
					//	//精确排查

					//	//忽略本条语句
					//	//并存入记录
					//	current_ins_Code = BgmCode::Unknow;
					//	//压入指令的标识符
					//	bgm_code.insert(current_ins_Code);
					//	continue;
					//}
					//如果是缺失指令,则site = -1
					current_ins_site = -1;
					current_ins = Default_Key;
					current_ins_Code = instruct.at(Default_Key);
					////压入指令的标识符
					//bgm_code.insert(current_ins_Code);
				}

				//参数之间的连接符参数[只有一个char]
				//以下根据一个句子的范围结合参数分割符的所有位置,将位置分配到每一个语句中
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
				//向代码池中加入命令(beg+1是因为要过滤掉code语句之间的分隔符)
				param.code_pool.emplace_back(param, sen_beg + 1, sen_end,
					current_ins, current_ins_site, current_ins_Code, current_para_link_set);

				//处理下一条语句
				sen_beg = sen_end;

				//初始化link_set
				current_para_link_set.clear();
			}
		}

	private:
		//参数的引用
		BGMCodeParam &param;
		//以下是代码相关的组成部分的各个识别位置的set/map集
		//为了处理方便多insert一个末尾
		std::set<size_t> code_set{ this->length() };
		std::map<size_t, std::pair<std::string, BgmCode> > instruct_map;
		std::set<size_t> para_link_set;
		//返回消息类型[改用BGMCodeParam管理]
		//BgmRetType ret_type;

		//命令池的引用[改用BGMCodeParam管理]
		//std::list<Code>& code_pool;
		//std::set<BgmCode>& bgm_code;
		//const int64_t& qq;
		//const int64_t& group;

		//根据set集中内容分割一个语句:
		//para1: 需要识别的set
		//para2: 存储识别结果(位置)的set集
		template<typename T>
		void Match(const T &identif_set, std::set<size_t>& ret_set) {

			for (const auto &is : identif_set) {
				size_t current_low = -1;
				while (current_low = (*this).find(is, current_low + 1), current_low != std::string::npos)
					ret_set.insert(current_low);
			}
		}
		//指令的识别
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
		//构造函数
		//Para:
		//first => 这个消息的此命令的起始位置
		//second => 这个消息此命令的结束位置
		//pa_li_site => 参数位置
		//instru_name => 命令名
		//instru_site => 命令位置
		Code(BGMCodeParam& Parameter, size_t first, size_t second,
			std::string instru_name, size_t instru_site, BgmCode icode_type, std::set<size_t> pa_li_site)
			:param(Parameter), beg(first), end(second),
			instruct_name(instru_name), instruct_site(instru_site),
			para_link_site(pa_li_site),code_type(icode_type)
		{
			//初始化参数
			MakePara();
			UnPara();
		}
		//打印Code结果
		bangumi::string Print() {

			bangumi::string ret = "\n------";
			ret >> param.str.substr(beg, end - beg)
				>> "指令: " << instruct_name
				<< "  位置:(" << instruct_site << "," << ((instruct_name.length() + instruct_site) - 1) << ")"
				>> "参数连接符位置(包括命令): ";
			for (const auto& i : para_link_site)
				ret << i << " ";

			ret >> "参数: ";
			for (const auto& i : parameters)
				ret << i << " ";

			ret >> "解参数(ID): ";
			for (const auto& i : unparameters_id)
				ret << i << " ";

			ret >> "解参数(STR): ";
			for (const auto& i : unparameters_str)
				ret << i << " ";

			ret >> "------";
			ret << '\n';


			return ret;
		}

		//执行code
		inline void ExecuteCode() {
			//因为一定存在相应的关键字,不需要异常检测
			//调用code对应函数执行
			//仅仅当Code不是一个Unkonw状态
			if (code_type != BgmCode::Unknow)
				instruct_func_map.at(code_type)(param, unparameters_id, unparameters_str);
		}
	private:
		const size_t beg;
		const size_t end;
		const std::string instruct_name;
		//一个Code只能有一个命令
		const size_t instruct_site;
		//一个Code可以有多个参数 //之后处理会添加上Code指令的占位
		//参数分隔位置集
		std::set<size_t> para_link_site;
		//Code的Type
		BgmCode code_type;
		//[改用BGMCodeParam管理]
		////Msg的Type
		//BgmRetType msg_type;
		////bgmCode池
		//std::set<BgmCode>& bgm_code;
		////用户的qq
		//const int64_t &qq;
		////来自的群或讨论组
		//const int64_t &group;
		//const std::string& sentence;
		BGMCodeParam &param;
		//Code与函数的对应表
		const static std::map<std::string, std::function<bangumi::string(int64_t, int64_t, std::string)>> code_map;

		//存储当前命令的参数
		std::list<std::string> parameters;
		//解析后的参数
		std::set<size_t> unparameters_id;
		std::set<std::string> unparameters_str;
		//向bgm_code中压入这个Code的类型
		void InsertBgmCode() {

			param.bgm_code.insert(code_type);
		}
		//根据已有的信息抽出有用的参数,并存储到parameters中
		void MakePara() {
			//遍历的参数
			std::vector<std::pair<size_t, size_t>> params_sites;
			//如果是缺失指令default的话跳过
			if (instruct_site != -1) {
				//不是缺失指令
				for (auto i = 0; i != instruct_name.length(); ++i)
					para_link_site.insert(instruct_site + i);
			}
			else {//缺失指令default

			}

			//填充params_sites(参数连接符的位置)
			auto first = beg;
			//逐个查找此单个Code的参数连接符
			for (auto i = beg; i < end; ++i) {
				//第一个没找到的是一个pair的first
				if (para_link_site.find(i) == para_link_site.cend()) {
					first = i;
					//直到第二个找到
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
			//前期处理完成
			//接下来向parameter中添加参数
			for (const auto& p : params_sites)
				parameters.push_back(param.str.substr(p.first, p.second - p.first));

		}
		//解析参数
		//对每个参数进行解析
		void UnPara() {
			//如果是需要Int类型的命令
			if (instruct_no_need_int.find(instruct_name) == instruct_no_need_int.cend())
			{
				for (const auto& para : parameters) {
					//默认没有使用解析符
					try {
						//首先判断是否使用了参数解符
						for (const auto& i : id_unref) {
							if (para.find(i) != std::string::npos)
							{
								//解析交由APIFUNCTION函数处理
								unparameters_str.insert(para);
								throw std::invalid_argument("");
							}
						}
						//如果没有解析符就看能否转换为int了
						//[注意]:123def是可以转换成123的
						size_t bgm_id = std::stol(para);
						unparameters_id.insert(bgm_id);
					}
					//若本身就是一个字符串参数
					catch (std::exception&) {
						//标记是否找到了解析运算符
						//bool decode_found = false;
						//如果使用了解析运算符
						//for (const auto& i : id_unref) {
						//	if (para.find(i) != std::string::npos)
						//	{
						//		//标记找到,略过之后的字符串参数
						//		decode_found = true;

						//		//解析交由其他函数处理
						//		//结果也是返回对应参数的Bangumi Subject ID
						//		size_t result_id = paramter_func_map.at(i)(i, para);
						//		//size_t result_id = 999;
						//		if (result_id != 0)
						//			unparameters_id.insert(result_id);
						//		break;
						//	}

						//}
						//if (!decode_found)
						//字符串参数,仅仅在个别的命令中有用
						if (instruct_need_str.find(instruct_name) != instruct_need_str.cend()) {
							unparameters_str.insert(para);
						}
					}
				}


			}
			else {//只需要字符串类型参数的命令
				for (const auto& para : parameters)
					unparameters_str.insert(para);
				//向bgm_code中压入这个Code的类型
				InsertBgmCode();
				//直接return
				return;
			}

			//如果没有int参数,也没有str参数,检查是否是使用last的命令
			if (unparameters_id.empty()&&unparameters_str.empty()) {
				//如果是一个可能的使用last参数的命令
				//暂时不用
				//if (instruct_may_use_last.find(instruct_name) == instruct_may_use_last.cend())
				//{
				//	//TODO
				//	//请求得到last subject id
				//	//向unparameters_id中插入 last id
				//}

				//针对特别的subject可以没有命令符
				//如果last也没有说明是一个无效的Code
				if (code_type == BgmCode::Subject)
					//经过上面步骤仍没有一个int类型参数则为未知的命令
					if (unparameters_id.empty()) {
						code_type = BgmCode::Unknow;
					}
			}
			//向bgm_code中压入这个Code的类型
			InsertBgmCode();
		}
	};

}
#endif
