#include "server/util/string/memmem.h"
#include <limits.h>
#include <stdint.h>
#include <string.h>

namespace ic {
namespace server {
namespace util {

#if CHAR_BIT < 10
#define LONG_NEEDLE_THRESHOLD 32U
#else
#define LONG_NEEDLE_THRESHOLD SIZE_MAX
#endif

#ifndef CANON_ELEMENT
#define CANON_ELEMENT(c) c
#endif
#ifndef CMP_FUNC
#define CMP_FUNC memcmp
#endif

#define hash2(p) (((size_t)(p)[0] - ((size_t)(p)[-1] << 3)) % sizeof(shift))

#define RETURN_TYPE void*
#define AVAILABLE(h, h_l, j, n_l) ((j) <= (h_l) - (n_l))

/* We use the Two-Way string matching algorithm, which guarantees
   linear complexity with constant space.  Additionally, for long
   needles, we also use a bad character shift table similar to the
   Boyer-Moore algorithm to achieve improved (potentially sub-linear)
   performance.

   See http://www-igm.univ-mlv.fr/~lecroq/string/node26.html#SECTION00260
   and http://en.wikipedia.org/wiki/Boyer-Moore_string_search_algorithm
*/

/* Point at which computing a bad-byte shift table is likely to be
   worthwhile.  Small needles should not compute a table, since it
   adds (1 << CHAR_BIT) + NEEDLE_LEN computations of preparation for a
   speedup no greater than a factor of NEEDLE_LEN.  The larger the
   needle, the better the potential performance gain.  On the other
   hand, on non-POSIX systems with CHAR_BIT larger than eight, the
   memory required for the table is prohibitive.  */
#if CHAR_BIT < 10
#define LONG_NEEDLE_THRESHOLD 32U
#else
#define LONG_NEEDLE_THRESHOLD SIZE_MAX
#endif

#ifndef CANON_ELEMENT
#define CANON_ELEMENT(c) c
#endif
#ifndef CMP_FUNC
#define CMP_FUNC memcmp
#endif

/* Check for end-of-line in strstr and strcasestr routines.
   We piggy-back matching procedure for detecting EOL where possible,
   and use AVAILABLE macro otherwise.  */
#ifndef CHECK_EOL
#define CHECK_EOL (0)
#endif

/* Return NULL if argument is '\0'.  */
#ifndef RET0_IF_0
#define RET0_IF_0(a) /* nothing */
#endif

/* Perform a critical factorization of NEEDLE, of length NEEDLE_LEN.
   Return the index of the first byte in the right half, and set
   *PERIOD to the global period of the right half.

   The global period of a string is the smallest index (possibly its
   length) at which all remaining bytes in the string are repetitions
   of the prefix (the last repetition may be a subset of the prefix).

   When NEEDLE is factored into two halves, a local period is the
   length of the smallest word that shares a suffix with the left half
   and shares a prefix with the right half.  All factorizations of a
   non-empty NEEDLE have a local period of at least 1 and no greater
   than NEEDLE_LEN.

   A critical factorization has the property that the local period
   equals the global period.  All strings have at least one critical
   factorization with the left half smaller than the global period.

   Given an ordered alphabet, a critical factorization can be computed
   in linear time, with 2 * NEEDLE_LEN comparisons, by computing the
   larger of two ordered maximal suffixes.  The ordered maximal
   suffixes are determined by lexicographic comparison of
   periodicity.  */
static size_t critical_factorization(const unsigned char* needle,
                                     size_t needle_len,
                                     size_t* period) {
    /* Index of last byte of left half, or SIZE_MAX.  */
    size_t max_suffix, max_suffix_rev;
    size_t j;           /* Index into NEEDLE for current candidate suffix.  */
    size_t k;           /* Offset into current period.  */
    size_t p;           /* Intermediate period.  */
    unsigned char a, b; /* Current comparison bytes.  */

    /* Invariants:
       0 <= j < NEEDLE_LEN - 1
       -1 <= max_suffix{,_rev} < j (treating SIZE_MAX as if it were signed)
       min(max_suffix, max_suffix_rev) < global period of NEEDLE
       1 <= p <= global period of NEEDLE
       p == global period of the substring NEEDLE[max_suffix{,_rev}+1...j]
       1 <= k <= p
    */

    /* Perform lexicographic search.  */
    max_suffix = SIZE_MAX;
    j = 0;
    k = p = 1;
    while (j + k < needle_len) {
        a = CANON_ELEMENT(needle[j + k]);
        b = CANON_ELEMENT(needle[max_suffix + k]);
        if (a < b) {
            /* Suffix is smaller, period is entire prefix so far.  */
            j += k;
            k = 1;
            p = j - max_suffix;
        } else if (a == b) {
            /* Advance through repetition of the current period.  */
            if (k != p) {
                ++k;
            }
            else {
                j += p;
                k = 1;
            }
        } else /* b < a */
        {
            /* Suffix is larger, start over from current location.  */
            max_suffix = j++;
            k = p = 1;
        }
    }
    *period = p;

    /* Perform reverse lexicographic search.  */
    max_suffix_rev = SIZE_MAX;
    j = 0;
    k = p = 1;
    while (j + k < needle_len) {
        a = CANON_ELEMENT(needle[j + k]);
        b = CANON_ELEMENT(needle[max_suffix_rev + k]);
        if (b < a) {
            /* Suffix is smaller, period is entire prefix so far.  */
            j += k;
            k = 1;
            p = j - max_suffix_rev;
        } else if (a == b) {
            /* Advance through repetition of the current period.  */
            if (k != p) {
                ++k;
            }
            else {
                j += p;
                k = 1;
            }
        } else /* a < b */
        {
            /* Suffix is larger, start over from current location.  */
            max_suffix_rev = j++;
            k = p = 1;
        }
    }

    /* Choose the longer suffix.  Return the first byte of the right
       half, rather than the last byte of the left half.  */
    if (max_suffix_rev + 1 < max_suffix + 1) {
        return max_suffix + 1;
    }
    *period = p;
    return max_suffix_rev + 1;
} // critical_factorization

static inline RETURN_TYPE two_way_long_needle(const unsigned char* haystack,
                                              size_t haystack_len,
                                              const unsigned char* needle,
                                              size_t needle_len) {
    size_t i;      /* Index into current byte of NEEDLE.  */
    size_t j;      /* Index into current window of HAYSTACK.  */
    size_t period; /* The period of the right half of needle.  */
    size_t suffix; /* The index of the right half of needle.  */
    size_t shift_table[1U << CHAR_BIT]; /* See below.  */

    /* Factor the needle into two halves, such that the left half is
       smaller than the global period, and the right half is
       periodic (with a period as large as NEEDLE_LEN - suffix).  */
    suffix = critical_factorization(needle, needle_len, &period);

    /* Populate shift_table.  For each possible byte value c,
       shift_table[c] is the distance from the last occurrence of c to
       the end of NEEDLE, or NEEDLE_LEN if c is absent from the NEEDLE.
       shift_table[NEEDLE[NEEDLE_LEN - 1]] contains the only 0.  */
    for (i = 0; i < 1U << CHAR_BIT; i++) {
        shift_table[i] = needle_len;
    }
    for (i = 0; i < needle_len; i++) {
        shift_table[CANON_ELEMENT(needle[i])] = needle_len - i - 1;
    }

    /* Perform the search.  Each iteration compares the right half
       first.  */
    if (CMP_FUNC(needle, needle + period, suffix) == 0) {
        /* Entire needle is periodic; a mismatch can only advance by the
       period, so use memory to avoid rescanning known occurrences
       of the period.  */
        size_t memory = 0;
        size_t shift;
        j = 0;
        while (AVAILABLE(haystack, haystack_len, j, needle_len)) {
            const unsigned char* pneedle;
            const unsigned char* phaystack;

            /* Check the last byte first; if it does not match, then
               shift to the next possible match location.  */
            shift = shift_table[CANON_ELEMENT(haystack[j + needle_len - 1])];
            if (0 < shift) {
                if (memory && shift < period) {
                    /* Since needle is periodic, but the last period has
                       a byte out of place, there can be no match until
                       after the mismatch.  */
                    shift = needle_len - period;
                }
                memory = 0;
                j += shift;
                continue;
            }
            /* Scan for matches in right half.  The last byte has
               already been matched, by virtue of the shift table.  */
            i = suffix > memory ? suffix : memory;
            pneedle = &needle[i];
            phaystack = &haystack[i + j];
            while (i < needle_len - 1 && (CANON_ELEMENT(*pneedle++) == CANON_ELEMENT(*phaystack++))) {
                ++i;
            }
            if (needle_len - 1 <= i) {
                /* Scan for matches in left half.  */
                i = suffix - 1;
                pneedle = &needle[i];
                phaystack = &haystack[i + j];
                while (memory < i + 1 && (CANON_ELEMENT(*pneedle--) == CANON_ELEMENT(*phaystack--))) {
                    --i;
                }
                if (i + 1 < memory + 1) {
                    return (RETURN_TYPE)(haystack + j);
                }
                /* No match, so remember how many repetitions of period
               on the right half were scanned.  */
                j += period;
                memory = needle_len - period;
            }
            else {
                j += i - suffix + 1;
                memory = 0;
            }
        }
    }
    else {
        /* The two halves of needle are distinct; no extra memory is
       required, and any mismatch results in a maximal shift.  */
        size_t shift;
        period = (suffix > needle_len - suffix ? suffix : needle_len - suffix) + 1;
        j = 0;
        while (AVAILABLE(haystack, haystack_len, j, needle_len)) {
            const unsigned char* pneedle;
            const unsigned char* phaystack;

            /* Check the last byte first; if it does not match, then
               shift to the next possible match location.  */
            shift = shift_table[CANON_ELEMENT(haystack[j + needle_len - 1])];
            if (0 < shift) {
                j += shift;
                continue;
            }
            /* Scan for matches in right half.  The last byte has
               already been matched, by virtue of the shift table.  */
            i = suffix;
            pneedle = &needle[i];
            phaystack = &haystack[i + j];
            while (i < needle_len - 1 && (CANON_ELEMENT(*pneedle++) == CANON_ELEMENT(*phaystack++))) {
                ++i;
            }
            if (needle_len - 1 <= i) {
                /* Scan for matches in left half.  */
                i = suffix - 1;
                pneedle = &needle[i];
                phaystack = &haystack[i + j];
                while (i != SIZE_MAX && (CANON_ELEMENT(*pneedle--) == CANON_ELEMENT(*phaystack--))) {
                    --i;
                }
                if (i == SIZE_MAX) {
                    return (RETURN_TYPE)(haystack + j);
                }
                j += period;
            }
            else {
                j += i - suffix + 1;
            }
        }
    }
    return NULL;
} // two_way_long_needle

void* memmem(const void* haystack,
               size_t hs_len,
               const void* needle,
               size_t ne_len) {
    const unsigned char* hs = (const unsigned char*)haystack;
    const unsigned char* ne = (const unsigned char*)needle;

    if (ne_len == 0) {
        return (void*)hs;
    }
    if (ne_len == 1) {
        return (void*)memchr(hs, ne[0], hs_len);
    }

    /* Ensure haystack length is >= needle length.  */
    if (hs_len < ne_len) {
        return NULL;
    }

    const unsigned char* end = hs + hs_len - ne_len;

    if (ne_len == 2) {
        uint32_t nw = ne[0] << 16 | ne[1], hw = hs[0] << 16 | hs[1];
        for (hs++; hs <= end && hw != nw;) {
            hw = hw << 16 | *++hs;
        }
        return hw == nw ? (void*)(hs - 1) : NULL;
    }

    /* Use Two-Way algorithm for very long needles.  */
    if (ne_len > 256) {
        return two_way_long_needle(hs, hs_len, ne, ne_len);
    }

    uint8_t shift[256];
    size_t tmp, shift1;
    size_t m1 = ne_len - 1;
    size_t offset = 0;

    memset(shift, 0, sizeof(shift));
    for (size_t i = 1; i < m1; i++) {
        shift[hash2(ne + i)] = (uint8_t)i;
    }
    /* Shift1 is the amount we can skip after matching the hash of the
       needle end but not the full needle.  */
    shift1 = m1 - shift[hash2(ne + m1)];
    shift[hash2(ne + m1)] = (uint8_t)m1;

    for (; hs <= end;) {
        /* Skip past character pairs not in the needle.  */
        do {
            hs += m1;
            tmp = shift[hash2(hs)];
        } while (tmp == 0 && hs <= end);

        /* If the match is not at the end of the needle, shift to the end
       and continue until we match the hash of the needle end.  */
        hs -= tmp;
        if (tmp < m1) {
            continue;
        }

        /* Hash of the last 2 characters matches.  If the needle is long,
       try to quickly filter out mismatches.  */
        if (m1 < 15 || memcmp(hs + offset, ne + offset, 8) == 0) {
            if (memcmp(hs, ne, m1) == 0) {
                return (void*)hs;
            }

            /* Adjust filter offset when it doesn't find the mismatch.  */
            offset = (offset >= 8 ? offset : m1) - 8;
        }

        /* Skip based on matching the hash of the needle end.  */
        hs += shift1;
    }
    return NULL;
} // memmem

}  // namespace util
}  // namespace server
}  // namespace ic
