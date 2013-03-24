#include "stdafx.h"
#include "DialogShortcut.h"

CDialogShortcut::CDialogShortcut(QKeySequence &shortcut)
:control_key(0), key(0), m_CloseResult(CloseCancel), pressed_count(0)
{
	ui.setupUi(this);

	set_shortcut(shortcut);
}

void CDialogShortcut::display_shortcut()
{
	ui.leShortcut->setText(m_Shortcut.toString());
}

void CDialogShortcut::set_shortcut(const QKeySequence &shortcut)
{
	m_Shortcut = shortcut;

	QSet<QString>::const_iterator it = m_ExistsShortcuts.find(m_Shortcut);
	const bool exists = (m_ExistsShortcuts.size() == 0 || it == m_ExistsShortcuts.end()) ? false : true;
	ui.pbAssign->setEnabled(!exists);
	ui.lShortcutExists->setVisible(exists);

	display_shortcut();
}

void CDialogShortcut::keyPressEvent(QKeyEvent *event)
{
	if (pressed_count <= 0)
	{
		control_key = 0;
		key = 0;
	}

	switch (event->key())
	{
		case Qt::Key_Control:
			control_key |= Qt::CTRL;
			key = 0;
			++pressed_count;
		break;

		case Qt::Key_Alt:
			control_key |= Qt::ALT;
			key = 0;
			++pressed_count;
		break;

		case Qt::Key_Shift:
			control_key |= Qt::SHIFT;
			key = 0;
			++pressed_count;
		break;

		default:
			if (event->key() & 0x0ff000000)
				break;

			key = event->key();
			++pressed_count;
		break;
	}

	if (key != 0)// && control_key != 0)
	{
		set_shortcut(QKeySequence(control_key + key));
	}
	else if (control_key != 0)
	{
		ui.leShortcut->setText(QKeySequence(control_key + key));
	}

}

void CDialogShortcut::keyReleaseEvent(QKeyEvent *event)
{
	switch (event->key())
	{
		case Qt::Key_Control:
			--pressed_count;
		break;

		case Qt::Key_Alt:
			--pressed_count;
		break;

		case Qt::Key_Shift:
			--pressed_count;
		break;

		default:
			if (event->key() & 0x0ff000000)
				break;

			--pressed_count;
	}

	if (pressed_count <= 0 && key == 0)
	{
		set_shortcut(QKeySequence());
	}
}

void CDialogShortcut::OnAssign()
{
	m_CloseResult = CloseAssign;
	close();
}

void CDialogShortcut::OnRemove()
{
	m_CloseResult = CloseRemove;
	close();
}

void CDialogShortcut::OnCancel()
{
	m_CloseResult = CloseCancel;
	close();
}