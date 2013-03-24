#include "stdafx.h"
#include "BehaviourUpdating.h"

IBehaviourUpdating::IBehaviourUpdating()
:m_UpdatingWidget(NULL), attempts(0)
{
}

IBehaviourUpdating::~IBehaviourUpdating()
{
	if (m_UpdatingWidget != NULL)
	{
		delete m_UpdatingWidget;
		m_UpdatingWidget = NULL;
	}
}

bool IBehaviourUpdating::UpdateBegin(QWidget *parent, int try_attempts)
{
	attempts = try_attempts;

	if (m_UpdatingWidget != NULL)
		UpdateEnd();

	m_UpdatingWidget = new WidgetUpdating(parent);
	m_UpdatingWidget->show();
	OnEnable(false);

	return true;
}

bool IBehaviourUpdating::UpdateEnd()
{
	if (m_UpdatingWidget == NULL)
		return true;

	delete m_UpdatingWidget;
	m_UpdatingWidget = NULL;

	OnEnable(true);

	return true;
}

bool IBehaviourUpdating::TryUpdateEnd()
{
	if (m_UpdatingWidget == NULL)
		return true;

	if (--attempts <= 0)
	{
		attempts = 0;
		return UpdateEnd();
	}

	return false;
}
