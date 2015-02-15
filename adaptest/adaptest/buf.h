#ifndef ADAPTEST_BUF_H
#define ADAPTEST_BUF_H

#include <adaptest.h>

// a Testcase Base Class for Adaptest which compares Buffers of a given type and
// is able to write the buffers and a matching gnuplot script to the filesystem
// upon test failure.

// upon a failed test: write a gnuplot script to visit the buffers written by
// ADAPTEST_BUFWRITE_FILE
#ifndef ADAPTEST_BUFWRITE_GNUPLOT
#define ADAPTEST_BUFWRITE_GNUPLOT 0
#endif // !ADAPTEST_BUFWRITE_GNUPLOT

#ifdef ADAPTEST_BUFWRITE_GNUPLOT
#ifndef ADAPTEST_BUFWRITE_FILE
#define ADAPTEST_BUFWRITE_FILE 1
#endif // !ADAPTEST_BUFWRITE_FILE
#endif // ADAPTEST_BUFWRITE_GNUPLOT

// upon a failed test: write the buffers into files.
#ifndef ADAPTEST_BUFWRITE_FILE
#define ADAPTEST_BUFWRITE_FILE 0
#endif // !ADAPTEST_BUFWRITE_FILE

#ifndef ADAPTEST_BUFWRITE_BINARY_FILENAME_FORMAT
#define ADAPTEST_BUFWRITE_BINARY_FILENAME_FORMAT "%s-%s.dat"
#endif // !ADAPTEST_BUFWRITE_BINARY_FILENAME_FORMAT

#ifndef ADAPTEST_BUFWRITE_CSV_FILENAME_FORMAT
#define ADAPTEST_BUFWRITE_CSV_FILENAME_FORMAT "%s.csv"
#endif // !ADAPTEST_BUFWRITE_CSV_FILENAME_FORMAT

#ifndef ADAPTEST_BUFWRITE_GNUPLOT_FILENAME_FORMAT
#define ADAPTEST_BUFWRITE_GNUPLOT_FILENAME_FORMAT "%s.gnuplot"
#endif

// Check config
#if ADAPTEST_BUFWRITE_GNUPLOT && !ADAPTEST_BUFWRITE_FILE
#error ADAPTEST_BUFWRITE_GNUPLOT requires ADAPTEST_BUFWRITE_FILE
#endif // ADAPTEST_BUFWRITE_GNUPLOT && !ADAPTEST_BUFWRITE_FILE

#ifdef ADAPTEST_BUFWRITE_FILE
#include <iostream>
#include <fstream>
#include <list>
#include <string>
#endif //ADAPTEST_BUFWRITE_FILE

namespace ADAPTEST_NAMESPACE {

#ifdef ADAPTEST_BUFWRITE_GNUPLOT  
  // for Gnuplot wee need some portable endianess checks
  enum Endianness { BIG, LITTLE, UNKNOWN };

  inline 
  Endianness endianness() {
    union Check {
      unsigned long longmem;
      unsigned char charmem [sizeof(unsigned long)];
    };
    Check checkvar;
    checkvar.longmem = 0xdeadbeef;
    if (checkvar.charmem[0] == 0xef) 
      return LITTLE;
    else if (checkvar.charmem[0] == 0xde) 
      return BIG;
    else
      return UNKNOWN;
  }

  template <class T> 
  struct BufferTypeToGnuplot {
    static const char * format() { return 0; }
  };

  #define ADAPTEST_BUFFER_GNUPLOT_MAPPING(_type, _str)                        \
    template<>                                                                \
    struct BufferTypeToGnuplot<_type> {                                       \
      static const char * format() { return _str; }                           \
    };

  ADAPTEST_BUFFER_GNUPLOT_MAPPING(float,          "%float32")
  ADAPTEST_BUFFER_GNUPLOT_MAPPING(double,         "%float64")
  ADAPTEST_BUFFER_GNUPLOT_MAPPING(long,           "%int32")
  ADAPTEST_BUFFER_GNUPLOT_MAPPING(unsigned long,  "%uint32")
  ADAPTEST_BUFFER_GNUPLOT_MAPPING(short,          "%int16")
  ADAPTEST_BUFFER_GNUPLOT_MAPPING(unsigned short, "%uint16")
  ADAPTEST_BUFFER_GNUPLOT_MAPPING(char,           "%int8")
  ADAPTEST_BUFFER_GNUPLOT_MAPPING(unsigned char,  "%uint8")
#endif

#ifdef ADAPTEST_BUFWRITE_FILE
#ifdef ADAPTEST_BUFWRITE_GNUPLOT
  template <class T>
  class BufferWriter {
  public:
    typedef T  value_type;
    typedef const T* pointer_type;

    struct Buffer {
      std::string name;
      pointer_type buf;
      size_t buflen;

      Buffer(std::string _name, pointer_type _buf, size_t _buflen)
      : name(_name)
      , buf(_buf)
      , buflen(_buflen)
      {}

    };

    typedef std::list<Buffer> BufferList;
    typedef typename BufferList::iterator BufferListIter;
    BufferList data;
    BufferList& getData() { return data; }
    std::ostream& msg;
    const int line;
    Testcase& testcase;

    virtual Result write() = 0;

    BufferWriter(
      std::ostream& _msg, const int _line, 
      class Testcase& _testcase)
    : msg(_msg)
    , line(_line)
    , testcase(_testcase)
    {}

    void add_buf(const T* buf, const size_t buflen, std::string name)
    {
      data.push_back(Buffer(name, buf, buflen));
    }

    Result error(std::string errmsg) {
      return testcase.error(msg, line, errmsg);
    }

    const char * getTestcaseName() {
      return testcase.getName().c_str();
    }
  };

  //--------------------------------------------------------------------------

  template <class T>
  class BinaryBufferWriter : public BufferWriter<T> {
    using typename BufferWriter<T>::Buffer;
    using typename BufferWriter<T>::BufferList;
    using typename BufferWriter<T>::BufferListIter;
    using BufferWriter<T>::getTestcaseName;
    using BufferWriter<T>::error;
    using BufferWriter<T>::write;
    using BufferWriter<T>::getData;
  public:
    BinaryBufferWriter(
      std::ostream& _msg, const int _line, 
      class Testcase& _testcase)
    : BufferWriter<T>(_msg,_line, _testcase)
    {}

    Result write_buffer(Buffer& d)
    {
      Formatted filename(ADAPTEST_BUFWRITE_BINARY_FILENAME_FORMAT, 
                         getTestcaseName(), d.name.c_str());
      std::fstream datafile(filename.ptr(), std::ios::out | std::ios::binary);
      
      if (!datafile.good()) {
        return error(Formatted("could not open %s", (const char *)filename));
      }

      // write the naked buffer into the output file
      datafile.write((char*)d.buf, d.buflen * sizeof(T));

      return OK;
    }

    #ifdef ADAPTEST_BUFWRITE_GNUPLOT
    Result write_gnuplot_for_buffer(std::ostream& gnuplot, Buffer& d)
    {
      Formatted filename(ADAPTEST_BUFWRITE_BINARY_FILENAME_FORMAT, 
                         getTestcaseName(), d.name.c_str());

      if (BufferTypeToGnuplot<T>::format()) {
        gnuplot 
          << "plot "
          << "\"" << filename << "\" "
          << "format=\"" << BufferTypeToGnuplot<T>::format() << "\" "
          << "record=" << d.buflen << " "
          << "endian=";
        if (endianness() == BIG)    gnuplot << "\"big\" ";
        else if (endianness() == LITTLE) gnuplot << "\"little\" ";
        else return error(Formatted("endianness was %i", endianness()));
        gnuplot << "with lines";
      } else {
        gnuplot
          << "# "
          << d.name
          << " has unknown type";
      }
      gnuplot << std::endl;

      return OK;
    }      
    #endif // ADAPTEST_BUFWRITE_GNUPLOT

    Result write() {
      #ifdef ADAPTEST_BUFWRITE_GNUPLOT
        Formatted gnuplot_filename(ADAPTEST_BUFWRITE_GNUPLOT_FILENAME_FORMAT, 
                                   getTestcaseName());
        std::fstream gnuplot_file(gnuplot_filename.ptr(), std::ios::out);
      #endif // ADAPTEST_BUFWRITE_GNUPLOT

      if (!gnuplot_file.good()) {
        return error(Formatted("could not open %s", 
                               (const char *)gnuplot_filename));
      }

      for (BufferListIter i = getData().begin(); i != getData().end(); ++i)
      {
        Buffer& data = *i;
        write_buffer(data);
        #ifdef ADAPTEST_BUFWRITE_GNUPLOT
          write_gnuplot_for_buffer(gnuplot_file, data);
        #endif // ADAPTEST_BUFWRITE_GNUPLOT
      }

      return OK;
    }    
  };

  //--------------------------------------------------------------------------

    template <class T>
    class CSVBufferWriter : public BufferWriter<T> {
      using typename BufferWriter<T>::Buffer;
      using typename BufferWriter<T>::BufferList;
      using typename BufferWriter<T>::BufferListIter;
      using BufferWriter<T>::getTestcaseName;
      using BufferWriter<T>::error;
      using BufferWriter<T>::write;
      using BufferWriter<T>::getData;
    public:
      CSVBufferWriter(
        std::ostream& _msg, const int _line, 
        class Testcase& _testcase)
      : BufferWriter<T>(_msg,_line, _testcase)
      {}

      virtual Result write_buffers(std::ostream& datafile)
      {
        // write the data columwise
        const size_t buflen = getData().begin()->buflen;
        for (int i = 0; i < buflen; ++i)
        {
          for (BufferListIter it = getData().begin(); it != getData().end(); ++it)
          {
            if (it != getData().begin()) datafile << ",";
            Buffer& data = *it;
            datafile << data.buf[i];
          }
          datafile << std::endl;
        }

        return OK;
      }

      #ifdef ADAPTEST_BUFWRITE_GNUPLOT
      virtual Result write_gnuplot(std::ostream& gnuplot, Formatted& filename)
      {
        int bufidx = 0;
        for (BufferListIter i = getData().begin(); i != getData().end(); ++i)
        {
          gnuplot 
            << "plot "
            << "\"" << filename << "\" "
            << "using " << bufidx << " "
            << "with lines"
            << std::endl;

          bufidx++;
        }
        return OK;
      }      
      #endif // ADAPTEST_BUFWRITE_GNUPLOT

      Result write() {
        Formatted filename(
          ADAPTEST_BUFWRITE_CSV_FILENAME_FORMAT, getTestcaseName());
        std::fstream datafile(filename.ptr(), std::ios::out);
        
        if (!datafile.good()) {
          return error(Formatted("could not open %s", (const char *)filename));
        }

        #ifdef ADAPTEST_BUFWRITE_GNUPLOT
          Formatted gnuplot_filename(ADAPTEST_BUFWRITE_GNUPLOT_FILENAME_FORMAT, 
                                     getTestcaseName());        
          std::fstream gnuplot_file(gnuplot_filename.ptr(), std::ios::out);
        #endif // ADAPTEST_BUFWRITE_GNUPLOT

        if (!gnuplot_file.good()) {
          return error(Formatted("could not open %s", 
                                 (const char *)gnuplot_filename));
        }

        write_buffers(datafile);
        #ifdef ADAPTEST_BUFWRITE_GNUPLOT
          write_gnuplot(gnuplot_file, filename);
        #endif // ADAPTEST_BUFWRITE_GNUPLOT

        return OK;
      }
    };


#endif // ADAPTEST_BUFWRITE_GNUPLOT
#endif // ADAPTEST_BUFWRITE_FILE  

  // ======================================================================== 

  template <template <class> class WriterPolicy>
  class BufferTestcase : public virtual Testcase {
  public:
    // make test_eq overridable
    using Testcase::test_eq;

    template <class T>
    Result test_buf( 
      std::ostream& msg, const int line, 
      const size_t buflen, const T* buf, const T* expected, std::string name)
    {
      Result res = OK;
      for (size_t i=0; i<buflen; i++){
        res = test_eq(msg, line, expected[i], buf[i], 
                      Formatted("%s[%i]", name.c_str(), i));
        if (res != OK ) break;
      }

      #ifdef ADAPTEST_BUFWRITE_FILE
        if (res != OK) {
          WriterPolicy<T> writer(msg, line, *this);
          writer.add_buf(buf, buflen, name);
          writer.add_buf(expected, buflen, Formatted("%s-expected", name.c_str()));
          writer.write();
        }
      #endif // ADAPTEST_BUFWRITE_FILE

      return res;
    }

    template <class T>
    Result test_buf( 
      std::ostream& msg, const int line, 
      const T* buf, const size_t buflen, const T expected, std::string name)
    {
      T expectedBuf[buflen];
      for (size_t i=0; i<buflen; i++){
        expectedBuf[i] = expected;
      }
      return test_buf(msg, line, buflen, buf, expectedBuf, 
                      Formatted("%s-expected", name.c_str()));
    }    

    template <class T>
    Result test_buf( 
      std::ostream& msg, const int line, 
      const size_t buflen, const T* buf, const size_t offset, const size_t step, 
      const T* expected, std::string name)
    {
      Result res = OK;
      for (size_t i=0; i<buflen; i++){
        res = test_eq(msg, line, expected[i], buf[i], 
                      Formatted("%s[%i]", name.c_str(), i));
        if (res != OK ) break;
      }

      #ifdef ADAPTEST_BUFWRITE_FILE
        if (res != OK) {
          WriterPolicy<T> writer(msg, line, *this);
          writer.add_buf(buf, buflen, name);
          writer.add_buf(expected, buflen, Formatted("%s-expected", name.c_str()));
          writer.write();
        }
      #endif // ADAPTEST_BUFWRITE_FILE

      return res;
    }

    template <class T, size_t N>
    Result test_buf( 
      std::ostream& msg, const int line, 
      const T (&buf)[N], const T expected, std::string name)
    {
      return test_buf(msg, line, &buf[0], N, expected, name);
    }

    template <class T, size_t N>
    Result test_buf( 
      std::ostream& msg, const int line, 
      const T (&buf)[N], const T (&expected)[N], std::string name)
    {
      return test_buf(msg, line, N, &buf[0], &expected[0], name);
    }    

  };
} // namespace ADAPTEST_NAMESPACE

#endif //ADAPTEST_BUF_H