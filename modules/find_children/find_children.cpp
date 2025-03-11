
#include "find_children.h"

#include "core/object/script_language.h"

//#include "godot_cpp/classes/global_constants.hpp"
#include "scene/main/node.h"
//#include "modules/packed_node_ptr_array/packed_node_ptr_array.h"

//#include "modules/stacktrace/stacktrace.h"

Find::Find() {
	//print_line("Find created");
}

Find::~Find() {
	//print_line("Find destroyed");
}

Ref<PackedNodePtrArray> Find::children(const Node *p_node, const bool p_include_internal) {
	//ERR_THREAD_GUARD_V(nullptr); // FIXME

	int cc;
	Node *const *cptr = p_node->_get_children_ptr(&cc, p_include_internal);

	Ref<PackedNodePtrArray> nodes = memnew(PackedNodePtrArray);
	LocalVector<Node *> *ptr = nodes->get_node_ptr();
	ptr->resize(cc);
	//for (int i = 0; i < cc; i++) {
	//	(*ptr)[i] = cptr[i];
	//}
	memcpy(ptr->ptr(), cptr, cc * sizeof(Node *));

	return nodes;
}

Ref<PackedNodePtrArray> Find::all(const Node *p_node) {
	return Find::by(p_node, "*", "", true, false);
}

Ref<PackedNodePtrArray> Find::by_name(const Node *p_node, const String &p_node_name) {
	return Find::by(p_node, p_node_name, "", true, false);
}

Ref<PackedNodePtrArray> Find::by_type(const Node *p_node, const String &p_type_name) {
	return Find::by(p_node, "*", p_type_name, true, false);
}

Ref<PackedNodePtrArray> Find::by_group(const Node *p_node, const String &p_group_name) {
	//ERR_THREAD_GUARD_V(TypedArray<Node>()); // FIXME

	LocalVector<Node *> to_search;
	Ref<PackedNodePtrArray> matches = memnew(PackedNodePtrArray);
	to_search.push_back((Node *)p_node);
	while (!to_search.is_empty()) {
		Node *entry = to_search[0];
		to_search.remove_at_unordered(0);

		entry->_update_children_cache();
		Node *const *cptr = entry->data.children_cache.ptr();
		const int ccount = entry->data.children_cache.size();
		for (int i = 0; i < ccount; i++) {
			to_search.push_back(cptr[i]);
		}

		if (entry->is_in_group(p_group_name)) {
			matches->add_node(entry);
		}
	}

	return matches;
}

Ref<PackedNodePtrArray> Find::by_groups(const Node *p_node, const TypedArray<String> &p_group_names) {
	//ERR_THREAD_GUARD_V(TypedArray<Node>()); // FIXME

	Ref<PackedNodePtrArray> matches = memnew(PackedNodePtrArray);
	LocalVector<Node *> to_search;
	to_search.push_back((Node *)p_node);
	while (!to_search.is_empty()) {
		Node *entry = to_search[0];
		to_search.remove_at_unordered(0);

		entry->_update_children_cache();
		Node *const *cptr = entry->data.children_cache.ptr();
		const int ccount = entry->data.children_cache.size();
		for (int i = 0; i < ccount; i++) {
			to_search.push_back(cptr[i]);
		}

		const int ncount = p_group_names.size();
		for (int i = 0; i < ncount; i++) {
			if (entry->is_in_group(p_group_names[i])) {
				matches->add_node(entry);
			}
		}
	}

	return matches;
}

Ref<PackedNodePtrArray> Find::by(const Node *p_node, const String &p_pattern, const String &p_type, const bool p_recursive, const bool p_owned) {
	//ERR_THREAD_GUARD; // FIXME
	//ERR_FAIL_COND(p_pattern.is_empty() && p_type.is_empty());

	// Save basic pattern and type info for faster lookup
	bool is_pattern_empty = p_pattern.is_empty();
	bool is_type_empty = p_type.is_empty();
	bool is_type_global_class = !is_type_empty && ScriptServer::is_global_class(p_type);
	String type_global_path = is_type_global_class ? ScriptServer::get_global_class_path(p_type) : "";

	LocalVector<Node *> to_search;
	Ref<PackedNodePtrArray> matches = memnew(PackedNodePtrArray);
	to_search.push_back((Node *)p_node);
	bool is_adding_children = true;
	while (!to_search.is_empty()) {
		// Pop the next entry off the search stack
		Node *entry = to_search[0];
		to_search.remove_at_unordered(0);

		// Add all the children to the list to search
		entry->_update_children_cache();
		if (is_adding_children) {
			Node *const *cptr = entry->data.children_cache.ptr();
			int ccount = entry->data.children_cache.size();
			for (int i = 0; i < ccount; i++) {
				if (p_owned && !cptr[i]->data.owner) {
					continue;
				}

				to_search.push_back(cptr[i]);
			}

			// Stop further child adding if we don't want recursive
			if (!p_recursive) {
				is_adding_children = false;
			}
		}

		// Check if the entry matches
		bool is_pattern_match = is_pattern_empty || entry->data.name.operator String().match(p_pattern);
		bool is_type_match = is_type_empty || entry->is_class(p_type);
		bool is_script_type_match = false;
		if (!is_type_match) {
			if (ScriptInstance *script_inst = entry->get_script_instance()) {
				Ref<Script> scr = script_inst->get_script();
				while (scr.is_valid()) {
					if ((is_type_global_class && type_global_path == scr->get_path()) || p_type == scr->get_path()) {
						is_script_type_match = true;
						break;
					}

					scr = scr->get_base_script();
				}
			}
		}

		// Save it if it matches the pattern and at least one type
		if (is_pattern_match && (is_type_match || is_script_type_match)) {
			matches->add_node(entry);
		}
	}

	return matches;
}

void Find::_bind_methods() {
	ClassDB::bind_static_method("Find", D_METHOD("children", "node", "include_internal"), &Find::children, DEFVAL(true));
	ClassDB::bind_static_method("Find", D_METHOD("all", "node"), &Find::all);
	ClassDB::bind_static_method("Find", D_METHOD("by", "node", "pattern", "type", "recursive", "owned"), &Find::by, DEFVAL(""), DEFVAL(true), DEFVAL(true));
	ClassDB::bind_static_method("Find", D_METHOD("by_name", "node", "node_name"), &Find::by_name);
	ClassDB::bind_static_method("Find", D_METHOD("by_type", "node", "type_name"), &Find::by_type);
	ClassDB::bind_static_method("Find", D_METHOD("by_group", "node", "group_name"), &Find::by_group);
	ClassDB::bind_static_method("Find", D_METHOD("by_groups", "node", "group_names"), &Find::by_groups);
}
