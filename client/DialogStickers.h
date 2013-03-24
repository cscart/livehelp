#pragma once

#include <QDialog>
#include "ui_DialogStickerView.h"
#include "ui_DialogStickerCreate.h"
#include "Stickers.h"

class CDialogStickerView : public QDialog
{
private:
	Ui::DialogStickerView ui;

public:
	CDialogStickerView(const CStickerRecord &sticker);

	bool IsMarkUnread() const { return ui.cbMarkAsUnread->isChecked(); }
};


class CDialogStickerCreate : public QDialog
{
private:
	Ui::DialogStickerCreate ui;

public:
	CDialogStickerCreate();

	QString GetTitle() const { return ui.leTitle->text(); }
	QString GetMessage() const { return ui.ptMessage->toPlainText(); }
};