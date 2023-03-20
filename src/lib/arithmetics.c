#include <stdlib.h>

#include "../s21_decimal.h"

int s21_add(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
  if (!result) {
    return ERROR;
  }

  big_decimal val1 = convert(value_1), val2 = convert(value_2);
  big_decimal res = {0};

  unsigned int scale;
  s21_decimal overflow = {0};
  int ret = scale_decimals(&value_1, &value_2, &scale, &overflow);

  set_exponent(result, scale);
  set_sign(result, get_sign(&value_1));

  if (get_sign(&value_1) == get_sign(&value_2)) {
    int additional_bit = add_same_signs(value_1, value_2, result);
    add_int_to_dec(overflow, additional_bit, &overflow);
    return try_add_overflow(result, overflow);
  }

  if (make_first_bigger_no_signs(&value_1, &value_2) == TRUE) {
    change_sign(result);
  }

  int additional_bit = sub_diff_signs(value_1, value_2, result);
  sub_int_fr_dec(overflow, additional_bit, &overflow);
  return try_add_overflow(result, overflow);
}

int s21_sub(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
  change_decimal_sign(&value_2);

  return s21_add(value_1, value_2, result);
}

int s21_mul(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
  if (!result) {
    return ERROR;
  }

  null_decimal(result);

  int ret = mul_without_signs(value_1, value_2, result);
  if (ret != OK) {
    return ret;
  }

  set_exponent(result, get_exponent(&value_1) + get_exponent(&value_2));

  int first_sign = get_sign(&value_1), second_sign = get_sign(&value_2);
  if (first_sign != second_sign) {
    set_sign(&res, NEGATIVE);
  } else {
    set_sign(&res, POSITIVE);
  }

  reduce_exponent(&res);

  return rconvert(res, result);
}

int s21_div(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
  return OK;
}

int s21_mod(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
  return OK;
}
