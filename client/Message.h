#pragma once

#include "common.h"

class cMessage
{
public:
	enum Direction {VisitorToOperator, OperatorToVisitor, ChatEvent};

private:
	uint_t id;
	uint_t visitor_id;
	QString text;
	Direction direction;


public:
	cMessage();
	cMessage(const uint_t _id, const Direction _direction=VisitorToOperator);
	~cMessage();

	void SetId(const uint_t _id) { id = _id; }
	void SetVisitorId(const uint_t _id) { visitor_id = _id; }
	void SetText(const QString &_text) { text = _text; }
	void SetDirection(const Direction _direction) { direction = _direction; }


	uint_t GetId() const { return id; }
	uint_t GetVisitorId() const { return visitor_id; }
	QString GetText() const { return text; }
	Direction GetDirection() const { return direction; }
	QIcon GetDirectionIcon() const;

//	bool SetParamFromXML(QXmlStreamReader &xml);
	bool SetParamFromXML(const CNodeXML &root);

};
