#pragma once
#include "boost/exception/all.hpp"
#include "boost/thread/thread.hpp"
#include "boost/thread/mutex.hpp"
#include <exception>

//定义机器人的错误代码
//常使用在HTTP中
namespace bangumi_bot_errors {
	enum bot_error_codes
	{
		http_error = 1,
		without_url_in_client = 2,
		dns_resolve_error_in_client = 3,
		empty_request = 4,
		bad_request_message = 5,
		empty_callback = 6,
		sync_http_dns_connect_error = 7,
		sync_http_dns_resolve_error = 8,
		sync_http_sokect_open_filed = 9,
		bad_response_header = 10,
		pic_download_with_unknown_url = 11,
		pic_download_with_https = 12,
		json_resolve_error = 13,
		empty_bgm_ret_param = 14,
		auth_error = 15,
		sql_connect_error = 16,
		sql_query_filed = 17,
		sql_result_convert_filed = 18,
		http_get_contents_error = 19,
		api_301_error = 20,
		no_error = 21,
		invalid_param = 22

	};
	enum bgm_error_codes
	{
		empty_user = 999,
		empty_subject = 998,
		subject_sql_miss = 997,
		auth_request_error = 996,
		auth_without_same_id = 995,
		user_not_collect_this_subject = 994,
		access_token_invalid = 993,
		search_failed = 992,
		subject_collect_failed = 991,
		you_need_bind_your_bgm_id = 990,
		maybe_301_maybe_limit = 899,
		update_with_the_same_eps = 898,
		net_error = 897
	};

	class bot_errors_category
		:public boost::system::error_category
	{
		//重写两个虚函数
	public:
		const char* name() const BOOST_SYSTEM_NOEXCEPT {
			return "Bot_Http_Error";
		}

		std::string message(int e)const {
			switch (e)
			{
			case bangumi_bot_errors::http_error:
				return "Bot Error in Http!";
				break;
			case bangumi_bot_errors::without_url_in_client:
				return "Bot HTTPClient don't has a url to resolve!";
				break;
			case bangumi_bot_errors::dns_resolve_error_in_client:
				return "Bot dns resolve errored!";
				break;
			case bangumi_bot_errors::empty_request:
				return "Bot has an empty request!";
				break;
			case bangumi_bot_errors::bad_request_message:
				return "Bot has an error request!";
				break;
			case bangumi_bot_errors::empty_callback:
				return "Bot has an empty callback function to exec on_finish!";
				break;
			case bangumi_bot_errors::empty_bgm_ret_param:
				return "Bot has an empty BGMRetParam to exec on_finish!";
				break;
			case bangumi_bot_errors::sync_http_dns_connect_error:
				return "Bot sync connect a ip resolved error!";
				break;
			case bangumi_bot_errors::sync_http_dns_resolve_error:
				return "Bot sync dns resolve errored!";
				break;
			case bangumi_bot_errors::sync_http_sokect_open_filed:
				return "Bot sync socket open filed!";
				break;
			case bangumi_bot_errors::bad_response_header:
				return "Bot has an error response!";
				break;
			case bangumi_bot_errors::pic_download_with_unknown_url:
				return "Bot has an unknow download url!";
				break;
			case bangumi_bot_errors::pic_download_with_https:
				return "Bot has an https download request!";
				break;
			case bangumi_bot_errors::json_resolve_error:
				return "Bot json resolve occurred error!";
				break;
			case bangumi_bot_errors::auth_error:
				return "Bot Oauth2.0 post error!";
				break;
			case bangumi_bot_errors::sql_connect_error:
				return "Bot SQL connected error";
				break;
			case bangumi_bot_errors::sql_query_filed:
				return "Bot SQL query filed!";
				break;
			case bangumi_bot_errors::sql_result_convert_filed:
				return "Bot SQL query result convert filed!";
				break;
			case bangumi_bot_errors::http_get_contents_error:
				return "Bot GetContents error!";
				break;
			case bangumi_bot_errors::api_301_error:
				return "Bot 301 error!";
				break;
			case bangumi_bot_errors::no_error:
				return "Bot no error!";
				break;
			case bangumi_bot_errors::invalid_param:
				return "Bot has an unresolve param!";
				break;
			default:
				return "Unknow Error!";
				break;
			}

		}
	};

	class bgm_errors_category
		:public boost::system::error_category
	{
		//重写两个虚函数
	public:
		const char* name() const BOOST_SYSTEM_NOEXCEPT {
			return "Bangumi故障";
		}

		std::string message(int e)const {
			switch (e)
			{
			case bangumi_bot_errors::empty_user:
				return "未找到用户";
				break;
			case bangumi_bot_errors::empty_subject:
				return "未找到条目";
				break;
			case bangumi_bot_errors::subject_sql_miss:
				return "未于SQL中找到条目...";
				break;
			case bangumi_bot_errors::auth_request_error:
				return "授权请求失败...请重新绑定Bangumi-ID...";
				break;
			case bangumi_bot_errors::auth_without_same_id:
				return "发生了意外的情况: 授权与ID不匹配，请重新绑定Bangumi-ID...";
				break;
			case bangumi_bot_errors::user_not_collect_this_subject:
				return "\n\n<未收藏>";
				break;
			case bangumi_bot_errors::access_token_invalid:
				return "\n\n用户授权失效...";
				break;
			case bangumi_bot_errors::search_failed:
				return "未找到相关条目...";
				break;
			case bangumi_bot_errors::subject_collect_failed:
				return "收藏条目失败...";
				break;
			case bangumi_bot_errors::you_need_bind_your_bgm_id:
				return "绑定Bangumi后才能使用进度更新功能...";
				break;
			case bangumi_bot_errors::maybe_301_maybe_limit:
				return "此条目被重定向或当前无法访问...";
				break;
			case bangumi_bot_errors::update_with_the_same_eps:
				return "更新失败: 请确保与上次更新不同或授权失效...";
				break;
			case bangumi_bot_errors::net_error:
				return "访问失败...";
				break;
			default:
				return "未知的错误...";
				break;
			}

		}
	};


	const boost::system::error_category &
		get_bot_errors_category() {
		static bot_errors_category cat;
		return cat;
	}

	const boost::system::error_category &
		get_bgm_errors_category() {
		static bgm_errors_category cat_bgm;
		return cat_bgm;
	}
	//定义一个函数重载

	boost::system::error_code
		make_error_code(bot_error_codes e) {
		return boost::system::error_code(
			static_cast<int>(e), get_bot_errors_category());

	}

	//定义一个函数重载

	boost::system::error_code
		make_error_code(bgm_error_codes e) {
		return boost::system::error_code(
			static_cast<int>(e), get_bgm_errors_category());

	}

}

//将自定的错误代码加入到boost::system::error_code域中
namespace boost {
	namespace system {
		//make_error_code是在这里设置可被自动调用
		//就在Request::on_finish()中参数从一个enum隐式转换为一个boost::system::error_code
		template<>
		struct is_error_code_enum
			<bangumi_bot_errors::bot_error_codes>
		{
			BOOST_STATIC_CONSTANT(bool, value = true);
		};

		template<>
		struct is_error_code_enum
			<bangumi_bot_errors::bgm_error_codes>
		{
			BOOST_STATIC_CONSTANT(bool, value = true);
		};
	}
}
