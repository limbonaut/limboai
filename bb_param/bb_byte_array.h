/* bb_byte_array.h */

#ifndef BB_BYTE_ARRAY_H
#define BB_BYTE_ARRAY_H

#include "bb_param.h"
#include "core/object.h"

class BBByteArray : public BBParam {
	GDCLASS(BBByteArray, BBParam);

protected:
	virtual Variant::Type get_type() const { return Variant::POOL_BYTE_ARRAY; }
};

#endif // BB_BYTE_ARRAY_H