In file ThreadPool.h line 10 there's a code line:
//#define NotPooled true
If this line is commmented the program will be real pool threaded.
If this line is not commented the program will create threads after request,
and not be pool threaded.

The make file expect you to have clang installed.

