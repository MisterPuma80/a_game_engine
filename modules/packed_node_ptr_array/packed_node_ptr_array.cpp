
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
	//current_size = 0;
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
Vector<Node*>* PackedNodePtrArray::get_node_ptr() {
	return &items;
}

void PackedNodePtrArray::add_node(Node *item) {
//	fprintf(stderr, "!!!! called PackedNodePtrArray::add_node\n"); fflush(stderr);
    items.append(item);
	//current_size = items.size();
}

Node* PackedNodePtrArray::get_node(int index) const {
    //	fprintf(stderr, "!!!! called PackedNodePtrArray::get\n"); fflush(stderr);
        if (index >= 0 && index < items.size()) {
            return items[index];
        }
        return nullptr;
    }

void PackedNodePtrArray::set(int index, Node *item) {
	items.set(index, item);
}

int PackedNodePtrArray::size() const {
    return items.size();
}

void PackedNodePtrArray::resize(int new_size) {
	items.resize(new_size);
	//current_size = new_size;
}

void PackedNodePtrArray::clear() {
    items.resize(0);
}


/*
bool PackedNodePtrArray::_iter_init(const Variant &args) {
//	fprintf(stderr, "!!!! called PackedNodePtrArray::_iter_init\n"); fflush(stderr);
	current_index = 0;
    return current_size > 0;
}

bool PackedNodePtrArray::_iter_next(const Variant &args) {
//	fprintf(stderr, "!!!! called PackedNodePtrArray::_iter_next: %d\n", current_index); fflush(stderr);
	current_index++;
	return current_index < current_size;
}


Node* PackedNodePtrArray::_iter_get(const Variant &arg) {
	int idx = current_index;
//	fprintf(stderr, "!!!! called PackedNodePtrArray::_iter_get: %d\n", idx); fflush(stderr);
    if (idx >= 0 && idx < current_size) {
        return items[idx];
    }
    return nullptr;
}
*/
void PackedNodePtrArray::_bind_methods() {
    //ClassDB::bind_static_method("PackedNodePtrArray", D_METHOD("destroy2", "inst"), &PackedNodePtrArray::destroy2);
    //ClassDB::bind_static_method("PackedNodePtrArray", D_METHOD("create2"), &PackedNodePtrArray::create2);

    ClassDB::bind_method(D_METHOD("add_node", "item"), &PackedNodePtrArray::add_node);
    ClassDB::bind_method(D_METHOD("get_node", "index"), &PackedNodePtrArray::get_node);
    ClassDB::bind_method(D_METHOD("size"), &PackedNodePtrArray::size);
    ClassDB::bind_method(D_METHOD("clear"), &PackedNodePtrArray::clear);

    //ClassDB::bind_method(D_METHOD("_iter_init", "args"), &PackedNodePtrArray::_iter_init);
    //ClassDB::bind_method(D_METHOD("_iter_next", "args"), &PackedNodePtrArray::_iter_next);
    //ClassDB::bind_method(D_METHOD("_iter_get", "arg"), &PackedNodePtrArray::_iter_get);
}