#include "xtp_trader_api.h"
#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <time.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif // _WIN32

#include "FileUtils.h"
#include "xtp_quote_api.h"
#include "quote_spi.h"

FileUtils* fileUtils = NULL;
std::string quote_server_ip;
int quote_server_port;
std::string quote_username;
std::string quote_password;
XTP_PROTOCOL_TYPE quote_protocol = XTP_PROTOCOL_UDP;
XTP::API::QuoteApi* pQuoteApi;

//文件名
std::string depth_csv;
std::string entrust_csv;
std::string trade_csv;
extern int cnt1;
extern int cnt2;

extern void * WriteDepthMarketData();
extern void * WriteTickByTick();

int main(int argc, char **argv)
{
	time_t time_now=time(0);
	struct tm * p_tm=localtime(&time_now);
	std::string date=std::to_string(p_tm->tm_year+1900)+std::to_string(p_tm->tm_mon+1);
	if(p_tm->tm_mday <10)
		date += "0";
	date += std::to_string(p_tm->tm_mday);
	umask(0);
	mkdir(date.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);//775
	depth_csv=date+"/depth.csv";
	entrust_csv=date+"/entrust.csv";
	trade_csv=date+"/trade.csv";

	//Judge whether the file is empty, if it is, add the title, else do nothing
	bool is_depth_empty=false, is_entrust_empty=false, is_trade_empty=false;
	std::ifstream in(depth_csv.c_str());
	std::ifstream in2(entrust_csv.c_str());
	std::ifstream in3(trade_csv.c_str());
	char c=in.get();
	if(c==EOF || in.eof())
		is_depth_empty=true;
	c=in2.get();
	if(c==EOF || in2.eof())
		is_entrust_empty=true;
	c=in3.get();
	if(c==EOF || in3.eof())
		is_trade_empty=true;

	if(is_depth_empty)
	{
    	std::ofstream os(depth_csv.c_str());
		os<<"receive_time,";
		os<<"exchange_id,ticker,last_price,pre_close_price,open_price,high_price,low_price,close_price,"<<
			"upper_limit_price,lower_limit_price,pre_delta,curr_delta,data_time,qty,turnover,avg_price,";
	
		for(int i=0;i<10;i++)
			os<<"bid"<<i<<",";
		for(int i=0;i<10;i++)
			os<<"ask"<<i<<",";
		for(int i=0;i<10;i++)
			os<<"bid_qty"<<i<<",";
		for(int i=0;i<10;i++)
			os<<"ask_qty"<<i<<",";
		
		os<<"trades_count,ticker_status\n";
		os.close();
	}
	if(is_entrust_empty)
	{
    	std::ofstream os1(entrust_csv.c_str());
		os1<<"receive_time,";
    	os1<<"exchange_id,ticker,data_time,channel_no,seq,price,qty,side,ord_type\n";
    	os1.close();
	}
	if(is_trade_empty)
	{
    	std::ofstream os2(trade_csv.c_str());
		os2<<"receive_time,";
    	os2<<"exchange_id,ticker,data_time,channel_no,seq,price,qty,money,bid_no,ask_no,trade_flag\n";
    	os2.close();
	}
	


    FileUtils *fileUtils=new FileUtils();
    if (!fileUtils->init())//读/api/config.json文件到fileUtils.m_docData中
	{
		std::cout << "The config.json file parse error." << std::endl;
#ifdef _WIN32
		system("pause");
#endif

		return 0;
	}

    //读取交易配置
	int client_id = fileUtils->intForKey("client_id");
#ifdef _WIN32
	std::string filepath = fileUtils->stdStringForKey("path");
#else
	std::string filepath = fileUtils->stdStringForKey("path_linux");
#endif // _WIN32

	//读取行情配置
	quote_server_ip = fileUtils->stdStringForKey("quote_ip");
	quote_server_port = fileUtils->intForKey("quote_port");
	quote_username = fileUtils->stdStringForKey("quote_user");
	quote_password = fileUtils->stdStringForKey("quote_password");
	quote_protocol = (XTP_PROTOCOL_TYPE)(fileUtils->intForKey("quote_protocol"));
	int32_t quote_buffer_size = fileUtils->intForKey("quote_buffer_size");

	//读取心跳超时配置
	int32_t heat_beat_interval = fileUtils->intForKey("hb_interval");


    //初始化行情api
	pQuoteApi = XTP::API::QuoteApi::CreateQuoteApi(client_id, filepath.c_str(), XTP_LOG_LEVEL_ERROR);//log日志级别可以调整
	MyQuoteSpi* pQuoteSpi = new MyQuoteSpi();
	pQuoteApi->RegisterSpi(pQuoteSpi);
	

	//设定行情服务器超时时间，单位为秒
	pQuoteApi->SetHeartBeatInterval(heat_beat_interval); //此为1.1.16新增接口
	//设定行情本地缓存大小，单位为MB
	pQuoteApi->SetUDPBufferSize(quote_buffer_size);//此为1.1.16新增接口

	int loginResult_quote = -1;
	//登录行情服务器,自1.1.16开始，行情服务器支持UDP连接，推荐使用UDP
	loginResult_quote = pQuoteApi->Login(quote_server_ip.c_str(), quote_server_port, quote_username.c_str(), quote_password.c_str(), quote_protocol); 
	if (loginResult_quote == 0)
	{
		//订阅所有行情
		pQuoteApi->SubscribeAllMarketData();
		//订阅所有逐笔行情
		pQuoteApi->SubscribeAllTickByTick();
	}
	else
	{
		//登录失败，获取失败原因
		XTPRI* error_info = pQuoteApi->GetApiLastError();
		std::cout << "Login to server error, " << error_info->error_id << " : " << error_info->error_msg << std::endl;
	
	}
	

	int end_time_hour=15, end_time_min=30;
	if(argc == 3){
		end_time_hour = atoi(argv[1]);
		end_time_min = atoi(argv[2]);
		if(end_time_hour >= 24 || end_time_hour < 0 || end_time_min < 0 || end_time_min >= 60){
			end_time_hour = 15;
			end_time_min = 30;
		}
	}
	int total_end_min = end_time_hour * 60 + end_time_min;
	int time_interval = 9 * 60 + 10;//每10分钟打印一次日志
    while(true)
    {
#ifdef _WIN32
		Sleep(1000);
#else
		sleep(1);
#endif // WIN32
		time_t t;
		struct tm *tm;
		time(&t);
		tm=localtime(&t);
		int total_min = tm->tm_hour * 60 + tm->tm_min;
		if(total_min >= total_end_min)
			break;
		if(total_min >= time_interval){
			std::ofstream out("log.txt", std::ios::app);
			out<<tm->tm_hour<<":"<<tm->tm_min<<std::endl;
			out<<"buffersize1:"<<cnt1<<std::endl;
			out<<"buffersize2:"<<cnt2<<std::endl;
			out.close();
			time_interval += 10;
		}
    }
	pQuoteApi->UnSubscribeAllMarketData();
	pQuoteApi->UnSubscribeAllTickByTick();
	pQuoteApi->Logout();
	pQuoteApi->Release();
	
	std::ofstream out("log.txt", std::ios::app);
	out<<"end buffersize1:"<<cnt1<<std::endl;
	out<<"end buffersize2:"<<cnt2<<std::endl;
	out.close();
	//写文件
	std::ofstream o("log.txt", std::ios::app);
	o<<"Writing files.\n";
	std::cout<<"Writing files.\n";
	WriteDepthMarketData();
	WriteTickByTick();
	std::cout<<"Write files complete.\n";
	o<<"Write files complete.\n";
	o.close();

	return 0;
}
