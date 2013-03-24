#include "stdafx.h"
#include "VisitorsList.h"

cVisitorsList::cVisitorsList()
{
}


void cVisitorsList::RenameVisitor(const uint_t visitor_id, const QString new_name)
{
	for (VisitorsMap::iterator it = visitors.begin(); it != visitors.end(); ++it)
	{
		if ((*it).GetId() == visitor_id)
		{
			const cVisitor old_visitor = *it;
			cVisitor &new_visitor = *it;
			new_visitor.Rename(new_name);

			OnVisitorUpdated(old_visitor, new_visitor);
		}
	}
}

/*
void cVisitorsList::UpdateVisitorsFromXML(const QString &string)
{
	QXmlStreamReader xml(string);

	while(!xml.atEnd())
	{
		if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "Visitors")
		{
			while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "Visitors"))
			{
				if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "Visitor")
				{
					uint_t visitor_id = 0;
					QXmlStreamAttributes attributes = xml.attributes();

					// get visitor id
					if (attributes.hasAttribute("Id"))
						visitor_id = attributes.value("Id").toString().toULong();

					xml.readNext();
					if (xml.hasError())
					{
						OnErrorParseXML();
						return;
					}

					cVisitor visitor(visitor_id);

					// fill visitor with params from XML
					while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "Visitor"))
					{
						if(xml.tokenType() == QXmlStreamReader::StartElement)
							visitor.SetParamFromXML(xml);

						xml.readNext();
						if (xml.hasError())
						{
							OnErrorParseXML();
							return;
						}
					}

					visitor_id = visitor.GetId();

					VisitorsMap::const_iterator it = visitors.find(visitor_id);
					if (it == visitors.end())
					{
						// insert new visitor
						visitors.insert(visitor_id, visitor);
						OnVisitorAdded(visitor);
					}
					else
					{
						// update exists visitor
						const cVisitor temp = *it;
//						if (temp.GetName() != visitor.GetName() || temp.GetStatus() != visitor.GetStatus())
						if (temp.GetHash() != visitor.GetHash())
							OnVisitorUpdated(temp, visitor); // TODO: split in 2 events

						if (temp.GetHash() != visitor.GetHash())
						{
							visitors.remove(visitor_id);
							visitors.insert(visitor_id, visitor);
						}
					}

				}

				xml.readNext();
				if (xml.hasError())
				{
					OnErrorParseXML();
					return;
				}
			}
		}
	
		xml.readNext();
		if (xml.hasError())
		{
			OnErrorParseXML();
			return;
		}
	};


	return;
}
//*/

void cVisitorsList::UpdateFromXML(const CNodeXML &root)
{
	bool is_new_visitor(false);
	bool is_waiting_visitor(false);
	bool is_away_visitor(false);

	const bool exapand_section = visitors.count() == 0;

	CNodeXML visitors_xml;
	if (root.GetNodeByPath("Visitors", visitors_xml) && visitors_xml.GetNodesCount() > 0 && visitors_xml.NodeStart())
	{
		do
		{
			CNodeXML visitor_xml = visitors_xml.GetNode();

			// get visitor id
			uint_t visitor_id = 0;
			if (visitor_xml.HasAttribute("Id"))
				visitor_id = visitor_xml.GetAttributeString("Id").toUInt();

			cVisitor visitor(visitor_id);
			visitor.UpdateFromXML(visitor_xml);

			visitor_id = visitor.GetId();
			VisitorsMap::const_iterator it = visitors.find(visitor_id);
			if (it == visitors.end())
			{
				// insert new visitor
				visitors.insert(visitor_id, visitor);
				OnVisitorAdded(visitor, exapand_section);

				// event: new visitor
				is_new_visitor |= true;
			}
			else
			{
				// update exists visitor
				const cVisitor temp = *it;
//						if (temp.GetName() != visitor.GetName() || temp.GetStatus() != visitor.GetStatus())
				if (temp.GetHash() != visitor.GetHash())
					OnVisitorUpdated(temp, visitor); // TODO: split in 2 events

				if (temp.GetStatus() != cVisitor::WaitingChat && visitor.GetStatus() == cVisitor::WaitingChat)
				{
					// event: is_waiting_visitor
					is_waiting_visitor |= true;
				}

				if (temp.GetHash() != visitor.GetHash())
				{
					visitors.remove(visitor_id);
					visitors.insert(visitor_id, visitor);
				}
			}

		} while (visitors_xml.NodeNext());
	}

	CNodeXML obsolete_visitors_xml;
	if (root.GetNodeByPath("ObsoleteVisitors", obsolete_visitors_xml) && obsolete_visitors_xml.GetNodesCount() > 0 && obsolete_visitors_xml.NodeStart())
	{
		do
		{
			CNodeXML visitor_xml = obsolete_visitors_xml.GetNode();

			// get visitor id
			uint_t visitor_id = 0;
			if (visitor_xml.HasAttribute("Id"))
				visitor_id = visitor_xml.GetAttributeString("Id").toUInt();

			if (visitor_id > 0)
			{
				// remove visitor and do broadcast
				cVisitor obsolete_visitor(visitors[visitor_id]);
				visitors.remove(visitor_id);
				obsolete_visitor.SetObsolete(true);
				OnVisitorRemoved(obsolete_visitor);

				// event: away visitor
				is_away_visitor |= true;
			}

		} while (obsolete_visitors_xml.NodeNext());
	}

	if (is_new_visitor)
		EventNewVisitor();

	if (is_waiting_visitor)
		EventWaitingVisitor();

	if (is_away_visitor)
		EventAwayVisitor();
}

/*
void cVisitorsList::ObsoleteVisitorsFromXML(const QString &string)
{
	QXmlStreamReader xml(string);

	while(!xml.atEnd())
	{
		if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "ObsoleteVisitors")
		{
			while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "ObsoleteVisitors"))
			{
				if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "Visitor")
				{
					QXmlStreamAttributes attributes = xml.attributes();

					uint_t visitor_id = 0;
					if (attributes.hasAttribute("Id"))
						visitor_id = attributes.value("Id").toString().toULong();

					if (visitor_id > 0)
					{
						// remove visitor and do broadcast
						cVisitor obsolete_visitor(visitors[visitor_id]);
						visitors.remove(visitor_id);
						obsolete_visitor.SetObsolete(true);
						OnVisitorRemoved(obsolete_visitor);
					}
				}

				xml.readNext();
				if (xml.hasError())
					return;
			}
		}

		xml.readNext();
		if (xml.hasError())
			return;
	}

	return;
}
//*/

/*
void cVisitorsList::UpdateFromXML(const QString &string)
{
//	UpdateVisitorsFromXML(string);
	ObsoleteVisitorsFromXML(string);
}
//*/

void cVisitorsList::GetVisitorsDOM(QDomElement &root) const
{
	QDomDocument doc;

	QDomElement elem = doc.createElement("Visitors");
	root.appendChild(elem);

	for (VisitorsMap::const_iterator it = visitors.begin(); it != visitors.end(); ++it)
	{
		const cVisitor &visitor = *it;
		visitor.GetShortParamsDOM(elem);
	}
}

void cVisitorsList::Cleanup()
{
	for (VisitorsMap::iterator it = visitors.begin(); it != visitors.end(); ++it)
		OnVisitorRemoved(*it);

	visitors.clear();
}

bool cVisitorsList::GetVisitor(const uint_t id, cVisitor &visitor)
{
	VisitorsMap::iterator it = visitors.find(id);
	if (it == visitors.end())
		return false;

	visitor = *it;
	return true;
}