#pragma once
#include "GlobalVariable.h"
#include "boost/asio.hpp"
#include "boost/thread.hpp"
using namespace boost;
//主要用于计时任务的处理
//使用asio的超时回调函数


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
			//从timer_list中删除自身
			timer_list.remove(timer);
		});
	}
	//取得当前任务队列的数量
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
