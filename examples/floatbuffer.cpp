#define ADAPTEST_BUFWRITE_FILE 1
#include <adaptest.h>
#include <adaptest/buf.h>
#include <adaptest/float.h>
#include <sstream>
#include <iostream>

class SpecializedTestcase : 
	public AdapTest::FloatingPointTestcase,
	public AdapTest::BufferTestcase<AdapTest::CSVBufferWriter> {
public:
	static const int buflen = 50;
	float source[buflen];
	float compare[buflen];
	virtual void setUp() {
		for (int i = 0; i < buflen; ++i)
		{
			compare[i] = (float)i;
			// tests have to fail - we want it this way
			source[i] =  (float)i + 1;
		}
	}
};

TESTSUITE(FloatBufferFails, SpecializedTestcase, "")
	TESTCASE(BufVsFloat, "")
		// test against an integer
		TEST(buf, source, 1.0f, "buf")
	END_TESTCASE()
	TESTCASE(BufVsBuf, "")
		// test against an integer
		TEST(buf, source, compare, "buf")
	END_TESTCASE()
END_TESTSUITE()

int main(int argc, char const *argv[])
{
	AdapTest::ConsoleLogger logger;
	FloatBufferFails(logger).run();
	return logger.getFailed();
}