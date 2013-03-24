#include "stdafx.h"
#include "Translator.h"

LabelsArray CTranslator::labels;

QString CTranslator::translate(const char *context, const char *sourceText, const char *disambiguation) const
{
	if (disambiguation != NULL)
	{
		QString s(disambiguation);
		LabelsArray::const_iterator it = labels.find(s);
		if (it != labels.end())
		{
			return it.value();
		}
	}

	return QTranslator::translate(context, sourceText, disambiguation);
}

LanguageInfoArray CTranslator::GetLanguages()
{
	QDir dir(LANGUAGE_DIR, QString("*.") + LANGUAGE_FILE_EXTENSION);
	QStringList files = dir.entryList();

	LanguageInfoArray infos;
	for (int i = 0; i < files.count(); ++i)
	{
		const QString filename = files[i];

		QSettings lang(get_filename(filename), QSettings::IniFormat);
		lang.setIniCodec("UTF-8");

		const QString native_language_name = lang.value("Information/LanguageName").toString();
		if (native_language_name == "")
			continue;

		LanguageInfo info;
		info.filename = filename;
		info.name = native_language_name;

		infos.push_back(info);
	}

	return infos;
}

void CTranslator::LoadLanguage(const QString &filename)
{
	labels.clear();

	QSettings lang(get_filename(filename), QSettings::IniFormat);
	lang.setIniCodec("UTF-8");
	if (lang.status() != QSettings::NoError)
		return;

	lang.beginGroup("Labels");
	QStringList keys = lang.allKeys();
	lang.endGroup();

	for (int i = 0; i < keys.count(); ++i)
	{
		const QString key = keys[i];
		const QString value = lang.value("Labels/" + key).toString();
		labels.insert(key, value);
	}

}