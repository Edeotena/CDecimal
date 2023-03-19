#include <math.h>
#include <stdio.h>

#include "../s21_decimal.h"

int get_bit(unsigned int val, int index) {
  return (val & ((unsigned int)1 << index)) ? 1 : 0;
}

int get_decimal_bit(const big_decimal* val, int index) {
  return (get_bit(val->bits[index / BITS_IN_INT], index % BITS_IN_INT));
}

void set_bit(unsigned int* val, int index, int bit) {
  if (bit == 1) {
    (*val) |= 1 << index;
  } else {
    (*val) &= ~(1 << index);
  }
}

void set_decimal_bit(big_decimal* val, int index, int bit) {
  set_bit(&val->bits[index / BITS_IN_INT], index % BITS_IN_INT, bit);
}

int get_higher_bit(unsigned int val) {
  int res = -1;
  for (int i = 0; val != 0; ++i) {
    if ((val & 1) == 1) {
      res = i;
    }
    val = val >> 1;
  }
  return res;
}

int decimal_size(big_decimal val) {
  for (int i = BIG_TOP_BIT; i >= BIG_BOT_BIT; --i) {
    if (val.bits[i] != 0) {
      return get_higher_bit(val.bits[i]) + BITS_IN_INT * i;
    }
  }
  return 0;
}

int get_sign(const big_decimal* val) {
  return get_bit(val->bits[BIG_SPEC_BIT], SIGN_BIT) ? NEGATIVE : POSITIVE;
}

void set_sign(big_decimal* val, int sign) {
  set_bit(&val->bits[BIG_SPEC_BIT], SIGN_BIT, sign);
}

void change_sign(big_decimal* val) {
  int sign = NEGATIVE;
  if (get_sign(val) == NEGATIVE) {
    sign = POSITIVE;
  }
  set_sign(val, sign);
}

unsigned int get_exponent(const big_decimal* val) {
  return (val->bits[BIG_SPEC_BIT] & EXPONENT_MASK) >> 16;
}

unsigned int get_decimal_exponent(const s21_decimal* val) {
  return (val->bits[SPEC_BIT] & EXPONENT_MASK) >> 16;
}

void set_exponent(big_decimal* val, unsigned int exp) {
  for (int i = 0; i < 8; ++i) {
    set_bit(&val->bits[BIG_SPEC_BIT], 16 + i, (int)(exp % 2));
    exp /= 2;
  }
}

int is_zero(const big_decimal* val) {
  int res = FALSE;
  if (val->bits[0] == 0 && val->bits[1] == 0 && val->bits[2] == 0) {
    res = TRUE;
  }
  return res;
}

int pure_mul(big_decimal val, int num, big_decimal* res) {
  null_big_decimal(res);
  set_sign(res, get_sign(&val));
  set_exponent(res, get_exponent(&val));
  if (num < 0) {
    num = -num;
    change_sign(res);
  }

  int ret = OK;
  while (num != 0) {
    if (num % 2 == 1) {
      ret = pure_add(res, &val, res);
      if (ret != OK) {
        return ret;
      }
    }

    num /= 2;
    if (num != 0) {
      ret = left_shift(&val);
      if (ret != OK) {
        return ret;
      }
    }
  }

  return ret;
}

int scal_mul(big_decimal val, int num, big_decimal* res) {
  null_big_decimal(res);
  set_sign(res, get_sign(&val));
  set_exponent(res, get_exponent(&val));
  if (num < 0) {
    num = -num;
    change_sign(res);
  }

  int ret = OK;
  while (num != 0) {
    if (num % 2 == 1) {
      ret = add_same_signs(*res, val, res);
      if (ret != OK) {
        return ret;
      }
    }

    num /= 2;
    if (num != 0) {
      ret = left_shift(&val);
      if (ret != OK) {
        return ret;
      }
    }
  }

  return ret;
}

int mul_without_signs(big_decimal val1, big_decimal val2, big_decimal* res) {
  null_big_decimal(res);
  made_first_bigger_no_signs(&val1, &val2);

  int ret = OK;
  while (is_zero(&val2) == FALSE) {
    int mod = 0;
    right_shift(&val2, &mod);
    if (mod == 1) {
      ret = add_same_signs(*res, val1, res);
      if (ret != OK) {
        return ret;
      }
    }

    if (is_zero(&val2) == FALSE) {
      ret = left_shift(&val1);
      if (ret != OK) {
        return ret;
      }
    }
  }

  return ret;
}

int scal_div(big_decimal val, int num, big_decimal* res, big_decimal* mod) {
  if (num == 0) {
    return ZERO_DIVISION;
  }

  null_big_decimal(res);
  set_sign(res, get_sign(&val));
  set_exponent(res, get_exponent(&val));
  if (num < 0) {
    num = -num;
    change_sign(res);
  }

  int ret = OK;

  null_big_decimal(mod);
  for (int i = decimal_size(val); i >= 0; --i) {
    ret = left_shift(res);
    if (ret != OK) {
      return ret;
    }
    ret = left_shift(mod);
    if (ret != OK) {
      return ret;
    }
    set_decimal_bit(mod, 0, get_decimal_bit(&val, i));
    if (mod->bits[BIG_BOT_BIT] >= num) {
      mod->bits[BIG_BOT_BIT] -= num;
      set_decimal_bit(res, 0, 1);
    }
  }

  return ret;
}

int pure_add(const big_decimal* value_1, const big_decimal* value_2,
             big_decimal* result) {
  int ret = OK;
  set_sign(result, get_sign(value_1));
  unsigned long long int overflow = 0;
  for (int i = BIG_BOT_BIT; i <= BIG_TOP_BIT; ++i) {
    unsigned long long bit_val =
        (unsigned long long)value_1->bits[i] + value_2->bits[i] + overflow;
    result->bits[i] = bit_val % OVERFLOW_BIT;
    overflow = bit_val / OVERFLOW_BIT;
  }
  if (overflow != 0) {
    ret = TOO_LARGE;
  }
  return ret;
}

int add_same_signs(big_decimal value_1, big_decimal value_2,
                   big_decimal* result) {
  int ret = OK;
  set_sign(result, get_sign(&value_1));
  unsigned long long int overflow = 0;
  for (int i = BIG_BOT_BIT; i <= BIG_TOP_BIT; ++i) {
    unsigned long long bit_val =
        (unsigned long long)value_1.bits[i] + value_2.bits[i] + overflow;
    result->bits[i] = bit_val % OVERFLOW_BIT;
    overflow = bit_val / OVERFLOW_BIT;
  }
  if (overflow != 0) {
    ret = TOO_LARGE;
  }
  reduce_exponent(result);
  return ret;
}

big_decimal decimal_abs(big_decimal val) {
  big_decimal res = val;
  set_sign(&res, POSITIVE);
  return res;
}

int sub_diff_signs(big_decimal value_1, big_decimal value_2,
                   big_decimal* result) {
  int ret = OK;
  set_sign(result, get_sign(&value_1));
  if (made_first_bigger_no_signs(&value_1, &value_2) == TRUE) {
    change_sign(result);
  }
  unsigned long long int overflow = 0;
  unsigned long long int bit_val;
  for (int i = BIG_BOT_BIT; i <= BIG_TOP_BIT; ++i) {
    if (value_1.bits[i] >= value_2.bits[i] + overflow) {
      bit_val = value_1.bits[i] - value_2.bits[i] - overflow;
      overflow = 0;
    } else {
      bit_val = OVERFLOW_BIT - value_2.bits[i] + value_1.bits[i] - overflow;
      overflow = 1;
    }
    result->bits[i] = bit_val;
  }
  if (overflow != 0) {
    ret = TOO_LARGE;
  }
  reduce_exponent(result);
  return ret;
}

int scale_decimals(big_decimal* num1, big_decimal* num2, unsigned int* exp) {
  int ret = OK;
  reduce_exponent(num1);
  reduce_exponent(num2);

  unsigned int exp1 = get_exponent(num1), exp2 = get_exponent(num2);

  if (exp1 > exp2) {
    *exp = exp1;
    set_exponent(num2, *exp);
    for (unsigned int i = exp2; i < exp1 && ret == OK; ++i) {
      ret = pure_mul(*num2, 10, num2);
    }
  } else {
    *exp = exp2;
    set_exponent(num1, *exp);
    for (unsigned int i = exp1; i < exp2 && ret == OK; ++i) {
      ret = pure_mul(*num1, 10, num1);
    }
  }

  return ret;
}

void null_decimal(s21_decimal* val) {
  for (int i = BOT_BIT; i <= SPEC_BIT; ++i) {
    val->bits[i] = 0;
  }
}

void null_big_decimal(big_decimal* val) {
  for (int i = BIG_BOT_BIT; i <= BIG_SPEC_BIT; ++i) {
    val->bits[i] = 0;
  }
}

void swap_decimals(big_decimal* val1, big_decimal* val2) {
  big_decimal cp = *val2;
  *val2 = *val1;
  *val1 = cp;
}

void reduce_exponent(big_decimal* val) {
  unsigned int exp = get_exponent(val);
  big_decimal reduced = *val, mod = DEFAULT_DECIMAL;

  while (exp > 0 && (is_zero(&mod) == TRUE || exp > 28) &&
         is_zero(&reduced) == FALSE) {
    int ret = scal_div(reduced, 10, &reduced, &mod);
    if (ret != OK) {
      return;
    }
    if (is_zero(&mod) == TRUE || exp > 28) {
      *val = reduced;
      --exp;
    }
  }

  if (is_zero(val) == TRUE) {
    set_sign(val, POSITIVE);
    set_exponent(val, 0);
    return;
  }

  set_exponent(val, exp);
}

s21_decimal create_decimal(unsigned int bit0, unsigned int bit1,
                           unsigned int bit2, unsigned int bit3) {
  s21_decimal res = {{bit0, bit1, bit2, bit3}};
  return res;
}

big_decimal create_big_decimal(unsigned int bit0, unsigned int bit1,
                               unsigned int bit2, unsigned int bit3,
                               unsigned int bit4, unsigned int bit5) {
  big_decimal res = {{bit0, bit1, bit2, bit3, bit4, bit5}};
  return res;
}

big_decimal convert(s21_decimal val) {
  big_decimal res = {0};
  res.bits[BIG_SPEC_BIT] = val.bits[SPEC_BIT];
  for (int i = BOT_BIT; i < TOP_BIT; ++i) {
    res.bits[i] = val.bits[i];
  }
  return res;
}

int convertable(big_decimal* val) { return OK; }

int rconvert(big_decimal val, s21_decimal* res) {
  int result = OK;
  if (convertable(&val) == OK) {
    res->bits[SPEC_BIT] = val.bits[BIG_SPEC_BIT];
    for (int i = BOT_BIT; i < TOP_BIT; ++i) {
      res->bits[i] = val.bits[i];
    }
  }
  return result;
}

int get_elder_bit_index(const s21_decimal* val) {
  if (val->bits[1] == 0) {
    return 0;
  }

  if (val->bits[2] == 0) {
    return 1;
  }

  return 2;
}

int get_first_integer_bit_index(const s21_decimal* val) {
  int exp = (int)get_decimal_exponent(val);
  if (exp < 10) {
    return 0;
  }

  if (exp < 20) {
    return 1;
  }

  return 2;
}

int left_shift(big_decimal* val) {
  int overflow = 0;

  for (int i = BIG_BOT_BIT; i <= BIG_TOP_BIT; ++i) {
    int next_overflow =
        get_decimal_bit(val, (BITS_IN_INT - 1) + i * BITS_IN_INT);
    val->bits[i] = (val->bits[i] << 1) + overflow;
    overflow = next_overflow;
  }

  if (overflow != 0) {
    return TOO_LARGE;
  }

  return OK;
}

int right_shift(big_decimal* val, int* mod) {
  int overflow = 0;

  for (int i = BIG_TOP_BIT; i >= BIG_BOT_BIT; --i) {
    int next_overflow = get_decimal_bit(val, i * BITS_IN_INT);
    val->bits[i] = (val->bits[i] >> 1) + (overflow * (OVERFLOW_BIT >> 1));
    overflow = next_overflow;
  }

  *mod = overflow;

  return OK;
}

void handle_decimal_inc(s21_decimal* val) {
  val->bits[0] += 1;
  if (val->bits[0] != 0) {
    return;
  }

  val->bits[1] += 1;
  if (val->bits[1] != 0) {
    return;
  }

  val->bits[2] += 1;
}

float remove_elder_digit(float val) {
  int digits = 0;
  int temp = (int)val;
  for (; temp > 0; temp /= 10, ++digits) {
  }

  temp = (int)val;
  int result = 0;
  for (int i = 0; i < digits - 1; ++i, temp /= 10) {
    result += temp % 10 * (int)pow(10.0, (double)i);
  }

  return val - (float)((int)val) + (float)result;
}

void print_decimal(const s21_decimal* val) {
  printf("\nsign = %s\nexp = %u\nbit[2] - %.8X\nbit[1] - %.8X\nbit[0] - %.8X\n",
         get_bit(val->bits[SPEC_BIT], SIGN_BIT) == POSITIVE ? "POSITIVE"
                                                            : "NEGATIVE",
         get_decimal_exponent(val), val->bits[2], val->bits[1], val->bits[0]);
}

void print_big_decimal(const big_decimal* val) {
  printf(
      "\nsign = %s\nexp = %u\nbit[4] - %.8X\nbit[3] - %.8X\nbit[2] - "
      "%.8X\nbit[1] - %.8X\nbit[0] - %.8X\n",
      get_sign(val) == POSITIVE ? "POSITIVE" : "NEGATIVE", get_exponent(val),
      val->bits[4], val->bits[3], val->bits[2], val->bits[1], val->bits[0]);
}

int made_first_bigger_no_signs(big_decimal* first, big_decimal* second) {
  for (int i = BIG_TOP_BIT; i >= BIG_BOT_BIT; --i) {
    if (first->bits[i] > second->bits[i]) {
      return TRUE;
    } else if (first->bits[i] < second->bits[i]) {
      swap_decimals(first, second);
      return FALSE;
    }
  }

  return FALSE;
}
