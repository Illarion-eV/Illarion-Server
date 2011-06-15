#include "CLanguage.hpp"

CLanguage::CLanguage(CLanguage::LanguageType language)  : _language(language) {
}

CLanguage* CLanguage::create(LanguageType language) {
	return new CLanguage(language);
}
