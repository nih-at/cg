EXTRA_DIST=runtest ${TESTCASES}
	mime-mixed.in mime-mixed.log mime-mixed.out.1 mime-mixed.out.2 \
	uu-multi.in.1 uu-multi.in.2 uu-multi.in.3 uu-multi.log \
		uu-multi.out.1 uu-multi.out.2
	uu-short.in uu-short.log uu-short.out.1 \
	yenc-single.in yenc-single.log yenc-single.out.1 \
	yenc-multi.in.1 yenc-multi.in.2 yenc-multi.log yenc-multi.out.1

TESTCASES=mime-mixed.test uu-multi.test uu-short.test \
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
