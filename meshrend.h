#ifndef MESHREND_H
#define MESHREND_H

void mesh_render(const struct mesh *mesh);
void mesh_compile_list(const struct mesh *mesh, GLuint list);
void mesh_calc_bounds(const struct mesh *mesh, float *min, float *max);

#endif
