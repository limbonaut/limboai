/* bb_node.h */

#ifndef BB_NODE_H
#define BB_NODE_H

#include "bb_param.h"
#include "core/object.h"

class BBNode : public BBParam {
	GDCLASS(BBNode, BBParam);

protected:
	virtual Variant::Type get_type() const { return Variant::NODE_PATH; }

public:
	virtual Variant get_value(Object *p_agent, const Ref<Blackboard> &p_blackboard, const Variant &p_default = Variant());
};

#endif // BB_NODE_H