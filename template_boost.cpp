//============================================================================
// Name        : template_boost.cpp
// Author      : BFV
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>

#include "stdafx.h"
using namespace std;

class BKK_Main {
	boost::thread_group				m_thrdGr;
	boost::asio::io_context			m_Ioc;
	boost::asio::steady_timer		m_WorkerThread;

	std::vector<uint8_t> buff;
	boost::mutex mutex_;

	boost::asio::strand<boost::asio::io_context::executor_type> m_strand;

	void WorkThread(size_t Number);

	template<class Rep, class Period>
	void TimerFunc(const std::chrono::duration<Rep, Period>& dur) {
		m_WorkerThread.expires_from_now(dur);
		m_WorkerThread.async_wait(boost::bind(&BKK_Main::StartSecFunc, this, boost::asio::placeholders::error));
	}
	void StartSecFunc(const boost::system::error_code& error);
public:
	BKK_Main();
	~BKK_Main();

	void cpaBuf(std::vector<uint8_t> bf);

	void WaitStopped();

	void init(uint num);
	void pri_init(uint num);

	void Conv(std::vector<uint8_t> dat);
	void pri_Conv(std::vector<uint8_t> dat);

	boost::asio::io_service& get_io_context();

};


int main() {

	BKK_Main BM;

	cout << "!!!Hello World!!!" << endl; // prints !!!Hello World!!!

	boost::this_thread::sleep(boost::posix_time::milliseconds(5000));

	BM.init(5);

	BM.WaitStopped();

	return 0;
}


BKK_Main::BKK_Main():m_WorkerThread(m_Ioc),
				m_strand(boost::asio::make_strand(m_Ioc)){

	TimerFunc(std::chrono::milliseconds(1000));

	for(size_t i=0; i < 3; ++i)
		m_thrdGr.create_thread(	std::bind(&BKK_Main::WorkThread, this, m_thrdGr.size())	);

}

BKK_Main::~BKK_Main() {
	boost::mutex::scoped_lock look(mutex_);

	m_Ioc.stop();
	m_thrdGr.join_all();
}
void BKK_Main::cpaBuf(std::vector<uint8_t> bf){
		for(auto it:bf)
			buff.push_back(it);
}

void BKK_Main::init(uint num){
	boost::asio::dispatch(m_strand, std::bind(&BKK_Main::pri_init, this, num)   );
}
void BKK_Main::pri_init(uint num){
	boost::mutex::scoped_lock look(mutex_);

	cout << "!!!Hello World!!!" << num << endl;
}

void BKK_Main::Conv(std::vector<uint8_t> dat){
	m_Ioc.post(boost::bind(&BKK_Main::pri_Conv, this,dat));
}
void BKK_Main::pri_Conv(std::vector<uint8_t> dat){

}

void BKK_Main::WorkThread(size_t Number){
	static const char* strFuncName = "ExecutionContext::WorkThread";

	boost::system::error_code ec;

	while (true)
	{
		try
		{
			m_Ioc.run(ec);

			if (ec)
				syslog(LOG_CRIT, "%s (n=%lu), run error: %s\n", strFuncName, Number, ec.message().c_str());

			break;
		}
		catch (std::exception& ex)
		{
			syslog(LOG_CRIT, "%s, %s\n", strFuncName, ex.what());
		}
	}
}

boost::asio::io_service& BKK_Main::get_io_context() {
	return m_Ioc;
}
void BKK_Main::StartSecFunc(const boost::system::error_code& error){
	//std::cout << "sec" << std::endl;
	TimerFunc(std::chrono::milliseconds(1000));
}

void BKK_Main::WaitStopped()	{
	m_thrdGr.join_all();
}
