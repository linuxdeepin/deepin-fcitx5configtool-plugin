/*
 * SPDX-FileCopyrightText: 2017~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#include "glo.h"
#include "imelog.h"

#include <DPinyin>
DCORE_USE_NAMESPACE

QMap<QString, QString> g_chineseToEnglishMap;
QMap<QString, QString> g_englishToChineseMap;

FcitxQtStringKeyValueList g_useIMList;
int g_maxLanguageIndex = 0;

typedef struct s_languageList {
	char chineseName[64];
	char englishName[64];
} s_languageList;

s_languageList languageList[] = {
    {"越南语",   "Vietnamese" },
    {"英语",     "English"    },
    {"印尼语",   "Indonesian" },
    {"印地语",   "Hindi"      },
    {"意大利语", "Italian"    },
    {"伊拉克",   "Iraqi"      },
    {"亚美尼亚语",   "Armenian"  },
    {"匈牙利",       "Hungarian" },
    {"希腊语",       "Greek"     },
    {"希伯来语",     "Hebrew"    },
    {"西班牙",       "Spanish"   },
    {"西班牙语",     "Spanish"   },
    {"乌兹别克语",   "Uzbek"     },
    {"乌克兰语",     "Ukrainian" },
    {"乌尔都语",     "Urdu"      },
    {"沃洛夫语",     "Wolof"     },
    {"土库曼语",     "Turkmen"   },
    {"土耳其",       "Turkish"   },
    {"泰语",         "Thai"      },
    {"台湾",         "Taiwan"    },
    {"塔吉克",       "Tajik"     },
    {"斯瓦希里语",   "Swahili"   },
    {"斯洛文尼亚语", "Slovenian" },
    {"斯洛伐克语",   "Slovak"    },
    {"世界语",       "Esperanto"  },
    {"僧伽罗语",     "Sinhala"    },
    {"塞尔维亚语",   "Serbian"    },
    {"瑞典语",       "Swedish"    },
    {"日语",         "Japanese"   },
    {"葡萄牙",       "Portuguese" },
    {"葡萄牙语",     "Portuguese" },
    {"挪威语",       "Norwegian"  },
    {"尼泊尔语",     "Nepali"     },
    {"摩尔多瓦语",   "Moldavian"  },
    {"缅甸",         "Burmese"    },
    {"孟加拉语",     "Bangla"     },
    {"蒙古",         "Mongolian"  },
    {"毛利语",       "Maori"      },
    {"盲文",         "Braille"    },
    {"马其顿语",     "Macedonian" },
    {"马来语",       "Malay"      },
    {"马耳他语",     "Maltese"    },
    {"罗马尼亚语",   "Romanian"   },
    {"立陶宛语",     "Lithuanian" },
    {"老挝语",       "Lao"        },
    {"拉脱维亚",     "Latvian"    },
    {"克罗地亚",     "Croatian"   },
    {"柯尔克孜语",   "Kirgiz"     },
    {"捷克",         "Czech"      },
    {"简体中文 (中国)", "Chinese"     },
    {"黑山语",          "Montenegrin" },
    {"荷兰语",          "Dutch"       },
    {"汉语",            "Chinese"     },
    {"哈萨克语",        "Kazakh"      },
    {"国际音标",        "International Phonetic Alphabet"},
    {"格鲁吉亚语",      "Georgian"   },
    {"高棉语",          "Khmer"      },
    {"芬兰语",          "Finnish"    },
    {"菲律宾语",        "Filipino"   },
    {"法语",            "French"     },
    {"法罗",            "Faroese"    },
    {"俄语",            "Russian"    },
    {"迪维希语",        "Dhivehi"    },
    {"德语",            "German"     },
    {"丹麦语",          "Danish"     },
    {"茨瓦纳语",        "Tswana"     },
    {"朝鲜语",          "Korean"     },
    {"不丹语",          "Dzongkha"   },
    {"波斯语",          "Persian"    },
    {"波斯尼亚语",      "Bosnian"    },
    {"波兰语",          "Polish"     },
    {"冰岛",            "Icelandic"  },
    {"比利时语",        "Belgian"    },
    {"保加利亚",        "Bulgaria"   },
    {"班巴拉语",        "Bambara"    },
    {"柏柏尔语",        "Berber"     },
    {"白俄罗斯语",      "Belarusian" },
    {"爱沙尼亚",        "Estonia"    },
    {"爱尔兰",          "Irish"      },
    {"阿塞拜疆语",      "Azerbaijani"},
    {"阿姆哈拉语",      "Amharic"    },
    {"阿拉伯语",        "Arabic"     },
    {"阿富汗",          "Afghanistan"},
    {"阿尔巴尼亚",      "Albanian"   },
    {"EurKEY",          "EurKEY"     },
    {"APL",             "APL"        },
};

int initCategroyLanguageMap()
{
	int count = sizeof(languageList) / sizeof(s_languageList);
	int index = 0;

	g_chineseToEnglishMap.clear();
	g_englishToChineseMap.clear();
	for (index = 0; index < count; index++) {
		g_chineseToEnglishMap[languageList[index].chineseName] = languageList[index].englishName;
		g_englishToChineseMap[languageList[index].englishName] = languageList[index].chineseName;
	}

	return 0;
}

QString buildCategroyLanguageName(QString chineseLanguage)
{
	QString languageName = "";
	QString englishName = "";

	chineseLanguage = chineseLanguage.trimmed();
	osaLogInfo(LOG_EXPANDED_NAME, LOG_EXPANDED_NUM, "====>\n");
	if (g_chineseToEnglishMap.contains(chineseLanguage)) {
		englishName = g_chineseToEnglishMap[chineseLanguage];

		languageName = chineseLanguage + " - " + englishName;
	}
	else if (g_englishToChineseMap.contains(chineseLanguage)) {
		englishName = chineseLanguage;
		chineseLanguage = g_englishToChineseMap[englishName];

		languageName = chineseLanguage + " - " + englishName;
	}
	else {
		languageName = chineseLanguage;
	}

	QString c_py = Dtk::Core::Chinese2Pinyin(chineseLanguage);
	osaLogInfo(LOG_EXPANDED_NAME, LOG_EXPANDED_NUM,
		"<==== languageName [%s], c_py.left(1) [%s], c_py [%s] <- chineseLanguage [%s]\n",
		languageName.toStdString().c_str(), c_py.left(1).toStdString().c_str(),
		c_py.toStdString().c_str(), chineseLanguage.toStdString().c_str());
	return languageName;
}

QString getEnglishLanguageName(QString chineseLanguage)
{
	QString englishName = "";

	chineseLanguage = chineseLanguage.trimmed();
	if (g_chineseToEnglishMap.contains(chineseLanguage)) {
		englishName = g_chineseToEnglishMap[chineseLanguage];
	}
	else if (g_englishToChineseMap.contains(chineseLanguage)) {
		englishName = chineseLanguage;
	}
	else {
		englishName = "";
	}
	return englishName;
}

void setUseIMList(const FcitxQtStringKeyValueList& useIMs)
{
	g_useIMList = useIMs;
}

FcitxQtStringKeyValueList& getUseIMList()
{
	return g_useIMList;
}

void setMaxUseIMLanguageIndex(int index)
{
	if (index > g_maxLanguageIndex) {
		g_maxLanguageIndex = index;
	}
}

int getMaxUseIMLanguageIndex()
{
	return g_maxLanguageIndex;
}
