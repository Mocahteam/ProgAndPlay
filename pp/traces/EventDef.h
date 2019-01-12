/**
 * \file EventDef.h
 * \brief Déclaration des classes dérivées de la classe Event
 * \author meresse
 * \version 0.1
 */

#ifndef __EVENT_DEF_H__
#define __EVENT_DEF_H__

#include <boost/lexical_cast.hpp>
#include "TraceConstantList.h"

class StartMissionEvent : public Event
{

  public:
	StartMissionEvent(std::string mission_name, int start_time) : Event(MISSION_START_TIME), mission_name(mission_name), start_time(start_time) {}

	StartMissionEvent(const StartMissionEvent *sme) : Event(sme)
	{
		mission_name = sme->mission_name;
		start_time = sme->start_time;
	}

	virtual Trace::sp_trace clone() const
	{
		return boost::make_shared<StartMissionEvent>(this);
	}

	virtual std::string getParams() const
	{
		return mission_name + " " + boost::lexical_cast<std::string>(start_time);
	}

	std::string getMissionName() const
	{
		return mission_name;
	}

	int getStartTime() const
	{
		return start_time;
	}

  private:
	std::string mission_name;
	int start_time;
};

class EndMissionEvent : public Event
{

  public:
	EndMissionEvent(std::string status, int end_time) : Event(MISSION_END_TIME), status(status), end_time(end_time) {}

	EndMissionEvent(const EndMissionEvent *eme) : Event(eme)
	{
		status = eme->status;
		end_time = eme->end_time;
	}

	virtual Trace::sp_trace clone() const
	{
		return boost::make_shared<EndMissionEvent>(this);
	}

	virtual std::string getParams() const
	{
		return status + " " + boost::lexical_cast<std::string>(end_time);
	}

	std::string getStatus() const
	{
		return status;
	}

	int getEndTime() const
	{
		return end_time;
	}

  private:
	std::string status;
	int end_time;
};

class NewExecutionEvent : public Event
{

  public:
	NewExecutionEvent(int start_time, std::string prog_lang_used) : Event(EXECUTION_START_TIME), start_time(start_time), prog_lang_used(prog_lang_used) {}

	NewExecutionEvent(const NewExecutionEvent *nee) : Event(nee)
	{
		start_time = nee->start_time;
		prog_lang_used = nee->prog_lang_used;
	}

	virtual Trace::sp_trace clone() const
	{
		return boost::make_shared<NewExecutionEvent>(this);
	}

	virtual std::string getParams() const
	{
		return boost::lexical_cast<std::string>(start_time) + " " + prog_lang_used;
	}

	int getStartTime() const
	{
		return start_time;
	}

	std::string getProgrammingLangageUsed() const
	{
		return prog_lang_used;
	}

  private:
	int start_time;

	std::string prog_lang_used;
};

class EndExecutionEvent : public Event
{

  public:
	EndExecutionEvent(int end_time) : Event(EXECUTION_END_TIME), end_time(end_time) {}

	EndExecutionEvent(const EndExecutionEvent *eee) : Event(eee)
	{
		end_time = eee->end_time;
	}

	virtual Trace::sp_trace clone() const
	{
		return boost::make_shared<EndExecutionEvent>(this);
	}

	virtual std::string getParams() const
	{
		return boost::lexical_cast<std::string>(end_time);
	}

	int getEndTime() const
	{
		return end_time;
	}

  private:
	int end_time;
};

#endif
