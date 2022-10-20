/* bb_dictionary.h */

#ifndef BB_DICTIONARY_H
#define BB_DICTIONARY_H

#include "bb_param.h"
#include "core/object.h"

class BBDictionary : public BBParam {
	GDCLASS(BBDictionary, BBParam);

protected:
	virtual Variant::Type get_type() const { return Variant::DICTIONARY; }
};

#endif // BB_DICTIONARY_H