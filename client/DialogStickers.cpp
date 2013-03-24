#include "stdafx.h"
#include "DialogStickers.h"

CDialogStickerView::CDialogStickerView(const CStickerRecord &sticker)
{
	ui.setupUi(this);

	ui.lTitle->setText(sticker.GetTitle());
	ui.lAuthor->setText(sticker.GetOperatorName());
	ui.lDate->setText(sticker.GetDateString(false));

	ui.teMessage->setPlainText(sticker.GetMessage());
}




CDialogStickerCreate::CDialogStickerCreate()
{
	ui.setupUi(this);
}
