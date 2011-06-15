#ifndef CLANGUAGE_HPP
#define CLANGUAGE_HPP

class CLanguage {

	public:
		enum LanguageType {  
			german = 0,
			english = 1,
			french = 2
		};

	static CLanguage* create(LanguageType);

	LanguageType _language;

	protected:
		CLanguage(LanguageType);
};

#endif
