#define __assume(c) if (!(c)) { __builtin_unreachable(); }

struct ce_only { ce_only() = delete; };

template <typename T> struct float_traits;

template <>
struct float_traits <__float32> final : ce_only
{
  using float_t = __float32;
	
  static constexpr const __uint8 significand_bits = 23;
  static constexpr const __uint8 exponent_bits = 8;
  static constexpr const __uint8 significand_offset = 0;
  static constexpr const __uint8 exponent_offset = significand_bits;
  static constexpr const __uint8 sign_offset = exponent_offset + exponent_bits;
  static constexpr const __uint8 bias = 127;

  using uint_t = __uint32;
  using sig_t = __uint24;
  using exp_t = __uint8;
  static constexpr const __uint8 bytes = sizeof(float_t);
  static constexpr const __uint8 bits = bytes * 8;
  static_assert(bits == (significand_bits + exponent_bits + 1), "float_trait size mismatch");
  static_assert(sizeof(uint_t) == sizeof(float_t), "float-integer size mismatch");
};

template <>
struct float_traits <__float64> final : ce_only
{
  using float_t = __float64;
	
  static constexpr const __uint8 significand_bits = 52;
  static constexpr const __uint8 exponent_bits = 11;
  static constexpr const __uint8 significand_offset = 0;
  static constexpr const __uint8 exponent_offset = significand_bits;
  static constexpr const __uint8 sign_offset = exponent_offset + exponent_bits;
  static constexpr const __uint16 bias = 1023;

  using uint_t = __uint64;
  using sig_t = __uint56;
  using exp_t = __uint16;
  static constexpr const __uint8 bytes = sizeof(float_t);
  static constexpr const __uint8 bits = bytes * 8;
  static_assert(bits == (significand_bits + exponent_bits + 1), "float_trait size mismatch");
  static_assert(sizeof(uint_t) == sizeof(float_t), "float-integer size mismatch");
};

template <>
struct float_traits <__float16> final : ce_only
{
  using float_t = __float16;
	
  static constexpr const __uint8 significand_bits = 10;
  static constexpr const __uint8 exponent_bits = 5;
  static constexpr const __uint8 significand_offset = 0;
  static constexpr const __uint8 exponent_offset = significand_bits;
  static constexpr const __uint8 sign_offset = exponent_offset + exponent_bits;
  static constexpr const __uint16 bias = 15;

  using uint_t = __uint16;
  using sig_t = __uint16;
  using exp_t = __uint8;
  static constexpr const __uint8 bytes = sizeof(float_t);
  static constexpr const __uint8 bits = bytes * 8;
  static_assert(bits == (significand_bits + exponent_bits + 1), "float_trait size mismatch");
  static_assert(sizeof(uint_t) == sizeof(float_t), "float-integer size mismatch");
};

template <>
struct float_traits <__float24> final : ce_only
{
  using float_t = __float24;
	
  static constexpr const __uint8 significand_bits = 16;
  static constexpr const __uint8 exponent_bits = 7;
  static constexpr const __uint8 significand_offset = 0;
  static constexpr const __uint8 exponent_offset = significand_bits;
  static constexpr const __uint8 sign_offset = exponent_offset + exponent_bits;
  static constexpr const __uint8 bias = 63;

  using uint_t = __uint24;
  using sig_t = __uint24;
  using exp_t = __uint8;
  static constexpr const __uint8 bytes = sizeof(float_t);
  static constexpr const __uint8 bits = bytes * 8;
  static_assert(bits == (significand_bits + exponent_bits + 1), "float_trait size mismatch");
  static_assert(sizeof(uint_t) == sizeof(float_t), "float-integer size mismatch");
};

template <typename T> using arg_t = const T & __restrict;

template <typename T> static constexpr const __uint8 bitsize_of = sizeof(T) * 8;

template <typename T, __uint8 bits> static constexpr const T bit_val = (bits == bitsize_of<T>) ? ~T(0) : ((T(1) << bits) - 1);

template <typename F>
static constexpr void validate_type()
{
  using traits = float_traits<F>;
  static_assert((traits::significand_bits + traits::exponent_bits + 1) == traits::bits, "floating-point format mismatch");
  static_assert(traits::significand_offset == 0, "floating-point format assumption error");
  static_assert(traits::exponent_offset == traits::significand_bits, "floating-point format assumption error");
  static_assert(traits::sign_offset == (traits::significand_bits + traits::exponent_bits), "floating-point format assumption error");
}

template <typename T1, typename T2> struct _is_same_t final : ce_only { static constexpr const bool value = false; };
template <typename T1> struct _is_same_t<T1, T1> final : ce_only { static constexpr const bool value = true; };
template <typename T1, typename T2> static constexpr const bool is_same = _is_same_t<T1, T2>::value;

template <bool, typename T1, typename T2> struct _conditional_t final : ce_only { using type = T2; };
template <typename T1, typename T2> struct _conditional_t<true, T1, T2> final : ce_only { using type = T1; };
template <bool b, typename T1, typename T2> using conditional = typename _conditional_t<b, T1, T2>::type;

template <typename T, typename U>
static constexpr conditional<sizeof(T) >= sizeof(U), T, U> difference(T a, U b)
{
  return (a >= b) ? (a - b) : (b - a);
}

template <typename Fout, typename Fin>
static inline __attribute__((const, flatten, always_inline)) Fout convert_to(Fin in)
{
  if constexpr (is_same<Fout, Fin>)
  {
    return in;
  }
  using Tout = float_traits<Fout>;
  using Tin = float_traits<Fin>;
  validate_type<Fout>();
  validate_type<Fin>();
  using uint_out = typename Tout::uint_t;
  using uint_in = typename Tin::uint_t;
  static_assert(sizeof(uint_out) == Tout::bytes, "uint_out / Fout size mismatch");
  static_assert(sizeof(uint_in) == Tin::bytes, "uint_in / Fin size mismatch");
  using uint_larger = conditional<sizeof(uint_out) >= sizeof(uint_in), uint_out, uint_in>;

  union in_union_t
  {
    const Fin value_;
    struct data
    {
      uint_in significand : Tin::significand_bits;
      uint_in exponent : Tin::exponent_bits;
      uint_in sign : 1;
    } const data_;

    in_union_t(arg_t<Fin> val) __attribute__((flatten, always_inline)) : value_(val) {}
  } const in_expand = { in };

  union out_union_t
  {
    const Fout value_;
    struct data
    {
      uint_out significand : Tout::significand_bits;
      uint_out exponent : Tout::exponent_bits;
      uint_out sign : 1;

      data(arg_t<in_union_t> in) __attribute__((flatten, always_inline)) :
        significand(
          (Tin::significand_bits >= Tout::significand_bits) ?
          ({__assume((in.data_.significand <= bit_val<typename Tin::sig_t, Tin::significand_bits>)); (uint_larger(in.data_.significand) >> difference(Tin::significand_bits, Tout::significand_bits));}) :
          ({__assume((in.data_.significand <= bit_val<typename Tin::sig_t, Tin::significand_bits>)); (uint_larger(in.data_.significand) << difference(Tin::significand_bits, Tout::significand_bits));})
        ),
        exponent(
          (Tin::bias >= Tout::bias) ?
          ({__assume((in.data_.exponent <= bit_val<typename Tin::exp_t, Tin::exponent_bits>)); (uint_larger(in.data_.exponent) - difference(Tin::bias, Tout::bias));}) :
          ({__assume((in.data_.exponent <= bit_val<typename Tin::exp_t, Tin::exponent_bits>)); (uint_larger(in.data_.exponent) + difference(Tin::bias, Tout::bias));})
        ),
        sign(in.data_.sign)
      {}
    } const data_;

    out_union_t(arg_t<in_union_t> in) __attribute__((flatten, always_inline)) : data_(in) {}
  };

  const out_union_t out_expand{ in_expand };

  return out_expand.value_;
}

#define __attrib __attribute__((const, flatten))

extern "C"
{

	__attrib __float24 __extendhfpsf2 (__float16 val)
	{
		return convert_to<__float24>(val);
	}
	__attrib __float32 __extendhfsf2 (__float16 val)
	{
		return convert_to<__float32>(val);
	}
	__attrib __float64 __extendhfdf2 (__float16 val)
	{
		return convert_to<__float64>(val);
	}
	__attrib __float32 __extendpsfsf2 (__float24 val)
	{
		return convert_to<__float32>(val);
	}
	__attrib __float64 __extendpsfdf2 (__float24 val)
	{
		return convert_to<__float64>(val);
	}
	__attrib __float64 __extendsfdf2 (__float32 val)
	{
		return convert_to<__float64>(val);
	}

	__attrib __float16 __truncpsfhf2 (__float24 val)
	{
		return convert_to<__float16>(val);
	}
	__attrib __float24 __truncsfpsf2 (__float32 val)
	{
		return convert_to<__float24>(val);
	}
	__attrib __float16 __truncsfhf2 (__float32 val)
	{
		return convert_to<__float16>(val);
	}
	__attrib __float32 __truncdfsf2 (__float64 val)
	{
		return convert_to<__float32>(val);
	}
	__attrib __float24 __truncdfpsf2 (__float64 val)
	{
		return convert_to<__float24>(val);
	}
	__attrib __float16 __truncdfhf2 (__float64 val)
	{
		return convert_to<__float16>(val);
	}

}

// __addpsf3 et al needed

/*
HFtype
__trunctfhf2 (TFtype a)
{
  FP_DECL_EX;
  FP_DECL_Q (A);
  FP_DECL_H (R);
  HFtype r;

  FP_INIT_ROUNDMODE;
  FP_UNPACK_SEMIRAW_Q (A, a);
#if (2 * _FP_W_TYPE_SIZE) < _FP_FRACBITS_Q
  FP_TRUNC (H, Q, 1, 4, R, A);
#else
  FP_TRUNC (H, Q, 1, 2, R, A);
#endif
  FP_PACK_SEMIRAW_H (r, R);
  FP_HANDLE_EXCEPTIONS;

  return r;
}
*/
