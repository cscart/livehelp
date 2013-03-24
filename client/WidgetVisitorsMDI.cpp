#include "stdafx.h"
#include "WidgetVisitorsMDI.h"

cWidgetVisitorsMDI::cWidgetVisitorsMDI()
{
	ui.setupUi(this);

	connect(ui.mdiArea, SIGNAL(subWindowActivated(QMdiSubWindow*)), this, SLOT(subWindowActivated(QMdiSubWindow*)));
}

cWidgetVisitorsMDI::~cWidgetVisitorsMDI()
{}

void cWidgetVisitorsMDI::ApplyLanguageLabels()
{
	QList<QMdiSubWindow*> list = ui.mdiArea->subWindowList();
	for (int i = 0; i < list.size(); ++i)
	{
		QMdiSubWindow *subWindow = list.at(i);
		if (subWindow == NULL)
			continue;

		cWidgetVisitorDoc *doc = static_cast<cWidgetVisitorDoc*>(subWindow->widget());
		if (doc != NULL)
			doc->ApplyLanguageLabels();
	}
}

void cWidgetVisitorsMDI::AddVisitor(const cVisitor &visitor)
{
	QMdiSubWindow *subWindow = NULL;

	// if visitor already opened move MDI window on top, otherwise create new one
	if (find_visitor_doc(visitor.GetId(), &subWindow))
	{
		if (subWindow->isMinimized())
			subWindow->showMaximized();

		ui.mdiArea->setActiveSubWindow(subWindow);
		return;
	}

	// create new MDI window
	cWidgetVisitorDoc *widget = new cWidgetVisitorDoc(visitor);
	subWindow = ui.mdiArea->addSubWindow(widget);
	if (subWindow)
	{
		subWindow->setWindowIcon(visitor.GetStatusIcon());
		connect(widget, SIGNAL(destroyed(QObject*)), this, SLOT(destroyVisitorDoc(QObject*)));
		OnOpenWindow(visitor);
		subWindow->showMaximized();
	}

}

void cWidgetVisitorsMDI::UpdateVisitor(const cVisitor &visitor, const bool)
{
	UpdateVisitor(visitor, visitor);
}

void cWidgetVisitorsMDI::UpdateVisitor(const cVisitor &old_visitor, const cVisitor &new_visitor)
{
	QMdiSubWindow *subWindow = NULL;
	cWidgetVisitorDoc *doc = find_visitor_doc(old_visitor.GetId(), &subWindow);
	if (doc != NULL)
	{
		doc->UpdateVisitor(new_visitor);
		refresh_sub_window(new_visitor, subWindow);
		OnUpdateWindow(old_visitor, new_visitor);
	}
}

void cWidgetVisitorsMDI::RemoveVisitor(const cVisitor &visitor)
{
	QMdiSubWindow *subWindow = NULL;
	cWidgetVisitorDoc *doc = find_visitor_doc(visitor.GetId(), &subWindow);
	if (doc != NULL)
	{
		const QString title = cVisitor::GetNoUserDesrc() + " - [" + visitor.GetCombinedName() + "]";
		subWindow->setWindowTitle(title);

		// icon
		subWindow->setWindowIcon(cVisitor::GetNoUserIcon());

		doc->UpdateVisitor(visitor);
	}
}



//cWidgetVisitorDoc* cWidgetVisitorsMDI::find_visitor_doc(const cVisitor &visitor, QMdiSubWindow **sub_window)
cWidgetVisitorDoc* cWidgetVisitorsMDI::find_visitor_doc(const uint_t visitor_id, QMdiSubWindow **sub_window)
{
	QList<QMdiSubWindow*> list = ui.mdiArea->subWindowList();
	for (int i = 0; i < list.size(); ++i)
	{
		QMdiSubWindow *subWindow = list.at(i);
		if (subWindow != NULL)
		{
			if (sub_window != NULL)
				*sub_window = subWindow;

			cWidgetVisitorDoc *doc = static_cast<cWidgetVisitorDoc*>(subWindow->widget());
			if (doc && doc->GetVisitorId() == visitor_id)
				return doc;
		}
	}

	return NULL;
}

void cWidgetVisitorsMDI::refresh_sub_window(const cVisitor &visitor, QMdiSubWindow *subWindow)
{
	if (subWindow == NULL)
		return;

	const QString title = visitor.GetStatusDescr() + " - [" + visitor.GetCombinedName() + "]";
	subWindow->setWindowTitle(title);

	// icon
	subWindow->setWindowIcon(visitor.GetStatusIcon());
}

void cWidgetVisitorsMDI::destroyVisitorDoc(QObject *object)
{
	cWidgetVisitorDoc *doc = static_cast<cWidgetVisitorDoc*>(object);
	if (doc != NULL)
		OnCloseWindow(doc->GetVisitorId());
}

void cWidgetVisitorsMDI::subWindowActivated(QMdiSubWindow *subWindow)
{
	if (subWindow == NULL)
		return;

	cWidgetVisitorDoc *doc = static_cast<cWidgetVisitorDoc*>(subWindow->widget());
	if (doc != NULL)
		OnWindowActivated(doc->GetVisitorId());
}

void cWidgetVisitorsMDI::SelectWindow(const cVisitor& visitor)
{
	QMdiSubWindow *subWindow = NULL;
	if (find_visitor_doc(visitor.GetId(), &subWindow))
	{
		const bool old_param = ui.mdiArea->blockSignals(true);
		ui.mdiArea->setActiveSubWindow(subWindow);
		ui.mdiArea->blockSignals(old_param);
	}
}



void cWidgetVisitorsMDI::GetVisitorParamsDOM(QDomElement &root) const
{
	QDomDocument doc;

	QDomElement elem = doc.createElement("ChatMessages");
	root.appendChild(elem);

	QDomElement elem_notes = doc.createElement("Notes");
	root.appendChild(elem_notes);

	QDomElement elem_files = doc.createElement("Files");
	root.appendChild(elem_files);

	QList<QMdiSubWindow*> list = ui.mdiArea->subWindowList();
	for (int i = 0; i < list.size(); ++i)
	{
		QMdiSubWindow *subWindow = list.at(i);
		if (subWindow != NULL)
		{
			cWidgetVisitorDoc *doc = static_cast<cWidgetVisitorDoc*>(subWindow->widget());
			doc->GetMessageDOM(elem);
			doc->GetNotesDOM(elem_notes);
			doc->GetFilesDOM(elem_files);
		}
	}
}

/*
void cWidgetVisitorsMDI::UpdateFromXML(const QString &string)
{
	UpdateChatMessagesFromXML(string);
}

void cWidgetVisitorsMDI::UpdateChatMessagesFromXML(const QString &string)
{
	QXmlStreamReader xml(string);

	while(!xml.atEnd())
	{
		if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "ChatMessages")
		{
			while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "ChatMessages"))
			{
				if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "Message")
				{
					QXmlStreamAttributes attributes = xml.attributes();

					uint_t message_id = 0;
					if (attributes.hasAttribute("Id"))
						message_id = attributes.value("Id").toString().toULong();

					uint_t visitor_id = 0;
					if (attributes.hasAttribute("VisitorId"))
						visitor_id = attributes.value("VisitorId").toString().toULong();

					if (visitor_id > 0)
					{
						QMdiSubWindow *subWindow = NULL;
						cWidgetVisitorDoc *visitor_doc = find_visitor_doc(visitor_id, &subWindow);
						if (visitor_doc != NULL)
						{
							cMessage message(message_id);
							message.SetVisitorId(visitor_id);

							while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "Message"))
							{
								message.SetParamFromXML(xml);

								xml.readNext();
								if (xml.hasError())
								{
									OnErrorParseXML();
									return;
								}
							}

							// print message in chat window
							visitor_doc->DisplayMessage(message);

							xml.readNext();
							if (xml.hasError())
							{
								OnErrorParseXML();
								return;
							}
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
	}

	return;
}
//*/


void cWidgetVisitorsMDI::UpdateFromXML(const CNodeXML &root)
{
	UpdateChatMessagesFromXML(root);
}


void cWidgetVisitorsMDI::UpdateChatMessagesFromXML(const CNodeXML &root)
{
	bool is_new_chat_message(false);

	CNodeXML messages_xml;
	if (root.GetNodeByPath("ChatMessages", messages_xml) && messages_xml.GetNodesCount() > 0 && messages_xml.NodeStart())
	{
		do
		{
			CNodeXML message_xml = messages_xml.GetNode();

			uint_t message_id = 0;
			if (message_xml.HasAttribute("Id"))
				message_id = message_xml.GetAttributeString("Id").toUInt();

			uint_t visitor_id = 0;
			if (message_xml.HasAttribute("VisitorId"))
				visitor_id = message_xml.GetAttributeString("VisitorId").toULong();

			if (visitor_id > 0)
			{
				QMdiSubWindow *subWindow = NULL;
				cWidgetVisitorDoc *visitor_doc = find_visitor_doc(visitor_id, &subWindow);
				if (visitor_doc != NULL)
				{
					cMessage message(message_id);
					message.SetVisitorId(visitor_id);
					message.SetParamFromXML(message_xml);

					// print message in chat window
					visitor_doc->DisplayMessage(message);

					// event: new chat message
					is_new_chat_message |= true;
				}
			}

		} while (messages_xml.NodeNext());
	}

	if (is_new_chat_message)
		EventNewChatMessage();
}


void cWidgetVisitorsMDI::HasOpenedChat(bool &result)
{
	result = false;

	QList<QMdiSubWindow*> list = ui.mdiArea->subWindowList();
	for (int i = 0; i < list.size(); ++i)
	{
		QMdiSubWindow *subWindow = list.at(i);
		if (subWindow != NULL)
		{
			cWidgetVisitorDoc *doc = static_cast<cWidgetVisitorDoc*>(subWindow->widget());
			if (doc && doc->IsVisitorChatting())
			{
				result = true;
				return;
			}
		}
	}
}
