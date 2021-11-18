// // testTradeApi.cpp : �������̨Ӧ�ó������ڵ㡣
// //

// #include "xtp_trader_api.h"
// #include <string>
// #include <map>
// #include <iostream>

// #ifdef _WIN32
// #include <windows.h>
// #else
// #include <unistd.h>
// #endif // _WIN32

// #include "trade_spi.h"
// #include "FileUtils.h"
// #include "xtp_quote_api.h"
// #include "quote_spi.h"

// XTP::API::TraderApi* pUserApi;
// bool is_connected_ = false;
// std::string trade_server_ip;
// int trade_server_port;
// uint64_t session_id_ = 0;
// std::map<uint64_t,uint64_t> map_session;
// uint64_t* session_arrary = NULL;
// FileUtils* fileUtils = NULL;
// XTPOrderInsertInfo* orderList = NULL;
// std::string quote_server_ip;
// int quote_server_port;
// std::string quote_username;
// std::string quote_password;
// XTP_PROTOCOL_TYPE quote_protocol = XTP_PROTOCOL_UDP;

// int main()
// {

// 	FileUtils* fileUtils = new FileUtils();
// 	if (!fileUtils->init())//��/api/config.json�ļ���fileUtils.m_docData��
// 	{
// 		std::cout << "The config.json file parse error." << std::endl;
// #ifdef _WIN32
// 		system("pause");
// #endif

// 		return 0;
// 	}

// 	//��ȡ��������
// 	trade_server_ip = fileUtils->stdStringForKey("trade_ip");
// 	trade_server_port = fileUtils->intForKey("trade_port");
// 	int out_count = fileUtils->intForKey("out_count");
// 	bool auto_save = fileUtils->boolForKey("auto_save");
// 	int client_id = fileUtils->intForKey("client_id");
// 	int account_count = fileUtils->countForKey("account");
// 	int resume_type = fileUtils->intForKey("resume_type");
// 	std::string account_key = fileUtils->stdStringForKey("account_key");
// #ifdef _WIN32
// 	std::string filepath = fileUtils->stdStringForKey("path");
// #else
// 	std::string filepath = fileUtils->stdStringForKey("path_linux");
// #endif // _WIN32

// 	//��ȡ��������
// 	quote_server_ip = fileUtils->stdStringForKey("quote_ip");
// 	quote_server_port = fileUtils->intForKey("quote_port");
// 	quote_username = fileUtils->stdStringForKey("quote_user");
// 	quote_password = fileUtils->stdStringForKey("quote_password");
// 	quote_protocol = (XTP_PROTOCOL_TYPE)(fileUtils->intForKey("quote_protocol"));
// 	int32_t quote_buffer_size = fileUtils->intForKey("quote_buffer_size");

// 	//��ȡ������ʱ����
// 	int32_t heat_beat_interval = fileUtils->intForKey("hb_interval");


// 	//��ʼ������api
// 	XTP::API::QuoteApi* pQuoteApi = XTP::API::QuoteApi::CreateQuoteApi(client_id, filepath.c_str(), XTP_LOG_LEVEL_DEBUG);//log��־������Ե���
// 	MyQuoteSpi* pQuoteSpi = new MyQuoteSpi();
// 	pQuoteApi->RegisterSpi(pQuoteSpi);

// 	//�趨�����������ʱʱ�䣬��λΪ��
// 	pQuoteApi->SetHeartBeatInterval(heat_beat_interval); //��Ϊ1.1.16�����ӿ�
// 	//�趨���鱾�ػ����С����λΪMB
// 	pQuoteApi->SetUDPBufferSize(quote_buffer_size);//��Ϊ1.1.16�����ӿ�

// 	int loginResult_quote = -1;
// 	//��¼���������,��1.1.16��ʼ�����������֧��UDP���ӣ��Ƽ�ʹ��UDP
// 	loginResult_quote = pQuoteApi->Login(quote_server_ip.c_str(), quote_server_port, quote_username.c_str(), quote_password.c_str(), quote_protocol); 
// 	if (loginResult_quote == 0)
// 	{
// 		//��¼����������ɹ��󣬶�������
// 		int instrument_count = fileUtils->countForKey("quote_ticker.instrument");
// 		int quote_exchange = fileUtils->intForKey("quote_ticker.exchange");

// 		//�������ļ��ж�ȡ��Ҫ���ĵĹ�Ʊ
// 		char* *allInstruments = new char*[instrument_count];
// 		for (int i = 0; i < instrument_count; i++) {
// 			allInstruments[i] = new char[7];
// 			std::string instrument = fileUtils->stdStringForKey("quote_ticker.instrument[%d]", i);
// 			strcpy(allInstruments[i], instrument.c_str());
// 		}

// 		//��ʼ����,ע�⹫�����Ի�����֧��TCP��ʽ�����ʹ��UDP��ʽ��û���������ݣ�ʵ�̴����ʹ��UDP����
// 		pQuoteApi->SubscribeMarketData(allInstruments, instrument_count, (XTP_EXCHANGE_TYPE)quote_exchange);

// 		//�ͷ�
// 		for (int i = 0; i < instrument_count; i++) {
// 			delete[] allInstruments[i];
// 			allInstruments[i] = NULL;
// 		}

// 		delete[] allInstruments;
// 		allInstruments = NULL;
// 	}
// 	else
// 	{
// 		//��¼ʧ�ܣ���ȡʧ��ԭ��
// 		XTPRI* error_info = pQuoteApi->GetApiLastError();
// 		std::cout << "Login to server error, " << error_info->error_id << " : " << error_info->error_msg << std::endl;
	
// 	}


// 	if (account_count > 0)
// 	{
// 		//��Զ��û������
// 		orderList = new XTPOrderInsertInfo[account_count];
// 		memset(orderList, 0, sizeof(XTPOrderInsertInfo)*account_count);
// 	}
	

// 	//��ʼ��������Api
// 	pUserApi = XTP::API::TraderApi::CreateTraderApi(client_id,filepath.c_str(), XTP_LOG_LEVEL_DEBUG);			// ����UserApi��log��־������Ե���
// 	pUserApi->SubscribePublicTopic((XTP_TE_RESUME_TYPE)resume_type);
// 	pUserApi->SetSoftwareVersion("1.1.0"); //�趨�������Ŀ����汾�ţ��û��Զ���
// 	pUserApi->SetSoftwareKey(account_key.c_str());//�趨�û��Ŀ������룬��XTP���뿪��ʱ����xtp��Ա�ṩ
// 	pUserApi->SetHeartBeatInterval(heat_beat_interval);//�趨���׷�������ʱʱ�䣬��λΪ�룬��Ϊ1.1.16�����ӿ�
// 	MyTraderSpi* pUserSpi = new MyTraderSpi();
// 	pUserApi->RegisterSpi(pUserSpi);						// ע���¼���
// 	pUserSpi->setUserAPI(pUserApi);
// 	pUserSpi->set_save_to_file(auto_save);
// 	if (out_count > 0)
// 	{
// 		pUserSpi->OutCount(out_count);
// 	}
// 	else
// 	{
// 		out_count = 1;
// 	}

// 	if (account_count > 0)
// 	{
// 		//���û�ʱ����session�����������û�session_id
// 		session_arrary = new uint64_t[account_count];

// 		for (int i = 0; i < account_count; i++)
// 		{
// 			//�������ļ��ж�ȡ��i���û���¼��Ϣ
// 			std::string account_name = fileUtils->stdStringForKey("account[%d].user", i);
// 			std::string account_pw = fileUtils->stdStringForKey("account[%d].password", i);
			
// 			uint64_t temp_session_ = 0;
// 			std::cout << account_name << " login begin." << std::endl;
// 			temp_session_ = pUserApi->Login(trade_server_ip.c_str(), trade_server_port, account_name.c_str(), account_pw.c_str(), XTP_PROTOCOL_TCP); //��¼���׷�����

// 			if (session_id_ == 0)
// 			{
// 				session_id_ = temp_session_;
// 			}

// 			if (temp_session_ > 0)
// 			{
// 				//��¼�ɹ��󣬽���session_id���û�֮���ӳ���ϵ
// 				map_session.insert(std::make_pair(temp_session_, i));
// 			}
// 			else
// 			{
// 				//��¼ʧ�ܣ���ȡ��¼ʧ��ԭ��
// 				XTPRI* error_info = pUserApi->GetApiLastError();
// 				std::cout << account_name << " login to server error, " << error_info->error_id << " : " << error_info->error_msg << std::endl;
// 			}

// 			session_arrary[i] = temp_session_;
// 		}
// 	}

// 	if (session_id_ > 0)
// 	{
// 		//���û��ɹ���¼���׷�����
// 		std::cout << "Login to server success." << std::endl;

// 		is_connected_ = true;

// 		//ÿ���û������������ļ�����
// 		for (int i = 0; i < account_count; i++)
// 		{
// 			//�������ļ��ж�ȡ������Ϣ
// 			int j = 0;
// 			orderList[i].order_client_id = i;
// 			std::string instrument = fileUtils->stdStringForKey("order[%d].instrument_id", j);
// 			strcpy(orderList[i].ticker, instrument.c_str());
// 			orderList[i].market = (XTP_MARKET_TYPE)fileUtils->intForKey("order[%d].exchange", j);
// 			orderList[i].price = fileUtils->floatForKey("order[%d].price", j);
// 			orderList[i].quantity = fileUtils->intForKey("order[%d].quantity", j);
// 			orderList[i].side = (XTP_SIDE_TYPE)fileUtils->intForKey("order[%d].side", j);
// 			orderList[i].price_type = (XTP_PRICE_TYPE)fileUtils->intForKey("order[%d].price_type", j);
// 			orderList[i].business_type = (XTP_BUSINESS_TYPE)fileUtils->intForKey("order[%d].business_type", j); //��Ϊ1.1.16�����ֶΣ���ͨҵ��ʹ��0������ҵ����ο�ע��
// 			orderList[i].position_effect = (XTP_POSITION_EFFECT_TYPE)fileUtils->intForKey("order[%d].position_effect", j);//��Ϊ1.1.18�����ֶ�,����Ȩ������ҵ��ʹ�ã���ͨҵ���ʹ��0

// 			if (session_arrary[i] == 0)
// 			{
// 				//�û���¼���ɹ�ʱ������
// 				continue;
// 			}

// 			//����
// 			int64_t xtp_id = pUserApi->InsertOrder(&(orderList[i]), session_arrary[i]);
// 			if (xtp_id == 0)
// 			{
// 				//�������ɹ�ʱ���ȴ�һ���������
// #ifdef _WIN32
// 				Sleep(1000);
// #else
// 				sleep(1);
// #endif // WIN32
// 				continue;
// 			}

// 			std::string account_name = fileUtils->stdStringForKey("account[%d].user", i);
// 			std::cout << account_name <<" insert order success." << std::endl;
// 		}

// 		//���߳�ѭ������ֹ�����˳�
// 		while (true)
// 		{
// #ifdef _WIN32
// 			Sleep(1000);
// #else
// 			sleep(1);
// #endif // WIN32

// 		}
// 	}

	
// 	delete fileUtils;
// 	fileUtils = NULL;

// 	if (orderList)
// 	{
// 		delete[] orderList;
// 		orderList = NULL;
// 	}

// 	if (session_arrary)
// 	{
// 		delete[] session_arrary;
// 		session_arrary = NULL;
// 	}

// 	return 0;
// }
