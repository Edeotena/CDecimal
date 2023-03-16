#include "../s21_decimal.h"

int s21_add(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
  null_decimal(result);
  unsigned int scale;
  if (scale_decimals(&value_1, &value_2, &scale) != OK) {
    return TOO_LARGE;
  }

  set_exponent(result, scale);

  if (get_sign(&value_1) == get_sign(&value_2)) {
    return add_same_signs(&value_1, &value_2, result);
  }

  return sub_diff_signs(value_1, value_2, result);
}

int s21_sub(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
  change_sign(&value_2);

  return s21_add(value_1, value_2, result);
}

int s21_mul(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
  return OK;
}

int s21_div(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
  return OK;
}

int s21_mod(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
  return OK;
}
