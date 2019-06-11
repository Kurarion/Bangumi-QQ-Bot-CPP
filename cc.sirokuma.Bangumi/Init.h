//#include "GlobalVariable.h"
#include "Http.h"
#include "Database.h"
//#include "Resolve.h"
#include "TimeWorker.h"
#include <boost/bimap.hpp>
//#include <direct.h>
#ifndef BANGUMI_INIT_H
#define BANGUMI_INIT_H
//bangumi-bot init




//global variable 
//Http
//TODO:完成Client的超时暂时关闭功能(使用asio的计时器)
static HTTPClient http_client;
//global variable 
//SQL
static SQLPool sql_pool;
//TimeWorker
TimeWorker time_worker(http_client.GetIOS());
//在异常后,为了清除request的索引
void HTTPRequest::RemoveSelf() {
	http_client.RemoveID(m_id);
}

//数字简单替换加密
const boost::bimap<char, char> num_bimap;
//@me
static std::string at_me_cq;//在Init中初始化
static const std::string at_me_1 = "BGM娘";
static const std::string at_me_2 = "bgm娘";
static const bangumi::string test_passed = "恭喜通过全部考核！\n已解锁 dmhy 和 moe 指令~";
static const bangumi::string error_answer = "不对哦~\n重复一下问题：\n";
static const bangumi::string right_answer = "Bingo！\n接着下一个问题：\n";
static const bangumi::string question[] =
{
	"请@或喊[bgm娘]并回复\"OK\"来接受我的考核吧~",
	"[1]如何使用帮助功能？(提示：帮助指令为help)",
	"[2]如何使用标签查询查找标签为\"原创\"的2018年10月番的第二页的结果？",
	"[3]如果想要进行自身使用统计并强制缓存刷新式查询Bangumi ID为 1 和 92981 的用户应当如何输入指令？",
	"[4]如果想要查询动画《一拳超人》的第二季的信息的放送状态与标签应当如何使用一个指令和一个参数完成？\n（注意：禁止使用上一次查询条目的空缺参数）",
	"[5]如果想要搜索关键字为\"英雄\"全部类型的从第3个条目开始的3个条目应当如何使用一个指令和一个参数完成？",
	"[6]如果想要更新上次使用的条目进度为全部完成并只吐槽\"我不会\"应当如何使用一个指令和一个参数完成？",
	"[7]如果想要更新数据库中关键字为\"Angel Beats\"的第二个条目的进度为当前放送进度的上一话应当如何使用一个指令和一个参数完成？",
	"[8]如果想要收藏Bangumi中关键字为\"rewrite\"的游戏类型的第二个条目为玩过，评分9分并吐槽\"GoodJob\"应当如何使用一个指令和一个参数完成？"
};
static bangumi::ComplexParam standard_answer[9];
//Bot初始化
void Init() {
	//CQ_getAppDirectory(ac)是酷Q提供的一个功能函数返回
	//酷Q Pr/data/app/cc.sirokuma.Bangumi/
	bgm.Init(CQ_getAppDirectory(ac));
	//暂停1s
	boost::this_thread::sleep(boost::posix_time::seconds(1));
	sql_pool.Init(&bgm);
	http_client.Init(&bgm);
	//num_bimap初始化
	auto x = num_bimap.left;

	x.insert({ '0','0' });
	x.insert({ '1','1' });
	x.insert({ '2','2' });
	x.insert({ '3','3' });
	x.insert({ '4','4' });
	x.insert({ '5','5' });
	x.insert({ '6','6' });
	x.insert({ '7','7' });
	x.insert({ '8','8' });
	x.insert({ '9','9' });

	at_me_cq = "[CQ:at,qq=" + std::to_string(CQ_getLoginQQ(ac)) + "]";
	
	//问题答案的初始化
	//2
	standard_answer[2].tag_keyword = "原创";
	standard_answer[2].tag_airtime = "2018-10";
	standard_answer[2].tag_page = 2;
	//4
	standard_answer[4].id = 193619;
	standard_answer[4].add_air_status = true;
	standard_answer[4].add_tag = true;
	standard_answer[4].single = true;
	//5
	standard_answer[5].search_max_num = 3;
	standard_answer[5].search_start_pos = 2;
	standard_answer[5].str = "英雄";
	standard_answer[5].search_type = 0;
	//6
	standard_answer[6].use_last_subject_id = true;
	standard_answer[6].collection_comment = "我不会";
	standard_answer[6].update_fin = true;
	//7
	standard_answer[7].update_air = true;
	standard_answer[7].update_eps_shift = -1;
	//8
	standard_answer[8].collection_status = "collect";
	standard_answer[8].collection_rating = 9;
	standard_answer[8].collection_comment = "GoodJob";
}

//一些FUNC用底层函数
//在进行API获取User信息之前,检查全局User池中的变量是否已经存在
inline bangumi::BangumiUser& BangumiPreFindUser(size_t user_id) {
#ifndef NDEBUG
	{
		bangumi::string debug_msg;
		debug_msg << "在API之前预先查找BGMUser缓存(只限ID是数字)";
		CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-BangumiPreFindUser", debug_msg);
	}
#endif

	return BGMUser.at(user_id);
}
//向User池中构造一个API获取的User信息
inline bangumi::BangumiUser& BangumiAddUser(size_t &user_id, std::string &url,
	std::string &user_name, std::string &nick_name,
	std::string &ava_file, std::string &sign/*, bool refresh = false*/) {

		{
			//没有命中,直接构造
#ifndef NDEBUG
			{
				bangumi::string debug_msg;
				debug_msg << "BGMUser中没有命中或需要Refresh,新构造User到BGMUser中";
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-ADDBGMUser", debug_msg);
			}
#endif
			//添加构造检查
			//暴力限制容器大小小于ini配置的数字
			if (BGMUser.size() >= bgm.max_user_map)
			{
				//直接清空BGMUser重新使用
				BGMUser.clear();
			}

			//首先earse那个id下的元素
			BGMUser.erase(user_id);
			//返回BangumiUser的引用
			return ((BGMUser.emplace(user_id, bangumi::BangumiUser{ user_id,url,user_name, nick_name, ava_file, sign })).first)->second;
			//因为同时要处理Refresh的情况,直接构造不会覆盖相同的key的value
			//以下语句会使用到默认构造函数
			//auto &new_user = BGMUser[user_id] = bangumi::BangumiUser{ user_id,url,user_name, nick_name, ava_file, sign };
			//return new_user;
		}

}
//在进行API获取Subject信息之前,检查全局Subject池中的变量是否已经存在
inline bangumi::BangumiSubject& BangumiPreFindSubject(size_t subject_id) {
#ifndef NDEBUG
	{
		bangumi::string debug_msg;
		debug_msg << "在API之前预先查找BGMSubject缓存";
		CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-BangumiPreFindSubject", debug_msg);
	}
#endif

	return BGMSubject.at(subject_id);
}
//在池中没有找到,通过API获取之前,从SQL中查找缓存
//注意:返回一个索引和之前是否已经存在的bool值
inline bangumi::BangumiSubject& BangumiSQLFindSubject(size_t subject_id) {


	//查询语句
	bangumi::string query;
	//SELECT * FROM bgm_subjects WHERE subject_id=1
	query << "SELECT * FROM bgm_subjects WHERE subject_id="
		<< subject_id;
	//查询结果
	BGMSQLResult result;
	//影响的行数
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
			debug_msg << "SQLCheckResult查询失败:"
				<< std::to_string(affect_rows_num)
				>> e.what();

			CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-BangumiSQLFindSubject-SQLCheckResult", debug_msg);
		}
#endif	
		//通过异常结束函数
		throw e;
	}
	//如果找到,直接向BGMSubject中Add
	//由于如果使用Refresh,则不会经过这个和Prefind函数,也不会担心有相同的Key而造成无法覆盖
	//这些都会在Add函数中处理
	if (affect_rows_num > 0) {
		//如果行数不为0,则存在
		//首先检查SQL中score_max是否为0,0则强制刷新
		if (result[28][0] == '0')
		{
			//通过异常结束函数
			throw boost::system::system_error(bangumi_bot_errors::subject_sql_miss);
		}
		//
		try {
#ifndef NDEBUG
			{
				bangumi::string debug_msg;
				debug_msg << "SQL中Prefind Subject";
				for (int i = 0; i < 18; ++i) {
					debug_msg >> std::to_string(i) << ": " << result[i];
				}
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-BangumiSQLFindSubject", debug_msg);
			}
#endif

			size_t sql_subject_id = std::stoul(result[0]);
			std::string url = result[1];
			int type = std::stoi(result[2]);
			std::string name = result[3];
			std::string name_cn = result[4];
			std::string summary = result[5];
			//boost::locale::conv::from_utf("", "GBK");
			//从UTF8转换为GBK
			//std::string name = boost::locale::conv::from_utf(result[3], "GBK");
			//std::string name_cn = boost::locale::conv::from_utf(result[4], "GBK");
			//std::string summary = boost::locale::conv::from_utf(result[5], "GBK");
			int eps = std::stoi(result[6]);
			std::string air_date_str = result[7];
			boost::gregorian::date air_date;
			if (air_date_str.compare("1000-01-01") != 0) {
				//如果SQL中是一个有效的日期,则赋值
				air_date = boost::gregorian::from_string(air_date_str);
			}
			int air_weekday = std::stoi(result[8]);
			int rating_num = std::stoi(result[9]);
			float rating_score = std::stof(result[10]);
			int rank = std::stoi(result[11]);
			std::string image_file = result[12];
			bangumi::Collection collection;
			collection.wish = std::stoi(result[13]);
			collection.collect = std::stoi(result[14]);
			collection.doing = std::stoi(result[15]);
			collection.on_hold = std::stoi(result[16]);
			collection.dropped = std::stoi(result[17]);
			collection.type = type;
			int detail_score[11];
			detail_score[1] = std::stoi(result[18]);
			detail_score[2] = std::stoi(result[19]);
			detail_score[3] = std::stoi(result[20]);
			detail_score[4] = std::stoi(result[21]);
			detail_score[5] = std::stoi(result[22]);
			detail_score[6] = std::stoi(result[23]);
			detail_score[7] = std::stoi(result[24]);
			detail_score[8] = std::stoi(result[25]);
			detail_score[9] = std::stoi(result[26]);
			detail_score[10] = std::stoi(result[27]);
			detail_score[0] = std::stoi(result[28]);
#ifndef NDEBUG
			{
				bangumi::string debug_msg;
				debug_msg << "SQL中Prefind Subject中变量转换OK";
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-BangumiSQLFindSubject", debug_msg);
			}
#endif
			//向BGMSubject中构造从SQL中的缓存Subject
			return ((BGMSubject.emplace(sql_subject_id, bangumi::BangumiSubject{
				sql_subject_id,
				url,
				type,
				name,
				name_cn,
				summary,
				eps,
				air_date,
				air_weekday,
				rating_num,
				rating_score,
				rank,
				image_file,
				collection,
				detail_score })).first)->second;
		}
		catch (std::invalid_argument) {
			throw boost::system::system_error(bangumi_bot_errors::sql_result_convert_filed);
		}
	}
	else {
		//不存在这个SQL缓存
		//通过异常结束函数
		throw boost::system::system_error(bangumi_bot_errors::subject_sql_miss);
	}

}
//向Subject池中构造一个API获取的Subject信息,同时也会向SQL中更新Subject
inline bangumi::BangumiSubject& BangumiAddSubject(
	size_t &subject_id,
	std::string& url,
	int& type,
	std::string& name,
	std::string& name_cn,
	std::string& summary,
	int& eps,
	boost::gregorian::date& air_date,
	int& air_weekday,
	int& rating_num,
	float& rating_score,
	int& rank,
	std::string& image_file,
	bangumi::Collection& collection,
	int detail_score[11]
) {


		{
		//没有命中,直接构造
#ifndef NDEBUG
		{
			bangumi::string debug_msg;
			debug_msg << "BGMSubject中没有命中SQL没有命中或需要Refresh,新构造Subject到BGMSubject中";
			CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-ADDBGMSubject", debug_msg);
		}
#endif
		//添加构造检查
		//暴力限制容器大小小于ini配置的数字
		if (BGMSubject.size() >= bgm.max_subject_map)
		{
			//直接清空BGMUser重新使用
			BGMSubject.clear();
		}

		//首先earse那个id下的元素
		BGMSubject.erase(subject_id);
		//向SQL中更新或创建这个条目的信息
		bangumi::string query;
		//只是插入就不用Res了
		//BGMSQLResult res;

		std::string sql_image_file = image_file;
		replace(sql_image_file.begin(), sql_image_file.end(), '\\', '/');
		//REPLACE INTO这个Subject
		query << "REPLACE INTO bgm_subjects VALUES("
			<< subject_id << ","
			<< "'" << url << "',"
			<< type << ","
			<< "'" << name << "',"
			<< "'" << name_cn << "',"
			<< "'" << summary << "',"
			//从GBK转换为UTF-8
			//<< "'" << boost::locale::conv::between(name, "UTF-8", "GBK") << "',"
			//<< "'" << boost::locale::conv::between(name_cn, "UTF-8", "GBK") << "',"
			//<< "'" << boost::locale::conv::between(summary, "UTF-8", "GBK") << "',"
			<< eps << ","
			<< "'" << (air_date == boost::gregorian::date() ? "1000-01-01" : boost::gregorian::to_iso_extended_string(air_date)) << "',"
			<< air_weekday << ","
			<< rating_num << ","
			<< rating_score << ","
			<< rank << ","
			<< "'" << sql_image_file << "',"
			<< collection.wish << ","
			<< collection.collect << ","
			<< collection.doing << ","
			<< collection.on_hold << ","
			<< collection.dropped << ","
			<< detail_score[1] << ","
			<< detail_score[2] << ","
			<< detail_score[3] << ","
			<< detail_score[4] << ","
			<< detail_score[5] << ","
			<< detail_score[6] << ","
			<< detail_score[7] << ","
			<< detail_score[8] << ","
			<< detail_score[9] << ","
			<< detail_score[10] << ","
			<< detail_score[0] << ")";
#ifndef NDEBUG
		{
			bangumi::string debug_msg;
			debug_msg << "Add Subject query";
			debug_msg >> query;
			debug_msg >> "原名:" << code_converter.Conv(name)
				>> "中文名:" << code_converter.Conv(name_cn)
				>> "简介:" << code_converter.Conv(summary);
			CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-BangumiADDSubject-SQL", debug_msg);
		}
#endif

		//如果可以的话,开启一个线程去执行
		//创建一个新的进程
		if (bgm.CheckThreadSize()) {
			//有空闲可用的进程
			std::shared_ptr<boost::thread> download_thread
			(new boost::thread([query]() {
#ifndef NDEBUG
				{
					std::ostringstream oss;
					oss << boost::this_thread::get_id();
					std::string idAsString = oss.str();
					std::string test = "开启新的线程 ID: " + idAsString +
						"\n总池大小: " + std::to_string(bgm.threadpool_size) + "\n"
						"可用大小: " + std::to_string(bgm.curr_thread_size);
					CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Subject-SQL-MultiThread", test.c_str());
				}
#endif
				
				//此处进行SQL插入
				auto affect_rows_num = sql_pool.ExecQueryNoRes(query);
				try
				{
					SQLCheckResult(affect_rows_num);
				}
				catch (boost::system::system_error& e)
				{
#ifndef NDEBUG
					{
						bangumi::string debug_msg;
						debug_msg << "Subject Insert Into失败:"
							<< std::to_string(affect_rows_num)
							>> e.what();

						CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-BangumiAddSubject-SQLCheckResult-MultiTheard", debug_msg);
					}
#endif	
					//这里不结束
					//通过异常结束函数
					//throw e;
				}

				bgm.AddAVAThreadSize();
#ifndef NDEBUG
				{
					bangumi::string debug_str;
					debug_str << "Subject Insert Into线程完成";
					CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Subject-SQL-MultiThread", debug_str);
				}
#endif
			}));
			//直接分离线程
			download_thread->detach();
		}
		else {
			//没有可用的线程,同步执行
			auto affect_rows_num = sql_pool.ExecQueryNoRes(query);
			try
			{
				SQLCheckResult(affect_rows_num);
			}
			catch (boost::system::system_error& e)
			{
#ifndef NDEBUG
				{
					bangumi::string debug_msg;
					debug_msg << "Subject Insert Into失败:"
						<< std::to_string(affect_rows_num)
						>> e.what();

					CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-BangumiAddSubject-SQLCheckResult", debug_msg);
				}
#endif	
				//这里不结束
				//通过异常结束函数
				//throw e;
			}
		}
		
		//
		//返回BangumiUser的引用
		return ((BGMSubject.emplace(subject_id, bangumi::BangumiSubject{
			subject_id,
			url,
			type,
			name,
			name_cn,
			summary,
			eps,
			air_date,
			air_weekday,
			rating_num,
			rating_score,
			rank,
			image_file,
			collection,
			detail_score })).first)->second;
		//因为同时要处理Refresh的情况,直接构造不会覆盖相同的key的value

	}

}

//https://stackoverflow.com/questions/154536/encode-decode-urls-in-c
//urlencode
std::string url_encode(const std::string &value) {
	std::ostringstream escaped;
	escaped.fill('0');
	escaped << std::hex;

	for (std::string::const_iterator i = value.begin(), n = value.end(); i != n; ++i) {
		std::string::value_type c = (*i);

		// Keep alphanumeric and other accepted characters intact
		if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~' || c == ':' || c == '/' || c == '?' || c == '=') {
			escaped << c;
			continue;
		}

		// Any other characters are percent-encoded
		escaped << std::uppercase;
		escaped << '%' << std::setw(2) << int((unsigned char)c);
		escaped << std::nouppercase;
	}

	return escaped.str();
}


std::string GetHtml(std::string uri, std::string host) {
	std::string request = "GET " + uri + " HTTP/1.1\r\n"
		"Host: " + host + "\r\n" "\r\n";
	try {
		//std::string html = boost::locale::conv::from_utf(http_client.SyncBGMHTTPRequest(request), "GBK");
		std::string html = http_client.SyncBGMHTTPRequest(request);

		return std::move(html);
	}
	catch (boost::system::system_error&) {
		return "";
	}
}

#endif