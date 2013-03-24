#include "stdafx.h"
#include "Message.h"

cMessage::cMessage()
:id(0), visitor_id(0), direction(VisitorToOperator)
{
}

cMessage::cMessage(const uint_t _id, const Direction _direction)
:id(_id), visitor_id(0), direction(_direction)
{

}

cMessage::~cMessage()
{
}

bool cMessage::SetParamFromXML(const CNodeXML &root)
{
	root.GetValueByPath("Text", text);
	return true;
}

QIcon cMessage::GetDirectionIcon() const
{
	QString icon_path(":/lh3/Resources/operators.png");

	switch (direction)
	{
		case VisitorToOperator:
			icon_path = ":/statuses/Resources/visitor_chatting.png";
		break;
	};

	return QIcon(icon_path);
}