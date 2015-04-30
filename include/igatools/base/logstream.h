//-+--------------------------------------------------------------------
// This file is part of igatools, a general purpose Isogeometric analysis
// library. It was copied from the deal.II project where it is licensed
// under the LGPL (see http://www.dealii.org/).
// It has been modified by the igatools authors to fit the igatools framework.
//-+--------------------------------------------------------------------

#ifndef _logstream_h
#define _logstream_h

#include <igatools/base/config.h>
#include <igatools/base/exceptions.h>

#include <memory>
#include <string>
#include <stack>
#include <map>
#include <cmath>
#include <sstream>
#include <vector>

#include <sys/times.h>

IGA_NAMESPACE_OPEN

/**
 * A class that simplifies the process of execution logging. It does so by
 * providing
 * <ul>
 * <li> a push and pop mechanism for prefixes, and
 * <li> the possibility of distributing information to files and the
 *   console.
 * </ul>
 *
 * The usual usage of this class is through the pregenerated object
 * <tt>deallog</tt>. Typical steps are
 * <ul>
 * <li> <tt>deallog.attach(std::ostream)</tt>: write logging information into a file.
 * <li> <tt>deallog.depth_console(n)</tt>: restrict output on screen to outer loops.
 * <li> Before entering a new phase of your program, e.g. a new loop,
 *       <tt>LogStream::Prefix p("loopname")</tt>.
 * <li> <tt>deallog << anything << std::endl;</tt> to write logging information
 *       (Usage of <tt>std::endl</tt> is mandatory!).
 * <li> Destructor of the prefix will pop the prefix text from the stack.
 * </ul>
 *
 * <h3>LogStream and reproducible regression test output</h3>
 *
 * Generating reproducible floating point output for regression tests
 * is mildly put a nightmare. In order to make life a little easier,
 * LogStream implements a few features that try to achieve such a
 * goal. These features are turned on by calling test_mode(), and it
 * is not recommended to use them in any other environment. Right now,
 * LogStream implements the following:
 *
 * <ol>
 * <li> A double number very close to zero will end up being output in
 * exponential format, although it has no significant digits. The
 * parameter #double_threshold determines which numbers are too close
 * to zero to be considered nonzero.
 * <li> For float numbers holds the same, but with a typically larger
 * #float_threshold.
 * <li> Rounded numbers become unreliable with inexact
 * arithmetics. Therefore, adding a small number before rounding makes
 * results more reproducible, assuming that numbers like 0.5 are more
 * likely than 0.49997.
 * </ol>
 * It should be pointed out that all of these measures distort the
 * output and make it less accurate. Therefore, they are only
 * recommended if the output needs to be reproducible.
 *
 * @author Guido Kanschat, Wolfgang Bangerth, 1999, 2003, 2011
 */
class LogStream
{
public:
    /**
     * A subclass allowing for the
     * safe generation and removal of
     * prefices.
     *
     * Somewhere at the beginning of
     * a block, create one of these
     * objects, and it will appear as
     * a prefix in LogStream output
     * like @p deallog. At the end of
     * the block, the prefix will
     * automatically be removed, when
     * this object is destroyed.
     */
    class Prefix
    {
    public:
        /**
         * Set a new prefix for
         * @p deallog, which will be
         * removed when the variable
         * is destroyed .
         */
        Prefix(const std::string &text);

        /**
         * Set a new prefix for the
         * given stream, which will
         * be removed when the
         * variable is destroyed .
         */
        Prefix(const std::string &text, LogStream &stream);

        /**
         * Remove the prefix
         * associated with this
         * variable.
         */
        ~Prefix();

    private:
        std::shared_ptr<LogStream> stream;
    };

    /**
     * Standard constructor, since we
     * intend to provide an object
     * <tt>deallog</tt> in the library. Set the
     * standard output stream to <tt>std::cerr</tt>.
     */
    LogStream(const std::string head = std::string());

    /**
     * Destructor.
     */
    ~LogStream();

    /**
     * Enable output to a second
     * stream <tt>o</tt>.
     */
    void attach(std::ostream &o);

    /**
     * Disable output to the second
     * stream. You may want to call
     * <tt>close</tt> on the stream that was
     * previously attached to this object.
     */
    void detach();

    /**
     * Setup the logstream for
     * regression test mode.
     *
     * This sets the parameters
     * #double_threshold,
     * #float_threshold, and #offset
     * to nonzero values. The exact
     * values being used have been
     * determined experimentally and
     * can be found in the source
     * code.
     *
     * Called with an argument
     * <tt>false</tt>, switches off
     * test mode and sets all
     * involved parameters to zero.
     */
    void test_mode(bool on=true);

    /**
     * Gives the default stream (<tt>std_out</tt>).
     */
    std::ostream &get_console();

    /**
     * Gives the file stream.
     */
    std::ostream &get_file_stream();

    /**
     * @return true, if file stream
     * has already been attached.
     */
    bool has_file() const;

    /**
     * Reroutes cerr to LogStream.
     * Works as a switch, turning
     * logging of <tt>cerr</tt> on
     * and off alternatingly with
     * every call.
     */
    void log_cerr();

    /**
     * Return the prefix string.
     */
    const std::string &get_prefix() const;

    /**
     *
     *
     * Push another prefix on the
     * stack. Prefixes are
     * automatically separated by a
     * colon and there is a double
     * colon after the last prefix.
     */
    void push(const std::string &text);

    /**
     *
     *
     * Remove the last prefix.
     */
    void pop();

    void begin_item(const std::string &text)
    {
        *this << text << std::endl;
        push("   ");
    }

    void end_item()
    {
        pop();
        *this << std::endl;
    }
    /**
     * Maximum number of levels to be
     * printed on the console. This
     * function allows to restrict
     * console output to the upmost
     * levels of iterations. Only
     * output with less than <tt>n</tt>
     * prefixes is printed. By calling
     * this function with <tt>n=0</tt>, no
     * console output will be written.
     *
     * The previous value of this
     * parameter is returned.
     */
    unsigned int depth_console(const unsigned int n);

    /**
     * Maximum number of levels to be
     * written to the log file. The
     * functionality is the same as
     * <tt>depth_console</tt>, nevertheless,
     * this function should be used
     * with care, since it may spoile
     * the value of a log file.
     *
     * The previous value of this
     * parameter is returned.
     */
    unsigned int depth_file(const unsigned int n);

    /**
     * Set time printing flag. If this flag
     * is true, each output line will
     * be prepended by the user time used
     * by the running program so far.
     *
     * The previous value of this
     * parameter is returned.
     */
    bool log_execution_time(const bool flag);

    /**
     * Output time differences
     * between consecutive logs. If
     * this function is invoked with
     * <tt>true</tt>, the time difference
     * between the previous log line
     * and the recent one is
     * printed. If it is invoked with
     * <tt>false</tt>, the accumulated
     * time since start of the
     * program is printed (default
     * behavior).
     *
     * The measurement of times is
     * not changed by this function,
     * just the output.
     *
     * The previous value of this
     * parameter is returned.
     */
    bool log_time_differences(const bool flag);

    /**
     * Write detailed timing
     * information.
     *
     *
     */
    void timestamp();

    /**
     * Log the thread id.
     */
    bool log_thread_id(const bool flag);

    /**
     * Set a threshold for the
     * minimal absolute value of
     * double values. All numbers
     * with a smaller absolute value
     * will be printed as zero.
     *
     * The default value for this
     * threshold is zero,
     * i.e. numbers are printed
     * according to their Real value.
     *
     * This feature is mostly useful
     * for automated tests: there,
     * one would like to reproduce
     * the exact same solution in
     * each run of a
     * testsuite. However, subtle
     * difference in processor,
     * operating system, or compiler
     * version can lead to
     * differences in the last few
     * digits of numbers, due to
     * different rounding. While one
     * can avoid trouble for most
     * numbers when comparing with
     * stored results by simply
     * limiting the accuracy of
     * output, this does not hold for
     * numbers very close to zero,
     * i.e. zero plus accumulated
     * round-off. For these numbers,
     * already the first digit is
     * tainted by round-off. Using
     * the present function, it is
     * possible to eliminate this
     * source of problems, by simply
     * writing zero to the output in
     * this case.
     */
    void threshold_double(const double t);
    /**
     * The same as
     * threshold_double(), but for
     * float values.
     */
    void threshold_float(const float t);

    /**
     * The same as
     * threshold_double(), but for
     * long double values.
     */
    void threshold_long_double(const long double t);

    /**
     * Output a constant something
     * through this stream.
     */
    template <typename T>
    LogStream &operator << (const T &t);

    /**
     * Output long double precision
     * numbers through this
     * stream.
     *
     * If they are set, this function
     * applies the methods for making
     * floating point output
     * reproducible as discussed in
     * the introduction.
     */
    LogStream &operator << (const long double t);

    /**
     * Output double precision
     * numbers through this
     * stream.
     *
     * If they are set, this function
     * applies the methods for making
     * floating point output
     * reproducible as discussed in
     * the introduction.
     */
    LogStream &operator << (const double t);

    /**
     * Output single precision
     * numbers through this
     * stream.
     *
     * If they are set, this function
     * applies the methods for making
     * floating point output
     * reproducible as discussed in
     * the introduction.
     */
    LogStream &operator << (const float t);

    /**
     * Treat ostream
     * manipulators. This passes on
     * the whole thing to the
     * template function with the
     * exception of the
     * <tt>std::endl</tt>
     * manipulator, for which special
     * action is performed: write the
     * temporary stream buffer
     * including a header to the file
     * and <tt>std::cout</tt> and
     * empty the buffer.
     *
     * An overload of this function is needed
     * anyway, since the compiler can't bind
     * manipulators like @p std::endl
     * directly to template arguments @p T
     * like in the previous general
     * template. This is due to the fact that
     * @p std::endl is actually an overloaded
     * set of functions for @p std::ostream,
     * @p std::wostream, and potentially more
     * of this kind. This function is
     * therefore necessary to pick one
     * element from this overload set.
     */
    LogStream &operator<< (std::ostream& (*p)(std::ostream &));

    /**
     * Determine an estimate for
     * the memory consumption (in
     * bytes) of this
     * object. Since sometimes
     * the size of objects can
     * not be determined exactly
     * (for example: what is the
     * memory consumption of an
     * STL <tt>std::map</tt> type with a
     * certain number of
     * elements?), this is only
     * an estimate. however often
     * quite close to the true
     * value.
     */
    std::size_t memory_consumption() const;

    /**
     * Exception.
     */
    DeclException0(ExcNoFileStreamGiven);

private:

    /**
     * Stack of strings which are printed
     * at the beginning of each line to
     * allow identification where the
     * output was generated.
     */
    std::stack<std::string> prefixes;

    /**
     * Default stream, where the output
     * is to go to. This stream defaults
     * to <tt>std::cerr</tt>, but can be set to another
     * stream through the constructor.
     */
    std::ostream  *std_out;

    /**
     * Pointer to a stream, where a copy of
     * the output is to go to. Usually, this
     * will be a file stream.
     *
     * You can set and reset this stream
     * by the <tt>attach</tt> function.
     */
    std::ostream  *file;

    /**
     * Value denoting the number of
     * prefixes to be printed to the
     * standard output. If more than
     * this number of prefixes is
     * pushed to the stack, then no
     * output will be generated until
     * the number of prefixes shrinks
     * back below this number.
     */
    unsigned int std_depth;

    /**
     * Same for the maximum depth of
     * prefixes for output to a file.
     */
    unsigned int file_depth;

    /**
     * Flag for printing execution time.
     */
    bool print_utime;

    /**
     * Flag for printing time differences.
     */
    bool diff_utime;

    /**
     * Time of last output line.
     */
    double last_time;

    /**
     * Threshold for printing double
     * values. Every number with
     * absolute value less than this
     * is printed as zero.
     */
    double double_threshold;

    /**
     * Threshold for printing long double
     * values. Every number with
     * absolute value less than this
     * is printed as zero.
     */
    long double long_double_threshold;

    /**
     * Threshold for printing float
     * values. Every number with
     * absolute value less than this
     * is printed as zero.
     */
    float float_threshold;

    /**
     * An offset added to every float
     * or double number upon
     * output. This is done after the
     * number is compared to
     * #double_threshold or #float_threshold,
     * but before rounding.
     *
     * This functionality was
     * introduced to produce more
     * reproducible floating point
     * output for regression
     * tests. The rationale is, that
     * an exact output value is much
     * more likely to be 1/8 than
     * 0.124997. If we round to two
     * digits though, 1/8 becomes
     * unreliably either .12 or .13
     * due to machine accuracy. On
     * the other hand, if we add a
     * something above machine
     * accuracy first, we will always
     * get .13.
     *
     * It is safe to leave this
     * value equal to zero. For
     * regression tests, the function
     * test_mode() sets it to a
     * reasonable value.
     *
     * The offset is relative to the
     * magnitude of the number.
     */
    double offset;

    /**
     * Flag for printing thread id.
     */
    bool print_thread_id;

    /**
     * The value times() returned
     * on initialization.
     */
    double reference_time_val;

    /**
     * The tms structure times()
     * filled on initialization.
     */
    struct tms reference_tms;

    /**
     * Original buffer of
     * <tt>std::cerr</tt>. We store
     * the address of that buffer
     * when #log_cerr is called, and
     * reset it to this value if
     * #log_cerr is called a second
     * time, or when the destructor
     * of this class is run.
     */
    std::streambuf *old_cerr;

    /**
     * Print head of line. This prints
     * optional time information and
     * the contents of the prefix stack.
     */
    void print_line_head();

    /**
     * Actually do the work of
     * writing output. This function
     * unifies the work that is
     * common to the two
     * <tt>operator<<</tt> functions.
     */
    template <typename T>
    void print(const T &t);

    /**
     * Check if we are on a new line
     * and print the header before
     * the data.
     */
    std::ostringstream &get_stream();

    /**
     * Type of the stream map
     */
    typedef std::map<unsigned int, std::shared_ptr<std::ostringstream> > stream_map_type;

    /**
     * We generate a stringstream for
     * every process that sends log
     * messages.
     */
    stream_map_type outstreams;


    /** Bitmask keeping the format flags for the stream */
    std::ios_base::fmtflags format_flags_;

    /** Maximum number of digits to be written to express floating-point values. */
    int precision_ ;
};


/* ----------------------------- Inline functions and templates ---------------- */


template <class T>
inline
void
LogStream::print(const T &t)
{
    // if the previous command was an
    // <tt>std::endl</tt>, print the topmost
    // prefix and a colon
    std::ostringstream &stream = get_stream();
    stream << t;
    // print the rest of the message
//   if (prefixes.size() <= std_depth)
//     *std_out << t;

//   if (file && (prefixes.size() <= file_depth))
//     *file << t;
}



template <class T>
inline
LogStream &
LogStream::operator<< (const T &t)
{
    // do the work that is common to
    // the operator<< functions
    print(t);
    return *this;
}


inline
LogStream &
LogStream::operator<< (const long double t)
{
    // we have to make sure that we
    // don't catch NaN's and +-Inf's
    // with the test, because for these
    // denormals all comparisons are
    // always false. thus, for a NaN,
    // both t<=0 and t>=0 are false at
    // the same time, which can't be
    // said for any other number
    if (!(t<=0) && !(t>=0))
        print(t);
    else if (std::fabs(t) < long_double_threshold)
        print('0');
    else
        print(t*(1.+offset));

    return *this;
}


inline
LogStream &
LogStream::operator<< (const double t)
{
    // we have to make sure that we
    // don't catch NaN's and +-Inf's
    // with the test, because for these
    // denormals all comparisons are
    // always false. thus, for a NaN,
    // both t<=0 and t>=0 are false at
    // the same time, which can't be
    // said for any other number
    if (!(t<=0) && !(t>=0))
        print(t);
    else if (std::fabs(t) < double_threshold)
        print('0');
    else
        print(t*(1.+offset));

    return *this;
}



inline
LogStream &
LogStream::operator<< (const float t)
{
    // we have to make sure that we
    // don't catch NaN's and +-Inf's
    // with the test, because for these
    // denormals all comparisons are
    // always false. thus, for a NaN,
    // both t<=0 and t>=0 are false at
    // the same time, which can't be
    // said for any other number
    if (!(t<=0) && !(t>=0))
        print(t);
    else if (std::fabs(t) < float_threshold)
        print('0');
    else
        print(t*(1.+offset));

    return *this;
}


inline
LogStream::Prefix::Prefix(const std::string &text, LogStream &s)
    :
    stream(&s)
{
    stream->push(text);
}


inline
LogStream::Prefix::~Prefix()
{
    stream->pop();
}


#if 0

/**
 * Ouput for vector onto a LogStream.
 * Mostly use for debugging.
 *
 * @relates LogStream
 */
template <class T>
LogStream &operator<<(LogStream &out, const vector<T> &vector)
{
    out << "[ ";
    for (auto i:vector)
        out << i << " ";
    out << "]";
    return out;
}


/**
 * Ouput for std::array onto a LogStream.
 * Mostly use for debugging.
 *
 * @relates LogStream
 */
template <class T>
LogStream &operator<<(LogStream &out, const std::array<T, 5> &vector)
{
    out << "[ ";
    for (auto i:vector)
        out << i << " ";
    out << "]";
    return out;
}
#endif




IGA_NAMESPACE_CLOSE

#endif
