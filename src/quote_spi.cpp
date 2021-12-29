#include "quote_spi.h"
#include <iostream>
#include <stdio.h>
#include <fstream>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
using namespace std;

extern string quote_server_ip;
extern int quote_server_port;
extern string quote_username;
extern string quote_password;
extern XTP_PROTOCOL_TYPE quote_protocol;
extern XTP::API::QuoteApi* pQuoteApi;

extern string depth_csv;
extern string entrust_csv;
extern string trade_csv;
//缓冲区，存储一天所有数据的指针
const int buffersize1 = 50000000;
const long long buffersize2 = 300000000;
XTPMD ** buffer1 = (XTPMD **)malloc(8 * buffersize1);
XTPTBT ** buffer2 = (XTPTBT **)malloc(8 * buffersize2);
long * time_buffer1 = (long *)malloc(sizeof(long) * buffersize1);
long * time_buffer2 = (long *)malloc(sizeof(long) * buffersize2);
int cnt1=0, cnt2=0;

void * WriteDepthMarketData()
{
	XTPMD **market_data = buffer1;
	ofstream os(depth_csv, ios::app);
	for(int c=0;c<cnt1;c++){
		os<<time_buffer1[c]<<",";
		os<<market_data[c]->exchange_id<<","<<market_data[c]->ticker<<","<<
		market_data[c]->last_price<<","<<
		market_data[c]->pre_close_price<<","<<market_data[c]->open_price<<","<<market_data[c]->high_price<<","<<
		market_data[c]->low_price<<","<<market_data[c]->close_price<<","<<market_data[c]->upper_limit_price<<","<<
		market_data[c]->lower_limit_price<<","<<market_data[c]->pre_delta<<","<<market_data[c]->curr_delta<<","<<
		market_data[c]->data_time<<","<<market_data[c]->qty<<","<<market_data[c]->turnover<<","<<market_data[c]->avg_price<<
		",";
		for(int i=0;i<10;i++)
			os<<market_data[c]->bid[i]<<",";
		for(int i=0;i<10;i++)
			os<<market_data[c]->ask[i]<<",";
		for(int i=0;i<10;i++)
			os<<market_data[c]->bid_qty[i]<<",";
		for(int i=0;i<10;i++)
			os<<market_data[c]->ask_qty[i]<<",";
		os<<market_data[c]->trades_count<<",";
		if(market_data[c]->ticker_status[0]!=0){
			if(market_data[c]->exchange_id==XTP_EXCHANGE_SH)
				os<<market_data[c]->ticker_status[0]<<market_data[c]->ticker_status[1]<<market_data[c]->ticker_status[2];
			else if(market_data[c]->exchange_id==XTP_EXCHANGE_SZ)
				os<<market_data[c]->ticker_status[0]<<market_data[c]->ticker_status[1];
		}
		os<<endl;
		free(market_data[c]);
	}
	os.close();
	free(market_data);

	return NULL;
}

void * WriteTickByTick()
{
	XTPTBT **tbt_data = buffer2;
	ofstream os(entrust_csv, ios::app);
	ofstream os2(trade_csv, ios::app);
	for(int c=0;c<cnt2;c++) {
		if(tbt_data[c]->type==XTP_TBT_ENTRUST)
		{
			os<<time_buffer2[c]<<",";
			os<<tbt_data[c]->exchange_id<<","<<tbt_data[c]->ticker<<","<<
			tbt_data[c]->data_time<<",";
			XTPTickByTickEntrust& entrust=tbt_data[c]->entrust;
			os<<entrust.channel_no<<","<<entrust.seq<<","<<
			entrust.price<<","<<entrust.qty<<","<<entrust.side<<
			","<<entrust.ord_type<<endl;
		}
		else if(tbt_data[c]->type==XTP_TBT_TRADE)
		{
			os2<<time_buffer2[c]<<",";
			os2<<tbt_data[c]->exchange_id<<","<<tbt_data[c]->ticker<<","
			<<tbt_data[c]->data_time<<",";
			XTPTickByTickTrade &trade=tbt_data[c]->trade;
			os2<<trade.channel_no<<","<<trade.seq<<","<<trade.price<<","
			<<trade.qty<<","<<trade.money<<","<<trade.bid_no<<","<<
			trade.ask_no<<","<<trade.trade_flag<<endl;
		}
		free(tbt_data[c]);
	}
	os.close();
	os2.close();
	free(tbt_data);
	return NULL;
}

void MyQuoteSpi::OnError(XTPRI *error_info, bool is_last)
{
	cout << "--->>> "<< "OnRspError" << endl;
	IsErrorRspInfo(error_info);
}

MyQuoteSpi::MyQuoteSpi()
{
}

MyQuoteSpi::~MyQuoteSpi()
{
	
}

void MyQuoteSpi::OnDisconnected(int reason)
{
	cout << "--->>> " << "OnDisconnected quote" << endl;
	cout << "--->>> Reason = " << reason << endl;

	time_t t;
	struct tm *tm;
	time(&t);
	tm=localtime(&t);
	if(tm->tm_hour<9 || tm->tm_hour>=15&&tm->tm_min>=30)  // 时间外
		return;
	signed long tmp=0;
	tmp+=tm->tm_year+1900;
	tmp*=100;	tmp+=tm->tm_mon+1;
	tmp*=100;	tmp+=tm->tm_mday;
	tmp*=100;	tmp+=tm->tm_hour;
	tmp*=100;	tmp+=tm->tm_min;
	tmp*=100;	tmp+=tm->tm_sec;
	ofstream o("log.txt", ios::app);
	o<<tmp<<": restart process\n";
	o.close();
	int loginResult=-1;
	while(loginResult != 0)
		loginResult = pQuoteApi->Login(quote_server_ip.c_str(), quote_server_port, quote_username.c_str(), quote_password.c_str(),quote_protocol);
	if(loginResult==0){
		//订阅所有行情
		pQuoteApi->SubscribeAllMarketData();
		//订阅所有逐笔行情
		pQuoteApi->SubscribeAllTickByTick();
	}
}

void MyQuoteSpi::OnSubMarketData(XTPST *ticker, XTPRI *error_info, bool is_last)
{
 	cout << "OnRspSubMarketData -----" << endl;
}

void MyQuoteSpi::OnUnSubMarketData(XTPST *ticker, XTPRI *error_info, bool is_last)
{
 	cout << "OnRspUnSubMarketData -----------" << endl;
}

void MyQuoteSpi::OnDepthMarketData(XTPMD * market_data, int64_t bid1_qty[], int32_t bid1_count, int32_t max_bid1_count, int64_t ask1_qty[], int32_t ask1_count, int32_t max_ask1_count)
{
	if(market_data->data_time==XTP_MARKETDATA_OPTION)//期权忽略
		return;
	//获取本地时间
	struct timeval tv;
	gettimeofday(&tv, NULL);
	time_t t;
	struct tm *tm;
	time(&t);
	tm=localtime(&t);
	signed long tmp=0;
	tmp+=tm->tm_year+1900;
	tmp*=100;	tmp+=tm->tm_mon+1;
	tmp*=100;	tmp+=tm->tm_mday;
	tmp*=100;	tmp+=tm->tm_hour;
	tmp*=100;	tmp+=tm->tm_min;
	tmp*=100;	tmp+=tm->tm_sec;
	tmp*=1000;	tmp+=tv.tv_usec/1000;
	time_buffer1[cnt1] = tmp;

	void *data=malloc(sizeof(XTPMD));
	memcpy(data, market_data, sizeof(XTPMD));
	if(cnt1==buffersize1){
		ofstream out("log.txt", ios::app);
		out<<tmp<<": buffer1 full\n";
		out.close();
		cnt1++;
		return;
	}
	else if(cnt1>buffersize1){
		cnt1++;
		return;
	}
	buffer1[cnt1++] = (XTPMD *)data;
}

void MyQuoteSpi::OnSubOrderBook(XTPST *ticker, XTPRI *error_info, bool is_last)
{
	cout << "OnRspSubOrderBook -----" << endl;
}

void MyQuoteSpi::OnUnSubOrderBook(XTPST *ticker, XTPRI *error_info, bool is_last)
{
	cout << "OnRspUnSubOrderBook -----------" << endl;
}

void MyQuoteSpi::OnSubTickByTick(XTPST *ticker, XTPRI *error_info, bool is_last)
{
	cout << "OnRspSubTickByTick -----" << endl;
}

void MyQuoteSpi::OnUnSubTickByTick(XTPST * ticker, XTPRI * error_info, bool is_last)
{
	cout << "OnRspUnSubTickByTick -----------" << endl;
}

void MyQuoteSpi::OnOrderBook(XTPOB *order_book)
{

}

void MyQuoteSpi::OnTickByTick(XTPTBT *tbt_data)
{
	//获取本地时间
	struct timeval tv;
	gettimeofday(&tv, NULL);
	time_t t;
	struct tm *tm;
	time(&t);
	tm=localtime(&t);
	signed long tmp=0;
	tmp+=tm->tm_year+1900;
	tmp*=100;	tmp+=tm->tm_mon+1;
	tmp*=100;	tmp+=tm->tm_mday;
	tmp*=100;	tmp+=tm->tm_hour;
	tmp*=100;	tmp+=tm->tm_min;
	tmp*=100;	tmp+=tm->tm_sec;
	tmp*=1000;	tmp+=tv.tv_usec/1000;
	time_buffer2[cnt2] = tmp;

	void *data=malloc(sizeof(XTPTBT));
	memcpy(data, tbt_data, sizeof(XTPTBT));
	if(cnt2==buffersize2){
		ofstream out("log.txt", ios::app);
		out<<tmp<<": buffer2 full\n";
		out.close();
		cnt2++;
		return;
	}
	else if(cnt2>buffersize2){
		cnt2++;
		return;
	}
	buffer2[cnt2++]=(XTPTBT *)data;
}

void MyQuoteSpi::OnQueryAllTickers(XTPQSI * ticker_info, XTPRI * error_info, bool is_last)
{
	cout << "OnQueryAllTickers -----------" << endl;
}

void MyQuoteSpi::OnQueryTickersPriceInfo(XTPTPI * ticker_info, XTPRI * error_info, bool is_last)
{
}

void MyQuoteSpi::OnSubscribeAllMarketData(XTP_EXCHANGE_TYPE exchange_id, XTPRI * error_info)
{
	cout << "OnRspSubAllMarketData -----" << endl;
}

void MyQuoteSpi::OnUnSubscribeAllMarketData(XTP_EXCHANGE_TYPE exchange_id, XTPRI * error_info)
{
	cout << "OnRspUnSubAllMarketData -----" << endl;
}

void MyQuoteSpi::OnSubscribeAllOrderBook(XTP_EXCHANGE_TYPE exchange_id, XTPRI * error_info)
{
}

void MyQuoteSpi::OnUnSubscribeAllOrderBook(XTP_EXCHANGE_TYPE exchange_id, XTPRI * error_info)
{
}

void MyQuoteSpi::OnSubscribeAllTickByTick(XTP_EXCHANGE_TYPE exchange_id, XTPRI * error_info)
{
	cout << "OnRspSubAllTickByTick -----" << endl;
}

void MyQuoteSpi::OnUnSubscribeAllTickByTick(XTP_EXCHANGE_TYPE exchange_id, XTPRI * error_info)
{
	cout << "OnRspUnSubAllTickByTick -----" << endl;
}

void MyQuoteSpi::OnSubscribeAllOptionMarketData(XTP_EXCHANGE_TYPE exchange_id, XTPRI * error_info)
{
}

void MyQuoteSpi::OnUnSubscribeAllOptionMarketData(XTP_EXCHANGE_TYPE exchange_id, XTPRI * error_info)
{
}

void MyQuoteSpi::OnSubscribeAllOptionOrderBook(XTP_EXCHANGE_TYPE exchange_id, XTPRI * error_info)
{
}

void MyQuoteSpi::OnUnSubscribeAllOptionOrderBook(XTP_EXCHANGE_TYPE exchange_id, XTPRI * error_info)
{
}

void MyQuoteSpi::OnSubscribeAllOptionTickByTick(XTP_EXCHANGE_TYPE exchange_id, XTPRI * error_info)
{
}

void MyQuoteSpi::OnUnSubscribeAllOptionTickByTick(XTP_EXCHANGE_TYPE exchange_id, XTPRI * error_info)
{
}

bool MyQuoteSpi::IsErrorRspInfo(XTPRI *pRspInfo)
{
	// ���ErrorID != 0, ˵���յ��˴�������?
	bool bResult = ((pRspInfo) && (pRspInfo->error_id != 0));
	if (bResult)
		cout << "--->>> ErrorID=" << pRspInfo->error_id << ", ErrorMsg=" << pRspInfo->error_msg << endl;
	return bResult;
}

