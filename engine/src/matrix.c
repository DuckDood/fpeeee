#include <math.h>
#include <matrix.h>

mat3 transpose(mat3 matrix) {
	mat3 transposed = matrix;
	float temp_number;

	temp_number = transposed.matrix[3];
	transposed.matrix[3] = transposed.matrix[1];
	transposed.matrix[1] = temp_number;

	temp_number = transposed.matrix[2];
	transposed.matrix[2] = transposed.matrix[6];
	transposed.matrix[6] = temp_number;

	temp_number = transposed.matrix[5];
	transposed.matrix[5] = transposed.matrix[7];
	transposed.matrix[7] = temp_number;

	return transposed;
}

mat3 m3_mult(mat3 a, mat3 b) {
	mat3 multed_matrix;
	int mat_index = 0;
	for(int column = 0; column < 3; ++column) {
		for(int row = 0; row < 3; ++row) {
			vec3 row_vector = (vec3){a.matrix[row + 0], a.matrix[row + 3], a.matrix[row + 6]};
			vec3 column_vector = (vec3){b.matrix[column * 3 + 0], b.matrix[column * 3 + 1], b.matrix[column * 3 + 2]};
			multed_matrix.matrix[mat_index++] = v3_dot(row_vector, column_vector);
		}
	}
	return multed_matrix;
}

vec3 m3_v3_mult(mat3 a, vec3 b) {
	vec3 multed_vector;
	vec3 row_vector;
	row_vector = (vec3){a.matrix[0 + 0], a.matrix[0 + 3], a.matrix[0 + 6]};
	multed_vector.x = v3_dot(row_vector, b);
	row_vector = (vec3){a.matrix[1 + 0], a.matrix[1 + 3], a.matrix[1 + 6]};
	multed_vector.y = v3_dot(row_vector, b);
	row_vector = (vec3){a.matrix[2 + 0], a.matrix[2 + 3], a.matrix[2 + 6]};
	multed_vector.z = v3_dot(row_vector, b);

	return multed_vector;
}

mat3 generate_rotation_matrix(float yaw, float pitch, float roll) {
	return (mat3){{
		cos(roll) * cos(yaw),
		sin(roll) * cos(yaw),
		-sin(yaw),

		cos(roll) * sin(yaw) * sin(pitch) - sin(roll) * cos(pitch),
		sin(roll) * sin(yaw) * sin(pitch) + cos(roll) * cos(pitch),
		cos(yaw) * sin(pitch),

		cos(roll) * sin(yaw) * cos(pitch) + sin(roll) * sin(pitch),
		sin(roll) * sin(yaw) * cos(pitch) - cos(roll) * sin(pitch),
		cos(yaw) * cos(pitch)
	}};
}

mat3 generate_scale_matrix(vec3 scale) {
	return (mat3){{
		scale.x,
		0,
		0,
		
		0,
		scale.y,
		0,

		0,
		0,
		scale.z
	}};
}
