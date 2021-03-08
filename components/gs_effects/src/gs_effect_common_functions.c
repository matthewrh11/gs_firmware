#include "gs_effect_common_functions.h"

int16_t max_left(int16_t *x, size_t size){
	int16_t max_val = abs_val(x[0]);
	for (size_t i = 0; i < size; i += 2){
		if (abs_val(x[i]) > max_val){
			max_val = abs_val(x[i]);
		}
	}
	return max_val;
}

int16_t max_right(int16_t *x, size_t size){
	int16_t max_val = abs_val(x[1]);
	for (size_t i = 1; i < size; i += 2){
		if (abs_val(x[i]) > max_val){
			max_val = abs_val(x[i]);
		}
	}
	return max_val;
}


int16_t max(int16_t *x, size_t size){
	int16_t max_val = abs_val(x[0]);
	for (size_t i = 0; i < size; i += 1){
		if (abs_val(x[i]) > max_val){
			max_val = abs_val(x[i]);
		}
	}
	return max_val;
}

float sign(int x){
	if (x > 0){
		return 1.0;
	}
	else if (x < 0){
		return -1.0;
	}
	else {
		return 0.0;
	}
}

int16_t abs_val(int16_t x){
	if (x < 0){
		x *= -1;
	}
	return x;
}
