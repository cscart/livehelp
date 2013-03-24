#pragma once

#include "ui_DialogShortcut.h"

class CDialogShortcut : public QDialog
{
	Q_OBJECT

public:
	enum CloseType
	{
		CloseAssign,
		CloseRemove,
		CloseCancel
	};

private:
	Ui::DialogShortcut ui;

	QKeySequence m_Shortcut;

	int control_key;
	int key;
	int pressed_count;

	CloseType m_CloseResult;

	QSet<QString> m_ExistsShortcuts;

	void display_shortcut();
	void set_shortcut(const QKeySequence &shortcut);

protected:
	virtual void keyPressEvent(QKeyEvent *event);
	virtual void keyReleaseEvent(QKeyEvent *event);

public:
	CDialogShortcut(QKeySequence &shortcut);
	~CDialogShortcut() {}

	CloseType GetCloseResult() const { return m_CloseResult; }
	const QKeySequence& GetShortcut() const { return m_Shortcut; }

	void SetExistsShortcuts(const QSet<QString> &set) { m_ExistsShortcuts = set; }

private slots:;

	void OnAssign();
	void OnRemove();
	void OnCancel();

};
