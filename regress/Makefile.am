EXTRA_DIST=runtest ${TESTCASES} \
	test.zip testfile.txt testlatin.txt true.sh xteddy.xbm \
	mime-mixed.desc mime-mixed.in mime-mixed.log \
	mime-multi.in.1 mime-multi.in.2 mime-multi.log \
	mime-quot.desc mime-quot.in mime-quot.log \
	uu-multi.desc uu-multi.in.1 uu-multi.in.2 uu-multi.in.3 uu-multi.log \
	uu-short.in uu-short.log \
	yenc-multi.in.1 yenc-multi.in.2 yenc-multi.in.3 yenc-multi.log \
	yenc-single.in yenc-single.log

TESTCASES=mime-mixed.test mime-multi.test mime-quot.test \
	uu-multi.test uu-short.test \
	yenc-single.test yenc-multi.test

check:
	@failed=0; all=0; \
	srcdir=$(srcdir); export srcdir; \
	for tst in ${TESTCASES}; do \
	  t=`basename $$tst .test`; \
	  if ${TESTS_ENVIRONMENT} $(srcdir)/runtest $$t; then \
	    all=`expr $$all + 1`; \
	    echo "PASS: $$tst"; \
	  elif test $$? -ne 77; then \
	    all=`expr $$all + 1`; \
	    failed=`expr $$failed + 1`; \
	    echo "FAIL: $$tst"; \
	  fi; \
	done; \
	if test "$$failed" -eq 0; then \
	  banner="All $$all tests passed"; \
	else \
	  banner="$$failed of $$all tests failed"; \
	fi; \
	dashes=`echo "$$banner" | sed s/./=/g`; \
	echo "$$dashes"; \
	echo "$$banner"; \
	echo "$$dashes"; \
	test "$$failed" -eq 0
