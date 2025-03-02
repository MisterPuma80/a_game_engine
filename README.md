# Just a Game Engine

<p align="center">
  <a href="https://github.com/MisterPuma80/a_game_engine">
    <img src="logo_outlined.svg" width="400" alt="Just a Game Engine">
  </a>
</p>

# Based on Godot 4.3 but with changes
* [x] Expose Engine.get_frame_ticks to GDScript
* [x] Change look_at to have default up of Vector3.INF and find appropriate axis itself if Vector3.INF is argument.
* [x] Optimize Node.find_children to be faster by using a vector as a stack instead of recursion, and caching values
* [X] Optimize Node.get_children by removing redundant cache update checks
* [X] Optimize Node.get_groups by making it return a LocalVector instead of a List
* [X] Optimize SceneTree.get_nodes_in_group by making it return a LocalVector instead of a List
* [X] Add Array::resize_uninitialized
* [X] Optimize InputMap::get_actions
* [X] Optimize Node::reparent
* [X] Optimize Area2D / Area3D::get_overlapping_bodies
* [X] Optimize Area2D / Area3D::get_overlapping_areas
* [X] Optimize RigidBody2D / RigidBody3D::get_colliding_bodies()
* [X] Optimize SoftBody3D / PhysicsBody2D / PhysicsBody3D::get_collision_exceptions()

# Get code

```sh
git clone https://github.com/MisterPuma80/a_game_engine
```


# Build

```sh
scons platform=linuxbsd target=editor dev_build=no dev_mode=no use_llvm=yes linker=mold tests=yes execinfo=yes scu_build=yes -j 25
```

# Build export templates

```sh
scons platform=linuxbsd target=template_release dev_build=no dev_mode=no use_llvm=yes linker=mold scu_build=yes -j 25
```


# Clean

```sh
scons -c
git clean -fixd
```
