#pragma once

#include <QMap>

#include "Visitor.h"

class cVisitorsList : public QObject
{
	Q_OBJECT

private:
	typedef QMap<uint_t, cVisitor> VisitorsMap;

private:
	VisitorsMap visitors;

//	void UpdateVisitorsFromXML(const QString &string);
//	void ObsoleteVisitorsFromXML(const QString &string);

public:
	cVisitorsList();
	~cVisitorsList() {}

//	void UpdateFromXML();

	bool GetVisitor(const uint_t id, cVisitor &visitor);


	const int GetSize() const { return visitors.size(); }


	void RenameVisitor(const uint_t visitor_id, const QString new_name);

public slots:;

//	void UpdateFromXML(const QString &string);
	void UpdateFromXML(const CNodeXML &root);

	void GetVisitorsDOM(QDomElement &root) const;

	void Cleanup();


signals:;

	void OnVisitorAdded(const cVisitor&, const bool expand);
	void OnVisitorUpdated(const cVisitor&, const cVisitor&);
	void OnVisitorRemoved(const cVisitor&);

	void OnErrorParseXML();


	void EventNewVisitor();
	void EventWaitingVisitor();
	void EventAwayVisitor();
};
