SHELL = /bin/bash

default: lab0.c
	gcc -g lab0.c -o lab0

check: lab0
	@echo "Test input file" > input.txt
	@./lab0 --input=input.txt --output=output.txt
	@diff input.txt output.txt > /dev/null && echo "PASS Input output test" || echo "FAIL Input output test"

	@./lab0 --input=input.txt > output.txt > /dev/null 2>&1 || [ $$? -eq 0 ] && echo "PASS Existent input file test" || echo "FAIL Existent input file test"
	@./lab0 --input=nonexist.txt > output.txt > /dev/null 2>&1 || [ $$? -eq 1 ] && echo "PASS Non-existent input file test" || echo "FAIL Non-existent input file test"

	@echo "Non writable file" > nonwritable.txt
	@chmod -w nonwritable.txt
	@./lab0 --output=nonwritable.txt < input.txt > /dev/null 2>&1 || [ $$? -eq 2 ] && echo "PASS Non-writable output file test" || echo "FAIL Non-writable output file test"

	@echo "Segmentation fault" > segfault.txt
	@( ./lab0 --segfault; true ) >& segfault_output.txt | grep -o "Segmentation fault" segfault_output.txt > segfault_clean_output.txt
	@diff segfault.txt segfault_clean_output.txt > /dev/null && echo "PASS Segfault test" || echo "FAIL Sefgault test"

	@./lab0 --segfault --catch > /dev/null 2>&1 || [ $$? -eq 3 ] && echo "PASS Catch test: exit status" || echo "FAIL Catch test: exit status"
	@echo "Signal number 11 received. Quitting program now." > catch.txt
	@( ./lab0 --segfault --catch; true) >& catch_output.txt
	@diff catch.txt catch_output.txt > /dev/null && echo "PASS Catch test: message logged" || echo "FAIL Catch test: message logged"

	@rm -rf *.txt
	@echo "Finish testing"

clean:
	@rm -rf lab0 *.o *.tar.gz *~

dist:	lab0.c Makefile stack_backtrace.png breakpoint.png README
	@tar -cvzf lab0-804638846.tar.gz lab0.c Makefile stack_backtrace.png breakpoint.png README
