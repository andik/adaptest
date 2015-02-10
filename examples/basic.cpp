#include <adaptest.h>

// specialized testcase base class
class SpecializedTestcase : public AdapTest::Testcase {
    // setup, teardown etc.
};

// MyTestsuite uses the specialised testcase base class
TESTSUITE(MyTestsuite, SpecializedTestcase, "a simple Testsuite")

  TESTCASE(mySimpleTest, "a simple Test")
    int i = 1;
    TEST(eq, 1, i, "i")

    bool mybool = false;
    TEST(false, mybool, "mybool")
    TEST(true,  mybool, "mybool") // will fail
  END_TESTCASE()  

END_TESTSUITE()

int main(int argc, const char* argv[]) {
  AdapTest::ConsoleLogger logger;
  MyTestsuite(logger).run();
}