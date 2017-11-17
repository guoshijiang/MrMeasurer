#ifndef HDTASPARSER_H
#define HDTASPARSER_H

#include "struct.h"
#define HDTAS_PARSER_EXPORT
#ifdef HDTAS_WINDOWS
    #ifdef HDTAS_PARSER_EXPORT
        #define HDTAS_PARSER_CLASS __declspec(dllexport)
    #else
        #define HDTAS_PARSER_CLASS __declspec(dllimport)
    #endif
#else
        #define HDTAS_PARSER_CLASS
#endif

namespace hdtas
{
	
	enum HDTAS_MSG_TYPE
	{
		HMT_UNKNOWN,
		HCT_REG_REQ,
		HCT_REG_RPN,
		HCT_REG_CFM,
		HCT_HB_REQ,
		HCT_HB_RPN,

		HMT_RD_CFG_REQ,
		HMT_RD_CFG_RPN,

		HMT_WT_CFG_REQ,
		HMT_WT_CFG_RPN,

		HMT_CTL_REQ,
        HMT_CTL_RPN,

        HMT_ISPD,
        HMT_MR_STU,
        HMT_PB_STU
		
	};

	enum HDTAS_CTL_TYPE
	{
		HCT_UNKNOWN,
		HCT_MR_RD_AR_STU, // Un-implement
		HCT_MR_RD_AR_ITR,
		HCT_MR_RD_STU_INFO,
		HCT_MR_WT_AR_STU, // Un-implement
		HCT_MR_WT_AR_ITR,
		HCT_MR_WT_REBOOT,
		HCT_MR_WT_RESET, // Un-implement

		HCT_PB_RD_AR_STU,
		HCT_PB_RD_AR_ITR,
		HCT_PB_RD_STU_INFO,
		HCT_PB_WT_AR_STU,
		HCT_PB_WT_AR_ITR,
		HCT_PB_WT_OC_CMP,
		HCT_PB_WT_REBOOT,
		HCT_PB_WT_RESET

		
	};

	enum HDTAS_RD_WT_TYPE
	{
		HRWT_READ,
		HRWT_WRITE
	};

	enum HDTAS_OC_STATUS
	{
		HOCS_UNKNOWN,
		HOCS_CLOSE,
		HOCS_OPEN
	};

	class HDTAS_PARSER_CLASS HdtasPackage
	{
	public:
		static int GetMMrIDFromPackage(const unsigned char* buffer, size_t len, device_mr_id& id);
	public:
		HdtasPackage();
		virtual ~HdtasPackage();

		const char* GetErrorString() const;

		h_uint GetPackageCount() const;
		h_uint GetPackageID() const;
		h_uint GetMsgID() const;
		device_mr_id GetMMrID() const;

		std::pair<const unsigned char*, size_t> Pack();
		int Unpack(const unsigned char* buffer, size_t len, size_t& off);
		
	private:
		class package_in;
		package_in* pkg_in_;

		friend class HdtasMessage;
	};

	class HDTAS_PARSER_CLASS HdtasMessage
	{
	public:
		HdtasMessage();
		virtual ~HdtasMessage();

		const char* GetErrorString() const;

		device_mr_id GetMMrID() const;
		h_uint32 GetMsgID() const;
		HDTAS_MSG_TYPE GetType() const;

		int Serialize(HdtasPackage* pkg);
		int Deserialize(const HdtasPackage* pkg);
	private:
		class message_in;
		message_in* msg_in_;

		friend class HdtasDataEntity;
	};

	class HDTAS_PARSER_CLASS HdtasDataEntity
	{
	public:
		HdtasDataEntity();
		virtual ~HdtasDataEntity();

		const char* GetErrorString() const;

		void SetMMrID(device_mr_id id);
		void SetMsgID(h_uint32 id);
		
		/** Functions for Serialize. */
		virtual int StartSerialize();
		virtual int GetSerializeCount();
		virtual int Serialize(HdtasMessage* msg);

		/** Functions for Deserialize. */
		//virtual int StartDeserialize();
		//virtual int AddMessage();
		virtual int Deserialize(const HdtasMessage* msg );

		HDTAS_MSG_TYPE GetType() const;
		device_mr_id GetMMrID() const;
		h_uint32 GetMsgID() const;
		
	protected:
		class entity_in;
		entity_in* ent_int_;
	};

	class HDTAS_PARSER_CLASS HdtasRegRequest : public HdtasDataEntity
	{
	public:
		HdtasRegRequest();
		virtual ~HdtasRegRequest();

		h_uint GetStatus() const;
	private:
		class request_in;
		request_in* req_in_;
	};

	class HDTAS_PARSER_CLASS HdtasRegResponse : public HdtasDataEntity
	{
	public:
		HdtasRegResponse();
		virtual ~HdtasRegResponse();

		void SetDateTime( const ispd_date_time& dt );
		void SetEcode( h_int e );
	private:
		class response_in;
		response_in* rpn_in_;
	};

	class HDTAS_PARSER_CLASS HdtasRegConfirm : public HdtasDataEntity
	{
	public:
		HdtasRegConfirm();
		virtual ~HdtasRegConfirm();

		h_int GetEcode() const;
	private:
		class confirm_in;
		confirm_in* cfm_in_;
	};

	class HDTAS_PARSER_CLASS HdtasHeartBeatRequest : public HdtasDataEntity
	{
	public:
		HdtasHeartBeatRequest();
		virtual ~HdtasHeartBeatRequest();

		void SetStatus(h_uint s);
	private:
		class request_in;
		request_in* req_in_;
	};

	class HDTAS_PARSER_CLASS HdtasHeartBeatResponse : public HdtasDataEntity
	{
	public:
		HdtasHeartBeatResponse();
		virtual ~HdtasHeartBeatResponse();

		h_int GetEcode() const;
	private:
		class response_in;
		response_in* rpn_in_;
	};

	class HDTAS_PARSER_CLASS HdtasCfgRequest : public HdtasDataEntity
	{
	public:
		HdtasCfgRequest(HDTAS_RD_WT_TYPE t);
		virtual ~HdtasCfgRequest();

		void SetRequestID( h_uint id );

		void ReadConfiguration();
		int WriteConfiguration(const mr_config_info& info);
	private:
		class request_in;
		request_in* req_in_;
	};

	class HDTAS_PARSER_CLASS HdtasCfgResponse : public HdtasDataEntity
	{
	public:
		HdtasCfgResponse(HDTAS_RD_WT_TYPE t);
		virtual ~HdtasCfgResponse();

		h_int GetEcode() const;

		h_uint GetResponseID() const;

		HDTAS_MSG_TYPE GetResponseType() const;

		const mr_config_info& GetConfiguration();
	private:
		class response_in;
		response_in* rpn_in_;
    };

    class HDTAS_PARSER_CLASS HdtasCtlMrPbRequest : public HdtasDataEntity
    {
    public:
        HdtasCtlMrPbRequest();
        virtual ~HdtasCtlMrPbRequest();

        void SetRequestID( h_uint id );

        // Functions for mr stadium.
        void ReadAutoReportStatus();
        void ReadAutoReportInterval();
        void ReadStatusInfo();
        void WriteAutoReportStatus( HDTAS_OC_STATUS s );
        void WriteAutoReportInterval( h_uint t );
        void Reboot_MR( device_mr_id id );
        void Reset_MR( device_mr_id id );

        // Functions for power board.
        void ReadAutoReportStatus(device_mr_id id);
        void ReadAutoReportInterval(device_mr_id id);
        void ReadStatusInfo(device_mr_id id);
        void WriteAutoReportStatus(device_mr_id id, HDTAS_OC_STATUS s);
        void WriteAutoReportInterval(device_mr_id id, h_uint t);
        void Open_Close(device_mr_id id, HDTAS_OC_STATUS c, HDTAS_OC_STATUS m, HDTAS_OC_STATUS p, HDTAS_OC_STATUS b);
        void Reboot_PB(device_mr_id id);
        void Reset_PB(device_mr_id id);

    private:
        class request_in;
        request_in* req_in_;
    };

    class HDTAS_PARSER_CLASS HdtasCtlMrPbResponse : public HdtasDataEntity
    {
    public:
        HdtasCtlMrPbResponse();
        virtual ~HdtasCtlMrPbResponse();

        h_int GetEcode() const;

        h_uint GetResponseID() const;
        HDTAS_CTL_TYPE GetResponseType() const;

        // Functions for mr.
        HDTAS_OC_STATUS GetAutoReportStatus() const;
        h_uint GetAutoReportInterval() const;
        const mr_status_array& GetStatusInfo() const;

        // Functions for power board.
        device_mr_id GetMrID() const;
        HDTAS_OC_STATUS GetAutoReportStatus2() const;
        h_uint GetAutoReportInterval2() const;
        const power_board_status& GetStatusInfo2() const;
        const power_board_status_group& GetStatusGroupInfo2() const;
    private:
        class response_in;
        response_in* rpn_in_;
    };

	class HDTAS_PARSER_CLASS HdtasCtlMrRequest : public HdtasDataEntity
	{
	public:
		HdtasCtlMrRequest();
		virtual ~HdtasCtlMrRequest();

		void SetRequestID( h_uint id );

		void ReadAutoReportStatus();
		void ReadAutoReportInterval();
		void ReadStatusInfo();

		void WriteAutoReportStatus( HDTAS_OC_STATUS s );
		void WriteAutoReportInterval( h_uint t );

		void Reboot( device_mr_id id );
		void Reset( device_mr_id id );
	private:
		class request_in;
		request_in* req_in_;
	};

	class HDTAS_PARSER_CLASS HdtasCtlMrResponse : public HdtasDataEntity
	{
	public:
		HdtasCtlMrResponse();
		virtual ~HdtasCtlMrResponse();

		h_int GetEcode() const;

		h_uint GetResponseID() const;
		HDTAS_CTL_TYPE GetResponseType() const;

		HDTAS_OC_STATUS GetAutoReportStatus() const;
		h_uint GetAutoReportInterval() const;

		const mr_status_array& GetStatusInfo() const;

	private:
		class response_in;
		response_in* rpn_in_;
	};

	class HDTAS_PARSER_CLASS HdtasCtlPbRequest : public HdtasDataEntity
	{
	public:
		HdtasCtlPbRequest();
		virtual ~HdtasCtlPbRequest();

		void SetRequestID( h_uint id );

		void ReadAutoReportStatus( device_mr_id id );
		void ReadAutoReportInterval( device_mr_id id );
		void ReadStatusInfo( device_mr_id id );

		void WriteAutoReportStatus( device_mr_id id, HDTAS_OC_STATUS s );
		void WriteAutoReportInterval( device_mr_id id, h_uint t );

		void Open_Close( device_mr_id id, HDTAS_OC_STATUS c, HDTAS_OC_STATUS m, HDTAS_OC_STATUS p, HDTAS_OC_STATUS b );

		void Reboot( device_mr_id id );
		void Reset( device_mr_id id );
	private:
		class request_in;
		request_in* req_in_;
	};

	class HDTAS_PARSER_CLASS HdtasCtlPbResponse : public HdtasDataEntity
	{
	public:
		HdtasCtlPbResponse();
		virtual ~HdtasCtlPbResponse();

		h_int GetEcode() const;

		h_uint GetResponseID() const;
		HDTAS_CTL_TYPE GetResponseType() const;

		device_mr_id GetMrID() const;

		HDTAS_OC_STATUS GetAutoReportStatus() const;
		h_uint GetAutoReportInterval() const;

		const power_board_status& GetStatusInfo() const;
		const power_board_status_group& GetStatusGroupInfo() const;

	private:
		class response_in;
		response_in* rpn_in_;
	};

	class HDTAS_PARSER_CLASS HdtasIspdData : public HdtasDataEntity
	{
	public:
		HdtasIspdData();
		virtual ~HdtasIspdData();

		const ispd_date_time& GetTimestamp() const;
		const isdp_data_array& GetDataInfo() const;
	private:
		class ispd_data_in;
		ispd_data_in* data_in_;
	};

	class HDTAS_PARSER_CLASS HdtasMrStatus : public HdtasDataEntity
	{
	public:
		HdtasMrStatus();
		virtual ~HdtasMrStatus();

		const mr_status_array& GetStatusInfo() const;
	private:
		class mr_status_in;
		mr_status_in* status_in_;
	};

	class HDTAS_PARSER_CLASS HdtasPbStatus : public HdtasDataEntity
	{
	public:
		HdtasPbStatus();
		virtual ~HdtasPbStatus();

		device_mr_id GetMrID() const;
		const power_board_status& GetStatusInfo() const;
		const power_board_status_group& GetStatusGroupInfo() const;
	private:
		class pb_status_in;
		pb_status_in* status_in_;
	};
}



#endif // HDTASPARSER_H#pragma once
