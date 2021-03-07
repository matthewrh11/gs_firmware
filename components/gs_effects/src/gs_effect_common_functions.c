#include "gs_effect_common_functions.h"

int32_t max(int32_t *x, size_t size){
	int32_t max_val = 0;
	for (size_t i = 0; i < size; i++){
		if (abs(x[i]) > max_val){
			max_val = abs(x[i]);
		}
	}
	return max_val;
}

int sign(int x){
	if (x > 0){
		return 1;
	}
	else if (x < 0){
		return -1;
	}
	else {
		return 0;
	}
}

int32_t abs(int32_t x){
	if (x < 0){
		x *= -1;
	}
	return x;
}
