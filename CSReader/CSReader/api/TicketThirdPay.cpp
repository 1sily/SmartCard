#include "TicketThirdPay.h"
#include "Errors.h"
#include "Publics.h"
#include "TimesEx.h"
#include "../link/myprintf.h"
#include "../json/json.h"
#include "../ble/ble.h"
#include <string>

TicketThirdPay::TicketThirdPay(void)
{

}

TicketThirdPay::TicketThirdPay(char * p_current_sam, char * p_sam_posid, uint8_t * physic_info)
{
	m_p_current_sam = p_current_sam;
	m_p_sam_posid = p_sam_posid;

	memset(&m_ticket_data, 0, sizeof(m_ticket_data));
	memset(&m_ReadInf, 0, sizeof(m_ReadInf));

	sprintf(m_ticket_data.physical_id, "%02X%02X%02X%02X%02X%02X%02X%-6C",
		physic_info[0], physic_info[1], physic_info[2], physic_info[3], physic_info[4], physic_info[5], physic_info[6], 0x20);

	uint8_t * pMemoryPos = ((uint8_t *)this) + sizeof(TicketThirdPay);
	m_ticket_data.p_entry_info = new(pMemoryPos) TradeInfo;
	m_ticket_data.p_jtb_entry_info = new(pMemoryPos) JTBTradeInfo;
	//m_ReadInf.p_eTicket_entry_info = new(pMemoryPos) ETICKET_ENTRY_INF;

	m_ticket_data.p_exit_info = &m_ticket_data.read_last_info;
	m_ticket_data.p_update_info = &m_ticket_data.read_last_info;

	memset(&m_ticket_data.read_last_info, 0, sizeof(TradeInfo));

	m_ticket_data.qr_flag = 0x01;
}

TicketThirdPay::~TicketThirdPay(void)
{

}

uint16_t TicketThirdPay::metro_elec_read(P_THIRD_METRO_ELECT_INF p_mtr_inf)
{
	uint16_t u_ret			= 0;
	//unsigned short nresult		= 0;
	unsigned short response_sw	= 0;
	unsigned char ulen			= 0;
	unsigned char sztmp[256]	= {0};

	do
	{
		//memcpy(p_mtr_inf->p_eTicket_entry_info->cTxnCode,"12",2);


	} while (0);

	return u_ret;
}

RETINFO TicketThirdPay::read_card(ETYTKOPER operType, TICKET_PARAM& ticket_prm, uint8_t * p_read_init/* = NULL*/)
{
	RETINFO ret				= {0};
	SCAN_ALL_BLE_INF sabi;
	uint8_t mac_address[7] = {0};
	//uint16_t count =;
	uint8_t data_exchange[2000] = {0};
	uint16_t len_data_exchange;
	uint8_t format_data_json[2000] = {0};
	uint16_t length_eticket;
	do
	{
		memset(&m_u_buffer, 0, sizeof(m_u_buffer));
		dbg_formatvar("TicketThirdPay::read_card");
		m_ReadInf = file_need_read(operType, m_u_buffer);

		dbg_formatvar("TicketThirdPay::get_qrInfo");

		//分析二维码数据合法性
		get_qrInfo(Api::qr_readbuf);


	} while (0);

	return ret;
}

uint8_t TicketThirdPay::unified_status()
{
	uint8_t ret = USTATUS_UNDEFINED;

	switch (m_ticket_data.init_status)
	{
	case MS_Init:
		ret = USTATUS_INIT;
		break;
	case MS_Es:
		ret = USTATUS_ES;
		break;
	case MS_Sale:
		ret = USTATUS_SALE;
		break;
	case MS_Entry:
		ret = USTATUS_ENTRY;
		break;
	case MS_Exit:
		ret = USTATUS_EXIT;
		break;
	case MS_Exit_Only:
		ret = USTATUS_EXIT_ONLY;
		break;
	case MS_Exit_Tt:
		ret = USTATUS_EXIT_T;
		break;
	case MS_Upd_FZone_Free:
		ret = USTATUS_UPD_OUT_FREE;
		break;
	case MS_Upd_FZone_Fare:
		ret = USTATUS_UPD_OUT_FARE;
		break;
	case MS_Upd_Wzone_Entry:
		ret = USTATUS_UPD_WO_STA;
		break;
	case MS_Upd_Wzone_Exit:
		ret = USTATUS_UPD_IN;
		break;
	case MS_Upd_Wzone_Time:
		ret = USTATUS_UPD_TM_OUT;
		break;
	case MS_Upd_Wzone_Trip:
		ret = USTATUS_UPD_TP_OUT;
		break;
	//case MS_Upd_Wzone_Free:
	//	ret = SStatus_Upd_WZone;
	//	break;
	case MS_Refund:
		ret = USTATUS_REFUND;
	}

	return ret;
}



RETINFO TicketThirdPay::analyse_common(uint8_t wk_area, MODE_EFFECT_FLAGS * p_degrade_falgs, TICKET_PARAM * p_ticket_prm, PTICKETLOCK p_lock, uint16_t& lenlock, PBOMANALYZE p_analyse, char * p_mode)
{
	RETINFO ret				= {0};
	char mode				= '0';

	do
	{

	} while (0);

	if (p_mode != NULL)
		*p_mode = mode;

	return ret;
}

RETINFO TicketThirdPay::analyse_work_zone(MODE_EFFECT_FLAGS * p_degrade_falgs, TICKET_PARAM * p_ticket_prm, PBOMANALYZE p_analyse, PPURSETRADE p_purse, PBOMETICANALYZE p_eticanalyse)
{
	RETINFO ret				= {0};
	char exit_mode				= 0;
	uint16_t result_prm			= 0;
	uint16_t err				= 0;
	uint16_t err1				= 0;
	long trade_val				= 0;
	long penalty_val			= 0;
	long penalty_val1			= 0;
	long fare_trade             = 0;
	uint32_t operation_status	= 0;
	uint16_t tempEntryID		= 0;

	char temp = 0;

	///////////////////////////////////////////////////////
	/////跳出传统票卡的逻辑，只用来判断电子票卡的逻辑//////
	///////////////////////////////////////////////////////

	memcpy(m_ticket_data.fare_type, "\x02\x00", 2);

	do
	{
		// 如果发生列车故障，直接返回正常
		if (p_degrade_falgs->train_trouble)
		{
			exit_mode = (char)(degrade_Trouble + 0x30);
			break;
		}


		if (m_ticket_data.simple_status == SStatus_Entry)
		{

			// 超时(先检查超时，再检查超程，便于既超时，又超程的判断)
			dbg_formatvar("uncheck_datetime");
		}

		// 查票价
		result_prm = g_Parameter.query_fare(CmdSort::m_time_now, m_ticket_data.fare_type,
				m_ticket_data.p_entry_info->station_id, m_ticket_data.p_entry_info->time, &trade_val);
		dbg_formatvar("result_prm:%04x",result_prm);
		if (result_prm)
		{
			ret.wErrCode = result_prm;
			break;
		}

	} while(0);


	if (p_purse != NULL)
	{
		p_purse->lTradeAmount = trade_val;
		p_purse->cExitMode = exit_mode;
	}

	if (err == ERR_EXIT_TIMEOUT && err1 == ERR_OVER_MILEAGE)
		ret.wErrCode = ERR_OVER_TIME_MILEAGE;

	return ret;
}

RETINFO TicketThirdPay::analyse_free_zone(MODE_EFFECT_FLAGS * p_degrade_falgs, TICKET_PARAM * p_ticket_prm, PBOMANALYZE p_analyse, PBOMETICANALYZE p_eticanalyse)
{
	RETINFO ret				= {0};
	uint16_t err				= 0;
	uint16_t err1				= 0;
	long penalty_val			= 0;
	long penalty_val1			= 0;
	long lowest_val				= 0;
	uint32_t operation_status	= 0;
	char entry_mode				= 0;
	uint16_t tempEntryID		= 0;
	uint8_t btime[7]	= {0};
	uint8_t etime[4]	= {0};
	uint8_t timenow[7]	= {0};
	uint8_t btimenow[7]	= {0};
	///////////////////////////////////////////////////////
	/////跳出传统票卡的逻辑，只用来判断电子票卡的逻辑//////
	///////////////////////////////////////////////////////
	do
	{
	}
	while (0);

	return ret;
}

uint32_t TicketThirdPay::add_oper_status(uint8_t wk_area, RETINFO ret, TICKET_PARAM * p_ticket_prm)
{
	uint32_t oper_status = 0;

	if (ret.wErrCode != ERR_OVER_PERIOD_L && ret.wErrCode != ERR_CARD_STATUS &&
		ret.wErrCode != ERR_CARD_REFUND && ret.wErrCode != ERR_OVER_WALLET &&
		ret.wErrCode != ERR_CARD_LOCKED && ret.wErrCode != ERR_WITHOUT_SELL &&
		ret.wErrCode != ERR_OVER_PERIOD_P)
	{
		if (p_ticket_prm->permit_charge && m_ticket_data.wallet_value < m_ticket_data.wallet_value_max)
			oper_status |= ALLOW_CHARGE;
	}
	return oper_status;
}


void getDealDataThird(ETYTKOPER operType,uint16_t& len_data,uint8_t * dataBack,long amount)
{
	int length = 0;
	char StationCode[2+1] = {0};
	char LineCode[2+1] = {0};
	char DeviceType[2+1] = {0};
	char DeviceCode[3+1] = {0};
	char PsamCode[12+1] = {0};
	char time[14+1] = {0};

	char dealData[256] = {0};

	sprintf(StationCode, "%02x", ((Api::current_station_id >>8) & 0x00FF));
	sprintf(LineCode, "%02x", (Api::current_station_id & 0xFF));
	sprintf(DeviceType, "%02x", (uint8_t)(Api::current_device_type));
	sprintf(DeviceCode, "%03x", Api::current_device_id);
	sprintf(time, "%02x%02x%02x%02x%02x%02x%02x", CmdSort::m_time_now[0], CmdSort::m_time_now[1], CmdSort::m_time_now[2], CmdSort::m_time_now[3], CmdSort::m_time_now[4], CmdSort::m_time_now[5], CmdSort::m_time_now[6]);
	switch (operType)
	{
	case operEntry:
		{
			memcpy(dealData,"01",2);
			length += 2;
			//dbg_dumpmemory("dealData1:",dealData,length);
			memcpy(dealData + length, time, 14);
			length += 14;
			memcpy(dealData + length, "20181111000000", 14);
			length += 14;
			//dbg_dumpmemory("dealData2:",dealData,length);
			memcpy(dealData + length, StationCode, 2);//线路编码
			length += 2;
			//dbg_dumpmemory("dealData3:",dealData,length);
			memcpy(dealData + length, LineCode, 2);//站点编码
			length += 2;
			//dbg_dumpmemory("dealData4:",dealData,length);
			memcpy(dealData + length, DeviceType, 2);//设备类型
			length += 2;
			//dbg_dumpmemory("dealData5:",dealData,length);
			memcpy(dealData + length, DeviceCode, 3);//设备编号
			length += 3;
			//dbg_dumpmemory("dealData6:",dealData,length);
			//memcpy(dealData + length, g_saminf[0].sam_terminal_id, 12);
			memcpy(dealData + length, "112233445566", 12);//暂时不写
			length += 12;
			//dbg_dumpmemory("dealData8:",dealData,length);
			memcpy(dealData + length, "00", 2);
			length += 2;
			//dbg_dumpmemory("dealData9:",dealData,length);
			memcpy(dealData + length, "00000000", 8);
			length += 8;
			//dbg_dumpmemory("dealData10:",dealData,length);
			len_data = length;

		}
		break;
	case operExit:
		{
			memcpy(dealData,"02",2);
			length += 2;
			//dbg_dumpmemory("dealData1:",dealData,length);
			memcpy(dealData + length, time, 14);
			length += 14;
			//dbg_dumpmemory("dealData2:",dealData,length);
			memcpy(dealData + length, StationCode, 2);//线路编码
			length += 2;
			//dbg_dumpmemory("dealData3:",dealData,length);
			memcpy(dealData + length, LineCode, 2);//站点编码
			length += 2;
			//dbg_dumpmemory("dealData4:",dealData,length);
			memcpy(dealData + length, DeviceType, 2);//设备类型
			length += 2;
			//dbg_dumpmemory("dealData5:",dealData,length);
			memcpy(dealData + length, DeviceCode, 3);//设备编号
			length += 3;
			//dbg_dumpmemory("dealData6:",dealData,length);
			memcpy(dealData + length, "112233445566", 12);
			length += 12;
			sprintf(dealData + length,"%08X",amount);
			//dbg_formatvar("price:%08X",amount);
			//memcpy(dealData + length, "000001F4", 8);
			length += 8;

			//dbg_dumpmemory("dealData8:",dealData,length);
			memcpy(dealData + length, "00", 2);
			length += 2;
			//dbg_dumpmemory("dealData9:",dealData,length);
			memcpy(dealData + length, "00000000", 8);
			length += 8;
			//dbg_dumpmemory("dealData10:",dealData,length);
			len_data = length;

		}
		break;
    default:
        break;
	}
	memcpy(dataBack, (uint8_t * )dealData, length);

}


RETINFO TicketThirdPay::entry_gate(PENTRYGATE p_entry,PETICKETDEALINFO p_eticketdealinfo)
{
	RETINFO ret				= {0};
	long lSamSeq			= 0;
	uint16_t length = 0;


	//m_ReadInf.p_eTicket_entry_info = (uint8_t *)(ble::ble_getDealJson(operEntry,length).c_str());
	//dbg_formatvar("length:%d",length);
	//dbg_dumpmemory("p_eTicket_entry_info:",m_ReadInf.p_eTicket_entry_info,length);
	getDealDataThird(operEntry,length,m_ReadInf.p_eTicket_entry_info,m_ticket_data.qr_ticketdata.price);
	//dbg_formatvar("length:%d",length);
	//dbg_dumpmemory("p_eTicket_entry_info:",m_ReadInf.p_eTicket_entry_info,length);

	THIRD_METRO_ELECT_INF write_inf = file_need_write(operEntry, &m_ReadInf);

	m_ticket_data.init_status = MS_Entry;
	m_ticket_data.p_jtb_entry_info = &m_ticket_data.jtb_write_last_info;
	current_jtbtrade_info(*m_ticket_data.p_jtb_entry_info);

	ret = write_card(operEntry, write_inf.p_eTicket_entry_info, length, 0x09, 0, lSamSeq, NULL);

	transfer_edata_for_out(operEntry, p_eticketdealinfo);

	/*
	//编数据
	memcpy(p_eticketdealinfo->bBussinessDate,CmdSort::m_time_now,4);
	p_eticketdealinfo->bStatus = 0x07;
	p_eticketdealinfo->bMainType = 0x11;
	p_eticketdealinfo->bSubType = 0x00;
	p_eticketdealinfo->bChipType = 0x07;
	memcpy(p_eticketdealinfo->bLogicalID,"\x00\x11\x22\x33\x44\x55\x66\x77",8);
	memcpy(p_eticketdealinfo->bPhysicalID,"\xAA\xBB\xCC\xDD\xEE\xFF\x11\x22",8);
	p_eticketdealinfo->wTicketCount = 0x0001;
	memcpy(p_eticketdealinfo->cOperatorID,"0000000000",10);
	p_eticketdealinfo->lBOMShfitID = 1;
	memcpy(p_eticketdealinfo->bSAMID,"\x00\x00\xAA\xAA\xAA\xAA\xAA\xAA",8);
	p_eticketdealinfo->lSAMTrSeqNo = 1;
	memcpy(p_eticketdealinfo->dtDate,CmdSort::m_time_now,7);
	p_eticketdealinfo->bPaymentType = 0x01;
	p_eticketdealinfo->lBefBalance = 1000;
	p_eticketdealinfo->lDealNum = 0;
	p_eticketdealinfo->lTradeAmount = 200;
	p_eticketdealinfo->wReduceAmount = 0x0000;
	memcpy(p_eticketdealinfo->bCurrentStation,"\xBB\xBB\xBB\xBB",4);
	memcpy(p_eticketdealinfo->bLastStation,"\xCC\xCC\xCC\xCC",4);
	memcpy(p_eticketdealinfo->dtLastDate,CmdSort::m_time_now,7);
	memcpy(p_eticketdealinfo->bDevFlag,"\xDD\xDD\xDD\xDD",4);
	p_eticketdealinfo->lTerSeqNo = 1;
	p_eticketdealinfo->bTest = 0x01;
	memcpy(p_eticketdealinfo->bQRMileSerialNo,CmdSort::m_time_now,4);
	memcpy(p_eticketdealinfo->bAgmOverTimePen,CmdSort::m_time_now,4);
	memcpy(p_eticketdealinfo->bTac,CmdSort::m_time_now,4);
	*/

	ret.bRfu[0] = 0x11;
	ret.bRfu[1] = 0x00;

	return ret;
}

RETINFO TicketThirdPay::exit_gate(PPURSETRADE p_exit,PETICKETDEALINFO p_eticketdealinfo, MODE_EFFECT_FLAGS * p_degrade_falgs)
{
	RETINFO ret				= {0};
	long lSamSeq			= 0;
	uint8_t tacType			= 0x09;
	uint16_t length = 0;

	//m_ReadInf.p_eTicket_exit_info = (uint8_t *)(ble::ble_getDealJson(operExit,length).c_str());
	//dbg_formatvar("length:%d",length);
	//dbg_dumpmemory("p_eTicket_exit_info:",m_ReadInf.p_eTicket_exit_info,length);
	getDealDataThird(operExit,length,m_ReadInf.p_eTicket_exit_info,p_exit->lTradeAmount);

	THIRD_METRO_ELECT_INF write_inf = file_need_write(operExit, &m_ReadInf);

	m_ticket_data.init_status = MS_Exit;

	m_ticket_data.p_exit_info = &m_ticket_data.write_last_info;

	current_trade_info(*m_ticket_data.p_exit_info);
	//p_exit->lTradeAmount = 1;//测试用

	ret = write_card(operExit, write_inf.p_eTicket_exit_info, length,tacType, p_exit->lTradeAmount, lSamSeq, p_exit->cMACorTAC);

	transfer_edata_for_out(operExit, p_eticketdealinfo);
	/*
		//编数据
	memcpy(p_eticketdealinfo->bBussinessDate,CmdSort::m_time_now,4);
	p_eticketdealinfo->bStatus = 0x08;
	p_eticketdealinfo->bMainType = 0x11;
	p_eticketdealinfo->bSubType = 0x00;
	p_eticketdealinfo->bChipType = 0x07;
	memcpy(p_eticketdealinfo->bLogicalID,"\x00\x11\x22\x33\x44\x55\x66\x77",8);
	memcpy(p_eticketdealinfo->bPhysicalID,"\xAA\xBB\xCC\xDD\xEE\xFF\x11\x22",8);
	p_eticketdealinfo->wTicketCount = 0x0001;
	memcpy(p_eticketdealinfo->cOperatorID,"0000000000",10);
	p_eticketdealinfo->lBOMShfitID = 1;
	memcpy(p_eticketdealinfo->bSAMID,"\x00\x00\xAA\xAA\xAA\xAA\xAA\xAA",8);
	p_eticketdealinfo->lSAMTrSeqNo = 1;
	memcpy(p_eticketdealinfo->dtDate,CmdSort::m_time_now,7);
	p_eticketdealinfo->bPaymentType = 0x01;
	p_eticketdealinfo->lBefBalance = 1000;
	p_eticketdealinfo->lDealNum = 0;
	p_eticketdealinfo->lTradeAmount = 200;
	p_eticketdealinfo->wReduceAmount = 0x0000;
	memcpy(p_eticketdealinfo->bCurrentStation,"\xBB\xBB\xBB\xBB",4);
	memcpy(p_eticketdealinfo->bLastStation,"\xCC\xCC\xCC\xCC",4);
	memcpy(p_eticketdealinfo->dtLastDate,CmdSort::m_time_now,7);
	memcpy(p_eticketdealinfo->bDevFlag,"\xDD\xDD\xDD\xDD",4);
	p_eticketdealinfo->lTerSeqNo = 1;
	p_eticketdealinfo->bTest = 0x01;
	memcpy(p_eticketdealinfo->bQRMileSerialNo,CmdSort::m_time_now,4);
	memcpy(p_eticketdealinfo->bAgmOverTimePen,CmdSort::m_time_now,4);
	memcpy(p_eticketdealinfo->bTac,CmdSort::m_time_now,4);
	*/
	ret.bRfu[0] = 0x11;
	ret.bRfu[1] = 0x00;

	return ret;
}

RETINFO TicketThirdPay::esale(PSJTSALE p_sjt_sale, uint16_t& len_sjt, PETICKETDEALINFO p_eticketdealinfo, uint16_t& len_eticket_sale, TICKET_PARAM& ticket_prm)
{
	RETINFO ret				= {0};
	return ret;
}

RETINFO TicketThirdPay::thirdesale(PSJTSALE p_sjt_sale, uint16_t& len_sjt, PETICKETDEALINFO p_eticketdealinfo, uint16_t& len_eticket_sale, TICKET_PARAM& ticket_prm , char * cNetTranNumber)
{
	RETINFO ret				= {0};
	return ret;
}

RETINFO TicketThirdPay::sale(PSJTSALE p_sjt_sale, uint16_t& len_sjt, POTHERSALE p_svt_sale, uint16_t& len_svt, TICKET_PARAM& ticket_prm)
{
	RETINFO ret				= {0};
	long lSamSeq			= 0;
	len_sjt					= 0;
	len_svt					= 0;

	THIRD_METRO_ELECT_INF write_inf = file_need_write(operSvtSale, &m_ReadInf);

	m_ticket_data.init_status = MS_Sale;
	current_trade_info(m_ticket_data.write_last_info);

	// 锁定应用
	m_ticket_data.app_lock_flag = 0x01;
	// 有效期其实时间
	memcpy(m_ticket_data.logical_peroidS, CmdSort::m_time_now, sizeof(m_ticket_data.logical_peroidS));

	// 发售
	m_ticket_data.globle_status = EXTERN_STATUS_SALE;
	// 有效期时间
	m_ticket_data.effect_mins = ticket_prm.effect_time_from_sale;
	// 押金
	m_ticket_data.deposit = p_svt_sale->nAmount;
	// 余额上限
	m_ticket_data.wallet_value_max = ticket_prm.balance_max;

	//ret = write_card(operSvtSale, write_inf, 0x09, 0, lSamSeq, NULL);
	p_svt_sale->lSAMTrSeqNo = lSamSeq;
	if (ret.wErrCode == 0)
		len_svt = sizeof(*p_svt_sale);

	p_svt_sale->lBrokerage = ticket_prm.sale_brokerage;
	transfer_jtb_data_for_out(operSvtSale, p_svt_sale);
	p_svt_sale->bStatus = LIFE_SALE;

	if (ret.bNoticeCode == NTC_MUST_CONFIRM)
	{
		save_last_stack(operSvtSale, p_svt_sale, sizeof(OTHERSALE), &write_inf, sizeof(write_inf), true);
	}
	return ret;
}


RETINFO TicketThirdPay::getETicket(PGETETICKETRESPINFO p_geteticketrespinfo)
{
	RETINFO ret				= {0};
	long lSamSeq			= 0;
	uint16_t length = 0;
	uint8_t tacType			= 0x09;
	char		cMACorTAC[10] ={0};

	getDealDataThird(operGetETicket,length,m_ReadInf.p_eTicket_get_info,m_ticket_data.qr_ticketdata.price);

	//m_ReadInf.p_eTicket_get_info = (uint8_t *)(ble::ble_getDealJson(operGetETicket,length).c_str());
	//dbg_formatvar("length:%d",length);
	//dbg_dumpmemory("p_eTicket_get_info:",m_ReadInf.p_eTicket_get_info,length);

	THIRD_METRO_ELECT_INF write_inf = file_need_write(operGetETicket, &m_ReadInf);

	ret = write_card(operGetETicket, write_inf.p_eTicket_get_info, length, tacType,0, lSamSeq, cMACorTAC);

	transfer_edata_for_out(operGetETicket, p_geteticketrespinfo);

	ret.bRfu[0] = 0x11;
	ret.bRfu[1] = 0x00;

	return ret;
}


RETINFO TicketThirdPay::bom_update(PTICKETUPDATE p_update, uint8_t * p_entry_station)
{
	RETINFO ret				= {0};
	long lSamSeq			= 0;
	long fare				= 0;

	THIRD_METRO_ELECT_INF write_inf = file_need_write(operUpdate, &m_ReadInf);

	switch (p_update->bUpdateReasonCode)
	{
	case 0x01:		// 付费区超时
		m_ticket_data.init_status = MS_Upd_Wzone_Time;
		p_update->bStatus = LIFE_UPD_TM_OUT;
		dbg_formatvar("MS_Upd_Wzone_Time");
		//g_Record.log_out(0, level_disaster, "1 MS_Upd_Wzone_Time");
		break;
	case 0x02:		// 付费区超乘
		m_ticket_data.init_status = MS_Upd_Wzone_Trip;
		p_update->bStatus = LIFE_UPD_TP_OUT;
		dbg_formatvar("MS_Upd_Wzone_Trip");
		//g_Record.log_out(0, level_disaster, "2 MS_Upd_Wzone_Trip");
		break;
	case 0x03:		// 付费区无进站码
		m_ticket_data.p_entry_info->station_id = (p_entry_station[0] << 8) + p_entry_station[1];
		m_ticket_data.init_status = MS_Upd_Wzone_Entry;
		p_update->bStatus = LIFE_UPD_WO_STA;
		dbg_formatvar("MS_Upd_Wzone_Entry");
		//g_Record.log_out(0, level_disaster, "3 MS_Upd_Wzone_Entry");
		ret.wErrCode = g_Parameter.query_fare(p_update->dtUpdateDate,
			m_ticket_data.fare_type, m_ticket_data.p_entry_info->station_id, p_update->dtUpdateDate, &fare);
		if (ret.wErrCode == 0)
		{
			if (fare > m_ticket_data.wallet_value && p_update->nForfeiture < fare - m_ticket_data.wallet_value)
				ret.wErrCode = ERR_INPUT_PARAM;
		}

		break;
	case 0x10:		// 非付费免费更新
		m_ticket_data.init_status = MS_Upd_FZone_Free;
		p_update->bStatus = LIFE_UPD_OUT_FREE;
		dbg_formatvar("MS_Upd_FZone_Free");
		//g_Record.log_out(0, level_disaster, "4 MS_Upd_FZone_Free");
		break;
	case 0x11:
	case 0x12:		// 非付费区付费更新
		m_ticket_data.init_status = MS_Upd_FZone_Fare;
		p_update->bStatus = LIFE_UPD_OUT_FARE;
		dbg_formatvar("MS_Upd_FZone_Fare");
		//g_Record.log_out(0, level_disaster, "5 MS_Upd_FZone_Fare");
		break;
	default:
		ret.wErrCode = ERR_INPUT_PARAM;
		break;
	}

	if (ret.wErrCode == 0)
	{
		m_ticket_data.p_jtb_update_info = &m_ticket_data.jtb_write_last_info;
		current_jtbtrade_info(*m_ticket_data.p_jtb_update_info);

		m_ticket_data.p_update_info = &m_ticket_data.write_last_info;
		current_trade_info(*m_ticket_data.p_update_info);

		//ret = write_card(operUpdate, write_inf, 0x09, 0, lSamSeq, NULL);
		p_update->lSAMTrSeqNo = lSamSeq;

		// 交易记录赋值
		transfer_jtb_data_for_out(operUpdate, p_update);

		if (ret.bNoticeCode == NTC_MUST_CONFIRM)
		{
			save_last_stack(operUpdate, p_update, sizeof(TICKETUPDATE), &write_inf, sizeof(write_inf), true);
		}
	}

	return ret;
}

RETINFO TicketThirdPay::bom_refund(PDIRECTREFUND p_refund)
{
	RETINFO ret				= {0};
	return ret;
}

RETINFO TicketThirdPay::svt_increase(PPURSETRADE p_purse, uint8_t * p_time, uint8_t * p_mac2)
{
	RETINFO ret					= {0};
	return ret;
}

RETINFO TicketThirdPay::svt_decrease(PPURSETRADE p_purse, uint8_t u_pay_flag)
{
	RETINFO ret				= {0};
	long lSamSeq			= 0;
	uint8_t tacType			= 0x06;

	dbg_formatvar("TicketThirdPay::svt_decrease");

	if (p_purse->lTradeAmount > m_ticket_data.wallet_value)
	{
		ret.wErrCode = ERR_LACK_WALLET;
	}
	else
	{
		THIRD_METRO_ELECT_INF write_inf = file_need_write(operDecr, &m_ReadInf);
		m_ticket_data.wallet_value -= p_purse->lTradeAmount;

		//ret = write_card(operDecr, write_inf, tacType, p_purse->lTradeAmount, lSamSeq, p_purse->cMACorTAC);
		p_purse->lSAMTrSeqNo = lSamSeq;

		transfer_data_for_out(operDecr, p_purse);

		if (u_pay_flag == PAY_UPD_PENALTY)
			memcpy(p_purse->cPaymentType, "1A", sizeof(p_purse->cPaymentType));
		else if (u_pay_flag == PAY_INC_DESIND)
			memcpy(p_purse->cPaymentType, "18", sizeof(p_purse->cPaymentType));
		else if (u_pay_flag == PAY_BUY_TOKEN)
			memcpy(p_purse->cPaymentType, "19", sizeof(p_purse->cPaymentType));

		p_purse->bStatus = LIFE_DECREASE;
		Publics::hex_to_two_char(tacType, p_purse->cClassicType);

		if (ret.bNoticeCode == NTC_MUST_CONFIRM)
		{
			save_last_stack(operDecr, p_purse, sizeof(PURSETRADE), &write_inf, sizeof(write_inf), true);
		}
	}

	return ret;
}

THIRD_METRO_ELECT_INF TicketThirdPay::file_need_read(ETYTKOPER type, uint8_t * p_buffer)
{
	int ptr_buffer			= 0;
	THIRD_METRO_ELECT_INF ret_inf	= {0};

	ret_inf.p_eTicket_base_info = p_buffer + ptr_buffer;
	ptr_buffer += LENM_ETICKETBASE;

	switch (type)
	{
	case operEntry:
		ret_inf.p_eTicket_entry_info = p_buffer + ptr_buffer;
		ptr_buffer += LENM_ETICKETENTRY;
		break;
	case operExit:
		ret_inf.p_eTicket_exit_info = p_buffer + ptr_buffer;
		ptr_buffer += LENM_ETICKETEXIT;

		break;

	case operGetETicket:
		ret_inf.p_eTicket_get_info = p_buffer + ptr_buffer;
		ptr_buffer += LENM_ETICKETGET;

		break;
	case operThirdGetETicket:
		ret_inf.p_eTicket_get_info = p_buffer + ptr_buffer;
		ptr_buffer += LENM_ETICKETGET;

		break;

    default:
        break;
	}

	return ret_inf;
}

THIRD_METRO_ELECT_INF TicketThirdPay::file_need_write(ETYTKOPER type, P_THIRD_METRO_ELECT_INF p_read_inf)
{
	THIRD_METRO_ELECT_INF ret_inf = {0};

	switch (type)
	{
	case operEntry:
		{
			ret_inf.p_eTicket_entry_info = p_read_inf->p_eTicket_entry_info;
		}
		break;
	case operExit:
		{
			ret_inf.p_eTicket_exit_info = p_read_inf->p_eTicket_exit_info;
		}
		break;
	case operGetETicket:
		{
			ret_inf.p_eTicket_get_info = p_read_inf->p_eTicket_get_info;
		}
		break;
	default:
		break;
	}

	return ret_inf;
}

void TicketThirdPay::get_issue_base(uint8_t * p_issue_base)
{
	if (p_issue_base != NULL)
	{
		// 发卡方代码
		Publics::bcds_to_string(p_issue_base, 2, m_ticket_data.issue_code, 4);

		//dbg_formatvar("issue_code = %s",m_ticket_data.issue_code);

		// 城市代码、行业代码
		Publics::bcds_to_string(p_issue_base + 2, 2, m_ticket_data.city_code, 4);
		Publics::bcds_to_string(p_issue_base + 4, 2, m_ticket_data.industry_code, 4);

		// 应用标识
		m_ticket_data.key_flag = p_issue_base[8];

		//dbg_formatvar("key_flag = %02x",m_ticket_data.key_flag);

		// 逻辑卡号
		sprintf(m_ticket_data.logical_id, "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",
			p_issue_base[10], p_issue_base[11],p_issue_base[12], p_issue_base[13],p_issue_base[14],
			p_issue_base[15], p_issue_base[16], p_issue_base[17], p_issue_base[18], p_issue_base[19]);

		//dbg_formatvar("logical_id = %s",m_ticket_data.logical_id);

		// 卡类型
		memcpy(m_ticket_data.logical_type, p_issue_base + 28, 2);
		memcpy(m_ticket_data.fare_type, m_ticket_data.logical_type, 2);

		//dbg_formatvar("logical_type = %02x%02x",m_ticket_data.logical_type[0],m_ticket_data.logical_type[1]);

		// 发行日期
		memcpy(m_ticket_data.date_issue, p_issue_base + 20, 4);

		//dbg_dumpmemory("date_issue = ",m_ticket_data.date_issue,4);

		// 物理有效期
		memcpy(m_ticket_data.phy_peroidE, p_issue_base + 24, 4);
		//dbg_dumpmemory("phy_peroidE = ",m_ticket_data.phy_peroidE,4);
	}
}

void TicketThirdPay::get_public_base(uint8_t * p_public_base)
{
	if (p_public_base != NULL)
	{
		memcpy(p_public_base,"\x00\x01\x00\x01\xFF\xFF\xFF\xFF\x02\x01\x00\x11\x22\x33\x44\x55\x66\x77\x88\x99\x20\x18\x06\x15\x20\x20\x06\x15\x11\x00",30);

		// 发卡方代码
		Publics::bcds_to_string(p_public_base, 2, m_ticket_data.issue_code, 4);

		//dbg_formatvar("issue_code = %s",m_ticket_data.issue_code);

		// 城市代码、行业代码
		Publics::bcds_to_string(p_public_base + 2, 2, m_ticket_data.city_code, 4);
		Publics::bcds_to_string(p_public_base + 4, 2, m_ticket_data.industry_code, 4);

		// 应用标识
		m_ticket_data.key_flag = p_public_base[8];

		//dbg_formatvar("key_flag = %02x",m_ticket_data.key_flag);

		// 逻辑卡号
		sprintf(m_ticket_data.logical_id, "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",
			p_public_base[10], p_public_base[11],p_public_base[12], p_public_base[13],p_public_base[14],
			p_public_base[15], p_public_base[16], p_public_base[17], p_public_base[18], p_public_base[19]);

		//dbg_formatvar("logical_id = %s",m_ticket_data.logical_id);

		// 卡类型
		memcpy(m_ticket_data.logical_type, p_public_base + 28, 2);
		memcpy(m_ticket_data.logical_type, "\x10\x00", 2);//正式的时候需要改成1000
		memcpy(m_ticket_data.fare_type, m_ticket_data.logical_type, 2);

		//dbg_formatvar("logical_type = %02x%02x",m_ticket_data.logical_type[0],m_ticket_data.logical_type[1]);

		// 发行日期
		memcpy(m_ticket_data.date_issue, p_public_base + 20, 4);

		//dbg_dumpmemory("date_issue = ",m_ticket_data.date_issue,4);

		m_ticket_data.globle_status = EXTERN_STATUS_SALE;

		// 物理有效期
		memcpy(m_ticket_data.phy_peroidE, p_public_base + 24, 4);
		//dbg_dumpmemory("phy_peroidE = ",m_ticket_data.phy_peroidE,4);

		memcpy(m_ticket_data.logical_peroidS, p_public_base + 20, 4);

		memcpy(m_ticket_data.logical_peroidE, p_public_base + 24, 4);
	}
}

void TicketThirdPay::get_owner_base(uint8_t * p_owner_base)
{
	if (p_owner_base != NULL)
	{
		memset(p_owner_base,0x00,55);
		memcpy(m_ticket_data.certificate_name, p_owner_base + 2, sizeof(m_ticket_data.certificate_name));
		memcpy(m_ticket_data.certificate_code, p_owner_base + 22, sizeof(m_ticket_data.certificate_code));
		m_ticket_data.certificate_type = p_owner_base[54];
	}
}

void TicketThirdPay::get_manage_info(uint8_t * p_manage_info)
{
	if (p_manage_info != NULL)
	{
		memcpy(p_manage_info,"\x00\x00\x01\x56\x00\x00\x41\x00\x00\x01\x11",11);

		memcpy(m_ticket_data.international_code, p_manage_info, sizeof(m_ticket_data.international_code));
		memcpy(m_ticket_data.province_code, p_manage_info + 4, sizeof(m_ticket_data.province_code));
		memcpy(m_ticket_data.city_code_17, p_manage_info + 6, sizeof(m_ticket_data.city_code_17));
		memcpy(m_ticket_data.interCardFlag, p_manage_info + 8, sizeof(m_ticket_data.interCardFlag));
		m_ticket_data.jtbCardType = p_manage_info[10];

		//dbg_dumpmemory("international_code = ",m_ticket_data.international_code,4);
		//dbg_dumpmemory("province_code = ",m_ticket_data.province_code,2);
		//dbg_dumpmemory("city_code_17 = ",m_ticket_data.city_code_17,2);
		//dbg_dumpmemory("interCardFlag = ",m_ticket_data.interCardFlag,2);
		//dbg_formatvar("jtbCardType = %02x",m_ticket_data.jtbCardType);
	}
}

void TicketThirdPay::get_wallet(uint8_t * p_wallet)
{
	if (p_wallet != NULL)
	{
		m_ticket_data.wallet_value = 1000;
	}
}

void TicketThirdPay::get_his_all(uint8_t * p_his_all)
{
	if (p_his_all != NULL)
	{

	}
}

void TicketThirdPay::get_trade_assist(uint8_t * p_trade_assist)
{
	if (p_trade_assist != NULL)
	{
		m_ticket_data.list_falg = p_trade_assist[2];
	}
}

void TicketThirdPay::get_metro(uint8_t * p_metro)
{
	if (p_metro != NULL)
	{
	}
}

void TicketThirdPay::get_ctrl_record(uint8_t * p_ctrl_record)
{
	if (p_ctrl_record != NULL)
	{

		m_ticket_data.app_lock_flag = 0x01;
		memcpy(m_ticket_data.logical_peroidS, "\x20\x18\x06\x15\x10\x47\x00", 7);
		memcpy(m_ticket_data.logical_peroidE, "\x20\x20\x06\x15\x10\x47\x00", 7);
	}
}

void TicketThirdPay::get_app_ctrl(uint8_t * p_app_ctrl)
{
	// 由于涉及有效期，调用本函数必须在调用get_ctrl_record后面
	if (p_app_ctrl != NULL)
	{
	}
}

void TicketThirdPay::get_change_count(uint8_t * p_charge_count)
{
	if (p_charge_count != NULL)
	{
		m_ticket_data.charge_counter = 0;
	}
}

void TicketThirdPay::get_consume_count(uint8_t * p_consume_count)
{
	if (p_consume_count != NULL)
	{
		m_ticket_data.total_trade_counter = 0;
	}
}

RETINFO TicketThirdPay::write_card(ETYTKOPER operType, uint8_t * p_eTicket_info, uint16_t length, uint8_t tac_type, long trade_amount, long& sam_seq, char * p_trade_tac)
{
	RETINFO ret			= {0};
	uint8_t u_recv[64]	= {0};
	char sz_tac[11]		= {0};
	int result = -1;

	do
	{

		//ble::ble_handshake();

		result = ble::ble_all_oper(operType,m_ticket_data.qr_passcode, p_eTicket_info, length);
		if(result != 0)
		{
			g_Record.log_out(0, level_error,"============================if ble_writeBack faile ble close==================================");
			//ble::ble_close();
			ret.wErrCode = ERR_BLE_WRITEBACK;
			break;
		}

	} while (0);

	return ret;
}


RETINFO TicketThirdPay::write_card_back(ETYTKOPER operType, uint8_t * p_eTicket_info, uint16_t length, int sysFlag)
{
	RETINFO ret			= {0};
	uint8_t u_recv[64]	= {0};
	char sz_tac[11]		= {0};
	int result = -1;

	do
	{

		//ble::ble_handshake();

		result = ble::ble_all_oper(operType,Api::qr_passcode, p_eTicket_info, length);
		if(result != 0)
		{
			if (result == -3)
			{
				g_Record.log_out(0, level_error,"============================if ble_writeBack match ble failed==================================");
				ret.wErrCode = ERR_BLE_ADDRESS_MATCH;
				break;
			}
			g_Record.log_out(0, level_error,"============================if ble_writeBack faile ble close==================================");
			//ble::ble_close();
			ret.wErrCode = ERR_BLE_WRITEBACK;
			break;
		}


	} while (0);

	return ret;
}

uint16_t TicketThirdPay::write_card(uint8_t tac_type, uint8_t * p_app_ctrl, int record_id, uint8_t lock_flag, uint8_t * p_file_data, long& sam_seq)
{
	uint16_t ret				= ERR_SUCCEED;
	unsigned char u_recv[512]	= {0};

	do
	{

	} while (0);

	return ret;
}

uint16_t TicketThirdPay::set_trade_assist(uint8_t * p_trade_assist)
{
	uint16_t ret = 0;
	unsigned char u_recv[512] = {0};

	if (p_trade_assist != NULL)
	{

	}

	return ret;
}

uint16_t TicketThirdPay::set_metro(uint8_t tac_type, uint8_t * p_metro, uint32_t lValue, uint8_t * p_respond)
{
	if (p_metro != NULL)
	{

	}


	return 0;
}

uint16_t TicketThirdPay::set_ctrl_record(uint8_t * p_ctrl_record)
{
	if (p_ctrl_record != NULL)
	{
	}

	return 0;
}

uint16_t TicketThirdPay::set_app_ctrl(uint8_t * p_app_ctrl)
{
	uint16_t wallet_max = 0;

	if (p_app_ctrl != NULL)
	{
	}

	return 0;
}

void TicketThirdPay::get_qrInfo(uint8_t * p_grInfo)
{
	dbg_formatvar("get_qrInfo");
	int i;
	for(i = 0;i < 512; i++)
	{
		if(p_grInfo[i] == 0x0D)
		{
			dbg_formatvar("p_grInfo[%d]=[%02x]",i,p_grInfo[i]);
			memset(m_ticket_data.third_qr_info, 0x20, 64);
			memcpy(m_ticket_data.third_qr_info, p_grInfo + 2 + 7, (i-2-7));
			break;
		}
	}

	dbg_formatvar("third_qr_info = %s",m_ticket_data.third_qr_info);

	//g_Record.log_buffer("third_qr_info = ",m_ticket_data.third_qr_info,64);
	//g_Record.log_out(0, level_disaster, "third_qr_info=(%s)", m_ticket_data.third_qr_info);

}

void TicketThirdPay::get_bleInfo(uint8_t * p_bleInfo)
{
	Json::Reader reader;
	Json::Value json_object;
	Json::Value json_object1;

	dbg_formatvar("create json");

	if (!reader.parse((char *)p_bleInfo, json_object))//字符串inbuf，json格式化
	{
		dbg_formatvar("json_object error!");
	}
	else
	{

		dbg_formatvar("json_object sucss");
		string jsonEncMode=json_object["encMode"].asString();
		dbg_formatvar("jsonEncMode=%s",jsonEncMode.c_str());
		string jsonLength=json_object["length"].asString();
		dbg_formatvar("length=%s",jsonLength.c_str());
		string jsonResult=json_object["result"].asString();
		dbg_formatvar("result=%s",jsonResult.c_str());
		string jsonType=json_object["type"].asString();
		dbg_formatvar("type=%s",jsonType.c_str());
		string jsonVersion=json_object["version"].asString();
		dbg_formatvar("version=%s",jsonVersion.c_str());


		json_object1 = json_object["ticket"];
		dbg_formatvar("ticket");

		m_ticket_data.ble_ticketdata.balance=(long)json_object1["balance"].asUInt();
		dbg_formatvar("balance=%ld",m_ticket_data.ble_ticketdata.balance);
		dbg_formatvar("cardNo=%s",json_object1["cardNo"].asString().c_str());

		strcpy(m_ticket_data.ble_ticketdata.cardNo,json_object1["cardNo"].asString().c_str());
		//memcpy(&m_ticket_data.ble_ticketdata.cardNo[0], (char *)(json_object1["cardNo"].asString().c_str()), 20);
		dbg_formatvar("cardNo=%s",m_ticket_data.ble_ticketdata.cardNo);

		m_ticket_data.ble_ticketdata.subType=(int)json_object1["subType"].asInt();
		dbg_formatvar("subType=%d",m_ticket_data.ble_ticketdata.subType);

		m_ticket_data.ble_ticketdata.mainType=(int)json_object1["mainType"].asInt();
		dbg_formatvar("mainType=%d",m_ticket_data.ble_ticketdata.mainType);

		m_ticket_data.ble_ticketdata.payAmount=(long)json_object1["payAmount"].asUInt();
		dbg_formatvar("payAmount=%ld",m_ticket_data.ble_ticketdata.payAmount);

		memcpy(&m_ticket_data.ble_ticketdata.terminalStation, json_object1["terminalStation"].asString().c_str(), 4);
		dbg_formatvar("terminalStation=%s",m_ticket_data.ble_ticketdata.terminalStation);

		memcpy(&m_ticket_data.ble_ticketdata.startStation, json_object1["startStation"].asString().c_str(), 4);
		dbg_formatvar("startStation=%s",m_ticket_data.ble_ticketdata.startStation);

		memcpy(m_ticket_data.logical_type, "\x10\x00", 2);//先当成1000的票卡获取票卡参数
		memcpy(m_ticket_data.fare_type, m_ticket_data.logical_type, 2);

    }


	Json::FastWriter fast_writer;
	string jsonRoot = fast_writer.write(json_object);
	dbg_formatvar("jsonRoot=%s",jsonRoot.c_str());

}

// 将状态格式化为简化状态
SimpleStatus TicketThirdPay::get_simple_status(uint8_t init_status)
{
	SimpleStatus ret = SStatus_Invalid;

	if (m_ticket_data.globle_status == EXTERN_STATUS_SALE)
	{
		switch (init_status)
		{
		case MS_Init:
			ret = SStatus_Init;
			break;
		case MS_Es:
			ret = SStatus_Es;
			break;
		case MS_Sale:
			ret = SStatus_Sale;
			break;
		case MS_Entry:
			ret = SStatus_Entry;
			break;
		case MS_Exit:
			ret = SStatus_Exit;
			break;
		case MS_Exit_Tt:
			ret = SStatus_Exit_Tt;
			break;
		case MS_Upd_FZone_Free:
		case MS_Upd_FZone_Fare:
			ret = SStatus_Upd_FZone;
			break;
		case MS_Upd_Wzone_Entry:
		case MS_Upd_Wzone_Exit:
		case MS_Upd_Wzone_Time:
		case MS_Upd_Wzone_Trip:
		case MS_Upd_Wzone_Free:
			ret = SStatus_Upd_WZone;
			break;
		case MS_Refund:
			ret = SStatus_Refund;
			break;
		}
	}
	else if (m_ticket_data.globle_status == EXTERN_STATUS_INIT)
	{
		ret = SStatus_Init;
	}
	else if (m_ticket_data.globle_status == EXTERN_STATUS_RFND)
	{
		ret = SStatus_Refund;
	}

	return ret;
}

RETINFO TicketThirdPay::format_history(uint8_t&	his_count, PHSSVT p_his_array, int his_max)
{
	RETINFO ret	= {0};
	HSSVT his_all[30];
	HSSVT his_temp;

	his_count = 0;
	memset(his_all, 0, sizeof(his_all));

	if (m_ReadInf.p_his_all != NULL)
	{
		for (int i=0;i<30;i++)
		{
			if (format_history(m_ReadInf.p_his_all + 23 * i, his_all[his_count]))
				his_count++;
		}

		for (int i=0;i<his_count;i++)
		{
			for (int j=i+1;j<his_count;j++)
			{
				if (memcmp(his_all[i].dtDate, his_all[j].dtDate, 7) < 0)
				{
					memcpy(&his_temp, his_all + i, sizeof(HSSVT));
					memcpy(his_all + i, his_all + j, sizeof(HSSVT));
					memcpy(his_all + j, &his_temp, sizeof(HSSVT));
				}
			}
		}

		if (his_count > his_max)
			his_count = his_max;

		memcpy(p_his_array, his_all, sizeof(HSSVT) * his_count);
	}
	return ret;
}

bool TicketThirdPay::format_history(uint8_t * p_his_buffer, HSSVT& his)
{
	char szSam[32]		= {0};
	char szTerminal[32] = {0};

	// 交易时间（BCD码)
	memcpy(his.dtDate, p_his_buffer + 16, sizeof(his.dtDate));

	// 票卡生命周期索引,具体定义见附录五
	if (p_his_buffer[9] == 0x02)
		his.bStatus = LIFE_CHARGE;
	else if (p_his_buffer[9] == 0x06)
		his.bStatus = LIFE_CONSUME_COMMON;
	else
		his.bStatus = LIFE_CONSUME_COMPOUND;

	// 交易金额,单位分
	his.lTradeAmount = (p_his_buffer[5] << 24) + (p_his_buffer[6] << 16) + (p_his_buffer[7] << 8) + p_his_buffer[8];

	// sam终端id
	sprintf(szTerminal, "0000%02X%02X%02X%02X%02X%02X",
		p_his_buffer[10], p_his_buffer[11], p_his_buffer[12], p_his_buffer[13], p_his_buffer[14], p_his_buffer[15]);
	memcpy(his.cSAMID, szTerminal, sizeof(his.cSAMID));

	// sam卡号
	sprintf(szSam, "%02X%02X0003%02X%02X%02X%02X",
		p_his_buffer[10], p_his_buffer[11], p_his_buffer[12], p_his_buffer[13], p_his_buffer[14], p_his_buffer[15]);

	g_Parameter.device_from_sam(szSam, SAM_SOCK_1, his.bStationID, &his.bDeviceType, his.bDeviceID);

	return TimesEx::bcd_time_valid(his.dtDate, T_TIME);
}

RETINFO TicketThirdPay::lock(PTICKETLOCK p_lock)
{
	RETINFO ret				= {0};
	long lSamSeq			= 0;

	THIRD_METRO_ELECT_INF write_inf = file_need_write(operLock, &m_ReadInf);

	m_ticket_data.list_falg = '2'- p_lock->cLockFlag;

	if (m_ticket_data.list_falg == 0)
	{
		//ret.wErrCode = write_card(0x06, NULL, 0x01, m_ticket_data.list_falg, write_inf.p_metro, lSamSeq);
	}
	else
	{
		//ret = write_card(operLock, write_inf, 0x09, 0, lSamSeq, NULL);
	}

	if (ret.bNoticeCode == NTC_MUST_CONFIRM)
		ret.bNoticeCode = 0;

	p_lock->lSAMTrSeqNo = lSamSeq;

	transfer_jtb_data_for_out(operLock, p_lock);
	if (m_ticket_data.list_falg == 1)
		p_lock->bStatus = LIFE_LOCKED;
	else
		p_lock->bStatus = LIFE_UNLOCKED;

	return ret;
}

RETINFO TicketThirdPay::deffer(PTICKETDEFER p_deffer, TICKET_PARAM& ticket_prm)
{
	RETINFO ret				= {0};
	long lSamSeq			= 0;

	THIRD_METRO_ELECT_INF write_inf = file_need_write(operDeffer, &m_ReadInf);

	memcpy(m_ticket_data.logical_peroidS, CmdSort::m_time_now, 7);
	m_ticket_data.effect_mins = ticket_prm.deffer_days * 24 * 60;

	if (write_inf.p_ctrl_record != NULL)
		memcpy(write_inf.p_ctrl_record + 4, m_ticket_data.logical_peroidS, 7);

	//ret.wErrCode = write_card(0x06, write_inf.p_app_ctrl, 0x11, write_inf.p_ctrl_record[2], write_inf.p_ctrl_record, lSamSeq);
	if (ret.bNoticeCode == NTC_MUST_CONFIRM)
		ret.bNoticeCode = 0;

	p_deffer->lSAMTrSeqNo = lSamSeq;

	transfer_jtb_data_for_out(operDeffer, p_deffer);
	p_deffer->bStatus = LIFE_DEFFER;

	return ret;
}

RETINFO TicketThirdPay::set_complex_file(ETYTKOPER operType,uint8_t tac_type, long lAmount,uint8_t * p_metro,uint8_t * p_recv)
{
	int pos_file_buf			= 0;
	unsigned char u_count		= 0;
	unsigned char u_rid[8]		= {0};
	unsigned char u_lock[8]		= {0};
	unsigned char u_send[512]	= {0};
	RETINFO ret					= {0};
	uint8_t his_record[48]       = {0};
	long label_val				= 0;

	dbg_formatvar("TicketJtb::set_complex_file - 1");

	if (p_metro != NULL)
	{
		dbg_formatvar("TicketJtb::set_complex_file - 2");

		switch (operType)
		{
			case operUpdate:
				{
					if((m_ticket_data.init_status == MS_Upd_Wzone_Time)||(m_ticket_data.init_status == MS_Upd_Wzone_Trip))
					{
						p_metro[14] = 0x04;//交易状态为出站更新;
						p_metro[43] = (uint8_t)(Api::current_station_id >> 8);
						p_metro[44] = (uint8_t)(Api::current_station_id & 0xFF);
					}
					else if((m_ticket_data.init_status == MS_Upd_FZone_Free)||(m_ticket_data.init_status == MS_Upd_FZone_Fare))
					{
						p_metro[14] = 0x03;//交易状态为进站更新;
					}
					else if((m_ticket_data.init_status == MS_Upd_Wzone_Entry))
					{
						p_metro[14] = 0x04;//交易状态为出站更新;
						p_metro[35] = (uint8_t)(m_ticket_data.p_entry_info->station_id >> 8);
						p_metro[36] = (uint8_t)(m_ticket_data.p_entry_info->station_id & 0xFF);
					}
					p_metro[81] = ((label_val >> 24) & 0xFF);
					p_metro[82] = ((label_val >> 16) & 0xFF);
					p_metro[83] = ((label_val >> 8) & 0xFF);
					p_metro[84] = (label_val & 0xFF);
				}
				break;
			case operEntry:
				{
					p_metro[14] = 0x01;//交易状态为进闸;
					Publics::string_to_bcds(m_ticket_data.p_jtb_entry_info->city_code, 4, &p_metro[15], 2);
					memcpy(p_metro + 19, m_ticket_data.p_jtb_entry_info->institutionCode, 8);
					p_metro[35] = (uint8_t)(m_ticket_data.p_jtb_entry_info->station_id >> 8);
					p_metro[36] = (uint8_t)(m_ticket_data.p_jtb_entry_info->station_id & 0xFF);

					//Publics::string_to_bcds(m_ticket_data.p_jtb_entry_info->sam, 16, &p_metro[51], 8);
					memcpy(p_metro + 67, m_ticket_data.p_jtb_entry_info->time, sizeof(m_ticket_data.p_jtb_entry_info->time));

					//获取本地最低票价
					g_Parameter.query_lowest_fare(CmdSort::m_time_now, m_ticket_data.logical_type, label_val);
					//dbg_formatvar("label_val |= %d",label_val);
					p_metro[81] = ((label_val >> 24) & 0xFF);
					p_metro[82] = ((label_val >> 16) & 0xFF);
					p_metro[83] = ((label_val >> 8) & 0xFF);
					p_metro[84] = (label_val & 0xFF);

					p_metro[85] = m_ticket_data.p_jtb_entry_info->line_id;
					p_metro[87] = ((lAmount >> 24) & 0xFF);
					p_metro[88] = ((lAmount >> 16) & 0xFF);
					p_metro[89] = ((lAmount >> 8) & 0xFF);
					p_metro[90] = (lAmount & 0xFF);
					p_metro[91] = ((m_ticket_data.wallet_value >> 24) & 0xFF);
					p_metro[92] = ((m_ticket_data.wallet_value >> 16) & 0xFF);
					p_metro[93] = ((m_ticket_data.wallet_value >> 8) & 0xFF);
					p_metro[94] = (m_ticket_data.wallet_value & 0xFF);
				}
				break;
			case operExit:
				{
					p_metro[14] = 0x02;//交易状态为出闸;
					Publics::string_to_bcds(m_ticket_data.p_jtb_exit_info->city_code, 4, &p_metro[17], 2);
					memcpy(p_metro + 27, m_ticket_data.p_jtb_exit_info->institutionCode, 8);
					p_metro[43] = (uint8_t)(m_ticket_data.p_jtb_exit_info->station_id >> 8);
					p_metro[44] = (uint8_t)(m_ticket_data.p_jtb_exit_info->station_id & 0xFF);

					//Publics::string_to_bcds(m_ticket_data.p_jtb_exit_info->sam, 16, &p_metro[59], 8);

					memcpy(p_metro + 74, m_ticket_data.p_jtb_exit_info->time, sizeof(m_ticket_data.p_jtb_exit_info->time));

					p_metro[81] = ((label_val >> 24) & 0xFF);
					p_metro[82] = ((label_val >> 16) & 0xFF);
					p_metro[83] = ((label_val >> 8) & 0xFF);
					p_metro[84] = (label_val & 0xFF);

					p_metro[86] = m_ticket_data.p_jtb_exit_info->line_id;
					p_metro[95] = ((lAmount >> 24) & 0xFF);
					p_metro[96] = ((lAmount >> 16) & 0xFF);
					p_metro[97] = ((lAmount >> 8) & 0xFF);
					p_metro[98] = (lAmount & 0xFF);
				}
				break;
		}
		/*if (lAmount == 0)
		{
			p_metro[14] = 0x01;//交易状态为进闸;
			Publics::string_to_bcds(m_ticket_data.p_jtb_entry_info->city_code, 4, &p_metro[15], 2);
			memcpy(p_metro + 19, m_ticket_data.p_jtb_entry_info->institutionCode, 8);
			p_metro[35] = (uint8_t)(m_ticket_data.p_jtb_entry_info->station_id >> 8);
			p_metro[36] = (uint8_t)(m_ticket_data.p_jtb_entry_info->station_id & 0xFF);

			//Publics::string_to_bcds(m_ticket_data.p_jtb_entry_info->sam, 16, &p_metro[51], 8);
			memcpy(p_metro + 67, m_ticket_data.p_jtb_entry_info->time, sizeof(m_ticket_data.p_jtb_entry_info->time));

			//获取本地最低票价
			g_Parameter.query_lowest_fare(CmdSort::m_time_now, m_ticket_data.logical_type, Api::current_station_id, label_val);
			dbg_formatvar("label_val |= %d",label_val);
			p_metro[81] = ((label_val >> 24) & 0xFF);
			p_metro[82] = ((label_val >> 16) & 0xFF);
			p_metro[83] = ((label_val >> 8) & 0xFF);
			p_metro[84] = (label_val & 0xFF);

			p_metro[85] = m_ticket_data.p_jtb_entry_info->line_id;
			p_metro[87] = ((lAmount >> 24) & 0xFF);
			p_metro[88] = ((lAmount >> 16) & 0xFF);
			p_metro[89] = ((lAmount >> 8) & 0xFF);
			p_metro[90] = (lAmount & 0xFF);
			p_metro[91] = ((m_ticket_data.wallet_value >> 24) & 0xFF);
			p_metro[92] = ((m_ticket_data.wallet_value >> 16) & 0xFF);
			p_metro[93] = ((m_ticket_data.wallet_value >> 8) & 0xFF);
			p_metro[94] = (m_ticket_data.wallet_value & 0xFF);
		}else{
			p_metro[14] = 0x02;//交易状态为出闸;
			Publics::string_to_bcds(m_ticket_data.p_jtb_exit_info->city_code, 4, &p_metro[17], 2);
			memcpy(p_metro + 27, m_ticket_data.p_jtb_exit_info->institutionCode, 8);
			p_metro[43] = (uint8_t)(m_ticket_data.p_jtb_exit_info->station_id >> 8);
			p_metro[44] = (uint8_t)(m_ticket_data.p_jtb_exit_info->station_id & 0xFF);

			//Publics::string_to_bcds(m_ticket_data.p_jtb_exit_info->sam, 16, &p_metro[59], 8);

			memcpy(p_metro + 74, m_ticket_data.p_jtb_exit_info->time, sizeof(m_ticket_data.p_jtb_exit_info->time));

			p_metro[81] = ((label_val >> 24) & 0xFF);
			p_metro[82] = ((label_val >> 16) & 0xFF);
			p_metro[83] = ((label_val >> 8) & 0xFF);
			p_metro[84] = (label_val & 0xFF);

			p_metro[86] = m_ticket_data.p_jtb_exit_info->line_id;
			p_metro[95] = ((lAmount >> 24) & 0xFF);
			p_metro[96] = ((lAmount >> 16) & 0xFF);
			p_metro[97] = ((lAmount >> 8) & 0xFF);
			p_metro[98] = (lAmount & 0xFF);
		}*/
	}

	dbg_formatvar("TicketJtb::set_complex_file - 3");

	if(p_metro != NULL)
	{

		dbg_formatvar("TicketJtb::set_complex_file - 4");

		if (lAmount == 0)
		{
			his_record[0] = 0x03;
			Publics::string_to_bcds(m_ticket_data.p_jtb_entry_info->sam, 16, &his_record[1], 8);
			his_record[9] = 0x01;//行业代码，01-地铁
			his_record[10] = (uint8_t)(m_ticket_data.p_jtb_entry_info->station_id >> 8);
			his_record[12] = (uint8_t)(m_ticket_data.p_jtb_entry_info->station_id & 0xFF);

			his_record[17] = ((lAmount >> 24) & 0xFF);
			his_record[18] = ((lAmount >> 16) & 0xFF);
			his_record[19] = ((lAmount >> 8) & 0xFF);
			his_record[20] = (lAmount & 0xFF);

			his_record[21] = ((m_ticket_data.wallet_value >> 24) & 0xFF);
			his_record[22] = ((m_ticket_data.wallet_value >> 16) & 0xFF);
			his_record[23] = ((m_ticket_data.wallet_value >> 8) & 0xFF);
			his_record[24] = (m_ticket_data.wallet_value & 0xFF);

			memcpy(his_record + 25, m_ticket_data.p_jtb_entry_info->time, sizeof(m_ticket_data.p_jtb_entry_info->time));

			Publics::string_to_bcds(m_ticket_data.p_jtb_entry_info->city_code, 4, &his_record[32], 2);
			memcpy(his_record + 34, m_ticket_data.p_jtb_entry_info->institutionCode, 8);
		}else{
			his_record[0] = 0x04;
			Publics::string_to_bcds(m_ticket_data.p_jtb_exit_info->sam, 16, &his_record[1], 8);
			his_record[9] = 0x01;//行业代码，01-地铁
			his_record[10] = (uint8_t)(m_ticket_data.p_jtb_exit_info->station_id >> 8);
			his_record[12] = (uint8_t)(m_ticket_data.p_jtb_exit_info->station_id & 0xFF);

			his_record[17] = ((lAmount >> 24) & 0xFF);
			his_record[18] = ((lAmount >> 16) & 0xFF);
			his_record[19] = ((lAmount >> 8) & 0xFF);
			his_record[20] = (lAmount & 0xFF);

			//dbg_formatvar("wallet_value |= %ld",m_ticket_data.wallet_value);
			//dbg_formatvar("lAmount |= %ld",lAmount);

			his_record[21] = ((m_ticket_data.wallet_value >> 24) & 0xFF);
			his_record[22] = ((m_ticket_data.wallet_value >> 16) & 0xFF);
			his_record[23] = ((m_ticket_data.wallet_value >> 8) & 0xFF);
			his_record[24] = (m_ticket_data.wallet_value & 0xFF);

			memcpy(his_record + 25, m_ticket_data.p_jtb_exit_info->time, sizeof(m_ticket_data.p_jtb_exit_info->time));

			Publics::string_to_bcds(m_ticket_data.p_jtb_exit_info->city_code, 4, &his_record[32], 2);
			memcpy(his_record + 34, m_ticket_data.p_jtb_exit_info->institutionCode, 8);
		}
	}

	ret.wErrCode = jtb_updata_more_complex(operType,tac_type, CmdSort::m_time_now,
		SAM_SOCK_4, m_ReadInf.p_public_base, p_metro,his_record, lAmount, 0,(unsigned char *)p_recv);
	if (ret.wErrCode != 0)
	{
		if (ret.wErrCode == 4)	// 消费并产生TAC时出错
			ret.bNoticeCode = NTC_MUST_CONFIRM;
		ret.wErrCode = ERR_CARD_WRITE;
	}

	return ret;
}

void TicketThirdPay::save_last_stack(ETYTKOPER operType, void * p_trade, size_t size_trade, void * p_written_inf, size_t size_written, bool save_to_file)
{
	METRO_CPU_INF_W written_info = {0};
	THIRD_METRO_ELECT_INF * p_old_write = (THIRD_METRO_ELECT_INF *)p_written_inf;

	if (p_written_inf == NULL)
	{
		TicketBase::save_last_stack(operType, p_trade, size_trade, NULL, 0, save_to_file);
	}
	else
	{
		if (p_old_write->p_trade_assist != NULL)
		{
			written_info.flag_trade_assist = 1;
			memcpy(written_info.w_trade_assist, p_old_write->p_trade_assist, LENM_TRADE_ASSIST);
		}

		if (p_old_write->p_metro != NULL)
		{
			written_info.flag_metro = 1;
			memcpy(written_info.w_metro, p_old_write->p_metro, LENM_METRO);
		}

		if (p_old_write->p_ctrl_record != NULL)
		{
			written_info.flag_ctrl_record = 1;
			memcpy(written_info.w_ctrl_record, p_old_write->p_ctrl_record, LENM_CTRL_RECORD);
		}

		if (p_old_write->p_app_ctrl != NULL)
		{
			written_info.flag_app_ctrl = 1;
			memcpy(written_info.w_app_ctrl, p_old_write->p_app_ctrl, LENM_APP_CTRL);
		}

		TicketBase::save_last_stack(operType, p_trade, size_trade, &written_info, sizeof(written_info), save_to_file);
	}
}

bool TicketThirdPay::last_trade_need_continue(uint8_t status_targ, uint16_t err_targ, uint16_t err_src)
{
	bool ret = false;

	METRO_CPU_INF_W * p_old_write = (METRO_CPU_INF_W *)cfm_point.dataWritten;

	do
	{
		CONFIRM_FACTOR factor = get_confirm_factor();

		if (!check_confirm(status_targ, m_ticket_data.init_status, err_targ, err_src))
			break;

		// 直接判断复合消费是否成功，如果成功则需要继续去流水和TAC，否则认为失败
		if (p_old_write->flag_trade_assist > 0)
		{
			if (memcmp(p_old_write->w_trade_assist, m_ReadInf.p_trade_assist, LENM_TRADE_ASSIST) != 0)
				break;
		}

		if (p_old_write->flag_metro > 0)
		{
			if (memcmp(p_old_write->w_metro, m_ReadInf.p_metro, LENM_METRO) != 0)
				break;
		}

		if (p_old_write->flag_ctrl_record > 0)
		{
			if (memcmp(p_old_write->w_ctrl_record, m_ReadInf.p_ctrl_record, LENM_CTRL_RECORD) != 0)
				break;
		}

		if (cfm_point.balance != m_ticket_data.wallet_value)
			break;

		if (cfm_point.operType == operIncr)
		{
			if (m_ticket_data.charge_counter != cfm_point.charge_counter + 1 ||	cfm_point.consume_counter != m_ticket_data.total_trade_counter)
				break;
		}
		else
		{
			if (m_ticket_data.charge_counter != cfm_point.charge_counter ||	m_ticket_data.total_trade_counter != cfm_point.consume_counter + 1)
				break;
		}

		ret = true;

	} while (0);

    return ret;
}

RETINFO TicketThirdPay::continue_last_trade(void * p_trade)
{
	RETINFO ret			= {0};
	char sz_tac[11]		= {0};
	uint8_t u_recv[32]	= {0};
	uint16_t counter	= 0;

	do
	{
		CONFIRM_FACTOR factor = get_confirm_factor();

		if (cfm_point.operType == operIncr)
		{
			counter = (uint16_t)(m_ticket_data.charge_counter);
		}
		else if (cfm_point.operType == operDecr)
		{
			counter = (uint16_t)(m_ticket_data.total_trade_counter);
		}
		else
		{
			counter = (uint16_t)(m_ticket_data.total_trade_counter);
		}

		// 要用本次的交易计数
		if (get_cpu_last_tac(counter, factor.tac_type, u_recv) != 0)
		{
			ret.wErrCode = ERR_CARD_WRITE;
			break;
		}

		sprintf(sz_tac, "%02X%02X%02X%02X  ", u_recv[0], u_recv[1], u_recv[2], u_recv[3]);

		set_confirm_factor(LIFE_CONFIRM, 0, sz_tac);

		memcpy(p_trade, &cfm_point.trade, cfm_point.size_trade);

	} while (0);

	g_Record.log_out(ret.wErrCode, level_error, "continue_last_trade(void * p_trade)=%d", ret.wErrCode);

	return ret;
}

// 赋值给上次交易未赋值的交易因子
//void TicketSvt::set_confirm_factor(uint8_t status, long sam_seq, char * p_tac)
//{
//	uint8_t trade_type = Publics::string_to_hex<uint8_t>((char *)(&cfm_point.trade), 2);
//	switch(trade_type)
//	{
//	//case 0x50:	// 单程票发售
//	//	cfm_point.trade.sjtSale.bStatus		= status;
//	//	memcpy(cfm_point.trade.sjtSale.cMACorTAC, p_tac, 10);
//	//	break;
//	case 0x51:	// 储值票发售
//		cfm_point.trade.svtSale.bStatus		= status;
//		break;
//	case 0x53:	// 进闸
//		cfm_point.trade.entry.bStatus		= status;
//		break;
//	case 0x54:	// 钱包交易
//		cfm_point.trade.purse.bStatus		= status;
//		memcpy(cfm_point.trade.purse.cMACorTAC, p_tac, 10);
//		break;
//	case 0x56:	// 更新
//		cfm_point.trade.update.bStatus		= status;
//		break;
//	case 0x57:	// 退款
//		cfm_point.trade.refund.bStatus		= status;
//		memcpy(cfm_point.trade.refund.cMACOrTAC, p_tac, 10);
//		break;
//	}
//}

RETINFO TicketThirdPay::data_version_upgrade()
{
	RETINFO ret		            = {0};
	unsigned char u_recv[256]	= {0};

	if (svt_unlock_recordfile(SAM_SOCK_1, m_ReadInf.p_issue_base + 8, 0x11, 0x00, u_recv) != 0)
		ret.wErrCode = ERR_CARD_WRITE;

	if (svt_consum(CmdSort::m_time_now, SAM_SOCK_1, m_ReadInf.p_issue_base + 8, m_ticket_data.wallet_value, 0x06, u_recv, u_recv) != 0)
		ret.wErrCode = ERR_CARD_WRITE;

	return ret;
}

uint16_t TicketThirdPay::read_wallet(long * p_wallet)
{
	uint16_t ret			= 0;
	unsigned char tmp[256]	= {0};
	ETPMDM physical_type	= mediNone;
	unsigned char type		= 0;

	do
	{
		if (ISO14443A_Init(g_ant_type) != 0)
		{
			ret = ERR_CARD_NONE;
			break;
		}

		ret = search_card_ex(tmp, type);
		if (ret != 0)	break;

		physical_type = (ETPMDM)type;
		if (physical_type != mediMetroCpu)
		{
			ret = ERR_DIFFRENT_CARD;
			break;
		}

		// 选择主目录
		if (svt_selectfile(0x3F00) < 0)
		{
			ret = ERR_CARD_READ;
			break;
		}

		// 读发行基本信息
		if (svt_readbinary(0x05, 0, LENM_ISSUE_BASE, tmp) < 0)
		{
			ret = ERR_CARD_READ;
			break;
		}

		// 选择ADF1
		if (svt_selectfile(0x1001) < 0)
		{
			if (svt_selecfileaid(9, (unsigned char *)"\xA0\x00\x00\x00\x03\x86\x98\x07\x01") < 0)
			{
				ret = ERR_CARD_READ;
				break;
			}
		}

		if (svt_getbalance(tmp,0x00) < 0)
		{
			ret = ERR_CARD_READ;
			break;
		}

		*p_wallet = (tmp[0] << 24) + (tmp[1] << 16) + (tmp[2] << 8) + tmp[3];

	} while (0);

	return ret;
}
