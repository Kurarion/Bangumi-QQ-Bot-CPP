#pragma once
#include "GlobalVariable.h"
#include "boost/asio.hpp"
#include "boost/thread.hpp"
using namespace boost;
//��Ҫ���ڼ�ʱ����Ĵ���
//ʹ��asio�ĳ�ʱ�ص�����


class TimeWorker {

public:
	TimeWorker(asio::io_service& ios)
		:m_ios(ios)
		//m_timer(new asio::steady_timer(ios, boost::asio::chrono::seconds(15)))
	{}

	//
	void AddAPIFunc(std::function<void(const bangumi::BGMCodeParam &, const std::set<size_t>&, const std::set<std::string>&)> func,
		const bangumi::BGMCodeParam param,
		const std::set<size_t> parameters_id,
		const std::set<std::string> parameters_str,
		unsigned countdown) {

		std::shared_ptr<asio::steady_timer> timer(new asio::steady_timer(m_ios));
		timer_list.emplace_back(timer);
		timer->expires_from_now(boost::asio::chrono::minutes(countdown));
		timer->async_wait([=](const boost::system::error_code&) {
			
			func(param, parameters_id, parameters_str);
			//��timer_list��ɾ������
			timer_list.remove(timer);
		});
	}
	//ȡ�õ�ǰ������е�����
	int GetCurrentListNum() {
		return timer_list.size();
	}
	//
	void Print() {
		bangumi::string print;
		print << ">>>>List<<<<";
		int num = 0;
		for (auto x : timer_list) {
			print >> std::to_string(num)<< ": *";
			++num;
		}
		print >> ">>>>List End<<<<";
#ifndef NDEBUG
		{
			bangumi::string debug_msg;
			debug_msg << "Time Worker Print!"
				>> print;
			CQ_addLog(ac, CQLOG_DEBUG, "Bangumi-Bot-TimeWorker", debug_msg);
		}
#endif	
	}

private:
	asio::io_service& m_ios;
	//std::shared_ptr<asio::steady_timer> m_timer;
	std::list<std::shared_ptr<asio::steady_timer>> timer_list;
};
