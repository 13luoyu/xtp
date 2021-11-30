#include "quote_spi.h"
#include <iostream>
#include <stdio.h>
#include <fstream>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include "thread_pool.h"
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
//线程池
ThreadPool * pool=NULL;
//缓冲区
const int buffersize=1000;
XTPMD * buffer1[buffersize*10];
XTPTBT * buffer2[buffersize*10];
//缓冲区锁，为的是互斥访问，但希望永远不要用到
pthread_mutex_t buffer1_lock[10];
pthread_mutex_t buffer2_lock[10];
int b1=0, b2=0;
int to1=buffersize, to2=buffersize;

void * WriteDepthMarketData(void *arg)
{
	XTPMD **market_data=(XTPMD **)arg;
	XTPMD **t_data=(XTPMD **)malloc(8*buffersize);
	pthread_mutex_lock(&buffer1_lock[(market_data-buffer1)/8/buffersize]);
	memcpy(t_data, market_data, 8*buffersize);
	pthread_mutex_unlock(&buffer1_lock[(market_data-buffer1)/8/buffersize]);
	market_data=t_data;
	
	ofstream os(depth_csv, ios::app);
	for(int c=0;c<buffersize;c++){
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
				os<<market_data[c]->ticker_status[0]<<market_data[c]->ticker_status[1]<<market_data[c]->ticker_status[2]<<"\n";
			else if(market_data[c]->exchange_id==XTP_EXCHANGE_SZ)
				os<<market_data[c]->ticker_status[0]<<market_data[c]->ticker_status[1]<<"\n";
		}
		free(market_data[c]);
	}
	os.close();
	free(market_data);

	return NULL;
}

void * WriteTickByTick(void *arg)
{
	XTPTBT **tbt_data = (XTPTBT **)arg;
	//将buffer2的指针数据拷贝到新的地方，这样不用一直锁着buffer2
	XTPTBT **t_data = (XTPTBT **)malloc(8*buffersize);
	pthread_mutex_lock(&buffer2_lock[(tbt_data-buffer2)/8/buffersize]);
	memcpy(t_data, tbt_data, 8*buffersize);
	pthread_mutex_unlock(&buffer2_lock[(tbt_data-buffer2)/8/buffersize]);
	tbt_data=t_data;

	ofstream os(entrust_csv, ios::app);
	ofstream os2(trade_csv, ios::app);
	for(int c=0;c<buffersize;c++) {
		if(tbt_data[c]->type==XTP_TBT_ENTRUST)
		{
			os<<tbt_data[c]->exchange_id<<","<<tbt_data[c]->ticker<<","<<
			tbt_data[c]->data_time<<",";
			XTPTickByTickEntrust& entrust=tbt_data[c]->entrust;
			os<<entrust.channel_no<<","<<entrust.seq<<","<<
			entrust.price<<","<<entrust.qty<<","<<entrust.side<<
			","<<entrust.ord_type<<"\n";

		}
		else if(tbt_data[c]->type==XTP_TBT_TRADE)
		{
			
			os2<<tbt_data[c]->exchange_id<<","<<tbt_data[c]->ticker<<","
			<<tbt_data[c]->data_time<<",";
			XTPTickByTickTrade &trade=tbt_data[c]->trade;
			os2<<trade.channel_no<<","<<trade.seq<<","<<trade.price<<","
			<<trade.qty<<","<<trade.money<<","<<trade.bid_no<<","<<
			trade.ask_no<<","<<trade.trade_flag<<"\n";

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
	pool=new ThreadPool(20,1000);
	for(int i=0;i<10;i++){
		pthread_mutex_init(&buffer1_lock[i], NULL);
		pthread_mutex_init(&buffer2_lock[i], NULL);
	}
}

MyQuoteSpi::~MyQuoteSpi()
{
	delete pool;
	for(int i=0;i<10;i++){
		pthread_mutex_destroy(&buffer1_lock[i]);
		pthread_mutex_destroy(&buffer2_lock[i]);
	}
}

void MyQuoteSpi::OnDisconnected(int reason)
{
	cout << "--->>> " << "OnDisconnected quote" << endl;
	cout << "--->>> Reason = " << reason << endl;
	//if disconnect, exit and restart
	int loginResult=pQuoteApi->Login(quote_server_ip.c_str(), quote_server_port, quote_username.c_str(), quote_password.c_str(),quote_protocol);
	if(loginResult==0){
		//订阅所有行情
		pQuoteApi->SubscribeAllMarketData();
		//订阅所有逐笔行情
		pQuoteApi->SubscribeAllTickByTick();
	}
	ofstream o("log.txt", ios::app);
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
	o<<tmp<<": restart process\n";
	o.close();
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
	market_data->data_time=tmp;

	void *data=malloc(sizeof(XTPMD));
	memcpy(data, market_data, sizeof(XTPMD));
	buffer1[b1++] = (XTPMD *)data;
	if(b1==to1){
		pool->ThreadPoolAddJob(WriteDepthMarketData, (void *)&buffer1[b1-buffersize]);
		pthread_mutex_unlock(&buffer1_lock[(b1-buffersize)/buffersize]);
		if(b1==10*buffersize){
			b1=0;
			to1=buffersize;
		}
		else{
			to1+=buffersize;
		}
		pthread_mutex_lock(&buffer1_lock[b1/buffersize]);
	}
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
	tbt_data->data_time=tmp;

	void *data=malloc(sizeof(XTPTBT));
	memcpy(data, tbt_data, sizeof(XTPTBT));
	buffer2[b2++]=(XTPTBT *)data;
	if(b2==to2){
		pool->ThreadPoolAddJob(WriteTickByTick, (void *)&buffer2[b2-buffersize]);
		pthread_mutex_unlock(&buffer2_lock[(b2-buffersize)/buffersize]);
		if(b2==10*buffersize){
			b2=0;
			to2=buffersize;
		}
		else{
			to2+=buffersize;
		}
		pthread_mutex_lock(&buffer2_lock[b2/buffersize]);
	}
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

