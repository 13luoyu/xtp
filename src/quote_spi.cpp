#include "quote_spi.h"
#include <iostream>
#include <stdio.h>
#include <fstream>
#include <string>
#include <string.h>
#include <stdlib.h>
#include "thread_pool.h"
using namespace std;

extern string depth_csv;
extern string entrust_csv;
extern string trade_csv;
struct threadpool *pool;

void * WriteDepthMarketData(void *arg)
{
	XTPMD *market_data=(XTPMD *)arg;
	ofstream os(depth_csv, ios::app);
	os<<market_data->exchange_id<<","<<market_data->ticker<<","<<
	market_data->last_price<<","<<
	market_data->pre_close_price<<","<<market_data->open_price<<","<<market_data->high_price<<","<<
	market_data->low_price<<","<<market_data->close_price<<","<<market_data->upper_limit_price<<","<<
	market_data->lower_limit_price<<","<<market_data->pre_delta<<","<<market_data->curr_delta<<","<<
	market_data->data_time<<","<<market_data->qty<<","<<market_data->turnover<<","<<market_data->avg_price<<
	",";
	for(int i=0;i<10;i++)
		os<<market_data->bid[i]<<",";
	for(int i=0;i<10;i++)
		os<<market_data->ask[i]<<",";
	for(int i=0;i<10;i++)
		os<<market_data->bid_qty[i]<<",";
	for(int i=0;i<10;i++)
		os<<market_data->ask_qty[i]<<",";
	os<<market_data->trades_count<<",";
	if(market_data->ticker_status[0]!=0){
		if(market_data->exchange_id==XTP_EXCHANGE_SH)
			os<<market_data->ticker_status[0]<<market_data->ticker_status[1]<<market_data->ticker_status[2]<<"\n";
		else if(market_data->exchange_id==XTP_EXCHANGE_SZ)
			os<<market_data->ticker_status[0]<<market_data->ticker_status[1]<<endl;
	}
	os.close();

	delete market_data;
	return NULL;
}

void * WriteTickByTick(void *arg)
{
	XTPTBT *tbt_data=(XTPTBT *)arg;
	if(tbt_data->type==XTP_TBT_ENTRUST)
	{
		ofstream os(entrust_csv, ios::app);
		os<<tbt_data->exchange_id<<","<<tbt_data->data_time<<
		",";
		XTPTickByTickEntrust& entrust=tbt_data->entrust;
		os<<entrust.channel_no<<","<<entrust.seq<<","<<
		entrust.price<<","<<entrust.qty<<","<<entrust.side<<
		","<<entrust.ord_type<<endl;
		os.close();
	}
	else if(tbt_data->type==XTP_TBT_TRADE)
	{
		ofstream os(trade_csv, ios::app);
		os<<tbt_data->exchange_id<<","<<tbt_data->data_time<<",";
		XTPTickByTickTrade &trade=tbt_data->trade;
		os<<trade.channel_no<<","<<trade.seq<<","<<trade.price<<","
		<<trade.qty<<","<<trade.money<<","<<trade.bid_no<<","<<
		trade.ask_no<<","<<trade.trade_flag<<endl;
		os.close();
	}
	delete tbt_data;
	return NULL;
}

void MyQuoteSpi::OnError(XTPRI *error_info, bool is_last)
{
	cout << "--->>> "<< "OnRspError" << endl;
	IsErrorRspInfo(error_info);
}

MyQuoteSpi::MyQuoteSpi()
{
	pool=threadpool_init(100, 10);
}

MyQuoteSpi::~MyQuoteSpi()
{
	threadpool_destroy(pool);
}

void MyQuoteSpi::OnDisconnected(int reason)
{
	cout << "--->>> " << "OnDisconnected quote" << endl;
	cout << "--->>> Reason = " << reason << endl;
	exit(1);//if disconnect, exit and restart
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
	void *data=malloc(sizeof(XTPMD));
	memcpy(data, market_data, sizeof(XTPMD));
	threadpool_add_job(pool, WriteDepthMarketData, data);
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
	XTPTBT *data=new XTPTBT;
	memcpy(data, tbt_data, sizeof(XTPTBT));
	threadpool_add_job(pool, WriteTickByTick, data);
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

