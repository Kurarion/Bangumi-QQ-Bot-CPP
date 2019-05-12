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
//TODO:���Client�ĳ�ʱ��ʱ�رչ���(ʹ��asio�ļ�ʱ��)
static HTTPClient http_client;
//global variable 
//SQL
static SQLPool sql_pool;
//TimeWorker
TimeWorker time_worker(http_client.GetIOS());
//���쳣��,Ϊ�����request������
void HTTPRequest::RemoveSelf() {
	http_client.RemoveID(m_id);
}

//���ּ��滻����
const boost::bimap<char, char> num_bimap;

//Bot��ʼ��
void Init() {
	//CQ_getAppDirectory(ac)�ǿ�Q�ṩ��һ�����ܺ�������
	//��Q Pr/data/app/cc.sirokuma.Bangumi/
	bgm.Init(CQ_getAppDirectory(ac));
	//��ͣ1s
	boost::this_thread::sleep(boost::posix_time::seconds(1));
	sql_pool.Init(&bgm);
	http_client.Init(&bgm);
	//num_bimap��ʼ��
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
	
}

//һЩFUNC�õײ㺯��
//�ڽ���API��ȡUser��Ϣ֮ǰ,���ȫ��User���еı����Ƿ��Ѿ�����
inline bangumi::BangumiUser& BangumiPreFindUser(size_t user_id) {
#ifndef NDEBUG
	{
		bangumi::string debug_msg;
		debug_msg << "��API֮ǰԤ�Ȳ���BGMUser����(ֻ��ID������)";
		CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-BangumiPreFindUser", debug_msg);
	}
#endif

	return BGMUser.at(user_id);
}
//��User���й���һ��API��ȡ��User��Ϣ
inline bangumi::BangumiUser& BangumiAddUser(size_t &user_id, std::string &url,
	std::string &user_name, std::string &nick_name,
	std::string &ava_file, std::string &sign/*, bool refresh = false*/) {

	
		{
			//û������,ֱ�ӹ���
#ifndef NDEBUG
			{
				bangumi::string debug_msg;
				debug_msg << "BGMUser��û�����л���ҪRefresh,�¹���User��BGMUser��";
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-ADDBGMUser", debug_msg);
			}
#endif
			//��ӹ�����
			//��������������СС��ini���õ�����
			if (BGMUser.size() >= bgm.max_user_map)
			{
				//ֱ�����BGMUser����ʹ��
				BGMUser.clear();
			}

			//����earse�Ǹ�id�µ�Ԫ��
			BGMUser.erase(user_id);
			//����BangumiUser������
			return ((BGMUser.emplace(user_id, bangumi::BangumiUser{ user_id,url,user_name, nick_name, ava_file, sign })).first)->second;
			//��ΪͬʱҪ����Refresh�����,ֱ�ӹ��첻�Ḳ����ͬ��key��value
			//��������ʹ�õ�Ĭ�Ϲ��캯��
			//auto &new_user = BGMUser[user_id] = bangumi::BangumiUser{ user_id,url,user_name, nick_name, ava_file, sign };
			//return new_user;
		}

}
//�ڽ���API��ȡSubject��Ϣ֮ǰ,���ȫ��Subject���еı����Ƿ��Ѿ�����
inline bangumi::BangumiSubject& BangumiPreFindSubject(size_t subject_id) {
#ifndef NDEBUG
	{
		bangumi::string debug_msg;
		debug_msg << "��API֮ǰԤ�Ȳ���BGMSubject����";
		CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-BangumiPreFindSubject", debug_msg);
	}
#endif

	return BGMSubject.at(subject_id);
}
//�ڳ���û���ҵ�,ͨ��API��ȡ֮ǰ,��SQL�в��һ���
//ע��:����һ��������֮ǰ�Ƿ��Ѿ����ڵ�boolֵ
inline bangumi::BangumiSubject& BangumiSQLFindSubject(size_t subject_id) {


	//��ѯ���
	bangumi::string query;
	//SELECT * FROM bgm_subjects WHERE subject_id=1
	query << "SELECT * FROM bgm_subjects WHERE subject_id="
		<< subject_id;
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
			debug_msg << "SQLCheckResult��ѯʧ��:"
				<< std::to_string(affect_rows_num)
				>> e.what();

			CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-BangumiSQLFindSubject-SQLCheckResult", debug_msg);
		}
#endif	
		//ͨ���쳣��������
		throw e;
	}
	//����ҵ�,ֱ����BGMSubject��Add
	//�������ʹ��Refresh,�򲻻ᾭ�������Prefind����,Ҳ���ᵣ������ͬ��Key������޷�����
	//��Щ������Add�����д���
	if (affect_rows_num > 0) {
		//���������Ϊ0,�����

		//
		try {
#ifndef NDEBUG
			{
				bangumi::string debug_msg;
				debug_msg << "SQL��Prefind Subject";
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
			//��UTF8ת��ΪGBK
			//std::string name = boost::locale::conv::from_utf(result[3], "GBK");
			//std::string name_cn = boost::locale::conv::from_utf(result[4], "GBK");
			//std::string summary = boost::locale::conv::from_utf(result[5], "GBK");
			int eps = std::stoi(result[6]);
			std::string air_date_str = result[7];
			boost::gregorian::date air_date;
			if (air_date_str.compare("1000-01-01") != 0) {
				//���SQL����һ����Ч������,��ֵ
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
#ifndef NDEBUG
			{
				bangumi::string debug_msg;
				debug_msg << "SQL��Prefind Subject�б���ת��OK";
				CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-BangumiSQLFindSubject", debug_msg);
			}
#endif
			//��BGMSubject�й����SQL�еĻ���Subject
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
				collection })).first)->second;
		}
		catch (std::invalid_argument) {
			throw boost::system::system_error(bangumi_bot_errors::sql_result_convert_filed);
		}
	}
	else {
		//���������SQL����
		//ͨ���쳣��������
		throw boost::system::system_error(bangumi_bot_errors::subject_sql_miss);
	}

}
//��Subject���й���һ��API��ȡ��Subject��Ϣ,ͬʱҲ����SQL�и���Subject
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
	bangumi::Collection& collection
) {


	
	{
		//û������,ֱ�ӹ���
#ifndef NDEBUG
		{
			bangumi::string debug_msg;
			debug_msg << "BGMSubject��û������SQLû�����л���ҪRefresh,�¹���Subject��BGMSubject��";
			CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-ADDBGMSubject", debug_msg);
		}
#endif
		//��ӹ�����
		//��������������СС��ini���õ�����
		if (BGMSubject.size() >= bgm.max_subject_map)
		{
			//ֱ�����BGMUser����ʹ��
			BGMSubject.clear();
		}

		//����earse�Ǹ�id�µ�Ԫ��
		BGMSubject.erase(subject_id);
		//��SQL�и��»򴴽������Ŀ����Ϣ
		bangumi::string query;
		//ֻ�ǲ���Ͳ���Res��
		//BGMSQLResult res;

		std::string sql_image_file = image_file;
		replace(sql_image_file.begin(), sql_image_file.end(), '\\', '/');
		//REPLACE INTO���Subject
		query << "REPLACE INTO bgm_subjects VALUES("
			<< subject_id << ","
			<< "'"<<url << "',"
			<< type << ","
			<< "'" << name << "',"
			<< "'" << name_cn << "',"
			<< "'" << summary << "',"
			//��GBKת��ΪUTF-8
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
			<< collection.dropped << ")";
#ifndef NDEBUG
		{
			bangumi::string debug_msg;
			debug_msg << "Add Subject query";
			debug_msg >> query;
			debug_msg >> "ԭ��:" << code_converter.Conv(name)
				>> "������:" << code_converter.Conv(name_cn)
				>> "���:" << code_converter.Conv(summary);
			CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-BangumiADDSubject-SQL", debug_msg);
		}
#endif

		//������ԵĻ�,����һ���߳�ȥִ��
		//����һ���µĽ���
		if (bgm.CheckThreadSize()) {
			//�п��п��õĽ���
			std::shared_ptr<boost::thread> download_thread
			(new boost::thread([query]() {
#ifndef NDEBUG
				{
					std::ostringstream oss;
					oss << boost::this_thread::get_id();
					std::string idAsString = oss.str();
					std::string test = "�����µ��߳� ID: " + idAsString +
						"\n�ܳش�С: " + std::to_string(bgm.threadpool_size) + "\n"
						"���ô�С: " + std::to_string(bgm.curr_thread_size);
					CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Subject-SQL-MultiThread", test.c_str());
				}
#endif
				
				//�˴�����SQL����
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
						debug_msg << "Subject Insert Intoʧ��:"
							<< std::to_string(affect_rows_num)
							>> e.what();

						CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-BangumiAddSubject-SQLCheckResult-MultiTheard", debug_msg);
					}
#endif	
					//���ﲻ����
					//ͨ���쳣��������
					//throw e;
				}

				bgm.AddAVAThreadSize();
#ifndef NDEBUG
				{
					bangumi::string debug_str;
					debug_str << "Subject Insert Into�߳����";
					CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-Subject-SQL-MultiThread", debug_str);
				}
#endif
			}));
			//ֱ�ӷ����߳�
			download_thread->detach();
		}
		else {
			//û�п��õ��߳�,ͬ��ִ��
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
					debug_msg << "Subject Insert Intoʧ��:"
						<< std::to_string(affect_rows_num)
						>> e.what();

					CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-BangumiAddSubject-SQLCheckResult", debug_msg);
				}
#endif	
				//���ﲻ����
				//ͨ���쳣��������
				//throw e;
			}
		}
		
		//
		//����BangumiUser������
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
			collection })).first)->second;
		//��ΪͬʱҪ����Refresh�����,ֱ�ӹ��첻�Ḳ����ͬ��key��value

	}

}




#endif