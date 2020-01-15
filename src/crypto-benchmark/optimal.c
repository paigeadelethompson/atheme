/*
 * SPDX-License-Identifier: ISC
 * SPDX-URL: https://spdx.org/licenses/ISC.html
 *
 * Copyright (C) 2019 Aaron M. D. Jones <aaronmdjones@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include <atheme/attributes.h>      // ATHEME_FATTR_WUR
#include <atheme/argon2.h>          // ATHEME_ARGON2_*
#include <atheme/digest.h>          // DIGALG_*, digest_oneshot_pbkdf2()
#include <atheme/pbkdf2.h>          // PBKDF2_*
#include <atheme/scrypt.h>          // ATHEME_SCRYPT_*
#include <atheme/stdheaders.h>      // (everything else)
#include <atheme/sysconf.h>         // HAVE_LIBARGON2

#ifdef HAVE_LIBARGON2
#  include <argon2.h>               // argon2_type, argon2_type2string()
#endif

#include "benchmark.h"              // (everything else)
#include "optimal.h"                // self-declarations

#ifdef HAVE_LIBARGON2

static bool ATHEME_FATTR_WUR
do_optimal_argon2_benchmark(const long double optimal_clocklimit, const size_t optimal_memlimit)
{
	(void) fprintf(stderr, "Beginning automatic optimal Argon2 benchmark ...\n");
	(void) fprintf(stderr, "\n");
	(void) fprintf(stderr, "NOTE: This does not test multithreading. Use '-a -p' for thread testing.\n");
	(void) fprintf(stderr, "\n");

	(void) argon2_print_colheaders();

	long double elapsed_prev = 0L;
	long double elapsed = 0L;
	size_t timecost_prev = 0U;

	const argon2_type type = Argon2_id;
	size_t memcost = optimal_memlimit;
	size_t timecost = ATHEME_ARGON2_TIMECOST_MIN;
	const size_t threads = 1ULL;

	// First try at our memory limit and the minimum time cost
	if (! benchmark_argon2(type, memcost, timecost, threads, &elapsed))
		// This function logs error messages on failure
		return false;

	// If that's still too slow, halve the memory usage until it isn't
	while (elapsed > optimal_clocklimit)
	{
		if (memcost <= ATHEME_ARGON2_MEMCOST_MIN)
		{
			(void) fprintf(stderr, "Reached minimum memory and time cost.\n");
			(void) fprintf(stderr, "Algorithm is still too slow; giving up.\n");
			(void) fprintf(stderr, "\n");
			(void) fflush(stderr);
			return true;
		}

		memcost--;

		if (! benchmark_argon2(type, memcost, timecost, threads, &elapsed))
			// This function logs error messages on failure
			return false;
	}

	// Now that it's fast enough, raise the time cost until it isn't
	while (elapsed < optimal_clocklimit)
	{
		elapsed_prev = elapsed;
		timecost_prev = timecost;
		timecost++;

		if (! benchmark_argon2(type, memcost, timecost, threads, &elapsed))
			// This function logs error messages on failure
			return false;
	}

	// If it was raised, go back to the previous loop's outputs (now that it's too slow)
	if (timecost_prev)
	{
		elapsed = elapsed_prev;
		timecost = timecost_prev;
	}

	(void) fprintf(stderr, "\n");
	(void) fprintf(stderr, "Recommended parameters:\n");
	(void) fprintf(stderr, "\n");
	(void) fflush(stderr);
	(void) fprintf(stdout, "crypto {\n");
	(void) fprintf(stdout, "\t/* Target: %LFs; Benchmarked: %LFs */\n", optimal_clocklimit, elapsed);
	(void) fprintf(stdout, "\targon2_type = \"%s\";\n", argon2_type2string(type, 0));
	(void) fprintf(stdout, "\targon2_memcost = %zu; /* %llu KiB */ \n", memcost, (1ULL << memcost));
	(void) fprintf(stdout, "\targon2_timecost = %zu;\n", timecost);
	(void) fprintf(stdout, "\targon2_threads = %zu;\n", threads);
	(void) fprintf(stdout, "};\n");
	(void) fflush(stdout);
	(void) fsync(fileno(stdout));
	(void) fprintf(stderr, "\n");
	(void) fprintf(stderr, "\n");
	(void) fprintf(stderr, "\n");
	(void) fflush(stderr);
	return true;
}

#endif /* HAVE_LIBARGON2 */

#ifdef HAVE_LIBSODIUM_SCRYPT

static bool ATHEME_FATTR_WUR
do_optimal_scrypt_benchmark(const long double optimal_clocklimit, const size_t optimal_memlimit)
{
	(void) fprintf(stderr, "Beginning automatic optimal scrypt benchmark ...\n");
	(void) fprintf(stderr, "\n");

	(void) scrypt_print_colheaders();

	long double elapsed_prev = 0L;
	long double elapsed = 0L;
	size_t opslimit_prev = 0U;

	/* For tuning, the recommendation is set opslimit to (memlimit / 32),
	 * but we interpret memlimit as a KiB power of 2, so we need to make
	 * memlimit a power of two, and then multiply it by 1024 (to make it
	 * a value in KiB), and then divide it by 32. Those last 2 operations
	 * are equivalent to just multiplying by 32. The benchmark_scrypt()
	 * function itself takes care of the above calculations as far as the
	 * memory limit is concerned.
	 */
	size_t memlimit = optimal_memlimit;
	size_t opslimit = ((1ULL << memlimit) * 32ULL);

	// First try at our memory limit and the corresponding default opslimit
	if (! benchmark_scrypt(memlimit, opslimit, &elapsed))
		// This function logs error messages on failure
		return false;

	// If that's still too slow, halve the memory limit and re-calculate opslimit until it isn't
	while (elapsed > optimal_clocklimit)
	{
		if (memlimit <= ATHEME_SCRYPT_MEMLIMIT_MIN)
		{
			(void) fprintf(stderr, "Reached minimum memory limit.\n");
			(void) fprintf(stderr, "Algorithm is still too slow; giving up.\n");
			(void) fprintf(stderr, "\n");
			(void) fflush(stderr);
			return true;
		}

		memlimit--;
		opslimit = ((1ULL << memlimit) * 32ULL);

		if (! benchmark_scrypt(memlimit, opslimit, &elapsed))
			// This function logs error messages on failure
			return false;
	}

	// Now that it's fast enough, raise the opslimit until it isn't
	while (elapsed < optimal_clocklimit)
	{
		elapsed_prev = elapsed;
		opslimit_prev = opslimit;
		opslimit *= 2U;

		if (! benchmark_scrypt(memlimit, opslimit, &elapsed))
			// This function logs error messages on failure
			return false;
	}

	// If it was raised, go back to the previous loop's outputs (now that it's too slow)
	if (opslimit_prev)
	{
		elapsed = elapsed_prev;
		opslimit = opslimit_prev;
	}

	(void) fprintf(stderr, "\n");
	(void) fprintf(stderr, "Recommended parameters:\n");
	(void) fprintf(stderr, "\n");
	(void) fflush(stderr);
	(void) fprintf(stdout, "crypto {\n");
	(void) fprintf(stdout, "\t/* Target: %LFs; Benchmarked: %LFs */\n", optimal_clocklimit, elapsed);
	(void) fprintf(stdout, "\tscrypt_memlimit = %zu; /* %llu KiB */ \n", memlimit, (1ULL << memlimit));
	(void) fprintf(stdout, "\tscrypt_opslimit = %zu;\n", opslimit);
	(void) fprintf(stdout, "};\n");
	(void) fflush(stdout);
	(void) fsync(fileno(stdout));
	(void) fprintf(stderr, "\n");
	(void) fprintf(stderr, "\n");
	(void) fprintf(stderr, "\n");
	(void) fflush(stderr);
	return true;
}

#endif /* HAVE_LIBSODIUM_SCRYPT */

static bool ATHEME_FATTR_WUR
do_optimal_pbkdf2_benchmark(const long double optimal_clocklimit)
{
	(void) fprintf(stderr, "Beginning automatic optimal PBKDF2 benchmark ...\n");
	(void) fprintf(stderr, "\n");
	(void) fprintf(stderr, "NOTE: This does not test SHA1. Use '-k -d' for SHA1 testing.\n");
	(void) fprintf(stderr, "\n");
	(void) fprintf(stderr, "NOTE: If you wish to support SASL SCRAM logins, please see the\n");
	(void) fprintf(stderr, "      'doc/SASL-SCRAM-SHA' file in the source code repository, whose parameter\n");
	(void) fprintf(stderr, "      advice takes precedence over the advice given by this benchmark utility!\n");
	(void) fprintf(stderr, "\n");

	(void) pbkdf2_print_colheaders();

	long double elapsed_sha256;
	long double elapsed_sha512;
	long double elapsed;

	enum digest_algorithm md;

#ifdef IN_CI_BUILD_ENVIRONMENT
	/* Go easier on Travis CI's build infrastructure;
	 * With the internal digest frontend, max takes upwards of 30 seconds!
	 *    -- amdj
	 */
	const size_t initial = PBKDF2_ITERCNT_DEF;
#else
	const size_t initial = PBKDF2_ITERCNT_MAX;
#endif

	if (! benchmark_pbkdf2(DIGALG_SHA2_512, initial, &elapsed_sha512))
		// This function logs error messages on failure
		return false;

	if (! benchmark_pbkdf2(DIGALG_SHA2_256, initial, &elapsed_sha256))
		// This function logs error messages on failure
		return false;

	if (elapsed_sha256 < elapsed_sha512)
	{
		md = DIGALG_SHA2_256;
		elapsed = elapsed_sha256;
	}
	else
	{
		md = DIGALG_SHA2_512;
		elapsed = elapsed_sha512;
	}

	/* PBKDF2 is pretty linear: There's only one parameter (iteration count), and it has
	 * almost perfect scaling on the algorithm's runtime. This enables a very simplified
	 * optimal parameter discovery process, compared to the other functions above.
	 */
	const char *const mdname = md_digest_to_name(md);
	size_t iterations = (size_t) (initial * (optimal_clocklimit / elapsed));
	iterations -= (iterations % 1000U);
	iterations = BENCH_MIN(initial, iterations);

	while (elapsed > optimal_clocklimit)
	{
		if (iterations <= PBKDF2_ITERCNT_MIN)
		{
			(void) fprintf(stderr, "Reached minimum iteration count.\n");
			(void) fprintf(stderr, "Algorithm is still too slow; giving up.\n");
			(void) fprintf(stderr, "\n");
			(void) fflush(stderr);
			return true;
		}

		iterations -= 1000U;

		if (! benchmark_pbkdf2(md, iterations, &elapsed))
			// This function logs error messages on failure
			return false;
	}

	(void) fprintf(stderr, "\n");
	(void) fprintf(stderr, "Recommended parameters:\n");
	(void) fprintf(stderr, "\n");
	(void) fflush(stderr);
	(void) fprintf(stdout, "crypto {\n");
	(void) fprintf(stdout, "\t/* Target: %LFs; Benchmarked: %LFs */\n", optimal_clocklimit, elapsed);
	(void) fprintf(stdout, "\tpbkdf2v2_digest = \"%s\";\n", mdname);
	(void) fprintf(stdout, "\tpbkdf2v2_rounds = %zu;\n", iterations);
	(void) fprintf(stdout, "};\n");
	(void) fflush(stdout);
	(void) fsync(fileno(stdout));
	(void) fprintf(stderr, "\n");
	(void) fprintf(stderr, "\n");
	(void) fprintf(stderr, "\n");
	(void) fflush(stderr);
	return true;
}

bool ATHEME_FATTR_WUR
do_optimal_benchmarks(const long double optimal_clocklimit, const size_t ATHEME_VATTR_MAYBE_UNUSED optimal_memlimit,
                      const bool ATHEME_VATTR_MAYBE_UNUSED optimal_memlimit_given)
{
#ifdef HAVE_ANY_MEMORY_HARD_ALGORITHM
	if (! optimal_memlimit_given)
	{
		(void) fprintf(stderr, "Be sure to specify -L/--optimal-memory-limit appropriately for this machine!\n");
		(void) fprintf(stderr, "\n");
		(void) fprintf(stderr, "\n");
		(void) fprintf(stderr, "\n");
	}
#endif

#ifdef HAVE_LIBARGON2
	if (! do_optimal_argon2_benchmark(optimal_clocklimit, optimal_memlimit))
		// This function logs error messages on failure
		return false;
#endif

#ifdef HAVE_LIBSODIUM_SCRYPT
	if (! do_optimal_scrypt_benchmark(optimal_clocklimit, optimal_memlimit))
		// This function logs error messages on failure
		return false;
#endif

	if (! do_optimal_pbkdf2_benchmark(optimal_clocklimit))
		// This function logs error messages on failure
		return false;

	return true;
}