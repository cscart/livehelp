#pragma once

#include <QList>
#include <QMap>
#include <QTranslator>

#include "defines.h"

struct LanguageInfo
{
	QString name;
	QString filename;
};

typedef QList<LanguageInfo> LanguageInfoArray;
typedef QMap<QString, QString> LabelsArray;

class CTranslator : public QTranslator
{
private:
	static LabelsArray labels;

	static QString get_filename(const QString &filename) { return LANGUAGE_DIR + QDir::separator() + filename; }

public:
	QString translate(const char *context, const char *sourceText, const char *disambiguation = 0) const;

	static LanguageInfoArray GetLanguages();
	static void LoadLanguage(const QString &filename);
};


class ILanguageUI
{
public:
	virtual void ApplyLanguageLabels() = 0;
};

static CTranslator& GetTranslator()
{
	static CTranslator translator;
	return translator;
}