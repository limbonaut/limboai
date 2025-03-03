/**
 * variant.cpp
 * =============================================================================
 * Copyright (c) 2023-present Serhii Snitsaruk and the LimboAI contributors.
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * =============================================================================
 */
#include "variant.h"

#ifdef LIMBOAI_MODULE
#include "core/object/ref_counted.h"
#endif // LIMBOAI_MODULE

#ifdef LIMBOAI_GDEXTENSION
#include <godot_cpp/classes/ref_counted.hpp>
#endif // LIMBOAI_GDEXTENSION

void VARIANT_DELETE_IF_OBJECT(const Variant &p_variant) {
	if (p_variant.get_type() == Variant::OBJECT) {
		Ref<RefCounted> r = p_variant;
		if (r.is_null()) {
			memdelete((Object *)p_variant);
		}
	}
}

Variant VARIANT_DEFAULT(Variant::Type p_type) {
	switch (p_type) {
		case Variant::Type::NIL: {
			return Variant();
		} break;
		case Variant::Type::BOOL: {
			return Variant(false);
		} break;
		case Variant::Type::INT: {
			return Variant(0);
		} break;
		case Variant::Type::FLOAT: {
			return Variant(0.0);
		} break;
		case Variant::Type::STRING: {
			return Variant("");
		} break;
		case Variant::Type::VECTOR2: {
			return Variant(Vector2());
		} break;
		case Variant::Type::VECTOR2I: {
			return Variant(Vector2i());
		} break;
		case Variant::Type::RECT2: {
			return Variant(Rect2());
		} break;
		case Variant::Type::RECT2I: {
			return Variant(Rect2i());
		} break;
		case Variant::Type::VECTOR3: {
			return Variant(Vector3());
		} break;
		case Variant::Type::VECTOR3I: {
			return Variant(Vector3i());
		} break;
		case Variant::Type::TRANSFORM2D: {
			return Variant(Transform2D());
		} break;
		case Variant::Type::VECTOR4: {
			return Variant(Vector4());
		} break;
		case Variant::Type::VECTOR4I: {
			return Variant(Vector4i());
		} break;
		case Variant::Type::PLANE: {
			return Variant(Plane());
		} break;
		case Variant::Type::QUATERNION: {
			return Variant(Quaternion());
		} break;
		case Variant::Type::AABB: {
			return Variant(AABB());
		} break;
		case Variant::Type::BASIS: {
			return Variant(Basis());
		} break;
		case Variant::Type::TRANSFORM3D: {
			return Variant(Transform3D());
		} break;
		case Variant::Type::PROJECTION: {
			return Variant(Projection());
		} break;
		case Variant::Type::COLOR: {
			return Variant(Color());
		} break;
		case Variant::Type::STRING_NAME: {
			return Variant(StringName());
		} break;
		case Variant::Type::NODE_PATH: {
			return Variant(NodePath());
		} break;
		case Variant::Type::RID: {
			return Variant(RID());
		} break;
		case Variant::Type::OBJECT: {
			return Variant();
		} break;
		case Variant::Type::CALLABLE: {
			return Variant();
		} break;
		case Variant::Type::SIGNAL: {
			return Variant();
		} break;
		case Variant::Type::DICTIONARY: {
			return Variant(Dictionary());
		} break;
		case Variant::Type::ARRAY: {
			return Variant(Array());
		} break;
		case Variant::Type::PACKED_BYTE_ARRAY: {
			return Variant(PackedByteArray());
		} break;
		case Variant::Type::PACKED_INT32_ARRAY: {
			return Variant(PackedInt32Array());
		} break;
		case Variant::Type::PACKED_INT64_ARRAY: {
			return Variant(PackedInt64Array());
		} break;
		case Variant::Type::PACKED_FLOAT32_ARRAY: {
			return Variant(PackedFloat32Array());
		} break;
		case Variant::Type::PACKED_FLOAT64_ARRAY: {
			return Variant(PackedFloat64Array());
		} break;
		case Variant::Type::PACKED_STRING_ARRAY: {
			return Variant(PackedStringArray());
		} break;
		case Variant::Type::PACKED_VECTOR2_ARRAY: {
			return Variant(PackedVector2Array());
		} break;
		case Variant::Type::PACKED_VECTOR3_ARRAY: {
			return Variant(PackedVector3Array());
		} break;
		case Variant::Type::PACKED_COLOR_ARRAY: {
			return Variant(PackedColorArray());
		} break;
		default: {
			return Variant();
		}
	}
}
