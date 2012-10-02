#ifndef Teredo_h
#define Teredo_h

#include "Analyzer.h"
#include "NetVar.h"

class Teredo_Analyzer : public Analyzer {
public:
	Teredo_Analyzer(Connection* conn) : Analyzer(AnalyzerTag::Teredo, conn),
	                                    valid_orig(false), valid_resp(false)
		{}

	virtual ~Teredo_Analyzer()
		{}

	virtual void Done();

	virtual void DeliverPacket(int len, const u_char* data, bool orig,
					int seq, const IP_Hdr* ip, int caplen);

	static Analyzer* InstantiateAnalyzer(Connection* conn)
		{ return new Teredo_Analyzer(conn); }

	static bool Available()
		{ return BifConst::Tunnel::enable_teredo &&
		         BifConst::Tunnel::max_depth > 0; }

	/**
	 * Emits a weird only if the analyzer has previously been able to
	 * decapsulate a Teredo packet in both directions or if *force* param is
	 * set, since otherwise the weirds could happen frequently enough to be less
	 * than helpful.  The *force* param is meant for cases where just one side
	 * has a valid encapsulation and so the weird would be informative.
	 */
	void Weird(const char* name, bool force = false) const
		{
		if ( ProtocolConfirmed() || force )
			reporter->Weird(Conn(), name);
		}

	/**
	 * If the delayed confirmation option is set, then a valid encapsulation
	 * seen from both end points is required before confirming
	 */
	void Confirm()
		{
		if ( ! BifConst::Tunnel::delay_teredo_confirmation ||
		     ( valid_orig && valid_resp ) )
			ProtocolConfirmation();
		}

protected:
	friend class AnalyzerTimer;
	void ExpireTimer(double t);

	bool valid_orig;
	bool valid_resp;
};

class TeredoEncapsulation {
public:
	TeredoEncapsulation(const Teredo_Analyzer* ta)
		: inner_ip(0), origin_indication(0), auth(0), analyzer(ta)
		{}

	/**
	 * Returns whether input data parsed as a valid Teredo encapsulation type.
	 * If it was valid, the len argument is decremented appropriately.
	 */
	bool Parse(const u_char* data, int& len)
		{ return DoParse(data, len, false, false); }

	const u_char* InnerIP() const
		{ return inner_ip; }

	const u_char* OriginIndication() const
		{ return origin_indication; }

	const u_char* Authentication() const
		{ return auth; }

	RecordVal* BuildVal(const IP_Hdr* inner) const;

protected:
	bool DoParse(const u_char* data, int& len, bool found_orig, bool found_au);

	void Weird(const char* name) const
		{ analyzer->Weird(name); }

	const u_char* inner_ip;
	const u_char* origin_indication;
	const u_char* auth;
	const Teredo_Analyzer* analyzer;
};

#endif
