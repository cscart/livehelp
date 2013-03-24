#pragma once

#include "WidgetUpdating.h"

class IBehaviourUpdating
{
private:
	WidgetUpdating *m_UpdatingWidget;

	int attempts;

public:
	IBehaviourUpdating();
	~IBehaviourUpdating();

	bool UpdateBegin(QWidget *parent, int try_attempts=1);
	bool UpdateEnd();
	bool TryUpdateEnd();

	virtual void OnEnable(const bool enabled) = 0;
};
