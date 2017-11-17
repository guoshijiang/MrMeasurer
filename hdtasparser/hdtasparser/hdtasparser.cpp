#include "hdtasparser.h"
#include "protocol_data_parser.h"

namespace hdtas
{
	int HdtasPackage::GetMMrIDFromPackage(const unsigned char* buffer, size_t len, device_mr_id& id)
	{
		return po_package2::get_mmr_id_from_buffer(buffer, len, id);
	}

	class HdtasPackage::package_in
	{
	public:
		package_in() { this->pkg_ = new po_package2(); }

		virtual ~package_in() { delete this->pkg_; }

		po_package2* pkg_;
	};

	HdtasPackage::HdtasPackage()
	{
		this->pkg_in_ = new HdtasPackage::package_in();
		this->pkg_in_->pkg_->initialize();
	}
	
	HdtasPackage::~HdtasPackage()
	{
		this->pkg_in_->pkg_->uninitialize();
		delete this->pkg_in_;
	}

	const char* HdtasPackage::GetErrorString() const
	{
		return this->pkg_in_->pkg_->get_error().c_str();
	}

	h_uint HdtasPackage::GetPackageCount() const
	{
		return this->pkg_in_->pkg_->get_pkg_cnt();
	}

	h_uint HdtasPackage::GetPackageID() const
	{
		return this->pkg_in_->pkg_->get_pkg_id();
	}

	h_uint HdtasPackage::GetMsgID() const
	{
		return this->pkg_in_->pkg_->get_msg_id();
	}

	device_mr_id HdtasPackage::GetMMrID() const
	{
		return this->pkg_in_->pkg_->get_mmr_id();
	}

	std::pair<const unsigned char*, size_t> HdtasPackage::Pack()
	{
		return std::make_pair(this->pkg_in_->pkg_->get_data_buf(),
							  this->pkg_in_->pkg_->get_data_buf_len());
	}

	int HdtasPackage::Unpack(const unsigned char* buffer, size_t len, size_t& off)
	{
		return this->pkg_in_->pkg_->deserialize(buffer, len, off);
	}

	//////////////////////////////////////////////////////////////////////////
	class HdtasMessage::message_in
	{
	public:
		message_in() { this->msg_ = new po_message2(); }

		virtual ~message_in() { delete this->msg_; }

		po_message2* msg_;

		h_uint pkg_count_ = 1;
		h_uint pkg_id_ = 0;
	};

	HdtasMessage::HdtasMessage()
	{
		this->msg_in_ = new HdtasMessage::message_in();
		this->msg_in_->msg_->initialize();
	}

	HdtasMessage::~HdtasMessage()
	{
		this->msg_in_->msg_->uninitialize();
		delete this->msg_in_;
	}

	const char* HdtasMessage::GetErrorString() const
	{
		return this->msg_in_->msg_->get_error().c_str();
	}

	device_mr_id HdtasMessage::GetMMrID() const
	{
		return this->msg_in_->msg_->get_mmr_id();
	}

	h_uint32 HdtasMessage::GetMsgID() const
	{
		return this->msg_in_->msg_->get_msg_id();
	}

	HDTAS_MSG_TYPE HdtasMessage::GetType() const
	{
		HDTAS_MSG_TYPE type;
		pkg_t t = this->msg_in_->msg_->get_msg_type();
		switch ( t )
		{
		case PO_MSG_REG_REQ: type = HDTAS_MSG_TYPE::HCT_REG_REQ; break;
		case PO_MSG_REG_RPD: type = HDTAS_MSG_TYPE::HCT_REG_RPN; break;
		case PO_MSG_REG_CFM: type = HDTAS_MSG_TYPE::HCT_REG_CFM; break;
		case PO_MSG_HB_REQ: type = HDTAS_MSG_TYPE::HCT_HB_REQ; break;
		case PO_MSG_HB_RPD: type = HDTAS_MSG_TYPE::HCT_HB_RPN; break;
		case PO_MSG_CFG_RD_REQ: type = HDTAS_MSG_TYPE::HMT_RD_CFG_REQ; break;
		case PO_MSG_CFG_RD_RPD: type = HDTAS_MSG_TYPE::HMT_RD_CFG_RPN; break;
		case PO_MSG_CFG_WT_REQ: type = HDTAS_MSG_TYPE::HMT_WT_CFG_REQ; break;
		case PO_MSG_CFG_WT_RPD: type = HDTAS_MSG_TYPE::HMT_WT_CFG_RPN; break;
		case PO_MSG_CONTROL_REQ: type = HDTAS_MSG_TYPE::HMT_CTL_REQ; break;
        case PO_MSG_CONTROL_RPD: type = HDTAS_MSG_TYPE::HMT_CTL_RPN; break;
        case PO_MSG_TAG_DATA_RESP: type = HDTAS_MSG_TYPE::HMT_ISPD; break;
        case PO_MSG_MR_STATUS_RESP: type = HDTAS_MSG_TYPE::HMT_MR_STU; break;
        case PO_MSG_POWER_BOARD_RESP : type = HDTAS_MSG_TYPE::HMT_PB_STU; break;
		default: type = HDTAS_MSG_TYPE::HMT_UNKNOWN; break;
		}
		return type;
	}
/*
	void HdtasMessage::SetType(HDTAS_MSG_TYPE t)
	{
		pkg_t type = 0;
		switch (t)
		{
		case HDTAS_MSG_TYPE::HCT_REG_REQ: type = PO_MSG_REG_REQ; break;
		case HDTAS_MSG_TYPE::HCT_REG_RPN: type = PO_MSG_REG_RPD; break;
		case HDTAS_MSG_TYPE::HCT_REG_CFM: type = PO_MSG_REG_CFM; break;
		case HDTAS_MSG_TYPE::HCT_HB_REQ: type = PO_MSG_HB_REQ; break;
		case HDTAS_MSG_TYPE::HCT_HB_RPN: type = PO_MSG_HB_RPD; break;
		case HDTAS_MSG_TYPE::HMT_RD_CFG_REQ: type = PO_MSG_CFG_RD_REQ; break;
		case HDTAS_MSG_TYPE::HMT_RD_CFG_RPN: type = PO_MSG_CFG_RD_RPD; break;
		case HDTAS_MSG_TYPE::HMT_WT_CFG_REQ: type = PO_MSG_CFG_WT_REQ; break;
		case HDTAS_MSG_TYPE::HMT_WT_CFG_RPN: type = PO_MSG_CFG_WT_RPD; break;
		case HDTAS_MSG_TYPE::HMT_CTL_REQ: type = PO_MSG_CONTROL_REQ; break;
		case HDTAS_MSG_TYPE::HMT_CTL_RPN: type = PO_MSG_CONTROL_RPD; break;
		default: return;
		}

		this->msg_in_->msg_->set_msg_type(type);
	}
	*/

	
	int HdtasMessage::Serialize(HdtasPackage* pkg)
	{
		bool bfirst = (0 == this->msg_in_->pkg_id_);
		int rtn = this->msg_in_->msg_->serialize( bfirst );
		if (0 != rtn)
			return rtn;
		pkg->pkg_in_->pkg_->set_pkg_cnt(this->msg_in_->pkg_count_);
		pkg->pkg_in_->pkg_->set_pkg_id(this->msg_in_->pkg_id_);
		pkg->pkg_in_->pkg_->set_msg_id(this->msg_in_->msg_->get_msg_id());
		pkg->pkg_in_->pkg_->set_mmr_id(this->msg_in_->msg_->get_mmr_id());
		size_t off = 0;
		return pkg->pkg_in_->pkg_->serialize(this->msg_in_->msg_->get_msg_buf(),
			this->msg_in_->msg_->get_msg_buf_len(), off);
	}

	int HdtasMessage::Deserialize(const HdtasPackage* p)
	{
		return this->msg_in_->msg_->deserialize(p->pkg_in_->pkg_);
	}

	//////////////////////////////////////////
	class HdtasDataEntity::entity_in
	{
	public:
		entity_in() {};

		virtual ~entity_in() {}

		po_protocol_middle* mid_ = nullptr;
	};

	HdtasDataEntity::HdtasDataEntity()
	{
		this->ent_int_ = new HdtasDataEntity::entity_in();
	}

	HdtasDataEntity::~HdtasDataEntity()
	{
		delete this->ent_int_;
	}

	const char* HdtasDataEntity::GetErrorString() const
	{
		return this->ent_int_->mid_->get_error().c_str();
	}

	void HdtasDataEntity::SetMMrID(device_mr_id id)
	{
		this->ent_int_->mid_->set_mmr_id(id);
	}

	void HdtasDataEntity::SetMsgID(h_uint32 id)
	{
		this->ent_int_->mid_->set_msg_id(id);
	}

	int HdtasDataEntity::StartSerialize()
	{
		return 0;
	}

	int HdtasDataEntity::GetSerializeCount()
	{
		return 1;
	}

	int HdtasDataEntity::Serialize(HdtasMessage* msg)
	{
		msg->msg_in_->msg_->set_msg_type(this->ent_int_->mid_->get_type());
		msg->msg_in_->msg_->set_msg_ack(0);
		msg->msg_in_->msg_->set_msg_ver(1);
		msg->msg_in_->msg_->set_msg_res(0);
		return this->ent_int_->mid_->serialize(msg->msg_in_->msg_);
	}

	int HdtasDataEntity::Deserialize(const HdtasMessage* msg)
	{
		return this->ent_int_->mid_->deserialize(msg->msg_in_->msg_);
	}

	HDTAS_MSG_TYPE HdtasDataEntity::GetType() const
	{
		HDTAS_MSG_TYPE type;
		pkg_t t = this->ent_int_->mid_->get_type();
		switch (t)
		{
		case PO_MSG_REG_REQ: type = HDTAS_MSG_TYPE::HCT_REG_REQ; break;
		case PO_MSG_REG_RPD: type = HDTAS_MSG_TYPE::HCT_REG_RPN; break;
		case PO_MSG_REG_CFM: type = HDTAS_MSG_TYPE::HCT_REG_CFM; break;
		case PO_MSG_HB_REQ: type = HDTAS_MSG_TYPE::HCT_HB_REQ; break;
		case PO_MSG_HB_RPD: type = HDTAS_MSG_TYPE::HCT_HB_RPN; break;
		case PO_MSG_CFG_RD_REQ: type = HDTAS_MSG_TYPE::HMT_RD_CFG_REQ; break;
		case PO_MSG_CFG_RD_RPD: type = HDTAS_MSG_TYPE::HMT_RD_CFG_RPN; break;
		case PO_MSG_CFG_WT_REQ: type = HDTAS_MSG_TYPE::HMT_WT_CFG_REQ; break;
		case PO_MSG_CFG_WT_RPD: type = HDTAS_MSG_TYPE::HMT_WT_CFG_RPN; break;
		case PO_MSG_CONTROL_REQ: type = HDTAS_MSG_TYPE::HMT_CTL_REQ; break;
        case PO_MSG_CONTROL_RPD: type = HDTAS_MSG_TYPE::HMT_CTL_RPN; break;
        case PO_MSG_TAG_DATA_RESP: type = HDTAS_MSG_TYPE::HMT_ISPD; break;
        case PO_MSG_MR_STATUS_RESP: type = HDTAS_MSG_TYPE::HMT_MR_STU; break;
        case PO_MSG_POWER_BOARD_RESP : type = HDTAS_MSG_TYPE::HMT_PB_STU; break;
		default: type = HDTAS_MSG_TYPE::HMT_UNKNOWN; break;
		}
		return type;
	}

	device_mr_id HdtasDataEntity::GetMMrID() const
	{
		return this->ent_int_->mid_->get_mmr_id();
	}

	h_uint32 HdtasDataEntity::GetMsgID() const
	{
		return this->ent_int_->mid_->get_msg_id();
	}

	///////////////////////////////////////
	class HdtasRegRequest::request_in
	{
	public:
		request_in() { this->req_ = new po_tcp_reg_req2(); }

		virtual ~request_in() { delete this->req_; }

		po_tcp_reg_req2* req_;
	};

	HdtasRegRequest::HdtasRegRequest()
	{
		this->req_in_ = new HdtasRegRequest::request_in();
		this->ent_int_->mid_ = this->req_in_->req_;
	}

	HdtasRegRequest::~HdtasRegRequest()
	{
		delete this->req_in_;
	}

	h_uint HdtasRegRequest::GetStatus() const
	{
		return this->req_in_->req_->get_status();
	}

	///////////////////////////////////////
	class HdtasRegResponse::response_in
	{
	public:
		response_in() { this->rpd_ = new po_tcp_reg_rpd2(); }
		virtual ~response_in() { delete this->rpd_; }

		po_tcp_reg_rpd2* rpd_;
	};

	HdtasRegResponse::HdtasRegResponse()
	{
		this->rpn_in_ = new HdtasRegResponse::response_in();
		this->ent_int_->mid_ = this->rpn_in_->rpd_;
	}

	HdtasRegResponse::~HdtasRegResponse()
	{
		delete this->rpn_in_;
	}

	void HdtasRegResponse::SetDateTime(const ispd_date_time& dt)
	{
		this->rpn_in_->rpd_->set_date_time(dt);
	}

	void HdtasRegResponse::SetEcode(h_int e)
	{
		this->rpn_in_->rpd_->set_reg_error(e);
	}

	///////////////////////////////////////
	class HdtasRegConfirm::confirm_in
	{
	public:
		confirm_in() { this->cfm_ = new po_tcp_reg_cfm2(); }
		virtual ~confirm_in() { delete this->cfm_; }

		po_tcp_reg_cfm2* cfm_;
	};

	HdtasRegConfirm::HdtasRegConfirm()
	{
		this->cfm_in_ = new HdtasRegConfirm::confirm_in();
		this->ent_int_->mid_ = this->cfm_in_->cfm_;
	}

	HdtasRegConfirm::~HdtasRegConfirm()
	{
		delete this->cfm_in_;
	}

	h_int HdtasRegConfirm::GetEcode() const
	{
		return (h_int)this->cfm_in_->cfm_->get_error_code();
	}

	///////////////////////////////////////
	class HdtasHeartBeatRequest::request_in
	{
	public:
		request_in() { this->req_ = new po_tcp_hb_req2(); }
		virtual ~request_in() { delete this->req_; }

		po_tcp_hb_req2* req_;
	};

	HdtasHeartBeatRequest::HdtasHeartBeatRequest()
	{
		this->req_in_ = new HdtasHeartBeatRequest::request_in();
		this->ent_int_->mid_ = this->req_in_->req_;
	}

	HdtasHeartBeatRequest::~HdtasHeartBeatRequest()
	{
		delete this->req_in_;
	}

	void HdtasHeartBeatRequest::SetStatus(h_uint s)
	{
		this->req_in_->req_->set_status(s);
	}

	///////////////////////////////////////
	class HdtasHeartBeatResponse::response_in
	{
	public:
		response_in() { this->rpd_ = new po_tcp_hb_rpd2(); }
		virtual ~response_in() { delete this->rpd_; }

		po_tcp_hb_rpd2* rpd_;
	};

	HdtasHeartBeatResponse::HdtasHeartBeatResponse()
	{
		this->rpn_in_ = new HdtasHeartBeatResponse::response_in();
		this->ent_int_->mid_ = this->rpn_in_->rpd_;
	}

	HdtasHeartBeatResponse::~HdtasHeartBeatResponse()
	{
		delete this->rpn_in_;
	}

	h_int HdtasHeartBeatResponse::GetEcode() const
	{
		return (h_int)this->rpn_in_->rpd_->get_error_code();
	}

	///////////////////////////////////////
	class HdtasCfgRequest::request_in
	{
	public:
		request_in(HDTAS_RD_WT_TYPE t)
		{
			this->type_ = (HDTAS_RD_WT_TYPE::HRWT_READ == t ?
				HDTAS_MSG_TYPE::HMT_RD_CFG_REQ :
				HDTAS_MSG_TYPE::HMT_WT_CFG_REQ);
			if (HDTAS_RD_WT_TYPE::HRWT_READ == t)
				this->rd_req_ = new po_tcp_cfg_read_req();
			else
				this->wt_req_ = new po_tcp_cfg_write_req();
		}

		virtual ~request_in() 
		{
			delete this->rd_req_;
			delete this->wt_req_;
		}

		po_tcp_cfg_read_req* rd_req_ = nullptr;
		po_tcp_cfg_write_req* wt_req_ = nullptr;

		HDTAS_MSG_TYPE type_;
	};

	HdtasCfgRequest::HdtasCfgRequest(HDTAS_RD_WT_TYPE t)
	{
		this->req_in_ = new HdtasCfgRequest::request_in(t);
		if (HDTAS_RD_WT_TYPE::HRWT_READ == t)
			this->ent_int_->mid_ = this->req_in_->rd_req_;
		else
			this->ent_int_->mid_ = this->req_in_->wt_req_;
	}

	HdtasCfgRequest::~HdtasCfgRequest()
	{
		delete this->req_in_;
	}

	void HdtasCfgRequest::SetRequestID(h_uint id)
	{
		this->req_in_->rd_req_->set_status( 0 );
		this->req_in_->rd_req_->set_pair_id( id );
	}

	void HdtasCfgRequest::ReadConfiguration()
	{
		this->req_in_->type_ = HDTAS_MSG_TYPE::HMT_RD_CFG_REQ;
	}

	int HdtasCfgRequest::WriteConfiguration(const mr_config_info& info)
	{
		this->req_in_->type_ = HDTAS_MSG_TYPE::HMT_WT_CFG_REQ;

		h_uint32 temp = 0;
		int rtn = utility::pre_to_net(info.ip, temp);
		if (0 == rtn) return rtn;
		//this->req_in_->wt_req_->set_ip(temp);

		rtn = utility::pre_to_net(info.gw, temp);
		if (0 == rtn) return rtn;
		//this->req_in_->wt_req_->set_gateway(temp);

		rtn = utility::pre_to_net(info.nm, temp);
		if (0 == rtn) return rtn;
		//this->req_in_->wt_req_->set_netmask(temp);

		rtn = utility::pre_to_net(info.dns, temp);
		if (0 == rtn) return rtn;
		//this->req_in_->wt_req_->set_dns_ip(temp);

		rtn = utility::pre_to_net(info.tip, temp);
		if (0 == rtn) return rtn;
		//this->req_in_->wt_req_->set_tcp_ip(temp);

		rtn = utility::pre_to_net(info.uip, temp);
		if (0 == rtn) return rtn;
		//this->req_in_->wt_req_->set_udp_ip(temp);

		//this->req_in_->wt_req_->set_tcp_port(info.tport);
		//this->req_in_->wt_req_->set_udp_port(info.uport);

		return 0;
	}


	///////////////////////////////////////
	class HdtasCfgResponse::response_in
	{
	public:
		response_in(HDTAS_RD_WT_TYPE t)
		{
			this->type_ = (HDTAS_RD_WT_TYPE::HRWT_READ == t ?
				HDTAS_MSG_TYPE::HMT_RD_CFG_RPN :
				HDTAS_MSG_TYPE::HMT_WT_CFG_RPN );
			if(HDTAS_RD_WT_TYPE::HRWT_READ == t)
				this->rd_rpd_ = new po_tcp_cfg_read_rpd();
			else
				this->wt_rpd_ = new po_tcp_cfg_write_rpd();
		}

		virtual ~response_in() 
		{
			delete this->rd_rpd_;
			delete this->wt_rpd_;
		}

		po_tcp_cfg_read_rpd* rd_rpd_ = nullptr;
		po_tcp_cfg_write_rpd* wt_rpd_ = nullptr;

		HDTAS_MSG_TYPE type_;
		mr_config_info info_;
	};
	
	HdtasCfgResponse::HdtasCfgResponse(HDTAS_RD_WT_TYPE t)
	{
		this->rpn_in_ = new HdtasCfgResponse::response_in(t);
		
		if (HDTAS_RD_WT_TYPE::HRWT_READ == t)
			this->ent_int_->mid_ = this->rpn_in_->rd_rpd_;
		else
			this->ent_int_->mid_ = this->rpn_in_->wt_rpd_;
	}

	HdtasCfgResponse::~HdtasCfgResponse()
	{
		delete this->rpn_in_;
	}

	h_int HdtasCfgResponse::GetEcode() const
	{
		if (HDTAS_MSG_TYPE::HMT_RD_CFG_RPN == this->rpn_in_->type_)
			return this->rpn_in_->rd_rpd_->get_ecode();
		else if (HDTAS_MSG_TYPE::HMT_WT_CFG_RPN == this->rpn_in_->type_)
			return this->rpn_in_->wt_rpd_->get_ecode();
		else
			return -1;
	}

	h_uint HdtasCfgResponse::GetResponseID() const
	{
		if (HDTAS_MSG_TYPE::HMT_RD_CFG_RPN == this->rpn_in_->type_)
			return this->rpn_in_->rd_rpd_->get_pair_id();
		else if (HDTAS_MSG_TYPE::HMT_WT_CFG_RPN == this->rpn_in_->type_)
			return this->rpn_in_->wt_rpd_->get_pair_id();
		else
			return 0;
	}

	HDTAS_MSG_TYPE HdtasCfgResponse::GetResponseType() const
	{
		return this->rpn_in_->type_;
	}

	const mr_config_info& HdtasCfgResponse::GetConfiguration()
	{
		if (HDTAS_MSG_TYPE::HMT_RD_CFG_RPN != this->rpn_in_->type_)
			return this->rpn_in_->info_;

		//std::string temp = utility::net_to_pre(this->rpn_in_->rd_rpd_->get_ip());
		//STRNCPY(this->rpn_in_->info_.ip, LEN_MAX_IP_ADDRESS, temp.c_str(), temp.size());

		//temp = utility::net_to_pre(this->rpn_in_->rd_rpd_->get_netmask());
		//STRNCPY(this->rpn_in_->info_.nm, LEN_MAX_IP_ADDRESS, temp.c_str(), temp.size());

		//temp = utility::net_to_pre(this->rpn_in_->rd_rpd_->get_gateway());
		//STRNCPY(this->rpn_in_->info_.gw, LEN_MAX_IP_ADDRESS, temp.c_str(), temp.size());

		//temp = utility::net_to_pre(this->rpn_in_->rd_rpd_->get_dns_ip());
		//STRNCPY(this->rpn_in_->info_.dns, LEN_MAX_IP_ADDRESS, temp.c_str(), temp.size());

		//temp = this->rpn_in_->rd_rpd_->get_mac();
		//STRNCPY(this->rpn_in_->info_.mac, LEN_MAX_MAC_ADDRESS, temp.c_str(), temp.size());

		//temp = utility::net_to_pre(this->rpn_in_->rd_rpd_->get_tcp_ip());
		//STRNCPY(this->rpn_in_->info_.tip, LEN_MAX_IP_ADDRESS, temp.c_str(), temp.size());

		//temp = utility::net_to_pre(this->rpn_in_->rd_rpd_->get_udp_ip());
		//STRNCPY(this->rpn_in_->info_.uip, LEN_MAX_IP_ADDRESS, temp.c_str(), temp.size());

		//this->rpn_in_->info_.tport = this->rpn_in_->rd_rpd_->get_tcp_port();
		//this->rpn_in_->info_.uport = this->rpn_in_->rd_rpd_->get_udp_port();

		return this->rpn_in_->info_;
	}

    //////////////////////////////////////////
    class HdtasCtlMrPbRequest::request_in
    {
    public:
        request_in() { this->req_ = new po_tcp_ctl_req(); }
        virtual ~request_in() { delete this->req_; }

        po_tcp_ctl_req* req_;
    };

    HdtasCtlMrPbRequest::HdtasCtlMrPbRequest()
    {
        this->req_in_ = new HdtasCtlMrPbRequest::request_in();
        this->ent_int_->mid_ = this->req_in_->req_;
    }

    HdtasCtlMrPbRequest::~HdtasCtlMrPbRequest()
    {
        delete this->req_in_;
    }

    void HdtasCtlMrPbRequest::SetRequestID(h_uint id)
    {
        this->req_in_->req_->set_status( 0 );
        this->req_in_->req_->set_pair_id( id );
    }

    void HdtasCtlMrPbRequest::ReadAutoReportStatus()
    {
        abort();
    }

    void HdtasCtlMrPbRequest::ReadAutoReportInterval()
    {
        this->req_in_->req_->set_rd_mr_status_report_interval();
    }

    void HdtasCtlMrPbRequest::ReadStatusInfo()
    {
        this->req_in_->req_->set_mr_status_report_once();
    }

    void HdtasCtlMrPbRequest::WriteAutoReportStatus(HDTAS_OC_STATUS s)
    {
        abort();
    }

    void HdtasCtlMrPbRequest::WriteAutoReportInterval(h_uint t)
    {
        this->req_in_->req_->set_wt_mr_status_report_interval(h_uint8(t));
    }

    void HdtasCtlMrPbRequest::Reboot_MR(device_mr_id id)
    {
        this->req_in_->req_->set_mr_reboot(id);
    }

    void HdtasCtlMrPbRequest::Reset_MR(device_mr_id id)
    {
        this->req_in_->req_->set_mr_reset(id);

    }



    void HdtasCtlMrPbRequest::ReadAutoReportStatus(device_mr_id id)
    {
        this->req_in_->req_->set_rd_power_board_auto_report(id);
    }

    void HdtasCtlMrPbRequest::ReadAutoReportInterval(device_mr_id id)
    {
        this->req_in_->req_->set_rd_power_board_report_interval(id);
    }

    void HdtasCtlMrPbRequest::ReadStatusInfo(device_mr_id id)
    {
        this->req_in_->req_->set_rd_power_board_shortcut_cmd(id);
    }

    void HdtasCtlMrPbRequest::WriteAutoReportStatus(device_mr_id id, HDTAS_OC_STATUS s)
    {
        bool oc = (s == HDTAS_OC_STATUS::HOCS_OPEN ? true : false);
        this->req_in_->req_->set_wt_power_board_auto_report(id, oc);
    }

    void HdtasCtlMrPbRequest::WriteAutoReportInterval(device_mr_id id, h_uint t)
    {
        unsigned char i = (unsigned char)t;
        this->req_in_->req_->set_wt_power_board_report_interval(id, i);
    }

    void HdtasCtlMrPbRequest::Open_Close(device_mr_id id, HDTAS_OC_STATUS c, HDTAS_OC_STATUS m, HDTAS_OC_STATUS p, HDTAS_OC_STATUS b)
    {
        bool cam = (HDTAS_OC_STATUS::HOCS_OPEN == c ? true : false);
        bool mic = (HDTAS_OC_STATUS::HOCS_OPEN == m ? true : false);
        bool pre = (HDTAS_OC_STATUS::HOCS_OPEN == p ? true : false);
        bool bat = (HDTAS_OC_STATUS::HOCS_OPEN == b ? true : false);
        this->req_in_->req_->set_wt_power_boart_open_close_mic_cam(id, mic, cam, pre, bat);
    }

    void HdtasCtlMrPbRequest::Reboot_PB(device_mr_id id)
    {
        this->req_in_->req_->set_wt_power_board_reboot(id);
    }

    void HdtasCtlMrPbRequest::Reset_PB(device_mr_id id)
    {
        this->req_in_->req_->set_wt_power_board_reset(id);
    }

    class HdtasCtlMrPbResponse::response_in
    {
    public:
        response_in() { this->rpd_ = new po_tcp_ctl_rpd(); }
        virtual ~response_in() { delete this->rpd_; }

        po_tcp_ctl_rpd* rpd_;
    };

    HdtasCtlMrPbResponse::HdtasCtlMrPbResponse()
    {
        this->rpn_in_ = new HdtasCtlMrPbResponse::response_in();
        this->ent_int_->mid_ = this->rpn_in_->rpd_;
    }

    HdtasCtlMrPbResponse::~HdtasCtlMrPbResponse()
    {
        delete this->rpn_in_;
    }

    h_int HdtasCtlMrPbResponse::GetEcode() const
    {
        h_uint rtn = this->rpn_in_->rpd_->get_ecode();
        if (0 != rtn)
            return (h_int)rtn;
        rtn = this->rpn_in_->rpd_->get_pb_ecode();
        return (h_int)rtn;
    }

    h_uint HdtasCtlMrPbResponse::GetResponseID() const
    {
        return this->rpn_in_->rpd_->get_pair_id();
    }

    HDTAS_CTL_TYPE HdtasCtlMrPbResponse::GetResponseType() const
    {
        HDTAS_CTL_TYPE type;
        h_uint32 cmd = this->rpn_in_->rpd_->get_cmd();
        switch (cmd)
        {
        case MMR_CFG_CMD_REBOOT:
            type = HDTAS_CTL_TYPE::HCT_MR_WT_REBOOT;
            break;
        case MMR_CFG_CMD_MR_STATUS_ONCE:
            type = HDTAS_CTL_TYPE::HCT_MR_RD_STU_INFO;
            break;
        case MMR_CFG_CMD_MR_WT_STATUS_INTERVAL:
            type = HDTAS_CTL_TYPE::HCT_MR_WT_AR_ITR;
            break;
        case MMR_CFG_CMD_MR_RD_STATUS_INTERVAL:
            type = HDTAS_CTL_TYPE::HCT_MR_RD_AR_ITR;
            break;
        case MMR_CFG_CMD_MR_POWER_BOARD:
        {
            pkg_t addr = this->rpn_in_->rpd_->get_pb_addr();
            switch (addr)
            {
            case POWER_BOARD_ADDR_RDWT_AUTO_REPORT:
            {
                if (this->rpn_in_->rpd_->get_pb_read_write_mode())
                    type = HDTAS_CTL_TYPE::HCT_PB_RD_AR_STU;
                else
                    type = HDTAS_CTL_TYPE::HCT_PB_WT_AR_STU;
                break;
            }
            case POWER_BOARD_ADDR_RDWT_REPORT_INTERVAL:
            {
                if (this->rpn_in_->rpd_->get_pb_read_write_mode())
                    type = HDTAS_CTL_TYPE::HCT_PB_RD_AR_ITR;
                else
                    type = HDTAS_CTL_TYPE::HCT_PB_WT_AR_ITR;
                break;
            }
            case POWER_BOARD_ADDR_WT_OPEN_CLOSE:
                type = HDTAS_CTL_TYPE::HCT_PB_WT_OC_CMP;
                break;
            case POWER_BOARD_ADDR_WT_REBOOT:
                type = HDTAS_CTL_TYPE::HCT_PB_WT_REBOOT;
                break;
            case POWER_BOARD_ADDR_WT_RESET:
                type = HDTAS_CTL_TYPE::HCT_PB_WT_RESET;
                break;
            case POWER_BOARD_ADDR_RD_SHORTCUT_CMD:
                type = HDTAS_CTL_TYPE::HCT_PB_RD_STU_INFO;
                break;
            default:
                type = HDTAS_CTL_TYPE::HCT_UNKNOWN;
            }
            break;
        }
        default:
            type = HDTAS_CTL_TYPE::HCT_UNKNOWN;
        }
        return type;
    }

    HDTAS_OC_STATUS HdtasCtlMrPbResponse::GetAutoReportStatus() const
    {
        return HDTAS_OC_STATUS::HOCS_UNKNOWN;
    }

    h_uint HdtasCtlMrPbResponse::GetAutoReportInterval() const
    {
        return this->rpn_in_->rpd_->get_mr_report_interval();
    }

    const mr_status_array& HdtasCtlMrPbResponse::GetStatusInfo() const
    {
        return this->rpn_in_->rpd_->get_mr_status_array();
    }





    device_mr_id HdtasCtlMrPbResponse::GetMrID() const
    {
        return this->rpn_in_->rpd_->get_mr_id();
    }

    HDTAS_OC_STATUS HdtasCtlMrPbResponse::GetAutoReportStatus2() const
    {
        h_uint8 res = this->rpn_in_->rpd_->get_power_board_status_group().auto_report_;
        if (0 == res) return HDTAS_OC_STATUS::HOCS_CLOSE;
        else if (1 == res) return HDTAS_OC_STATUS::HOCS_OPEN;
        else return HDTAS_OC_STATUS::HOCS_UNKNOWN;
    }

    h_uint HdtasCtlMrPbResponse::GetAutoReportInterval2() const
    {
        return this->rpn_in_->rpd_->get_power_board_status_group().auto_report_interval_;
    }

    const power_board_status& HdtasCtlMrPbResponse::GetStatusInfo2() const
    {
        return this->rpn_in_->rpd_->get_power_board_cur_status();
    }

    const power_board_status_group& HdtasCtlMrPbResponse::GetStatusGroupInfo2() const
    {
        return this->rpn_in_->rpd_->get_power_board_status_group();
    }


    //////////////////////////////////////////
    class HdtasCtlMrRequest::request_in
    {
    public:
        request_in() { this->req_ = new po_tcp_ctl_req(); }
        virtual ~request_in() { delete this->req_; }

        po_tcp_ctl_req* req_;
    };

    HdtasCtlMrRequest::HdtasCtlMrRequest()
    {
        this->req_in_ = new HdtasCtlMrRequest::request_in();
        this->ent_int_->mid_ = this->req_in_->req_;
    }

    HdtasCtlMrRequest::~HdtasCtlMrRequest()
    {
        delete this->req_in_;
    }

    void HdtasCtlMrRequest::SetRequestID(h_uint id)
    {
        this->req_in_->req_->set_status( 0 );
        this->req_in_->req_->set_pair_id( id );
    }

    void HdtasCtlMrRequest::ReadAutoReportStatus()
    {
        abort();
    }

    void HdtasCtlMrRequest::ReadAutoReportInterval()
    {
        this->req_in_->req_->set_rd_mr_status_report_interval();
    }

    void HdtasCtlMrRequest::ReadStatusInfo()
    {
        this->req_in_->req_->set_mr_status_report_once();
    }

	void HdtasCtlMrRequest::WriteAutoReportStatus(HDTAS_OC_STATUS s)
	{
		abort();
	}

	void HdtasCtlMrRequest::WriteAutoReportInterval(h_uint t)
	{
		this->req_in_->req_->set_wt_mr_status_report_interval(h_uint8(t));
	}

	void HdtasCtlMrRequest::Reboot(device_mr_id id)
	{
		this->req_in_->req_->set_mr_reboot(id);
	}

	void HdtasCtlMrRequest::Reset(device_mr_id id)
	{
		abort();
	}

	class HdtasCtlMrResponse::response_in
	{
	public:
		response_in() { this->rpd_ = new po_tcp_ctl_rpd(); }
		virtual ~response_in() { delete this->rpd_; }

		po_tcp_ctl_rpd* rpd_;
	};

	HdtasCtlMrResponse::HdtasCtlMrResponse()
	{
		this->rpn_in_ = new HdtasCtlMrResponse::response_in();
		this->ent_int_->mid_ = this->rpn_in_->rpd_;
	}

	HdtasCtlMrResponse::~HdtasCtlMrResponse()
	{
		delete this->rpn_in_;
	}

	h_int HdtasCtlMrResponse::GetEcode() const
	{
		return this->rpn_in_->rpd_->get_ecode();
	}

	h_uint HdtasCtlMrResponse::GetResponseID() const
	{
		return this->rpn_in_->rpd_->get_pair_id();
	}

	HDTAS_CTL_TYPE HdtasCtlMrResponse::GetResponseType() const
	{
		HDTAS_CTL_TYPE type;
		h_uint32 cmd = this->rpn_in_->rpd_->get_cmd();
		switch (cmd)
		{
		case MMR_CFG_CMD_REBOOT:
			type = HDTAS_CTL_TYPE::HCT_MR_WT_REBOOT;
			break;
		case MMR_CFG_CMD_MR_STATUS_ONCE:
			type = HDTAS_CTL_TYPE::HCT_MR_RD_STU_INFO;
			break;
		case MMR_CFG_CMD_MR_WT_STATUS_INTERVAL:
			type = HDTAS_CTL_TYPE::HCT_MR_WT_AR_ITR;
			break;
		case MMR_CFG_CMD_MR_RD_STATUS_INTERVAL:
			type = HDTAS_CTL_TYPE::HCT_MR_RD_AR_ITR;
			break;
		default:
			type = HDTAS_CTL_TYPE::HCT_UNKNOWN;
		}
		return type;
	}

	HDTAS_OC_STATUS HdtasCtlMrResponse::GetAutoReportStatus() const
	{
		return HDTAS_OC_STATUS::HOCS_UNKNOWN;
	}

	h_uint HdtasCtlMrResponse::GetAutoReportInterval() const
	{
		return this->rpn_in_->rpd_->get_mr_report_interval();
	}

	const mr_status_array& HdtasCtlMrResponse::GetStatusInfo() const
	{
		return this->rpn_in_->rpd_->get_mr_status_array();
	}

	class HdtasCtlPbRequest::request_in
	{
	public:
		request_in() { this->req_ = new po_tcp_ctl_req();  }
		virtual ~request_in() { delete this->req_; }

		po_tcp_ctl_req* req_;
	};

	HdtasCtlPbRequest::HdtasCtlPbRequest()
	{
		this->req_in_ = new HdtasCtlPbRequest::request_in();
		this->ent_int_->mid_ = this->req_in_->req_;
	}

	HdtasCtlPbRequest::~HdtasCtlPbRequest()
	{
		delete this->req_in_;
	}

	void HdtasCtlPbRequest::SetRequestID(h_uint id)
	{
		this->req_in_->req_->set_status(0);
		this->req_in_->req_->set_pair_id(id);
	}

	void HdtasCtlPbRequest::ReadAutoReportStatus(device_mr_id id)
	{
		this->req_in_->req_->set_rd_power_board_auto_report( id );
	}

	void HdtasCtlPbRequest::ReadAutoReportInterval(device_mr_id id)
	{
		this->req_in_->req_->set_rd_power_board_auto_report( id );
	}

	void HdtasCtlPbRequest::ReadStatusInfo(device_mr_id id)
	{
		this->req_in_->req_->set_rd_power_board_shortcut_cmd( id );
	}

	void HdtasCtlPbRequest::WriteAutoReportStatus(device_mr_id id, HDTAS_OC_STATUS s)
	{
		bool oc = (s == HDTAS_OC_STATUS::HOCS_OPEN ? true : false);
		this->req_in_->req_->set_wt_power_board_auto_report( id, oc );
	}

	void HdtasCtlPbRequest::WriteAutoReportInterval(device_mr_id id, h_uint t)
	{
		unsigned char i = (unsigned char)t;
		this->req_in_->req_->set_wt_power_board_report_interval( id, i );
	}

	void HdtasCtlPbRequest::Open_Close(device_mr_id id, HDTAS_OC_STATUS c, HDTAS_OC_STATUS m, HDTAS_OC_STATUS p, HDTAS_OC_STATUS b )
	{
		bool cam = (HDTAS_OC_STATUS::HOCS_OPEN == c ? true : false);
		bool mic = (HDTAS_OC_STATUS::HOCS_OPEN == m ? true : false);
		bool pre = (HDTAS_OC_STATUS::HOCS_OPEN == p ? true : false);
		bool bat = (HDTAS_OC_STATUS::HOCS_OPEN == b ? true : false);
		this->req_in_->req_->set_wt_power_boart_open_close_mic_cam( id, cam, mic, pre, bat );
	}

	void HdtasCtlPbRequest::Reboot(device_mr_id id)
	{
		this->req_in_->req_->set_wt_power_board_reboot( id );
	}

	void HdtasCtlPbRequest::Reset(device_mr_id id)
	{
		this->req_in_->req_->set_wt_power_board_reset(id);
	}
	
	class HdtasCtlPbResponse::response_in
	{
	public:
		response_in() { this->rpd_ = new po_tcp_ctl_rpd(); }
		virtual ~response_in() { delete this->rpd_; }

		po_tcp_ctl_rpd*  rpd_;
	};

	HdtasCtlPbResponse::HdtasCtlPbResponse()
	{
		this->rpn_in_ = new HdtasCtlPbResponse::response_in();
		this->ent_int_->mid_ = this->rpn_in_->rpd_;
	}
	
	HdtasCtlPbResponse::~HdtasCtlPbResponse()
	{
		delete this->rpn_in_;
	}


	h_int HdtasCtlPbResponse::GetEcode() const
	{
		h_uint rtn = this->rpn_in_->rpd_->get_ecode();
		if (0 != rtn)
			return (h_int)rtn;
		rtn = this->rpn_in_->rpd_->get_pb_ecode();
		return (h_int)rtn;
	}

	h_uint HdtasCtlPbResponse::GetResponseID() const
	{
		return this->rpn_in_->rpd_->get_pair_id();
	}
	
	HDTAS_CTL_TYPE HdtasCtlPbResponse::GetResponseType() const
	{
		h_uint32 cmd = this->rpn_in_->rpd_->get_cmd();
		if (MMR_CFG_CMD_MR_POWER_BOARD == cmd )
			return HDTAS_CTL_TYPE::HCT_UNKNOWN;

		HDTAS_CTL_TYPE type;

		pkg_t addr = this->rpn_in_->rpd_->get_pb_addr();
		switch (addr)
		{
		case POWER_BOARD_ADDR_RDWT_AUTO_REPORT:
		{
			if (this->rpn_in_->rpd_->get_pb_read_write_mode())
				type = HDTAS_CTL_TYPE::HCT_PB_RD_AR_STU;
			else
				type = HDTAS_CTL_TYPE::HCT_PB_WT_AR_STU;
			break;
		}
		case POWER_BOARD_ADDR_RDWT_REPORT_INTERVAL:
		{
			if (this->rpn_in_->rpd_->get_pb_read_write_mode())
				type = HDTAS_CTL_TYPE::HCT_PB_RD_AR_ITR;
			else
				type = HDTAS_CTL_TYPE::HCT_PB_WT_AR_ITR;
			break;
		}
		case POWER_BOARD_ADDR_WT_OPEN_CLOSE:
			type = HDTAS_CTL_TYPE::HCT_PB_WT_OC_CMP;
			break;
		case POWER_BOARD_ADDR_WT_REBOOT:
			type = HDTAS_CTL_TYPE::HCT_PB_WT_REBOOT;
			break;
		case POWER_BOARD_ADDR_WT_RESET:
			type = HDTAS_CTL_TYPE::HCT_PB_WT_RESET;
			break;
		case POWER_BOARD_ADDR_RD_SHORTCUT_CMD:
			type = HDTAS_CTL_TYPE::HCT_PB_RD_STU_INFO;
			break;
		default:
			type = HDTAS_CTL_TYPE::HCT_UNKNOWN;
		}
		return type;
	}

	device_mr_id HdtasCtlPbResponse::GetMrID() const
	{
		return this->rpn_in_->rpd_->get_mr_id();
	}

	HDTAS_OC_STATUS HdtasCtlPbResponse::GetAutoReportStatus() const
	{
		h_uint8 res = this->rpn_in_->rpd_->get_power_board_status_group().auto_report_;
		if (0 == res) return HDTAS_OC_STATUS::HOCS_CLOSE;
		else if (1 == res) return HDTAS_OC_STATUS::HOCS_OPEN;
		else return HDTAS_OC_STATUS::HOCS_UNKNOWN;
	}
	
	h_uint HdtasCtlPbResponse::GetAutoReportInterval() const
	{
		return this->rpn_in_->rpd_->get_power_board_status_group().auto_report_interval_;
	}

	const power_board_status& HdtasCtlPbResponse::GetStatusInfo() const
	{
		return this->rpn_in_->rpd_->get_power_board_cur_status();
	}

	const power_board_status_group& HdtasCtlPbResponse::GetStatusGroupInfo() const
	{
		return this->rpn_in_->rpd_->get_power_board_status_group();
	}

	class HdtasIspdData::ispd_data_in
	{
	public:
		ispd_data_in() { this->data_ = new po_udp_data_resp2();  }
		virtual ~ispd_data_in() { delete this->data_;  }

		po_udp_data_resp2* data_;
	};

	HdtasIspdData::HdtasIspdData()
	{
		this->data_in_ = new HdtasIspdData::ispd_data_in();
		this->ent_int_->mid_ = this->data_in_->data_;
	}

	HdtasIspdData::~HdtasIspdData()
	{
		delete this->data_in_;
	}

	const ispd_date_time& HdtasIspdData::GetTimestamp() const
	{
		return this->data_in_->data_->get_date_time();
	}

	const isdp_data_array& HdtasIspdData::GetDataInfo() const
	{
		return this->data_in_->data_->get_data_array();
	}

	class HdtasMrStatus::mr_status_in
	{
	public:
		mr_status_in() { this->status_ = new po_udp_mr_status(); }
		virtual ~mr_status_in() { delete this->status_;  }

		po_udp_mr_status* status_;
	};

	HdtasMrStatus::HdtasMrStatus()
	{
		this->status_in_ = new HdtasMrStatus::mr_status_in();
		this->ent_int_->mid_ = this->status_in_->status_;
	}

	HdtasMrStatus::~HdtasMrStatus()
	{
		delete this->status_in_;
	}

	const mr_status_array& HdtasMrStatus::GetStatusInfo() const
	{
		return this->status_in_->status_->get_status_array();
	}

	class HdtasPbStatus::pb_status_in
	{
	public:
		pb_status_in() { this->pb_ = new po_udp_power_board(); }
		virtual ~pb_status_in() { delete this->pb_; }

		po_udp_power_board* pb_;
	};

	HdtasPbStatus::HdtasPbStatus()
	{
		this->status_in_ = new HdtasPbStatus::pb_status_in();
		this->ent_int_->mid_ = this->status_in_->pb_;
	}

	HdtasPbStatus::~HdtasPbStatus()
	{
		delete this->status_in_;
	}

	device_mr_id HdtasPbStatus::GetMrID() const
	{
		return this->status_in_->pb_->get_pb_mr_id();
	}

	const power_board_status& HdtasPbStatus::GetStatusInfo() const
	{
		return this->status_in_->pb_->get_status();
	}

	const power_board_status_group& HdtasPbStatus::GetStatusGroupInfo() const
	{
		return this->status_in_->pb_->get_status_group();
	}
}
