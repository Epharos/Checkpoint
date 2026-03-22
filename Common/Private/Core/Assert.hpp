#pragma once

#define CP_USE_ASSERTION
#define CP_USE_STANDARD_ASSERTION

#if defined(CP_USE_ASSERTION)
	#if defined(CP_USE_STANDARD_ASSERTION)
	#include <cassert>

		#define CP_ASSERT(cond) assert(cond)
		#define CP_ASSERT_MSG(cond, msg) assert(cond && msg)
		#define CP_EXPECT(cond) CP_ASSERT(cond)
		#define CP_EXPECT_MSG(cond, msg) CP_ASSERT_MSG(cond, msg)
		#define CP_ENSURE(cond) CP_ASSERT(cond)
		#define CP_ENSURE_MSG(cond, msg) CP_ASSERT_MSG(cond, msg)
	#else
	// TODO : Make my own version of assertions

		#define CP_ASSERT(cond)
		#define CP_ASSERT_MSG(cond, msg)
		#define CP_EXPECT(cond)
		#define CP_EXPECT_MSG(cond, msg)
		#define CP_ENSURE(cond)
		#define CP_ENSURE_MSG(cond, msg)
	#endif
#else
	#define CP_ASSERT(cond)
	#define CP_ASSERT_MSG(cond, msg)
	#define CP_EXPECT(cond)
	#define CP_EXPECT_MSG(cond, msg)
	#define CP_ENSURE(cond)
	#define CP_ENSURE_MSG(cond, msg)
#endif