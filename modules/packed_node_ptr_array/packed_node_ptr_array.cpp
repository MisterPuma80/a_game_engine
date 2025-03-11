
#include "packed_node_ptr_array.h"

//#include "godot_cpp/classes/global_constants.hpp"
#include "scene/main/node.h"

//#include "modules/stacktrace/stacktrace.h"

PackedNodePtrArray::PackedNodePtrArray() {
	//print_line("PackedNodePtrArray created");
}

PackedNodePtrArray::~PackedNodePtrArray() {
	//print_line("PackedNodePtrArray destroyed");
	items.clear();
}
/*
void PackedNodePtrArray::destroy2(PackedNodePtrArray* inst) {
	if (inst != nullptr) {
		memfree(inst);
		inst = nullptr;
	}
}

PackedNodePtrArray* PackedNodePtrArray::create2() {
	//PackedNodePtrArray* nodes = memnew(PackedNodePtrArray);
	return memnew(PackedNodePtrArray);
}
*/
LocalVector<Node *> *PackedNodePtrArray::get_node_ptr() {
	return &items;
}

void PackedNodePtrArray::add_node(Node *item) {
	//	fprintf(stderr, "!!!! called PackedNodePtrArray::add_node\n"); fflush(stderr);
	items.push_back(item);
}

Node *PackedNodePtrArray::get_node(int index) const {
	//	fprintf(stderr, "!!!! called PackedNodePtrArray::get\n"); fflush(stderr);
	if (index >= 0 && index < (int)items.size()) {
		return items[index];
	}
	return nullptr;
}

void PackedNodePtrArray::set(int index, Node *item) {
	items[index] = item;
}

int PackedNodePtrArray::size() const {
	return (int)items.size();
}

void PackedNodePtrArray::resize(int new_size) {
	items.resize(new_size);
}

void PackedNodePtrArray::clear() {
	items.resize(0);
}

Node *PackedNodePtrArray::front() const {
	if (items.size() == 0) return nullptr;
	return items[0];
}

Node *PackedNodePtrArray::back() const {
	if (items.size() == 0) return nullptr;
	return items[items.size() - 1];
}

Node *PackedNodePtrArray::pick_random() const {
	//ERR_FAIL_COND_V_MSG(_p->array.is_empty(), nullptr, "Can't take value from empty PackedNodePtrArray.");
	int i = Math::rand() % items.size();
	return items[i];
}

TypedArray<Node> PackedNodePtrArray::to_array() const {
	TypedArray<Node> retval;
	int ic = items.size();
	retval.resize_uninitialized(ic);
	for (int i = 0; i < ic; i++) {
		retval.set(i, items[i]);
	}
	return retval;
}

bool PackedNodePtrArray::is_empty() const {
	return items.size() > 0;
}

bool PackedNodePtrArray::_iter_init(const Variant &args) {
//	fprintf(stderr, "!!!! called PackedNodePtrArray::_iter_init\n"); fflush(stderr);
	current_index = 0;
	return items.size() > 0;
}

bool PackedNodePtrArray::_iter_next(const Variant &args) {
//	fprintf(stderr, "!!!! called PackedNodePtrArray::_iter_next: %d\n", current_index); fflush(stderr);
	current_index++;
	return current_index < items.size();
}


Node *PackedNodePtrArray::_iter_get(const Variant &arg) {
	uint32_t idx = current_index;
//	fprintf(stderr, "!!!! called PackedNodePtrArray::_iter_get: %d\n", idx); fflush(stderr);
	if (idx >= 0 && idx < items.size()) {
		return items[idx];
	}
	return nullptr;
}

void PackedNodePtrArray::_bind_methods() {
	//ClassDB::bind_static_method("PackedNodePtrArray", D_METHOD("destroy2", "inst"), &PackedNodePtrArray::destroy2);
	//ClassDB::bind_static_method("PackedNodePtrArray", D_METHOD("create2"), &PackedNodePtrArray::create2);

	ClassDB::bind_method(D_METHOD("add_node", "item"), &PackedNodePtrArray::add_node);
	ClassDB::bind_method(D_METHOD("get_node", "index"), &PackedNodePtrArray::get_node);
	ClassDB::bind_method(D_METHOD("size"), &PackedNodePtrArray::size);
	ClassDB::bind_method(D_METHOD("clear"), &PackedNodePtrArray::clear);

	ClassDB::bind_method(D_METHOD("front"), &PackedNodePtrArray::front);
	ClassDB::bind_method(D_METHOD("back"), &PackedNodePtrArray::back);
	ClassDB::bind_method(D_METHOD("pick_random"), &PackedNodePtrArray::pick_random);
	ClassDB::bind_method(D_METHOD("to_array"), &PackedNodePtrArray::to_array);
	ClassDB::bind_method(D_METHOD("is_empty"), &PackedNodePtrArray::is_empty);

	ClassDB::bind_method(D_METHOD("_iter_init", "args"), &PackedNodePtrArray::_iter_init);
	ClassDB::bind_method(D_METHOD("_iter_next", "args"), &PackedNodePtrArray::_iter_next);
	ClassDB::bind_method(D_METHOD("_iter_get", "arg"), &PackedNodePtrArray::_iter_get);
}
