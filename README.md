# AdapTest

Welcome to AdapTest. A simple yet usefull Unittest Framework for C++.

AdapTest is inpired by [lest](https://github.com/martinmoene/lest) and [catch](https://github.com/philsquared/Catch), but makes things a little different. 

* It uses one Class per Testcase, allowing them to inherit methods and properties
* It does use a single `TEST(xyz, ...)` Macro that maps to a `test_xyz(...)` memberfunction in the testcase class.
* this way one can easily provide new specialisations and adaptions to the tested software.
* it does not require C++11

## Example

this is a simple example of how testsuites are looking within AdapTest

```c++
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

ADAPTEST_MAIN()
```

## Installation

* copy `adaptest.h` in your project
* look at the head of `adaptest.h` to see the configuration macros
* create a header which includes the `adaptest.h` header and uses these configuration macros before including `adaptest.h`
* use the headers below the `adaptest/` folder to have more comparisons
  * `adaptest/float.h` adds `test_eq()` for floating point numbers
* write your testcase base classes which inherit from `AdapTest::Testcase` or any class defined in the `adaptest/` folder.
* write your testsuites. Tested is one executable per Testsuite. (As in the example)
* simply run the binaries. Currently no testcase selection is provided.
* if the Logger doesn't suite you, simply provide a new, inherited of the `AdapTest::Logger` Class

## how AdapTest works

I'll explain how AdapTest works, because it's quite simple and you'll see instantly what the macros do hide from you.
The following is the basic class layout of the example without any macro hideaway:

```c++
 #include <adaptest.h>

 class SpecializedTestcase : public AdapTest::Testcase {
   // setup, teardown etc.
 };

 // We needs to define a variable which holds all testcases for the suite.
 Testcases* MyTestsuiteStorage = 0;   
 // The testsuite itself
 class MyTestsuite;
 RegisterTestsuite<MyTestsuite> MyTestsuiteReg;
 class MyTestsuite 
    : public Testsuite< SpecializedTestcase, MyTestsuiteStorage > 
 {
    class MyTestcase;
    TestcaseRegistration<"simple test", MyTestcase, __LINE__> MyTestcase_reg;
    class MyTestcase : public SpecializedTestcase {
        Result run(std::ostream& failstream) {
            int i = 1;
            { 
                const Result result = test_eq(failstream, __LINE__, 1, i, "i");
                if (result != OK) return result;
            }
            bool mybool = false;
            { 
                const Result result = test_false(failstream, __LINE__, mybool, "mybool");
                if (result != OK) return result;
            }
            { 
                const Result result = test_true(failstream, __LINE__, mybool, "mybool");
                if (result != OK) return result;
            }
            return OK;
        }
    };
 };

 AdapTest::Testsuites* AdapTest::TestsuiteRegistration::storage = 0;
 int main(int argc, const char* argv[]) {
   AdapTest::ConsoleLogger logger;
   AdapTest::run(logger);
 }
```

It all works extremly simple:
* `TestcaseRegistration<>` adds a instance of `SpecialisedTestcase` to `MyTestsuiteStorage` upon it's instantiation (which is ordered by declaration or `TestcaseRegistration`'s in the class.)
* the `TestcaseRegistration<>` template is a subclass of `Testsuite<>`. Thus it can access it's static methods easily. It uses `Testsuite<>::addTestcase()` for the job described above.
* `RegisterTestsuite<>` registers an instance of  `MyTestsuite` for the call of `AdapTest::run()` it works the same way as `TestcaseRegistration<>` but on a global variable.
* the `TESTCASE()` macro also uses the `Testsuite<>` namespace: the Type ``Testsuite<>::LocalTestcase` defines the Type which `MyTestcase` inherits from.
* `AdapTest::run()` iterated through the registered Testsuites in `TestsuiteRegistration::storage` and calls `MyTestsuite::run(logger)`
* `MyTestsuite::run(logger)` iterates through `MyTestsuiteStorage` and calls `MyTestcase::run()` upon each testcase instance, logging the results using `logger`.
* `Testcase::test_eq()` returns a `Result` Struct which contains what happend (`FAILED`) and additional data such as a log message. `FAILED` causes `MyTestsuite::run()` to count the test as failed and write a log.
* The class names of testcases can get automatically generated based upon the `__LINE__` macro if `ADAPTEST_AUTONAMES` was defined to `1` before including `adaptest.h`
* a `TEST(...)` macro expands to a simple function call which can be implemented in the `SpecialisedTestcase` easily. This way we can easily extend the testability. p.e. `TEST(eq, ...)` will be `test_eq(...)` but it also returns when `test_eq()` fails.

The magic here is the automatic Testcase/Testsuite registration, so that one doesn't have to maintain a separate testcase/testsuite list or generate code. the idea was taken from the catch framework and adapted to use classes instead of functions.