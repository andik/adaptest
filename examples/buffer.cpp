#define ADAPTEST_BUFWRITE_FILE 1
#include <adaptest.h>
#include <adaptest/buf.h>
#include <sstream>
#include <iostream>

class SpecializedTestcase : 
	public AdapTest::BufferTestcase<AdapTest::CSVBufferWriter> {
public:
	static const int buflen = 50;
	int source[buflen];
	int compare[buflen];
	virtual void setUp() {
		for (int i = 0; i < buflen; ++i)
		{
			compare[i] = i;
			// tests have to fail - we want it this way
			source[i] =  i + 1;
		}
	}
};

TESTSUITE(BufferFails, SpecializedTestcase, "")
	TESTCASE(BufVsFloat, "")
		// test against an integer
		TEST(buf, source, 1, "buf")
	END_TESTCASE()
	TESTCASE(BufVsBuf, "")
		// test against an integer
		TEST(buf, source, compare, "buf")
	END_TESTCASE()
END_TESTSUITE()

ADAPTEST_MAIN(ConsoleLogger)